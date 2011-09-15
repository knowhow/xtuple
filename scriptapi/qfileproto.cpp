/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "scriptapi_internal.h"

#include "qfileproto.h"

/** \ingroup scriptapi
    \brief Script Exposure of QFile class.

    \todo implement QFile::setDecodingFunction(DecoderFn)
    \todo implement QFile::setEncodingFunction(EncoderFn)
  */

//static functions
static QScriptValue qfile_copy(QScriptContext *context, QScriptEngine * /*engine*/ )
{
  bool result = false;
  
  if (context->argumentCount() == 2 &&
      context->argument(0).isString() &&
      context->argument(1).isString())
    result = QFile::copy(context->argument(0).toString(), context->argument(1).toString());
  else
    context->throwError(QScriptContext::UnknownError, "Could not find an appropriate QFile::copy()");

  return QScriptValue(result);
}

static QScriptValue qfile_decodeName(QScriptContext *context, QScriptEngine * /*engine*/ )
{
  QString result;
  
  if (context->argumentCount() == 1 &&
      context->argument(0).isString())
    result = QFile::decodeName(context->argument(0).toString().toUtf8());
  else if (context->argumentCount() == 1 &&
      ! qscriptvalue_cast<QByteArray>(context->argument(0)).isEmpty())
    result = QFile::decodeName(qscriptvalue_cast<QByteArray>(context->argument(0)));
  else
    context->throwError(QScriptContext::UnknownError, "Could not find an appropriate QFile::decodeName()");

  return QScriptValue(result);
}

static QScriptValue qfile_encodeName(QScriptContext *context, QScriptEngine *engine)
{
  QByteArray result;
  
  if (context->argumentCount() == 1 &&
      context->argument(0).isString())
    result = QFile::encodeName(context->argument(0).toString());
  else
    context->throwError(QScriptContext::UnknownError, "Could not find an appropriate QFile::encodeName()");

  return engine->newVariant(result);
}

static QScriptValue qfile_exists(QScriptContext *context, QScriptEngine * /*engine*/)
{
  bool result = false;
  
  if (context->argumentCount() == 1 &&
      context->argument(0).isString())
    result = QFile::exists(context->argument(0).toString());
  else
    context->throwError(QScriptContext::UnknownError,
                        "Could not find an appropriate QFile::exists()");

  return QScriptValue(result);
}

static QScriptValue qfile_link(QScriptContext *context, QScriptEngine * /*engine*/ )
{
  bool result = false;
  
  if (context->argumentCount() == 2 &&
      context->argument(0).isString() &&
      context->argument(1).isString())
    result = QFile::link(context->argument(0).toString(),
                         context->argument(1).toString());
  else
    context->throwError(QScriptContext::UnknownError,
                        "Could not find an appropriate QFile::link()");

  return QScriptValue(result);
}

static QScriptValue qfile_permissions(QScriptContext *context, QScriptEngine * /*engine*/ )
{
  QFile::Permissions result;
  
  if (context->argumentCount() == 1 &&
      context->argument(0).isString())
    result = QFile::permissions(context->argument(0).toString());
  else
    context->throwError(QScriptContext::UnknownError,
                        "Could not find an appropriate QFile::permissions()");

  return QScriptValue(result);
}

static QScriptValue qfile_remove(QScriptContext *context, QScriptEngine * /*engine*/ )
{
  bool result = false;
  
  if (context->argumentCount() == 1 &&
      context->argument(0).isString())
    result = QFile::remove(context->argument(0).toString());
  else
    context->throwError(QScriptContext::UnknownError,
                        "Could not find an appropriate QFile::remove()");

  return QScriptValue(result);
}

static QScriptValue qfile_rename(QScriptContext *context, QScriptEngine * /*engine*/ )
{
  bool result = false;
  
  if (context->argumentCount() == 2 &&
      context->argument(0).isString() &&
      context->argument(1).isString())
    result = QFile::rename(context->argument(0).toString(),
                           context->argument(1).toString());
  else
    context->throwError(QScriptContext::UnknownError, "Could not find an appropriate QFile::rename()");

  return QScriptValue(result);
}

static QScriptValue qfile_resize(QScriptContext *context, QScriptEngine * /*engine*/ )
{
  bool result = false;
  
  if (context->argumentCount() == 2 &&
      context->argument(0).isString() &&
      context->argument(1).isNumber())
    result = QFile::resize(context->argument(0).toString(),
                           context->argument(1).toInteger());
  else
    context->throwError(QScriptContext::UnknownError,
                        "Could not find an appropriate QFile::resize()");

  return QScriptValue(result);
}

