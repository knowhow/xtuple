/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "shippingInformation.h"

#include <QSqlError>
#include <QMessageBox>
#include <QVariant>
#include <QDebug>

#include <metasql.h>

#include "issueToShipping.h"
#include "salesOrderItem.h"
#include "storedProcErrorLookup.h"
#include "transferOrderItem.h"

shippingInformation::shippingInformation(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);
  connect(_item,	SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)),
					  this, SLOT(sPopulateMenu(QMenu*)));
  connect(_order, SIGNAL(newId(int, QString)), this, SLOT(sFillList()));
  connect(_buttonBox,	SIGNAL(accepted()), this, SLOT(sSave()));

  _captive = FALSE;

  _shippingForm->setCurrentIndex(-1);

  _item->addColumn(tr("#"),           _seqColumn, Qt::AlignCenter, true,  "linenumber" );
  _item->addColumn(tr("Item"),        -1,         Qt::AlignLeft,   true,  "item_number"   );
  _item->addColumn(tr("At Shipping"), _qtyColumn, Qt::AlignRight,  true,  "qtyatshipping"  );
  _item->addColumn(tr("Net Wght."),   _qtyColumn, Qt::AlignRight,  true,  "netweight"  );
  _item->addColumn(tr("Tare Wght."),  _qtyColumn, Qt::AlignRight,  true,  "tareweight"  );
  _item->addColumn(tr("Gross Wght."), _qtyColumn, Qt::AlignRight,  true,  "grossweight"  );

  // Issue #11398 - Shipping charge data on shipment goes nowhere and logically can't because multiple
  // shipments converge on one billing selection.  Hide shipping charges field for now until we can
  // understand how this should work.
  _shippingCharges->hide();
  _shippingChargesLit->hide();
}

shippingInformation::~shippingInformation()
{
  // no need to delete child widgets, Qt does it all for us
}

void shippingInformation::languageChange()
{
  retranslateUi(this);
}

enum SetResponse shippingInformation::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("cosmisc_id", &valid);	// deprecated
  if (valid)
    _shipment->setId(param.toInt());

  param = pParams.value("shiphead_id", &valid);
  if (valid)
    _shipment->setId(param.toInt());

  if (_shipment->isValid())
  {
    q.prepare( "SELECT shiphead_order_id, shiphead_order_type "
               "FROM shiphead "
               "WHERE (shiphead_id=:shiphead_id);" );
    q.bindValue(":shiphead_id", _shipment->id());
    q.exec();
    if (q.first())
    {
      _captive = TRUE;

      _order->setId(q.value("shiphead_order_id").toInt(),
                    q.value("shiphead_order_type").toString());

      _shipDate->setFocus();
    }
    else if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return UndefinedError;
    }
  }

  return NoError;
}

void shippingInformation::sSave()
{
  if (!_shipDate->isValid())
  {
    QMessageBox::information( this, tr("No Ship Date Entered"),
                              tr("<p>You must enter a Ship Date before "
				 "selecting this order for billing.") );

    _shipDate->setFocus();
    return;
  }

  if (_shipment->id() > 0)
  {
    q.prepare( "UPDATE shiphead "
               "SET shiphead_freight=:shiphead_freight,"
	       "    shiphead_freight_curr_id=:shiphead_freight_curr_id,"
	       "    shiphead_notes=:shiphead_notes,"
               "    shiphead_shipdate=:shiphead_shipdate, shiphead_shipvia=:shiphead_shipvia,"
               "    shiphead_shipchrg_id=:shiphead_shipchrg_id, shiphead_shipform_id=:shiphead_shipform_id "
               "WHERE (shiphead_id=:shiphead_id);" );
    q.bindValue(":shiphead_id", _shipment->id());
  }
  else
    q.prepare( "INSERT INTO shiphead "
               "( shiphead_order_id, shiphead_order_type, shiphead_freight,"
	       "  shiphead_freight_curr_id, shiphead_notes,"
               "  shiphead_shipdate, shiphead_shipvia, shiphead_number,"
               "  shiphead_shipchrg_id, shiphead_shipform_id, shiphead_shipped ) "
               "VALUES "
               "( :shiphead_order_id, :shiphead_order_type, :shiphead_freight,"
	       "  :shiphead_freight_curr_id, :shiphead_notes,"
               "  :shiphead_shipdate, :shiphead_shipvia, fetchShipmentNumber(),"
               "  :shiphead_shipchrg_id, :shiphead_shipform_id, FALSE );" );

  q.bindValue(":shiphead_order_id",		_order->id());
  q.bindValue(":shiphead_order_type",           _order->type());
  q.bindValue(":shiphead_freight",		_freight->localValue());
  q.bindValue(":shiphead_freight_curr_id",	_freight->id());
  q.bindValue(":shiphead_notes", _notes->toPlainText());
  q.bindValue(":shiphead_shipdate", _shipDate->date());
  q.bindValue(":shiphead_shipvia", _shipVia->currentText());
  q.bindValue(":shiphead_shipchrg_id", _shippingCharges->id());
  q.bindValue(":shiphead_shipform_id", _shippingForm->id());
  q.exec();
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  if (_captive)
    accept();
  else
    _order->setId(-1, QString::null);
}

