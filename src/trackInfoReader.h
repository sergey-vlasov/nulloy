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

#ifndef N_TRACK_INFO_READER_H
#define N_TRACK_INFO_READER_H

#include "tagReaderInterface.h"

#include <QFileInfo>
#include <QObject>

class NTrackInfoReader : public QObject
{
    Q_OBJECT

private:
    NTagReaderInterface *m_reader;
    QFileInfo m_fileInfo;

    int m_durationSec;
    int m_positionSec;
    int m_playlistDurationSec;

    QString parseFormat(const QString &format, int &cur, bool skip, bool &ok) const;

public:
    NTrackInfoReader(NTagReaderInterface *tagReader, QObject *parent = 0);
    ~NTrackInfoReader() {}

    void setSource(const QString &file);
    void updatePlaybackPosition(int seconds);
    void updatePlaylistDuration(int seconds);
    QString toString(const QString &format) const;
    Q_INVOKABLE QString format(const QString &fmt) const { return toString(fmt); }
    QString getInfo(QChar ch) const;
    static QString formatTime(int durationSec);
};

#endif
