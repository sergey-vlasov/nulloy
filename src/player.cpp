/********************************************************************
**  Nulloy Music Player, http://nulloy.com
**  Copyright (C) 2010-2024 Sergey Vlasov <sergey@vlasov.me>
**
**  This program can be distributed under the terms of the GNU
**  General Public License version 3.0 as published by the Free
**  Software Foundation and appearing in the file LICENSE.GPL3
**  included in the packaging of this file.  Please review the
**  following information to ensure the GNU General Public License
**  version 3.0 requirements will be met:
**
**  http://www.gnu.org/licenses/gpl-3.0.html
**
*********************************************************************/

#include "player.h"

#include "action.h"
#include "common.h"
#include "coverWidget.h"
#include "dialogHandler.h"
#include "i18nLoader.h"
#include "image.h"
#include "logDialog.h"
#include "mainWindow.h"
#include "playbackEngineInterface.h"
#include "playlistController.h"
#include "playlistDataItem.h"
#include "playlistModel.h"
#include "playlistStorage.h"
#include "playlistWidget.h"
#include "playlistWidgetItem.h"
#include "pluginLoader.h"
#include "preferencesDialogHandler.h"
#include "scriptEngine.h"
#include "settings.h"
#include "svgImage.h"
#include "tagEditorDialog.h"
#include "trackInfoModel.h"
#include "trackInfoReader.h"
#include "trackInfoWidget.h"
#include "utils.h"
#include "volumeSlider.h"
#include "waveformBar.h"
#include "waveformBuilderInterface.h"
#include "waveformSlider.h"

#ifndef _N_NO_SKINS_
#include "skinFileSystem.h"
#include "skinLoader.h"
#endif

#ifdef Q_OS_WIN
#include "w7TaskBar.h"
#include "winIcon.h"
#endif

#ifdef Q_OS_MAC
#include "macDock.h"
#endif

#include <QFileDialog>
#include <QFileInfo>
#include <QGuiApplication>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QMetaObject>
#include <QQmlApplicationEngine>
//#include <QQuickWindow>
#include <QResizeEvent>
#include <QToolTip>

#ifndef _N_NO_UPDATE_CHECK_
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#endif

static const qreal kLog10over20 = qLn(10) / 20;

NPlayer::NPlayer()
{
    qsrand((uint)QTime::currentTime().msec());
    m_settings = NSettings::instance();

    QString styleName = m_settings->value("Style").toString();
    if (!styleName.isEmpty()) {
        QApplication::setStyle(styleName);
    }

    NI18NLoader::init();
    NPluginLoader::init();

    m_trackInfoReader = new NTrackInfoReader(dynamic_cast<NTagReaderInterface *>(
                                                 NPluginLoader::getPlugin(N::TagReader)),
                                             this);

    m_coverReader = dynamic_cast<NCoverReaderInterface *>(NPluginLoader::getPlugin(N::CoverReader));

    m_playbackEngine = dynamic_cast<NPlaybackEngineInterface *>(
        NPluginLoader::getPlugin(N::PlaybackEngine));
    Q_ASSERT(m_playbackEngine);
    m_playbackEngine->setParent(this);

#ifndef _N_NO_SKINS_
    m_mainWindow = new NMainWindow(NSkinLoader::skinUiFormFile());
#else
    m_mainWindow = new NMainWindow();
#endif

    // loading skin script
    m_scriptEngine = new NScriptEngine(this);
#ifndef _N_NO_SKINS_
    QString scriptFileName(NSkinLoader::skinScriptFile());
#else
    QString scriptFileName(":skins/native/script.js");
#endif
    QFile scriptFile(scriptFileName);
    scriptFile.open(QIODevice::ReadOnly);
    m_scriptEngine->evaluate(scriptFile.readAll(), scriptFileName);
    scriptFile.close();
    QScriptValue skinProgram = m_scriptEngine->evaluate("Main").construct();

    m_logDialog = new NLogDialog(m_mainWindow);
    m_preferencesDialogHandler = new NPreferencesDialogHandler(m_mainWindow);
    m_volumeSlider = m_mainWindow->findChild<NVolumeSlider *>("volumeSlider");
    m_coverWidget = m_mainWindow->findChild<NCoverWidget *>("coverWidget");

    m_playlistWidget = m_mainWindow->findChild<NPlaylistWidget *>("playlistWidget");
    if (QAbstractButton *repeatButton = m_mainWindow->findChild<QAbstractButton *>(
            "repeatButton")) {
        repeatButton->setChecked(m_playlistWidget->repeatMode());
    }
    m_playlistWidget->setTrackInfoReader(m_trackInfoReader);

    m_trackInfoWidget = new NTrackInfoWidget();
    m_trackInfoWidget->setTrackInfoReader(m_trackInfoReader);
    connect(m_trackInfoWidget, SIGNAL(showToolTip(const QString &)), this,
            SLOT(showToolTip(const QString &)));
    QVBoxLayout *trackInfoLayout = new QVBoxLayout;
    trackInfoLayout->setContentsMargins(0, 0, 0, 0);
    trackInfoLayout->addWidget(m_trackInfoWidget);
    m_waveformSlider = m_mainWindow->findChild<NWaveformSlider *>("waveformSlider");
    m_waveformSlider->setLayout(trackInfoLayout);
    m_trackInfoModel = new NTrackInfoModel(m_trackInfoReader, this);

#ifndef _N_NO_UPDATE_CHECK_
    m_versionDownloader = new QNetworkAccessManager(this);
    connect(m_versionDownloader, SIGNAL(finished(QNetworkReply *)), this,
            SLOT(on_versionDownloader_finished(QNetworkReply *)));
    connect(m_preferencesDialogHandler, SIGNAL(versionRequested()), this, SLOT(downloadVersion()));
#endif

    createActions();
    loadSettings();

#ifdef Q_OS_WIN
    NW7TaskBar::instance()->setWindow(m_mainWindow);
    NW7TaskBar::instance()->setEnabled(NSettings::instance()->value("TaskbarProgress").toBool());
    connect(m_playbackEngine, SIGNAL(positionChanged(qreal)), NW7TaskBar::instance(),
            SLOT(setProgress(qreal)));
#endif

#ifdef Q_OS_MAC
    NMacDock::instance()->registerClickHandler();
    connect(NMacDock::instance(), SIGNAL(clicked()), m_mainWindow, SLOT(show()));
#endif

    m_utils = new NUtils(this);

    m_mainWindow->setTitle(QCoreApplication::applicationName() + " " +
                           QCoreApplication::applicationVersion());
    m_mainWindow->show();
    m_mainWindow->loadSettings();
    QResizeEvent e(m_mainWindow->size(), m_mainWindow->size());
    QCoreApplication::sendEvent(m_mainWindow, &e);

    skinProgram.property("afterShow").call(skinProgram);

    connectSignals();

    m_settingsSaveTimer = new QTimer(this);
    connect(m_settingsSaveTimer, &QTimer::timeout, [this]() { saveSettings(); });
    m_settingsSaveTimer->start(5000); // 5 seconds

    m_writeDefaultPlaylistTimer = new QTimer(this);
    m_writeDefaultPlaylistTimer->setSingleShot(true);
    connect(m_writeDefaultPlaylistTimer, &QTimer::timeout,
            [this]() { writePlaylist(NCore::defaultPlaylistPath(), N::NulloyM3u); });

    m_qmlEngine = new QQmlApplicationEngine();
    QQmlContext *context = m_qmlEngine->rootContext();
    context->setContextProperty("playbackEngine", m_playbackEngine);
    context->setContextProperty("player", this);

    m_playlistController = new NPlaylistController(this);
    auto syncItems = [&]() {
        NPlaylistModel *model = m_playlistController->model();
        model->removeAll();
        for (int i = 0; i < m_playlistWidget->count(); ++i) {
            NPlaylistDataItem dataItem = m_playlistWidget->itemAtRow(i)->dataItem();
            NPlaylistModel::DataItem modelItem;
            modelItem.text = dataItem.title;
            modelItem.filePath = dataItem.path;
            model->appenRow(modelItem);
        }
    };
    auto syncCurrentItem = [&]() {
        m_playlistController->model()->setCurrentRow(m_playlistWidget->playingRow());
    };
    syncItems();
    syncCurrentItem();
    connect(m_playlistWidget, &NPlaylistWidget::itemsChanged, syncItems);
    connect(m_playlistController, &NPlaylistController::rowActivated, m_playlistWidget,
            &NPlaylistWidget::playRow);
    connect(m_playlistWidget, &NPlaylistWidget::playingItemChanged, syncCurrentItem);
    qmlRegisterType<NPlaylistModel>("PlaylistModel", 1, 0, "PlaylistModel");
    context->setContextProperty("playlistController", m_playlistController);
    context->setContextProperty("utils", m_utils);

    context->setContextProperty("settings", NSettings::instance());
    context->setContextProperty("skinFileSystem", NSkinFileSystem::instance());
    context->setContextProperty("oldMainWindow", m_mainWindow);
    context->setContextProperty("trackInfoReader", m_trackInfoReader);
    context->setContextProperty("trackInfoModel", m_trackInfoModel);
    qmlRegisterType<NWaveformBar>("NWaveformBar", 1, 0, "NWaveformBar");
    qmlRegisterType<NSvgImage>("NSvgImage", 1, 0, "NSvgImage");
    qmlRegisterType<NImage>("NImage", 1, 0, "NImage");

    m_qmlEngine->load(QUrl::fromLocalFile("src/mainWindow.qml"));

    QObject *qmlMainWindow = m_qmlEngine->rootObjects().first();
    qmlMainWindow->setProperty("width", m_mainWindow->width());
    qmlMainWindow->setProperty("height", m_mainWindow->height());
    qmlMainWindow->setProperty("x", m_mainWindow->x() + m_mainWindow->width() + 20);
    //qmlMainWindow->setProperty("x", m_mainWindow->x());
    qmlMainWindow->setProperty("y", m_mainWindow->y());
    //qobject_cast<QQuickWindow *>(qmlMainWindow->property("window").value<QObject *>())
    //    ->setTextRenderType(QQuickWindow::NativeTextRendering);
    QObject::connect(qmlMainWindow, SIGNAL(closing(QQuickCloseEvent *)), this,
                     SLOT(on_mainWindow_closed()));

    m_coverImage = m_qmlEngine->rootObjects().first()->findChild<NImage *>("coverImage");

    if (NSettings::instance()->value("RestorePlaylist").toBool()) {
        loadDefaultPlaylist();
    }
}

