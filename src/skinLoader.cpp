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

#include "skinLoader.h"

#include <qtiocompressor.h>

#include <QBuffer>
#include <QCoreApplication>
#include <QMessageBox>

#include "common.h"
#include "settings.h"
#include "skinFileSystem.h"

static const char _skinPrefer[] = "Slim";
static const char _skinSuffix[] = "nzs";
static const char _idFileName[] = "id.txt";
static const char _formFileName[] = "form.ui";
static const char _scriptFileName[] = "script.js";
static const char _skinsDirName[] = "skins";

namespace NSkinLoader
{
    bool __init = false;
    QMap<int, QString> _identifiers;
    QString _formPath;
    QString _scriptPath;

    bool _nextFile(QFile &zipFile, QString &fileName, QByteArray &data);
    void _init();
} // namespace NSkinLoader

bool NSkinLoader::_nextFile(QFile &zipFile, QString &fileName, QByteArray &data)
{
    quint32 signature, crc, compSize, unCompSize;
    quint16 extractVersion, bitFlag, compMethod, modTime, modDate;
    quint16 nameLen, extraLen;

    QDataStream in(&zipFile);
    in.setByteOrder(QDataStream::LittleEndian);

    in >> signature;
    if (signature != 0x04034b50) {
        return false;
    }

    in >> extractVersion >> bitFlag >> compMethod;
    in >> modTime >> modDate >> crc >> compSize >> unCompSize;
    in >> nameLen >> extraLen;

    fileName = QString(zipFile.read(nameLen));
    zipFile.read(extraLen);

    QByteArray compData = zipFile.read(compSize);
    QByteArray unCompData;
    if (compMethod == 0) {
        unCompData = compData;
    } else {
        QBuffer compBuf(&compData);
        QtIOCompressor compressor(&compBuf);
        compressor.setStreamFormat(QtIOCompressor::RawZipFormat);
        compressor.open(QIODevice::ReadOnly);
        unCompData = compressor.readAll();
    }

    data = unCompData;

    return true;
}

