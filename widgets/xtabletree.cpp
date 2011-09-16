#include <QMessageBox>
#include <QSqlError>

#include "xtabletree.h"
#include "xsqlquery.h"

/*  XTableTreeNode */

XTableTreeNode::XTableTreeNode(const QString &tableName, ParameterList &relations, XTableTreeNode *parent)
{
  _tableName = tableName;
  _relations = relations;
  _parent = parent;
}

XTableTreeNode::~XTableTreeNode()
{
  qDeleteAll(_children);
}


int XTableTreeNode::index() const
{
  if (_parent)
    return _parent->children().indexOf(const_cast<XTableTreeNode*>(this));

  return 0;
}

XTableTreeNode* XTableTreeNode::appendChild(const QString &tableName, ParameterList &relations)
{
  XTableTreeNode* child = new XTableTreeNode(tableName, relations, this);
  appendChild(child);
  return child;
}

XTableTreeNode* XTableTreeNode::child(const QString &tableName)
{
  for (int i = 0; i < _children.count(); i++)
  {
    if (_children.at(i)->tableName() == tableName)
      return _children.at(i);
  }

  return 0;
}

XSqlTableModel* XTableTreeNode::model(XSqlTableModel* parent, int index)
{
  return _modelMap.value(parent).at(index);
}

void XTableTreeNode::clear()
{
  for (int n = 0; n < _children.count(); n++)
  {
    _children.at(n)->clear();
    _modelMap.clear();
  }
}

void XTableTreeNode::load()
{ 
    // Load up child nodes with data
    // Loop through "grand parent" models on this node to find parent madels
    QMapIterator<XSqlTableModel*, QList<XSqlTableModel *> > g(_modelMap);
    while (g.hasNext())
    {
      // Loop through what will be parents at next node down and add to our model map
      for (int p = 0; p < map.value(g.key()).count(); p++)
      {
        XSqlTableModel* pmodel = map.value(g.key()).at(p);

        // Loop through parent (pmodel) rows to create a list of populated
        // child models (cmodel) related to their parent as set by relations paramater list
        QList<XSqlTableModel *> clist;
        for (int r = 0; r < pmodel->rowCount(); r++)
        {
          XSqlTableModel* cmodel = new XSqlTableModel();
          cmodel->setTable(_tableName);
          ParameterList cparams = buildParams(pmodel, _relations);
          cmodel->setFilter(buildFilter(cparams));
          cmodel->select();
          clist.append(cmodel);
        }

        // Now map the child model list for the rows to our parent model
        _modelMap.insert(pmodel, clist);
      }

      // On to the next grand parent
      g.next();
    }
}

/* Saves the current model to the database*/
bool XTableTreeNode::save()
{
  //return _model->submitAll();
  return true;
}

/*!
    Saves the current model and all of it's child node models to the database where
    a\ transact wraps all submissions in a database transaction.
*/
bool XTableTreeNode::saveAll(bool transact)
{
  /*
  XSqlQuery trans;

  if (transact)
    trans.exec("BEGIN");

  if (_model->submitAll())
  {
    for ( int i = 0; i < _childNodes.count(); i++ )
    {
      if (!_childNodes.at(i)->saveAll(false))
        return false;
    }
  }
  else
  {
    if (transact)
      trans.exec("ROLLBACK");
    if (_model->lastError().type() != QSqlError::NoError)
      QMessageBox::critical(0, QString("A System Error Occurred at %1::%2.")
                            .arg(__FILE__)
                            .arg(__LINE__),
                            _model->lastError().databaseText());
    return false;
  }
*/
  return true;
}

////////////////////////////////////////

XTableTree::XTableTree(const QString &tableName, QObject *parent) : QObject(parent)
{
  _model = new XSqlTableModel();
  _model->setTable(tableName);
}

XTableTree::~XTableTree()
{
  qDeleteAll(_children);
}

