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

#include "mainWindow.h"

#include "common.h"
#include "settings.h"

#ifndef _N_NO_SKINS_
#include <QUiLoader>

#include "skinFileSystem.h"
#endif

#ifdef Q_OS_WIN
#include <dwmapi.h>
#include <windows.h>

#include "w7TaskBar.h"
// These window messages are not defined in dwmapi.h
#ifndef WM_DWMCOMPOSITIONCHANGED
#define WM_DWMCOMPOSITIONCHANGED 0x031E
#endif
#endif

#include <QApplication>
#include <QDesktopWidget>
#include <QEvent>
#include <QIcon>
#include <QLayout>
#include <QTime>
#include <QWindowStateChangeEvent>

#define RESIZE_BORDER 5

NMainWindow::NMainWindow(const QString &uiFile, QWidget *parent) : QDialog(parent)
{
#ifdef Q_OS_WIN
    m_framelessShadow = false;
#endif

    setObjectName("mainWindow");

#ifndef _N_NO_SKINS_
    QUiLoader loader;
    QFile formFile(uiFile);
    formFile.open(QIODevice::ReadOnly);
    QWidget *form = loader.load(&formFile);
    formFile.close();

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(form->layout()->itemAt(0)->widget());
    layout->setContentsMargins(0, 0, 0, 0);
    setLayout(layout);
    setStyleSheet(form->styleSheet());
    form->setStyleSheet("");
#else
    Q_UNUSED(uiFile)
    ui.setupUi(this);
#endif

    m_unmaximizedSize = QSize();
    m_unmaximizedPos = QPoint();
    m_isFullScreen = false;
    m_dragActive = false;
    m_resizeActive = false;
    m_resizeSection = Qt::NoSection;

    // enabling dragging window from any point
    QList<QWidget *> widgets = findChildren<QWidget *>();
    foreach (QWidget *widget, widgets)
        widget->installEventFilter(this);

    QMetaObject::connectSlotsByName(this);
}

NMainWindow::~NMainWindow() {}

bool NMainWindow::isFullSceen()
{
    return m_isFullScreen;
}

void NMainWindow::loadSettings()
{
    QPoint _pos;
    QStringList posList = NSettings::instance()->value("Position").toStringList();
    if (!posList.isEmpty()) {
        _pos = QPoint(posList.at(0).toInt(), posList.at(1).toInt());
        move(_pos);
    }

    QSize _size;
    QStringList sizeList = NSettings::instance()->value("Size").toStringList();
    if (!sizeList.isEmpty()) {
        _size = QSize(sizeList.at(0).toInt(), sizeList.at(1).toInt());
    } else {
        _size = QSize(430, 350);
    }
    resize(_size);

    if (NSettings::instance()->value("Maximized").toBool()) {
        m_unmaximizedPos = _pos;
        m_unmaximizedSize = _size;
        toggleMaximize();
    }
}

void NMainWindow::saveSettings()
{
    NSettings::instance()->setValue("Maximized", isMaximized());

    QPoint _pos = pos();
    QSize _size = size();
    if (m_unmaximizedSize.isValid()) {
        _pos = m_unmaximizedPos;
        _size = m_unmaximizedSize;
    }

    NSettings::instance()->setValue("Position", QStringList() << QString::number(_pos.x())
                                                              << QString::number(_pos.y()));
    NSettings::instance()->setValue("Size", QStringList() << QString::number(_size.width())
                                                          << QString::number(_size.height()));
}

void NMainWindow::show()
{
    if (isMaximized()) {
        showMaximized();
        setGeometry(QApplication::desktop()->availableGeometry());
        showMaximized();
    } else {
        showNormal();
    }
}

void NMainWindow::toggleMaximize()
{
    if (isMaximized()) {
        showNormal();
        resize(m_unmaximizedSize);
        move(m_unmaximizedPos);
        m_unmaximizedPos = QPoint();
        m_unmaximizedSize = QSize();
    } else {
        m_unmaximizedPos = pos();
        m_unmaximizedSize = size();
        showMaximized();
#ifdef Q_OS_WIN
        setGeometry(QApplication::desktop()->availableGeometry());
        showMaximized();
#endif
    }

    emit fullScreenEnabled(false);
    emit maximizeEnabled(isMaximized());
}