NPlayer::~NPlayer()
{
    NPluginLoader::deinit();
    delete m_mainWindow;
    delete m_settings;
}

void NPlayer::createActions()
{
    m_showHideAction =
        new NAction(QIcon::fromTheme("preferences-system-windows",
                                     QIcon(
                                         ":/trolltech/styles/commonstyle/images/dockdock-16.png")),
                    tr("Show / Hide"), this);
    m_showHideAction->setObjectName("ShowHideAction");
    m_showHideAction->setStatusTip(tr("Toggle window visibility"));
    m_showHideAction->setCustomizable(true);

    m_playAction = new NAction(QIcon::fromTheme("media-playback-start",
                                                style()->standardIcon(QStyle::SP_MediaPlay)),
                               tr("Play / Pause"), this);
    m_playAction->setObjectName("PlayAction");
    m_playAction->setStatusTip(tr("Toggle playback"));
    m_playAction->setCustomizable(true);

    m_stopAction = new NAction(QIcon::fromTheme("media-playback-stop",
                                                style()->standardIcon(QStyle::SP_MediaStop)),
                               tr("Stop"), this);
    m_stopAction->setObjectName("StopAction");
    m_stopAction->setStatusTip(tr("Stop playback"));
    m_stopAction->setCustomizable(true);

    m_prevAction = new NAction(QIcon::fromTheme("media-playback-backward",
                                                style()->standardIcon(QStyle::SP_MediaSkipBackward)),
                               tr("Previous"), this);
    m_prevAction->setObjectName("PrevAction");
    m_prevAction->setStatusTip(tr("Play previous track in playlist"));
    m_prevAction->setCustomizable(true);

    m_nextAction = new NAction(QIcon::fromTheme("media-playback-forward",
                                                style()->standardIcon(QStyle::SP_MediaSkipForward)),
                               tr("Next"), this);
    m_nextAction->setObjectName("NextAction");
    m_nextAction->setStatusTip(tr("Play next track in playlist"));
    m_nextAction->setCustomizable(true);

    QList<QIcon> winIcons;
#ifdef Q_OS_WIN
    winIcons = NWinIcon::getIcons(QProcessEnvironment::systemEnvironment().value("SystemRoot") +
                                  "/system32/imageres.dll");
#endif

    m_preferencesAction = new NAction(QIcon::fromTheme("configure", winIcons.value(109)),
                                      tr("Preferences..."), this);
    m_preferencesAction->setShortcut(QKeySequence("Ctrl+P"));

    m_exitAction = new NAction(QIcon::fromTheme("exit", winIcons.value(259)), tr("Exit"), this);
    m_exitAction->setShortcut(QKeySequence("Ctrl+Q"));

    m_addFilesAction = new NAction(QIcon::fromTheme("add", winIcons.value(171)), tr("Add Files..."),
                                   this);
    m_addFilesAction->setShortcut(QKeySequence("Ctrl+O"));

    m_addDirAction = new NAction(QIcon::fromTheme("folder-add", winIcons.value(3)),
                                 tr("Add Directory..."), this);
    m_addDirAction->setShortcut(QKeySequence("Ctrl+Shift+O"));

    m_savePlaylistAction = new NAction(QIcon::fromTheme("document-save", winIcons.value(175)),
                                       tr("Save Playlist..."), this);
    m_savePlaylistAction->setShortcut(QKeySequence("Ctrl+S"));

    m_showCoverAction = new NAction(tr("Show Cover Art"), this);
    m_showCoverAction->setCheckable(true);
    m_showCoverAction->setObjectName("ShowCoverAction");

    m_showPlaybackControlsAction = new NAction(tr("Show Playback Controls"), this);
    m_showPlaybackControlsAction->setCheckable(true);
    m_showPlaybackControlsAction->setObjectName("ShowPlaybackControls");

    m_aboutAction = new NAction(QIcon::fromTheme("help", winIcons.value(76)), tr("About"), this);

    m_playingOnTopAction = new NAction(tr("On Top During Playback"), this);
    m_playingOnTopAction->setCheckable(true);
    m_playingOnTopAction->setObjectName("PlayingOnTopAction");

    m_alwaysOnTopAction = new NAction(tr("Always On Top"), this);
    m_alwaysOnTopAction->setCheckable(true);
    m_alwaysOnTopAction->setObjectName("AlwaysOnTopAction");

    m_fullScreenAction = new NAction(tr("Fullscreen Mode"), this);
    m_fullScreenAction->setStatusTip(tr("Hide all controls except waveform"));
    m_fullScreenAction->setObjectName("FullScreenAction");
    m_fullScreenAction->setCustomizable(true);

    // playlist actions >>
    m_shufflePlaylistAction = new NAction(tr("Shuffle"), this);
    m_shufflePlaylistAction->setObjectName("ShufflePlaylistAction");
    m_shufflePlaylistAction->setStatusTip(tr("Shuffle items in playlist"));
    m_shufflePlaylistAction->setCustomizable(true);

    m_repeatPlaylistAction = new NAction(tr("Repeat"), this);
    m_repeatPlaylistAction->setCheckable(true);
    m_repeatPlaylistAction->setObjectName("RepeatPlaylistAction");
    m_repeatPlaylistAction->setStatusTip(tr("Toggle current item repeat"));
    m_repeatPlaylistAction->setCustomizable(true);

    m_loopPlaylistAction = new NAction(tr("Loop playlist"), this);
    m_loopPlaylistAction->setCheckable(true);
    m_loopPlaylistAction->setObjectName("LoopPlaylistAction");

    m_scrollToItemPlaylistAction = new NAction(tr("Scroll to playing item"), this);
    m_scrollToItemPlaylistAction->setCheckable(true);
    m_scrollToItemPlaylistAction->setStatusTip(
        tr("Automatically scroll playlist to currently playing item"));
    m_scrollToItemPlaylistAction->setObjectName("ScrollToItemPlaylistAction");

    m_nextFileEnableAction = new NAction(tr("Load next file in directory when finished"), this);
    m_nextFileEnableAction->setCheckable(true);
    m_nextFileEnableAction->setObjectName("NextFileEnableAction");

    m_nextFileByNameAscdAction = new NAction(QString::fromUtf8("    ├  %1 ↓").arg(tr("By Name")),
                                             this);
    m_nextFileByNameAscdAction->setCheckable(true);
    m_nextFileByNameAscdAction->setObjectName("NextFileByNameAscdAction");

    m_nextFileByNameDescAction = new NAction(QString::fromUtf8("    ├  %1 ↑").arg(tr("By Name")),
                                             this);
    m_nextFileByNameDescAction->setCheckable(true);
    m_nextFileByNameDescAction->setObjectName("NextFileByNameDescAction");

    m_nextFileByDateAscd = new NAction(QString::fromUtf8("    ├  %1 ↓").arg(tr("By Date")), this);
    m_nextFileByDateAscd->setCheckable(true);
    m_nextFileByDateAscd->setObjectName("NextFileByDateAscd");

    m_nextFileByDateDesc = new NAction(QString::fromUtf8("    └  %1 ↑").arg(tr("By Date")), this);
    m_nextFileByDateDesc->setCheckable(true);
    m_nextFileByDateDesc->setObjectName("NextFileByDateDesc");

    QActionGroup *group = new QActionGroup(this);
    m_nextFileByNameAscdAction->setActionGroup(group);
    m_nextFileByNameDescAction->setActionGroup(group);
    m_nextFileByDateAscd->setActionGroup(group);
    m_nextFileByDateDesc->setActionGroup(group);
    // << playlist actions

    // jump actions >>
    for (int i = 1; i <= 3; ++i) {
        QString num = QString::number(i);

        NAction *jumpFwAction = new NAction(tr("Jump Forward #%1").arg(num), this);
        jumpFwAction->setObjectName(QString("Jump%1ForwardAction").arg(num));
        jumpFwAction->setStatusTip(tr("Make a jump forward #%1").arg(num));
        jumpFwAction->setCustomizable(true);

        NAction *jumpBwAction = new NAction(tr("Jump Backwards #%1").arg(num), this);
        jumpBwAction->setObjectName(QString("Jump%1BackwardsAction").arg(num));
        jumpBwAction->setStatusTip(tr("Make a jump backwards #%1").arg(num));
        jumpBwAction->setCustomizable(true);
    }
    // << jump actions

    // speed actions >>
    m_speedIncreaseAction = new NAction(tr("Speed Increase"), this);
    m_speedIncreaseAction->setObjectName("SpeedIncreaseAction");
    m_speedIncreaseAction->setStatusTip(tr("Increase playback speed"));
    m_speedIncreaseAction->setCustomizable(true);

    m_speedDecreaseAction = new NAction(tr("Speed Decrease"), this);
    m_speedDecreaseAction->setObjectName("SpeedDecreaseAction");
    m_speedDecreaseAction->setStatusTip(tr("Decrease playback speed"));
    m_speedDecreaseAction->setCustomizable(true);

    m_speedResetAction = new NAction(tr("Speed Reset"), this);
    m_speedResetAction->setObjectName("SpeedResetAction");
    m_speedResetAction->setStatusTip(tr("Reset playback speed to 1.0"));
    m_speedResetAction->setCustomizable(true);
    // << speed actions

    // pitch actions >>
    /*
    m_pitchIncreaseAction = new NAction(tr("Pitch Increase"), this);
    m_pitchIncreaseAction->setObjectName("PitchIncreaseAction");
    m_pitchIncreaseAction->setStatusTip(tr("Increase playback pitch"));
    m_pitchIncreaseAction->setCustomizable(true);

    m_pitchDecreaseAction = new NAction(tr("Pitch Decrease"), this);
    m_pitchDecreaseAction->setObjectName("PitchDecreaseAction");
    m_pitchDecreaseAction->setStatusTip(tr("Decrease playback pitch"));
    m_pitchDecreaseAction->setCustomizable(true);

    m_pitchResetAction = new NAction(tr("Pitch Reset"), this);
    m_pitchResetAction->setObjectName("PitchResetAction");
    m_pitchResetAction->setStatusTip(tr("Reset pitch to 1.0"));
    m_pitchResetAction->setCustomizable(true);
    */
    // << pitch actions

    // keyboard shortcuts
    m_settings->initShortcuts(this);
    foreach (NAction *action, findChildren<NAction *>()) {
        if (!action->shortcuts().isEmpty()) {
            m_mainWindow->addAction(action);
        }
    }

    createContextMenu();
    createGlobalMenu();
    createTrayIcon();
}

