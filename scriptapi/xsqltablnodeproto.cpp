/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "xsqltablenodeproto.h"
#include "parameterlistsetup.h"
#include "parameter.h"

void setupXSqlTableNodeProto(QScriptEngine *engine)
{
  QScriptValue proto = engine->newQObject(new XSqlTableNodeProto(engine));
  engine->setDefaultPrototype(qMetaTypeId<XSqlTableNode*>(), proto);

  QScriptValue constructor = engine->newFunction(constructXSqlTableNode,
                                                 proto);
  engine->globalObject().setProperty("XSqlTableNode",  constructor);
}

QScriptValue constructXSqlTableNode(QScriptContext * context,
                                    QScriptEngine  *engine)
{
  XSqlTableNode *obj = 0;
  if (!context->argument(1).isNull())
    obj = 0;
  else
  {
    if ( (context->argumentCount() == 3) &&
         context->argument(0).isString() &&
         qscriptvalue_cast<QObject*>(context->argument(2)))
      obj = new XSqlTableNode(context->argument(0).toString(),
                              qscriptvalue_cast<ParameterList>(context->argument(1)),
                              qscriptvalue_cast<XSqlTableNode*>(context->argument(2)));
    else if (context->argumentCount() == 2 &&
             context->argument(0).isString())
      obj = new XSqlTableNode(context->argument(0).toString(),
                              qscriptvalue_cast<ParameterList>(context->argument(1)));
    else
      obj = 0;
  }
  return engine->toScriptValue(obj);
}

XSqlTableNodeProto::XSqlTableNodeProto(QObject *parent)
    : QObject(parent)
{
}

XSqlTableNodeProto::~XSqlTableNodeProto()
{
}

int XSqlTableNodeProto::index() const
{
  XSqlTableNode *item = qscriptvalue_cast<XSqlTableNode*>(thisObject());
  if (item)
    return item->index();
  return 0;
}

int XSqlTableNodeProto::count() const
{
  XSqlTableNode *item = qscriptvalue_cast<XSqlTableNode*>(thisObject());
  if (item)
    return item->count();
  return 0;
}

void XSqlTableNodeProto::appendChild(XSqlTableNode *child)
{
  XSqlTableNode *item = qscriptvalue_cast<XSqlTableNode*>(thisObject());
  if (item)
    item->appendChild(child);
}

void XSqlTableNodeProto::removeChild(int index)
{
  XSqlTableNode *item = qscriptvalue_cast<XSqlTableNode*>(thisObject());
  if (item)
    item->removeChild(index);
}

XSqlTableNode* XSqlTableNodeProto::appendChild(const QString &tableName, ParameterList &relations)
{
  XSqlTableNode *item = qscriptvalue_cast<XSqlTableNode*>(thisObject());
  if (item)
    return item->appendChild(tableName, relations);
  return 0;
}

QMap<QPair<XSqlTableModel*, int>, XSqlTableModel* > XSqlTableNodeProto::modelMap()
{
  XSqlTableNode *item = qscriptvalue_cast<XSqlTableNode*>(thisObject());
  if (item)
    return item->modelMap();
  return QMap<QPair<XSqlTableModel*, int>, XSqlTableModel* >();
}

QString XSqlTableNodeProto::tableName()
{
  XSqlTableNode *item = qscriptvalue_cast<XSqlTableNode*>(thisObject());
  if (item)
    return item->tableName();
  return QString();
}

QList<XSqlTableNode *> XSqlTableNodeProto::children()
{
  XSqlTableNode *item = qscriptvalue_cast<XSqlTableNode*>(thisObject());
  if (item)
    return item->children();
  return QList<XSqlTableNode *>();
}

ParameterList XSqlTableNodeProto::relations()
{
  XSqlTableNode *item = qscriptvalue_cast<XSqlTableNode*>(thisObject());
  if (item)
    return item->relations();
  return ParameterList();
}

XSqlTableNode* XSqlTableNodeProto::child(int index)
{
  XSqlTableNode *item = qscriptvalue_cast<XSqlTableNode*>(thisObject());
  if (item)
    return item->child(index);
  return 0;
}

XSqlTableNode* XSqlTableNodeProto::child(const QString &tableName)
{
  XSqlTableNode *item = qscriptvalue_cast<XSqlTableNode*>(thisObject());
  if (item)
    return item->child(tableName);
  return 0;
}

XSqlTableNode* XSqlTableNodeProto::parent() const
{
  XSqlTableNode *item = qscriptvalue_cast<XSqlTableNode*>(thisObject());
  if (item)
    return item->parent();
  return 0;
}

XSqlTableModel* XSqlTableNodeProto::model(XSqlTableModel* parent, int row)
{
  XSqlTableNode *item = qscriptvalue_cast<XSqlTableNode*>(thisObject());
  if (item)
    return item->model(parent, row);
  return 0;
}

void XSqlTableNodeProto::clear()
{
  XSqlTableNode *item = qscriptvalue_cast<XSqlTableNode*>(thisObject());
  if (item)
    item->clear();
}

void XSqlTableNodeProto::load(QPair<XSqlTableModel*, int> key)
{
  XSqlTableNode *item = qscriptvalue_cast<XSqlTableNode*>(thisObject());
  if (item)
    item->load(key);
}

bool XSqlTableNodeProto::save()
{
  XSqlTableNode *item = qscriptvalue_cast<XSqlTableNode*>(thisObject());
  if (item)
    return item->save();
  return false;
}

