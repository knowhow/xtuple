#ifndef XTABLETREE_H
#define XATBLETREE_H

#include <QList>
#include <QMap>
#include <QObject>
#include <QString>

#include "parameter.h"
#include "xsqltablemodel.h"

class XTableTreeNode
{
public:
  XTableTreeNode(const QString &tableName, ParameterList &relations, XTableTreeNode *parent = 0);
  ~XTableTreeNode();

  int index() const;
  int count() const { return _children.count(); }
  void appendChild(XTableTreeNode *child) { _children.append(child); }
  void removeChild(int index) { _children.removeAt(index); }
  XTableTreeNode* appendChild(const QString &tableName, ParameterList &relations);

  QMap<XSqlTableModel*, QList<XSqlTableModel *> > modelMap() { return _modelMap; }
  QString tableName() const { return _tableName; }
  QList<XTableTreeNode *> children() const { return _children; }
  ParameterList relations() const { return _relations; }
  XTableTreeNode* child(int index) { return _children.at(index); }
  XTableTreeNode* child(const QString &tableName);
  XTableTreeNode* parent() const { return _parent; }
  XSqlTableModel* model(XSqlTableModel* parent = 0, int index = 0);

  void load(ParameterList &params = 0, XSqlTableModel* model = 0, bool recursive = true);
  bool save();
  bool saveAll(bool transact = true);

protected:
  ParameterList buildParams(XSqlTableModel* parent, ParameterList &relations);

private:
  ParameterList _relations;
  QMap<XSqlTableModel*, QList<XSqlTableModel *> >_modelMap;
  QList<XTableTreeNode *> _children;
  QString _filter;
  QString _tableName;
  XTableTreeNode *_parent;
};

class XTableTree : QObject
{
public:
  XTableTree(const QString &tableName, QObject *parent = 0);
  ~XTableTree();

  static::QString buildFilter(ParameterList &params);

  int count() const { return _children.count(); }
  void appendChild(XTableTreeNode *child) { _children.append(child); }
  void removeChild(int index) { _children.removeAt(index); }
  XTableTreeNode* appendChild(const QString &tableName, ParameterList &relations);

  QString tableName() const { return _model->tableName(); }
  QList<XTableTreeNode *> children() { return _children; }
  XTableTreeNode* child(int index) { return _children.at(index); }
  XTableTreeNode* child(const QString &tableName);
  XSqlTableModel* model() { return _model; }

  void load(ParameterList &params = 0, bool recursive = true);
  bool save();
  bool saveAll(bool transact = true);

private:
  ParameterList _relations;
  QList<XTableTreeNode *> _children;
  QString _filter;
  QString _tableName;
  XSqlTableModel* _model;
};

#endif
