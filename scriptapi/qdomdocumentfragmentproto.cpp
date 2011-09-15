/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "qdomdocumentfragmentproto.h"

#include <QDomDocumentFragment>

void setupQDomDocumentFragmentProto(QScriptEngine *engine)
{
  QScriptValue proto = engine->newQObject(new QDomDocumentFragmentProto(engine));
  engine->setDefaultPrototype(qMetaTypeId<QDomDocumentFragment*>(), proto);
  engine->setDefaultPrototype(qMetaTypeId<QDomDocumentFragment>(),  proto);

  QScriptValue constructor = engine->newFunction(constructQDomDocumentFragment,
                                                 proto);
  engine->globalObject().setProperty("QDomDocumentFragment",  constructor);
}

QScriptValue constructQDomDocumentFragment(QScriptContext * /*context*/,
                                    QScriptEngine  *engine)
{
  QDomDocumentFragment *obj = 0;
  // if (context->argumentCount() > 0)
  // else
    obj = new QDomDocumentFragment();
  return engine->toScriptValue(obj);
}

QDomDocumentFragmentProto::QDomDocumentFragmentProto(QObject *parent)
    : QObject(parent)
{
}

int QDomDocumentFragmentProto:: nodeType() const
{
  QDomDocumentFragment *item = qscriptvalue_cast<QDomDocumentFragment*>(thisObject());
  if (item)
    return item->nodeType();
  return 0;
}

QString QDomDocumentFragmentProto::toString() const
{
  QDomDocumentFragment *item = qscriptvalue_cast<QDomDocumentFragment*>(thisObject());
  if (item)
    return QString("[QDomDocumentFragment(nodeType=%1)]").arg(item->nodeType());
  return QString("[QDomDocumentFragment(unknown)]");
}
