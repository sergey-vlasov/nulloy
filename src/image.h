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

#ifndef N_COVER_IMAGE_H
#define N_COVER_IMAGE_H

#include <QImage>
#include <QPainter>
#include <QQuickPaintedItem>

class NImage : public QQuickPaintedItem
{
    Q_OBJECT
    Q_PROPERTY(QImage image READ image WRITE setImage NOTIFY imageChanged)
    Q_PROPERTY(qreal margin MEMBER m_margin)
    Q_PROPERTY(bool upscale MEMBER m_upscale)
    Q_PROPERTY(bool growVertically MEMBER m_growVertically)
    Q_PROPERTY(bool growHorizontally MEMBER m_growHorizontally)

public:
    NImage(QQuickItem *parent = nullptr);

    QImage image() const;
    void setImage(const QImage &image);

signals:
    void imageChanged();

protected:
    void paint(QPainter *painter) override;

private:
    void scale(bool force = false);
    QImage m_image;
    QImage m_scaledImage;
    QSize m_scaledSize;
    bool m_upscale;
    qreal m_margin;
    bool m_growVertically;
    bool m_growHorizontally;
};

#endif
