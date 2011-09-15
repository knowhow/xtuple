/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef __PARAMETEREDITPROTO_H__
#define __PARAMETEREDITPROTO_H__

#include "parameteredit.h"

#include <QObject>
#include <QtScript>

Q_DECLARE_METATYPE(ParameterEdit*)
//Q_DECLARE_METATYPE(ParameterEdit)

void setupParameterEditProto(QScriptEngine *engine);
QScriptValue constructParameterEdit(QScriptContext *context, QScriptEngine *engine);

class ParameterEditProto : public QObject, public QScriptable
{
  Q_OBJECT

  public:
    ParameterEditProto(QObject *parent);
};

#endif
