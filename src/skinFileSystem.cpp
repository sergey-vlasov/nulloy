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

#include "skinFileSystem.h"

#include <QtCore/private/qabstractfileengine_p.h>

#include <QByteArray>
#include <QDebug>
#include <QString>

static const char _prefix[] = "skin:";

QHash<QString, QByteArray> NSkinFileSystem::m_fileHash;
NSkinFileSystem *NSkinFileSystem::m_instance;

class NSkinFileEngine : public QAbstractFileEngine
{
public:
    NSkinFileEngine(const QByteArray &ba, const QString &fileName);
    bool open(QIODevice::OpenMode mode);
    bool close() { return true; }
    bool isSequential() const { return false; }
    qint64 pos() const { return m_pos; }
    qint64 read(char *data, qint64 maxlen);
    bool seek(qint64 offset);
    qint64 size() const { return m_bytes.size(); }
    QString fileName(FileName file) const;
    FileFlags fileFlags(FileFlags flags) const;

private:
    QByteArray m_bytes;
    qint64 m_pos;
    QString m_fileName;
};

NSkinFileSystem::NSkinFileSystem(QObject *parent) : QObject(parent)
{
    Q_ASSERT_X(!m_instance, "NSkinFileSystem", "NSkinFileSystem instance already exists.");

    m_instance = this;
}

NSkinFileSystem *NSkinFileSystem::instance()
{
    if (!m_instance) {
        m_instance = new NSkinFileSystem();
    }

    return m_instance;
}

QString NSkinFileSystem::prefix()
{
    return _prefix;
}

NSkinFileEngine::NSkinFileEngine(const QByteArray &ba, const QString &fileName)
{
    m_bytes = ba;
    m_fileName = fileName;
}

QAbstractFileEngine *NSkinFileSystem::create(const QString &fileName) const
{
    if (!fileName.startsWith(_prefix)) {
        return NULL;
    }

    QString key = fileName.mid(strlen(_prefix));
    if (m_fileHash.contains(key)) {
        return new NSkinFileEngine(m_fileHash.value(key), fileName);
    } else {
        return NULL;
    }
}

void NSkinFileSystem::addFile(const QString &filePath, const QByteArray &ba)
{
    m_fileHash[filePath] = ba;
}

QByteArray NSkinFileSystem::readFile(const QString &filePath)
{
    if (m_fileHash.contains(filePath)) {
        return m_fileHash.value(filePath);
    } else {
        qCritical() << "NSkinFileSystem: file not found:" << filePath;
        return QByteArray();
    }
}

bool NSkinFileEngine::open(QIODevice::OpenMode mode)
{
    if (mode & QIODevice::ReadOnly) {
        m_pos = 0;
        return true;
    } else {
        return false;
    }
}

qint64 NSkinFileEngine::read(char *data, qint64 maxlen)
{
    maxlen = qBound(qint64(0), m_bytes.size() - m_pos, maxlen);
    qCopy(m_bytes.constData() + m_pos, m_bytes.constData() + m_pos + maxlen, data);
    m_pos += maxlen;
    return maxlen;
}

bool NSkinFileEngine::seek(qint64 offset)
{
    if (offset < 0 || offset >= m_bytes.size()) {
        return false;
    }
    m_pos = offset;
    return true;
}

QString NSkinFileEngine::fileName(FileName file) const
{
    Q_UNUSED(file);
    return m_fileName;
}

QAbstractFileEngine::FileFlags NSkinFileEngine::fileFlags(QAbstractFileEngine::FileFlags flags) const
{
    flags |= WriteOwnerPerm;
    flags |= WriteUserPerm;
    flags |= WriteGroupPerm;
    flags |= WriteOtherPerm;
    flags |= LinkType;
    flags |= BundleType;
    flags |= DirectoryType;
    flags |= HiddenFlag;
    return flags;
}
