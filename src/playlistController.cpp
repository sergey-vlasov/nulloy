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

#include "playlistController.h"

#include "playlistModel.h"

#include <QKeyEvent>

NPlaylistController::NPlaylistController(QObject *parent) : QObject(parent)
{
    m_model = new NPlaylistModel(this);
    m_rowsPerPage = 1;
}

NPlaylistModel *NPlaylistController::model() const
{
    return m_model;
}

QList<int> NPlaylistController::selectedRows() const
{
    QList<int> selectedRows;
    for (int i = 0; i < m_model->rowCount(); ++i) {
        if (m_model->data(i, NPlaylistModel::IsSelectedRole).toBool()) {
            selectedRows << i;
        }
    }
    return selectedRows;
}

void NPlaylistController::mousePress(int row, Qt::KeyboardModifiers modifiers)
{
    if (!modifiers) {
        if (!m_model->data(row, NPlaylistModel::IsSelectedRole).toBool()) {
            m_model->setAll(false, NPlaylistModel::IsSelectedRole);
            m_model->setRow(row, true, NPlaylistModel::IsSelectedRole);
        }
    } else if (modifiers & Qt::ShiftModifier) {
        int oldFocusedIndex = m_model->focusedRow();
        if (oldFocusedIndex < row) {
            m_model->setRows(oldFocusedIndex, row, true, NPlaylistModel::IsSelectedRole);
        } else {
            m_model->setRows(row, oldFocusedIndex, true, NPlaylistModel::IsSelectedRole);
        }
    } else if (modifiers & Qt::ControlModifier) {
        m_model->setRow(row, !m_model->data(row, NPlaylistModel::IsSelectedRole).toBool(),
                        NPlaylistModel::IsSelectedRole);
    }

    m_model->setFocusedRow(row);
}

void NPlaylistController::mouseRelease(int row, Qt::KeyboardModifiers modifiers)
{
    if (modifiers) {
        return;
    }
    m_model->setAll(false, NPlaylistModel::IsSelectedRole);
    m_model->setRow(row, true, NPlaylistModel::IsSelectedRole);
}

void NPlaylistController::mouseEnter(int row)
{
    m_model->setRow(row, true, NPlaylistModel::IsHoveredRole);
}

void NPlaylistController::mouseExit(int row)
{
    m_model->setRow(row, false, NPlaylistModel::IsHoveredRole);
}

void NPlaylistController::mouseDoubleClick(int row)
{
    if (row == -1) {
        return;
    }
    emit rowActivated(row);
}

void NPlaylistController::moveSelected(int destRow)
{
    m_model->moveRows(selectedRows(), destRow);
}

void NPlaylistController::keyPress(int key, Qt::KeyboardModifiers modifiers)
{
    QKeyEvent keyEvent(QEvent::KeyPress, key, modifiers);
    int oldFocusedIndex = m_model->focusedRow();

    switch (key) {
        case Qt::Key_Up:
            m_model->setFocusedRow(qMax(0, oldFocusedIndex - 1));
            break;
        case Qt::Key_Down:
            m_model->setFocusedRow(qMin(m_model->rowCount() - 1, oldFocusedIndex + 1));
            break;
        case Qt::Key_Home:
            m_model->setFocusedRow(0);
            break;
        case Qt::Key_End:
            m_model->setFocusedRow(m_model->rowCount() - 1);
            break;
        case Qt::Key_PageUp:
            m_model->setFocusedRow(qMax(0, oldFocusedIndex - m_rowsPerPage));
            break;
        case Qt::Key_PageDown:
            m_model->setFocusedRow(qMin(m_model->rowCount() - 1, oldFocusedIndex + m_rowsPerPage));
            break;
    }

    if (keyEvent.matches(QKeySequence::SelectAll)) {
        m_model->setAll(true, NPlaylistModel::IsSelectedRole);
        return;
    }

    if (keyEvent.matches(QKeySequence::Delete)) {
        auto selectedRows = this->selectedRows();
        int firstSelectedRow = selectedRows.isEmpty() ? -1 : selectedRows.first();
        m_model->removeRows(selectedRows);
        m_model->setFocusedRow(qBound(0, firstSelectedRow, m_model->rowCount() - 1));
        m_model->setRow(m_model->focusedRow(), true, NPlaylistModel::IsSelectedRole);
        return;
    }

    if (!modifiers) {
        m_model->setAll(false, NPlaylistModel::IsSelectedRole);
        m_model->setRow(m_model->focusedRow(), true, NPlaylistModel::IsSelectedRole);
        return;
    } else if (modifiers & Qt::ShiftModifier) {
        int focusedRow = m_model->focusedRow();
        if (oldFocusedIndex < focusedRow) {
            m_model->setRows(oldFocusedIndex, focusedRow, true, NPlaylistModel::IsSelectedRole);
        } else {
            m_model->setRows(focusedRow, oldFocusedIndex, true, NPlaylistModel::IsSelectedRole);
        }
        return;
    } else if (modifiers & Qt::ControlModifier) {
        switch (key) {
            case Qt::Key_Space:
                m_model->setRow(m_model->focusedRow(),
                                !m_model->data(m_model->focusedRow(), NPlaylistModel::IsSelectedRole)
                                     .toBool(),
                                NPlaylistModel::IsSelectedRole);
                return;
        }
    }
}

void NPlaylistController::keyRelease(int key, Qt::KeyboardModifiers modifiers) {}

void NPlaylistController::dropUrls(const QList<QUrl> &urls, int row)
{
    for (const QUrl &url : urls) {
        NPlaylistModel::DataItem item;
        item.text = url.fileName();
        item.filePath = url.toLocalFile();
        row = qMin(row, m_model->rowCount());
        m_model->insertRow(item, row);
        ++row;
    }
}
