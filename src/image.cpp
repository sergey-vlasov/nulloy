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

#include "image.h"

#include <QtGlobal>

Q_DECLARE_METATYPE(QMargins)

NImage::NImage(QQuickItem *parent) : QQuickPaintedItem(parent)
{
    m_upscale = true;
    m_margin = 0.0;
    m_growVertically = false;
    m_growHorizontally = false;
    setImplicitWidth(1);
    setImplicitHeight(1);
    connect(this, &QQuickItem::heightChanged, [&]() { scale(false); });
    connect(this, &QQuickItem::widthChanged, [&]() { scale(false); });
}

void NImage::scale(bool force)
{
    if (!m_image.isNull()) {
        int maxWidth = INT_MAX;
        if (!m_growHorizontally) {
            maxWidth = boundingRect().width() - m_margin * 2;
        }
        if (maxWidth <= 0) {
            return;
        }

        int maxHeight = INT_MAX;
        if (!m_growVertically) {
            maxHeight = boundingRect().height() - m_margin * 2;
        }
        if (maxHeight <= 0) {
            return;
        }

        if (!m_upscale) {
            maxHeight = qMin(maxHeight, m_image.height());
            maxWidth = qMin(maxWidth, m_image.width());
        }

        QSize scaledSize = m_image.size().scaled(maxWidth, maxHeight, Qt::KeepAspectRatio);
        if (!force && scaledSize == m_scaledSize) {
            return;
        }

        m_scaledSize = scaledSize;
        m_scaledImage = m_image.scaled(m_scaledSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        setImplicitWidth(m_scaledSize.width() + m_margin * 2);
        setImplicitHeight(m_scaledSize.height() + m_margin * 2);
        update();
    }
}

QImage NImage::image() const
{
    return m_image;
}

void NImage::setImage(const QImage &image)
{
    if (m_image != image) {
        m_image = image;
        scale(true);
        emit imageChanged();
    }
}

void NImage::paint(QPainter *painter)
{
    if (!m_image.isNull()) {
        painter->setRenderHint(QPainter::SmoothPixmapTransform);
        painter->drawImage(m_margin + (boundingRect().width() - m_margin * 2) / 2 -
                               m_scaledSize.width() / 2,
                           m_margin + (boundingRect().height() - m_margin * 2) / 2 -
                               m_scaledSize.height() / 2,
                           m_scaledImage);
    }
}
