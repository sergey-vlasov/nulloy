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

#include "dialogHandler.h"

#include <QQmlApplicationEngine>
#include <QWidget>

NDialogHandler::~NDialogHandler() {}

NDialogHandler::NDialogHandler(const QUrl &url, QWidget *parent) : QObject(parent), m_url(url) {}

void NDialogHandler::setBeforeShowCallback(Callback callback)
{
    m_beforeShowCallback = callback;
}

void NDialogHandler::setAfterShowCallback(Callback callback)
{
    m_afterShowCallback = callback;
}

void NDialogHandler::showDialog()
{
    m_qmlEngine = new QQmlApplicationEngine();

    QQmlContext *context = m_qmlEngine->rootContext();
    context->setContextProperty("dialogHandler", this);
    context->setContextProperty("parentWindow", qobject_cast<QWidget *>(parent()));

    if (m_beforeShowCallback) {
        m_beforeShowCallback();
    }

    m_qmlEngine->load(m_url);

    QObject *rootObject = m_qmlEngine->rootObjects().first();
    QObject::connect(rootObject, SIGNAL(accepted()), this, SLOT(on_closed()));
    QObject::connect(rootObject, SIGNAL(rejected()), this, SLOT(on_closed()));

    if (m_afterShowCallback) {
        m_afterShowCallback();
    }
}

QQmlContext *NDialogHandler::rootContext()
{
    if (m_qmlEngine) {
        return m_qmlEngine->rootContext();
    }

    return nullptr;
}

QObject *NDialogHandler::rootObject()
{
    if (m_qmlEngine) {
        return m_qmlEngine->rootObjects().first();
    }

    return nullptr;
}

void NDialogHandler::centerToParent()
{
    QObject *rootObject = m_qmlEngine->rootObjects().first();
    QRect geometry = qobject_cast<QWidget *>(parent())->geometry();
    int x = geometry.x() + (geometry.width() - rootObject->property("width").toInt()) / 2;
    int y = geometry.y() + (geometry.height() - rootObject->property("height").toInt()) / 2;
    rootObject->setProperty("x", x);
    rootObject->setProperty("y", y);
}

void NDialogHandler::on_closed()
{
    m_qmlEngine->deleteLater();
    m_qmlEngine = nullptr;
}