/*
static QScriptValue qfile_setDecodingFunction(DecoderFn function) { }
static QScriptValue qfile_setEncodingFunction(EncoderFn function) { }
*/

static QScriptValue qfile_setPermissions(QScriptContext *context, QScriptEngine * /*engine*/ )
{
  bool result = false;
  
  if (context->argumentCount() == 2 &&
      context->argument(0).isString() &&
      context->argument(1).isNumber())
    result = QFile::setPermissions(context->argument(0).toString(),
                                   (QFile::Permissions)(context->argument(1).toInt32()));
  else
    context->throwError(QScriptContext::UnknownError, "Could not find an appropriate QFile::setPermissions()");

  return QScriptValue(result);
}

static QScriptValue qfile_symLinkTarget(QScriptContext *context, QScriptEngine * /*engine*/ )
{
  QString result;
  if (context->argumentCount() == 1 && context->argument(0).isString())
    result = QFile::symLinkTarget(context->argument(0).toString());
  else
    context->throwError(QScriptContext::UnknownError, "Could not find an appropriate QFile::symLinkTarget()");

  return QScriptValue(result);
}

void setupQFileProto(QScriptEngine *engine)
{
  QScriptValue proto = engine->newQObject(new QFileProto(engine));
  engine->setDefaultPrototype(qMetaTypeId<QFile*>(), proto);

  QScriptValue constructor = engine->newFunction(constructQFile,
                                                 proto);
  engine->globalObject().setProperty("QFile",  constructor, CTORPROPFLAGS);

  constructor.setProperty("copy",               engine->newFunction(qfile_copy),               STATICPROPFLAGS);
  constructor.setProperty("decodeName",         engine->newFunction(qfile_decodeName),         STATICPROPFLAGS);
  constructor.setProperty("encodeName",         engine->newFunction(qfile_encodeName),         STATICPROPFLAGS);
  constructor.setProperty("exists",             engine->newFunction(qfile_exists),             STATICPROPFLAGS);
  constructor.setProperty("link",               engine->newFunction(qfile_link),               STATICPROPFLAGS);
  constructor.setProperty("permissions",        engine->newFunction(qfile_permissions),        STATICPROPFLAGS);
  constructor.setProperty("remove",             engine->newFunction(qfile_remove),             STATICPROPFLAGS);
  constructor.setProperty("rename",             engine->newFunction(qfile_rename),             STATICPROPFLAGS);
  constructor.setProperty("resize",             engine->newFunction(qfile_resize),             STATICPROPFLAGS);
  /*
  constructor.setProperty("setDecodingFunction",engine->newFunction(qfile_setDecodingFunction),STATICPROPFLAGS);
  constructor.setProperty("setEncodingFunction",engine->newFunction(qfile_setEncodingFunction),STATICPROPFLAGS);
  */
  constructor.setProperty("setPermissions",     engine->newFunction(qfile_setPermissions),     STATICPROPFLAGS);
  constructor.setProperty("symLinkTarget",      engine->newFunction(qfile_symLinkTarget),      STATICPROPFLAGS);

  constructor.setProperty("NoError",          QScriptValue(engine, QFile::NoError),         ENUMPROPFLAGS);
  constructor.setProperty("ReadError",        QScriptValue(engine, QFile::ReadError),       ENUMPROPFLAGS);
  constructor.setProperty("WriteError",       QScriptValue(engine, QFile::WriteError),      ENUMPROPFLAGS);
  constructor.setProperty("FatalError",       QScriptValue(engine, QFile::FatalError),      ENUMPROPFLAGS);
  constructor.setProperty("ResourceError",    QScriptValue(engine, QFile::ResourceError),   ENUMPROPFLAGS);
  constructor.setProperty("OpenError",        QScriptValue(engine, QFile::OpenError),       ENUMPROPFLAGS);
  constructor.setProperty("AbortError",       QScriptValue(engine, QFile::AbortError),      ENUMPROPFLAGS);
  constructor.setProperty("TimeOutError",     QScriptValue(engine, QFile::TimeOutError),    ENUMPROPFLAGS);
  constructor.setProperty("UnspecifiedError", QScriptValue(engine, QFile::UnspecifiedError),ENUMPROPFLAGS);
  constructor.setProperty("RemoveError",      QScriptValue(engine, QFile::RemoveError),     ENUMPROPFLAGS);
  constructor.setProperty("RenameError",      QScriptValue(engine, QFile::RenameError),     ENUMPROPFLAGS);
  constructor.setProperty("PositionError",    QScriptValue(engine, QFile::PositionError),   ENUMPROPFLAGS);
  constructor.setProperty("ResizeError",      QScriptValue(engine, QFile::ResizeError),     ENUMPROPFLAGS);
  constructor.setProperty("PermissionsError", QScriptValue(engine, QFile::PermissionsError),ENUMPROPFLAGS);
  constructor.setProperty("CopyError",        QScriptValue(engine, QFile::CopyError),       ENUMPROPFLAGS);
  constructor.setProperty("NoOptions",        QScriptValue(engine, QFile::NoOptions),       ENUMPROPFLAGS);
  constructor.setProperty("ReadOwner",        QScriptValue(engine, QFile::ReadOwner),       ENUMPROPFLAGS);
  constructor.setProperty("WriteOwner",       QScriptValue(engine, QFile::WriteOwner),      ENUMPROPFLAGS);
  constructor.setProperty("ExeOwner",         QScriptValue(engine, QFile::ExeOwner),        ENUMPROPFLAGS);
  constructor.setProperty("ReadUser",         QScriptValue(engine, QFile::ReadUser),        ENUMPROPFLAGS);
  constructor.setProperty("WriteUser",        QScriptValue(engine, QFile::WriteUser),       ENUMPROPFLAGS);
  constructor.setProperty("ExeUser",          QScriptValue(engine, QFile::ExeUser),         ENUMPROPFLAGS);
  constructor.setProperty("ReadGroup",        QScriptValue(engine, QFile::ReadGroup),       ENUMPROPFLAGS);
  constructor.setProperty("WriteGroup",       QScriptValue(engine, QFile::WriteGroup),      ENUMPROPFLAGS);
  constructor.setProperty("ExeGroup",         QScriptValue(engine, QFile::ExeGroup),        ENUMPROPFLAGS);
  constructor.setProperty("ReadOther",        QScriptValue(engine, QFile::ReadOther),       ENUMPROPFLAGS);
  constructor.setProperty("WriteOther",       QScriptValue(engine, QFile::WriteOther),      ENUMPROPFLAGS);
  constructor.setProperty("ExeOther",         QScriptValue(engine, QFile::ExeOther),        ENUMPROPFLAGS);

}