void shippingInformation::sPopulateMenu(QMenu *menuThis)
{
  QAction * act = menuThis->addAction(tr("Issue Additional Stock for this Order Line to Shipping..."), this, SLOT(sIssueStock()));
  if(_order->isClosed())
    act->setEnabled(false);
  menuThis->addAction(tr("Return ALL Stock Issued for this Order Line to the Site..."), this, SLOT(sReturnAllLineStock()));
  menuThis->addAction(tr("View Order Line..."), this, SLOT(sViewLine()));
}

void shippingInformation::sIssueStock()
{
  ParameterList params;
  if (_order->isValid() && _order->type() == "SO")
    params.append("sohead_id", _item->altId());
  else if (_order->isValid() && _order->type() == "TO")
    params.append("tohead_id", _item->altId());

  issueToShipping *newdlg = new issueToShipping(this);
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void shippingInformation::sReturnAllLineStock()
{
  q.prepare("SELECT returnItemShipments(:ordertype, :lineitemid,"
	    "                           0, CURRENT_TIMESTAMP) AS result;");
  q.bindValue(":ordertype", _order->type());
  q.bindValue(":lineitemid", _item->id());
  q.exec();
  if (q.first())
  {
    int result = q.value("result").toInt();
    if (result < 0)
    {
      systemError(this, storedProcErrorLookup("returnItemShipments", result), __FILE__, __LINE__);
      return;
    }
  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  sFillList();
}

void shippingInformation::sViewLine()
{
  ParameterList params;
  params.append("mode", "view");
  if (_order->isValid() && _order->type() == "SO")
  {
    params.append("soitem_id", _item->id());

    salesOrderItem newdlg(this);
    newdlg.set(params);
    newdlg.exec();
  }
  else if (_order->isValid() && _order->type() == "TO")
  {
    params.append("toitem_id", _item->id());

    transferOrderItem newdlg(this, "", TRUE);
    newdlg.set(params);
    newdlg.exec();
  }
}

void shippingInformation::sFillList()
{
  QString shs( "SELECT shiphead_notes, shiphead_shipvia,"
               "       shiphead_shipchrg_id, shiphead_shipform_id,"
               "       shiphead_freight, shiphead_freight_curr_id,"
               "       shiphead_shipdate,"
               "       CASE WHEN (shiphead_shipdate IS NULL) THEN FALSE"
               "            ELSE TRUE"
               "       END AS validdata "
               "FROM shiphead "
               "WHERE ((NOT shiphead_shipped)"
	       "  AND  (shiphead_order_type=<? value(\"ordertype\") ?>)"
               "  AND  (shiphead_order_id=<? value(\"orderid\") ?>) );" ) ;
  ParameterList shp;
  if (_order->isValid())
  {
    shp.append("ordertype", _order->type());
    shp.append("orderid",   _order->id());
  }
  else
  {
    _order->setId(-1, "");
    _orderDate->setNull();
    _poNumber->clear();
    _custName->clear();
    _custPhone->clear();
    _shipDate->setNull();
    _shipVia->setNull();

    _freight->reset();
    return;
  }

  MetaSQLQuery shm(shs);
  q = shm.toQuery(shp);
  bool fetchFromHead = TRUE;

  if (q.first())
  {
    if (q.value("validdata").toBool())
    {
      fetchFromHead = FALSE;

      _shipDate->setDate(q.value("shiphead_shipdate").toDate());
      _shipVia->setText(q.value("shiphead_shipvia").toString());
      _shippingCharges->setId(q.value("shiphead_shipchrg_id").toInt());
      _shippingForm->setId(q.value("shiphead_shipform_id").toInt());
      _freight->setId(q.value("shiphead_freight_curr_id").toInt());
      _freight->setLocalValue(q.value("shiphead_freight").toDouble());
      _notes->setText(q.value("shiphead_notes").toString());
    }
  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  if (_order->isValid() && _order->type() == "SO")
  {
    q.prepare( "SELECT cohead_orderdate AS orderdate,"
	       "       cohead_holdtype AS holdtype,"
               "       cohead_custponumber AS ponumber,"
               "       cust_name AS name, cust_phone AS phone,"
               "       cohead_shipcomments AS shipcomments,"
	       "       cohead_shipvia AS shipvia,"
               "       cohead_shipchrg_id AS shipchrg_id,"
               "       cohead_shipform_id AS shipform_id, "
               "       cohead_shiptoname AS shiptoname "
               "FROM cohead, cust "
               "WHERE ((cohead_cust_id=cust_id)"
               " AND (cohead_id=:cohead_id));" );
    q.bindValue(":cohead_id", _order->id());
  }
  else if (_order->isValid() && _order->type() == "TO")
  {
    q.prepare( "SELECT tohead_orderdate AS orderdate,"
	       "       'N' AS holdtype,"
	       "       :to AS ponumber,"
	       "       tohead_destname AS name, tohead_destphone AS phone,"
	       "       tohead_shipcomments AS shipcomments,"
	       "       tohead_shipvia AS shipvia,"
	       "       tohead_shipchrg_id AS shipchrg_id,"
               "       tohead_shipform_id AS shipform_id, "
               "       tohead_destname AS shiptoname "
	       "FROM tohead "
	       "WHERE (tohead_id=:tohead_id);" );
    q.bindValue(":tohead_id", _order->id());
  }
  q.exec();
  if (q.first())
  {
    _orderDate->setDate(q.value("orderdate").toDate());
    _poNumber->setText(q.value("ponumber").toString());
    _custName->setText(q.value("name").toString());
    _custPhone->setText(q.value("phone").toString());
    _shipToName->setText(q.value("shiptoname").toString());

    QString msg;
    if (q.value("head_holdtype").toString() == "C")
      msg = storedProcErrorLookup("issuetoshipping", -12);
    else if (q.value("head_holdtype").toString() == "P")
      msg = storedProcErrorLookup("issuetoshipping", -13);
    else if (q.value("head_holdtype").toString() == "R")
      msg = storedProcErrorLookup("issuetoshipping", -14);

    if (! msg.isEmpty())
    {
      QMessageBox::warning(this, tr("Order on Hold"), msg);
      _order->setId(-1, "");
      _order->setFocus();
      return;
    }

    if (fetchFromHead)
    {
      _shipDate->setDate(omfgThis->dbDate());

      _shippingCharges->setId(q.value("shipchrg_id").toInt());
      _shippingForm->setId(q.value("shipform_id").toInt());
      _notes->setText(q.value("shipcomments").toString());
      _shipVia->setText(q.value("shipvia").toString());
    }
  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  ParameterList params;
  params.append("order_id", _order->id());
  params.append(_order->type());

  MetaSQLQuery mql("SELECT itemid, headid,"
                   "       linenumber, item_number,"
                   "       netweight, tareweight, (netweight + tareweight) AS grossweight,"
                   "       qtyatshipping,"
                   "       'weight' AS netweight_xtnumericrole,"
                   "       'weight' AS tareweight_xtnumericrole,"
                   "       'weight' AS grossweight_xtnumericrole,"
                   "       0        AS netweight_xttotalrole,"
                   "       0        AS tareweight_xttotalrole,"
                   "       0        AS grossweight_xttotalrole,"
                   "       'qty' AS qtyatshipping_xtnumericrole "
                   "FROM ( "
                   "<? if exists('SO') ?>"
                   "SELECT coitem_id AS itemid, coitem_cohead_id AS headid,"
                   "       coitem_linenumber AS linenumber, item_number,"
                   "      (item_prodweight * itemuomtouom(item_id, coitem_price_uom_id, NULL, qtyAtShipping('SO', coitem_id))) AS netweight,"
                   "      (item_packweight * itemuomtouom(item_id, coitem_price_uom_id, NULL, qtyAtShipping('SO', coitem_id))) AS tareweight,"
                   "      qtyAtShipping('SO', coitem_id) AS qtyatshipping "
                   "FROM coitem, itemsite, item "
                   "WHERE ((coitem_itemsite_id=itemsite_id)"
                   "  AND  (itemsite_item_id=item_id)"
                   "  AND  (coitem_cohead_id=<? value('order_id') ?>) ) "
                   "<? elseif exists('TO') ?>"
                   "SELECT toitem_id AS itemid, toitem_tohead_id AS headid,"
                   "       toitem_linenumber AS linenumber, item_number,"
                   "      (item_prodweight * qtyAtShipping('TO', toitem_id)) AS netweight,"
                   "      (item_packweight * qtyAtShipping('TO', toitem_id)) AS tareweight,"
                   "      qtyAtShipping('TO', toitem_id) AS qtyatshipping "
                   "FROM toitem, item "
                   "WHERE ((toitem_item_id=item_id)"
                   "  AND  (toitem_tohead_id=<? value('order_id') ?>) ) "
                   "<? endif ?>"
                   "  ) AS data "
                   "ORDER BY linenumber;");
         
  XSqlQuery qry = mql.toQuery(params);
  _item->populate(qry, true);

  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}
