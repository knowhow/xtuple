/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */


#include <QMessageBox>
#include <QSqlError>

#include <metasql.h>

#include "xsqlquery.h"


#include "racluster.h"

RaCluster::RaCluster(QWidget *pParent, const char *pName) :
  VirtualCluster(pParent, pName)
{
  addNumberWidget(new RaLineEdit(this, pName));
  connect(_number, SIGNAL(numberChanged(const QString &)), this, SIGNAL(numberChanged(const QString &)));
}

RaLineEdit::RaStatuses RaCluster::allowedStatuses() const
{
  return ((RaLineEdit*)_number)->allowedStatuses();
}

void RaCluster::setAllowedStatuses(const RaLineEdit::RaStatuses p)
{
  ((RaLineEdit*)_number)->setAllowedStatuses(p);
}

bool RaCluster::isClosed() const
{
  return ((RaLineEdit*)_number)->isClosed();
}

bool RaCluster::isOpen() const
{
  return ((RaLineEdit*)_number)->isOpen();
}

RaLineEdit::RaStatus RaCluster::status() const
{
  return ((RaLineEdit*)_number)->status();
}

// LineEdit ///////////////////////////////////////////////////////////////////

RaLineEdit::RaLineEdit(QWidget *pParent, const char *pName) :
  VirtualClusterLineEdit(pParent, "rahead", "rahead_id",
			 "rahead_number", "rahead_billtoname",
			 0, pName)
{
  setTitles(tr("Return Authorization"), tr("Return Authorizations"));
  setUiName("returnAuthorization");
  setEditPriv("MaintainReturns");
  setViewPriv("ViewReturns");

  setValidator(new QIntValidator(0, 9999999, this));
  connect(this, SIGNAL(textChanged(const QString &)), this, SIGNAL(numberChanged(const QString &)));
}

RaLineEdit::RaStatuses RaLineEdit::allowedStatuses() const
{
  return _statuses;
}

void RaLineEdit::setId(const int pId)
{
  if ((_x_preferences) && (pId != -1))
  {
    if (_x_preferences->boolean("selectedSites"))
    {
      QString msql("SELECT raitem_id "
                "FROM raitem, itemsite "
                "WHERE ((raitem_rahead_id=<? value(\"rahead_id\") ?>) "
                "  AND (raitem_itemsite_id=itemsite_id) "
                "  AND (itemsite_warehous_id NOT IN ("
                "       SELECT usrsite_warehous_id "
                "       FROM usrsite "
                "       WHERE (usrsite_username=getEffectiveXtUser())))) "
                "UNION "
                "SELECT raitem_id "
                "FROM raitem, itemsite "
                "WHERE ((raitem_rahead_id=<? value(\"rahead_id\") ?>) "
                "  AND (raitem_coitem_itemsite_id=itemsite_id) "
                "  AND (itemsite_warehous_id NOT IN ("
                "       SELECT usrsite_warehous_id "
                "       FROM usrsite "
                "       WHERE (usrsite_username=getEffectiveXtUser()))));");
      MetaSQLQuery mql(msql);
      ParameterList params;
      params.append("rahead_id", pId);
      XSqlQuery chk = mql.toQuery(params);
      if (chk.first())
      {
              QMessageBox::critical(this, tr("Access Denied"),
                                    tr("You may not view or edit this Return Authorization as it references "
                                       "a warehouse for which you have not been granted privileges.")) ;
              VirtualClusterLineEdit::setId(-1);
      }
      else
        VirtualClusterLineEdit::setId(pId);
    }
    else
      VirtualClusterLineEdit::setId(pId);
  }
  else 
    VirtualClusterLineEdit::setId(pId);
}


void RaLineEdit::setAllowedStatuses(const RaStatuses p)
{
  if (p & (Open | Closed) || p == 0)
    clearExtraClause();
  else if (p & Open)
    setExtraClause(" AND EXISTS ( SELECT raitem_id "
                   " FROM ratiem "
                   " WHERE ((raitem_status='O') "
                   " AND (raitem_rahead_id=rahead_id)) ");
  else if (p & Closed)
    setExtraClause(" AND NOT EXISTS ( SELECT raitem_id "
                   " FROM ratiem "
                   " WHERE ((raitem_status='O') "
                   " AND (raitem_rahead_id=rahead_id)) ");
  else
    clearExtraClause();

  _statuses = p;
}

bool RaLineEdit::isClosed() const
{
  return _description == "C";
}

bool RaLineEdit::isOpen() const
{
  return _description == "O";
}

RaLineEdit::RaStatus RaLineEdit::status() const
{
  if (_description == "C")	return Closed;
  else if (_description == "O")	return Open;
  else return AnyStatus;
}

// List ///////////////////////////////////////////////////////////////////////

RaList::RaList(QWidget *pParent, Qt::WindowFlags pFlags) :
  VirtualList(pParent, pFlags)
{
  _listTab->headerItem()->setData(1, Qt::DisplayRole, tr("Disposition"));
  _listTab->headerItem()->setData(2, Qt::DisplayRole, tr("Status"));
}
