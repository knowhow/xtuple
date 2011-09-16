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

/** \ingroup scriptapi
    \brief Expose QDir class to scriptiong

    \todo implement QDir::drives()
  */

static QScriptValue qdir_addSearchPath(QScriptContext *context,
                                       QScriptEngine * /*engine*/ )
{
  if (context->argumentCount() == 2 &&
      context->argument(0).isString() &&
      context->argument(1).isString())
    QDir::addSearchPath(context->argument(0).toString(),
                        context->argument(1).toString());
  else
    context->throwError(QScriptContext::UnknownError,
                        "Coule not find an appropriate QDir::addSearchPath()");

  return QScriptValue();
}

static QScriptValue qdir_cleanPath(QScriptContext *context,
                                   QScriptEngine * /*engine*/ )
{
  QString result;
  
  if (context->argumentCount() == 1 &&
      context->argument(0).isString())
    result = QDir::cleanPath(context->argument(0).toString());
  else
    context->throwError(QScriptContext::UnknownError,
                        "Could not find an appropriate QDir::cleanPath()");

  return QScriptValue(result);
}

static QScriptValue qdir_current(QScriptContext * /*context*/,
                                 QScriptEngine *engine )
{
  QVariant v;
  v.setValue<QDir*>(new QDir(QDir::current()));
  return engine->newVariant(v);
}

static QScriptValue qdir_currentPath(QScriptContext * /*context*/,
                                     QScriptEngine * /*engine*/ )
{
  QString result = QDir::currentPath();
  return QScriptValue(result);
}

static QScriptValue qdir_drives(QScriptContext * /*context*/,
                                QScriptEngine * /*engine*/ )
{
  /*
  QFileInfoList result = QDir::drives();
  return QScriptValue(result);
  */
  return QScriptValue();
}

static QScriptValue qdir_fromNativeSeparators(QScriptContext *context,
                                              QScriptEngine * /*engine*/ )
{
  QString result;
  
  if (context->argumentCount() == 1 &&
      context->argument(0).isString())
    result = QDir::fromNativeSeparators(context->argument(0).toString());
  else
    context->throwError(QScriptContext::UnknownError,
                        "Could not find an appropriate QDir::fromNativeSeparators()");
  return QScriptValue(result);
}

static QScriptValue qdir_home(QScriptContext * /*context*/,
                              QScriptEngine *engine)
{
  QVariant v;
  v.setValue<QDir*>(new QDir(QDir::home()));
  return engine->newVariant(v);
}

static QScriptValue qdir_homePath(QScriptContext * /*context*/,
                                  QScriptEngine *engine)
{
  QVariant v;
  v.setValue<QDir*>(new QDir(QDir::homePath()));
  return engine->newVariant(v);
}

static QScriptValue qdir_isAbsolutePath(QScriptContext *context,
                                        QScriptEngine * /*engine*/ )
{
  bool result = false;
  
  if (context->argumentCount() == 1 &&
      context->argument(0).isString())
    result = QDir::isAbsolutePath(context->argument(0).toString());
  else
    context->throwError(QScriptContext::UnknownError,
                        "Could not find an appropriate QDir::isAbsolutePath()");
  return QScriptValue(result);
}

static QScriptValue qdir_isRelativePath(QScriptContext *context,
                                        QScriptEngine * /*engine*/ )
{
  bool result = false;
  
  if (context->argumentCount() == 1 &&
      context->argument(0).isString())
    result = QDir::isRelativePath(context->argument(0).toString());
  else
    context->throwError(QScriptContext::UnknownError,
                        "Could not find an appropriate QDir::isiRelativePath()");
  return QScriptValue(result);
}

static QScriptValue qdir_match(QScriptContext *context,
                               QScriptEngine * /*engine*/ )
{
  bool result = false;
  
  if (context->argumentCount() == 2 &&
      context->argument(0).isString() &&
      context->argument(1).isString())
    result = QDir::match(context->argument(0).toString(),
                         context->argument(1).toString());
  else if (context->argumentCount() == 2 &&
           context->argument(0).isArray() &&
           context->argument(1).isString())
    result = QDir::match(context->argument(0).toVariant().toStringList(),
                         context->argument(1).toString());
  else
    context->throwError(QScriptContext::UnknownError,
                        "Could not find an appropriate QDir::match()");
  return QScriptValue(result);
}

