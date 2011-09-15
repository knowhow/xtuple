/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "qnetworkreplyproto.h"

#include <QNetworkReply>

void setupQNetworkReplyProto(QScriptEngine *engine)
{
  QScriptValue replyproto = engine->newQObject(new QNetworkReplyProto(engine));
  engine->setDefaultPrototype(qMetaTypeId<QNetworkReply*>(), replyproto);
}

QNetworkReplyProto::QNetworkReplyProto(QObject *parent)
  : QObject(parent)
{
}

void  QNetworkReplyProto::abort() const
{
  QNetworkReply *item = qscriptvalue_cast<QNetworkReply*>(thisObject());
  if (item)
    item->abort();
}

QVariant QNetworkReplyProto::attribute(const QNetworkRequest::Attribute &code) const
{
  QNetworkReply *item = qscriptvalue_cast<QNetworkReply*>(thisObject());
  if (item)
    return item->attribute(code);
  return QVariant();
}

void QNetworkReplyProto::close()
{
  QNetworkReply *item = qscriptvalue_cast<QNetworkReply*>(thisObject());
  if (item)
    item->close();
}

int QNetworkReplyProto::error() const
{
  QNetworkReply *item = qscriptvalue_cast<QNetworkReply*>(thisObject());
  if (item)
    return item->error();
  return (QNetworkReply::UnknownNetworkError);
}

bool QNetworkReplyProto::hasRawHeader(const QByteArray &headerName) const
{
  QNetworkReply *item = qscriptvalue_cast<QNetworkReply*>(thisObject());
  if (item)
    return item->hasRawHeader(headerName);
  return false;
}    

QVariant QNetworkReplyProto::header(QNetworkRequest::KnownHeaders header) const
{
  QNetworkReply *item = qscriptvalue_cast<QNetworkReply*>(thisObject());
  if (item)
    return item->header(header);
  return QVariant();
}

QNetworkAccessManager *QNetworkReplyProto::manager() const
{
  QNetworkReply *item = qscriptvalue_cast<QNetworkReply*>(thisObject());
  if (item)
    return item->manager();
  return 0;
}

QNetworkAccessManager::Operation QNetworkReplyProto::operation() const
{
  QNetworkReply *item = qscriptvalue_cast<QNetworkReply*>(thisObject());
  if (item)
    return item->operation();
  return QNetworkAccessManager::HeadOperation;
}

QByteArray QNetworkReplyProto::rawHeader(const QByteArray &headerName) const
{
  QNetworkReply *item = qscriptvalue_cast<QNetworkReply*>(thisObject());
  if (item)
    item->rawHeader(headerName);
  return QByteArray();
}

QList<QByteArray> QNetworkReplyProto::rawHeaderList() const
{
  QNetworkReply *item = qscriptvalue_cast<QNetworkReply*>(thisObject());
  if (item)
    return item->rawHeaderList();
  return QList<QByteArray>();
}

qint64 QNetworkReplyProto::readBufferSize() const
{
  QNetworkReply *item = qscriptvalue_cast<QNetworkReply*>(thisObject());
  if (item)
    return item->readBufferSize();
  return 0;
}

QNetworkRequest QNetworkReplyProto::request() const
{
  QNetworkReply *item = qscriptvalue_cast<QNetworkReply*>(thisObject());
  if (item)
    return item->request();
  return QNetworkRequest();
}

void QNetworkReplyProto::setReadBufferSize(qint64 size)
{
  QNetworkReply *item = qscriptvalue_cast<QNetworkReply*>(thisObject());
  if (item)
    item->setReadBufferSize(size);
}

#ifndef QT_NO_OPENSSL
void QNetworkReplyProto::setSslConfiguration(const QSslConfiguration &config)
{
  QNetworkReply *item = qscriptvalue_cast<QNetworkReply*>(thisObject());
  if (item)
    item->setSslConfiguration(config);
}

