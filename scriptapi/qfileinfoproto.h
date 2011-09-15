/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which(including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef __QFILEINFOPROTO_H__
#define __QFILEINFOPROTO_H__

#include <QDateTime>
#include <QDir>
#include <QFileInfo>
#include <QObject>
#include <QString>
#include <QtScript>

Q_DECLARE_METATYPE(QFileInfo*)

void setupQFileInfoProto(QScriptEngine *engine);
QScriptValue constructQFileInfo(QScriptContext *context, QScriptEngine *engine);

class QFileInfoProto : public QObject, public QScriptable
{
  Q_OBJECT

  public:
    QFileInfoProto(QObject *parent);

    Q_INVOKABLE QDir            absoluteDir() const;
    Q_INVOKABLE QString         absoluteFilePath() const;
    Q_INVOKABLE QString         absolutePath() const;
    Q_INVOKABLE QString         baseName() const;
    Q_INVOKABLE QString         bundleName() const;
    Q_INVOKABLE bool            caching() const;
    Q_INVOKABLE QString         canonicalFilePath() const;
    Q_INVOKABLE QString         canonicalPath() const;
    Q_INVOKABLE QString         completeBaseName() const;
    Q_INVOKABLE QString         completeSuffix() const;
    Q_INVOKABLE QDateTime       created() const;
    Q_INVOKABLE QDir            dir() const;
    Q_INVOKABLE bool            exists() const;
    Q_INVOKABLE QString         fileName() const;
    Q_INVOKABLE QString         filePath() const;
    Q_INVOKABLE QString         group() const;
    Q_INVOKABLE uint            groupId() const;
    Q_INVOKABLE bool            isAbsolute() const;
    Q_INVOKABLE bool            isBundle() const;
    Q_INVOKABLE bool            isDir() const;
    Q_INVOKABLE bool            isExecutable() const;
    Q_INVOKABLE bool            isFile() const;
    Q_INVOKABLE bool            isHidden() const;
    Q_INVOKABLE bool            isReadable() const;
    Q_INVOKABLE bool            isRelative() const;
    Q_INVOKABLE bool            isRoot() const;
    Q_INVOKABLE bool            isSymLink() const;
    Q_INVOKABLE bool            isWritable() const;
    Q_INVOKABLE QDateTime       lastModified() const;
    Q_INVOKABLE QDateTime       lastRead() const;
    Q_INVOKABLE bool            makeAbsolute();
    Q_INVOKABLE QString         owner() const;
    Q_INVOKABLE uint            ownerId() const;
    Q_INVOKABLE QString         path() const;
    Q_INVOKABLE bool            permission(QFile::Permissions permissions) const;
    Q_INVOKABLE QFile::Permissions      permissions() const;
    Q_INVOKABLE void            refresh();
    Q_INVOKABLE void            setCaching(bool enable);
    Q_INVOKABLE void            setFile(const QString & file);
    Q_INVOKABLE void            setFile(const QFile & file);
    Q_INVOKABLE void            setFile(const QDir & dir, const QString & file);
    Q_INVOKABLE qint64          size() const;
    Q_INVOKABLE QString         suffix() const;
    Q_INVOKABLE QString         symLinkTarget() const;
    
};

#endif