static QScriptValue qdir_root(QScriptContext * /*context*/,
                              QScriptEngine *engine)
{
  QVariant v;
  v.setValue<QDir*>(new QDir(QDir::root()));
  return engine->newVariant(v);
}

static QScriptValue qdir_rootPath(QScriptContext * /*context*/,
                                  QScriptEngine *engine)
{
  QVariant v;
  v.setValue<QDir*>(new QDir(QDir::rootPath()));
  return engine->newVariant(v);
}

static QScriptValue qdir_searchPaths(QScriptContext *context,
                                     QScriptEngine *engine)
{
  QStringList result;
  
  if (context->argumentCount() == 1 &&
      context->argument(0).isString())
    result = QDir::searchPaths(context->argument(0).toString());
  else
    context->throwError(QScriptContext::UnknownError,
                        "Could not find an appropriate QDir::searchPaths()");
  return engine->newVariant(result);
}

static QScriptValue qdir_separator(QScriptContext * /*context*/,
                                   QScriptEngine * /*engine*/ )
{
  QChar result = QDir::separator();
  return QScriptValue(result);
}

static QScriptValue qdir_setCurrent(QScriptContext *context,
                                    QScriptEngine * /*engine*/ )
{
  bool result = false;
  
  if (context->argumentCount() == 1 &&
      context->argument(0).isString())
    result = QDir::setCurrent(context->argument(0).toString());
  else
    context->throwError(QScriptContext::UnknownError,
                        "Could not find an appropriate QDir::setCurrent()");
  return QScriptValue(result);
}

static QScriptValue qdir_setSearchPaths(QScriptContext *context,
                                        QScriptEngine * /*engine*/ )
{
  if (context->argumentCount() == 2 &&
      context->argument(0).isString() &&
      context->argument(1).isArray())
    QDir::setSearchPaths(context->argument(0).toString(),
                         context->argument(0).toVariant().toStringList());
  else
    context->throwError(QScriptContext::UnknownError,
                        "Could not find an appropriate QDir::setSearchPaths()");
  return QScriptValue();
}

static QScriptValue qdir_temp(QScriptContext * /*context*/,
                              QScriptEngine *engine)
{
  QVariant v;
  v.setValue<QDir*>(new QDir(QDir::temp()));
  return engine->newVariant(v);
}

static QScriptValue qdir_tempPath(QScriptContext * /*context*/,
                                  QScriptEngine * /*engine*/ )
{
  QString result = QDir::tempPath();
  return QScriptValue(result);
}

static QScriptValue qdir_toNativeSeparators(QScriptContext *context,
                                            QScriptEngine * /*engine*/ )
{
  QString result;
  if (context->argumentCount() == 1 &&
      context->argument(0).isString())
    result = QDir::toNativeSeparators(context->argument(0).toString());
  else
    context->throwError(QScriptContext::UnknownError,
                        "Could not find an appropriate "
                        "QDir::toNativeSeparators()");
  return QScriptValue(result);
}

