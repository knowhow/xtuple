/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "qdomcommentproto.h"

#include <QDomAttr>
#include <QDomCDATASection>
#include <QDomCharacterData>
#include <QDomComment>
#include <QDomDocument>
#include <QDomDocumentFragment>
#include <QDomDocumentType>
#include <QDomElement>
#include <QDomEntity>
#include <QDomEntityReference>
#include <QDomImplementation>
#include <QDomNamedNodeMap>
#include <QDomNode>
#include <QDomNodeList>
#include <QDomNotation>
#include <QDomProcessingInstruction>
#include <QDomText>

void setupQDomCommentProto(QScriptEngine *engine)
{
  QScriptValue proto = engine->newQObject(new QDomCommentProto(engine));
  engine->setDefaultPrototype(qMetaTypeId<QDomComment*>(), proto);
  engine->setDefaultPrototype(qMetaTypeId<QDomComment>(),  proto);

  QScriptValue constructor = engine->newFunction(constructQDomComment,
                                                 proto);
  engine->globalObject().setProperty("QDomComment",  constructor);
}

QScriptValue constructQDomComment(QScriptContext * /*context*/,
                                    QScriptEngine  *engine)
{
  QDomComment *obj = 0;
  // if (context->argumentCount() > 0)
  // else
    obj = new QDomComment();
  return engine->toScriptValue(obj);
}

QDomCommentProto::QDomCommentProto(QObject *parent)
    : QObject(parent)
{
}

int QDomCommentProto:: nodeType() const
{
  QDomComment *item = qscriptvalue_cast<QDomComment*>(thisObject());
  if (item)
    return item->nodeType();
  return 0;
}

QString QDomCommentProto::toString() const
{
  QDomComment *item = qscriptvalue_cast<QDomComment*>(thisObject());
  if (item)
    return QString("[QDomComment(%1)").arg(item->data().left(25));
  return QString("[QDomComment(unknown)]");
}
