/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef __XSQLTABLEMODELPROTO_H__
#define __XSQLTABLEMODELPROTO_H__

#include <QMap>
#include <QModelIndex>
#include <QObject>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlIndex>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QSqlRelation>
#include <QString>
#include <QStringList>
#include <QVariant>
#include <QtScript>

#include "xsqltablemodel.h"

class QMimeData;

Q_DECLARE_METATYPE(XSqlTableModel*)

void setupXSqlTableModelProto(QScriptEngine *engine);
QScriptValue constructXSqlTableModel(QScriptContext *context, QScriptEngine *engine);

class XSqlTableModelProto : public QObject, public QScriptable
{
  Q_OBJECT

  public:
    XSqlTableModelProto(QObject *parent = 0);

    Q_INVOKABLE QModelIndex     buddy(const QModelIndex &index) const;
    Q_INVOKABLE bool            canFetchMore(const QModelIndex &parent = QModelIndex()) const;
    Q_INVOKABLE void            clear();
    Q_INVOKABLE int             columnCount(const QModelIndex &parent = QModelIndex());
    Q_INVOKABLE QVariant        data(const QModelIndex &item, int role = Qt::DisplayRole) const;
    Q_INVOKABLE QSqlDatabase    database()                          const;
    Q_INVOKABLE bool            dropMimeData(const QMimeData *data, int action, int row, int column, const QModelIndex &parent);
    Q_INVOKABLE int             editStrategy()                      const;
    Q_INVOKABLE void            fetchMore(const QModelIndex &parent = QModelIndex());
    Q_INVOKABLE int             fieldIndex(const QString &fieldName) const;
    Q_INVOKABLE QString         filter()                             const;
    Q_INVOKABLE int             flags(const QModelIndex &index)      const;
    Q_INVOKABLE bool            hasIndex(int row, int column, const QModelIndex &parent = QModelIndex()) const;
    Q_INVOKABLE QVariant        headerData(int section, int orientation, int role = Qt::DisplayRole) const;
    Q_INVOKABLE QModelIndex     index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
    Q_INVOKABLE bool            insertColumn(int column, const QModelIndex &parent = QModelIndex());
    Q_INVOKABLE bool            insertColumns(int column, int count, const QModelIndex &parent = QModelIndex());
    Q_INVOKABLE bool            insertRecord(int row, const QSqlRecord &record);
    Q_INVOKABLE bool            insertRow(int row, const QModelIndex &parent = QModelIndex());
    Q_INVOKABLE bool            insertRows(int row, int count, const QModelIndex &parent = QModelIndex());
    Q_INVOKABLE bool            isDirty(const QModelIndex &index)      const;
    Q_INVOKABLE QMap<int, QVariant> itemData(const QModelIndex &index) const;
    Q_INVOKABLE QSqlError       lastError()                            const;
    Q_INVOKABLE QModelIndexList match(const QModelIndex &start, int role, const QVariant &value, int hits = 1, int flags = Qt::MatchFlags(Qt::MatchStartsWith | Qt::MatchWrap)) const;
    Q_INVOKABLE QMimeData      *mimeData(const QModelIndexList &indexes) const;
    Q_INVOKABLE QStringList     mimeTypes()                              const;
    Q_INVOKABLE QSqlIndex       primaryKey()                             const;
    Q_INVOKABLE QSqlQuery       query()                                  const;
    Q_INVOKABLE QSqlRecord      record()                                 const;
    Q_INVOKABLE QSqlRecord      record(int row)                          const;
    Q_INVOKABLE QSqlRelation    relation(int column)                     const;
    Q_INVOKABLE QSqlTableModel *relationModel(int column)                const;
    Q_INVOKABLE bool            removeColumn(int column, const QModelIndex &parent = QModelIndex());
    Q_INVOKABLE bool            removeColumns(int column, int count, const QModelIndex &parent = QModelIndex());
    Q_INVOKABLE bool            removeRows(int row, int count, const QModelIndex &parent = QModelIndex());
    Q_INVOKABLE void            revertRow(int row);
    Q_INVOKABLE int             rowCount(const QModelIndex &parent = QModelIndex()) const;
    Q_INVOKABLE bool            select();
    Q_INVOKABLE bool            setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);
    Q_INVOKABLE void            setEditStrategy(int strategy);
    Q_INVOKABLE void            setFilter(const QString &filter);
    Q_INVOKABLE bool            setHeaderData(int section, int orientation, const QVariant &value, int role = Qt::EditRole);
    Q_INVOKABLE bool            setItemData(const QModelIndex &index, const QMap<int, QVariant> &roles);
    Q_INVOKABLE void            setKeys(int keyColumns);
    Q_INVOKABLE bool            setRecord(int row, const QSqlRecord &record);
    Q_INVOKABLE void            setRelation(int column, const QSqlRelation &relation);
    Q_INVOKABLE void            setSort(int column, int order);
    Q_INVOKABLE void            setSupportedDragActions(int actions);
    Q_INVOKABLE void            setTable(const QString &tableName, int keyColumns);
    Q_INVOKABLE QModelIndex     sibling(int row, int column, const QModelIndex &index) const;
    Q_INVOKABLE void            sort(int column, int order = Qt::AscendingOrder);
    Q_INVOKABLE QSize           span(const QModelIndex &index) const;
    Q_INVOKABLE int             supportedDragActions()         const;
    Q_INVOKABLE int             supportedDropActions()         const;
    Q_INVOKABLE QString         tableName()                    const;
    Q_INVOKABLE QString         toString()                     const;

    Q_INVOKABLE int             nodeCount();
    Q_INVOKABLE void            appendChild(XSqlTableNode *child);
    Q_INVOKABLE void            clearChildren();
    Q_INVOKABLE void            removeChild(int index);
    Q_INVOKABLE XSqlTableNode*  appendChild(const QString &tableName, ParameterList &relations);

    Q_INVOKABLE QList<XSqlTableNode *>  children();
    Q_INVOKABLE XSqlTableNode*          child(int index);
    Q_INVOKABLE XSqlTableNode*          child(const QString &tableName);

    Q_INVOKABLE void set(ParameterList params);
    Q_INVOKABLE ParameterList parameters();

    Q_INVOKABLE void            load(int row);
    Q_INVOKABLE void            loadAll();
    Q_INVOKABLE bool            save();
};

#endif
