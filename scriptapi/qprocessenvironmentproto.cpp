/*
 *This file is part of the xTuple ERP: PostBooks Edition, a free and
 *open source Enterprise Resource Planning software suite,
 *Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 *It is licensed to you under the Common Public Attribution License
 *version 1.0, the full text of which(including xTuple-specific Exhibits)
 *is available at www.xtuple.com/CPAL.  By using this software, you agree
 *to be bound by its terms.
 */

#include "qprocessenvironmentproto.h"
#include <QScriptValue>
#include <QScriptValueIterator>

#define DEBUG true

/** \ingroup scriptapi
    \class QProcessEnvironment
    \brief This class exposes the QProcessEnvironment class to Qt Scripting.

    In this first implementation, the QProcessEnvironment is
    represented in JavaScript as a straightforward Object with
    properties. The name of the property is the name of the
    corresponding environment variable and the value of the property
    is the value of the environment variable.  This makes use of
    the QScriptValue::SkipInEnumeration property when converting
    back and forth betweeh C++ QProcessEnvironment objects and
    JavaScript Objects to decide what to represent in the environment.

    For example, to set the environment variable \c PGHOST to the value
    \c localhost in a QProcessEnvironemt, C++ code would look like this:
    \code
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    if (! env.contains("PGHOST")
      env.insert("PGHOST", "localhost");
    \endcode
    while the equivalent JavaScript would look like this:
    \code
    var env = QProcessEnvironment.systemEnvironment();
    if (! ("PGHOST" in env))
      env.PGHOST = "localhost";
    \endcode
*/

// TODO: remove as much of this file as possible, as most of it isn't necessary
QScriptValue QProcessEnvironmenttoScriptValue(QScriptEngine *engine, const QProcessEnvironment &item)
{
  QScriptValue result = engine->newObject();
  QStringList env = item.toStringList();
  for (int i = 0; i < env.size(); i++)
  {
    QStringList line = env[i].split("=");
    result.setProperty(line[0], line[1]);
  }
  return result;
}

void QProcessEnvironmentfromScriptValue(const QScriptValue &obj, QProcessEnvironment &item)
{
  item = QProcessEnvironment();
  
  QScriptValueIterator it(obj);
  while (it.hasNext())
  {
    it.next();
    if (it.flags() & QScriptValue::SkipInEnumeration)
      continue;
    item.insert(it.name(), it.value().toString());
  }
}

static QScriptValue getSystemEnvironment(QScriptContext * /*context*/, QScriptEngine *engine)
{
  return QProcessEnvironmenttoScriptValue(engine,
                                          QProcessEnvironment::systemEnvironment());
}

void setupQProcessEnvironmentProto(QScriptEngine *engine)
{
  qScriptRegisterMetaType(engine, QProcessEnvironmenttoScriptValue, QProcessEnvironmentfromScriptValue);

  QScriptValue proto = engine->newQObject(new QProcessEnvironmentProto(engine));
  engine->setDefaultPrototype(qMetaTypeId<QProcessEnvironment*>(), proto);

  QScriptValue constructor = engine->newFunction(constructQProcessEnvironment,
                                                 proto);
  constructor.setProperty("systemEnvironment", engine->newFunction(getSystemEnvironment, QScriptValue::SkipInEnumeration));

  engine->globalObject().setProperty("QProcessEnvironment",  constructor);
}

QScriptValue constructQProcessEnvironment(QScriptContext *context,
                                          QScriptEngine  *engine)
{
  QProcessEnvironment *obj = 0;
  if (context->argumentCount() > 0 &&
      qscriptvalue_cast<QProcessEnvironment*>(context->argument(0)))
    obj = new QProcessEnvironment(*(qscriptvalue_cast<QProcessEnvironment*>(context->argument(0))));
  else
    obj = new QProcessEnvironment();
  return engine->toScriptValue(obj);
}

QProcessEnvironmentProto::QProcessEnvironmentProto(QObject *parent)
    : QObject(parent)
{
}

/*
void QProcessEnvironmentProto::clear()
{
  QProcessEnvironment *item = qscriptvalue_cast<QProcessEnvironment*>(thisObject());
  if (item)
    item->clear();
}

bool QProcessEnvironmentProto::contains(const QString &name) const
{
  QProcessEnvironment *item = qscriptvalue_cast<QProcessEnvironment*>(thisObject());
  if (item)
    return item->contains(name);
  return false;
}

void QProcessEnvironmentProto::insert(const QString &name, const QString &value)
{
  QProcessEnvironment *item = qscriptvalue_cast<QProcessEnvironment*>(thisObject());
  if (item)
    item->insert(name, value);
}

bool QProcessEnvironmentProto::isEmpty() const
{
  QProcessEnvironment *item = qscriptvalue_cast<QProcessEnvironment*>(thisObject());
  if (item)
    return item->isEmpty();
  return false;
}

void QProcessEnvironmentProto::remove(const QString &name)
{
  QProcessEnvironment *item = qscriptvalue_cast<QProcessEnvironment*>(thisObject());
  if (item)
    item->remove(name);
}

QStringList QProcessEnvironmentProto::toStringList() const
{
  QProcessEnvironment *item = qscriptvalue_cast<QProcessEnvironment*>(thisObject());
  if (item)
    return item->toStringList();
  return QStringList();
}

QString QProcessEnvironmentProto::value(const QString &name, const QString &defaultValue) const
{
  QProcessEnvironment *item = qscriptvalue_cast<QProcessEnvironment*>(thisObject());
  if (item)
    return item->value(name, defaultValue);
  return QString();
}
*/

QProcessEnvironment QProcessEnvironmentProto::systemEnvironment()
{
  return QProcessEnvironment::systemEnvironment();
}

/*
QString QProcessEnvironmentProto::toString() const
{
  QProcessEnvironment *item = qscriptvalue_cast<QProcessEnvironment*>(thisObject());
  if (item)
    return QString("QProcessEnvironment()");
  return QString("QProcessEnvironment(unknown)");
}
*/
