/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "reserveSalesOrderItem.h"

#include <QApplication>
#include <QSqlError>
#include <QVariant>
#include <QValidator>

#include <metasql.h>

#include "xmessagebox.h"
#include "storedProcErrorLookup.h"

reserveSalesOrderItem::reserveSalesOrderItem(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_issue, SIGNAL(clicked()), this, SLOT(sIssue()));
  connect(_close, SIGNAL(clicked()), this, SLOT(reject()));

  _item->setReadOnly(TRUE);

  _qtyToIssue->setValidator(omfgThis->qtyVal());

  _qtyOrdered->setPrecision(omfgThis->qtyVal());
  _qtyShipped->setPrecision(omfgThis->qtyVal());
  _balance->setPrecision(omfgThis->qtyVal());
  _reserved->setPrecision(omfgThis->qtyVal());
  _onHand->setPrecision(omfgThis->qtyVal());
  _allocated->setPrecision(omfgThis->qtyVal());
  _unreserved->setPrecision(omfgThis->qtyVal());
}

reserveSalesOrderItem::~reserveSalesOrderItem()
{
  // no need to delete child widgets, Qt does it all for us
}

void reserveSalesOrderItem::languageChange()
{
  retranslateUi(this);
}

enum SetResponse reserveSalesOrderItem::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("soitem_id", &valid);
  if (valid)
  {
    _itemid = param.toInt();
    //_orderNumberLit->setText(tr("Sales Order #:"));
    populate();
  }

  param = pParams.value("qty", &valid);
  if (valid)
    _qtyToIssue->setDouble(param.toDouble());

  return NoError;
}

void reserveSalesOrderItem::sIssue()
{
  if (_qtyToIssue->toDouble() <= 0)
  {
    QMessageBox::warning( this, tr("Invalid Quantity to Issue to Shipping"),
                          tr(  "<p>Please enter a non-negative, non-zero value to indicate the amount "
                               "of Stock you wish to Reserve for this Order Line." ));
    _qtyToIssue->setFocus();
    return;
  }

  XSqlQuery issue;
  issue.prepare("SELECT reserveSoLineQty(:lineitem_id, :qty) AS result;");
  issue.bindValue(":lineitem_id", _itemid);
  issue.bindValue(":qty", _qtyToIssue->toDouble());
  issue.exec();

  if (issue.first())
  {
    int result = issue.value("result").toInt();
    if (result < 0)
    {
      systemError( this, storedProcErrorLookup("reserveSoLineQty", result),
		  __FILE__, __LINE__);
      return;
    }
  }
  else if (issue.lastError().type() != QSqlError::NoError)
  {
    systemError(this, issue.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  accept();
}

void reserveSalesOrderItem::populate()
{
  ParameterList itemp;
  itemp.append("soitem_id", _itemid);

  QString sql = "SELECT cohead_number AS order_number,"
                "       coitem_linenumber,"
		"       itemsite_item_id AS item_id,"
		"       warehous_code,"
                "       uom_name,"
                "       itemsite_qtyonhand,"
                "       qtyReserved(itemsite_id) AS totreserved,"
                "       qtyUnreserved(itemsite_id) AS totunreserved,"
		"       coitem_qtyord,"
		"       coitem_qtyshipped,"
		"       formatQty(coitem_qtyreturned) AS qtyreturned,"
                "       coitem_qtyreserved,"
		"       noNeg(coitem_qtyord - coitem_qtyshipped +"
		"             coitem_qtyreturned - coitem_qtyreserved) AS balance "
		"FROM cohead, coitem, itemsite, item, warehous, uom "
		"WHERE ((coitem_cohead_id=cohead_id)"
		"  AND  (coitem_itemsite_id=itemsite_id)"
		"  AND  (coitem_status <> 'X')"
                "  AND  (coitem_qty_uom_id=uom_id)"
		"  AND  (itemsite_item_id=item_id)"
		"  AND  (itemsite_warehous_id=warehous_id)"
		"  AND  (coitem_id=<? value(\"soitem_id\") ?>) );";

  MetaSQLQuery itemm(sql);
  XSqlQuery itemq = itemm.toQuery(itemp);

  if (itemq.first())
  {
    _salesOrderNumber->setText(itemq.value("order_number").toString());
    _salesOrderLine->setText(itemq.value("coitem_linenumber").toString());
    _item->setId(itemq.value("item_id").toInt());
    _warehouse->setText(itemq.value("warehous_code").toString());
    _qtyUOM->setText(itemq.value("uom_name").toString());
    _qtyOrdered->setDouble(itemq.value("coitem_qtyord").toDouble());
    _qtyShipped->setDouble(itemq.value("coitem_qtyshipped").toDouble());
    _balance->setDouble(itemq.value("balance").toDouble());
    _reserved->setDouble(itemq.value("coitem_qtyreserved").toDouble());
    _onHand->setDouble(itemq.value("itemsite_qtyonhand").toDouble());
    _allocated->setDouble(itemq.value("totreserved").toDouble());
    _unreserved->setDouble(itemq.value("totunreserved").toDouble());
  }
  else if (itemq.lastError().type() != QSqlError::NoError)
  {
    systemError(this, itemq.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  _qtyToIssue->setDouble(itemq.value("balance").toDouble());
}
