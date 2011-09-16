/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QMessageBox>
#include <QHBoxLayout>
#include <QVBoxLayout>

#include <parameter.h>
#include <xsqlquery.h>

#include "shiptocluster.h"

ShiptoEdit::ShiptoEdit(QWidget *pParent, const char *pName) :
   VirtualClusterLineEdit(pParent, "shiptoinfo", "shipto_id", "shipto_num", "shipto_name", "addr_line1", " (false) ", pName, "shipto_active")
{
  setTitles(tr("Ship To Address"), tr("Ship To Addresses"));
  setUiName("shipTo");
  setEditPriv("MaintainShiptos");
  setViewPriv("ViewShiptos");
  setNewPriv("MaintainShiptos");

  _custid = 0;
  setCustid(-1);

  _query = "SELECT shipto_id AS id, shipto_num AS number, shipto_name AS name, "
           "  addr_line1 AS description, "
           "  (addr_city || ', ' || addr_state || '  ' || addr_postalcode) AS csv,"
           "  shipto_active AS active "
           "FROM shiptoinfo "
           "  LEFT OUTER JOIN addr ON (shipto_addr_id=addr_id) "
           "WHERE (true) ";
}

void ShiptoEdit::setId(int pId)
{
  if (pId == _id)
    return;

  VirtualClusterLineEdit::setId(pId);

  if (pId != -1)
  {
      emit nameChanged(_name);
      emit address1Changed(_description);
  }
  else
  {
    _custid = -1;
    _extraClause = " (false) ";
    emit newCustid(_custid);
  }
}

void ShiptoEdit::setCustid(int pCustid)
{
  if (pCustid == _custid)
    return;

  clear();
  _custid = pCustid;

  if (_custid != -1)
  {
    _extraClause = QString(" (shipto_cust_id=%1) ").arg(_custid);
    setNewPriv("MaintainShiptos");
  }
  else
  {
    _extraClause = " (false) ";
    setNewPriv("");
  }

  sUpdateMenu();

  emit newCustid(pCustid);
  emit disableList(_custid == -1);
}

void ShiptoEdit::sNew()
{
  if (canOpen())
  {
    if (!_x_privileges->check(_newPriv))
      return;

    ParameterList params;
    params.append("mode", "new");
    params.append("cust_id", _custid);

    QWidget* w = 0;
    if (parentWidget()->window())
    {
      if (parentWidget()->window()->isModal())
        w = _guiClientInterface->openWindow(_uiName, params, parentWidget()->window() , Qt::WindowModal, Qt::Dialog);
      else
        w = _guiClientInterface->openWindow(_uiName, params, parentWidget()->window() , Qt::NonModal, Qt::Window);
    }

    if (w->inherits("QDialog"))
    {
      QDialog* newdlg = qobject_cast<QDialog*>(w);
      int id = newdlg->exec();
      if (id != QDialog::Rejected)
      {
        silentSetId(id);
        emit newId(_id);
        emit valid(_id != -1);
      }
    }
  }
}

void ShiptoEdit::sList()
{
  disconnect(this, SIGNAL(editingFinished()), this, SLOT(sParse()));

  shipToList* newdlg = listFactory();
  if (newdlg)
  {
    ParameterList params;
    params.append("cust_id", _custid);
    newdlg->set(params);

    int id = newdlg->exec();
    setId(id);
  }
  else
    QMessageBox::critical(this, tr("A System Error Occurred at %1::%2.")
                          .arg(__FILE__)
                          .arg(__LINE__),
                          tr("%1::sList() not yet defined")
                          .arg(metaObject()->className()));

  connect(this, SIGNAL(editingFinished()), this, SLOT(sParse()));
}

