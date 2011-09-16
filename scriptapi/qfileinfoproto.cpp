/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright(c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which(including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "scriptapi_internal.h"

#include "qdirproto.h"
#include "qfileproto.h"
#include "qfileinfoproto.h"

QScriptValue QFileInfotoScriptValue(QScriptEngine *engine, QFileInfo* const &item)
{
  QVariant v;
  v.setValue(item);

  return engine->newVariant(v);
}

void QFileInfofromScriptValue(const QScriptValue &obj, QFileInfo* &item)
{ item = obj.toVariant().value<QFileInfo*>(); }

void setupQFileInfoProto(QScriptEngine *engine)
{
  qScriptRegisterMetaType(engine, QFileInfotoScriptValue, QFileInfofromScriptValue);

  QScriptValue proto = engine->newQObject(new QFileInfoProto(engine));
  engine->setDefaultPrototype(qMetaTypeId<QFileInfo*>(), proto);

  QScriptValue constructor = engine->newFunction(constructQFileInfo,
                                                 proto);
  engine->globalObject().setProperty("QFileInfo",  constructor, CTORPROPFLAGS);
}

QScriptValue constructQFileInfo(QScriptContext *context,
                                    QScriptEngine  *engine)
{
  QFileInfo *obj = 0;
  if (context->argumentCount() == 1 && context->argument(0).isString())
    obj = new QFileInfo(context->argument(0).toString());

  else if (context->argumentCount() == 1 &&
           qscriptvalue_cast<QFileInfo*>(context->argument(0)))
    obj = new QFileInfo(*(qscriptvalue_cast<QFileInfo*>(context->argument(0))));

  else if (context->argumentCount() == 1 &&
           qscriptvalue_cast<QFile*>(context->argument(0)))
    obj = new QFileInfo(*(qscriptvalue_cast<QFile*>(context->argument(0))));

  else if (context->argumentCount() == 2 &&
           // qscriptvalue_cast<QDir>(context->argument(0)) && // TODO: how do we check if this worked?
           context->argument(1).isString())
    obj = new QFileInfo(qscriptvalue_cast<QDir>(context->argument(0)),
                        context->argument(1).toString());
  else
    obj = new QFileInfo();

  return engine->toScriptValue(obj);
}

QFileInfoProto::QFileInfoProto(QObject *parent)
    : QObject(parent)
{
}

QDir QFileInfoProto::absoluteDir() const
{
  QFileInfo *item = qscriptvalue_cast<QFileInfo*>(thisObject());
  if (item)
    return item->absoluteDir();
  return QDir();
}

QString QFileInfoProto::absoluteFilePath() const
{
  QFileInfo *item = qscriptvalue_cast<QFileInfo*>(thisObject());
  if (item)
    return item->absoluteFilePath();
  return QString();
}

QString QFileInfoProto::absolutePath() const
{
  QFileInfo *item = qscriptvalue_cast<QFileInfo*>(thisObject());
  if (item)
    return item->absolutePath();
  return QString();
}

QString QFileInfoProto::baseName() const
{
  QFileInfo *item = qscriptvalue_cast<QFileInfo*>(thisObject());
  if (item)
    return item->baseName();
  return QString();
}

QString QFileInfoProto::bundleName() const
{
  QFileInfo *item = qscriptvalue_cast<QFileInfo*>(thisObject());
  if (item)
    return item->bundleName();
  return QString();
}

bool QFileInfoProto::caching() const
{
  QFileInfo *item = qscriptvalue_cast<QFileInfo*>(thisObject());
  if (item)
    return item->caching();
  return false;
}

QString QFileInfoProto::canonicalFilePath() const
{
  QFileInfo *item = qscriptvalue_cast<QFileInfo*>(thisObject());
  if (item)
    return item->canonicalFilePath();
  return QString();
}

QString QFileInfoProto::canonicalPath() const
{
  QFileInfo *item = qscriptvalue_cast<QFileInfo*>(thisObject());
  if (item)
    return item->canonicalPath();
  return QString();
}

QString QFileInfoProto::completeBaseName() const
{
  QFileInfo *item = qscriptvalue_cast<QFileInfo*>(thisObject());
  if (item)
    return item->completeBaseName();
  return QString();
}

QString QFileInfoProto::completeSuffix() const
{
  QFileInfo *item = qscriptvalue_cast<QFileInfo*>(thisObject());
  if (item)
    return item->completeSuffix();
  return QString();
}

QDateTime QFileInfoProto::created() const
{
  QFileInfo *item = qscriptvalue_cast<QFileInfo*>(thisObject());
  if (item)
    return item->created();
  return QDateTime();
}

QDir QFileInfoProto::dir() const
{
  QFileInfo *item = qscriptvalue_cast<QFileInfo*>(thisObject());
  if (item)
    return item->dir();
  return QDir();
}

bool QFileInfoProto::exists() const
{
  QFileInfo *item = qscriptvalue_cast<QFileInfo*>(thisObject());
  if (item)
    return item->exists();
  return false;
}

QString QFileInfoProto::fileName() const
{
  QFileInfo *item = qscriptvalue_cast<QFileInfo*>(thisObject());
  if (item)
    return item->fileName();
  return QString();
}

QString QFileInfoProto::filePath() const
{
  QFileInfo *item = qscriptvalue_cast<QFileInfo*>(thisObject());
  if (item)
    return item->filePath();
  return QString();
}

QString QFileInfoProto::group() const
{
  QFileInfo *item = qscriptvalue_cast<QFileInfo*>(thisObject());
  if (item)
    return item->group();
  return QString();
}

uint QFileInfoProto::groupId() const
{
  QFileInfo *item = qscriptvalue_cast<QFileInfo*>(thisObject());
  if (item)
    return item->groupId();
  return 0;
}

bool QFileInfoProto::isAbsolute() const
{
  QFileInfo *item = qscriptvalue_cast<QFileInfo*>(thisObject());
  if (item)
    return item->isAbsolute();
  return false;
}

bool QFileInfoProto::isBundle() const
{
  QFileInfo *item = qscriptvalue_cast<QFileInfo*>(thisObject());
  if (item)
    return item->isBundle();
  return false;
}

bool QFileInfoProto::isDir() const
{
  QFileInfo *item = qscriptvalue_cast<QFileInfo*>(thisObject());
  if (item)
    return item->isDir();
  return false;
}

bool QFileInfoProto::isExecutable() const
{
  QFileInfo *item = qscriptvalue_cast<QFileInfo*>(thisObject());
  if (item)
    return item->isExecutable();
  return false;
}

bool QFileInfoProto::isFile() const
{
  QFileInfo *item = qscriptvalue_cast<QFileInfo*>(thisObject());
  if (item)
    return item->isFile();
  return false;
}

bool QFileInfoProto::isHidden() const
{
  QFileInfo *item = qscriptvalue_cast<QFileInfo*>(thisObject());
  if (item)
    return item->isHidden();
  return false;
}

bool QFileInfoProto::isReadable() const
{
  QFileInfo *item = qscriptvalue_cast<QFileInfo*>(thisObject());
  if (item)
    return item->isReadable();
  return false;
}

bool QFileInfoProto::isRelative() const
{
  QFileInfo *item = qscriptvalue_cast<QFileInfo*>(thisObject());
  if (item)
    return item->isRelative();
  return false;
}

bool QFileInfoProto::isRoot() const
{
  QFileInfo *item = qscriptvalue_cast<QFileInfo*>(thisObject());
  if (item)
    return item->isRoot();
  return false;
}

bool QFileInfoProto::isSymLink() const
{
  QFileInfo *item = qscriptvalue_cast<QFileInfo*>(thisObject());
  if (item)
    return item->isSymLink();
  return false;
}

bool QFileInfoProto::isWritable() const
{
  QFileInfo *item = qscriptvalue_cast<QFileInfo*>(thisObject());
  if (item)
    return item->isWritable();
  return false;
}

QDateTime QFileInfoProto::lastModified() const
{
  QFileInfo *item = qscriptvalue_cast<QFileInfo*>(thisObject());
  if (item)
    return item->lastModified();
  return QDateTime();
}

QDateTime QFileInfoProto::lastRead() const
{
  QFileInfo *item = qscriptvalue_cast<QFileInfo*>(thisObject());
  if (item)
    return item->lastRead();
  return QDateTime();
}

bool QFileInfoProto::makeAbsolute()
{
  QFileInfo *item = qscriptvalue_cast<QFileInfo*>(thisObject());
  if (item)
    return item->makeAbsolute();
  return false;
}

QString QFileInfoProto::owner() const
{
  QFileInfo *item = qscriptvalue_cast<QFileInfo*>(thisObject());
  if (item)
    return item->owner();
  return QString();
}

uint QFileInfoProto::ownerId() const
{
  QFileInfo *item = qscriptvalue_cast<QFileInfo*>(thisObject());
  if (item)
    return item->ownerId();
  return 0;
}

QString QFileInfoProto::path() const
{
  QFileInfo *item = qscriptvalue_cast<QFileInfo*>(thisObject());
  if (item)
    return item->path();
  return QString();
}

bool QFileInfoProto::permission(QFile::Permissions permissions) const
{
  QFileInfo *item = qscriptvalue_cast<QFileInfo*>(thisObject());
  if (item)
    return item->permission(permissions);
  return false;
}

QFile::Permissions QFileInfoProto::permissions() const
{
  QFileInfo *item = qscriptvalue_cast<QFileInfo*>(thisObject());
  if (item)
    return item->permissions();
  return QFile::Permissions();
}

void QFileInfoProto::refresh()
{
  QFileInfo *item = qscriptvalue_cast<QFileInfo*>(thisObject());
  if (item)
    item->refresh();
}

void QFileInfoProto::setCaching(bool enable)
{
  QFileInfo *item = qscriptvalue_cast<QFileInfo*>(thisObject());
  if (item)
    item->setCaching(enable);
}

void QFileInfoProto::setFile(const QString & file)
{
  QFileInfo *item = qscriptvalue_cast<QFileInfo*>(thisObject());
  if (item)
    item->setFile(file);
}

void QFileInfoProto::setFile(const QFile & file)
{
  QFileInfo *item = qscriptvalue_cast<QFileInfo*>(thisObject());
  if (item)
    item->setFile(file);
}

void QFileInfoProto::setFile(const QDir & dir, const QString & file)
{
  QFileInfo *item = qscriptvalue_cast<QFileInfo*>(thisObject());
  if (item)
    item->setFile(dir, file);
}

qint64 QFileInfoProto::size() const
{
  QFileInfo *item = qscriptvalue_cast<QFileInfo*>(thisObject());
  if (item)
    return item->size();
  return 0;
}

QString QFileInfoProto::suffix() const
{
  QFileInfo *item = qscriptvalue_cast<QFileInfo*>(thisObject());
  if (item)
    return item->suffix();
  return QString();
}

QString QFileInfoProto::symLinkTarget() const
{
  QFileInfo *item = qscriptvalue_cast<QFileInfo*>(thisObject());
  if (item)
    return item->symLinkTarget();
  return QString();
}

/*
QString QFileInfoProto::toString() const
{
  QFileInfo *item = qscriptvalue_cast<QFileInfo*>(thisObject());
  if (item)
    return QString("QFileInfo()");
  return QString("QFileInfo(unknown)");
}
*/