NMainWindow *NPlayer::mainWindow()
{
    return m_mainWindow;
}

void NPlayer::createContextMenu()
{
    m_contextMenu = new QMenu(m_mainWindow);
    m_mainWindow->setContextMenuPolicy(Qt::CustomContextMenu);
    m_contextMenu->addAction(m_addFilesAction);
    m_contextMenu->addAction(m_addDirAction);
    m_contextMenu->addAction(m_savePlaylistAction);

    m_windowSubMenu = new QMenu(tr("Window"), m_mainWindow);
    m_windowSubMenu->addAction(m_showCoverAction);
    m_windowSubMenu->addAction(m_showPlaybackControlsAction);
    m_windowSubMenu->addAction(m_playingOnTopAction);
    m_windowSubMenu->addAction(m_alwaysOnTopAction);
    m_windowSubMenu->addAction(m_fullScreenAction);
    m_contextMenu->addMenu(m_windowSubMenu);

    m_playlistSubMenu = new QMenu(tr("Playlist"), m_mainWindow);
    m_playlistSubMenu->addAction(m_shufflePlaylistAction);
    m_playlistSubMenu->addAction(m_repeatPlaylistAction);
    m_playlistSubMenu->addAction(m_loopPlaylistAction);
    m_playlistSubMenu->addAction(m_scrollToItemPlaylistAction);
    m_playlistSubMenu->addAction(m_nextFileEnableAction);
    m_playlistSubMenu->addAction(m_nextFileByNameAscdAction);
    m_playlistSubMenu->addAction(m_nextFileByNameDescAction);
    m_playlistSubMenu->addAction(m_nextFileByDateAscd);
    m_playlistSubMenu->addAction(m_nextFileByDateDesc);
    m_contextMenu->addMenu(m_playlistSubMenu);

    m_contextMenu->addAction(m_preferencesAction);
    m_contextMenu->addSeparator();
    m_contextMenu->addAction(m_aboutAction);
    m_contextMenu->addSeparator();
    m_contextMenu->addAction(m_exitAction);
}

