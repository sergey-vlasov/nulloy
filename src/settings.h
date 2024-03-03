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

#ifndef N_SETTINGS_H
#define N_SETTINGS_H

#include <QSettings>

class QVariant;
class QString;
class NAction;

class NSettings : public QSettings
{
    Q_OBJECT

private:
    static NSettings *m_instance;
    QList<NAction *> m_actionList;
    void initValue(const QString &key, const QVariant &defaultValue);
    NSettings(QObject *parent = nullptr);

public:
    ~NSettings();
    static NSettings *instance();

    void initShortcuts(QObject *instance);
    void saveShortcuts();
    QList<NAction *> shortcuts() const;

    Q_INVOKABLE QVariant value(const QString &key, const QVariant &defaultValue = QVariant()) const;
    Q_INVOKABLE void setValue(const QString &key, const QVariant &value);
    void remove(const QString &key);

signals:
    void valueChanged(const QString &key, const QVariant &value);
};

class NSettingsOverlay : public QObject
{
    Q_OBJECT

    QMap<QString, QVariant> m_map;

public:
    explicit NSettingsOverlay(QObject *parent = nullptr);
    Q_INVOKABLE void commit();

    Q_INVOKABLE QVariant value(const QString &key) const;
    Q_INVOKABLE void setValue(const QString &key, const QVariant &value);

signals:
    void committed();
};

#endif