XTableTreeNode* XTableTree::appendChild(const QString &tableName, ParameterList &relationsde)
{
  XTableTreeNode* child = new XTableTreeNode(tableName, relations, this);
  appendChild(child);
  return child;
}

XTableTreeNode* XTableTree::child(const QString &tableName)
{
  for (int i = 0; i < _children.count(); i++)
  {
    if (_children.at(i)->tableName() == tableName)
      return _children.at(i);
  }

  return 0;
}

QString XTableTree::buildFilter(ParameterList &params)
{
  QStringList clauses;
  for (int i = 0; i < params.count(); i++)
  {
    QString clause = QString(" (%1=%2) ").arg(params.at(i).name(), "%1");
    QVariant::Type type = params.at(i).value().type();
    switch (type)
    {
    case QVariant::Bool:
      if (params.at(i).value().toBool())
        clauses.append(QString(" (%1) ").arg(params.at(i).name()));
      else
        clauses.append(QString(" (NOT %1) ").arg(params.at(i).name()));
      break;
    case QVariant::Date:
      clauses.append(clause.arg(params.at(i).value().toDate().toString(Qt::ISODate)));
      break;
    case QVariant::Int:
    case QVariant::Double:
      clauses.append(clause.arg(params.at(i).value().toDouble()));
      break;
    default:
      clauses.append(clause.arg(params.at(i).value().toString().prepend("'").append("'")));
    }
  }
  return clauses.join(" AND ");
}

ParameterList XTableTree::buildParams(XSqlTableModel* parent, ParameterList &relations)
{
  ParameterList params;
  return params;
}

void XTableTree::load(ParameterList &params, bool recursive)
{
  // Populate mode based on params
  _model->setFilter(buildFilter(params));
  if (!_model->query().isActive())
    _model->select();

  if (recursive)
  {
    /*
    // Loop through "grand parent" models on the xtabletree parent to find parent madels
    QMap<XSqlTableModel*, QList<XSqlTableModel *> > map = parent()->modelMap();
    i = map;
    while (i.hasNext())
    {
      // Loop through each row to populate children
      for (int r = 0; r < _model->rowCount(); r++)
      {
        // Loop through each child node to create models
        for (int n = 0; n < _children.count(); n++)
        {
          XTreeNode* node = _children.at(n);
          node->
        XSqlTableModel* pmodel = map.value(i.key()).at(p);

        // Loop through parent rows to create a list of child models
        // related to their parent set by relations paramater list
        QList<XSqlTableModel *> clist;
        for (int r = 0; r < pmodel->rowCount(); r++)
        {
          XSqlTableModel* cmodel = new XSqlTableModel();
          cmodel->setTable(_tableName);
          ParameterList cparams = buildParams(pmodel, _relations);
          cmodel->setFilter(buildFilter(cparams));
          clist.append(cmodel);
        }

        // Now map the child model list for the rows to our parent model
        _modelMap.insert(pmodel, clist);
      }

      // On to the next grand parent
      i.next();
    } */
  }
}

/* Saves the current model to the database*/
bool XTableTree::save()
{
  //return _model->submitAll();
  return true;
}

/*!
    Saves the current model and all of it's child node models to the database where
    a\ transact wraps all submissions in a database transaction.
*/
bool XTableTree::saveAll(bool transact)
{
  /*
  XSqlQuery trans;

  if (transact)
    trans.exec("BEGIN");

  if (_model->submitAll())
  {
    for ( int i = 0; i < _childNodes.count(); i++ )
    {
      if (!_childNodes.at(i)->saveAll(false))
        return false;
    }
  }
  else
  {
    if (transact)
      trans.exec("ROLLBACK");
    if (_model->lastError().type() != QSqlError::NoError)
      QMessageBox::critical(0, QString("A System Error Occurred at %1::%2.")
                            .arg(__FILE__)
                            .arg(__LINE__),
                            _model->lastError().databaseText());
    return false;
  }
*/
  return true;
}