void setupQDirProto(QScriptEngine *engine)
{
  QScriptValue proto = engine->newQObject(new QDirProto(engine));
  engine->setDefaultPrototype(qMetaTypeId<QDir>(),  proto);
  engine->setDefaultPrototype(qMetaTypeId<QDir*>(), proto);

  QScriptValue constructor = engine->newFunction(constructQDir, proto);
  engine->globalObject().setProperty("QDir",  constructor, CTORPROPFLAGS);

  constructor.setProperty("addSearchPath",        engine->newFunction(qdir_addSearchPath),       STATICPROPFLAGS);
  constructor.setProperty("cleanPath",            engine->newFunction(qdir_cleanPath),           STATICPROPFLAGS);
  constructor.setProperty("current",              engine->newFunction(qdir_current),             STATICPROPFLAGS);
  constructor.setProperty("currentPath",          engine->newFunction(qdir_currentPath),         STATICPROPFLAGS);
  constructor.setProperty("drives",               engine->newFunction(qdir_drives),              STATICPROPFLAGS);
  constructor.setProperty("fromNativeSeparators", engine->newFunction(qdir_fromNativeSeparators),STATICPROPFLAGS);
  constructor.setProperty("home",                 engine->newFunction(qdir_home),                STATICPROPFLAGS);
  constructor.setProperty("homePath",             engine->newFunction(qdir_homePath),            STATICPROPFLAGS);
  constructor.setProperty("isAbsolutePath",       engine->newFunction(qdir_isAbsolutePath),      STATICPROPFLAGS);
  constructor.setProperty("isRelativePath",       engine->newFunction(qdir_isRelativePath),      STATICPROPFLAGS);
  constructor.setProperty("match",                engine->newFunction(qdir_match),               STATICPROPFLAGS);
  constructor.setProperty("root",                 engine->newFunction(qdir_root),                STATICPROPFLAGS);
  constructor.setProperty("rootPath",             engine->newFunction(qdir_rootPath),            STATICPROPFLAGS);
  constructor.setProperty("searchPaths",          engine->newFunction(qdir_searchPaths),         STATICPROPFLAGS);
  constructor.setProperty("separator",            engine->newFunction(qdir_separator),           STATICPROPFLAGS);
  constructor.setProperty("setCurrent",           engine->newFunction(qdir_setCurrent),          STATICPROPFLAGS);
  constructor.setProperty("setSearchPaths",       engine->newFunction(qdir_setSearchPaths),      STATICPROPFLAGS);
  constructor.setProperty("temp",                 engine->newFunction(qdir_temp),                STATICPROPFLAGS);
  constructor.setProperty("tempPath",             engine->newFunction(qdir_tempPath),            STATICPROPFLAGS);
  constructor.setProperty("toNativeSeparators",   engine->newFunction(qdir_toNativeSeparators),  STATICPROPFLAGS);

  constructor.setProperty("Dirs",           QScriptValue(engine, QDir::Dirs),           ENUMPROPFLAGS);
  constructor.setProperty("AllDirs",        QScriptValue(engine, QDir::AllDirs),        ENUMPROPFLAGS);
  constructor.setProperty("Files",          QScriptValue(engine, QDir::Files),          ENUMPROPFLAGS);
  constructor.setProperty("Drives",         QScriptValue(engine, QDir::Drives),         ENUMPROPFLAGS);
  constructor.setProperty("NoSymLinks",     QScriptValue(engine, QDir::NoSymLinks),     ENUMPROPFLAGS);
  constructor.setProperty("NoDotAndDotDot", QScriptValue(engine, QDir::NoDotAndDotDot), ENUMPROPFLAGS);
  constructor.setProperty("AllEntries",     QScriptValue(engine, QDir::AllEntries),     ENUMPROPFLAGS);
  constructor.setProperty("Readable",       QScriptValue(engine, QDir::Readable),       ENUMPROPFLAGS);
  constructor.setProperty("Writable",       QScriptValue(engine, QDir::Writable),       ENUMPROPFLAGS);
  constructor.setProperty("Executable",     QScriptValue(engine, QDir::Executable),     ENUMPROPFLAGS);
  constructor.setProperty("Modified",       QScriptValue(engine, QDir::Modified),       ENUMPROPFLAGS);
  constructor.setProperty("Hidden",         QScriptValue(engine, QDir::Hidden),         ENUMPROPFLAGS);
  constructor.setProperty("System",         QScriptValue(engine, QDir::System),         ENUMPROPFLAGS);
  constructor.setProperty("CaseSensitive",  QScriptValue(engine, QDir::CaseSensitive),  ENUMPROPFLAGS);
  constructor.setProperty("Name",           QScriptValue(engine, QDir::Name),           ENUMPROPFLAGS);
  constructor.setProperty("Time",           QScriptValue(engine, QDir::Time),           ENUMPROPFLAGS);
  constructor.setProperty("Size",           QScriptValue(engine, QDir::Size),           ENUMPROPFLAGS);
  constructor.setProperty("Type",           QScriptValue(engine, QDir::Type),           ENUMPROPFLAGS);
  constructor.setProperty("Unsorted",       QScriptValue(engine, QDir::Unsorted),       ENUMPROPFLAGS);
  constructor.setProperty("NoSort",         QScriptValue(engine, QDir::NoSort),         ENUMPROPFLAGS);
  constructor.setProperty("DirsFirst",      QScriptValue(engine, QDir::DirsFirst),      ENUMPROPFLAGS);
  constructor.setProperty("DirsLast",       QScriptValue(engine, QDir::DirsLast),       ENUMPROPFLAGS);
  constructor.setProperty("Reversed",       QScriptValue(engine, QDir::Reversed),       ENUMPROPFLAGS);
  constructor.setProperty("IgnoreCase",     QScriptValue(engine, QDir::IgnoreCase),     ENUMPROPFLAGS);
  constructor.setProperty("LocaleAware",    QScriptValue(engine, QDir::LocaleAware),    ENUMPROPFLAGS);

}

