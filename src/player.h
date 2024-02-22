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

#ifndef N_PLAYER_H
#define N_PLAYER_H

#include <QSystemTrayIcon>
#include <QWidget>

#include "global.h"

class NLogDialog;
class NMainWindow;
class NPlaybackEngineInterface;
class NPlaylistWidget;
class NWaveformSlider;
class NCoverWidget;
class NCoverReaderInterface;
class NVolumeSlider;
class NPreferencesDialogHandler;
class NAboutDialog;
class NScriptEngine;
class NSettings;
class NTrackInfoReader;
class NTrackInfoWidget;
class QMenu;
class NAction;
class QString;
class QTimer;

#ifndef _N_NO_UPDATE_CHECK_
class QNetworkAccessManager;
class QNetworkReply;
#endif

class NPlayer : public QWidget
{
    Q_OBJECT

private:
    NSettings *m_settings;
    NScriptEngine *m_scriptEngine;
    NMainWindow *m_mainWindow;
    NCoverWidget *m_coverWidget;
    NCoverReaderInterface *m_coverReader;
    NWaveformSlider *m_waveformSlider;
    NPreferencesDialogHandler *m_preferencesDialogHandler;
    NAboutDialog *m_aboutDialog;
    NVolumeSlider *m_volumeSlider;
    NTrackInfoReader *m_trackInfoReader;
    NPlaybackEngineInterface *m_playbackEngine;
    QMenu *m_contextMenu;
    QMenu *m_windowSubMenu;
    QMenu *m_playlistSubMenu;
    NPlaylistWidget *m_playlistWidget;
    NTrackInfoWidget *m_trackInfoWidget;
    NLogDialog *m_logDialog;
    QSystemTrayIcon *m_systemTray;
    QTimer *m_trayClickTimer;
    QTimer *m_settingsSaveTimer;
    QTimer *m_writeDefaultPlaylistTimer;
    bool m_trayIconDoubleClickCheck;

    bool eventFilter(QObject *obj, QEvent *event);
    void writePlaylist(const QString &file, N::M3uExtention ext);

    NAction *m_showHideAction;
    NAction *m_playAction;
    NAction *m_stopAction;
    NAction *m_prevAction;
    NAction *m_nextAction;
    NAction *m_preferencesAction;
    NAction *m_exitAction;
    NAction *m_addFilesAction;
    NAction *m_addDirAction;
    NAction *m_savePlaylistAction;
    NAction *m_showCoverAction;
    NAction *m_showPlaybackControlsAction;
    NAction *m_aboutAction;
    NAction *m_playingOnTopAction;
    NAction *m_alwaysOnTopAction;
    NAction *m_fullScreenAction;
    NAction *m_shufflePlaylistAction;
    NAction *m_repeatPlaylistAction;
    NAction *m_loopPlaylistAction;
    NAction *m_scrollToItemPlaylistAction;
    NAction *m_nextFileEnableAction;
    NAction *m_nextFileByNameAscdAction;
    NAction *m_nextFileByNameDescAction;
    NAction *m_nextFileByDateAscd;
    NAction *m_nextFileByDateDesc;
    NAction *m_speedIncreaseAction;
    NAction *m_speedDecreaseAction;
    NAction *m_speedResetAction;
    /*
    NAction *m_pitchIncreaseAction;
    NAction *m_pitchDecreaseAction;
    NAction *m_pitchResetAction;
    */
    void createActions();
    void createContextMenu();
    void createGlobalMenu();
    void createTrayIcon();

    void connectSignals();
    void loadCoverArt(const QString &file);

    void loadDefaultPlaylist();
    void loadSettings();
    void saveSettings();
    void savePlaybackState();

public:
    NPlayer();
    ~NPlayer();
    NMainWindow *mainWindow();
    NPlaybackEngineInterface *playbackEngine();

private slots:
    void on_preferencesDialog_settingsChanged();
    void on_playbackEngine_mediaChanged(const QString &path, int);
    void on_playbackEngine_mediaFailed(const QString &, int);
    void on_playbackEngine_stateChanged(N::PlaybackState state);
    void on_alwaysOnTopAction_toggled(bool checked);
    void on_whilePlayingOnTopAction_toggled(bool checked);
    void on_showCoverAction_toggled(bool checked);
    void on_playButton_clicked();
    void on_playlistAction_triggered();
    void on_playlist_tagEditorRequested(const QString &path);
    void on_playlist_addMoreRequested();
    void on_jumpAction_triggered();
    void on_speedIncreaseAction_triggered();
    void on_speedDecreaseAction_triggered();
    void on_speedResetAction_triggered();
    void on_pitchIncreaseAction_triggered();
    void on_pitchDecreaseAction_triggered();
    void on_pitchResetAction_triggered();

    void on_mainWindow_closed();
    void on_mainWindow_scrolled(int delta);
    void on_trayIcon_activated(QSystemTrayIcon::ActivationReason reason);
    void on_trayClickTimer_timeout();
    void trayIconCountClicks(int clicks);

#ifndef _N_NO_UPDATE_CHECK_
private:
    QNetworkAccessManager *m_versionDownloader;

private slots:
    void downloadVersion();
    void on_versionDownloader_finished(QNetworkReply *reply);
#endif

public slots:
    void quit();
    void toggleWindowVisibility();
    void showAboutMessageBox();
    void showOpenFileDialog();
    void showOpenDirDialog();
    void showSavePlaylistDialog();
    void showToolTip(const QString &text);
    void showContextMenu(const QPoint &pos);
    void readMessage(const QString &str);
};

#endif
