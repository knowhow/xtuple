/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "scriptablePrivate.h"

#include <QDebug>
#include <QWidget>
#include <QScriptEngine>
#include <QScriptEngineDebugger>

#include "scripttoolbox.h"
#include "../scriptapi/qeventproto.h"
#include "../scriptapi/parameterlistsetup.h"

ScriptablePrivate::ScriptablePrivate(bool dialog, QWidget* parent)
  : _engine(0), _debugger(0), _scriptLoaded(false), _dialog(dialog), _parent(parent)
{
  ScriptToolbox::setLastWindow(parent);
}

ScriptablePrivate::~ScriptablePrivate()
{
  if(_engine)
    delete _engine;
}

QScriptEngine *ScriptablePrivate::engine()
{
  if(!_engine)
  {
    _engine = new QScriptEngine(_parent);
    if (_preferences->boolean("EnableScriptDebug"))
    {
      _debugger = new QScriptEngineDebugger(_parent);
      _debugger->attachTo(_engine);
    }
    omfgThis->loadScriptGlobals(_engine);
    QScriptValue mywindow = _engine->newQObject(_parent);
    _engine->globalObject().setProperty("mywindow", mywindow);
    if(_dialog)
    {
      QScriptValue mydialog = _engine->newQObject(_parent);
      _engine->globalObject().setProperty("mydialog", mydialog);
    }
  }

  return _engine;
}

void ScriptablePrivate::loadScript(const QString& oName)
{
  qDebug() << "Looking for a script " << oName;
  XSqlQuery scriptq;
  scriptq.prepare("SELECT script_source, script_order"
            "  FROM script"
            " WHERE((script_name=:script_name)"
            "   AND (script_enabled))"
            " ORDER BY script_order;");
  scriptq.bindValue(":script_name", oName);
  scriptq.exec();
  while(scriptq.next())
  {
    if(engine())
    {
      QString script = scriptHandleIncludes(scriptq.value("script_source").toString());
      QScriptValue result = _engine->evaluate(script, _parent->objectName());
      if (_engine->hasUncaughtException())
      {
        int line = _engine->uncaughtExceptionLineNumber();
        qDebug() << "uncaught exception at line" << line << ":" << result.toString();
      }
    }
    else
      qDebug() << "could not initialize engine";
  }
}

void ScriptablePrivate::loadScriptEngine()
{
  if(_scriptLoaded || !_parent)
    return;
  _scriptLoaded = true;

  QStringList scriptList;

  // load scripts by class heirarchy name
  const QMetaObject *m = _parent->metaObject();
  while(m)
  {
    scriptList.prepend(m->className());
    m = m->superClass();
  }

  // load scripts by object name
  QStringList parts = _parent->objectName().split(" ");
  QStringList search_parts;
  QString oName;
  while(!parts.isEmpty())
  {
    search_parts.append(parts.takeFirst());
    oName = search_parts.join(" ");
    scriptList.append(oName);
  }

  scriptList.removeDuplicates();
  for (int i = 0; i < scriptList.size(); ++i)
    loadScript(scriptList.at(i));
}

enum SetResponse ScriptablePrivate::callSet(const ParameterList & params)
{
  loadScriptEngine();

  enum SetResponse returnValue = NoError;
  if(_engine && _engine->globalObject().property("set").isFunction())
  {
    QScriptValueList args;
    args << ParameterListtoScriptValue(_engine, params);
    QScriptValue tmp = _engine->globalObject().property("set").call(QScriptValue(), args);
    SetResponsefromScriptValue(tmp, returnValue);
  }

  return returnValue;
}

void ScriptablePrivate::callShowEvent(QEvent *event)
{
  if(_engine && (_engine->globalObject().property("showEvent").isFunction()))
  {
    QScriptValueList args;
    args << _engine->toScriptValue(event);
    _engine->globalObject().property("showEvent").call(QScriptValue(), args);
  }
}

void ScriptablePrivate::callCloseEvent(QEvent *event)
{
  if(_engine && (_engine->globalObject().property("closeEvent").isFunction()))
  {
    QScriptValueList args;
    args << _engine->toScriptValue(event);
    _engine->globalObject().property("closeEvent").call(QScriptValue(), args);
  }
}

