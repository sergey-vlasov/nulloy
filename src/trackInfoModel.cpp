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

#include "trackInfoModel.h"

#include "pluginLoader.h"
#include "settings.h"
#include "trackInfoReader.h"

NTrackInfoModel::~NTrackInfoModel() {}

NTrackInfoModel::NTrackInfoModel(NTrackInfoReader *reader, QObject *parent)
    : QQmlPropertyMap(this, parent), m_trackInfoReader(reader)
{
    // clang-format off
    QStringList keys{
        "topLeft",    "topCenter",    "topRight",
        "middleLeft", "middleCenter", "middleRight",
        "bottomLeft", "bottomCenter", "bottomRight",
    };
    // clang-format on
    foreach (const QString &key, keys) {
        insert(key, "");
    }

    m_msec = 0;

    loadSettings();
}

QString NTrackInfoModel::formatTooltip(qreal pos)
{
    QString text = m_tooltipFormat;
    if (m_tooltipFormat.isEmpty()) {
        return "";
    }

    QString seconds = m_trackInfoReader->getInfo('D');
    if (seconds.isEmpty()) {
        return "";
    }

    int seconds_at_mouse_pos = seconds.toInt() * pos;
    // time position under mouse pointer:
    text.replace("%C", NTrackInfoReader::formatTime(seconds_at_mouse_pos));

    int seconds_elapsed = m_msec / 1000;
    int seconds_delta = seconds_at_mouse_pos - seconds_elapsed;
    QString delta_formatted = NTrackInfoReader::formatTime(qAbs(seconds_delta));
    // time offset under mouse pointer:
    text.replace("%o", QString("%1%2").arg(seconds_delta < 0 ? "-" : "+").arg(delta_formatted));

    return text;
}

void NTrackInfoModel::updateFileLabels(const QString &file)
{
    m_trackInfoReader->setSource(file);

    if (!QFileInfo(file).exists()) {
        return;
    }

    QString encoding = NSettings::instance()->value("EncodingTrackInfo").toString();
    foreach (const QString &key, m_fileKeysMap.keys()) {
        const QString &format = m_fileKeysMap[key];
        const QString &text = m_trackInfoReader->toString(format);
        insert(key, text);
    }
}

void NTrackInfoModel::updatePlaylistLabels()
{
    foreach (const QString &key, m_playlistKeysMap.keys()) {
        const QString &format = m_playlistKeysMap[key];
        const QString &text = m_trackInfoReader->toString(format);
        insert(key, text);
    }
}

void NTrackInfoModel::loadSettings()
{
    m_fileKeysMap.clear();
    m_playbackKeysMap.clear();
    m_playlistKeysMap.clear();

    foreach (const QString &key, keys()) {
        QString capitalizedKey = key;
        capitalizedKey[0] = capitalizedKey[0].toUpper();
        const QString &format =
            NSettings::instance()->value("TrackInfo/" + capitalizedKey).toString();

        if (format.contains("%T") || format.contains("%r")) { // elapsed or remaining playback time
            m_playbackKeysMap[key] = format;
        }

        if (format.contains("%L")) { // playlist duration
            m_playlistKeysMap[key] = format;
        }

        m_fileKeysMap[key] = format;
    }

    m_tooltipFormat = NSettings::instance()->value("TooltipTrackInfo").toString();
}

void NTrackInfoModel::updatePlayback(qint64 msec)
{
    m_msec = msec;

    m_trackInfoReader->updatePlaybackPosition(msec / 1000);
    foreach (const QString &key, m_playbackKeysMap.keys()) {
        const QString &format = m_playbackKeysMap[key];
        const QString &text = m_trackInfoReader->toString(format);
        insert(key, text);
    }
}
