/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "crmcluster.h"
#include <QMessageBox>
#include <QSqlError>

#define DEBUG false

CrmClusterLineEdit::CrmClusterLineEdit(QWidget* pParent,
                                       const char* pTabName,
                                       const char* pIdColumn,
                                       const char* pNumberColumn,
                                       const char* pNameColumn,
                                       const char* pDescripColumn,
                                       const char* pOwnerColumn,
                                       const char* pAssignToColumn,
                                       const char* pExtra,
                                       const char* pName,
                                       const char* pActiveColumn) :
    VirtualClusterLineEdit(pParent, pTabName, pIdColumn, pNumberColumn, pNameColumn, pDescripColumn, pExtra, pName, pActiveColumn)
{
  setTableAndColumnNames(pTabName, pIdColumn, pNumberColumn, pNameColumn, pDescripColumn, pActiveColumn, pOwnerColumn, pAssignToColumn);
}

void CrmClusterLineEdit::setTableAndColumnNames(const char* pTabName,
                                                const char* pIdColumn,
                                                const char* pNumberColumn,
                                                const char* pNameColumn,
                                                const char* pDescripColumn,
                                                const char* pActiveColumn,
                                                const char* pOwnerColumn,
                                                const char* pAssignToColumn)
{
  // If no owner ignore because this call came from virtualCluster constructor
  if(!pOwnerColumn)
    return;

  _idColName = QString(pIdColumn);
  _numColName = QString(pNumberColumn);
  _nameColName = QString(pNameColumn);
  _descripColName = QString(pDescripColumn);
  _activeColName = QString(pActiveColumn);
  _ownerColName = QString(pOwnerColumn);
  _assigntoColName = QString(pAssignToColumn);

  _query = QString("SELECT %1 AS id, %2 AS number ")
                  .arg(pIdColumn).arg(pNumberColumn);

  _hasName = (pNameColumn && QString(pNameColumn).trimmed().length());
  if (_hasName)
    _query += QString(", %1 AS name ").arg(pNameColumn);

  _hasDescription = (pDescripColumn &&
                     QString(pDescripColumn).trimmed().length());
  if (_hasDescription)
    _query += QString(", %1 AS description ").arg(pDescripColumn);

  _hasOwner = (pOwnerColumn && QString(pOwnerColumn).trimmed().length());
  if (_hasOwner)
    _query += QString(", %1 AS owner ").arg(pOwnerColumn);

  _hasAssignto = (pAssignToColumn && QString(pAssignToColumn).trimmed().length());
  if (_hasAssignto)
    _query += QString(", %1 AS assignto ").arg(pAssignToColumn);

  _hasActive = (pActiveColumn && QString(pActiveColumn).trimmed().length());
  if (_hasActive)
    _query += QString(", %1 AS active ").arg(pActiveColumn);

  _query += QString("FROM %1 WHERE (TRUE) ").arg(pTabName);

  _idClause = QString(" AND (%1=:id) ").arg(pIdColumn);
  _numClause = QString(" AND (%1 ~* :number) ").arg(pNumberColumn);

  if (_hasActive)
    _activeClause = QString(" AND (%1) ").arg(pActiveColumn);
  else
    _activeClause = "";

  _extraClause = "";
  _model = new QSqlQueryModel(this);
}

void CrmClusterLineEdit::silentSetId(const int pId)
{
  if (DEBUG)
    qDebug("VCLE %s::silentSetId(%d)", qPrintable(objectName()), pId);

  if (pId == -1)
  {
    XLineEdit::clear();
    _model = new QSqlQueryModel(this);
  }
  else
  {
    XSqlQuery idQ;
    idQ.prepare(_query + _idClause + QString(";"));
    idQ.bindValue(":id", pId);
    idQ.exec();
    if (idQ.first())
    {
      if (_completer)
        static_cast<QSqlQueryModel* >(_completer->model())->setQuery(QSqlQuery());

      _id = pId;
      _valid = true;

      _model->setQuery(idQ);

      setText(idQ.value("number").toString());
      if (_hasName)
        _name = (idQ.value("name").toString());
      if (_hasDescription)
        _description = idQ.value("description").toString();
      if (_hasActive)
        setStrikeOut(!idQ.value("active").toBool());
      if (_hasOwner)
        _owner = idQ.value("owner").toString();
      if (_hasAssignto)
        _assignto = idQ.value("assignto").toString();
    }
    else if (idQ.lastError().type() != QSqlError::NoError)
      QMessageBox::critical(this, tr("A System Error Occurred at %1::%2.")
                            .arg(__FILE__)
                            .arg(__LINE__),
                            idQ.lastError().databaseText());
  }

  _parsed = true;
  sHandleNullStr();
  emit parsed();
}

void CrmClusterLineEdit::sUpdateMenu()
{
  if (! _x_privileges)
    return;

  _listAct->setEnabled(isEnabled());
  _searchAct->setEnabled(isEnabled());
  _aliasAct->setEnabled(isEnabled());
  _infoAct->setEnabled(_id != -1);
  _openAct->setEnabled((_x_privileges->check(_editPriv) ||
                        _x_privileges->check(_viewPriv) ||
                        (_x_privileges->check(_editOwnPriv) && (_owner == _x_username || _assignto == _x_username)) ||
                        (_x_privileges->check(_viewOwnPriv) && (_owner == _x_username || _assignto == _x_username))) &&
                        _id != -1);
  _newAct->setEnabled((_x_privileges->check(_newPriv) || _x_privileges->check(_editOwnPriv)) &&
                      isEnabled());

  if (canOpen())
  {
    if (!menu()->actions().contains(_openAct))
      menu()->addAction(_openAct);

    if (!menu()->actions().contains(_newAct) &&
        !_newPriv.isEmpty())
      menu()->addAction(_newAct);
  }
  else
  {
    if (menu()->actions().contains(_openAct))
      menu()->removeAction(_openAct);

    if (menu()->actions().contains(_newAct))
      menu()->removeAction(_newAct);
  }
}

void CrmClusterLineEdit::sOpen()
{
  ParameterList params;
  if (_x_privileges->check(_editPriv) ||
      _x_privileges->check(_editOwnPriv) && (_owner == _x_username || _assignto == _x_username))
    params.append("mode", "edit");
  else
    params.append("mode", "view");
  params.append(_idColName, id());

  sOpenWindow(_uiName, params);
}

void CrmClusterLineEdit::setEditOwnPriv(const QString& priv)
{
  _editOwnPriv = priv;
  buildExtraClause();
  sUpdateMenu();
}

void CrmClusterLineEdit::setViewOwnPriv(const QString& priv)
{
  _viewOwnPriv = priv;
  buildExtraClause();
  sUpdateMenu();
}

void CrmClusterLineEdit::buildExtraClause()
{
  if ( _x_privileges &&
       !_x_privileges->check(_editPriv) &&
       !_x_privileges->check(_viewPriv) &&
       (_x_privileges->check(_editOwnPriv) || _x_privileges->check(_viewOwnPriv) ) )
  {
    QStringList userCols;
    if(_hasOwner)
      userCols.append(_ownerColName);
    if(_hasAssignto)
      userCols.append(_assigntoColName);

    _extraClause = QString("'%1' IN (%2)").arg(_x_username).arg(userCols.join(","));
  }

}
