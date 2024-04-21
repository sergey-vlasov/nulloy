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

#ifndef N_WAVEFORM_BAR_H
#define N_WAVEFORM_BAR_H

#include <QPainterPath>
#include <QQuickPaintedItem>

class NWaveformBuilderInterface;

class NWaveformBar : public QQuickPaintedItem
{
    Q_OBJECT
    Q_PROPERTY(QColor color WRITE setColor)
    Q_PROPERTY(QColor borderColor WRITE setBorderColor)
    Q_PROPERTY(qreal borderWidth WRITE setBorderWidth)

private:
    NWaveformBuilderInterface *m_waveBuilder;

    QColor m_color;
    QColor m_borderColor;
    qreal m_borderWidth;
    QTimer *m_timer;
    int m_oldBuilderIndex;
    float m_oldBuilderPos;
    bool m_needsUpdate;

    void init();

public:
    NWaveformBar(QQuickItem *parent = nullptr);
    void paint(QPainter *painter);

    void setColor(const QColor &color);
    void setBorderColor(const QColor &color);
    void setBorderWidth(qreal width);

private slots:
    void checkForUpdate();
};

#endif