QScriptValue constructQFile(QScriptContext *context, QScriptEngine *engine)
{
  QFile *obj = 0;
  if (context->argumentCount() == 1 &&
      context->argument(0).isString())
    obj = new QFile(context->argument(0).toString());

  else if (context->argumentCount() == 1 &&
           context->argument(0).isQObject())
    obj = new QFile(context->argument(0).toQObject());

  else if (context->argumentCount() == 2 &&
           context->argument(0).isString() &&
           context->argument(1).isQObject())
    obj = new QFile(context->argument(0).toString(),
                    context->argument(1).toQObject());
  else
    context->throwError(QScriptContext::UnknownError,
                        "Could not find an appropriate QFile constructor");

  return engine->toScriptValue(obj);
}

QFileProto::QFileProto(QObject *parent)
    : QObject(parent)
{
}

bool QFileProto::atEnd() const
{
  QFile *item = qscriptvalue_cast<QFile*>(thisObject());
  if (item)
    return item->atEnd();
  return false;
}

qint64 QFileProto::bytesAvailable() const
{
  QFile *item = qscriptvalue_cast<QFile*>(thisObject());
  if (item)
    return item->bytesAvailable();
  return 0;
}

qint64 QFileProto::bytesToWrite() const
{
  QFile *item = qscriptvalue_cast<QFile*>(thisObject());
  if (item)
    return item->bytesToWrite();
  return 0;
}

bool QFileProto::canReadLine() const
{
  QFile *item = qscriptvalue_cast<QFile*>(thisObject());
  if (item)
    return item->canReadLine();
  return false;
}

void QFileProto::close()
{
  QFile *item = qscriptvalue_cast<QFile*>(thisObject());
  if (item)
    item->close();
}

bool QFileProto::copy(const QString &newName)
{
  QFile *item = qscriptvalue_cast<QFile*>(thisObject());
  if (item)
    return item->copy(newName);
  return false;
}

QFile::FileError QFileProto::error() const
{
  QFile *item = qscriptvalue_cast<QFile*>(thisObject());
  if (item)
    return item->error();
  return QFile::FileError();
}

QString QFileProto::errorString() const
{
  QFile *item = qscriptvalue_cast<QFile*>(thisObject());
  if (item)
    return item->errorString();
  return QString();
}

