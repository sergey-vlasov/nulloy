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

#ifndef N_PLAYLIST_CONTROLLER_H
#define N_PLAYLIST_CONTROLLER_H

#include <QObject>
#include <QUrl>

class NPlaylistModel;

class NPlaylistController : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int rowsPerPage MEMBER m_rowsPerPage)

public:
    NPlaylistController(QObject *parent = nullptr);

    Q_INVOKABLE NPlaylistModel *model() const;

    Q_INVOKABLE void mousePress(int row, Qt::KeyboardModifiers modifiers);
    Q_INVOKABLE void mouseRelease(int row, Qt::KeyboardModifiers modifiers);
    Q_INVOKABLE void mouseEnter(int row);
    Q_INVOKABLE void mouseExit(int row);
    Q_INVOKABLE void mouseDoubleClick(int row);

    Q_INVOKABLE void keyPress(int key, Qt::KeyboardModifiers modifiers);
    Q_INVOKABLE void keyRelease(int key, Qt::KeyboardModifiers modifiers);

    Q_INVOKABLE void moveSelected(int destRow);
    Q_INVOKABLE void dropUrls(const QList<QUrl> &urls, int row);

private:
    QList<int> selectedRows() const;

signals:
    void rowActivated(int row);

private:
    NPlaylistModel *m_model;
    int m_rowsPerPage;
};
#endif
