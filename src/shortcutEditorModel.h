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

#ifndef N_SHORTCUT_EDITOR_MODEL_H
#define N_SHORTCUT_EDITOR_MODEL_H

#include <QAbstractListModel>

class NAction;

class NShortcutEditorModel : public QAbstractListModel
{
    Q_OBJECT

public:
    enum Role
    {
        NameRole = Qt::UserRole + 1,
        DescriptionRole,
        ShortcutRole,
        GlobalShortcutRole,
    };

    NShortcutEditorModel(const QList<NAction *> &actionList, QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE void applyShortcuts();
    Q_INVOKABLE static QString appendShortcut(const QString &oldValue, int key, int modifiers);

private:
    QList<NAction *> m_actionList;
    QList<QString> m_shortcuts;
    QList<QString> m_globalShortcuts;

    static QString keyEventToString(int key, int modifiers);
};

#endif
