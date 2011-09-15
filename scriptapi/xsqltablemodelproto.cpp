/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "xsqltablemodelproto.h"
#include "parameterlistsetup.h"

#include <QMimeData>

void setupXSqlTableModelProto(QScriptEngine *engine)
{
  QScriptValue proto = engine->newQObject(new XSqlTableModelProto(engine));
  engine->setDefaultPrototype(qMetaTypeId<XSqlTableModel*>(), proto);

  QScriptValue constructor = engine->newFunction(constructXSqlTableModel,
                                                 proto);
  engine->globalObject().setProperty("XSqlTableModel",  constructor);
}

QScriptValue constructXSqlTableModel(QScriptContext * context,
                                    QScriptEngine  *engine)
{
  XSqlTableModel *obj = 0;
  if (context->argumentCount() == 1)
    obj = new XSqlTableModel(context->argument(1).toQObject());
  else
    obj = new XSqlTableModel();
  return engine->toScriptValue(obj);
}

XSqlTableModelProto::XSqlTableModelProto(QObject *parent)
    : QObject(parent)
{
}

QModelIndex XSqlTableModelProto::buddy(const QModelIndex &index) const
{
  XSqlTableModel *item = qscriptvalue_cast<XSqlTableModel*>(thisObject());
  if (item)
    return item->buddy(index);
  return QModelIndex();
}

bool XSqlTableModelProto::canFetchMore(const QModelIndex &parent) const
{
  XSqlTableModel *item = qscriptvalue_cast<XSqlTableModel*>(thisObject());
  if (item)
    return item->canFetchMore(parent);
  return false;
}

void XSqlTableModelProto::clear()
{
  XSqlTableModel *item = qscriptvalue_cast<XSqlTableModel*>(thisObject());
  if (item)
    item->clear();
}

int XSqlTableModelProto::columnCount(const QModelIndex &parent)
{
  XSqlTableModel *item = qscriptvalue_cast<XSqlTableModel*>(thisObject());
  if (item)
    return item->columnCount(parent);
  return 0;
}

QVariant XSqlTableModelProto::data(const QModelIndex &item, int role) const
{
  XSqlTableModel *tmpitem = qscriptvalue_cast<XSqlTableModel*>(thisObject());
  if (tmpitem)
    return tmpitem->data(item, role);
  return QVariant();
}

QSqlDatabase XSqlTableModelProto::database() const
{
  XSqlTableModel *item = qscriptvalue_cast<XSqlTableModel*>(thisObject());
  if (item)
    return item->database();
  return QSqlDatabase();
}

bool XSqlTableModelProto::dropMimeData(const QMimeData *data, int action, int row, int column, const QModelIndex &parent)
{
  XSqlTableModel *item = qscriptvalue_cast<XSqlTableModel*>(thisObject());
  if (item)
    return item->dropMimeData(data, (Qt::DropAction)action, row, column, parent);
  return false;
}

int XSqlTableModelProto::editStrategy() const
{
  XSqlTableModel *item = qscriptvalue_cast<XSqlTableModel*>(thisObject());
  if (item)
    return item->editStrategy();
  return 0;
}

void XSqlTableModelProto::fetchMore(const QModelIndex &parent)
{
  XSqlTableModel *item = qscriptvalue_cast<XSqlTableModel*>(thisObject());
  if (item)
    item->fetchMore(parent);
}

int XSqlTableModelProto::fieldIndex(const QString &fieldName) const
{
  XSqlTableModel *item = qscriptvalue_cast<XSqlTableModel*>(thisObject());
  if (item)
    return item->fieldIndex(fieldName);
  return 0;
}

QString XSqlTableModelProto::filter() const
{
  XSqlTableModel *item = qscriptvalue_cast<XSqlTableModel*>(thisObject());
  if (item)
    return item->filter();
  return QString();
}

int XSqlTableModelProto::flags(const QModelIndex &index) const
{
  XSqlTableModel *item = qscriptvalue_cast<XSqlTableModel*>(thisObject());
  if (item)
    return item->flags(index);
  return 0;
}

bool XSqlTableModelProto::hasIndex(int row, int column, const QModelIndex &parent) const
{
  XSqlTableModel *item = qscriptvalue_cast<XSqlTableModel*>(thisObject());
  if (item)
    return item->hasIndex(row, column, parent);
  return false;
}

QVariant XSqlTableModelProto::headerData(int section, int orientation, int role) const
{
  XSqlTableModel *item = qscriptvalue_cast<XSqlTableModel*>(thisObject());
  if (item)
    return item->headerData(section, (Qt::Orientation)orientation, role);
  return QVariant();
}