bool QFileProto::exists() const
{
  QFile *item = qscriptvalue_cast<QFile*>(thisObject());
  if (item)
    return item->exists();
  return false;
}

QString QFileProto::fileName() const
{
  QFile *item = qscriptvalue_cast<QFile*>(thisObject());
  if (item)
    return item->fileName();
  return QString();
}

bool QFileProto::flush()
{
  QFile *item = qscriptvalue_cast<QFile*>(thisObject());
  if (item)
    return item->flush();
  return false;
}

bool QFileProto::getChar(char *c)
{
  QFile *item = qscriptvalue_cast<QFile*>(thisObject());
  if (item)
    return item->getChar(c);
  return false;
}

int QFileProto::handle() const
{
  QFile *item = qscriptvalue_cast<QFile*>(thisObject());
  if (item)
    return item->handle();
  return 0;
}

bool QFileProto::isOpen() const
{
  QFile *item = qscriptvalue_cast<QFile*>(thisObject());
  if (item)
    return item->isOpen();
  return false;
}

bool QFileProto::isReadable() const
{
  QFile *item = qscriptvalue_cast<QFile*>(thisObject());
  if (item)
    return item->isReadable();
  return false;
}

bool QFileProto::isSequential() const
{
  QFile *item = qscriptvalue_cast<QFile*>(thisObject());
  if (item)
    return item->isSequential();
  return false;
}

bool QFileProto::isTextModeEnabled() const
{
  QFile *item = qscriptvalue_cast<QFile*>(thisObject());
  if (item)
    return item->isTextModeEnabled();
  return false;
}

bool QFileProto::isWritable() const
{
  QFile *item = qscriptvalue_cast<QFile*>(thisObject());
  if (item)
    return item->isWritable();
  return false;
}

bool QFileProto::link(const QString &linkName)
{
  QFile *item = qscriptvalue_cast<QFile*>(thisObject());
  if (item)
    return item->link(linkName);
  return false;
}

uchar *QFileProto::map(qint64 offset, qint64 size, QFile::MemoryMapFlags flags)
{
  QFile *item = qscriptvalue_cast<QFile*>(thisObject());
  if (item)
    return item->map(offset, size, flags);
  return 0;
}

bool QFileProto::open(FILE *fh, int mode)
{
  QIODevice::OpenModeFlag p;
  p = (enum QIODevice::OpenModeFlag)mode;
  QFile *item = qscriptvalue_cast<QFile*>(thisObject());
  if (item)
    return item->open(fh, p);
  return false;
}

bool QFileProto::open(int fd, int mode)
{
  QIODevice::OpenModeFlag p;
  p = (enum QIODevice::OpenModeFlag)mode;
  QFile *item = qscriptvalue_cast<QFile*>(thisObject());
  if (item)
    return item->open(fd, p);
  return false;
}

bool QFileProto::open(int mode)
{
  QIODevice::OpenModeFlag p;
  p = (enum QIODevice::OpenModeFlag)mode;
  QFile *item = qscriptvalue_cast<QFile*>(thisObject());
  if (item)
    return item->open(p);
  return false;
}

QIODevice::OpenMode QFileProto::openMode() const
{
  QFile *item = qscriptvalue_cast<QFile*>(thisObject());
  if (item)
    return item->openMode();
  return QIODevice::OpenMode();
}

qint64 QFileProto::peek(char *data, qint64 maxSize)
{
  QFile *item = qscriptvalue_cast<QFile*>(thisObject());
  if (item)
    return item->peek(data, maxSize);
  return 0;
}

QByteArray QFileProto::peek(qint64 maxSize)
{
  QFile *item = qscriptvalue_cast<QFile*>(thisObject());
  if (item)
    return item->peek(maxSize);
  return QByteArray();
}

QFile::Permissions QFileProto::permissions() const
{
  QFile *item = qscriptvalue_cast<QFile*>(thisObject());
  if (item)
    return item->permissions();
  return QFile::Permissions();
}

qint64 QFileProto::pos() const
{
  QFile *item = qscriptvalue_cast<QFile*>(thisObject());
  if (item)
    return item->pos();
  return 0;
}

bool QFileProto::putChar(char c)
{
  QFile *item = qscriptvalue_cast<QFile*>(thisObject());
  if (item)
    return item->putChar(c);
  return false;
}

qint64 QFileProto::read(char *data, qint64 maxSize)
{
  QFile *item = qscriptvalue_cast<QFile*>(thisObject());
  if (item)
    return item->read(data, maxSize);
  return 0;
}

