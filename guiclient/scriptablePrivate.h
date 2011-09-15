/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef __SCRIPTABLEPRIVATE_H__
#define __SCRIPTABLEPRIVATE_H__

class QWidget;
class QScriptEngine;
class QScriptEngineDebugger;
class QEvent;

#include <QString>

#include "guiclient.h"
#include "parameter.h"

class ScriptablePrivate
{
  public:
    ScriptablePrivate(bool, QWidget*);
    virtual ~ScriptablePrivate();

    QScriptEngine *engine();
    void loadScript(const QString&);
    void loadScriptEngine();

    enum SetResponse callSet(const ParameterList &);
    void callShowEvent(QEvent*);
    void callCloseEvent(QEvent*);

    QScriptEngine * _engine;
    QScriptEngineDebugger * _debugger;

    bool _scriptLoaded;
    bool _dialog;

  private:
    QWidget *_parent;
};

#endif
