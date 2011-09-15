/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "selectOrderForBilling.h"

#include <QMessageBox>
#include <QSqlError>
//#include <QStatusBar>
#include <QVariant>

#include <metasql.h>

#include "salesOrder.h"
#include "salesOrderList.h"
#include "selectBillingQty.h"
#include "storedProcErrorLookup.h"
#include "taxBreakdown.h"

selectOrderForBilling::selectOrderForBilling(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

//  (void)statusBar();

  _so->setAllowedTypes(OrderLineEdit::Sales);
  _so->setAllowedStatuses(OrderLineEdit::Open);

  connect(_cancel, SIGNAL(clicked()), this, SLOT(sCancelSelection()));
  connect(_edit, SIGNAL(clicked()), this, SLOT(sEditOrder()));
  connect(_freight, SIGNAL(valueChanged()), this, SLOT(sFreightChanged()));
  connect(_miscCharge, SIGNAL(valueChanged()), this, SLOT(sUpdateTotal()));
  connect(_salesTaxLit, SIGNAL(leftClickedURL(const QString&)), this, SLOT(sTaxDetail()));
  connect(_save, SIGNAL(clicked()), this, SLOT(sSave()));
  connect(_select, SIGNAL(clicked()), this, SLOT(sEditSelection()));
  connect(_selectBalance, SIGNAL(clicked()), this, SLOT(sSelectBalance()));
  connect(_showClosed, SIGNAL(toggled(bool)), this, SLOT(sFillList()));
  connect(_so, SIGNAL(newId(int,QString)), this, SLOT(sPopulate(int)));
  connect(_salesTax,	SIGNAL(valueChanged()),	this, SLOT(sUpdateTotal()));
  connect(_subtotal,	SIGNAL(valueChanged()),	this, SLOT(sUpdateTotal()));
  connect(_taxZone,	SIGNAL(newID(int)),	this, SLOT(sTaxZoneChanged()));
  
  _cobmiscid = -1;
  _taxzoneidCache = -1;
  _captive = FALSE;
  _updated = FALSE;
  _freightCache = 0;

  _custCurrency->setLabel(_custCurrencyLit);

  _freight->clear();
  _payment->clear();

  _soitem->addColumn(tr("#"),          _seqColumn,   Qt::AlignCenter, true, "linenumber" );
  _soitem->addColumn(tr("Item"),       -1,           Qt::AlignLeft  , true, "item_number" );
  _soitem->addColumn(tr("Site"),       _whsColumn,   Qt::AlignCenter, true, "warehous_code" );
  _soitem->addColumn(tr("UOM"),        _uomColumn,   Qt::AlignLeft  , true, "uom_name" );
  _soitem->addColumn(tr("Ordered"),    _qtyColumn,   Qt::AlignRight , true, "coitem_qtyord" );
  _soitem->addColumn(tr("Shipped"),    _qtyColumn,   Qt::AlignRight , true, "coitem_qtyshipped" );
  _soitem->addColumn(tr("Returned"),   _qtyColumn,   Qt::AlignRight , true, "coitem_qtyreturned" );
  _soitem->addColumn(tr("Uninvoiced"), _qtyColumn,   Qt::AlignRight , true, "qtyatship" );
  _soitem->addColumn(tr("Selected"),   _qtyColumn,   Qt::AlignRight , true, "qtytobill" );
  _soitem->addColumn(tr("Extended"),   _moneyColumn, Qt::AlignRight , true, "extended" );
  _soitem->addColumn(tr("Close"),      _ynColumn,    Qt::AlignCenter, true, "toclose");

  if (_privileges->check("MaintainSalesOrders"))
    connect(_so, SIGNAL(valid(bool)), _edit, SLOT(setEnabled(bool)));

  if(!_privileges->check("AllowSelectOrderEditing"))
  {
    _miscCharge->setEnabled(false);
    _miscChargeDescription->setEnabled(false);
    _miscChargeAccount->setEnabled(false);
  }
  else
  {
    connect(_soitem, SIGNAL(valid(bool)), _select, SLOT(setEnabled(bool)));
    connect(_soitem, SIGNAL(valid(bool)), _selectBalance, SLOT(setEnabled(bool)));
    connect(_soitem, SIGNAL(valid(bool)), _cancel, SLOT(setEnabled(bool)));
  }

  // some customers are creating scripts to show these widgets, probably shouldn't remove
  _paymentLit->hide();
  _payment->hide(); // Issue 10254:  if no objections over time, we should ultimately remove this.

  _miscChargeAccount->setType(GLCluster::cRevenue | GLCluster::cExpense);
}