QModelIndex XSqlTableModelProto::index(int row, int column, const QModelIndex &parent) const
{
  XSqlTableModel *item = qscriptvalue_cast<XSqlTableModel*>(thisObject());
  if (item)
    return item->index(row, column, parent);
  return QModelIndex();
}

bool XSqlTableModelProto::insertColumn(int column, const QModelIndex &parent)
{
  XSqlTableModel *item = qscriptvalue_cast<XSqlTableModel*>(thisObject());
  if (item)
    return item->insertColumn(column, parent);
  return false;
}

bool XSqlTableModelProto::insertColumns(int column, int count, const QModelIndex &parent)
{
  XSqlTableModel *item = qscriptvalue_cast<XSqlTableModel*>(thisObject());
  if (item)
    return item->insertColumns(column, count, parent);
  return false;
}

bool XSqlTableModelProto::insertRecord(int row, const QSqlRecord &record)
{
  XSqlTableModel *item = qscriptvalue_cast<XSqlTableModel*>(thisObject());
  if (item)
    return item->insertRecord(row, record);
  return false;
}

bool XSqlTableModelProto::insertRow(int row, const QModelIndex &parent)
{
  XSqlTableModel *item = qscriptvalue_cast<XSqlTableModel*>(thisObject());
  if (item)
    return item->insertRow(row, parent);
  return false;
}

bool XSqlTableModelProto::insertRows(int row, int count, const QModelIndex &parent)
{
  XSqlTableModel *item = qscriptvalue_cast<XSqlTableModel*>(thisObject());
  if (item)
    return item->insertRows(row, count, parent);
  return false;
}

bool XSqlTableModelProto::isDirty(const QModelIndex &index) const
{
  XSqlTableModel *item = qscriptvalue_cast<XSqlTableModel*>(thisObject());
  if (item)
    return item->isDirty(index);
  return false;
}

QMap<int, QVariant> XSqlTableModelProto::itemData(const QModelIndex &index) const
{
  XSqlTableModel *item = qscriptvalue_cast<XSqlTableModel*>(thisObject());
  if (item)
    return item->itemData(index);
  return QMap<int, QVariant>();
}

QSqlError XSqlTableModelProto::lastError() const
{
  XSqlTableModel *item = qscriptvalue_cast<XSqlTableModel*>(thisObject());
  if (item)
    return item->lastError();
  return QSqlError();
}

QModelIndexList XSqlTableModelProto::match(const QModelIndex &start, int role, const QVariant &value, int hits, int flags) const
{
  XSqlTableModel *item = qscriptvalue_cast<XSqlTableModel*>(thisObject());
  if (item)
    return item->match(start, role, value, hits, (Qt::MatchFlags)flags);
  return QModelIndexList();
}

QMimeData *XSqlTableModelProto::mimeData(const QModelIndexList &indexes) const
{
  XSqlTableModel *item = qscriptvalue_cast<XSqlTableModel*>(thisObject());
  if (item)
    return item->mimeData(indexes);
  return 0;
}

QStringList XSqlTableModelProto::mimeTypes() const
{
  XSqlTableModel *item = qscriptvalue_cast<XSqlTableModel*>(thisObject());
  if (item)
    return item->mimeTypes();
  return QStringList();
}

QSqlIndex XSqlTableModelProto::primaryKey() const
{
  XSqlTableModel *item = qscriptvalue_cast<XSqlTableModel*>(thisObject());
  if (item)
    return item->primaryKey();
  return QSqlIndex();
}

QSqlQuery XSqlTableModelProto::query() const
{
  XSqlTableModel *item = qscriptvalue_cast<XSqlTableModel*>(thisObject());
  if (item)
    return item->query();
  return QSqlQuery();
}

QSqlRecord XSqlTableModelProto::record() const
{
  XSqlTableModel *item = qscriptvalue_cast<XSqlTableModel*>(thisObject());
  if (item)
    return item->record();
  return QSqlRecord();
}

QSqlRecord XSqlTableModelProto::record(int row) const
{
  XSqlTableModel *item = qscriptvalue_cast<XSqlTableModel*>(thisObject());
  if (item)
    return item->record(row);
  return QSqlRecord();
}

QSqlRelation XSqlTableModelProto::relation(int column) const
{
  XSqlTableModel *item = qscriptvalue_cast<XSqlTableModel*>(thisObject());
  if (item)
    return item->relation(column);
  return QSqlRelation();
}

