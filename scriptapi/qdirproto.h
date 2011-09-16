/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright(c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which(including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef __QDIRPROTO_H__
#define __QDIRPROTO_H__

#include <QDateTime>
#include <QDir>
#include <QObject>
#include <QString>
#include <QtScript>

Q_DECLARE_METATYPE(QDir)
Q_DECLARE_METATYPE(QDir*)

Q_DECLARE_METATYPE(enum QDir::Filter);
Q_DECLARE_METATYPE(enum QDir::SortFlag);

void setupQDirProto(QScriptEngine *engine);
QScriptValue constructQDir(QScriptContext *context, QScriptEngine *engine);

class QDirProto : public QObject, public QScriptable
{
  Q_OBJECT

  public:
    QDirProto(QObject *parent);

    Q_INVOKABLE QString         absoluteFilePath(const QString &fileName) const;
    Q_INVOKABLE QString         absolutePath() const;
    Q_INVOKABLE QString         canonicalPath() const;
    Q_INVOKABLE bool            cd(const QString &dirName);
    Q_INVOKABLE bool            cdUp();
    Q_INVOKABLE uint            count() const;
    Q_INVOKABLE QString         dirName() const;
    Q_INVOKABLE QFileInfoList   entryInfoList(const QStringList &nameFilters, QDir::Filters filters = QDir::NoFilter, QDir::SortFlags sort = QDir::NoSort) const;
    Q_INVOKABLE QFileInfoList   entryInfoList(QDir::Filters filters = QDir::NoFilter, QDir::SortFlags sort = QDir::NoSort) const;
    Q_INVOKABLE QStringList     entryList(const QStringList &nameFilters, QDir::Filters filters = QDir::NoFilter, QDir::SortFlags sort = QDir::NoSort) const;
    Q_INVOKABLE QStringList     entryList(QDir::Filters filters = QDir::NoFilter, QDir::SortFlags sort = QDir::NoSort) const;
    Q_INVOKABLE bool            exists(const QString &name) const;
    Q_INVOKABLE bool            exists() const;
    Q_INVOKABLE QString         filePath(const QString &fileName) const;
    Q_INVOKABLE QDir::Filters   filter() const;
    Q_INVOKABLE bool            isAbsolute() const;
    Q_INVOKABLE bool            isReadable() const;
    Q_INVOKABLE bool            isRelative() const;
    Q_INVOKABLE bool            isRoot() const;
    Q_INVOKABLE bool            makeAbsolute();
    Q_INVOKABLE bool            mkdir(const QString &dirName) const;
    Q_INVOKABLE bool            mkpath(const QString &dirPath) const;
    Q_INVOKABLE QStringList     nameFilters() const;
    Q_INVOKABLE QString         path() const;
    Q_INVOKABLE void            refresh();
    Q_INVOKABLE QString         relativeFilePath(const QString &fileName) const;
    Q_INVOKABLE bool            remove(const QString &fileName);
    Q_INVOKABLE bool	        rename(const QString &oldName, const QString &newName);
    Q_INVOKABLE bool	        rmdir(const QString &dirName) const;
    Q_INVOKABLE bool	        rmpath(const QString &dirPath) const;
    Q_INVOKABLE void	        setFilter(QDir::Filters filters);
    Q_INVOKABLE void	        setNameFilters(const QStringList &nameFilters);
    Q_INVOKABLE void	        setPath(const QString &path);
    Q_INVOKABLE void	        setSorting(QDir::SortFlags sort);
    Q_INVOKABLE QDir::SortFlags	sorting() const;

};

#endif