void NMainWindow::toggleFullScreen()
{
    if (!m_isFullScreen) {
        if (!m_unmaximizedSize.isValid()) {
            m_unmaximizedSize = size();
            m_unmaximizedPos = pos();
        }
        QDialog::showFullScreen();
    } else {
        QPoint _pos = m_unmaximizedPos;
        QSize _size = m_unmaximizedSize;
        QDialog::showNormal();
        m_unmaximizedPos = _pos;
        m_unmaximizedSize = _size;
        if (m_unmaximizedSize.isValid()) {
            resize(m_unmaximizedSize);
            move(m_unmaximizedPos);
            m_unmaximizedSize = QSize();
            m_unmaximizedPos = QPoint();
        }
    }

    m_isFullScreen = !m_isFullScreen;
    emit fullScreenEnabled(m_isFullScreen);
}

void NMainWindow::setTitle(QString title)
{
    setWindowTitle(title);
    emit newTitle(title);
}

void NMainWindow::changeEvent(QEvent *event)
{
    QWidget::changeEvent(event);

    emit focusChanged(isActiveWindow());

    if (windowFlags() & Qt::FramelessWindowHint) {
        setAttribute(Qt::WA_Hover, true);
        return;
    }

    if (event->type() == QEvent::WindowStateChange) {
        QWindowStateChangeEvent *stateEvent = static_cast<QWindowStateChangeEvent *>(event);
        if (stateEvent->oldState() == Qt::WindowNoState && isMaximized()) {
            if (!m_unmaximizedSize.isValid()) {
                m_unmaximizedPos = pos();
                m_unmaximizedSize = size();
            }
        } else if (!isMaximized() && !isMinimized() && !m_isFullScreen) {
            m_unmaximizedSize = QSize();
            m_unmaximizedPos = QPoint();
        }
    }
}

Qt::WindowFrameSection NMainWindow::getSection(const QPoint &pos)
{
    int x = pos.x();
    int y = pos.y();
    QRect r = rect();
    int left = r.left();
    int right = r.right();
    int top = r.top();
    int bottom = r.bottom();

    if ((x >= left && x < left + RESIZE_BORDER) && (y >= top && y < top + RESIZE_BORDER)) {
        return Qt::TopLeftSection;
    } else if ((x < right && x >= right - RESIZE_BORDER) && (y >= top && y < top + RESIZE_BORDER)) {
        return Qt::TopRightSection;
    } else if ((x < right && x >= right - RESIZE_BORDER) &&
               (y < bottom && y >= bottom - RESIZE_BORDER)) {
        return Qt::BottomRightSection;
    } else if ((x >= left && x < left + RESIZE_BORDER) &&
               (y < bottom && y >= bottom - RESIZE_BORDER)) {
        return Qt::BottomLeftSection;
    } else if (y >= top && y < top + RESIZE_BORDER) {
        return Qt::TopSection;
    } else if (x < right && x >= right - RESIZE_BORDER) {
        return Qt::RightSection;
    } else if (y < bottom && y >= bottom - RESIZE_BORDER) {
        return Qt::BottomSection;
    } else if (x >= left && x < left + RESIZE_BORDER) {
        return Qt::LeftSection;
    } else {
        return Qt::NoSection;
    }
}

void NMainWindow::updateCursor(Qt::WindowFrameSection section)
{
    switch (section) {
        case Qt::TopLeftSection:
        case Qt::BottomRightSection:
            setCursor(Qt::SizeFDiagCursor);
            break;
        case Qt::TopRightSection:
        case Qt::BottomLeftSection:
            setCursor(Qt::SizeBDiagCursor);
            break;
        case Qt::TopSection:
        case Qt::BottomSection:
            setCursor(Qt::SizeVerCursor);
            break;
        case Qt::RightSection:
        case Qt::LeftSection:
            setCursor(Qt::SizeHorCursor);
            break;
        default:
            setCursor(Qt::ArrowCursor);
    }
}

bool NMainWindow::event(QEvent *event)
{
    if (event->type() == QEvent::HoverMove && !m_dragActive) {
        QPoint pos = static_cast<QHoverEvent *>(event)->pos();
        if (!m_resizeActive) {
            m_resizeSection = getSection(pos);
            updateCursor(m_resizeSection);
            return true;
        }
    }

    return QDialog::event(event);
}

