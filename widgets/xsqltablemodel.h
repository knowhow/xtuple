/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef XSQLTABLEMODEL_H
#define XSQLTABLEMODEL_H

#include <QSqlRelationalTableModel>
#include <QHash>

#include "widgets.h"

class XSqlTableModel;

class XTUPLEWIDGETS_EXPORT XSqlTableNode : public QObject
{
public:
  XSqlTableNode(const QString tableName, ParameterList relations, XSqlTableNode *parent = 0);
  ~XSqlTableNode();

  int index() const;
  int count() const { return _children.count(); }
  void appendChild(XSqlTableNode *child) { _children.append(child); }
  void removeChild(int index) { _children.removeAt(index); }
  XSqlTableNode* appendChild(const QString &tableName, ParameterList &relations);

  QMap<QPair<XSqlTableModel*, int>, XSqlTableModel* > modelMap() { return _modelMap; }
  QString tableName() const { return _tableName; }
  QList<XSqlTableNode *> children() const { return _children; }
  ParameterList relations() const { return _relations; }
  XSqlTableNode* child(int index) { return _children.at(index); }
  XSqlTableNode* child(const QString &tableName);
  XSqlTableNode* parent() const { return _parent; }
  XSqlTableModel* model(XSqlTableModel* parent = 0, int row = 0);

  void clear();
  void load(QPair<XSqlTableModel*, int> key);
  bool save();

private:
  ParameterList _relations;
  QMap<QPair<XSqlTableModel*, int>, XSqlTableModel* >_modelMap;
  QList<XSqlTableNode *> _children;
  QString _filter;
  QString _tableName;
  XSqlTableNode *_parent;
};

class XTUPLEWIDGETS_EXPORT XSqlTableModel : public QSqlRelationalTableModel
{
    Q_OBJECT

    public:
      XSqlTableModel(QObject *parent = 0);
      ~XSqlTableModel();
    
    enum itemDataRole { FormatRole = (Qt::UserRole + 1),
                  EditorRole,  
                  MenuRole, /* Other roles for xtreewidget?
                  RawRole,
                  IdRole,
                  RunningSetRole,
                  RunningInitRole,
                  TotalSetRole,
                  TotalInitRole,
                  IndentRole */ 
    };
    
    enum FormatFlags { Money, Qty, Curr, Percent, Cost, QtyPer, 
      SalesPrice, PurchPrice, UOMRatio, ExtPrice, Weight
    };

    void applyColumnRole(int column, int role, QVariant value);
    void applyColumnRoles();
    void applyColumnRoles(int row);
    void clear();
    void setColumnRole(int column, int role, QVariant value);
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    QVariant formatValue(const QVariant &dataValue, const QVariant &formatValue) const;
    bool select();
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);
    void setTable(const QString &tableName, int keyColumns = 0);
    void setKeys(int keyColumns);

    virtual QString selectStatement() const;

    // Table node functions
    static::QString buildFilter(ParameterList &params);
    static::ParameterList buildParams(XSqlTableModel* parent, int row, ParameterList relations);

    int nodeCount() const { return _children.count(); }
    void appendChild(XSqlTableNode *child) { _children.append(child); }
    void clearChildren();
    void removeChild(int index) { _children.removeAt(index); }
    XSqlTableNode* appendChild(const QString &tableName, ParameterList &relations);

    QList<XSqlTableNode *> children() { return _children; }
    XSqlTableNode* child(int index) { return _children.at(index); }
    XSqlTableNode* child(const QString &tableName);

    void set(ParameterList params) { _params = params; }
    ParameterList parameters() { return _params; }

    void load(int row);
    void loadAll();
    bool save();
    
  private:
    QHash<QPair<QModelIndex, int>, QVariant> roles;
    QMultiHash<int, QPair<QVariant, int> > _columnRoles;
    QList<QString> _locales;

    QList<XSqlTableNode *> _children;
    ParameterList _params;
};

#endif
