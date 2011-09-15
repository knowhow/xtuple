/*
 *This file is part of the xTuple ERP: PostBooks Edition, a free and
 *open source Enterprise Resource Planning software suite,
 *Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 *It is licensed to you under the Common Public Attribution License
 *version 1.0, the full text of which(including xTuple-specific Exhibits)
 *is available at www.xtuple.com/CPAL.  By using this software, you agree
 *to be bound by its terms.
 */

#include "qiodeviceproto.h"

/** \ingroup scriptapi
    \class QIODeviceProto
    \brief This class exposes the QIODevice class to Qt Scripting.

    The QIODeviceProto class exposes as much of the API to the
    QIODevice class as possible.
*/

QScriptValue QIODevicetoScriptValue(QScriptEngine *engine, QIODevice* const &item)
{ return engine->newQObject(item); }

void QIODevicefromScriptValue(const QScriptValue &obj, QIODevice* &item)
{ item = qobject_cast<QIODevice*>(obj.toQObject()); }

void setupQIODeviceProto(QScriptEngine *engine)
{
  qScriptRegisterMetaType(engine, QIODevicetoScriptValue, QIODevicefromScriptValue);

  QScriptValue iodev = engine->newObject();
  engine->globalObject().setProperty("QIODevice",  iodev, QScriptValue::ReadOnly | QScriptValue::Undeletable);

  // enum QIODevice::OpenModeFlag
  iodev.setProperty("NotOpen", QScriptValue(engine, QIODevice::NotOpen), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  iodev.setProperty("ReadOnly", QScriptValue(engine, QIODevice::ReadOnly), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  iodev.setProperty("WriteOnly", QScriptValue(engine, QIODevice::WriteOnly), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  iodev.setProperty("ReadWrite", QScriptValue(engine, QIODevice::ReadWrite), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  iodev.setProperty("Append", QScriptValue(engine, QIODevice::Append), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  iodev.setProperty("Truncate", QScriptValue(engine, QIODevice::Truncate), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  iodev.setProperty("Text", QScriptValue(engine, QIODevice::Text), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  iodev.setProperty("Unbuffered", QScriptValue(engine, QIODevice::Unbuffered), QScriptValue::ReadOnly | QScriptValue::Undeletable);
}

Q_DECLARE_METATYPE(enum QIODevice::OpenModeFlag);

QIODeviceProto::QIODeviceProto(QObject *parent)
    : QObject(parent)
{
}

bool QIODeviceProto::atEnd() const
{
  QIODevice *item = qscriptvalue_cast<QIODevice*>(thisObject());
  if (item)
    return item->atEnd();
  return false;
}

qint64 QIODeviceProto::bytesAvailable() const
{
  QIODevice *item = qscriptvalue_cast<QIODevice*>(thisObject());
  if (item)
    return item->bytesAvailable();
  return 0;
}

qint64 QIODeviceProto::bytesToWrite() const
{
  QIODevice *item = qscriptvalue_cast<QIODevice*>(thisObject());
  if (item)
    return item->bytesToWrite();
  return 0;
}

bool QIODeviceProto::canReadLine() const
{
  QIODevice *item = qscriptvalue_cast<QIODevice*>(thisObject());
  if (item)
    return item->canReadLine();
  return false;
}

void QIODeviceProto::close()
{
  QIODevice *item = qscriptvalue_cast<QIODevice*>(thisObject());
  if (item)
    item->close();
}

QString QIODeviceProto::errorString() const
{
  QIODevice *item = qscriptvalue_cast<QIODevice*>(thisObject());
  if (item)
    return item->errorString();
  return QString();
}

bool QIODeviceProto::getChar(char *c)
{
  QIODevice *item = qscriptvalue_cast<QIODevice*>(thisObject());
  if (item)
    return item->getChar(c);
  return false;
}

bool QIODeviceProto::isOpen() const
{
  QIODevice *item = qscriptvalue_cast<QIODevice*>(thisObject());
  if (item)
    return item->isOpen();
  return false;
}

bool QIODeviceProto::isReadable() const
{
  QIODevice *item = qscriptvalue_cast<QIODevice*>(thisObject());
  if (item)
    return item->isReadable();
  return false;
}

bool QIODeviceProto::isSequential() const
{
  QIODevice *item = qscriptvalue_cast<QIODevice*>(thisObject());
  if (item)
    return item->isSequential();
  return false;
}

bool QIODeviceProto::isTextModeEnabled() const
{
  QIODevice *item = qscriptvalue_cast<QIODevice*>(thisObject());
  if (item)
    return item->isTextModeEnabled();
  return false;
}

bool QIODeviceProto::isWritable() const
{
  QIODevice *item = qscriptvalue_cast<QIODevice*>(thisObject());
  if (item)
    return item->isWritable();
  return false;
}

bool QIODeviceProto::open(QIODevice::OpenMode mode)
{
  QIODevice *item = qscriptvalue_cast<QIODevice*>(thisObject());
  if (item)
    return item->open(mode);
  return false;
}

QIODevice::OpenMode QIODeviceProto::openMode() const
{
  QIODevice *item = qscriptvalue_cast<QIODevice*>(thisObject());
  if (item)
    return item->openMode();
  return QIODevice::OpenMode();
}

qint64 QIODeviceProto::peek(char *data, qint64 maxSize)
{
  QIODevice *item = qscriptvalue_cast<QIODevice*>(thisObject());
  if (item)
    return item->peek(data, maxSize);
  return 0;
}

QByteArray QIODeviceProto::peek(qint64 maxSize)
{
  QIODevice *item = qscriptvalue_cast<QIODevice*>(thisObject());
  if (item)
    return item->peek(maxSize);
  return QByteArray();
}

qint64 QIODeviceProto::pos() const
{
  QIODevice *item = qscriptvalue_cast<QIODevice*>(thisObject());
  if (item)
    return item->pos();
  return 0;
}

bool QIODeviceProto::putChar(char c)
{
  QIODevice *item = qscriptvalue_cast<QIODevice*>(thisObject());
  if (item)
    return item->putChar(c);
  return false;
}

qint64 QIODeviceProto::read(char *data, qint64 maxSize)
{
  QIODevice *item = qscriptvalue_cast<QIODevice*>(thisObject());
  if (item)
    return item->read(data, maxSize);
  return 0;
}

QByteArray QIODeviceProto::read(qint64 maxSize)
{
  QIODevice *item = qscriptvalue_cast<QIODevice*>(thisObject());
  if (item)
    return item->read(maxSize);
  return QByteArray();
}

QByteArray QIODeviceProto::readAll()
{
  QIODevice *item = qscriptvalue_cast<QIODevice*>(thisObject());
  if (item)
    return item->readAll();
  return QByteArray();
}

qint64 QIODeviceProto::readLine(char *data, qint64 maxSize)
{
  QIODevice *item = qscriptvalue_cast<QIODevice*>(thisObject());
  if (item)
    return item->readLine(data, maxSize);
  return 0;
}

QByteArray QIODeviceProto::readLine(qint64 maxSize)
{
  QIODevice *item = qscriptvalue_cast<QIODevice*>(thisObject());
  if (item)
    return item->readLine(maxSize);
  return QByteArray();
}

bool QIODeviceProto::reset()
{
  QIODevice *item = qscriptvalue_cast<QIODevice*>(thisObject());
  if (item)
    return item->reset();
  return false;
}

bool QIODeviceProto::seek(qint64 pos)
{
  QIODevice *item = qscriptvalue_cast<QIODevice*>(thisObject());
  if (item)
    return item->seek(pos);
  return false;
}

void QIODeviceProto::setTextModeEnabled(bool enabled)
{
  QIODevice *item = qscriptvalue_cast<QIODevice*>(thisObject());
  if (item)
    item->setTextModeEnabled(enabled);
}

qint64 QIODeviceProto::size() const
{
  QIODevice *item = qscriptvalue_cast<QIODevice*>(thisObject());
  if (item)
    return item->size();
  return 0;
}

void QIODeviceProto::ungetChar(char c)
{
  QIODevice *item = qscriptvalue_cast<QIODevice*>(thisObject());
  if (item)
    item->ungetChar(c);
}

bool QIODeviceProto::waitForBytesWritten(int msecs)
{
  QIODevice *item = qscriptvalue_cast<QIODevice*>(thisObject());
  if (item)
    return item->waitForBytesWritten(msecs);
  return false;
}

bool QIODeviceProto::waitForReadyRead(int msecs)
{
  QIODevice *item = qscriptvalue_cast<QIODevice*>(thisObject());
  if (item)
    return item->waitForReadyRead(msecs);
  return false;
}

qint64 QIODeviceProto::write(const QByteArray &byteArray)
{
  QIODevice *item = qscriptvalue_cast<QIODevice*>(thisObject());
  if (item)
    return item->write(byteArray);
  return 0;
}

qint64 QIODeviceProto::write(const char *data)
{
  QIODevice *item = qscriptvalue_cast<QIODevice*>(thisObject());
  if (item)
    return item->write(data);
  return 0;
}

qint64 QIODeviceProto::write(const char *data, qint64 maxSize)
{
  QIODevice *item = qscriptvalue_cast<QIODevice*>(thisObject());
  if (item)
    return item->write(data, maxSize);
  return 0;
}

/**
QString QIODeviceProto::toString() const
{
  QIODevice *item = qscriptvalue_cast<QIODevice*>(thisObject());
  if (item)
    return QString("QIODevice()");
  return QString("QIODevice(unknown)");
}
*/