QScriptValue constructQDir(QScriptContext *context, QScriptEngine  *engine)
{
  QDir *obj = 0;
  if (context->argumentCount() == 1 && context->argument(0).isString())
    obj = new QDir(context->argument(0).toString());
  else if (context->argumentCount() == 1 &&
           qscriptvalue_cast<QDir*>(context->argument(0)))
    obj = new QDir(*(qscriptvalue_cast<QDir*>(context->argument(0))));
  else if (context->argumentCount() >= 2 &&
           context->argument(0).isString() &&
           context->argument(1).isString())
  {
    obj = new QDir(context->argument(0).toString(), context->argument(1).toString());
    if (context->argumentCount() >= 3 &&
        context->argument(2).isNumber())
      obj->setSorting((QDir::SortFlags)(context->argument(2).toInt32()));
    if (context->argumentCount() >= 4 &&
        context->argument(3).isNumber())
      obj->setFilter((QDir::Filters)(context->argument(3).toInt32()));
  }
  else
    obj = new QDir();
  return engine->toScriptValue(obj);
}

QDirProto::QDirProto(QObject *parent)
    : QObject(parent)
{
}

QString QDirProto::absoluteFilePath(const QString &fileName) const
{
  QDir *item = qscriptvalue_cast<QDir*>(thisObject());
  if (item)
    return item->absoluteFilePath(fileName);
  else if (! thisObject().isNull())
    return thisObject().toVariant().value<QDir>().absoluteFilePath(fileName);
  return QString();
}

QString QDirProto::absolutePath() const
{
  QDir *item = qscriptvalue_cast<QDir*>(thisObject());
  if (item)
    return item->absolutePath();
  else if (! thisObject().isNull())
    return thisObject().toVariant().value<QDir>().absolutePath();
  return QString();
}

QString QDirProto::canonicalPath() const
{
  QDir *item = qscriptvalue_cast<QDir*>(thisObject());
  if (item)
    return item->canonicalPath();
  else if (! thisObject().isNull())
    return thisObject().toVariant().value<QDir>().canonicalPath();
  return QString();
}

bool QDirProto::cd(const QString &dirName)
{
  QDir *item = qscriptvalue_cast<QDir*>(thisObject());
  if (item)
    return item->cd(dirName);
  else if (! thisObject().isNull())
    return thisObject().toVariant().value<QDir>().cd(dirName);
  return false;
}

bool QDirProto::cdUp()
{
  QDir *item = qscriptvalue_cast<QDir*>(thisObject());
  if (item)
    return item->cdUp();
  else if (! thisObject().isNull())
    return thisObject().toVariant().value<QDir>().cdUp();
  return false;
}

uint QDirProto::count() const
{
  QDir *item = qscriptvalue_cast<QDir*>(thisObject());
  if (item)
    return item->count();
  else if (! thisObject().isNull())
    return thisObject().toVariant().value<QDir>().count();
  return 0;
}

QString QDirProto::dirName() const
{
  QDir *item = qscriptvalue_cast<QDir*>(thisObject());
  if (item)
    return item->dirName();
  else if (! thisObject().isNull())
    return thisObject().toVariant().value<QDir>().dirName();
  return QString();
}

QFileInfoList QDirProto::entryInfoList(const QStringList &nameFilters, QDir::Filters filters, QDir::SortFlags sort) const
{
  QDir *item = qscriptvalue_cast<QDir*>(thisObject());
  if (item)
    return item->entryInfoList(nameFilters, filters, sort);
  else if (! thisObject().isNull())
    return thisObject().toVariant().value<QDir>().entryInfoList(nameFilters, filters, sort);
  return QFileInfoList();
}

