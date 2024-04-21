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

#include <QMap>
#include <QPainter>
#include <QQuickPaintedItem>

class QSvgRenderer;

class NSvgImage : public QQuickPaintedItem
{
    Q_OBJECT
    Q_PROPERTY(QString source READ source WRITE setSource)
    Q_PROPERTY(QString elementId READ elementId WRITE setElementId)
    Q_PROPERTY(QColor colorOverlay READ colorOverlay WRITE setColorOverlay)

public:
    NSvgImage(QQuickItem *parent = nullptr);
    void paint(QPainter *painter);

    QString source() const;
    QString elementId() const;
    QColor colorOverlay() const;

    void setSource(const QString &source);
    void setElementId(const QString &id);
    void setColorOverlay(const QColor &color);

private:
    QString m_source;
    QString m_elementId;
    QColor m_colorOverlay;
    QSvgRenderer *m_renderer;
    static QMap<QString, QSvgRenderer *> m_rendererCache;
};
