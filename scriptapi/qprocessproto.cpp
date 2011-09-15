/*
 *This file is part of the xTuple ERP: PostBooks Edition, a free and
 *open source Enterprise Resource Planning software suite,
 *Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 *It is licensed to you under the Common Public Attribution License
 *version 1.0, the full text of which(including xTuple-specific Exhibits)
 *is available at www.xtuple.com/CPAL.  By using this software, you agree
 *to be bound by its terms.
 */

#include "qprocessproto.h"

/** \ingroup scriptapi
    \class QProcessProto
    \brief This class exposes the QIODevice class to Qt Scripting.

    The QProcessProto class exposes as much of the API to the
    QProcess class as possible.
*/

QScriptValue QProcesstoScriptValue(QScriptEngine *engine, QProcess* const &item)
{ return engine->newQObject(item); }

void QProcessfromScriptValue(const QScriptValue &obj, QProcess* &item)
{ item = qobject_cast<QProcess*>(obj.toQObject()); }

void ExitStatusfromScriptValue(const QScriptValue &obj, enum QProcess::ExitStatus &p)
{ p = (enum QProcess::ExitStatus)obj.toInt32(); }
QScriptValue ExitStatustoScriptValue(QScriptEngine *engine, const enum QProcess::ExitStatus &p)
{ return QScriptValue(engine, (int)p); }

void ProcessChannelfromScriptValue(const QScriptValue &obj, enum QProcess::ProcessChannel &p)
{ p = (enum QProcess::ProcessChannel)obj.toInt32(); }
QScriptValue ProcessChanneltoScriptValue(QScriptEngine *engine, const enum QProcess::ProcessChannel &p)
{ return QScriptValue(engine, (int)p); }

void ProcessChannelModefromScriptValue(const QScriptValue &obj, enum QProcess::ProcessChannelMode &p)
{ p = (enum QProcess::ProcessChannelMode)obj.toInt32(); }
QScriptValue ProcessChannelModetoScriptValue(QScriptEngine *engine, const enum QProcess::ProcessChannelMode &p)
{ return QScriptValue(engine, (int)p); }

void ProcessErrorfromScriptValue(const QScriptValue &obj, enum QProcess::ProcessError &p)
{ p = (enum QProcess::ProcessError)obj.toInt32(); }
QScriptValue ProcessErrortoScriptValue(QScriptEngine *engine, const enum QProcess::ProcessError &p)
{ return QScriptValue(engine, (int)p); }

void ProcessStatefromScriptValue(const QScriptValue &obj, enum QProcess::ProcessState &p)
{ p = (enum QProcess::ProcessState)obj.toInt32(); }
QScriptValue ProcessStatetoScriptValue(QScriptEngine *engine, const enum QProcess::ProcessState &p)
{ return QScriptValue(engine, (int)p); }