void NPlayer::createGlobalMenu()
{
#ifdef Q_OS_MAC
    // removing icons from context menu
    QList<NAction *> actions = findChildren<NAction *>();
    for (int i = 0; i < actions.size(); ++i)
        actions.at(i)->setIcon(QIcon());

    QMenuBar *menuBar = new QMenuBar(m_mainWindow);

    QMenu *fileMenu = menuBar->addMenu(tr("File"));
    fileMenu->addAction(m_addFilesAction);
    fileMenu->addAction(m_addDirAction);
    fileMenu->addAction(m_savePlaylistAction);
    fileMenu->addAction(m_aboutAction);
    fileMenu->addAction(m_exitAction);
    fileMenu->addAction(m_preferencesAction);

    QMenu *controlsMenu = menuBar->addMenu(tr("Controls"));
    controlsMenu->addAction(m_playAction);
    controlsMenu->addAction(m_stopAction);
    controlsMenu->addAction(m_prevAction);
    controlsMenu->addAction(m_nextAction);
    controlsMenu->addSeparator();

    QMenu *playlistSubMenu = controlsMenu->addMenu(tr("Playlist"));
    playlistSubMenu->addAction(m_shufflePlaylistAction);
    playlistSubMenu->addAction(m_repeatPlaylistAction);
    playlistSubMenu->addAction(m_loopPlaylistAction);
    playlistSubMenu->addAction(m_scrollToItemPlaylistAction);
    playlistSubMenu->addAction(m_nextFileEnableAction);
    playlistSubMenu->addAction(m_nextFileByNameAscdAction);
    playlistSubMenu->addAction(m_nextFileByNameDescAction);
    playlistSubMenu->addAction(m_nextFileByDateAscd);
    playlistSubMenu->addAction(m_nextFileByDateDesc);
    controlsMenu->addMenu(playlistSubMenu);

    QMenu *windowMenu = menuBar->addMenu(tr("Window"));
    windowMenu->addAction(m_showCoverAction);
    windowMenu->addAction(m_showPlaybackControlsAction);
    windowMenu->addAction(m_playingOnTopAction);
    windowMenu->addAction(m_alwaysOnTopAction);
    windowMenu->addAction(m_fullScreenAction);
#endif
}

void NPlayer::createTrayIcon()
{
    QMenu *trayIconMenu = new QMenu(this);
    trayIconMenu->addAction(m_showHideAction);
    trayIconMenu->addSeparator();
    trayIconMenu->addAction(m_playAction);
    trayIconMenu->addAction(m_stopAction);
    trayIconMenu->addAction(m_prevAction);
    trayIconMenu->addAction(m_nextAction);
    trayIconMenu->addSeparator();
    trayIconMenu->addAction(m_preferencesAction);
    trayIconMenu->addAction(m_exitAction);
    m_systemTray = new QSystemTrayIcon(this);
    m_systemTray->setContextMenu(trayIconMenu);
    m_systemTray->setIcon(m_mainWindow->windowIcon());
    m_trayClickTimer = new QTimer(this);
    m_trayClickTimer->setSingleShot(true);
}