selectOrderForBilling::~selectOrderForBilling()
{
  // no need to delete child widgets, Qt does it all for us
}

void selectOrderForBilling::languageChange()
{
  retranslateUi(this);
}

SetResponse selectOrderForBilling::set(const ParameterList &pParams)
{
  XWidget::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("sohead_id", &valid);
  if (valid)
  {
    _captive = TRUE;

    _so->setId(param.toInt());
  }

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      _mode = cNew;

      _so->setFocus();
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;

      _close->setText(tr("&Cancel"));

      _save->setFocus();
    }
  }

  return NoError;
}

void selectOrderForBilling::clear()
{
  _so->setId(-1);
  _cobmiscid = -1;
  _orderDate->clear();
  _shipDate->clear();
  _invoiceDate->clear();
  _custName->clear();
  _poNumber->clear();
  _shipToName->clear();
  _custCurrency->setCurrentIndex(0);
  _taxZone->setId(-1);
  _soitem->clear();
  _shipvia->clear();
  _miscChargeAccount->setId(-1);
  _miscChargeDescription->clear();
  _comments->clear();
  _subtotal->clear();
  _miscCharge->clear();
  _freight->clear();
  _salesTax->clear();
  _payment->clear();
  _total->clear();
}

void selectOrderForBilling::sSave()
{
  if (!_shipDate->isValid())
  {
    QMessageBox::information(this, tr("No Ship Date Entered"),
                             tr("<p>You must enter a Ship Date before "
				"selecting this order for billing."  ) );

    _shipDate->setFocus();
    return;
  }

  if ( (! _miscCharge->isZero()) && (!_miscChargeAccount->isValid()) )
  {
    QMessageBox::warning( this, tr("No Misc. Charge Account Number"),
                          tr("<p>You may not enter a Misc. Charge without "
			     "indicating the G/L Sales Account number for the "
			     "charge. Please set the Misc. Charge amount to 0 "
			     "or select a Misc. Charge Sales Account." ) );
    _miscChargeAccount->setFocus();
    return;
  }
  
  if (_cobmiscid != -1)
  {
    q.prepare( "UPDATE cobmisc "
               "SET cobmisc_freight=:cobmisc_freight,"
               "    cobmisc_payment=:cobmisc_payment,"
	       "    cobmisc_taxzone_id=:cobmisc_taxzone_id,"
	       "    cobmisc_notes=:cobmisc_notes,"
               "    cobmisc_shipdate=:cobmisc_shipdate, cobmisc_invcdate=:cobmisc_invcdate,"
               "    cobmisc_shipvia=:cobmisc_shipvia, cobmisc_closeorder=:cobmisc_closeorder,"
               "    cobmisc_misc=:cobmisc_misc, cobmisc_misc_accnt_id=:cobmisc_misc_accnt_id,"
               "    cobmisc_misc_descrip=:cobmisc_misc_descrip, "
	       "    cobmisc_curr_id=:cobmisc_curr_id "
               "WHERE (cobmisc_id=:cobmisc_id);" );
    q.bindValue(":cobmisc_id", _cobmiscid);
    q.bindValue(":cobmisc_freight", _freight->localValue());
    q.bindValue(":cobmisc_payment", _payment->localValue());

    if (_taxZone->isValid())
      q.bindValue(":cobmisc_taxzone_id", _taxZone->id());

    q.bindValue(":cobmisc_notes", _comments->toPlainText());
    q.bindValue(":cobmisc_shipdate", _shipDate->date());
    q.bindValue(":cobmisc_invcdate", _invoiceDate->date());
    q.bindValue(":cobmisc_shipvia", _shipvia->text());
    q.bindValue(":cobmisc_closeorder", QVariant(_closeOpenItems->isChecked()));
    q.bindValue(":cobmisc_misc", _miscCharge->localValue());
    q.bindValue(":cobmisc_misc_accnt_id", _miscChargeAccount->id());
    q.bindValue(":cobmisc_misc_descrip", _miscChargeDescription->text().trimmed());
    q.bindValue(":cobmisc_curr_id",	_custCurrency->id());
    q.exec();
    if (q.lastError().type() != QSqlError::NoError)
    {
	systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
	return;
    }
  }

  omfgThis->sBillingSelectionUpdated(_so->id(), TRUE);

  if (_captive)
    close();
  else
  {
    clear();
    _so->setFocus();
  }
}