bool NMainWindow::eventFilter(QObject *, QEvent *event)
{
    if (event->type() == QEvent::MouseButtonPress) {
        m_dragActive = false;
        m_resizeActive = false;
    }

    return false;
}

void NMainWindow::mousePressEvent(QMouseEvent *event)
{
    activateWindow();
    m_dragActive = false;
    m_resizeActive = false;
    if (event->button() == Qt::LeftButton) {
        if (m_resizeSection != Qt::NoSection) {
            m_resizeActive = true;
            m_resizePoint = event->pos();
            m_resizeRect = rect();
        } else {
            m_dragActive = true;
            m_dragPoint = event->globalPos() - frameGeometry().topLeft();
        }
        event->accept();
    }
}

void NMainWindow::mouseMoveEvent(QMouseEvent *event)
{
    if ((event->buttons() & Qt::LeftButton) && !isMaximized()) {
        if (m_dragActive) {
            move(event->globalPos() - m_dragPoint);
            event->accept();
        } else if (m_resizeActive) {
            QRect g = geometry();
            QRect origR = geometry();
            QPoint pos = event->globalPos() - m_resizePoint;
            switch (m_resizeSection) {
                case Qt::TopLeftSection:
                    g.setTopLeft(pos + m_resizeRect.topLeft());
                    break;
                case Qt::TopRightSection:
                    g.setTopRight(pos + m_resizeRect.topRight());
                    break;
                case Qt::BottomRightSection:
                    g.setBottomRight(pos + m_resizeRect.bottomRight());
                    break;
                case Qt::BottomLeftSection:
                    g.setBottomLeft(pos + m_resizeRect.bottomLeft());
                    break;
                case Qt::TopSection:
                    g.setTop(pos.y() + m_resizeRect.top());
                    break;
                case Qt::RightSection:
                    g.setRight(pos.x() + m_resizeRect.right());
                    break;
                case Qt::BottomSection:
                    g.setBottom(pos.y() + m_resizeRect.bottom());
                    break;
                case Qt::LeftSection:
                    g.setLeft(pos.x() + m_resizeRect.left());
                    break;
                default:
                    break;
            }
            QSize min = QLayout::closestAcceptableSize(this, g.size());
            QRect desk = QApplication::desktop()->availableGeometry(this);
            if (min.width() > g.width() || min.height() > g.height() || desk.left() > g.left() ||
                desk.right() < g.right() || desk.top() > g.top() || desk.bottom() < g.bottom()) {
                switch (m_resizeSection) {
                    case Qt::TopLeftSection:
                    case Qt::TopSection:
                    case Qt::LeftSection:
                        if (min.width() > g.width()) {
                            g.setLeft(origR.left());
                        } else if (desk.left() > g.left()) {
                            g.setLeft(desk.left());
                        }
                        if (min.height() > g.height()) {
                            g.setTop(origR.top());
                        } else if (desk.top() > g.top()) {
                            g.setTop(desk.top());
                        }
                        break;
                    case Qt::TopRightSection:
                        if (min.width() > g.width()) {
                            g.setRight(origR.right());
                        } else if (desk.right() < g.right()) {
                            g.setRight(desk.right());
                        }
                        if (min.height() > g.height()) {
                            g.setTop(origR.top());
                        } else if (desk.top() > g.top()) {
                            g.setTop(desk.top());
                        }
                        break;
                    case Qt::BottomRightSection:
                    case Qt::BottomSection:
                    case Qt::RightSection:
                        if (min.width() > g.width()) {
                            g.setRight(origR.right());
                        } else if (desk.right() < g.right()) {
                            g.setRight(desk.right());
                        }
                        if (min.height() > g.height()) {
                            g.setBottom(origR.bottom());
                        } else if (desk.bottom() < g.bottom()) {
                            g.setBottom(desk.bottom());
                        }
                        break;
                    case Qt::BottomLeftSection:
                        if (min.width() > g.width()) {
                            g.setLeft(origR.left());
                        } else if (desk.left() > g.left()) {
                            g.setLeft(desk.left());
                        }
                        if (min.height() > g.height()) {
                            g.setBottom(origR.bottom());
                        } else if (desk.bottom() < g.bottom()) {
                            g.setBottom(desk.bottom());
                        }
                        break;
                    default:
                        break;
                }
            }
            setGeometry(g);
        }
    }
}