QSslConfiguration QNetworkReplyProto::sslConfiguration() const
{
  QNetworkReply *item = qscriptvalue_cast<QNetworkReply*>(thisObject());
  if (item)
    return item->sslConfiguration();
  return QSslConfiguration();
}
#endif 

QUrl QNetworkReplyProto::url() const
{
  QNetworkReply *item = qscriptvalue_cast<QNetworkReply*>(thisObject());
  if (item)
    return item->url();
  return QUrl();
}

QString QNetworkReplyProto::toString() const
{
  QNetworkReply *item = qscriptvalue_cast<QNetworkReply*>(thisObject());
  if (item)
    return QString("[QNetworkReply(url=%1)]")
            .arg(url().toString(QUrl::RemovePassword));
  return QString("[QNetworkReply(unknown)]")
          .arg(url().toString(QUrl::RemovePassword));
}

qint64 QNetworkReplyProto::bytesAvailable() const
{
  QNetworkReply *item = qscriptvalue_cast<QNetworkReply*>(thisObject());
  if (item)
    return item->bytesAvailable();
  return 0;
}

qint64 QNetworkReplyProto::bytesToWrite() const
{
  QNetworkReply *item = qscriptvalue_cast<QNetworkReply*>(thisObject());
  if (item)
    return item->bytesToWrite();
  return 0;
}

bool QNetworkReplyProto::canReadLine() const
{
  QNetworkReply *item = qscriptvalue_cast<QNetworkReply*>(thisObject());
  if (item)
    return item->canReadLine();
  return false;
}

QString QNetworkReplyProto::errorString() const
{
  QNetworkReply *item = qscriptvalue_cast<QNetworkReply*>(thisObject());
  if (item)
    return item->errorString();
  return QString();
}

bool QNetworkReplyProto::getChar(char * c)
{
  QNetworkReply *item = qscriptvalue_cast<QNetworkReply*>(thisObject());
  if (item)
    return item->getChar(c);
  return false;
}

bool QNetworkReplyProto::isOpen() const
{
  QNetworkReply *item = qscriptvalue_cast<QNetworkReply*>(thisObject());
  if (item)
    return item->isOpen();
  return false;
}

bool QNetworkReplyProto::isReadable() const
{
  QNetworkReply *item = qscriptvalue_cast<QNetworkReply*>(thisObject());
  if (item)
    return item->isReadable();
  return false;
}

bool QNetworkReplyProto::isSequential() const
{
  QNetworkReply *item = qscriptvalue_cast<QNetworkReply*>(thisObject());
  if (item)
    return item->isSequential();
  return false;
}

bool QNetworkReplyProto::isTextModeEnabled() const
{
  QNetworkReply *item = qscriptvalue_cast<QNetworkReply*>(thisObject());
  if (item)
    return item->isTextModeEnabled();
  return false;
}

bool QNetworkReplyProto::isWritable() const
{
  QNetworkReply *item = qscriptvalue_cast<QNetworkReply*>(thisObject());
  if (item)
    return item->isWritable();
  return false;
}

bool QNetworkReplyProto::open(int mode)
{
  QNetworkReply *item = qscriptvalue_cast<QNetworkReply*>(thisObject());
  if (item)
    return item->open((QIODevice::OpenMode)mode);
  return false;
}

int QNetworkReplyProto::openMode() const
{
  QNetworkReply *item = qscriptvalue_cast<QNetworkReply*>(thisObject());
  if (item)
    return item->openMode();
  return 0;
}

qint64 QNetworkReplyProto::peek(char * data, qint64 maxSize)
{
  QNetworkReply *item = qscriptvalue_cast<QNetworkReply*>(thisObject());
  if (item)
    return item->peek(data, maxSize);
  return 0;
}

QByteArray QNetworkReplyProto::peek(qint64 maxSize)
{
  QNetworkReply *item = qscriptvalue_cast<QNetworkReply*>(thisObject());
  if (item)
    return item->peek(maxSize);
  return QByteArray();
}

