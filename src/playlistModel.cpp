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

#include "playlistModel.h"

unsigned int NPlaylistModel::m_nextId = 1000; // IDs below 1000 are reserved, 0 means invalid ID

NPlaylistModel::NPlaylistModel(QObject *parent) : QAbstractListModel(parent) {}

int NPlaylistModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return m_data.size();
}

QHash<int, QByteArray> NPlaylistModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[DisplayRole] = "text";
    roles[FilePathRole] = "filePath";
    roles[DurationRole] = "duration";
    roles[IsFailedRole] = "isFailed";
    roles[IsCurrentRole] = "isCurrent";
    roles[IsFocusedRole] = "isFocused";
    roles[IsSelectedRole] = "isSelected";
    roles[IsHoveredRole] = "isHovered";
    roles[RowRole] = "row";
    return roles;
}

QVariant NPlaylistModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= rowCount()) {
        return QVariant();
    }

    const DataItem &item = m_data.at(index.row());

    switch (role) {
        case DisplayRole:
            return item.text;
        case TrackInfoFormatIdRole:
            return item.trackInfoFormatId;
        case FilePathRole:
            return item.filePath;
        case IsFailedRole:
            return item.isFailed;
        case IsSelectedRole:
            return item.isSelected;
        case IsCurrentRole:
            return index == m_currentIndex;
        case IsFocusedRole:
            return index == m_focusedIndex;
        case IsHoveredRole:
            return index == m_hoveredIndex;
        case DurationRole:
            return item.durationSec;
        case LastPositionRole:
            return item.lastPosition;
        case PlaybackCountRole:
            return item.playbackCount;
        case RowRole:
            return index.row() + 1;
        case IdRole:
            return item.id;
        default:
            return QVariant();
    }
}

QVariant NPlaylistModel::data(int row, int role) const
{
    return data(createIndex(row, 0), role);
}

bool NPlaylistModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid() || index.row() >= rowCount()) {
        return false;
    }

    int row = index.row();
    DataItem &item = m_data[row];
    switch (role) {
        case DisplayRole:
            item.text = value.toString();
            break;
        case TrackInfoFormatIdRole:
            item.trackInfoFormatId = value.toInt();
            break;
        case FilePathRole:
            item.filePath = value.toString();
            break;
        case IsFailedRole:
            item.isFailed = value.toBool();
            break;
        case IsSelectedRole:
            item.isSelected = value.toBool();
            break;
        case IsCurrentRole: {
            QModelIndex oldIndex = m_currentIndex;
            m_currentIndex = QModelIndex();
            emit dataChanged(oldIndex, oldIndex, {role});
            if (value.toBool()) {
                m_currentIndex = index;
            }
            break;
        }
        case IsFocusedRole: {
            QModelIndex oldIndex = m_focusedIndex;
            m_focusedIndex = QModelIndex();
            emit dataChanged(oldIndex, oldIndex, {role});
            if (value.toBool()) {
                m_focusedIndex = index;
            }
            break;
        }
        case IsHoveredRole: {
            QModelIndex oldIndex = m_hoveredIndex;
            m_hoveredIndex = QModelIndex();
            emit dataChanged(oldIndex, oldIndex, {role});
            if (value.toBool()) {
                m_hoveredIndex = index;
            }
            break;
        }
        case DurationRole:
            item.durationSec = value.toInt();
            break;
        case LastPositionRole:
            item.lastPosition = value.toReal();
            break;
        case PlaybackCountRole:
            item.playbackCount = value.toInt();
            break;
        default:
            return false;
    }

    emit dataChanged(index, index, {role});
    return true;
}

bool NPlaylistModel::setRow(int row, const QVariant &value, int role)
{
    return setData(createIndex(row, 0), value, role);
}

bool NPlaylistModel::setRows(int startRow, int endRow, const QVariant &value, int role)
{
    if (startRow < 0 || endRow >= rowCount() || startRow > endRow || endRow < 0 ||
        endRow >= rowCount()) {
        return false;
    }

    for (int i = startRow; i <= endRow; ++i) {
        if (!setRow(i, value, role)) {
            return false;
        }
    }

    return true;
}

bool NPlaylistModel::setAll(const QVariant &value, int role)
{
    return setRows(0, rowCount() - 1, value, role);
}

Qt::ItemFlags NPlaylistModel::flags(const QModelIndex &index) const
{
    if (!index.isValid()) {
        return Qt::NoItemFlags;
    }

    return QAbstractListModel::flags(index);
}

bool NPlaylistModel::insertRow(const DataItem &data, int row)
{
    if (row < 0 || row > rowCount()) {
        return false;
    }

    beginInsertRows(QModelIndex(), row, row);
    m_data.insert(row, data);
    endInsertRows();

    return true;
}

bool NPlaylistModel::appenRow(const DataItem &data)
{
    return insertRow(data, rowCount());
}

bool NPlaylistModel::moveRows(int startRow, int count, int destRow)
{
    if (startRow < 0 || startRow + count - 1 >= rowCount() || destRow < 0 || destRow > rowCount() ||
        count <= 0) {
        return false;
    }

    if (!beginMoveRows(QModelIndex(), startRow, startRow + count - 1, QModelIndex(), destRow)) {
        return false;
    }

    int fromRow = startRow;
    if (destRow < startRow) {
        fromRow += count - 1;
    } else {
        --destRow;
    }

    while (count--) {
        m_data.move(fromRow, destRow);
    }

    endMoveRows();
    return true;
}

void NPlaylistModel::moveRows(const QList<int> &rows, int destRow)
{
    QList<QPersistentModelIndex> indexes;
    for (int row : rows) {
        indexes << createIndex(row, 0);
    }

    std::sort(indexes.begin(), indexes.end());
    if (destRow <= indexes.first().row()) {
        std::reverse(indexes.begin(), indexes.end());
    }

    int startRow = -1;
    int count = 0;
    for (const auto &index : indexes) {
        if (startRow == -1) {
            startRow = index.row();
            count = 1;
        } else if (startRow + count == index.row()) {
            ++count;
        } else {
            moveRows(startRow, count, destRow);
            startRow = index.row();
            count = 1;
        }
    }

    if (startRow != -1) {
        moveRows(startRow, count, destRow);
    }
}

void NPlaylistModel::removeRow(int row)
{
    if (row < 0 || row >= rowCount()) {
        return;
    }

    beginRemoveRows(QModelIndex(), row, row);
    m_data.removeAt(row);
    endRemoveRows();
}

void NPlaylistModel::removeRows(const QList<int> &rows)
{
    QList<QPersistentModelIndex> indexes;
    for (int row : rows) {
        indexes << createIndex(row, 0);
    }

    std::sort(indexes.begin(), indexes.end());
    for (const auto &index : indexes) {
        removeRow(index.row());
    }
}

void NPlaylistModel::removeAll()
{
    beginRemoveRows(QModelIndex(), 0, rowCount() - 1);
    m_data.clear();
    endRemoveRows();
}

int NPlaylistModel::currentRow() const
{
    return m_currentIndex.row();
}

void NPlaylistModel::setCurrentRow(int row)
{
    setRow(row, true, IsCurrentRole);
}

int NPlaylistModel::focusedRow() const
{
    return m_focusedIndex.row();
}

void NPlaylistModel::setFocusedRow(int row)
{
    setRow(row, true, IsFocusedRole);
}