QFileInfoList QDirProto::entryInfoList(QDir::Filters filters, QDir::SortFlags sort) const
{
  QDir *item = qscriptvalue_cast<QDir*>(thisObject());
  if (item)
    return item->entryInfoList(filters, sort);
  else if (! thisObject().isNull())
    return thisObject().toVariant().value<QDir>().entryInfoList(filters, sort);
  return QFileInfoList();
}

QStringList QDirProto::entryList(const QStringList &nameFilters, QDir::Filters filters, QDir::SortFlags sort) const
{
  QDir *item = qscriptvalue_cast<QDir*>(thisObject());
  if (item)
    return item->entryList(nameFilters, filters, sort);
  else if (! thisObject().isNull())
    return thisObject().toVariant().value<QDir>().entryList(nameFilters, filters, sort);
  return QStringList();
}

QStringList QDirProto::entryList(QDir::Filters filters, QDir::SortFlags sort) const
{
  QDir *item = qscriptvalue_cast<QDir*>(thisObject());
  if (item)
    return item->entryList(filters, sort);
  else if (! thisObject().isNull())
    return thisObject().toVariant().value<QDir>().entryList(filters, sort);
  return QStringList();
}

bool QDirProto::exists(const QString &name) const
{
  QDir *item = qscriptvalue_cast<QDir*>(thisObject());
  if (item)
    return item->exists(name);
  else if (! thisObject().isNull())
    return thisObject().toVariant().value<QDir>().exists(name);
  return false;
}

bool QDirProto::exists() const
{
  QDir *item = qscriptvalue_cast<QDir*>(thisObject());
  if (item)
    return item->exists();
  else if (! thisObject().isNull())
    return thisObject().toVariant().value<QDir>().exists();
  return false;
}

QString QDirProto::filePath(const QString &fileName) const
{
  QDir *item = qscriptvalue_cast<QDir*>(thisObject());
  if (item)
    return item->filePath(fileName);
  else if (! thisObject().isNull())
    return thisObject().toVariant().value<QDir>().filePath(fileName);
  return QString();
}

QDir::Filters QDirProto::filter() const
{
  QDir *item = qscriptvalue_cast<QDir*>(thisObject());
  if (item)
    return item->filter();
  else if (! thisObject().isNull())
    return thisObject().toVariant().value<QDir>().filter();
  return QDir::Filters();
}

bool QDirProto::isAbsolute() const
{
  QDir *item = qscriptvalue_cast<QDir*>(thisObject());
  if (item)
    return item->isAbsolute();
  else if (! thisObject().isNull())
    return thisObject().toVariant().value<QDir>().isAbsolute();
  return false;
}

bool QDirProto::isReadable() const
{
  QDir *item = qscriptvalue_cast<QDir*>(thisObject());
  if (item)
    return item->isReadable();
  else if (! thisObject().isNull())
    return thisObject().toVariant().value<QDir>().isReadable();
  return false;
}

bool QDirProto::isRelative() const
{
  QDir *item = qscriptvalue_cast<QDir*>(thisObject());
  if (item)
    return item->isRelative();
  else if (! thisObject().isNull())
    return thisObject().toVariant().value<QDir>().isRelative();
  return false;
}

bool QDirProto::isRoot() const
{
  QDir *item = qscriptvalue_cast<QDir*>(thisObject());
  if (item)
    return item->isRoot();
  else if (! thisObject().isNull())
    return thisObject().toVariant().value<QDir>().isRoot();
  return false;
}

bool QDirProto::makeAbsolute()
{
  QDir *item = qscriptvalue_cast<QDir*>(thisObject());
  if (item)
    return item->makeAbsolute();
  else if (! thisObject().isNull())
    return thisObject().toVariant().value<QDir>().makeAbsolute();
  return false;
}

bool QDirProto::mkdir(const QString &dirName) const
{
  QDir *item = qscriptvalue_cast<QDir*>(thisObject());
  if (item)
    return item->mkdir(dirName);
  else if (! thisObject().isNull())
    return thisObject().toVariant().value<QDir>().mkdir(dirName);
  return false;
}

bool QDirProto::mkpath(const QString &dirPath) const
{
  QDir *item = qscriptvalue_cast<QDir*>(thisObject());
  if (item)
    return item->mkpath(dirPath);
  else if (! thisObject().isNull())
    return thisObject().toVariant().value<QDir>().mkpath(dirPath);
  return false;
}