QSqlTableModel *XSqlTableModelProto::relationModel(int column) const
{
  XSqlTableModel *item = qscriptvalue_cast<XSqlTableModel*>(thisObject());
  if (item)
    return item->relationModel(column);
  return 0;
}

bool XSqlTableModelProto::removeColumn(int column, const QModelIndex &parent)
{
  XSqlTableModel *item = qscriptvalue_cast<XSqlTableModel*>(thisObject());
  if (item)
    return item->removeColumn(column, parent);
  return false;
}

bool XSqlTableModelProto::removeColumns(int column, int count, const QModelIndex &parent)
{
  XSqlTableModel *item = qscriptvalue_cast<XSqlTableModel*>(thisObject());
  if (item)
    return item->removeColumns(column, count, parent);
  return false;
}

bool XSqlTableModelProto::removeRows(int row, int count, const QModelIndex &parent)
{
  XSqlTableModel *item = qscriptvalue_cast<XSqlTableModel*>(thisObject());
  if (item)
    return item->removeRows(row, count, parent);
  return false;
}

void XSqlTableModelProto::revertRow(int row)
{
  XSqlTableModel *item = qscriptvalue_cast<XSqlTableModel*>(thisObject());
  if (item)
    item->revertRow(row);
}

int XSqlTableModelProto::rowCount(const QModelIndex &parent) const
{
  XSqlTableModel *item = qscriptvalue_cast<XSqlTableModel*>(thisObject());
  if (item)
    return item->rowCount(parent);
  return 0;
}

bool XSqlTableModelProto::select()
{
  XSqlTableModel *item = qscriptvalue_cast<XSqlTableModel*>(thisObject());
  if (item)
    return item->select();
  return false;
}

bool XSqlTableModelProto::setData(const QModelIndex &index, const QVariant &value, int role)
{
  XSqlTableModel *item = qscriptvalue_cast<XSqlTableModel*>(thisObject());
  if (item)
    return item->setData(index, value, role);
  return false;
}

void XSqlTableModelProto::setEditStrategy(int strategy)
{
  XSqlTableModel *item = qscriptvalue_cast<XSqlTableModel*>(thisObject());
  if (item)
    item->setEditStrategy((QSqlTableModel::EditStrategy)strategy);
}

void XSqlTableModelProto::setFilter(const QString &filter)
{
  XSqlTableModel *item = qscriptvalue_cast<XSqlTableModel*>(thisObject());
  if (item)
    item->setFilter(filter);
}

bool XSqlTableModelProto::setHeaderData(int section, int orientation, const QVariant &value, int role)
{
  XSqlTableModel *item = qscriptvalue_cast<XSqlTableModel*>(thisObject());
  if (item)
    return item->setHeaderData(section, (Qt::Orientation)orientation, value, role);
  return false;
}

bool XSqlTableModelProto::setItemData(const QModelIndex &index, const QMap<int, QVariant> &roles)
{
  XSqlTableModel *item = qscriptvalue_cast<XSqlTableModel*>(thisObject());
  if (item)
    return item->setItemData(index, roles);
  return false;
}

void XSqlTableModelProto::setKeys(int keyColumns)
{
  XSqlTableModel *item = qscriptvalue_cast<XSqlTableModel*>(thisObject());
  if (item)
    item->setKeys(keyColumns);
}

bool XSqlTableModelProto::setRecord(int row, const QSqlRecord &record)
{
  XSqlTableModel *item = qscriptvalue_cast<XSqlTableModel*>(thisObject());
  if (item)
    return item->setRecord(row, record);
  return false;
}

void XSqlTableModelProto::setRelation(int column, const QSqlRelation &relation)
{
  XSqlTableModel *item = qscriptvalue_cast<XSqlTableModel*>(thisObject());
  if (item)
    item->setRelation(column, relation);
}

void XSqlTableModelProto::setSort(int column, int order)
{
  XSqlTableModel *item = qscriptvalue_cast<XSqlTableModel*>(thisObject());
  if (item)
    item->setSort(column, (Qt::SortOrder)order);
}

void XSqlTableModelProto::setSupportedDragActions(int actions)
{
  XSqlTableModel *item = qscriptvalue_cast<XSqlTableModel*>(thisObject());
  if (item)
    item->setSupportedDragActions((Qt::DropActions)actions);
}

void XSqlTableModelProto::setTable(const QString &tableName, int keyColumns)
{
  XSqlTableModel *item = qscriptvalue_cast<XSqlTableModel*>(thisObject());
  if (item)
    item->setTable(tableName, keyColumns);
}

