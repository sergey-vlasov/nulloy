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

#include <qtsingleapplication.h>

#include "common.h"
#include "player.h"
#include "settings.h"

#ifndef _N_NO_SKINS_
#include "skinFileSystem.h"
Q_IMPORT_PLUGIN(NWidgetCollection)
#endif

bool logToFile = false;

static void print_out(const QString &out)
{
    fprintf(stdout, "%s\n", out.toStdString().c_str());
}

static void print_err(const QString &err)
{
    fprintf(stderr, "%s\n", err.toStdString().c_str());
}

static void print_help()
{
    print_out("Usage:  " + NCore::applicationBasenameName() +
              " [[option] | [files]]\n"
              "\n"
              "Options:\n"
              "    --next         play next file\n"
              "    --prev         play previous file\n"
              "    --stop         stop playback\n"
              "    --pause        pause playback\n"
              "    --log          log to file\n"
              "    --version      print version\n"
              "    -h, --help     print this message\n");
}

static void print_try()
{
    print_out("Try `" + NCore::applicationBasenameName() + " --help' for more information");
}

void messageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    Q_UNUSED(context);
    print_err(msg);

    if (logToFile) {
        QString prefix;
        switch (type) {
            case QtInfoMsg:
                prefix = "Info";
                break;
            case QtDebugMsg:
                prefix = "Debug";
                break;
            case QtWarningMsg:
                prefix = "Warning";
                break;
            case QtCriticalMsg:
                prefix = "Critical";
                break;
            case QtFatalMsg:
                prefix = "Fatal";
                break;
        }
        QFile logFile(NCore::rcDir() + "/" + NCore::applicationBinaryName() + ".log");
        logFile.open(QIODevice::Text | QIODevice::WriteOnly | QIODevice::Append);
        QTextStream stream(&logFile);
        stream << QString("%1 %2: %3")
                      .arg(QTime::currentTime().toString("hh:mm:ss.zzz"), prefix,
                           msg.toLocal8Bit().constData())
               << endl;
        logFile.close();
    }
}

int main(int argc, char *argv[])
{
    // for Qt core plugins
#if defined(Q_OS_WIN)
    QCoreApplication::addLibraryPath(QFileInfo(argv[0]).dir().path() + "/Plugins/");
#elif defined(Q_OS_MAC)
    QCoreApplication::addLibraryPath(QFileInfo(argv[0]).dir().path() + "/plugins/");
#endif

    QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

#ifdef Q_OS_MAC
    // https://bugreports.qt-project.org/browse/QTBUG-32789
    if (QSysInfo::MacintoshVersion > QSysInfo::MV_10_8)
        QFont::insertSubstitution(".Lucida Grande UI", "Lucida Grande");

    // https://bugreports.qt-project.org/browse/QTBUG-40833
    if (QSysInfo::MacintoshVersion > QSysInfo::MV_10_9)
        QFont::insertSubstitution(".Helvetica Neue DeskInterface", "Helvetica Neue");
#endif

    QtSingleApplication instance(argc, argv);
    instance.setApplicationName("Nulloy");
    instance.setApplicationVersion(QString(_N_VERSION_));
    instance.setOrganizationDomain("nulloy.com");
    instance.setQuitOnLastWindowClosed(false);

    qInstallMessageHandler(messageHandler);

    QStringList argList = instance.arguments();
    argList.takeFirst();
    QStringList files;
    QStringList options;
    foreach (QString arg, argList) {
        if (arg.startsWith("-")) {
            if (arg == "-h") {
                print_help();
                return 0;
            } else if (arg.startsWith("--")) {
                if (arg == "--next" || arg == "--prev" || arg == "--stop" || arg == "--pause") {
                    options << arg;
                } else if (arg == "--log") {
                    logToFile = true;
                } else if (arg == "--version") {
                    print_out(instance.applicationVersion());
                    return 0;
                } else if (arg == "--help") {
                    print_help();
                    return 0;
                } else {
                    print_err("unrecognized option '" + arg + "'");
                    print_try();
                    return 1;
                }
            } else {
                print_err("unrecognized option '" + arg + "'");
                print_try();
                return 1;
            }
        } else {
            files << arg;
        }
    }

    // construct message
    QString msg = (options + files).join(MSG_SPLITTER);
    if (NSettings::instance()->value("SingleInstance").toBool()) {
        // try to send it to an already running instrance
        if (instance.sendMessage(msg)) {
            return 0; // return if delivered
        }
    }

    QIcon icon;
#ifdef Q_OS_LINUX
    icon = QIcon::fromTheme("nulloy");
#endif
    if (icon.isNull()) {
        icon.addFile(":icon.svg");
    }
    QGuiApplication::setWindowIcon(icon);

#ifndef _N_NO_SKINS_
    NSkinFileSystem::init();
#endif

    NPlayer p;
    QObject::connect(&instance, SIGNAL(messageReceived(const QString &)), &p,
                     SLOT(readMessage(const QString &)));
    QObject::connect(&instance, SIGNAL(aboutToQuit()), &p, SLOT(quit()));

    // manually read the message
    if (!msg.isEmpty()) {
        p.readMessage(msg);
    }

    instance.installEventFilter(&p);

    return instance.exec();
}