void NPlayer::connectSignals()
{
    connect(m_playbackEngine, SIGNAL(mediaChanged(const QString &, int)), this,
            SLOT(on_playbackEngine_mediaChanged(const QString &, int)));
    connect(m_playbackEngine, SIGNAL(mediaFailed(const QString &, int)), this,
            SLOT(on_playbackEngine_mediaFailed(const QString &, int)));
    connect(m_playbackEngine, SIGNAL(stateChanged(N::PlaybackState)), this,
            SLOT(on_playbackEngine_stateChanged(N::PlaybackState)));
    connect(m_playbackEngine, SIGNAL(positionChanged(qreal)), m_waveformSlider,
            SLOT(setValue(qreal)));
    connect(m_playbackEngine, SIGNAL(tick(qint64)), m_trackInfoWidget,
            SLOT(updatePlaybackLabels(qint64)));
    connect(m_playbackEngine, SIGNAL(tick(qint64)), m_trackInfoModel, SLOT(updatePlayback(qint64)));
    connect(m_playbackEngine, SIGNAL(message(N::MessageIcon, const QString &, const QString &)),
            m_logDialog, SLOT(showMessage(N::MessageIcon, const QString &, const QString &)));

    connect(m_mainWindow, SIGNAL(closed()), this, SLOT(on_mainWindow_closed()));

    connect(m_preferencesDialogHandler, SIGNAL(settingsApplied()), this,
            SLOT(on_preferencesDialog_settingsChanged()));

    if (QAbstractButton *playButton = m_mainWindow->findChild<QAbstractButton *>("playButton")) {
        connect(playButton, SIGNAL(clicked()), this, SLOT(on_playButton_clicked()));
    }

    if (QAbstractButton *stopButton = m_mainWindow->findChild<QAbstractButton *>("stopButton")) {
        connect(stopButton, SIGNAL(clicked()), m_playbackEngine, SLOT(stop()));
    }

    if (QAbstractButton *prevButton = m_mainWindow->findChild<QAbstractButton *>("prevButton")) {
        connect(prevButton, SIGNAL(clicked()), m_playlistWidget, SLOT(playPrevItem()));
    }

    if (QAbstractButton *nextButton = m_mainWindow->findChild<QAbstractButton *>("nextButton")) {
        connect(nextButton, SIGNAL(clicked()), m_playlistWidget, SLOT(playNextItem()));
    }

    if (QAbstractButton *closeButton = m_mainWindow->findChild<QAbstractButton *>("closeButton")) {
        connect(closeButton, SIGNAL(clicked()), m_mainWindow, SLOT(close()));
    }

    if (QAbstractButton *minimizeButton = m_mainWindow->findChild<QAbstractButton *>(
            "minimizeButton")) {
        connect(minimizeButton, SIGNAL(clicked()), m_mainWindow, SLOT(showMinimized()));
    }

    if (m_volumeSlider) {
        connect(m_volumeSlider, SIGNAL(sliderMoved(qreal)), m_playbackEngine,
                SLOT(setVolume(qreal)));
        connect(m_playbackEngine, SIGNAL(volumeChanged(qreal)), m_volumeSlider,
                SLOT(setValue(qreal)));
        connect(m_mainWindow, SIGNAL(scrolled(int)), this, SLOT(on_mainWindow_scrolled(int)));
    }

    if (QAbstractButton *repeatButton = m_mainWindow->findChild<QAbstractButton *>(
            "repeatButton")) {
        connect(repeatButton, SIGNAL(clicked(bool)), m_playlistWidget, SLOT(setRepeatMode(bool)));
        connect(m_playlistWidget, SIGNAL(repeatModeChanged(bool)), repeatButton,
                SLOT(setChecked(bool)));
    }

    if (QAbstractButton *shuffleButton = m_mainWindow->findChild<QAbstractButton *>(
            "shuffleButton")) {
        connect(shuffleButton, SIGNAL(clicked()), m_playlistWidget, SLOT(shufflePlaylist()));
    }

    connect(m_playlistWidget, SIGNAL(repeatModeChanged(bool)), m_repeatPlaylistAction,
            SLOT(setChecked(bool)));
    connect(m_playlistWidget, SIGNAL(tagEditorRequested(const QString &)), this,
            SLOT(on_playlist_tagEditorRequested(const QString &)));
    connect(m_playlistWidget, SIGNAL(addMoreRequested()), this,
            SLOT(on_playlist_addMoreRequested()));
    connect(m_playlistWidget, &NPlaylistWidget::durationChanged, [this](int durationSec) {
        NPlaylistWidgetItem *item = m_playlistWidget->playingItem();
        if (item) {
            m_trackInfoReader->setSource(item->data(N::PathRole).toString());
        }

        m_trackInfoReader->updatePlaylistDuration(durationSec);

        m_trackInfoWidget->updatePlaylistLabels();
        m_trackInfoModel->updatePlaylistLabels();

        QString format = NSettings::instance()->value("WindowTitleTrackInfo").toString();
        if (!format.isEmpty()) {
            QString title = m_trackInfoReader->toString(format);
            m_mainWindow->setTitle(title);
        }
    });
    connect(m_playlistWidget, &NPlaylistWidget::itemsChanged, [this]() {
        m_writeDefaultPlaylistTimer->start(100); //
    });
    connect(m_playlistWidget, &NPlaylistWidget::playingItemChanged,
            [this]() { savePlaybackState(); });
    connect(m_playlistWidget, &NPlaylistWidget::playlistFinished, [this]() {
        if (NSettings::instance()->value("QuitWhenFinished").toBool()) {
            QCoreApplication::quit();
        }
    });

    connect(m_waveformSlider, &NWaveformSlider::filesDropped,
            [this](const QList<NPlaylistDataItem> &dataItems) {
                m_playlistWidget->setItems(dataItems);
                m_playlistWidget->playRow(0);
            });
    connect(m_waveformSlider, SIGNAL(sliderMoved(qreal)), m_playbackEngine,
            SLOT(setPosition(qreal)));

    connect(m_showHideAction, SIGNAL(triggered()), this, SLOT(toggleWindowVisibility()));
    connect(m_playAction, &NAction::triggered, [this]() {
        if (m_playbackEngine->state() != N::PlaybackPlaying) {
            m_playbackEngine->play();
        } else {
            m_playbackEngine->pause();
        }
    });
    connect(m_stopAction, SIGNAL(triggered()), m_playbackEngine, SLOT(stop()));
    connect(m_prevAction, SIGNAL(triggered()), m_playlistWidget, SLOT(playPrevItem()));
    connect(m_nextAction, SIGNAL(triggered()), m_playlistWidget, SLOT(playNextItem()));
    connect(m_preferencesAction, SIGNAL(triggered()), m_preferencesDialogHandler,
            SLOT(showDialog()));
    connect(m_exitAction, SIGNAL(triggered()), QCoreApplication::instance(), SLOT(quit()));
    connect(m_addFilesAction, SIGNAL(triggered()), this, SLOT(showOpenFileDialog()));
    connect(m_addDirAction, SIGNAL(triggered()), this, SLOT(showOpenDirDialog()));
    connect(m_savePlaylistAction, SIGNAL(triggered()), this, SLOT(showSavePlaylistDialog()));
    connect(m_showCoverAction, SIGNAL(toggled(bool)), this, SLOT(on_showCoverAction_toggled(bool)));
    connect(m_showPlaybackControlsAction, SIGNAL(toggled(bool)), m_mainWindow,
            SLOT(showPlaybackControls(bool)));
    connect(m_aboutAction, SIGNAL(triggered()), this, SLOT(showAboutDialog()));
    connect(m_playingOnTopAction, SIGNAL(toggled(bool)), this,
            SLOT(on_whilePlayingOnTopAction_toggled(bool)));
    connect(m_alwaysOnTopAction, SIGNAL(toggled(bool)), this,
            SLOT(on_alwaysOnTopAction_toggled(bool)));
    connect(m_fullScreenAction, SIGNAL(triggered()), m_mainWindow, SLOT(toggleFullScreen()));
    connect(m_shufflePlaylistAction, SIGNAL(triggered()), m_playlistWidget, SLOT(shufflePlaylist()));
    connect(m_repeatPlaylistAction, SIGNAL(triggered(bool)), m_playlistWidget,
            SLOT(setRepeatMode(bool)));
    connect(m_loopPlaylistAction, SIGNAL(triggered(bool)), this,
            SLOT(on_playlistAction_triggered()));
    connect(m_scrollToItemPlaylistAction, SIGNAL(triggered(bool)), this,
            SLOT(on_playlistAction_triggered()));
    connect(m_nextFileEnableAction, SIGNAL(triggered()), this, SLOT(on_playlistAction_triggered()));
    connect(m_nextFileByNameAscdAction, SIGNAL(triggered()), this,
            SLOT(on_playlistAction_triggered()));
    connect(m_nextFileByNameDescAction, SIGNAL(triggered()), this,
            SLOT(on_playlistAction_triggered()));
    connect(m_nextFileByDateAscd, SIGNAL(triggered()), this, SLOT(on_playlistAction_triggered()));
    connect(m_nextFileByDateDesc, SIGNAL(triggered()), this, SLOT(on_playlistAction_triggered()));

    foreach (NAction *action, findChildren<NAction *>()) {
        if (action->objectName().startsWith("Jump")) {
            connect(action, SIGNAL(triggered()), this, SLOT(on_jumpAction_triggered()));
        }
    }

    connect(m_speedIncreaseAction, SIGNAL(triggered()), this,
            SLOT(on_speedIncreaseAction_triggered()));
    connect(m_speedDecreaseAction, SIGNAL(triggered()), this,
            SLOT(on_speedDecreaseAction_triggered()));
    connect(m_speedResetAction, SIGNAL(triggered()), this, SLOT(on_speedResetAction_triggered()));

    /*
    connect(m_pitchIncreaseAction, SIGNAL(triggered()), this,
            SLOT(on_pitchIncreaseAction_triggered()));
    connect(m_pitchDecreaseAction, SIGNAL(triggered()), this,
            SLOT(on_pitchDecreaseAction_triggered()));
    connect(m_pitchResetAction, SIGNAL(triggered()), this, SLOT(on_pitchResetAction_triggered()));
    */

    connect(m_mainWindow, SIGNAL(customContextMenuRequested(const QPoint &)), this,
            SLOT(showContextMenu(const QPoint &)));
    connect(m_systemTray, SIGNAL(activated(QSystemTrayIcon::ActivationReason)), this,
            SLOT(on_trayIcon_activated(QSystemTrayIcon::ActivationReason)));
    connect(m_trayClickTimer, SIGNAL(timeout()), this, SLOT(on_trayClickTimer_timeout()));
}

NPlaybackEngineInterface *NPlayer::playbackEngine()
{
    return m_playbackEngine;
}

QString NPlayer::volumeTooltipText(qreal value) const
{
    if (NSettings::instance()->value("ShowDecibelsVolume").toBool()) {
        qreal decibel = 0.67 * log(value) / kLog10over20;
        QString decibelStr;
        decibelStr.setNum(decibel, 'g', 2);
        return QString("%1 %2 dB").arg(tr("Volume")).arg(decibelStr);
    } else {
        return QString("%1 %2\%").arg(tr("Volume")).arg(QString::number(int(value * 100)));
    }
}

void NPlayer::setOverrideCursor(Qt::CursorShape shape)
{
    QCursor cursor(shape);
    QGuiApplication::setOverrideCursor(cursor);
}

void NPlayer::restoreOverrideCursor()
{
    QGuiApplication::restoreOverrideCursor();
}