QModelIndex XSqlTableModelProto::sibling(int row, int column, const QModelIndex &index) const
{
  XSqlTableModel *item = qscriptvalue_cast<XSqlTableModel*>(thisObject());
  if (item)
    return item->sibling(row, column, index);
  return QModelIndex();
}

void XSqlTableModelProto::sort(int column, int order)
{
  XSqlTableModel *item = qscriptvalue_cast<XSqlTableModel*>(thisObject());
  if (item)
    item->sort(column, (Qt::SortOrder)order);
}

QSize XSqlTableModelProto::span(const QModelIndex &index) const
{
  XSqlTableModel *item = qscriptvalue_cast<XSqlTableModel*>(thisObject());
  if (item)
    return item->span(index);
  return QSize();
}

int XSqlTableModelProto::supportedDragActions() const
{
  XSqlTableModel *item = qscriptvalue_cast<XSqlTableModel*>(thisObject());
  if (item)
    return item->supportedDragActions();
  return 0;
}

int XSqlTableModelProto::supportedDropActions() const
{
  XSqlTableModel *item = qscriptvalue_cast<XSqlTableModel*>(thisObject());
  if (item)
    return item->supportedDropActions();
  return 0;
}

QString XSqlTableModelProto::tableName() const
{
  XSqlTableModel *item = qscriptvalue_cast<XSqlTableModel*>(thisObject());
  if (item)
    return item->tableName();
  return QString();
}

QString XSqlTableModelProto::toString() const
{
  XSqlTableModel *item = qscriptvalue_cast<XSqlTableModel*>(thisObject());
  if (item)
    return QString("[XSqlTableModelProto(table %1, query %2)]")
                      .arg(item->tableName())
                      .arg(item->query().lastQuery().left(80));
  return QString();
}

int XSqlTableModelProto::nodeCount()
{
  XSqlTableModel *item = qscriptvalue_cast<XSqlTableModel*>(thisObject());
  if (item)
    return item->nodeCount();
  return 0;
}

void XSqlTableModelProto::appendChild(XSqlTableNode *child)
{
  XSqlTableModel *item = qscriptvalue_cast<XSqlTableModel*>(thisObject());
  if (item)
    item->appendChild(child);
}

void XSqlTableModelProto::clearChildren()
{
  XSqlTableModel *item = qscriptvalue_cast<XSqlTableModel*>(thisObject());
  if (item)
    item->clearChildren();
}

void XSqlTableModelProto::removeChild(int index)
{
  XSqlTableModel *item = qscriptvalue_cast<XSqlTableModel*>(thisObject());
  if (item)
    item->removeChild(index);
}

XSqlTableNode* XSqlTableModelProto::appendChild(const QString &tableName, ParameterList &relations)
{
  XSqlTableModel *item = qscriptvalue_cast<XSqlTableModel*>(thisObject());
  if (item)
    return item->appendChild(tableName, relations);
  return 0;
}

QList<XSqlTableNode *> XSqlTableModelProto::children()
{
  XSqlTableModel *item = qscriptvalue_cast<XSqlTableModel*>(thisObject());
  if (item)
    return item->children();
  return QList<XSqlTableNode *>();
}

XSqlTableNode* XSqlTableModelProto::child(int index)
{
  XSqlTableModel *item = qscriptvalue_cast<XSqlTableModel*>(thisObject());
  if (item)
    return item->child(index);
  return 0;
}

XSqlTableNode* XSqlTableModelProto::child(const QString &tableName)
{
  XSqlTableModel *item = qscriptvalue_cast<XSqlTableModel*>(thisObject());
  if (item)
    return item->child(tableName);
  return 0;
}

void XSqlTableModelProto::set(ParameterList params)
{
  XSqlTableModel *item = qscriptvalue_cast<XSqlTableModel*>(thisObject());
  if (item)
    item->set(params);
}

ParameterList XSqlTableModelProto::parameters()
{
  XSqlTableModel *item = qscriptvalue_cast<XSqlTableModel*>(thisObject());
  if (item)
    return item->parameters();
  return ParameterList();
}

void XSqlTableModelProto::loadAll()
{
  XSqlTableModel *item = qscriptvalue_cast<XSqlTableModel*>(thisObject());
  if (item)
    item->loadAll();
}

void XSqlTableModelProto::load(int row)
{
  XSqlTableModel *item = qscriptvalue_cast<XSqlTableModel*>(thisObject());
  if (item)
    item->load(row);
}

bool XSqlTableModelProto::save()
{
  XSqlTableModel *item = qscriptvalue_cast<XSqlTableModel*>(thisObject());
  if (item)
    return item->save();
  return false;
}


