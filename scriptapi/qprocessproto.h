/*
 *This file is part of the xTuple ERP: PostBooks Edition, a free and
 *open source Enterprise Resource Planning software suite,
 *Copyright(c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 *It is licensed to you under the Common Public Attribution License
 *version 1.0, the full text of which(including xTuple-specific Exhibits)
 *is available at www.xtuple.com/CPAL.  By using this software, you agree
 *to be bound by its terms.
 */

#ifndef __QPROCESSPROTO_H__
#define __QPROCESSPROTO_H__

#include <QObject>
#include <QProcess>
#include <QtScript>

Q_DECLARE_METATYPE(QProcess*)

void setupQProcessProto(QScriptEngine *engine);
QScriptValue constructQProcess(QScriptContext *context, QScriptEngine *engine);

class QProcessProto : public QObject, public QScriptable
{
  Q_OBJECT

  public:
    QProcessProto(QObject *parent);

    Q_INVOKABLE bool                    atEnd() const;
    Q_INVOKABLE qint64                  bytesAvailable() const;
    Q_INVOKABLE qint64                  bytesToWrite() const;
    Q_INVOKABLE bool                    canReadLine() const;
    Q_INVOKABLE void                    close();
    Q_INVOKABLE void                    closeReadChannel(QProcess::ProcessChannel channel);
    Q_INVOKABLE void                    closeWriteChannel();
    Q_INVOKABLE QProcess::ProcessError  error() const;
    Q_INVOKABLE QString                 errorString() const;
    Q_INVOKABLE int                     exitCode() const;
    Q_INVOKABLE QProcess::ExitStatus    exitStatus() const;
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
    Q_INVOKABLE Q_PID                   pid() const;
    Q_INVOKABLE qint64                  pos() const;
    Q_INVOKABLE QProcess::ProcessChannelMode processChannelMode() const;
    Q_INVOKABLE QProcessEnvironment     processEnvironment() const;
    Q_INVOKABLE bool                    putChar(char c);
    Q_INVOKABLE qint64                  read(char *data, qint64 maxSize);
    Q_INVOKABLE QByteArray              read(qint64 maxSize);
    Q_INVOKABLE QByteArray              readAll();
    Q_INVOKABLE QByteArray              readAllStandardError();
    Q_INVOKABLE QByteArray              readAllStandardOutput();
    Q_INVOKABLE QProcess::ProcessChannel readChannel() const;
    Q_INVOKABLE qint64                  readLine(char *data, qint64 maxSize);
    Q_INVOKABLE QByteArray              readLine(qint64 maxSize = 0);
    Q_INVOKABLE bool                    reset();
    Q_INVOKABLE bool                    seek(qint64 pos);
    Q_INVOKABLE void                    setProcessChannelMode(QProcess::ProcessChannelMode mode);
    Q_INVOKABLE void                    setProcessEnvironment(const QProcessEnvironment &environment);
    Q_INVOKABLE void                    setReadChannel(QProcess::ProcessChannel channel);
    Q_INVOKABLE void                    setStandardErrorFile(const QString &fileName, QIODevice::OpenMode mode = QIODevice::Truncate);
    Q_INVOKABLE void                    setStandardInputFile(const QString &fileName);
    Q_INVOKABLE void                    setStandardOutputFile(const QString &fileName, QIODevice::OpenMode mode = QIODevice::Truncate);
    Q_INVOKABLE void                    setStandardOutputProcess(QProcess *destination);
    Q_INVOKABLE void                    setTextModeEnabled(bool enabled);
    Q_INVOKABLE void                    setWorkingDirectory(const QString &dir);
    Q_INVOKABLE qint64                  size() const;
    Q_INVOKABLE void                    start(const QString &program, QIODevice::OpenMode mode = QIODevice::ReadWrite);
    Q_INVOKABLE void                    start(const QString &program, const QStringList &arguments, QIODevice::OpenMode mode = QIODevice::ReadWrite);
    Q_INVOKABLE QProcess::ProcessState  state() const;
    Q_INVOKABLE void                    ungetChar(char c);
    Q_INVOKABLE bool                    waitForBytesWritten(int msecs);
    Q_INVOKABLE bool                    waitForFinished(int msecs = 30000);
    Q_INVOKABLE bool                    waitForReadyRead(int msecs);
    Q_INVOKABLE bool                    waitForStarted(int msecs = 30000);
    Q_INVOKABLE QString                 workingDirectory() const;
    Q_INVOKABLE qint64                  write(const QByteArray &byteArray);
    Q_INVOKABLE qint64                  write(const char *data);
    Q_INVOKABLE qint64                  write(const char *data, qint64 maxSize);

  public slots:
    void kill();
    void terminate();

  signals:
    void aboutToClose();
    void bytesWritten(qint64 bytes);
    void readChannelFinished();
    void readyRead();
};

#endif