QByteArray QFileProto::read(qint64 maxSize)
{
  QFile *item = qscriptvalue_cast<QFile*>(thisObject());
  if (item)
    return item->read(maxSize);
  return QByteArray();
}

QByteArray QFileProto::readAll()
{
  QFile *item = qscriptvalue_cast<QFile*>(thisObject());
  if (item)
    return item->readAll();
  return QByteArray();
}

qint64 QFileProto::readLine(char *data, qint64 maxSize)
{
  QFile *item = qscriptvalue_cast<QFile*>(thisObject());
  if (item)
    return item->readLine(data, maxSize);
  return 0;
}

QByteArray QFileProto::readLine(qint64 maxSize)
{
  QFile *item = qscriptvalue_cast<QFile*>(thisObject());
  if (item)
    return item->readLine(maxSize);
  return QByteArray();
}

bool QFileProto::remove()
{
  QFile *item = qscriptvalue_cast<QFile*>(thisObject());
  if (item)
    return item->remove();
  return false;
}

bool QFileProto::rename(const QString &newName)
{
  QFile *item = qscriptvalue_cast<QFile*>(thisObject());
  if (item)
    return item->rename(newName);
  return false;
}

bool QFileProto::reset()
{
  QFile *item = qscriptvalue_cast<QFile*>(thisObject());
  if (item)
    return item->reset();
  return false;
}

bool QFileProto::resize(qint64 sz)
{
  QFile *item = qscriptvalue_cast<QFile*>(thisObject());
  if (item)
    return item->resize(sz);
  return false;
}

bool QFileProto::seek(qint64 pos)
{
  QFile *item = qscriptvalue_cast<QFile*>(thisObject());
  if (item)
    return item->seek(pos);
  return false;
}

void QFileProto::setFileName(const QString &name)
{
  QFile *item = qscriptvalue_cast<QFile*>(thisObject());
  if (item)
    item->setFileName(name);
}

bool QFileProto::setPermissions(QFile::Permissions permissions)
{
  QFile *item = qscriptvalue_cast<QFile*>(thisObject());
  if (item)
    return item->setPermissions(permissions);
  return false;
}

void QFileProto::setTextModeEnabled(bool enabled)
{
  QFile *item = qscriptvalue_cast<QFile*>(thisObject());
  if (item)
    item->setTextModeEnabled(enabled);
}

qint64 QFileProto::size() const
{
  QFile *item = qscriptvalue_cast<QFile*>(thisObject());
  if (item)
    return item->size();
  return 0;
}

QString QFileProto::symLinkTarget() const
{
  QFile *item = qscriptvalue_cast<QFile*>(thisObject());
  if (item)
    return item->symLinkTarget();
  return QString();
}

void QFileProto::ungetChar(char c)
{
  QFile *item = qscriptvalue_cast<QFile*>(thisObject());
  if (item)
    item->ungetChar(c);
}

bool QFileProto::unmap(uchar *address)
{
  QFile *item = qscriptvalue_cast<QFile*>(thisObject());
  if (item)
    return item->unmap(address);
  return false;
}

void QFileProto::unsetError()
{
  QFile *item = qscriptvalue_cast<QFile*>(thisObject());
  if (item)
    item->unsetError();
}

bool QFileProto::waitForBytesWritten(int msecs)
{
  QFile *item = qscriptvalue_cast<QFile*>(thisObject());
  if (item)
    return item->waitForBytesWritten(msecs);
  return false;
}

bool QFileProto::waitForReadyRead(int msecs)
{
  QFile *item = qscriptvalue_cast<QFile*>(thisObject());
  if (item)
    return item->waitForReadyRead(msecs);
  return false;
}

qint64 QFileProto::write(const QByteArray &byteArray)
{
  QFile *item = qscriptvalue_cast<QFile*>(thisObject());
  if (item)
    return item->write(byteArray);
  return 0;
}

qint64 QFileProto::write(const char *data)
{
  QFile *item = qscriptvalue_cast<QFile*>(thisObject());
  if (item)
    return item->write(data);
  return 0;
}

qint64 QFileProto::write(const char *data, qint64 maxSize)
{
  QFile *item = qscriptvalue_cast<QFile*>(thisObject());
  if (item)
    return item->write(data, maxSize);
  return 0;
}

/*
QString QFileProto::toString() const
{
  QFile *item = qscriptvalue_cast<QFile*>(thisObject());
  if (item)
    return QString("QFile()");
  return QString("QFile(unknown)");
}
*/