void selectOrderForBilling::sSoList()
{
  ParameterList params;
  params.append("sohead_id", _so->id());

  if (_showClosed->isChecked())
    params.append("soType", (cSoOpen || cSoClosed));
  else
    params.append("soType", cSoOpen);

  salesOrderList newdlg(this, "", TRUE);
  newdlg.set(params);

  int id = newdlg.exec();
  if(id != QDialog::Rejected)
    _so->setId(id);
}

void selectOrderForBilling::sPopulate(int pSoheadid)
{
  if (_so->isValid())
  {
    q.prepare("SELECT createBillingHeader(:sohead_id) AS cobmisc_id;");
    q.bindValue(":sohead_id", pSoheadid);
    q.exec();
    if (q.first())
    {
      _cobmiscid = q.value("cobmisc_id").toInt();
      if (_cobmiscid < 0)
      {
	systemError(this, storedProcErrorLookup("createBillingHeader", _cobmiscid),
		    __FILE__, __LINE__);
	return;
      }
    }
    else if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }

    XSqlQuery cobmisc;
    cobmisc.prepare( "SELECT cobmisc_id, cobmisc_notes, cobmisc_shipvia,"
                     "       cohead_orderdate, cobmisc_shipdate,"
		     "       cobmisc_invcdate, cobmisc_taxzone_id,"
                     "       cobmisc_freight AS freight,"
                     "       cobmisc_misc, cobmisc_misc_accnt_id, cobmisc_misc_descrip,"
                     "       cobmisc_payment AS payment,"
                     "       cobmisc_closeorder,"
                     "       cobmisc_curr_id,"
                     "       cohead_number, cohead_shipto_id,"
                     "       cohead_custponumber,"
                     "       cohead_billtoname, cohead_shiptoname,"
                     "       CASE WHEN (shipchrg_custfreight IS NULL) THEN TRUE"
                     "            ELSE shipchrg_custfreight"
                     "       END AS custfreight "
                     "FROM cobmisc , cohead "
                     "  LEFT OUTER JOIN shipchrg ON (cohead_shipchrg_id=shipchrg_id) "
                     "WHERE ( (cobmisc_id=:cobmisc_id)"
                     " AND (cohead_id=:cohead_id) ) ");
    cobmisc.bindValue(":cobmisc_id", _cobmiscid);
    cobmisc.bindValue(":cohead_id", pSoheadid);
    cobmisc.exec();
    if (cobmisc.first())
    {
      _cobmiscid = cobmisc.value("cobmisc_id").toInt();
      // do taxzone first so we can overwrite the result of the signal cascade
      _taxzoneidCache = cobmisc.value("cobmisc_taxzone_id").toInt();
      _taxZone->setId(cobmisc.value("cobmisc_taxzone_id").toInt());

      _orderDate->setDate(cobmisc.value("cohead_orderdate").toDate(), true);
      _shipDate->setDate(cobmisc.value("cobmisc_shipdate").toDate());
      _invoiceDate->setDate(cobmisc.value("cobmisc_invcdate").toDate());

      _poNumber->setText(cobmisc.value("cohead_custponumber").toString());
      _custName->setText(cobmisc.value("cohead_billtoname").toString());
      _shipToName->setText(cobmisc.value("cohead_shiptoname").toString());
      _shipvia->setText(cobmisc.value("cobmisc_shipvia").toString());
     
      _miscCharge->setLocalValue(cobmisc.value("cobmisc_misc").toDouble());
      _miscChargeAccount->setId(cobmisc.value("cobmisc_misc_accnt_id").toInt());
      _miscChargeDescription->setText(cobmisc.value("cobmisc_misc_descrip"));
      _payment->set(cobmisc.value("payment").toDouble(),
		    cobmisc.value("cobmisc_curr_id").toInt(),
		    cobmisc.value("cohead_orderdate").toDate(), false);
      _custCurrency->setId(cobmisc.value("cobmisc_curr_id").toInt());
      _comments->setText(cobmisc.value("cobmisc_notes").toString());
      _closeOpenItems->setChecked(cobmisc.value("cobmisc_closeorder").toBool());
      _freightCache = cobmisc.value("freight").toDouble();
      if (cobmisc.value("custfreight").toBool())
      {
        _freight->setEnabled(TRUE);
        _freight->setLocalValue(cobmisc.value("freight").toDouble());
      }
      else
      {
        _freight->setEnabled(FALSE);
        _freight->clear();
      }
    }
    else if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }
  else
  {
    clear();
  }

  sFillList();
}