void ShiptoEdit::sSearch()
{
  disconnect(this, SIGNAL(editingFinished()), this, SLOT(sParse()));

  shipToSearch* newdlg = searchFactory();
  if (newdlg)
  {
    ParameterList params;
    params.append("cust_id", _custid);
    newdlg->set(params);
    newdlg->setSearchText(text());
    int id = newdlg->exec();
    setId(id);
  }
  else
    QMessageBox::critical(this, tr("A System Error Occurred at %1::%2.")
                          .arg(__FILE__)
                          .arg(__LINE__),
                          tr("%1::sSearch() not yet defined")
                          .arg(metaObject()->className()));

  connect(this, SIGNAL(editingFinished()), this, SLOT(sParse()));
}

shipToList* ShiptoEdit::listFactory()
{
  return new shipToList(this);
}

shipToSearch* ShiptoEdit::searchFactory()
{
  return new shipToSearch(this);
}

///////////////////////////////

ShiptoCluster::ShiptoCluster(QWidget *pParent, const char *pName) :
    VirtualCluster(pParent, pName)
{
  addNumberWidget(new ShiptoEdit(this, pName));
  setLabel(tr("Ship To#"));
  setNameVisible(true);
  setDescriptionVisible(true);

  setCustid(-1);
}

void ShiptoCluster::setCustid(int pId)
{
  static_cast<ShiptoEdit*>(_number)->setCustid(pId);
}

///////////////////////////////////////////////////////////////////////////////

shipToList::shipToList(QWidget* pParent, Qt::WindowFlags pFlags) :
    VirtualList(pParent, pFlags)
{
  setMinimumWidth(600);

  QLabel* custNumberLit = new QLabel(tr("Customer #:"), this);
  QLabel* custNameLit = new QLabel(tr("Name:"), this);
  QLabel* custAddrLit = new QLabel(tr("Address:"), this);
  _custNumber = new QLabel(this);
  _custName = new QLabel(this);
  _custAddr = new QLabel(this);
  QGridLayout* grid = new QGridLayout(this);
  grid->addWidget(custNumberLit,0,0,1,1,Qt::AlignRight);
  grid->addWidget(custNameLit,1,0,1,1,Qt::AlignRight);
  grid->addWidget(custAddrLit,2,0,1,1,Qt::AlignRight);
  grid->addWidget(_custNumber,0,1,1,1,Qt::AlignLeft);
  grid->addWidget(_custName,1,1,1,1,Qt::AlignLeft);
  grid->addWidget(_custAddr,2,1,1,1,Qt::AlignLeft);
  QVBoxLayout* searchLayout = findChild<QVBoxLayout*>("searchLyt");
  searchLayout->insertLayout(0,grid);

  _listTab->setColumnCount(2);
  _listTab->addColumn(tr("Address"),  75, Qt::AlignLeft,  true, "description");
  _listTab->addColumn(tr("City, State, Zip"),      100, Qt::AlignLeft,  true, "csv");
}

void shipToList::set(const ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("cust_id", &valid);
  if (valid)
  {
     XSqlQuery cust;
     cust.prepare("SELECT cust_number, cust_name, cust_address1 "
                  "FROM cust "
                  "WHERE (cust_id=:cust_id)");
     cust.bindValue(":cust_id", param.toInt());
     cust.exec();
     if (cust.first())
     {
       _custNumber->setText(cust.value("cust_number").toString());
       _custName->setText(cust.value("cust_name").toString());
       _custAddr->setText(cust.value("cust_address1").toString());
     }
  }
}

///////////////////////////

shipToSearch::shipToSearch(QWidget* pParent, Qt::WindowFlags pFlags)
    : VirtualSearch(pParent, pFlags)
{
  setMinimumWidth(600);

  _custid = -1;

  _listTab->setColumnCount(2);
  _listTab->addColumn(tr("Address"),  75, Qt::AlignLeft,  true, "description");
  _listTab->addColumn(tr("City, State, Zip"),      100, Qt::AlignLeft,  true, "csv");
}

void shipToSearch::set(const ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("cust_id", &valid);
  if (valid)
    _custid = param.toInt();

}



