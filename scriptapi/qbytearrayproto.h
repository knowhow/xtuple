/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which(including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef __QBYTEARRAYPROTO_H__
#define __QBYTEARRAYPROTO_H__

#include <QByteArray>
#include <QObject>
#include <QtScript>

Q_DECLARE_METATYPE(QByteArray*)

void setupQByteArrayProto(QScriptEngine *engine);
QScriptValue constructQByteArray(QScriptContext *context, QScriptEngine *engine);

class QByteArrayProto : public QObject, public QScriptable
{
  Q_OBJECT

  public:
    QByteArrayProto(QObject *parent);

    /*
    Q_INVOKABLE QByteArray &append(const QByteArray &ba);
    Q_INVOKABLE QByteArray &append(const QString &str);
    Q_INVOKABLE QByteArray &append(const char *str);
    Q_INVOKABLE QByteArray &append(const char *str, int len);
    Q_INVOKABLE QByteArray &append(char ch);
    Q_INVOKABLE char at(int i) const;
    Q_INVOKABLE int capacity() const;
    Q_INVOKABLE void chop(int n);
    Q_INVOKABLE void clear();
    Q_INVOKABLE const char *constData() const;
    Q_INVOKABLE bool contains(const QByteArray &ba) const;
    Q_INVOKABLE bool contains(const char *str) const;
    Q_INVOKABLE bool contains(char ch) const;
    Q_INVOKABLE int count(const QByteArray &ba) const;
    Q_INVOKABLE int count(const char *str) const;
    Q_INVOKABLE int count(char ch) const;
    Q_INVOKABLE int count() const;
    Q_INVOKABLE char *data();
    Q_INVOKABLE const char *data() const;
    Q_INVOKABLE bool endsWith(const QByteArray &ba) const;
    Q_INVOKABLE bool endsWith(const char *str) const;
    Q_INVOKABLE bool endsWith(char ch) const;
    Q_INVOKABLE QByteArray &fill(char ch, int size = -1);
    Q_INVOKABLE int indexOf(const QByteArray &ba, int from = 0) const;
    Q_INVOKABLE int indexOf(const QString &str, int from = 0) const;
    Q_INVOKABLE int indexOf(const char *str, int from = 0) const;
    Q_INVOKABLE int indexOf(char ch, int from = 0) const;
    Q_INVOKABLE QByteArray &insert(int i, const QByteArray &ba);
    Q_INVOKABLE QByteArray &insert(int i, const QString &str);
    Q_INVOKABLE QByteArray &insert(int i, const char *str);
    Q_INVOKABLE QByteArray &insert(int i, char ch);
    Q_INVOKABLE bool isEmpty() const;
    Q_INVOKABLE bool isNull() const;
    Q_INVOKABLE int lastIndexOf(const QByteArray &ba, int from = -1) const;
    Q_INVOKABLE int lastIndexOf(const QString &str, int from = -1) const;
    Q_INVOKABLE int lastIndexOf(const char *str, int from = -1) const;
    Q_INVOKABLE int lastIndexOf(char ch, int from = -1) const;
    Q_INVOKABLE QByteArray left(int len) const;
    Q_INVOKABLE QByteArray leftJustified(int width, char fill = ' ', bool truncate = false) const;
    Q_INVOKABLE int length() const;
    Q_INVOKABLE QByteArray mid(int pos, int len = -1) const;
    Q_INVOKABLE QByteArray &prepend(const QByteArray &ba);
    Q_INVOKABLE QByteArray &prepend(const char *str);
    Q_INVOKABLE QByteArray &prepend(char ch);
    Q_INVOKABLE void push_back(const QByteArray &other);
    Q_INVOKABLE void push_back(const char *str);
    Q_INVOKABLE void push_back(char ch);
    Q_INVOKABLE void push_front(const QByteArray &other);
    Q_INVOKABLE void push_front(const char *str);
    Q_INVOKABLE void push_front(char ch);
    Q_INVOKABLE QByteArray &remove(int pos, int len);
    Q_INVOKABLE QByteArray repeated(int times) const;
    Q_INVOKABLE QByteArray &replace(int pos, int len, const QByteArray &after);
    Q_INVOKABLE QByteArray &replace(int pos, int len, const char *after);
    Q_INVOKABLE QByteArray &replace(const QByteArray &before, const QByteArray &after);
    Q_INVOKABLE QByteArray &replace(const char *before, const QByteArray &after);
    Q_INVOKABLE QByteArray &replace(const char *before, int bsize, const char *after, int asize);
    Q_INVOKABLE QByteArray &replace(const QByteArray &before, const char *after);
    Q_INVOKABLE QByteArray &replace(const QString &before, const QByteArray &after);
    Q_INVOKABLE QByteArray &replace(const QString &before, const char *after);
    Q_INVOKABLE QByteArray &replace(const char *before, const char *after);
    Q_INVOKABLE QByteArray &replace(char before, const QByteArray &after);
    Q_INVOKABLE QByteArray &replace(char before, const QString &after);
    Q_INVOKABLE QByteArray &replace(char before, const char *after);
    Q_INVOKABLE QByteArray &replace(char before, char after);
    Q_INVOKABLE void reserve(int size);
    Q_INVOKABLE void resize(int size);
    Q_INVOKABLE QByteArray right(int len) const;
    Q_INVOKABLE QByteArray rightJustified(int width, char fill = ' ', bool truncate = false) const;
    Q_INVOKABLE QByteArray &setNum(int n, int base = 10);
    Q_INVOKABLE QByteArray &setNum(uint n, int base = 10);
    Q_INVOKABLE QByteArray &setNum(short n, int base = 10);
    Q_INVOKABLE QByteArray &setNum(ushort n, int base = 10);
    Q_INVOKABLE QByteArray &setNum(qlonglong n, int base = 10);
    Q_INVOKABLE QByteArray &setNum(qulonglong n, int base = 10);
    Q_INVOKABLE QByteArray &setNum(double n, char f = 'g', int prec = 6);
    Q_INVOKABLE QByteArray &setNum(float n, char f = 'g', int prec = 6);
    Q_INVOKABLE QByteArray simplified() const;
    Q_INVOKABLE int size() const;
    Q_INVOKABLE QList<QByteArray> split(char sep) const;
    Q_INVOKABLE void squeeze();
    Q_INVOKABLE bool startsWith(const QByteArray &ba) const;
    Q_INVOKABLE bool startsWith(const char *str) const;
    Q_INVOKABLE bool startsWith(char ch) const;
    Q_INVOKABLE QByteArray toBase64() const;
    Q_INVOKABLE double toDouble(bool *ok = 0) const;
    Q_INVOKABLE float toFloat(bool *ok = 0) const;
    Q_INVOKABLE QByteArray toHex() const;
    Q_INVOKABLE int toInt(bool *ok = 0, int base = 10) const;
    Q_INVOKABLE long toLong(bool *ok = 0, int base = 10) const;
    Q_INVOKABLE qlonglong toLongLong(bool *ok = 0, int base = 10) const;
    Q_INVOKABLE QByteArray toLower() const;
    Q_INVOKABLE QByteArray toPercentEncoding(const QByteArray &exclude = QByteArray(), const QByteArray &include = QByteArray(), char percent = '%') const;
    Q_INVOKABLE short toShort(bool *ok = 0, int base = 10) const;
    Q_INVOKABLE uint toUInt(bool *ok = 0, int base = 10) const;
    Q_INVOKABLE ulong toULong(bool *ok = 0, int base = 10) const;
    Q_INVOKABLE qulonglong toULongLong(bool *ok = 0, int base = 10) const;
    Q_INVOKABLE ushort toUShort(bool *ok = 0, int base = 10) const;
    Q_INVOKABLE QByteArray toUpper() const;
    Q_INVOKABLE QByteArray trimmed() const;
    Q_INVOKABLE void truncate(int pos);
    */

    Q_INVOKABLE QString toString() const;

};

#endif