bool NPlayer::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::FileOpen) {
        QFileOpenEvent *fileEvent = static_cast<QFileOpenEvent *>(event);

        if (!fileEvent->file().isEmpty()) {
            m_playlistWidget->setFiles(QStringList() << fileEvent->file());
            m_playlistWidget->playRow(0);
        }

        return false;
    }

    return QObject::eventFilter(obj, event);
}

void NPlayer::readMessage(const QString &str)
{
    if (str.isEmpty()) {
        m_mainWindow->show();
        m_mainWindow->activateWindow();
        m_mainWindow->raise();
        return;
    }
    QStringList argList = str.split(MSG_SPLITTER);
    QStringList files;
    QStringList options;
    foreach (QString arg, argList) {
        if (arg.startsWith("--")) {
            options << arg;
        } else if (QFile(arg).exists()) {
            files << arg;
        }
    }

    foreach (QString arg, options) {
        if (arg == "--next") {
            m_playlistWidget->playNextItem();
            return;
        } else if (arg == "--prev") {
            m_playlistWidget->playPrevItem();
            return;
        } else if (arg == "--stop") {
            m_playbackEngine->stop();
            return;
        } else if (arg == "--pause") {
            m_playbackEngine->play();
            return;
        }
    }

    if (!files.isEmpty()) {
        if (NSettings::instance()->value("EnqueueFiles").toBool()) {
            int lastRow = m_playlistWidget->count();
            m_playlistWidget->addFiles(files);
            if (m_playbackEngine->state() == N::PlaybackStopped ||
                NSettings::instance()->value("PlayEnqueued").toBool()) {
                m_playlistWidget->playRow(lastRow);
                m_playbackEngine->setPosition(0); // overrides setPosition() in loadDefaultPlaylist()
            }
        } else {
            m_playlistWidget->setFiles(files);
            m_playlistWidget->playRow(0);
        }
    }
}

void NPlayer::loadDefaultPlaylist()
{
    if (!QFileInfo(NCore::defaultPlaylistPath()).exists() ||
        !m_playlistWidget->setPlaylist(NCore::defaultPlaylistPath())) {
        return;
    }

    QStringList playlistRowValues = m_settings->value("PlaylistRow").toStringList();
    if (!playlistRowValues.isEmpty()) {
        int row = playlistRowValues.at(0).toInt();
        qreal pos = playlistRowValues.at(1).toFloat();

        if (row < 0 || row > m_playlistWidget->count() - 1) {
            return;
        }

        if (!m_settings->value("StartPaused").toBool()) {
            m_playlistWidget->playRow(row);
            m_playbackEngine->setPosition(pos);
        } else { // do the work that would have been done upon m_playbackEngine::mediaChanged()
            NPlaylistWidgetItem *item = m_playlistWidget->itemAtRow(row);
            m_playlistWidget->setPlayingItem(item);

            QString file = item->data(N::PathRole).toString();
            int id = item->data(N::IdRole).toInt();

            m_playbackEngine->setMedia(file, id);
            m_playbackEngine->setPosition(pos);
            m_playbackEngine->positionChanged(pos);

            loadCoverArt(file);

            m_waveformSlider->setMedia(file);
            m_waveformSlider->setValue(pos);
            m_waveformSlider->setPausedState(true);
            m_trackInfoWidget->updateFileLabels(file);
            m_trackInfoModel->updateFileLabels(file);
        }
    }
}

void NPlayer::writePlaylist(const QString &file, N::M3uExtention ext)
{
    QList<NPlaylistDataItem> dataItemsList;
    for (int i = 0; i < m_playlistWidget->count(); ++i) {
        NPlaylistDataItem dataItem = m_playlistWidget->itemAtRow(i)->dataItem();
        dataItemsList << dataItem;
    }
    NPlaylistStorage::writeM3u(file, dataItemsList, ext);
}

void NPlayer::loadSettings()
{
    m_systemTray->setVisible(m_settings->value("TrayIcon").toBool());

#ifndef _N_NO_UPDATE_CHECK_
    if (m_settings->value("AutoCheckUpdates").toBool()) {
        downloadVersion();
    }
#endif

    bool showCoverArt = m_settings->value("ShowCoverArt").toBool();
    m_showCoverAction->setChecked(showCoverArt);

    m_showPlaybackControlsAction->setChecked(m_settings->value("ShowPlaybackControls").toBool());
    m_mainWindow->showPlaybackControls(m_settings->value("ShowPlaybackControls").toBool());

    m_alwaysOnTopAction->setChecked(m_settings->value("AlwaysOnTop").toBool());
    m_playingOnTopAction->setChecked(m_settings->value("WhilePlayingOnTop").toBool());
    m_loopPlaylistAction->setChecked(m_settings->value("LoopPlaylist").toBool());
    m_scrollToItemPlaylistAction->setChecked(m_settings->value("ScrollToItem").toBool());
    m_nextFileEnableAction->setChecked(m_settings->value("LoadNext").toBool());
    m_repeatPlaylistAction->setChecked(NSettings::instance()->value("Repeat").toBool());

    QDir::SortFlag flag = (QDir::SortFlag)m_settings->value("LoadNextSort").toInt();
    if (flag == (QDir::Name)) {
        m_nextFileByNameAscdAction->setChecked(true);
    } else if (flag == (QDir::Name | QDir::Reversed)) {
        m_nextFileByNameDescAction->setChecked(true);
    } else if (flag == (QDir::Time | QDir::Reversed)) {
        m_nextFileByDateAscd->setChecked(true);
    } else if (flag == (QDir::Time)) {
        m_nextFileByDateDesc->setChecked(true);
    } else {
        m_nextFileByNameAscdAction->setChecked(true);
    }

    m_playbackEngine->setVolume(m_settings->value("Volume").toFloat());
    m_volumeSlider->setValue(m_settings->value("Volume").toFloat());
}

void NPlayer::saveSettings()
{
    m_settings->setValue("Volume", QString::number(m_playbackEngine->volume()));
    m_mainWindow->saveSettings();
    savePlaybackState();
}

void NPlayer::savePlaybackState()
{
    int row = m_playlistWidget->playingRow();
    qreal pos = m_playbackEngine->position();
    m_settings->setValue("PlaylistRow", QStringList()
                                            << QString::number(row) << QString::number(pos));
}

void NPlayer::on_preferencesDialog_settingsChanged()
{
    m_systemTray->setVisible(m_settings->value("TrayIcon").toBool());
#ifdef Q_OS_WIN
    NW7TaskBar::instance()->setEnabled(NSettings::instance()->value("TaskbarProgress").toBool());
#endif
    m_trackInfoWidget->loadSettings();
    m_trackInfoWidget->updateFileLabels(m_playbackEngine->currentMedia());
    m_trackInfoModel->loadSettings();
    m_trackInfoModel->updateFileLabels(m_playbackEngine->currentMedia());
    m_playlistWidget->processVisibleItems();
}

#ifndef _N_NO_UPDATE_CHECK_
void NPlayer::downloadVersion()
{
    m_versionDownloader->get(QNetworkRequest(
        QUrl("https://static." + QCoreApplication::organizationDomain() + "/release/version")));
}

void NPlayer::on_versionDownloader_finished(QNetworkReply *reply)
{
    if (!reply->error()) {
        QString versionOnline = reply->readAll().simplified();

        m_preferencesDialogHandler->setVersionLabel(versionOnline);

        if (NSettings::instance()->value("UpdateIgnore", "").toString() == versionOnline) {
            return;
        }

        if (QCoreApplication::applicationVersion() < versionOnline) {
            QMessageBox::information(m_mainWindow, tr("Update"),
                                     QCoreApplication::applicationName() + " " + versionOnline +
                                         " " + tr("released!") + "<br><br>" + "<a href='https://" +
                                         QCoreApplication::organizationDomain() +
                                         "/download'>https://" +
                                         QCoreApplication::organizationDomain() + "/download</a>",
                                     QMessageBox::Ignore);
        }

        NSettings::instance()->setValue("UpdateIgnore", versionOnline);
    }

    reply->deleteLater();
}
#endif