qint64 QNetworkReplyProto::pos() const
{
  QNetworkReply *item = qscriptvalue_cast<QNetworkReply*>(thisObject());
  if (item)
    return item->pos();
  return 0;
}

bool QNetworkReplyProto::putChar(char c)
{
  QNetworkReply *item = qscriptvalue_cast<QNetworkReply*>(thisObject());
  if (item)
    return item->putChar(c);
  return false;
}

qint64 QNetworkReplyProto::read(char * data, qint64 maxSize)
{
  QNetworkReply *item = qscriptvalue_cast<QNetworkReply*>(thisObject());
  if (item)
    return item->read(data, maxSize);
  return 0;
}

QByteArray QNetworkReplyProto::read(qint64 maxSize)
{
  QNetworkReply *item = qscriptvalue_cast<QNetworkReply*>(thisObject());
  if (item)
    return item->read(maxSize);
  return QByteArray();
}

QByteArray QNetworkReplyProto::readAll()
{
  QNetworkReply *item = qscriptvalue_cast<QNetworkReply*>(thisObject());
  if (item)
    return item->readAll();
  return QByteArray();
}

qint64 QNetworkReplyProto::readLine(char * data, qint64 maxSize)
{
  QNetworkReply *item = qscriptvalue_cast<QNetworkReply*>(thisObject());
  if (item)
    return item->readLine(data, maxSize);
  return 0;
}

QByteArray QNetworkReplyProto::readLine(qint64 maxSize)
{
  QNetworkReply *item = qscriptvalue_cast<QNetworkReply*>(thisObject());
  if (item)
    return item->readLine(maxSize);
  return QByteArray();
}

bool QNetworkReplyProto::reset()
{
  QNetworkReply *item = qscriptvalue_cast<QNetworkReply*>(thisObject());
  if (item)
    return item->reset();
  return false;
}

bool QNetworkReplyProto::seek(qint64 pos)
{
  QNetworkReply *item = qscriptvalue_cast<QNetworkReply*>(thisObject());
  if (item)
    return item->seek(pos);
  return false;
}

void QNetworkReplyProto::setTextModeEnabled(bool enabled)
{
  QNetworkReply *item = qscriptvalue_cast<QNetworkReply*>(thisObject());
  if (item)
    item->setTextModeEnabled(enabled);
}

qint64 QNetworkReplyProto::size() const
{
  QNetworkReply *item = qscriptvalue_cast<QNetworkReply*>(thisObject());
  if (item)
    return item->size();
  return 0;
}

void QNetworkReplyProto::ungetChar(char c)
{
  QNetworkReply *item = qscriptvalue_cast<QNetworkReply*>(thisObject());
  if (item)
    item->ungetChar(c);
}

bool QNetworkReplyProto::waitForBytesWritten(int msecs)
{
  QNetworkReply *item = qscriptvalue_cast<QNetworkReply*>(thisObject());
  if (item)
    return item->waitForBytesWritten(msecs);
  return false;
}

bool QNetworkReplyProto::waitForReadyRead(int msecs)
{
  QNetworkReply *item = qscriptvalue_cast<QNetworkReply*>(thisObject());
  if (item)
    return item->waitForReadyRead(msecs);
  return false;
}

qint64 QNetworkReplyProto::write(const char * data, qint64 maxSize)
{
  QNetworkReply *item = qscriptvalue_cast<QNetworkReply*>(thisObject());
  if (item)
    return item->write(data, maxSize);
  return 0;
}

qint64 QNetworkReplyProto::write(const QByteArray &byteArray)
{
  QNetworkReply *item = qscriptvalue_cast<QNetworkReply*>(thisObject());
  if (item)
    return item->write(byteArray);
  return 0;
}

qint64 QNetworkReplyProto::write(const QString &string)
{
  QNetworkReply *item = qscriptvalue_cast<QNetworkReply*>(thisObject());
  if (item)
    return item->write(string.toAscii());
  return 0;
}
