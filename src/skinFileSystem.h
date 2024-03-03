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

#ifndef N_SKIN_FILE_SYSTEM_H
#define N_SKIN_FILE_SYSTEM_H

//#include <QAbstractFileEngineHandler>
#include <QtCore/private/qabstractfileengine_p.h>

#include <QHash>

class QByteArray;
class QString;
// class QAbstractFileEngineHandler;

class NSkinFileSystem : public QObject, public QAbstractFileEngineHandler
{
    Q_OBJECT
public:
    QAbstractFileEngine *create(const QString &fileName) const;
    static void addFile(const QString &filePath, const QByteArray &ba);
    Q_INVOKABLE static QByteArray readFile(const QString &filePath);
    static QString prefix();
    static NSkinFileSystem *instance();

private:
    NSkinFileSystem(QObject *parent = nullptr);
    static QHash<QString, QByteArray> m_fileHash;
    static NSkinFileSystem *m_instance;
};

#endif
