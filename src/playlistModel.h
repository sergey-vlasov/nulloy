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

#ifndef N_PLAYLIST_MODEL_H
#define N_PLAYLIST_MODEL_H

#include <QAbstractListModel>
#include <QList>
#include <QVariant>

class NPlaylistModel : public QAbstractListModel
{
    Q_OBJECT

    static unsigned int m_nextId;

public:
    struct DataItem
    {
        QString text{};
        unsigned int trackInfoFormatId{};
        QString filePath{};
        bool isFailed{};
        bool isSelected{};
        unsigned int durationSec{};
        qreal lastPosition{};
        unsigned int playbackCount{};
        unsigned int id{m_nextId++};
    };

    enum Role
    {
        DisplayRole = Qt::UserRole + 1,
        TrackInfoFormatIdRole,
        FilePathRole,
        DurationRole,
        LastPositionRole,
        PlaybackCountRole,
        IsFailedRole,
        IsCurrentRole,
        IsFocusedRole,
        IsHoveredRole,
        IsSelectedRole,
        RowRole,
        IdRole,
    };

    NPlaylistModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QHash<int, QByteArray> roleNames() const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;

    QVariant data(const QModelIndex &index, int role) const override;
    QVariant data(int row, int role) const;

    bool setData(const QModelIndex &index, const QVariant &value, int role) override;
    bool setRow(int row, const QVariant &value, int role);
    bool setRows(int startRow, int endRow, const QVariant &value, int role);
    bool setAll(const QVariant &value, int role);

    bool insertRow(const DataItem &data, int row);
    bool appenRow(const DataItem &data);

    bool moveRows(int startRow, int count, int destRow);
    void moveRows(const QList<int> &rows, int destRow);

    void removeRow(int row);
    void removeRows(const QList<int> &rows);
    void removeAll();

    int currentRow() const;
    void setCurrentRow(int row);

    int focusedRow() const;
    void setFocusedRow(int row);

private:
    QList<DataItem> m_data;
    QPersistentModelIndex m_currentIndex;
    QPersistentModelIndex m_focusedIndex;
    QPersistentModelIndex m_hoveredIndex;
};

#endif
