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

#include "svgImage.h"

#include "skinFileSystem.h"

#include <QQmlContext>
#include <QQmlEngine>
#include <QQuickWindow>
#include <QSvgRenderer>

QMap<QString, QSvgRenderer *> NSvgImage::m_rendererCache;

NSvgImage::NSvgImage(QQuickItem *parent) : QQuickPaintedItem(parent)
{
    m_colorOverlay = Qt::transparent;
}

QString NSvgImage::source() const
{
    return m_source;
}

QString NSvgImage::elementId() const
{
    return m_elementId;
}

QColor NSvgImage::colorOverlay() const
{
    return m_colorOverlay;
}

void NSvgImage::setSource(const QString &source)
{
    m_source = source;
    if (m_rendererCache.contains(source)) {
        m_renderer = m_rendererCache[source];
        return;
    }

    QByteArray data = NSkinFileSystem::readFile(m_source);
    if (data.isEmpty()) {
        qCritical() << "NSvgImage: failed to read source:" << m_source;
        m_source = "";
        return;
    }

    m_renderer = new QSvgRenderer(data);
    m_rendererCache[m_source] = m_renderer;
}

void NSvgImage::setElementId(const QString &id)
{
    m_elementId = id;
    update();
}

void NSvgImage::setColorOverlay(const QColor &color)
{
    m_colorOverlay = color;
    update();
}

void NSvgImage::paint(QPainter *painter)
{
    if (m_source.isEmpty() || m_elementId.isEmpty()) {
        return;
    }

    QRectF elementBounds = m_renderer->boundsOnElement(m_elementId);
    if (!elementBounds.isValid()) {
        qCritical() << "NSvgImage: element ID not found:" << m_elementId;
        return;
    }

    qreal dpr = window()->devicePixelRatio();
    qreal scaleX = elementBounds.width() / width();
    qreal scaleY = elementBounds.height() / height();
    int offsetX = (width() - elementBounds.width()) / 2;
    int offsetY = (height() - elementBounds.height()) / 2;

    //qDebug() << m_elementId << ":" << width() << height() << "bounds:" << elementBounds.width()
    //         << elementBounds.height();

    painter->translate(offsetX, offsetY);
    painter->scale(scaleX / dpr, scaleY / dpr);

    m_renderer->render(painter, m_elementId);

    if (m_colorOverlay != Qt::transparent) {
        QColor color(m_colorOverlay);
        painter->setCompositionMode(QPainter::CompositionMode_SourceIn);
        painter->fillRect(0, 0, width() * dpr, height() * dpr, color);
    }
}