void NMainWindow::mouseReleaseEvent(QMouseEvent *event)
{
    m_resizeActive = false;
    m_dragActive = false;
    updateCursor(Qt::NoSection);

    QDialog::mouseReleaseEvent(event);
}

void NMainWindow::wheelEvent(QWheelEvent *event)
{
    QDialog::wheelEvent(event);

    if (event->orientation() == Qt::Vertical) {
        emit scrolled(event->delta());
    }
}

void NMainWindow::showPlaybackControls(bool enable)
{
    NSettings::instance()->setValue("ShowPlaybackControls", enable);
    emit showPlaybackControlsEnabled(enable);
}

void NMainWindow::resizeEvent(QResizeEvent *event)
{
    QDialog::resizeEvent(event);
    emit resized();
}

void NMainWindow::closeEvent(QCloseEvent *event)
{
    accept();
    QDialog::closeEvent(event);
    emit closed();
}

#ifdef Q_OS_WIN
bool _DwmIsCompositionEnabled()
{
    HMODULE library = LoadLibrary(L"dwmapi.dll");
    bool result = false;
    if (library) {
        BOOL enabled = false;
        // clang-format off
        HRESULT (WINAPI *pFn)(BOOL *enabled) = (HRESULT (WINAPI *)(BOOL *enabled))(GetProcAddress(library, "DwmIsCompositionEnabled"));
        // clang-format on
        result = SUCCEEDED(pFn(&enabled)) && enabled;
        FreeLibrary(library);
    }
    return result;
}

void NMainWindow::setFramelessShadow(bool enabled)
{
    if (enabled != m_framelessShadow) {
        m_framelessShadow = enabled;
        updateFramelessShadow();
    }
}

void NMainWindow::updateFramelessShadow()
{
    DWORD version = GetVersion();
    DWORD major = (DWORD)(LOBYTE(LOWORD(version))); // major = 6 for vista/7/2008

    if (_DwmIsCompositionEnabled() && m_framelessShadow && major == 6) {
        SetClassLongPtr((HWND)winId(), GCL_STYLE,
                        GetClassLongPtr((HWND)winId(), GCL_STYLE) | CS_DROPSHADOW);
    } else {
        SetClassLongPtr((HWND)winId(), GCL_STYLE,
                        GetClassLongPtr((HWND)winId(), GCL_STYLE) & ~CS_DROPSHADOW);
    }

    hide();
    show();
}

bool NMainWindow::nativeEvent(const QByteArray &eventType, void *message, long *result)
{
    MSG *msg = reinterpret_cast<MSG *>(message);
    if (msg->message == WM_DWMCOMPOSITIONCHANGED) {
        updateFramelessShadow();
        return true;
    } else {
        return NW7TaskBar::instance()->nativeEvent(eventType, message, result);
    }
}
#endif

bool NMainWindow::isOnTop()
{
#ifdef Q_OS_WIN
    DWORD dwExStyle = GetWindowLong((HWND)this->winId(), GWL_EXSTYLE);
    return (dwExStyle & WS_EX_TOPMOST);
#else
    Qt::WindowFlags flags = windowFlags();
    return (flags & Qt::WindowStaysOnTopHint);
#endif
}

void NMainWindow::setOnTop(bool onTop)
{
#ifdef Q_OS_WIN
    if (onTop)
        SetWindowPos((HWND)this->winId(), HWND_TOPMOST, 0, 0, 0, 0,
                     SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
    else
        SetWindowPos((HWND)this->winId(), HWND_NOTOPMOST, 0, 0, 0, 0,
                     SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
#else
    Qt::WindowFlags flags = windowFlags();
    if (onTop) {
        flags |= Qt::WindowStaysOnTopHint;
    } else {
        flags &= ~Qt::WindowStaysOnTopHint;
    }
    setWindowFlags(flags);
    show();
#endif

#ifdef Q_OS_WIN
    NW7TaskBar::instance()->setWindow(this);
#endif
}
