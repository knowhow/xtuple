/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "metasqlhighlighterproto.h"

#include <QTextDocument>
#include <QString>

#include "qtextdocumentproto.h"
#include "qtexteditproto.h"

#define DEBUG  true

void setupMetaSQLHighlighterProto(QScriptEngine *engine)
{
  QScriptValue proto = engine->newQObject(new MetaSQLHighlighterProto(engine));
  //engine->setDefaultPrototype(qMetaTypeId<MetaSQLHighlighter>(), proto);
  engine->setDefaultPrototype(qMetaTypeId<MetaSQLHighlighter*>(),  proto);

  QScriptValue constructor = engine->newFunction(constructMetaSQLHighlighter,
                                                 proto);
  engine->globalObject().setProperty("MetaSQLHighlighter",  constructor);
}

QScriptValue constructMetaSQLHighlighter(QScriptContext *context,
                                         QScriptEngine  *engine)
{
  MetaSQLHighlighter *obj = 0;
  if (context->argumentCount() > 0)
  {
    if (QTextDocument *doc = qscriptvalue_cast<QTextDocument*>(context->argument(0)))
    {
      if (DEBUG)
        qDebug("constructMetaSQLHighlighter(): doc = %p", doc);
      obj = new MetaSQLHighlighter(doc);
    }
    else if (QTextEdit *edit = qscriptvalue_cast<QTextEdit*>(context->argument(0)))
    {
      if (DEBUG)
        qDebug("constructMetaSQLHighlighter(): edit = %p", edit);
      obj = new MetaSQLHighlighter(edit);
    }
    else if (QObject *qobj = qscriptvalue_cast<QObject*>(context->argument(0)))
    {
      if (DEBUG)
        qDebug("constructMetaSQLHighlighter(): qobj = %p", qobj);
      obj = new MetaSQLHighlighter(qobj);
    }
  }
  return engine->toScriptValue(obj);
}

MetaSQLHighlighterProto::MetaSQLHighlighterProto(QObject *parent)
    : QObject(parent)
{
}

QString MetaSQLHighlighterProto::toString()     const
{
  MetaSQLHighlighter *item = qscriptvalue_cast<MetaSQLHighlighter*>(thisObject());
  if (item)
    return QString("[MetaSQLHighlighter %1 on %2]")
      .arg(item->objectName())
      .arg(item->parent() ? item->parent()->objectName() : "(no parent)");
  return QString();
}