void setupQProcessProto(QScriptEngine *engine)
{
  qScriptRegisterMetaType(engine, QProcesstoScriptValue, QProcessfromScriptValue);

  QScriptValue proto = engine->newQObject(new QProcessProto(engine));
  engine->setDefaultPrototype(qMetaTypeId<QProcess*>(), proto);

  QScriptValue constructor = engine->newFunction(constructQProcess,
                                                 proto);
  engine->globalObject().setProperty("QProcess",  constructor);

  qScriptRegisterMetaType(engine, ExitStatustoScriptValue, ExitStatusfromScriptValue);
  constructor.setProperty("NormalExit", QScriptValue(engine, QProcess::NormalExit), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  constructor.setProperty("CrashExit", QScriptValue(engine, QProcess::CrashExit), QScriptValue::ReadOnly | QScriptValue::Undeletable);

  qScriptRegisterMetaType(engine, ProcessChanneltoScriptValue, ProcessChannelfromScriptValue);
  constructor.setProperty("StandardOutput", QScriptValue(engine, QProcess::StandardOutput), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  constructor.setProperty("StandardError", QScriptValue(engine, QProcess::StandardError), QScriptValue::ReadOnly | QScriptValue::Undeletable);

  qScriptRegisterMetaType(engine, ProcessChannelModetoScriptValue, ProcessChannelModefromScriptValue);
  constructor.setProperty("SeparateChannels", QScriptValue(engine, QProcess::SeparateChannels), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  constructor.setProperty("MergedChannels", QScriptValue(engine, QProcess::MergedChannels), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  constructor.setProperty("ForwardedChannels", QScriptValue(engine, QProcess::ForwardedChannels), QScriptValue::ReadOnly | QScriptValue::Undeletable);

  qScriptRegisterMetaType(engine, ProcessErrortoScriptValue, ProcessErrorfromScriptValue);
  constructor.setProperty("FailedToStart", QScriptValue(engine, QProcess::FailedToStart), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  constructor.setProperty("Crashed", QScriptValue(engine, QProcess::Crashed), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  constructor.setProperty("Timedout", QScriptValue(engine, QProcess::Timedout), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  constructor.setProperty("WriteError", QScriptValue(engine, QProcess::WriteError), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  constructor.setProperty("ReadError", QScriptValue(engine, QProcess::ReadError), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  constructor.setProperty("UnknownError", QScriptValue(engine, QProcess::UnknownError), QScriptValue::ReadOnly | QScriptValue::Undeletable);

  qScriptRegisterMetaType(engine, ProcessStatetoScriptValue, ProcessStatefromScriptValue);
  constructor.setProperty("NotRunning", QScriptValue(engine, QProcess::NotRunning), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  constructor.setProperty("Starting", QScriptValue(engine, QProcess::Starting), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  constructor.setProperty("Running", QScriptValue(engine, QProcess::Running), QScriptValue::ReadOnly | QScriptValue::Undeletable);

}

Q_DECLARE_METATYPE(enum QProcess::ExitStatus);
Q_DECLARE_METATYPE(enum QProcess::ProcessChannel);
Q_DECLARE_METATYPE(enum QProcess::ProcessChannelMode);
Q_DECLARE_METATYPE(enum QProcess::ProcessError);
Q_DECLARE_METATYPE(enum QProcess::ProcessState);

QScriptValue constructQProcess(QScriptContext *context, QScriptEngine *engine)
{
  QProcess *obj = 0;
  if (context->argumentCount() == 0)
    obj = new QProcess();
  else if (context->argumentCount() == 1 &&
      qscriptvalue_cast<QObject*>(context->argument(0)))
    obj = new QProcess(qscriptvalue_cast<QObject*>(context->argument(0)));
  else
     context->throwError(QScriptContext::UnknownError,
                         QString("Could not find an appropriate QProcess constructor to call"));

  return engine->toScriptValue(obj);
}

QProcessProto::QProcessProto(QObject *parent)
    : QObject(parent)
{
}

bool QProcessProto::atEnd() const
{
  QProcess *item = qscriptvalue_cast<QProcess*>(thisObject());
  if (item)
    return item->atEnd();
  return false;
}

qint64 QProcessProto::bytesAvailable() const
{
  QProcess *item = qscriptvalue_cast<QProcess*>(thisObject());
  if (item)
    return item->bytesAvailable();
  return 0;
}

qint64 QProcessProto::bytesToWrite() const
{
  QProcess *item = qscriptvalue_cast<QProcess*>(thisObject());
  if (item)
    return item->bytesToWrite();
  return 0;
}

bool QProcessProto::canReadLine() const
{
  QProcess *item = qscriptvalue_cast<QProcess*>(thisObject());
  if (item)
    return item->canReadLine();
  return false;
}

void QProcessProto::close()
{
  QProcess *item = qscriptvalue_cast<QProcess*>(thisObject());
  if (item)
    item->close();
}

void QProcessProto::closeReadChannel(QProcess::ProcessChannel channel)
{
  QProcess *item = qscriptvalue_cast<QProcess*>(thisObject());
  if (item)
    item->closeReadChannel(channel);
}

void QProcessProto::closeWriteChannel()
{
  QProcess *item = qscriptvalue_cast<QProcess*>(thisObject());
  if (item)
    item->closeWriteChannel();
}

QProcess::ProcessError QProcessProto::error() const
{
  QProcess *item = qscriptvalue_cast<QProcess*>(thisObject());
  if (item)
    return item->error();
  return QProcess::ProcessError();
}

QString QProcessProto::errorString() const
{
  QProcess *item = qscriptvalue_cast<QProcess*>(thisObject());
  if (item)
    return item->errorString();
  return QString();
}

int QProcessProto::exitCode() const
{
  QProcess *item = qscriptvalue_cast<QProcess*>(thisObject());
  if (item)
    return item->exitCode();
  return 0;
}

QProcess::ExitStatus QProcessProto::exitStatus() const
{
  QProcess *item = qscriptvalue_cast<QProcess*>(thisObject());
  if (item)
    return item->exitStatus();
  return QProcess::ExitStatus();
}

bool QProcessProto::getChar(char *c)
{
  QProcess *item = qscriptvalue_cast<QProcess*>(thisObject());
  if (item)
    return item->getChar(c);
  return false;
}

bool QProcessProto::isOpen() const
{
  QProcess *item = qscriptvalue_cast<QProcess*>(thisObject());
  if (item)
    return item->isOpen();
  return false;
}

bool QProcessProto::isReadable() const
{
  QProcess *item = qscriptvalue_cast<QProcess*>(thisObject());
  if (item)
    return item->isReadable();
  return false;
}

bool QProcessProto::isSequential() const
{
  QProcess *item = qscriptvalue_cast<QProcess*>(thisObject());
  if (item)
    return item->isSequential();
  return false;
}

bool QProcessProto::isTextModeEnabled() const
{
  QProcess *item = qscriptvalue_cast<QProcess*>(thisObject());
  if (item)
    return item->isTextModeEnabled();
  return false;
}

bool QProcessProto::isWritable() const
{
  QProcess *item = qscriptvalue_cast<QProcess*>(thisObject());
  if (item)
    return item->isWritable();
  return false;
}

bool QProcessProto::open(QIODevice::OpenMode mode)
{
  QProcess *item = qscriptvalue_cast<QProcess*>(thisObject());
  if (item)
    return item->open(mode);
  return false;
}

QIODevice::OpenMode QProcessProto::openMode() const
{
  QProcess *item = qscriptvalue_cast<QProcess*>(thisObject());
  if (item)
    return item->openMode();
  return QIODevice::OpenMode();
}

qint64 QProcessProto::peek(char *data, qint64 maxSize)
{
  QProcess *item = qscriptvalue_cast<QProcess*>(thisObject());
  if (item)
    return item->peek(data, maxSize);
  return 0;
}

QByteArray QProcessProto::peek(qint64 maxSize)
{
  QProcess *item = qscriptvalue_cast<QProcess*>(thisObject());
  if (item)
    return item->peek(maxSize);
  return QByteArray();
}

Q_PID QProcessProto::pid() const
{
  QProcess *item = qscriptvalue_cast<QProcess*>(thisObject());
  if (item)
    return item->pid();
  return Q_PID();
}

qint64 QProcessProto::pos() const
{
  QProcess *item = qscriptvalue_cast<QProcess*>(thisObject());
  if (item)
    return item->pos();
  return 0;
}

QProcess::ProcessChannelMode QProcessProto::processChannelMode() const
{
  QProcess *item = qscriptvalue_cast<QProcess*>(thisObject());
  if (item)
    return item->processChannelMode();
  return QProcess::ProcessChannelMode();
}

QProcessEnvironment QProcessProto::processEnvironment() const
{
  QProcess *item = qscriptvalue_cast<QProcess*>(thisObject());
  if (item)
    return item->processEnvironment();
  return QProcessEnvironment();
}

bool QProcessProto::putChar(char c)
{
  QProcess *item = qscriptvalue_cast<QProcess*>(thisObject());
  if (item)
    return item->putChar(c);
  return false;
}

qint64 QProcessProto::read(char *data, qint64 maxSize)
{
  QProcess *item = qscriptvalue_cast<QProcess*>(thisObject());
  if (item)
    return item->read(data, maxSize);
  return 0;
}

QByteArray QProcessProto::read(qint64 maxSize)
{
  QProcess *item = qscriptvalue_cast<QProcess*>(thisObject());
  if (item)
    return item->read(maxSize);
  return QByteArray();
}

QByteArray QProcessProto::readAll()
{
  QProcess *item = qscriptvalue_cast<QProcess*>(thisObject());
  if (item)
    return item->readAll();
  return QByteArray();
}

QByteArray QProcessProto::readAllStandardError()
{
  QProcess *item = qscriptvalue_cast<QProcess*>(thisObject());
  if (item)
    return item->readAllStandardError();
  return QByteArray();
}

QByteArray QProcessProto::readAllStandardOutput()
{
  QProcess *item = qscriptvalue_cast<QProcess*>(thisObject());
  if (item)
    return item->readAllStandardOutput();
  return QByteArray();
}

QProcess::ProcessChannel QProcessProto::readChannel() const
{
  QProcess *item = qscriptvalue_cast<QProcess*>(thisObject());
  if (item)
    return item->readChannel();
  return QProcess::ProcessChannel();
}

qint64 QProcessProto::readLine(char *data, qint64 maxSize)
{
  QProcess *item = qscriptvalue_cast<QProcess*>(thisObject());
  if (item)
    return item->readLine(data, maxSize);
  return 0;
}

QByteArray QProcessProto::readLine(qint64 maxSize)
{
  QProcess *item = qscriptvalue_cast<QProcess*>(thisObject());
  if (item)
    return item->readLine(maxSize);
  return QByteArray();
}

bool QProcessProto::reset()
{
  QProcess *item = qscriptvalue_cast<QProcess*>(thisObject());
  if (item)
    return item->reset();
  return false;
}

bool QProcessProto::seek(qint64 pos)
{
  QProcess *item = qscriptvalue_cast<QProcess*>(thisObject());
  if (item)
    return item->seek(pos);
  return false;
}

void QProcessProto::setProcessChannelMode(QProcess::ProcessChannelMode mode)
{
  QProcess *item = qscriptvalue_cast<QProcess*>(thisObject());
  if (item)
    item->setProcessChannelMode(mode);
}

void QProcessProto::setProcessEnvironment(const QProcessEnvironment &environment)
{
  QProcess *item = qscriptvalue_cast<QProcess*>(thisObject());
  if (item)
    item->setProcessEnvironment(environment);
}

void QProcessProto::setReadChannel(QProcess::ProcessChannel channel)
{
  QProcess *item = qscriptvalue_cast<QProcess*>(thisObject());
  if (item)
    item->setReadChannel(channel);
}

void QProcessProto::setStandardErrorFile(const QString &fileName, QIODevice::OpenMode mode)
{
  QProcess *item = qscriptvalue_cast<QProcess*>(thisObject());
  if (item)
    item->setStandardErrorFile(fileName, mode);
}

void QProcessProto::setStandardInputFile(const QString &fileName)
{
  QProcess *item = qscriptvalue_cast<QProcess*>(thisObject());
  if (item)
    item->setStandardInputFile(fileName);
}

void QProcessProto::setStandardOutputFile(const QString &fileName, QIODevice::OpenMode mode)
{
  QProcess *item = qscriptvalue_cast<QProcess*>(thisObject());
  if (item)
    item->setStandardOutputFile(fileName, mode);
}

void QProcessProto::setStandardOutputProcess(QProcess *destination)
{
  QProcess *item = qscriptvalue_cast<QProcess*>(thisObject());
  if (item)
    item->setStandardOutputProcess(destination);
}

void QProcessProto::setTextModeEnabled(bool enabled)
{
  QProcess *item = qscriptvalue_cast<QProcess*>(thisObject());
  if (item)
    item->setTextModeEnabled(enabled);
}

void QProcessProto::setWorkingDirectory(const QString &dir)
{
  QProcess *item = qscriptvalue_cast<QProcess*>(thisObject());
  if (item)
    item->setWorkingDirectory(dir);
}

qint64 QProcessProto::size() const
{
  QProcess *item = qscriptvalue_cast<QProcess*>(thisObject());
  if (item)
    return item->size();
  return 0;
}

void QProcessProto::start(const QString &program, QIODevice::OpenMode mode)
{
  QProcess *item = qscriptvalue_cast<QProcess*>(thisObject());
  if (item)
    item->start(program, mode);
}

void QProcessProto::start(const QString &program, const QStringList &arguments, QIODevice::OpenMode mode)
{
  QProcess *item = qscriptvalue_cast<QProcess*>(thisObject());
  if (item)
    item->start(program, arguments, mode);
}

QProcess::ProcessState QProcessProto::state() const
{
  QProcess *item = qscriptvalue_cast<QProcess*>(thisObject());
  if (item)
    return item->state();
  return QProcess::ProcessState();
}

void QProcessProto::ungetChar(char c)
{
  QProcess *item = qscriptvalue_cast<QProcess*>(thisObject());
  if (item)
    item->ungetChar(c);
}

bool QProcessProto::waitForBytesWritten(int msecs)
{
  QProcess *item = qscriptvalue_cast<QProcess*>(thisObject());
  if (item)
    return item->waitForBytesWritten(msecs);
  return false;
}

bool QProcessProto::waitForFinished(int msecs)
{
  QProcess *item = qscriptvalue_cast<QProcess*>(thisObject());
  if (item)
    return item->waitForFinished(msecs);
  return false;
}

bool QProcessProto::waitForReadyRead(int msecs)
{
  QProcess *item = qscriptvalue_cast<QProcess*>(thisObject());
  if (item)
    return item->waitForReadyRead(msecs);
  return false;
}

bool QProcessProto::waitForStarted(int msecs)
{
  QProcess *item = qscriptvalue_cast<QProcess*>(thisObject());
  if (item)
    return item->waitForStarted(msecs);
  return false;
}

QString QProcessProto::workingDirectory() const
{
  QProcess *item = qscriptvalue_cast<QProcess*>(thisObject());
  if (item)
    return item->workingDirectory();
  return QString();
}

qint64 QProcessProto::write(const QByteArray &byteArray)
{
  QProcess *item = qscriptvalue_cast<QProcess*>(thisObject());
  if (item)
    return item->write(byteArray);
  return 0;
}

qint64 QProcessProto::write(const char *data)
{
  QProcess *item = qscriptvalue_cast<QProcess*>(thisObject());
  if (item)
    return item->write(data);
  return 0;
}

qint64 QProcessProto::write(const char *data, qint64 maxSize)
{
  QProcess *item = qscriptvalue_cast<QProcess*>(thisObject());
  if (item)
    return item->write(data, maxSize);
  return 0;
}

void QProcessProto::kill()
{
  QProcess *item = qscriptvalue_cast<QProcess*>(thisObject());
  if (item)
    item->kill();
}

void QProcessProto::terminate()
{
  QProcess *item = qscriptvalue_cast<QProcess*>(thisObject());
  if (item)
    item->terminate();
}

/**
QString QProcessProto::toString() const
{
  QProcess *item = qscriptvalue_cast<QProcess*>(thisObject());
  if (item)
    return QString("QProcess()");
  return QString("QProcess(unknown)");
}
*/