void NPlayer::on_mainWindow_closed()
{
    if (m_settings->value("MinimizeToTray").toBool()) {
        m_systemTray->setVisible(true);
    } else {
#ifdef Q_OS_MAC
        if (m_settings->value("QuitOnClose").toBool()) {
            QCoreApplication::quit();
        }
#else
        QCoreApplication::quit();
#endif
    }
}

void NPlayer::on_trayClickTimer_timeout()
{
    if (!m_trayIconDoubleClickCheck) {
        trayIconCountClicks(1);
    }
}

void NPlayer::on_trayIcon_activated(QSystemTrayIcon::ActivationReason reason)
{
    if (reason == QSystemTrayIcon::Trigger) { // single click
        m_trayIconDoubleClickCheck = false;
        m_trayClickTimer->start(QApplication::doubleClickInterval());
    } else if (reason == QSystemTrayIcon::DoubleClick) {
        m_trayIconDoubleClickCheck = true;
        trayIconCountClicks(2);
    }
}

void NPlayer::toggleWindowVisibility()
{
    if (!m_mainWindow->isVisible() || m_mainWindow->isMinimized()) {
        m_mainWindow->show();
        m_mainWindow->activateWindow();
        m_mainWindow->raise();

    } else if (m_settings->value("MinimizeToTray").toBool()) {
        m_mainWindow->setVisible(false);
        m_systemTray->setVisible(true);
    } else {
        m_mainWindow->showMinimized();
    }
}

void NPlayer::trayIconCountClicks(int clicks)
{
    if (clicks == 1) {
        m_mainWindow->show();
        m_mainWindow->activateWindow();
        m_mainWindow->raise();
    } else if (clicks == 2) {
        toggleWindowVisibility();
    }
    if (!m_settings->value("TrayIcon").toBool()) {
        m_systemTray->setVisible(!m_mainWindow->isVisible());
    }
}

void NPlayer::quit()
{
    delete m_qmlEngine;
    saveSettings();
}

void NPlayer::on_playbackEngine_mediaChanged(const QString &file, int)
{
    QString title;
    QString format = NSettings::instance()->value("WindowTitleTrackInfo").toString();
    if (!format.isEmpty()) {
        m_trackInfoReader->setSource(file);
        title = m_trackInfoReader->toString(format);
    }
    m_mainWindow->setTitle(title);
    m_systemTray->setToolTip(title);

    m_waveformSlider->setMedia(file);
    m_trackInfoWidget->updateFileLabels(file);
    m_trackInfoModel->updateFileLabels(file);
    loadCoverArt(file);
}

void NPlayer::loadCoverArt(const QString &file)
{
    if (!m_settings->value("ShowCoverArt").toBool()) {
        return;
    }
    QImage image;
    if (m_coverReader) {
        m_coverReader->setSource(file);
        QList<QImage> images = m_coverReader->getImages();

        if (!images.isEmpty()) {
            image = images.first();
        }
    }

    if (image.isNull()) {
        QFileInfo fileInfo(file);
        QDir dir = fileInfo.absoluteDir();
        QStringList images = dir.entryList(QStringList() << "*.jpg"
                                                         << "*.jpeg"
                                                         << "*.png",
                                           QDir::Files);

        // search for image which file name starts same as source file:
        QString baseName = fileInfo.completeBaseName();
        QString imageFile;
        foreach (QString image, images) {
            if (baseName.startsWith(QFileInfo(image).completeBaseName())) {
                imageFile = dir.absolutePath() + "/" + image;
                break;
            }
        }

        // search for cover.* or folder.* or front.*:
        if (imageFile.isEmpty()) {
            QStringList matchedImages = images.filter(
                QRegExp("^(cover|folder|front)\\..*$", Qt::CaseInsensitive));
            if (!matchedImages.isEmpty()) {
                imageFile = dir.absolutePath() + "/" + matchedImages.first();
            }
        }
        if (!imageFile.isEmpty()) {
            image = QImage(imageFile);
        }
    }

    if (image.isNull()) {
        m_coverWidget->hide();
        if (m_coverImage) {
            m_coverImage->setVisible(false);
        }
    } else {
        m_coverWidget->show();
        m_coverWidget->setPixmap(QPixmap::fromImage(image));
        if (m_coverImage) {
            m_coverImage->setVisible(true);
            m_coverImage->setImage(image);
        }
    }
}

void NPlayer::on_playbackEngine_mediaFailed(const QString &, int)
{
    on_playbackEngine_mediaChanged("", 0);
}

void NPlayer::on_playbackEngine_stateChanged(N::PlaybackState state)
{
    bool whilePlaying = m_settings->value("WhilePlayingOnTop").toBool();
    bool alwaysOnTop = m_settings->value("AlwaysOnTop").toBool();
    bool oldOnTop = m_mainWindow->isOnTop();
    bool newOnTop = (whilePlaying && state == N::PlaybackPlaying);
    if (!alwaysOnTop && (oldOnTop != newOnTop)) {
        m_mainWindow->setOnTop(newOnTop);
    }
#ifdef Q_OS_WIN
    if (NW7TaskBar::instance()->isEnabled()) {
        if (state == N::PlaybackPlaying) {
            NW7TaskBar::instance()->setState(NW7TaskBar::Normal);
        } else {
            if (m_playbackEngine->position() != 0)
                NW7TaskBar::instance()->setState(NW7TaskBar::Paused);
            else
                NW7TaskBar::instance()->setState(NW7TaskBar::NoProgress);
        }
    }
#endif

    m_waveformSlider->setPausedState(state == N::PlaybackPaused);
}

void NPlayer::on_alwaysOnTopAction_toggled(bool checked)
{
    m_settings->setValue("AlwaysOnTop", checked);

    bool whilePlaying = m_settings->value("WhilePlayingOnTop").toBool();
    if (!whilePlaying || m_playbackEngine->state() != N::PlaybackPlaying) {
        m_mainWindow->setOnTop(checked);
    }
}

void NPlayer::on_whilePlayingOnTopAction_toggled(bool checked)
{
    m_settings->setValue("WhilePlayingOnTop", checked);

    bool alwaysOnTop = m_settings->value("AlwaysOnTop").toBool();
    if (!alwaysOnTop) {
        m_mainWindow->setOnTop(checked && m_playbackEngine->state() == N::PlaybackPlaying);
    }
}

void NPlayer::on_playlistAction_triggered()
{
    NAction *action = reinterpret_cast<NAction *>(QObject::sender());
    if (action == m_loopPlaylistAction) {
        m_settings->setValue("LoopPlaylist", action->isChecked());
    } else if (action == m_scrollToItemPlaylistAction) {
        m_settings->setValue("ScrollToItem", action->isChecked());
    } else if (action == m_nextFileEnableAction) {
        m_settings->setValue("LoadNext", action->isChecked());
    } else if (action == m_nextFileByNameAscdAction) {
        m_settings->setValue("LoadNextSort", (int)QDir::Name);
    } else if (action == m_nextFileByNameDescAction) {
        m_settings->setValue("LoadNextSort", (int)(QDir::Name | QDir::Reversed));
    } else if (action == m_nextFileByDateAscd) {
        m_settings->setValue("LoadNextSort", (int)QDir::Time | QDir::Reversed);
    } else if (action == m_nextFileByDateDesc) {
        m_settings->setValue("LoadNextSort", (int)(QDir::Time));
    }
}

