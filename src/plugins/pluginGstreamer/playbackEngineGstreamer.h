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

#ifndef N_PLAYBACK_ENGINE_GSTREAMER_H
#define N_PLAYBACK_ENGINE_GSTREAMER_H

#include <gst/gst.h>

#include "global.h"
#include "playbackEngineInterface.h"
#include "plugin.h"

class QTimer;

class NPlaybackEngineGStreamer : public NPlaybackEngineInterface, public NPlugin
{
    Q_OBJECT
    Q_INTERFACES(NPlaybackEngineInterface NPlugin)
    Q_PROPERTY(int state READ state NOTIFY stateChanged)
    Q_PROPERTY(qreal position READ position WRITE setPosition NOTIFY positionChanged)
    Q_PROPERTY(qreal volume READ volume WRITE setVolume NOTIFY volumeChanged)

private:
    GstElement *m_playbin;
    GstElement *m_pitchElement;

    QTimer *m_checkStatusTimer;
    QTimer *m_emitStateTimer;
    QTimer *m_gstBusPopTimer;
    qreal m_speed;
    bool m_speedPostponed;
    qreal m_pitch;
    qreal m_volume;
    qreal m_position;
    bool m_positionPostponed;
    GstState m_gstState;
    gint64 m_durationNsec;
    bool m_crossfading;
    bool m_nextMediaRequestBlock;

    QString m_currentMedia;
    int m_currentContext;
    QString m_bkpMedia;
    int m_bkpContext;

    N::PlaybackState fromGstState(GstState state) const;
    bool gstSetFile(const QString &file, int context, bool prepareNext);
    void processGstMessage(GstMessage *msg);
    void fail();

public:
    NPlaybackEngineGStreamer(QObject *parent = NULL) : NPlaybackEngineInterface(parent) {}
    ~NPlaybackEngineGStreamer();
    void init();
    QString interfaceString() const { return NPlaybackEngineInterface::interfaceString(); }
    N::PluginType type() const { return N::PlaybackEngine; }

    Q_INVOKABLE bool hasMedia() const;
    Q_INVOKABLE QString currentMedia() const;
    Q_INVOKABLE N::PlaybackState state() const;

    Q_INVOKABLE qreal volume() const;
    Q_INVOKABLE qreal position() const;
    Q_INVOKABLE qint64 durationMsec() const;
    Q_INVOKABLE qreal speed() const;
    Q_INVOKABLE qreal pitch() const;

    void _emitNextMediaRequest();
    bool _nextMediaRequestBlocked();

public slots:
    Q_INVOKABLE void setMedia(const QString &file, int context);
    Q_INVOKABLE void nextMediaRespond(const QString &file, int context);
    Q_INVOKABLE void setVolume(qreal volume);
    Q_INVOKABLE void setPosition(qreal pos);
    Q_INVOKABLE void jump(qint64 msec);
    Q_INVOKABLE void setSpeed(qreal speed);
    Q_INVOKABLE void setPitch(qreal speed);

    Q_INVOKABLE void play();
    Q_INVOKABLE void stop();
    Q_INVOKABLE void pause();

private slots:
    void checkStatus();

signals:
    void positionChanged(qreal pos);
    void volumeChanged(qreal volume);
    void message(N::MessageIcon icon, const QString &file, const QString &msg);
    void mediaChanged(const QString &file, int context);
    void mediaFinished(const QString &file, int context);
    void nextMediaRequested();
    void mediaFailed(const QString &file, int context);
    void stateChanged(N::PlaybackState state);
    void tick(qint64 msec);
};

#endif