void NSkinLoader::_init()
{
    if (__init) {
        return;
    }
    __init = true;

    QStringList skinsDirList;
    // ":" is a prefix for .qrc resources
    skinsDirList << QString() + ":" + _skinsDirName
                 << QCoreApplication::applicationDirPath() + "/" + _skinsDirName;
#ifndef Q_OS_WIN
    if (NCore::rcDir() != QCoreApplication::applicationDirPath()) {
        skinsDirList << NCore::rcDir() + "/" + _skinsDirName;
    }
    if (QDir(QCoreApplication::applicationDirPath()).dirName() == "bin") {
        QDir dir(QCoreApplication::applicationDirPath());
        dir.cd(QString() + "../share/nulloy/" + _skinsDirName);
        skinsDirList << dir.absolutePath();
    }
#endif
    QFileInfoList containersInfoList;
    foreach (QString dirStr, skinsDirList) {
        QDir dir(dirStr);
        if (dir.exists()) {
            containersInfoList << dir.entryInfoList(QDir::AllDirs | QDir::Files |
                                                    QDir::NoDotAndDotDot);
        }
    }

    for (int i = 0; i < containersInfoList.size(); ++i) {
        QFileInfo skinContainer = containersInfoList.at(i);
        QString id;
        if (!skinContainer.isDir()) {
            if (skinContainer.suffix() != _skinSuffix) {
                containersInfoList.removeAt(i);
                --i;
                continue;
            }

            QFile zipFile(skinContainer.absoluteFilePath());
            zipFile.open(QIODevice::ReadOnly);
            QString fileName;
            QByteArray data;
            while (_nextFile(zipFile, fileName, data)) {
                if (fileName == _idFileName) {
                    id = data.mid(0, data.indexOf('\n')).replace('\r', "") + " (." + _skinSuffix +
                         ")";
                    break;
                }
            }
            zipFile.close();
        } else if (skinContainer.isDir()) {
            QFileInfoList infoList = QDir(skinContainer.absoluteFilePath())
                                         .entryInfoList(QDir::Files | QDir::NoDotAndDotDot);
            bool isSkin = false;
            foreach (QFileInfo fileInfo, infoList) {
                if (fileInfo.fileName() != _idFileName) {
                    continue;
                }
                isSkin = true;
                QFile file(fileInfo.absoluteFilePath());
                file.open(QIODevice::ReadOnly);
                QByteArray data = file.readAll();
                id = data.mid(0, data.indexOf('\n')).replace('\r', "");
                file.close();
                break;
            }
            if (!isSkin) {
                containersInfoList.removeAt(i);
                --i;
                continue;
            }
        }
        if (skinContainer.absoluteFilePath().startsWith(QString() + ":" + _skinsDirName)) {
            id.insert(id.lastIndexOf('/'), QObject::tr(" (Built-in)"));
        }
        _identifiers.insert(i, id);
    }

    QString skinStr = NSettings::instance()->value("Skin").toString();
    QStringList values = _identifiers.values();
    int index;
    index = values.indexOf("Nulloy/Skin/" + skinStr);
    if (index == -1) {
        index = values.indexOf(QRegExp(QString() + "Nulloy/Skin/" + _skinPrefer + ".*"));
    }
    if (index == -1) {
        index = 0;
    }

    if (_identifiers.count() == 0) {
        QMessageBox box(QMessageBox::Critical, QObject::tr("Skin loading error"),
                        QObject::tr("No skins found."), QMessageBox::Close);
        box.exec();
        return;
    }

    QFileInfo skinContainer = containersInfoList.at(index);
    if (!skinContainer.isDir() && skinContainer.suffix() == _skinSuffix) {
        QFile zipFile(skinContainer.absoluteFilePath());
        zipFile.open(QIODevice::ReadOnly);
        QString fileName;
        QByteArray data;
        while (_nextFile(zipFile, fileName, data)) {
            if (fileName != _scriptFileName && fileName != _formFileName) {
                NSkinFileSystem::addFile(fileName, data);
                continue;
            }
            QString str(data);
            QRegExp rx("(url\\()([^:])");
            str.replace(rx, "\\1" + NSkinFileSystem::prefix() + "\\2");
            NSkinFileSystem::addFile(fileName, str.toUtf8());

            if (fileName == _formFileName) {
                _formPath = NSkinFileSystem::prefix() + fileName;
            } else if (fileName == _scriptFileName) {
                _scriptPath = NSkinFileSystem::prefix() + fileName;
            }
        }
        zipFile.close();
    } else if (skinContainer.isDir()) {
        QFileInfoList infoList =
            QDir(skinContainer.absoluteFilePath()).entryInfoList(QDir::Files | QDir::NoDotAndDotDot);
        foreach (QFileInfo fileInfo, infoList) {
            QFile file(fileInfo.absoluteFilePath());
            file.open(QIODevice::ReadOnly);
            QByteArray data = file.readAll();
            if (fileInfo.fileName() != _scriptFileName && fileInfo.fileName() != _formFileName) {
                NSkinFileSystem::addFile(fileInfo.fileName(), data);
                continue;
            }
            QString str(data);
            QRegExp rx("(url\\()([^:])");
            str.replace(rx, "\\1" + NSkinFileSystem::prefix() + "\\2");
            str.replace("<iconset resource=\"resources.qrc\">", "<iconset>");
            NSkinFileSystem::addFile(fileInfo.fileName(), str.toUtf8());

            if (fileInfo.fileName() == _formFileName) {
                _formPath = NSkinFileSystem::prefix() + fileInfo.fileName();
            } else if (fileInfo.fileName() == _scriptFileName) {
                _scriptPath = NSkinFileSystem::prefix() + fileInfo.fileName();
            }
        }
    }

    NSettings::instance()->setValue("Skin", _identifiers.value(index).section('/', 2));
}

QStringList NSkinLoader::skinIdentifiers()
{
    _init();
    return _identifiers.values();
}

QString NSkinLoader::skinUiFormFile()
{
    _init();
    return _formPath;
}

QString NSkinLoader::skinScriptFile()
{
    _init();
    return _scriptPath;
}
