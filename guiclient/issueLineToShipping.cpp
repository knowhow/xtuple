/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "issueLineToShipping.h"

#include <QApplication>
#include <QSqlError>
#include <QVariant>
#include <QValidator>

#include <metasql.h>

#include "xmessagebox.h"
#include "distributeInventory.h"
#include "storedProcErrorLookup.h"

issueLineToShipping::issueLineToShipping(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_issue, SIGNAL(clicked()), this, SLOT(sIssue()));
  connect(_close, SIGNAL(clicked()), this, SLOT(reject()));

  _requireInventory = false;
  _snooze = false;
  _transTS = QDateTime::currentDateTime();
  _item->setReadOnly(TRUE);

  _qtyToIssue->setValidator(omfgThis->qtyVal());
  _qtyOrdered->setPrecision(omfgThis->qtyVal());
  _qtyShipped->setPrecision(omfgThis->qtyVal());
  _qtyReturned->setPrecision(omfgThis->qtyVal());
  _balance->setPrecision(omfgThis->qtyVal());
  _qtyAtShip->setPrecision(omfgThis->qtyVal());
  
  adjustSize();
}

issueLineToShipping::~issueLineToShipping()
{
  // no need to delete child widgets, Qt does it all for us
}

void issueLineToShipping::languageChange()
{
  retranslateUi(this);
}

enum SetResponse issueLineToShipping::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("transTS", &valid);
  if (valid)
    _transTS = param.toDateTime();

  param = pParams.value("order_type", &valid);
  if (valid)
  {
    _ordertype = param.toString();
    if (_ordertype == "SO")
      _orderNumberLit->setText(tr("Sales Order #:"));
    else if (_ordertype == "TO")
      _orderNumberLit->setText(tr("Transfer Order #:"));
  }

  param = pParams.value("order_id", &valid);
  if (valid)
  {
    _itemid = param.toInt();
    populate();
  }

  // TODO: deprecate by remoing from salesOrder and transferOrder windows
  param = pParams.value("soitem_id", &valid);
  if (valid)
  {
    _itemid = param.toInt();
    _ordertype = "SO";
    _orderNumberLit->setText(tr("Sales Order #:"));
    populate();
  }

  // TODO: deprecate by remoing from salesOrder and transferOrder windows
  param = pParams.value("toitem_id", &valid);
  if (valid)
  {
    _itemid = param.toInt();
    _ordertype = "TO";
    _orderNumberLit->setText(tr("Transfer Order #:"));
    populate();
  }

  if (pParams.inList("requireInventory"))
    _requireInventory = true;

  param = pParams.value("qty", &valid);
  if (valid)
    _qtyToIssue->setDouble(param.toDouble());

  _snooze = pParams.inList("snooze");

  if(pParams.inList("issue"))
    sIssue();

  return NoError;
}

