/*
 *This file is part of the xTuple ERP: PostBooks Edition, a free and
 *open source Enterprise Resource Planning software suite,
 *Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 *It is licensed to you under the Common Public Attribution License
 *version 1.0, the full text of which(including xTuple-specific Exhibits)
 *is available at www.xtuple.com/CPAL.  By using this software, you agree
 *to be bound by its terms.
 */

#ifndef __QIODEVICEPROTO_H__
#define __QIODEVICEPROTO_H__

#include <QObject>
#include <QIODevice>
#include <QtScript>

Q_DECLARE_METATYPE(QIODevice*)

void setupQIODeviceProto(QScriptEngine *engine);
QScriptValue constructQIODevice(QScriptContext *context, QScriptEngine *engine);

class QIODeviceProto : public QObject, public QScriptable
{
  Q_OBJECT

  public:
    QIODeviceProto(QObject *parent);

    Q_INVOKABLE bool                    atEnd() const;
    Q_INVOKABLE qint64                  bytesAvailable() const;
    Q_INVOKABLE qint64                  bytesToWrite() const;
    Q_INVOKABLE bool                    canReadLine() const;
    Q_INVOKABLE void                    close();
    Q_INVOKABLE QString                 errorString() const;
    Q_INVOKABLE bool                    getChar(char *c);
    Q_INVOKABLE bool                    isOpen() const;
    Q_INVOKABLE bool                    isReadable() const;
    Q_INVOKABLE bool                    isSequential() const;
    Q_INVOKABLE bool                    isTextModeEnabled() const;
    Q_INVOKABLE bool                    isWritable() const;
    Q_INVOKABLE bool                    open(QIODevice::OpenMode mode);
    Q_INVOKABLE QIODevice::OpenMode     openMode() const;
    Q_INVOKABLE qint64                  peek(char *data, qint64 maxSize);
    Q_INVOKABLE QByteArray              peek(qint64 maxSize);
    Q_INVOKABLE qint64                  pos() const;
    Q_INVOKABLE bool                    putChar(char c);
    Q_INVOKABLE qint64                  read(char *data, qint64 maxSize);
    Q_INVOKABLE QByteArray              read(qint64 maxSize);
    Q_INVOKABLE QByteArray              readAll();
    Q_INVOKABLE qint64                  readLine(char *data, qint64 maxSize);
    Q_INVOKABLE QByteArray              readLine(qint64 maxSize = 0);
    Q_INVOKABLE bool                    reset();
    Q_INVOKABLE bool                    seek(qint64 pos);
    Q_INVOKABLE void                    setTextModeEnabled(bool enabled);
    Q_INVOKABLE qint64                  size() const;
    Q_INVOKABLE void                    ungetChar(char c);
    Q_INVOKABLE bool                    waitForBytesWritten(int msecs);
    Q_INVOKABLE bool                    waitForReadyRead(int msecs);
    Q_INVOKABLE qint64                  write(const QByteArray &byteArray);
    Q_INVOKABLE qint64                  write(const char *data);
    Q_INVOKABLE qint64                  write(const char *data, qint64 maxSize);

  public slots:

  signals:
    void aboutToClose();
    void bytesWritten(qint64 bytes);
    void readChannelFinished();
    void readyRead();
};

#endif
