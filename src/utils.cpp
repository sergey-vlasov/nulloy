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

#include "utils.h"

#include "playlistStorage.h"
#include "settings.h"

NUtils::NUtils(QObject *parent) : QObject(parent) {}

static QList<NPlaylistDataItem> _processPath(const QString &path, const QStringList &nameFilters)
{
    QList<NPlaylistDataItem> dataItemsList;
    QFileInfo fileInfo = QFileInfo(path);
    if (fileInfo.isDir()) {
        QStringList entryList;
        if (!nameFilters.isEmpty()) {
            entryList = QDir(path).entryList(nameFilters,
                                             QDir::AllDirs | QDir::Files | QDir::NoDotAndDotDot);
        } else {
            entryList = QDir(path).entryList(QDir::AllDirs | QDir::Files | QDir::NoDotAndDotDot);
        }

        foreach (QString f, entryList)
            dataItemsList << _processPath(path + "/" + f, nameFilters);
    } else {
        if (QDir::match(nameFilters, fileInfo.fileName())) {
            if (path.endsWith(".m3u") || path.endsWith(".m3u8")) {
                dataItemsList << NPlaylistStorage::readM3u(path);
            } else {
                dataItemsList << NPlaylistDataItem(path);
            }
        }
    }

    return dataItemsList;
}

QList<NPlaylistDataItem> NUtils::dirListRecursive(const QString &path)
{
    QStringList nameFilters = NSettings::instance()->value("FileFilters").toString().split(' ');
    return _processPath(path, nameFilters);
}

QString NUtils::readFile(const QString &path)
{
    QFile file(path);
    file.open(QIODevice::ReadOnly | QIODevice::Text);
    QTextStream thanksStream(&file);
    QString text = thanksStream.readAll();
    file.close();
    return text;
}

QString NUtils::pathToUri(const QString &path)
{
    return QUrl::fromLocalFile(path).toString();
}