void issueLineToShipping::sIssue()
{
  if (_qtyToIssue->toDouble() <= 0)
  {
    XMessageBox::message( (isVisible() ? this : parentWidget()), QMessageBox::Warning, tr("Invalid Quantity to Issue to Shipping"),
                          tr(  "<p>Please enter a non-negative, non-zero value to indicate the amount "
                               "of Stock you wish to Issue to Shipping for this Order Line." ),
                          QString::null, QString::null, _snooze );
    _qtyToIssue->setFocus();
    return;
  }

  if(_requireInventory || ("SO" == _ordertype && _metrics->boolean("EnableSOReservations")))
  {
    q.prepare("SELECT sufficientInventoryToShipItem(:ordertype, :orderitemid, :orderqty) AS result;");
    q.bindValue(":ordertype",   _ordertype);
    q.bindValue(":orderitemid", _itemid);
    q.bindValue(":orderqty",  _qtyToIssue->toDouble());
    q.exec();
    if (q.first())
    {
      int result = q.value("result").toInt();
      if (result < 0)
      {
        ParameterList errp;
        if (_ordertype == "SO")
          errp.append("soitem_id", _itemid);
        else if (_ordertype == "TO")
          errp.append("toitem_id", _itemid);

        QString errs = "<? if exists(\"soitem_id\") ?>"
            "SELECT item_number, warehous_code "
            "  FROM coitem, item, itemsite, whsinfo "
            " WHERE ((coitem_itemsite_id=itemsite_id)"
            "   AND  (itemsite_item_id=item_id)"
            "   AND  (itemsite_warehous_id=warehous_id)"
            "   AND  (coitem_id=<? value(\"soitem_id\") ?>));"
            "<? elseif exists(\"toitem_id\")?>"
            "SELECT item_number, tohead_srcname AS warehous_code "
            "  FROM toitem, tohead, item "
            " WHERE ((toitem_item_id=item_id)"
            "   AND  (toitem_tohead_id=tohead_id)"
            "   AND  (toitem_id=<? value(\"toitem_id\") ?>));"
            "<? endif ?>" ;
        MetaSQLQuery errm(errs);
        q = errm.toQuery(errp);
        if (! q.first() && q.lastError().type() != QSqlError::NoError)
            systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
        systemError(this,
              storedProcErrorLookup("sufficientInventoryToShipItem",
                  result)
              .arg(q.value("item_number").toString())
              .arg(q.value("warehous_code").toString()), __FILE__, __LINE__);
        return;
      }
    }
    else if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }

  // check to see if we are over issuing
  ParameterList params;
  if (_ordertype == "SO")
    params.append("soitem_id", _itemid);
  else if (_ordertype == "TO")
    params.append("toitem_id", _itemid);
  params.append("qty", _qtyToIssue->toDouble());

  QString sql = "<? if exists(\"soitem_id\") ?>"
                "SELECT  itemsite_costmethod,"
                "  (noNeg(coitem_qtyord - coitem_qtyshipped + coitem_qtyreturned) <"
                "           (COALESCE(SUM(shipitem_qty), 0) + <? value(\"qty\") ?>)) AS overship"
                "  FROM coitem LEFT OUTER JOIN"
                "        ( shipitem JOIN shiphead"
                "          ON ( (shipitem_shiphead_id=shiphead_id) AND (NOT shiphead_shipped) )"
                "        ) ON  (shipitem_orderitem_id=coitem_id)"
                "  JOIN itemsite ON (coitem_itemsite_id=itemsite_id) "
                " WHERE (coitem_id=<? value(\"soitem_id\") ?>)"
                " GROUP BY coitem_qtyord, coitem_qtyshipped, coitem_qtyreturned, "
                "   itemsite_costmethod, itemsite_controlmethod;"
                "<? elseif exists(\"toitem_id\") ?>"
                "SELECT false AS postprod,"
                "  (noNeg(toitem_qty_ordered - toitem_qty_shipped) <"
                "           (COALESCE(SUM(shipitem_qty), 0) + <? value(\"qty\") ?>)) AS overship"
                "  FROM toitem LEFT OUTER JOIN"
                "        ( shipitem JOIN shiphead"
                "          ON ( (shipitem_shiphead_id=shiphead_id) AND (NOT shiphead_shipped) )"
                "        ) ON  (shipitem_orderitem_id=toitem_id)"
                " WHERE (toitem_id=<? value(\"toitem_id\") ?>)"
                " GROUP BY toitem_qty_ordered, toitem_qty_shipped;"
                "<? endif ?>";
  MetaSQLQuery mql(sql);
  q = mql.toQuery(params);
  if (q.next() && q.value("overship").toBool())
  {
    if(XMessageBox::message( (isVisible() ? this : parentWidget()) , QMessageBox::Question, tr("Inventory Overshipped"),
        tr("<p>You have selected to ship more inventory than required. Do you want to continue?"),
        tr("Yes"), tr("No"), _snooze, 0, 1) == 1)
      return;
  }
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  XSqlQuery rollback;
  rollback.prepare("ROLLBACK;");

  int invhistid = 0;
  int itemlocSeries = 0;

  XSqlQuery issue;
  issue.exec("BEGIN;");

  // If this is a lot/serial controlled job item, we need to post production first
  if (q.value("itemsite_costmethod").toString() == "J")
  {
    XSqlQuery prod;
    prod.prepare("SELECT postSoItemProduction(:soitem_id, :qty, :ts) AS result;");
    prod.bindValue(":soitem_id", _itemid);
    prod.bindValue(":qty", _qtyToIssue->toDouble());
    prod.bindValue(":ts", _transTS);
    prod.exec();
    if (prod.first())
    {
      itemlocSeries = prod.value("result").toInt();

      if (itemlocSeries < 0)
      {
        rollback.exec();
        systemError(this, storedProcErrorLookup("postProduction", itemlocSeries),
                    __FILE__, __LINE__);
        return;
      }
      else if (distributeInventory::SeriesAdjust(itemlocSeries, this) == XDialog::Rejected)
      {
        rollback.exec();
        QMessageBox::information( this, tr("Issue to Shipping"), tr("Issue Canceled") );
        return;
      }

      // Need to get the inventory history id so we can auto reverse the distribution when issuing
      prod.prepare("SELECT invhist_id "
                "FROM invhist "
                "WHERE ((invhist_series = :itemlocseries) "
                " AND (invhist_transtype = 'RM')); ");
      prod.bindValue(":itemlocseries" , itemlocSeries);
      prod.exec();
      if (prod.first())
        invhistid = prod.value("invhist_id").toInt();
      else
      {
        rollback.exec();
        systemError(this, tr("Inventory history not found"),
                    __FILE__, __LINE__);
        return;
      }
    }
  }

  issue.prepare("SELECT issueToShipping(:ordertype, :lineitem_id, :qty, :itemlocseries, :ts, :invhist_id) AS result;");
  issue.bindValue(":ordertype",   _ordertype);
  issue.bindValue(":lineitem_id", _itemid);
  issue.bindValue(":qty",         _qtyToIssue->toDouble());
  issue.bindValue(":ts",          _transTS);
  if (invhistid)
    issue.bindValue(":invhist_id", invhistid);
  issue.bindValue(":itemlocseries", itemlocSeries);
  issue.exec();

  if (issue.first())
  {
    int result = issue.value("result").toInt();
    if (result < 0)
    {
      rollback.exec();
      systemError( this, storedProcErrorLookup("issueToShipping", result),
		  __FILE__, __LINE__);
      return;
    }
    else
    {
      if (distributeInventory::SeriesAdjust(result, this) == XDialog::Rejected)
      {
        rollback.exec();
        QMessageBox::information( this, tr("Issue to Shipping"), tr("Issue Canceled") );
        return;
      }
	
	  // If Transfer Order then insert special pre-assign records for the lot/serial#
	  // so they are available when the Transfer Order is received
	  if (_ordertype == "TO")
	  {
        XSqlQuery lsdetail;
        lsdetail.prepare("INSERT INTO lsdetail "
	                     "            (lsdetail_itemsite_id, lsdetail_created, lsdetail_source_type, "
	  	  			     "             lsdetail_source_id, lsdetail_source_number, lsdetail_ls_id, lsdetail_qtytoassign) "
					     "SELECT invhist_itemsite_id, NOW(), 'TR', "
					     "       :orderitemid, invhist_ordnumber, invdetail_ls_id, (invdetail_qty * -1.0) "
					     "FROM invhist JOIN invdetail ON (invdetail_invhist_id=invhist_id) "
					     "WHERE (invhist_series=:itemlocseries);");
        lsdetail.bindValue(":orderitemid", _itemid);
        lsdetail.bindValue(":itemlocseries", result);
        lsdetail.exec();
        if (lsdetail.lastError().type() != QSqlError::NoError)
        {
          rollback.exec();
          systemError(this, lsdetail.lastError().databaseText(), __FILE__, __LINE__);
          return;
        }
	  }
	
      issue.exec("COMMIT;");
      accept();
    }
  }
  else if (issue.lastError().type() != QSqlError::NoError)
  {
    rollback.exec();
    systemError(this, issue.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void issueLineToShipping::populate()
{
  ParameterList itemp;
  if (_ordertype == "SO")
    itemp.append("soitem_id", _itemid);
  else if (_ordertype == "TO")
    itemp.append("toitem_id", _itemid);
  itemp.append("ordertype", _ordertype);

  // TODO: make this an orderitem select
  QString sql = "<? if exists(\"soitem_id\") ?>"
		"SELECT cohead_number AS order_number,"
		"       itemsite_item_id AS item_id,"
		"       warehous_code, uom_name,"
		"       coitem_qtyord AS qtyordered,"
    "       coitem_qtyshipped AS qtyshipped,"
    "       coitem_qtyreturned AS qtyreturned,"
		"       noNeg(coitem_qtyord - coitem_qtyshipped +"
		"             coitem_qtyreturned) AS balance "
		"FROM cohead, coitem, itemsite, item, warehous, uom "
		"WHERE ((coitem_cohead_id=cohead_id)"
		"  AND  (coitem_itemsite_id=itemsite_id)"
		"  AND  (coitem_status <> 'X')"
                "  AND  (coitem_qty_uom_id=uom_id)"
		"  AND  (itemsite_item_id=item_id)"
		"  AND  (itemsite_warehous_id=warehous_id)"
		"  AND  (coitem_id=<? value(\"soitem_id\") ?>) );"
		"<? elseif exists(\"toitem_id\") ?>"
		"SELECT tohead_number AS order_number,"
		"       toitem_item_id AS item_id,"
		"       warehous_code, toitem_uom AS uom_name,"
		"       toitem_qty_ordered AS qtyordered,"
		"       toitem_qty_shipped AS qtyshipped,"
		"       0 AS qtyreturned,"
		"       noNeg(toitem_qty_ordered -"
		"             toitem_qty_shipped) AS balance "
		"FROM tohead, toitem, warehous, item "
		"WHERE ((toitem_tohead_id=tohead_id)"
		"  AND  (toitem_status <> 'X')"
		"  AND  (tohead_src_warehous_id=warehous_id)"
		"  AND  (toitem_id=<? value(\"toitem_id\") ?>) );"
		"<? endif ?>";

  MetaSQLQuery itemm(sql);
  XSqlQuery itemq = itemm.toQuery(itemp);

  if (itemq.first())
  {
    _orderNumber->setText(itemq.value("order_number").toString());
    _item->setId(itemq.value("item_id").toInt());
    _warehouse->setText(itemq.value("warehous_code").toString());
    _shippingUOM->setText(itemq.value("uom_name").toString());
    _qtyOrdered->setDouble(itemq.value("qtyordered").toDouble());
    _qtyShipped->setDouble(itemq.value("qtyshipped").toDouble());
    _qtyReturned->setDouble(itemq.value("qtyreturned").toDouble());
    _balance->setDouble(itemq.value("balance").toDouble());
  }
  else if (itemq.lastError().type() != QSqlError::NoError)
  {
    systemError(this, itemq.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  ParameterList shipp;
  shipp.append("ordertype", _ordertype);
  shipp.append("orderitem_id", _itemid);

  sql = "SELECT shiphead_id AS misc_id,"
        "       SUM(shipitem_qty) AS qtyatship "
        "FROM shiphead, shipitem "
        "WHERE ((shipitem_shiphead_id=shiphead_id)"
        "  AND  (NOT shiphead_shipped)"
        "  AND  (shiphead_order_type=<? value(\"ordertype\") ?>)"
        "  AND  (shipitem_orderitem_id=<? value(\"orderitem_id\") ?>) ) "
        "GROUP BY shiphead_id;" ;

  MetaSQLQuery shipm(sql);
  XSqlQuery shipq = shipm.toQuery(shipp);

  if (shipq.first())
  {
    _shipment->setType(_ordertype);
    _shipment->setId(shipq.value("misc_id").toInt());
    _qtyAtShip->setDouble(shipq.value("qtyatship").toDouble());
  }
  else if (shipq.lastError().type() != QSqlError::NoError)
  {
    systemError( this, shipq.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  if (_qtyAtShip->toDouble() == 0.0)
    _qtyToIssue->setDouble(itemq.value("balance").toDouble());
}
