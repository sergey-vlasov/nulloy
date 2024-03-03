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

#ifndef N_TRACK_INFO_MODEL_H
#define N_TRACK_INFO_MODEL_H

#include <QMap>
#include <QQmlPropertyMap>

class NTrackInfoReader;

class NTrackInfoModel : public QQmlPropertyMap
{
    Q_OBJECT

private:
    qint64 m_msec;
    QString m_tooltipFormat;
    QMap<QString, QString> m_fileKeysMap;
    QMap<QString, QString> m_playlistKeysMap;
    QMap<QString, QString> m_playbackKeysMap;
    NTrackInfoReader *m_trackInfoReader;

public:
    NTrackInfoModel(NTrackInfoReader *reader, QObject *parent = 0);
    ~NTrackInfoModel();

    Q_INVOKABLE QString formatTooltip(qreal pos);

public slots:
    void updateFileLabels(const QString &file);
    void updatePlaylistLabels();
    void updatePlayback(qint64 msec);
    void loadSettings();
};

#endif
