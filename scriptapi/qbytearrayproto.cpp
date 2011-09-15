/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which(including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "qbytearrayproto.h"

static QByteArray nullBA = QByteArray();

QScriptValue QByteArraytoScriptValue(QScriptEngine *engine, QByteArray* const &item)
{
  QByteArray tmpBA = *item;
  QVariant  *tmpVar = new QVariant(tmpBA);      // TODO: memory leak?
  return engine->newVariant(*tmpVar);
}

void QByteArrayfromScriptValue(const QScriptValue &obj, QByteArray* &item)
{
  item = new QByteArray(obj.toVariant().toByteArray());
}

void setupQByteArrayProto(QScriptEngine *engine)
{
  qScriptRegisterMetaType(engine, QByteArraytoScriptValue, QByteArrayfromScriptValue);

  QScriptValue proto = engine->newQObject(new QByteArrayProto(engine));
  engine->setDefaultPrototype(qMetaTypeId<QByteArray*>(), proto);

  QScriptValue constructor = engine->newFunction(constructQByteArray,
                                                 proto);
  engine->globalObject().setProperty("QByteArray",  constructor);
}

QScriptValue constructQByteArray(QScriptContext *context,
                                 QScriptEngine  *engine)
{
  QByteArray *obj = 0;

  if (context->argumentCount() == 0)
    obj = new QByteArray();

  else if (context->argumentCount() == 1 &&
           qscriptvalue_cast<QByteArray*>(context->argument(0)))
    obj = new QByteArray(*(qscriptvalue_cast<QByteArray*>(context->argument(0))));

  else if (context->argumentCount() == 1 &&
           context->argument(0).isString())
    obj = new QByteArray(context->argument(0).toString().toAscii().data());

  else if (context->argumentCount() == 2 &&
           context->argument(0).isString() &&
           context->argument(1).isNumber())
    obj = new QByteArray(context->argument(0).toString().toAscii().data(),
                         context->argument(1).toInt32());

  else if (context->argumentCount() == 2 &&
           context->argument(0).isNumber() &&
           context->argument(1).isString())
    obj = new QByteArray(context->argument(0).toInt32(),
                         context->argument(1).toString().at(0).toAscii());

  else
    context->throwError(QScriptContext::UnknownError,
                       "could not find appropriate QByteArray constructor");

  return engine->toScriptValue(obj);
}

QByteArrayProto::QByteArrayProto(QObject *parent)
    : QObject(parent)
{
}

/*
QByteArray &QByteArrayProto::append(const QByteArray &ba)
{
  QByteArray *item = qscriptvalue_cast<QByteArray*>(thisObject());
  if (item)
    return item->append(ba);
  return nullBA;
}

QByteArray &QByteArrayProto::append(const QString &str)
{
  QByteArray *item = qscriptvalue_cast<QByteArray*>(thisObject());
  if (item)
    return item->append(str);
  return nullBA;
}

QByteArray &QByteArrayProto::append(const char *str)
{
  QByteArray *item = qscriptvalue_cast<QByteArray*>(thisObject());
  if (item)
    return item->append(str);
  return nullBA;
}

QByteArray &QByteArrayProto::append(const char *str, int len)
{
  QByteArray *item = qscriptvalue_cast<QByteArray*>(thisObject());
  if (item)
    return item->append(str, len);
  return nullBA;
}

QByteArray &QByteArrayProto::append(char ch)
{
  QByteArray *item = qscriptvalue_cast<QByteArray*>(thisObject());
  if (item)
    return item->append(ch);
  return nullBA;
}

char QByteArrayProto::at(int i) const
{
  QByteArray *item = qscriptvalue_cast<QByteArray*>(thisObject());
  if (item)
    return item->at(i);
  return char();
}

int QByteArrayProto::capacity() const
{
  QByteArray *item = qscriptvalue_cast<QByteArray*>(thisObject());
  if (item)
    return item->capacity();
  return 0;
}

void QByteArrayProto::chop(int n)
{
  QByteArray *item = qscriptvalue_cast<QByteArray*>(thisObject());
  if (item)
    item->chop(n);
}

void QByteArrayProto::clear()
{
  QByteArray *item = qscriptvalue_cast<QByteArray*>(thisObject());
  if (item)
    item->clear();
}

const char *QByteArrayProto::constData() const
{
  QByteArray *item = qscriptvalue_cast<QByteArray*>(thisObject());
  if (item)
    return item->constData();
  return 0;
}

bool QByteArrayProto::contains(const QByteArray &ba) const
{
  QByteArray *item = qscriptvalue_cast<QByteArray*>(thisObject());
  if (item)
    return item->contains(ba);
  return false;
}

bool QByteArrayProto::contains(const char *str) const
{
  QByteArray *item = qscriptvalue_cast<QByteArray*>(thisObject());
  if (item)
    return item->contains(str);
  return false;
}

bool QByteArrayProto::contains(char ch) const
{
  QByteArray *item = qscriptvalue_cast<QByteArray*>(thisObject());
  if (item)
    return item->contains(ch);
  return false;
}

int QByteArrayProto::count(const QByteArray &ba) const
{
  QByteArray *item = qscriptvalue_cast<QByteArray*>(thisObject());
  if (item)
    return item->count(ba);
  return 0;
}

int QByteArrayProto::count(const char *str) const
{
  QByteArray *item = qscriptvalue_cast<QByteArray*>(thisObject());
  if (item)
    return item->count(str);
  return 0;
}

int QByteArrayProto::count(char ch) const
{
  QByteArray *item = qscriptvalue_cast<QByteArray*>(thisObject());
  if (item)
    return item->count(ch);
  return 0;
}

int QByteArrayProto::count() const
{
  QByteArray *item = qscriptvalue_cast<QByteArray*>(thisObject());
  if (item)
    return item->count();
  return 0;
}

char *QByteArrayProto::data()
{
  QByteArray *item = qscriptvalue_cast<QByteArray*>(thisObject());
  if (item)
    return item->data();
  return 0;
}

const char *QByteArrayProto::data() const
{
  QByteArray *item = qscriptvalue_cast<QByteArray*>(thisObject());
  if (item)
    return item->data();
  return 0;
}

bool QByteArrayProto::endsWith(const QByteArray &ba) const
{
  QByteArray *item = qscriptvalue_cast<QByteArray*>(thisObject());
  if (item)
    return item->endsWith(ba);
  return false;
}

bool QByteArrayProto::endsWith(const char *str) const
{
  QByteArray *item = qscriptvalue_cast<QByteArray*>(thisObject());
  if (item)
    return item->endsWith(str);
  return false;
}

bool QByteArrayProto::endsWith(char ch) const
{
  QByteArray *item = qscriptvalue_cast<QByteArray*>(thisObject());
  if (item)
    return item->endsWith(ch);
  return false;
}

QByteArray &QByteArrayProto::fill(char ch, int size)
{
  QByteArray *item = qscriptvalue_cast<QByteArray*>(thisObject());
  if (item)
    return item->fill(ch, size);
  return nullBA;
}

int QByteArrayProto::indexOf(const QByteArray &ba, int from) const
{
  QByteArray *item = qscriptvalue_cast<QByteArray*>(thisObject());
  if (item)
    return item->indexOf(ba, from);
  return 0;
}

int QByteArrayProto::indexOf(const QString &str, int from) const
{
  QByteArray *item = qscriptvalue_cast<QByteArray*>(thisObject());
  if (item)
    return item->indexOf(str, from);
  return 0;
}

int QByteArrayProto::indexOf(const char *str, int from) const
{
  QByteArray *item = qscriptvalue_cast<QByteArray*>(thisObject());
  if (item)
    return item->indexOf(str, from);
  return 0;
}

int QByteArrayProto::indexOf(char ch, int from) const
{
  QByteArray *item = qscriptvalue_cast<QByteArray*>(thisObject());
  if (item)
    return item->indexOf(ch, from);
  return 0;
}

QByteArray &QByteArrayProto::insert(int i, const QByteArray &ba)
{
  QByteArray *item = qscriptvalue_cast<QByteArray*>(thisObject());
  if (item)
    return item->insert(i, ba);
  return nullBA;
}

QByteArray &QByteArrayProto::insert(int i, const QString &str)
{
  QByteArray *item = qscriptvalue_cast<QByteArray*>(thisObject());
  if (item)
    return item->insert(i, str);
  return nullBA;
}

QByteArray &QByteArrayProto::insert(int i, const char *str)
{
  QByteArray *item = qscriptvalue_cast<QByteArray*>(thisObject());
  if (item)
    return item->insert(i, str);
  return nullBA;
}

QByteArray &QByteArrayProto::insert(int i, char ch)
{
  QByteArray *item = qscriptvalue_cast<QByteArray*>(thisObject());
  if (item)
    return item->insert(i, ch);
  return nullBA;
}

bool QByteArrayProto::isEmpty() const
{
  QByteArray *item = qscriptvalue_cast<QByteArray*>(thisObject());
  if (item)
    return item->isEmpty();
  return false;
}

bool QByteArrayProto::isNull() const
{
  QByteArray *item = qscriptvalue_cast<QByteArray*>(thisObject());
  if (item)
    return item->isNull();
  return false;
}

int QByteArrayProto::lastIndexOf(const QByteArray &ba, int from) const
{
  QByteArray *item = qscriptvalue_cast<QByteArray*>(thisObject());
  if (item)
    return item->lastIndexOf(ba, from);
  return 0;
}

int QByteArrayProto::lastIndexOf(const QString &str, int from) const
{
  QByteArray *item = qscriptvalue_cast<QByteArray*>(thisObject());
  if (item)
    return item->lastIndexOf(str, from);
  return 0;
}

int QByteArrayProto::lastIndexOf(const char *str, int from) const
{
  QByteArray *item = qscriptvalue_cast<QByteArray*>(thisObject());
  if (item)
    return item->lastIndexOf(str, from);
  return 0;
}

int QByteArrayProto::lastIndexOf(char ch, int from) const
{
  QByteArray *item = qscriptvalue_cast<QByteArray*>(thisObject());
  if (item)
    return item->lastIndexOf(ch, from);
  return 0;
}

QByteArray QByteArrayProto::left(int len) const
{
  QByteArray *item = qscriptvalue_cast<QByteArray*>(thisObject());
  if (item)
    return item->left(len);
  return nullBA;
}

QByteArray QByteArrayProto::leftJustified(int width, char fill, bool truncate) const
{
  QByteArray *item = qscriptvalue_cast<QByteArray*>(thisObject());
  if (item)
    return item->leftJustified(width, fill, truncate);
  return nullBA;
}

int QByteArrayProto::length() const
{
  QByteArray *item = qscriptvalue_cast<QByteArray*>(thisObject());
  if (item)
    return item->length();
  return 0;
}

QByteArray QByteArrayProto::mid(int pos, int len) const
{
  QByteArray *item = qscriptvalue_cast<QByteArray*>(thisObject());
  if (item)
    return item->mid(pos, len);
  return nullBA;
}

QByteArray &QByteArrayProto::prepend(const QByteArray &ba)
{
  QByteArray *item = qscriptvalue_cast<QByteArray*>(thisObject());
  if (item)
    return item->prepend(ba);
  return nullBA;
}

QByteArray &QByteArrayProto::prepend(const char *str)
{
  QByteArray *item = qscriptvalue_cast<QByteArray*>(thisObject());
  if (item)
    return item->prepend(str);
  return nullBA;
}

QByteArray &QByteArrayProto::prepend(char ch)
{
  QByteArray *item = qscriptvalue_cast<QByteArray*>(thisObject());
  if (item)
    return item->prepend(ch);
  return nullBA;
}

void QByteArrayProto::push_back(const QByteArray &other)
{
  QByteArray *item = qscriptvalue_cast<QByteArray*>(thisObject());
  if (item)
    item->push_back(other);
}

void QByteArrayProto::push_back(const char *str)
{
  QByteArray *item = qscriptvalue_cast<QByteArray*>(thisObject());
  if (item)
    item->push_back(str);
}

void QByteArrayProto::push_back(char ch)
{
  QByteArray *item = qscriptvalue_cast<QByteArray*>(thisObject());
  if (item)
    item->push_back(ch);
}

void QByteArrayProto::push_front(const QByteArray &other)
{
  QByteArray *item = qscriptvalue_cast<QByteArray*>(thisObject());
  if (item)
    item->push_front(other);
}

void QByteArrayProto::push_front(const char *str)
{
  QByteArray *item = qscriptvalue_cast<QByteArray*>(thisObject());
  if (item)
    item->push_front(str);
}

void QByteArrayProto::push_front(char ch)
{
  QByteArray *item = qscriptvalue_cast<QByteArray*>(thisObject());
  if (item)
    item->push_front(ch);
}

QByteArray &QByteArrayProto::remove(int pos, int len)
{
  QByteArray *item = qscriptvalue_cast<QByteArray*>(thisObject());
  if (item)
    return item->remove(pos, len);
  return nullBA;
}

QByteArray QByteArrayProto::repeated(int times) const
{
  QByteArray *item = qscriptvalue_cast<QByteArray*>(thisObject());
  if (item)
    return item->repeated(times);
  return nullBA;
}

QByteArray &QByteArrayProto::replace(int pos, int len, const QByteArray &after)
{
  QByteArray *item = qscriptvalue_cast<QByteArray*>(thisObject());
  if (item)
    return item->replace(pos, len, after);
  return nullBA;
}

QByteArray &QByteArrayProto::replace(int pos, int len, const char *after)
{
  QByteArray *item = qscriptvalue_cast<QByteArray*>(thisObject());
  if (item)
    return item->replace(pos, len, after);
  return nullBA;
}

QByteArray &QByteArrayProto::replace(const QByteArray &before, const QByteArray &after)
{
  QByteArray *item = qscriptvalue_cast<QByteArray*>(thisObject());
  if (item)
    return item->replace(before, after);
  return nullBA;
}

QByteArray &QByteArrayProto::replace(const char *before, const QByteArray &after)
{
  QByteArray *item = qscriptvalue_cast<QByteArray*>(thisObject());
  if (item)
    return item->replace(before, after);
  return nullBA;
}

QByteArray &QByteArrayProto::replace(const char *before, int bsize, const char *after, int asize)
{
  QByteArray *item = qscriptvalue_cast<QByteArray*>(thisObject());
  if (item)
    return item->replace(before, bsize, after, asize);
  return nullBA;
}

QByteArray &QByteArrayProto::replace(const QByteArray &before, const char *after)
{
  QByteArray *item = qscriptvalue_cast<QByteArray*>(thisObject());
  if (item)
    return item->replace(before, after);
  return nullBA;
}

QByteArray &QByteArrayProto::replace(const QString &before, const QByteArray &after)
{
  QByteArray *item = qscriptvalue_cast<QByteArray*>(thisObject());
  if (item)
    return item->replace(before, after);
  return nullBA;
}

QByteArray &QByteArrayProto::replace(const QString &before, const char *after)
{
  QByteArray *item = qscriptvalue_cast<QByteArray*>(thisObject());
  if (item)
    return item->replace(before, after);
  return nullBA;
}

QByteArray &QByteArrayProto::replace(const char *before, const char *after)
{
  QByteArray *item = qscriptvalue_cast<QByteArray*>(thisObject());
  if (item)
    return item->replace(before, after);
  return nullBA;
}

QByteArray &QByteArrayProto::replace(char before, const QByteArray &after)
{
  QByteArray *item = qscriptvalue_cast<QByteArray*>(thisObject());
  if (item)
    return item->replace(before, after);
  return nullBA;
}

QByteArray &QByteArrayProto::replace(char before, const QString &after)
{
  QByteArray *item = qscriptvalue_cast<QByteArray*>(thisObject());
  if (item)
    return item->replace(before, after);
  return nullBA;
}

QByteArray &QByteArrayProto::replace(char before, const char *after)
{
  QByteArray *item = qscriptvalue_cast<QByteArray*>(thisObject());
  if (item)
    return item->replace(before, after);
  return nullBA;
}

QByteArray &QByteArrayProto::replace(char before, char after)
{
  QByteArray *item = qscriptvalue_cast<QByteArray*>(thisObject());
  if (item)
    return item->replace(before, after);
  return nullBA;
}

void QByteArrayProto::reserve(int size)
{
  QByteArray *item = qscriptvalue_cast<QByteArray*>(thisObject());
  if (item)
    item->reserve(size);
}

void QByteArrayProto::resize(int size)
{
  QByteArray *item = qscriptvalue_cast<QByteArray*>(thisObject());
  if (item)
    item->resize(size);
}

QByteArray QByteArrayProto::right(int len) const
{
  QByteArray *item = qscriptvalue_cast<QByteArray*>(thisObject());
  if (item)
    return item->right(len);
  return nullBA;
}

QByteArray QByteArrayProto::rightJustified(int width, char fill, bool truncate) const
{
  QByteArray *item = qscriptvalue_cast<QByteArray*>(thisObject());
  if (item)
    return item->rightJustified(width, fill, truncate);
  return nullBA;
}

QByteArray &QByteArrayProto::setNum(int n, int base)
{
  QByteArray *item = qscriptvalue_cast<QByteArray*>(thisObject());
  if (item)
    return item->setNum(n, base);
  return nullBA;
}

QByteArray &QByteArrayProto::setNum(uint n, int base)
{
  QByteArray *item = qscriptvalue_cast<QByteArray*>(thisObject());
  if (item)
    return item->setNum(n, base);
  return nullBA;
}

QByteArray &QByteArrayProto::setNum(short n, int base)
{
  QByteArray *item = qscriptvalue_cast<QByteArray*>(thisObject());
  if (item)
    return item->setNum(n, base);
  return nullBA;
}

QByteArray &QByteArrayProto::setNum(ushort n, int base)
{
  QByteArray *item = qscriptvalue_cast<QByteArray*>(thisObject());
  if (item)
    return item->setNum(n, base);
  return nullBA;
}

QByteArray &QByteArrayProto::setNum(qlonglong n, int base)
{
  QByteArray *item = qscriptvalue_cast<QByteArray*>(thisObject());
  if (item)
    return item->setNum(n, base);
  return nullBA;
}

QByteArray &QByteArrayProto::setNum(qulonglong n, int base)
{
  QByteArray *item = qscriptvalue_cast<QByteArray*>(thisObject());
  if (item)
    return item->setNum(n, base);
  return nullBA;
}

QByteArray &QByteArrayProto::setNum(double n, char f, int prec)
{
  QByteArray *item = qscriptvalue_cast<QByteArray*>(thisObject());
  if (item)
    return item->setNum(n, f, prec);
  return nullBA;
}

QByteArray &QByteArrayProto::setNum(float n, char f, int prec)
{
  QByteArray *item = qscriptvalue_cast<QByteArray*>(thisObject());
  if (item)
    return item->setNum(n, f, prec);
  return nullBA;
}

QByteArray QByteArrayProto::simplified() const
{
  QByteArray *item = qscriptvalue_cast<QByteArray*>(thisObject());
  if (item)
    return item->simplified();
  return nullBA;
}

int QByteArrayProto::size() const
{
  QByteArray *item = qscriptvalue_cast<QByteArray*>(thisObject());
  if (item)
    return item->size();
  return 0;
}

QList<QByteArray> QByteArrayProto::split(char sep) const
{
  QByteArray *item = qscriptvalue_cast<QByteArray*>(thisObject());
  if (item)
    return item->split(sep);
  return QList<QByteArray>();
}

void QByteArrayProto::squeeze()
{
  QByteArray *item = qscriptvalue_cast<QByteArray*>(thisObject());
  if (item)
    item->squeeze();
}

bool QByteArrayProto::startsWith(const QByteArray &ba) const
{
  QByteArray *item = qscriptvalue_cast<QByteArray*>(thisObject());
  if (item)
    return item->startsWith(ba);
  return false;
}

bool QByteArrayProto::startsWith(const char *str) const
{
  QByteArray *item = qscriptvalue_cast<QByteArray*>(thisObject());
  if (item)
    return item->startsWith(str);
  return false;
}

bool QByteArrayProto::startsWith(char ch) const
{
  QByteArray *item = qscriptvalue_cast<QByteArray*>(thisObject());
  if (item)
    return item->startsWith(ch);
  return false;
}

QByteArray QByteArrayProto::toBase64() const
{
  QByteArray *item = qscriptvalue_cast<QByteArray*>(thisObject());
  if (item)
    return item->toBase64();
  return nullBA;
}

double QByteArrayProto::toDouble(bool *ok) const
{
  QByteArray *item = qscriptvalue_cast<QByteArray*>(thisObject());
  if (item)
    return item->toDouble(ok);
  return double();
}

float QByteArrayProto::toFloat(bool *ok) const
{
  QByteArray *item = qscriptvalue_cast<QByteArray*>(thisObject());
  if (item)
    return item->toFloat(ok);
  return float();
}

QByteArray QByteArrayProto::toHex() const
{
  QByteArray *item = qscriptvalue_cast<QByteArray*>(thisObject());
  if (item)
    return item->toHex();
  return nullBA;
}

int QByteArrayProto::toInt(bool *ok, int base) const
{
  QByteArray *item = qscriptvalue_cast<QByteArray*>(thisObject());
  if (item)
    return item->toInt(ok, base);
  return 0;
}

long QByteArrayProto::toLong(bool *ok, int base) const
{
  QByteArray *item = qscriptvalue_cast<QByteArray*>(thisObject());
  if (item)
    return item->toLong(ok, base);
  return long();
}

qlonglong QByteArrayProto::toLongLong(bool *ok, int base) const
{
  QByteArray *item = qscriptvalue_cast<QByteArray*>(thisObject());
  if (item)
    return item->toLongLong(ok, base);
  return qlonglong();
}

QByteArray QByteArrayProto::toLower() const
{
  QByteArray *item = qscriptvalue_cast<QByteArray*>(thisObject());
  if (item)
    return item->toLower();
  return nullBA;
}

QByteArray QByteArrayProto::toPercentEncoding(const QByteArray &exclude, const QByteArray &include, char percent) const
{
  QByteArray *item = qscriptvalue_cast<QByteArray*>(thisObject());
  if (item)
    return item->toPercentEncoding(exclude, include, percent);
  return nullBA;
}

short QByteArrayProto::toShort(bool *ok, int base) const
{
  QByteArray *item = qscriptvalue_cast<QByteArray*>(thisObject());
  if (item)
    return item->toShort(ok, base);
  return short();
}

uint QByteArrayProto::toUInt(bool *ok, int base) const
{
  QByteArray *item = qscriptvalue_cast<QByteArray*>(thisObject());
  if (item)
    return item->toUInt(ok, base);
  return 0;
}

ulong QByteArrayProto::toULong(bool *ok, int base) const
{
  QByteArray *item = qscriptvalue_cast<QByteArray*>(thisObject());
  if (item)
    return item->toULong(ok, base);
  return ulong();
}

qulonglong QByteArrayProto::toULongLong(bool *ok, int base) const
{
  QByteArray *item = qscriptvalue_cast<QByteArray*>(thisObject());
  if (item)
    return item->toULongLong(ok, base);
  return qulonglong();
}

ushort QByteArrayProto::toUShort(bool *ok, int base) const
{
  QByteArray *item = qscriptvalue_cast<QByteArray*>(thisObject());
  if (item)
    return item->toUShort(ok, base);
  return ushort();
}

QByteArray QByteArrayProto::toUpper() const
{
  QByteArray *item = qscriptvalue_cast<QByteArray*>(thisObject());
  if (item)
    return item->toUpper();
  return nullBA;
}

QByteArray QByteArrayProto::trimmed() const
{
  QByteArray *item = qscriptvalue_cast<QByteArray*>(thisObject());
  if (item)
    return item->trimmed();
  return nullBA;
}

void QByteArrayProto::truncate(int pos)
{
  QByteArray *item = qscriptvalue_cast<QByteArray*>(thisObject());
  if (item)
    item->truncate(pos);
}
*/

QString QByteArrayProto::toString() const
{
  QByteArray *item = qscriptvalue_cast<QByteArray*>(thisObject());
  if (item)
    return QString(*item);
  return QString();
}