void selectOrderForBilling::sEditOrder()
{
  salesOrder::editSalesOrder(_so->id(), false);
}

void selectOrderForBilling::sEditSelection()
{
  ParameterList params;
  params.append("soitem_id", _soitem->id());
  params.append("taxzone_id", _taxZone->id());

  selectBillingQty newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
  {
    sFillList();
    _updated = TRUE;
  }
}

void selectOrderForBilling::sCancelSelection()
{
  q.prepare( "DELETE FROM cobill "
             "WHERE ((cobill_coitem_id=:coitem_id)"
             " AND (SELECT NOT cobmisc_posted"
             "      FROM cobmisc"
             "      WHERE (cobill_cobmisc_id=cobmisc_id) ) ) ");
  q.bindValue(":coitem_id", _soitem->id());
  q.exec();

  sFillList();
}

void selectOrderForBilling::sSelectBalance()
{
  q.prepare("SELECT selectBalanceForBilling(:sohead_id) AS result;");
  q.bindValue(":sohead_id", _so->id());
  q.exec();
  if (q.first())
  {
    int result = q.value("result").toInt();
    if (result < 0)
    {
      systemError(this, storedProcErrorLookup("selectBalanceForBilling", result),
		  __FILE__, __LINE__);
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

void selectOrderForBilling::sCalculateTax()   
{
  XSqlQuery taxq;
  taxq.prepare( "SELECT SUM(tax) AS tax "
                "FROM ("
                "SELECT ROUND(SUM(taxdetail_tax),2) AS tax "
                "FROM tax "
                " JOIN calculateTaxDetailSummary('B', :cobmisc_id, 'T') ON (taxdetail_tax_id=tax_id)"
	        "GROUP BY tax_id) AS data;" );
  taxq.bindValue(":cobmisc_id", _cobmiscid);
  taxq.exec();
  if (taxq.first())
    _salesTax->setLocalValue(taxq.value("tax").toDouble());
  else if (taxq.lastError().type() != QSqlError::NoError)
  {
    systemError(this, taxq.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
  // changing _tax fires sCalculateTotal()
}

void selectOrderForBilling::sUpdateTotal()
{
  _total->setLocalValue( _subtotal->localValue() +
                                _salesTax->localValue() +
                                _miscCharge->localValue() +
                                _freight->localValue() );
}

void selectOrderForBilling::sFillList()
{
  _soitem->clear();

  if (_so->isValid())
  {
    QString sql( "SELECT coitem_id, formatSoLineNumber(coitem_id) AS linenumber,"
                 "       item_number, iteminvpricerat(item_id) AS invpricerat,"
                 "       warehous_code, coitem_price,"
                 "       uom_name, coitem_qtyord, coitem_qtyshipped,"
                 "       coitem_qtyreturned,"
                 "       formatQty( ( SELECT COALESCE(SUM(coship_qty), 0)"
                 "                    FROM coship "
                 "                    WHERE ( (coship_coitem_id=coitem_id)"
                 "                     AND (NOT coship_invoiced) ) )"
                 "                 ) AS qtyatship,"
                 "       ( SELECT COALESCE(SUM(cobill_qty), 0)"
                 "         FROM cobill, cobmisc "
                 "         WHERE ( (cobill_cobmisc_id=cobmisc_id)"
                 "          AND (cobill_coitem_id=coitem_id)"
                 "          AND (NOT cobmisc_posted) ) ) AS qtytobill,"
                 "       round(( "
                 "         ( SELECT COALESCE(SUM(cobill_qty), 0)"
                 "           FROM cobill, cobmisc "
                 "           WHERE ( (cobill_cobmisc_id=cobmisc_id)"
                 "            AND (cobill_coitem_id=coitem_id)"
                 "            AND (NOT cobmisc_posted) ) ) * coitem_qty_invuomratio "
                 "           * ( coitem_price / coitem_price_invuomratio)), 2) AS extended, "
                 "       COALESCE((SELECT cobill_toclose"
                 "                       FROM cobill, cobmisc "
                 "                       WHERE ((cobill_cobmisc_id=cobmisc_id)"
                 "                        AND (cobill_coitem_id=coitem_id)"
                 "                        AND (NOT cobmisc_posted))"
                 "                       ORDER BY cobill_toclose DESC"
                 "                       LIMIT 1),FALSE) AS toclose, "
                 "       'qty' AS coitem_qtyord_xtnumericrole, "
                 "       'qty' AS coitem_qtyshipped_xtnumericrole, "
                 "       'qty' AS coitem_qtyreturned_xtnumericrole, "
                 "       'qty' AS qtyatship_xtnumericrole, "
                 "       'qty' AS qtytobill_xtnumericrole, "
                 "       'extprice' AS extended_xtnumericrole "
                 "FROM coitem, itemsite, item, site(), uom "
                 "WHERE ( (coitem_itemsite_id=itemsite_id)"
                 " AND (coitem_status <> 'X')"
                 " AND (coitem_qty_uom_id=uom_id)"
                 " AND (itemsite_item_id=item_id)"
                 " AND (itemsite_warehous_id=warehous_id)"
		 " <? if exists(\"showOpenOnly\") ?>"
		 " AND (coitem_status <> 'C')"
		 " <? endif ?>"
		 " AND (coitem_cohead_id=<? value(\"sohead_id\") ?>) ) "
		 "ORDER BY coitem_linenumber, coitem_subnumber;" );

    ParameterList params;
    if (!_showClosed->isChecked())
      params.append("showOpenOnly");
    params.append("sohead_id", _so->id());
    MetaSQLQuery mql(sql);
    q = mql.toQuery(params);
    _soitem->populate(q);
    if (q.first())
    {
      double subtotal = 0.0;
      do
        subtotal += q.value("extended").toDouble();
      while (q.next());
      _subtotal->setLocalValue(subtotal);
    }
    else
    {
      if (q.lastError().type() != QSqlError::NoError)
	systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      _subtotal->clear();
    }

    sCalculateTax();
  }
}

void selectOrderForBilling::sHandleShipchrg(int pShipchrgid)
{
  XSqlQuery query;
  query.prepare( "SELECT shipchrg_custfreight "
                 "FROM shipchrg "
                 "WHERE (shipchrg_id=:shipchrg_id);" );
  query.bindValue(":shipchrg_id", pShipchrgid);
  query.exec();
  if (query.first())
  {
    if (query.value("shipchrg_custfreight").toBool())
      _freight->setEnabled(TRUE);
    else
    {
      _freight->setEnabled(FALSE);
      _freight->clear();
    }
  }
  else if (query.lastError().type() != QSqlError::NoError)
  {
    systemError(this, query.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void selectOrderForBilling::sTaxDetail()
{
  XSqlQuery taxq;
  taxq.prepare("UPDATE cobmisc SET cobmisc_taxzone_id=:taxzone,"
	           "  cobmisc_freight=:freight,"
	           "  cobmisc_invcdate=:invcdate "
	           "WHERE (cobmisc_id=:cobmisc_id);");
  if(_taxZone->isValid())
    taxq.bindValue(":taxzone",	_taxZone->id());
  taxq.bindValue(":freight",	_freight->localValue());
  taxq.bindValue(":cobmisc_id",	_cobmiscid);
  taxq.bindValue(":invcdate",   _invoiceDate->date());
  taxq.exec();
  if (taxq.lastError().type() != QSqlError::NoError)
  {
    systemError(this, taxq.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  ParameterList params;
  params.append("order_id",	_cobmiscid);
  params.append("order_type",	"B");
  params.append("mode",		"edit");

  taxBreakdown newdlg(this, "", TRUE);
  if (newdlg.set(params) == NoError)
  {
    newdlg.exec();
    sPopulate(_so->id());
  }
}

void selectOrderForBilling::closeEvent(QCloseEvent * pEvent)
{
  q.prepare("SELECT releaseUnusedBillingHeader(:cobmisc_id) AS result;");
  q.bindValue(":cobmisc_id", _cobmiscid);
  q.exec();
  if (q.first())
  {
    int result = q.value("result").toInt();
    if (result < -2) // don't bother the user with -1:posted or -2:has-lines
      systemError(this, storedProcErrorLookup("releaseUnusedBillingHeader", result),
		  __FILE__, __LINE__);
  }
  else if (q.lastError().type() != QSqlError::NoError)
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);

  XWidget::closeEvent(pEvent);
}

void selectOrderForBilling::sTaxZoneChanged()
{
  if (_cobmiscid != -1 && _taxzoneidCache != _taxZone->id())
  {
    XSqlQuery taxq;
    taxq.prepare("UPDATE cobmisc SET "
      "  cobmisc_taxzone_id=:taxzone_id, "
      "  cobmisc_freight=:freight "
      "WHERE (cobmisc_id=:cobmisc_id) ");
    if (_taxZone->id() != -1)
      taxq.bindValue(":taxzone_id", _taxZone->id());
    taxq.bindValue(":cobmisc_id", _cobmiscid);
    taxq.bindValue(":freight", _freight->localValue());
    taxq.exec();
    if (taxq.lastError().type() != QSqlError::NoError)
    {
      systemError(this, taxq.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
    _taxzoneidCache = _taxZone->id();
    sCalculateTax();
  }
}

void selectOrderForBilling::sFreightChanged()
{
  if (_cobmiscid != -1 && _freightCache != _freight->localValue())
  {
    XSqlQuery taxq;
    taxq.prepare("UPDATE cobmisc SET "
      "  cobmisc_freight=:freight "
      "WHERE (cobmisc_id=:cobmisc_id) ");
    taxq.bindValue(":cobmisc_id", _cobmiscid);
    taxq.bindValue(":freight", _freight->localValue());
    taxq.exec();
    if (taxq.lastError().type() != QSqlError::NoError)
    {
      systemError(this, taxq.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
    _freightCache = _freight->localValue();
    sCalculateTax();
  }   
}