QStringList QDirProto::nameFilters() const
{
  QDir *item = qscriptvalue_cast<QDir*>(thisObject());
  if (item)
    return item->nameFilters();
  else if (! thisObject().isNull())
    return thisObject().toVariant().value<QDir>().nameFilters();
  return QStringList();
}

QString QDirProto::path() const
{
  QDir *item = qscriptvalue_cast<QDir*>(thisObject());
  if (item)
    return item->path();
  else if (! thisObject().isNull())
    return thisObject().toVariant().value<QDir>().path();
  return QString();
}

void QDirProto::refresh()
{
  QDir *item = qscriptvalue_cast<QDir*>(thisObject());
  if (item)
    item->refresh();
  else if (! thisObject().isNull())
    thisObject().toVariant().value<QDir>().refresh();
}

QString QDirProto::relativeFilePath(const QString &fileName) const
{
  QDir *item = qscriptvalue_cast<QDir*>(thisObject());
  if (item)
    return item->relativeFilePath(fileName);
  else if (! thisObject().isNull())
    return thisObject().toVariant().value<QDir>().relativeFilePath(fileName);
  return QString();
}

bool QDirProto::remove(const QString &fileName)
{
  QDir *item = qscriptvalue_cast<QDir*>(thisObject());
  if (item)
    return item->remove(fileName);
  else if (! thisObject().isNull())
    return thisObject().toVariant().value<QDir>().remove(fileName);
  return false;
}

bool QDirProto::rename(const QString &oldName, const QString &newName)
{
  QDir *item = qscriptvalue_cast<QDir*>(thisObject());
  if (item)
    return item->rename(oldName, newName);
  else if (! thisObject().isNull())
    return thisObject().toVariant().value<QDir>().rename(oldName, newName);
  return false;
}

bool QDirProto::rmdir(const QString &dirName) const
{
  QDir *item = qscriptvalue_cast<QDir*>(thisObject());
  if (item)
    return item->rmdir(dirName);
  else if (! thisObject().isNull())
    return thisObject().toVariant().value<QDir>().rmdir(dirName);
  return false;
}

bool QDirProto::rmpath(const QString &dirPath) const
{
  QDir *item = qscriptvalue_cast<QDir*>(thisObject());
  if (item)
    return item->rmpath(dirPath);
  else if (! thisObject().isNull())
    return thisObject().toVariant().value<QDir>().rmpath(dirPath);
  return false;
}

void QDirProto::setFilter(QDir::Filters filters)
{
  QDir *item = qscriptvalue_cast<QDir*>(thisObject());
  if (item)
    item->setFilter(filters);
  else if (! thisObject().isNull())
    thisObject().toVariant().value<QDir>().setFilter(filters);
}

void QDirProto::setNameFilters(const QStringList &nameFilters)
{
  QDir *item = qscriptvalue_cast<QDir*>(thisObject());
  if (item)
    item->setNameFilters(nameFilters);
  else if (! thisObject().isNull())
    thisObject().toVariant().value<QDir>().setNameFilters(nameFilters);
}

void QDirProto::setPath(const QString &path)
{
  QDir *item = qscriptvalue_cast<QDir*>(thisObject());
  if (item)
    item->setPath(path);
  else if (! thisObject().isNull())
    thisObject().toVariant().value<QDir>().setPath(path);
}

void QDirProto::setSorting(QDir::SortFlags sort)
{
  QDir *item = qscriptvalue_cast<QDir*>(thisObject());
  if (item)
    item->setSorting(sort);
  else if (! thisObject().isNull())
    thisObject().toVariant().value<QDir>().setSorting(sort);
}

QDir::SortFlags QDirProto::sorting() const
{
  QDir *item = qscriptvalue_cast<QDir*>(thisObject());
  if (item)
    return item->sorting();
  else if (! thisObject().isNull())
    return thisObject().toVariant().value<QDir>().sorting();
  return QDir::SortFlags();
}

/*
QString QDirProto::toString() const
{
  QDir *item = qscriptvalue_cast<QDir*>(thisObject());
  if (item)
    return QString("QDir()");
  return QString("QDir(unknown)");
}
*/
