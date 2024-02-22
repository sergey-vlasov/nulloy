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

#include "shortcutEditorModel.h"

#include "action.h"

NShortcutEditorModel::NShortcutEditorModel(const QList<NAction *> &actionList, QObject *parent)
    : QAbstractListModel(parent)
{
    m_actionList = actionList;
    for (int i = 0; i < m_actionList.size(); ++i) {
        NAction *action = m_actionList.at(i);

        {
            QList<QKeySequence> shortcut = action->shortcuts();
            QStringList shortcutStr;
            foreach (QKeySequence seq, shortcut) {
                shortcutStr << seq.toString();
            }

            m_shortcuts << shortcutStr.join(", ");
        }

        {
            QList<QKeySequence> shortcut = action->globalShortcuts();
            QStringList shortcutStr;
            foreach (QKeySequence seq, shortcut) {
                shortcutStr << seq.toString();
            }

            m_globalShortcuts << shortcutStr.join(", ");
        }
    }
}

int NShortcutEditorModel::rowCount(const QModelIndex &parent) const
{
    return m_actionList.count();
}

QVariant NShortcutEditorModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    switch (role) {
        case NameRole: {
            // also removes "..." at the end of shortcut text, if present:
            return m_actionList.at(index.row())->text().replace("...", "");
        }
        case DescriptionRole: {
            return m_actionList.at(index.row())->statusTip();
        }
        case ShortcutRole: {
            return m_shortcuts.at(index.row());
        }
        case GlobalShortcutRole: {
            return m_globalShortcuts.at(index.row());
        }
        default:
            return QVariant();
    }
}

QHash<int, QByteArray> NShortcutEditorModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[NameRole] = "name";
    roles[DescriptionRole] = "description";
    roles[ShortcutRole] = "shortcut";
    roles[GlobalShortcutRole] = "globalShortcut";
    return roles;
}

void NShortcutEditorModel::applyShortcuts()
{
    for (int i = 0; i < m_actionList.size(); ++i) {
        NAction *action = m_actionList.at(i);
        {
            QList<QKeySequence> keySequenes;
            QString shortcuts = m_shortcuts.at(i);
            if (!shortcuts.isEmpty()) {
                QStringList localsList = shortcuts.split(", ");
                foreach (QString str, localsList) {
                    keySequenes << QKeySequence(str);
                }
            }
            action->setShortcuts(keySequenes);
        }

        {
            QList<QKeySequence> keySequenes;
            QString shortcuts = m_globalShortcuts.at(i);
            if (!shortcuts.isEmpty()) {
                QStringList localsList = shortcuts.split(", ");
                foreach (QString str, localsList) {
                    keySequenes << QKeySequence(str);
                }
            }
            action->setGlobalShortcuts(keySequenes);
        }
    }
}

QString NShortcutEditorModel::keyEventToString(int key, int modifiers)
{
    QString seqStr = QKeySequence(key).toString();

    // clang-format off
    if (seqStr.isEmpty() ||
        key == Qt::Key_Control ||
        key == Qt::Key_Alt || key == Qt::Key_AltGr ||
        key == Qt::Key_Meta ||
        key == Qt::Key_Shift)
    {
        return "";
    }
    // clang-format on

    QStringList strSequence;
    if (modifiers & Qt::ControlModifier) {
        strSequence << "Ctrl";
    }
    if (modifiers & Qt::AltModifier) {
        strSequence << "Alt";
    }
    if (modifiers & Qt::ShiftModifier) {
        strSequence << "Shift";
    }
    if (modifiers & Qt::MetaModifier) {
        strSequence << "Meta";
    }

    return strSequence.join("+") + (strSequence.isEmpty() ? "" : "+") + seqStr;
}

Qt::ItemFlags NShortcutEditorModel::flags(const QModelIndex &index) const
{
    return Qt::ItemIsEditable | QAbstractListModel::flags(index);
}

bool NShortcutEditorModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid()) {
        return false;
    }

    switch (role) {
        case ShortcutRole:
            m_shortcuts[index.row()] = value.toString();
            break;
        case GlobalShortcutRole:
            m_globalShortcuts[index.row()] = value.toString();
            break;
        default:
            return false;
    }

    emit dataChanged(index, index);
    return true;
}

QString NShortcutEditorModel::appendShortcut(const QString &oldValue, int key, int modifiers)
{
    modifiers &= Qt::ControlModifier | Qt::ShiftModifier | Qt::AltModifier | Qt::MetaModifier;

    QString shortcut = keyEventToString(key, modifiers);
    if (shortcut == "") {
        return oldValue;
    }

    QString text = oldValue;
    if (text.split(", ").size() >= 3) {
        text = "";
    }

    if (!text.isEmpty()) {
        text += ", ";
    }

    return text + shortcut;
}