void NPlayer::on_playlist_tagEditorRequested(const QString &path)
{
    NTagEditorDialog(path, m_mainWindow);
}

void NPlayer::on_playlist_addMoreRequested()
{
    if (!NSettings::instance()->value("LoadNext").toBool()) {
        return;
    }
    QDir::SortFlag flag = (QDir::SortFlag)NSettings::instance()->value("LoadNextSort").toInt();
    QString file = m_playlistWidget->playingItem()->data(N::PathRole).toString();
    QString path = QFileInfo(file).path();
    QStringList entryList =
        QDir(path).entryList(NSettings::instance()->value("FileFilters").toString().split(' '),
                             QDir::Files | QDir::NoDotAndDotDot, flag);
    int index = entryList.indexOf(QFileInfo(file).fileName());
    if (index != -1 && entryList.size() > index + 1) {
        m_playlistWidget->addFiles(QStringList() << path + "/" + entryList.at(index + 1));
    }
}

void NPlayer::on_jumpAction_triggered()
{
    NAction *action = reinterpret_cast<NAction *>(QObject::sender());
    QRegExp regex("(\\w+\\d)(\\w+)Action");
    regex.indexIn(action->objectName());
    m_playbackEngine->jump((regex.cap(2) == "Forward" ? 1 : -1) *
                           NSettings::instance()->value(regex.cap(1)).toDouble() * 1000);
}

void NPlayer::on_speedIncreaseAction_triggered()
{
    qreal newSpeed = qMax(0.01, m_playbackEngine->speed() +
                                    NSettings::instance()->value("SpeedStep").toDouble());
    m_playbackEngine->setSpeed(newSpeed);
    showToolTip(tr("Speed: %1").arg(m_playbackEngine->speed()));
}

void NPlayer::on_speedDecreaseAction_triggered()
{
    qreal newSpeed = qMax(0.01, m_playbackEngine->speed() -
                                    NSettings::instance()->value("SpeedStep").toDouble());
    m_playbackEngine->setSpeed(newSpeed);
    showToolTip(tr("Speed: %1").arg(m_playbackEngine->speed()));
}

void NPlayer::on_speedResetAction_triggered()
{
    qreal newSpeed = 1.0;
    m_playbackEngine->setSpeed(newSpeed);
    showToolTip(tr("Speed: %1").arg(m_playbackEngine->speed()));
}

void NPlayer::on_pitchIncreaseAction_triggered()
{
    qreal newPitch = qMax(0.01, m_playbackEngine->pitch() +
                                    NSettings::instance()->value("PitchStep").toDouble());
    m_playbackEngine->setPitch(newPitch);
    showToolTip(tr("Pitch: %1").arg(m_playbackEngine->pitch()));
}

void NPlayer::on_pitchDecreaseAction_triggered()
{
    qreal newPitch = qMax(0.01, m_playbackEngine->pitch() -
                                    NSettings::instance()->value("PitchStep").toDouble());
    m_playbackEngine->setPitch(newPitch);
    showToolTip(tr("Pitch: %1").arg(m_playbackEngine->pitch()));
}

void NPlayer::on_pitchResetAction_triggered()
{
    qreal newPitch = 1.0;
    m_playbackEngine->setPitch(newPitch);
    showToolTip(tr("Pitch: %1").arg(m_playbackEngine->pitch()));
}

void NPlayer::on_showCoverAction_toggled(bool checked)
{
    m_settings->setValue("ShowCoverArt", checked);
    if (m_coverWidget) {
        m_coverWidget->setVisible(checked);
    }
}

void NPlayer::on_playButton_clicked()
{
    if (!m_playlistWidget->hasPlayingItem()) {
        if (m_playlistWidget->count() > 0) {
            m_playlistWidget->playRow(0);
        } else {
            showOpenFileDialog();
        }
    } else {
        if (m_playbackEngine->state() == N::PlaybackPlaying) {
            m_playbackEngine->pause();
        } else {
            m_playbackEngine->play();
        }
    }
}

void NPlayer::showAboutDialog()
{
    NDialogHandler *dialogHandler = new NDialogHandler(QUrl::fromLocalFile(":src/aboutDialog.qml"),
                                                       m_mainWindow);
    dialogHandler->setBeforeShowCallback([&]() {
        QQmlContext *context = dialogHandler->rootContext();
        context->setContextProperty("utils", m_utils);
    });
    dialogHandler->showDialog();
}

void NPlayer::showOpenFileDialog()
{
    QString filters = NSettings::instance()->value("FileFilters").toString();
    QStringList files = QFileDialog::getOpenFileNames(m_mainWindow, tr("Add Files"),
                                                      m_settings->value("LastDirectory").toString(),
                                                      tr("All supported") + " (" + filters + ");;" +
                                                          tr("All files") + " (*)");

    if (files.isEmpty()) {
        return;
    }

    QString lastDir = QFileInfo(files.first()).path();
    m_settings->setValue("LastDirectory", lastDir);

    bool isEmpty = (m_playlistWidget->count() == 0);
    m_playlistWidget->addFiles(files);
    if (isEmpty) {
        m_playlistWidget->playRow(0);
    }
}

void NPlayer::on_mainWindow_scrolled(int delta)
{
    QWheelEvent event(QPoint(), delta, Qt::NoButton, Qt::NoModifier);
    QApplication::sendEvent(m_volumeSlider, &event);
}

void NPlayer::showOpenDirDialog()
{
    QString dir = QFileDialog::getExistingDirectory(m_mainWindow, tr("Add Directory"),
                                                    m_settings->value("LastDirectory").toString(),
                                                    QFileDialog::ShowDirsOnly |
                                                        QFileDialog::DontResolveSymlinks);

    if (dir.isEmpty()) {
        return;
    }

    QString lastDir = QFileInfo(dir).path();
    m_settings->setValue("LastDirectory", lastDir);

    bool isEmpty = (m_playlistWidget->count() == 0);
    m_playlistWidget->addItems(m_utils->dirListRecursive(dir));
    if (isEmpty) {
        m_playlistWidget->playRow(0);
    }
}

void NPlayer::showSavePlaylistDialog()
{
    QString selectedFilter;
    QString file = QFileDialog::getSaveFileName(m_mainWindow, tr("Save Playlist"),
                                                m_settings->value("LastDirectory").toString(),
                                                tr("M3U Playlist") + " (*.m3u);;" +
                                                    tr("Extended M3U Playlist") + " (*.m3u)",
                                                &selectedFilter);

    if (file.isEmpty()) {
        return;
    }

    QString lastDir = QFileInfo(file).path();
    m_settings->setValue("LastDirectory", lastDir);

    if (!file.endsWith(".m3u")) {
        file.append(".m3u");
    }

    if (selectedFilter.startsWith("Extended")) {
        writePlaylist(file, N::ExtM3u);
    } else {
        writePlaylist(file, N::MinimalM3u);
    }
}

void NPlayer::showContextMenu(const QPoint &pos)
{
    // (1, 1) offset to avoid accidental item activation
    m_contextMenu->exec(m_mainWindow->mapToGlobal(pos) + QPoint(1, 1));
}

void NPlayer::showToolTip(const QString &text)
{
    if (text.isEmpty()) {
        QToolTip::hideText();
        return;
    }

    QStringList offsetList = NSettings::instance()->value("TooltipOffset").toStringList();
    QToolTip::showText(mapToGlobal(QCursor::pos() +
                                   QPoint(offsetList.at(0).toInt(), offsetList.at(1).toInt())),
                       text);
}
