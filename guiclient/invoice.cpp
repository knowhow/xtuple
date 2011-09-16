/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "invoice.h"

#include <QCloseEvent>
#include <QKeyEvent>
#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include <QDebug>

#include "invoiceItem.h"
#include "storedProcErrorLookup.h"
#include "taxBreakdown.h"
#include "allocateARCreditMemo.h"

#define cViewQuote (0x20 | cView)

invoice::invoice(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, fl)
{
  if(name)
    setObjectName(name);

  setupUi(this);

  connect(_close, SIGNAL(clicked()), this, SLOT(sClose()));
  connect(_save, SIGNAL(clicked()), this, SLOT(sSave()));
  connect(_cust, SIGNAL(newId(int)),   _shipTo,     SLOT(setCustid(int)));
  connect(_cust, SIGNAL(newId(int)), this, SLOT(sPopulateCustomerInfo(int)));
  connect(_new, SIGNAL(clicked()), this, SLOT(sNew()));
  connect(_edit, SIGNAL(clicked()), this, SLOT(sEdit()));
  connect(_view, SIGNAL(clicked()), this, SLOT(sView()));
  connect(_delete, SIGNAL(clicked()), this, SLOT(sDelete()));
  connect(_copyToShipto, SIGNAL(clicked()), this, SLOT(sCopyToShipto()));
  connect(_taxLit, SIGNAL(leftClickedURL(const QString&)), this, SLOT(sTaxDetail()));
  connect(_shipTo, SIGNAL(newId(int)), this, SLOT(populateShipto(int)));
  connect(_shipToName, SIGNAL(textChanged(const QString&)), this, SLOT(sShipToModified()));
  connect(_subtotal, SIGNAL(valueChanged()), this, SLOT(sCalculateTotal()));
  connect(_tax, SIGNAL(valueChanged()), this, SLOT(sCalculateTotal()));
  connect(_miscAmount, SIGNAL(valueChanged()), this, SLOT(sCalculateTotal()));
  connect(_freight, SIGNAL(valueChanged()), this, SLOT(sFreightChanged()));
  connect(_allocatedCM, SIGNAL(valueChanged()), this, SLOT(sCalculateTotal()));
  connect(_outstandingCM, SIGNAL(valueChanged()), this, SLOT(sCalculateTotal()));
  connect(_authCC, SIGNAL(valueChanged()), this, SLOT(sCalculateTotal()));
  connect(_shipToAddr, SIGNAL(changed()), this, SLOT(sShipToModified()));
  connect(_shipToPhone, SIGNAL(textChanged(const QString&)), this, SLOT(sShipToModified()));
  connect(_authCC, SIGNAL(idChanged(int)), this, SLOT(populateCCInfo()));
  connect(_allocatedCM, SIGNAL(idChanged(int)), this, SLOT(populateCMInfo()));
  connect(_outstandingCM, SIGNAL(idChanged(int)), this, SLOT(populateCMInfo()));
  connect(_authCC, SIGNAL(effectiveChanged(const QDate&)), this, SLOT(populateCCInfo()));
  if (_privileges->check("ApplyARMemos"))
    connect(_allocatedCMLit,    SIGNAL(leftClickedURL(const QString &)),        this,         SLOT(sCreditAllocate()));
  connect(_allocatedCM, SIGNAL(effectiveChanged(const QDate&)), this, SLOT(populateCMInfo()));
  connect(_outstandingCM, SIGNAL(effectiveChanged(const QDate&)), this, SLOT(populateCMInfo()));
  connect(_invcitem, SIGNAL(valid(bool)),       _edit, SLOT(setEnabled(bool)));
  connect(_invcitem, SIGNAL(valid(bool)),     _delete, SLOT(setEnabled(bool)));
  connect(_invcitem, SIGNAL(itemSelected(int)), _edit, SLOT(animateClick()));
  connect(_taxzone,  SIGNAL(newID(int)),	 this, SLOT(sTaxZoneChanged()));
  connect(_shipChrgs, SIGNAL(newID(int)), this, SLOT(sHandleShipchrg(int)));
  connect(_cust, SIGNAL(newCrmacctId(int)), _billToAddr, SLOT(setSearchAcct(int)));
  connect(_cust, SIGNAL(newCrmacctId(int)), _shipToAddr, SLOT(setSearchAcct(int)));
  connect(_invoiceNumber, SIGNAL(editingFinished()), this, SLOT(sCheckInvoiceNumber()));

  setFreeFormShipto(false);

  _custtaxzoneid  = -1;
  _invcheadid	  = -1;
  _taxzoneidCache = -1;
  _loading = false;
  _freightCache = 0;
  _posted = false;

  _cust->setType(CLineEdit::ActiveCustomers);

  _shipTo->setNameVisible(false);
  _shipTo->setDescriptionVisible(false);

  _invcitem->addColumn(tr("#"),           _seqColumn,      Qt::AlignCenter, true,  "invcitem_linenumber" );
  _invcitem->addColumn(tr("Order #"),     _itemColumn,     Qt::AlignLeft,   true,  "soitemnumber"   );
  _invcitem->addColumn(tr("Item"),        _itemColumn,     Qt::AlignLeft,   true,  "itemnumber"   );
  _invcitem->addColumn(tr("Description"), -1,              Qt::AlignLeft,   true,  "itemdescription"   );
  _invcitem->addColumn(tr("Qty. UOM"),    _uomColumn,      Qt::AlignLeft,   true,  "qtyuom"   );
  _invcitem->addColumn(tr("Ordered"),     _qtyColumn,      Qt::AlignRight,  true,  "invcitem_ordered"  );
  _invcitem->addColumn(tr("Billed"),      _qtyColumn,      Qt::AlignRight,  true,  "invcitem_billed"  );
  _invcitem->addColumn(tr("Price UOM"),   _uomColumn,      Qt::AlignLeft,   true,  "priceuom"   );
  _invcitem->addColumn(tr("Price"),       _moneyColumn,    Qt::AlignRight,  true,  "invcitem_price"  );
  _invcitem->addColumn(tr("Extended"),    _bigMoneyColumn, Qt::AlignRight,  true,  "extprice"  );

  _custCurrency->setLabel(_custCurrencyLit);

  _project->setType(ProjectLineEdit::SalesOrder);
  if(!_metrics->boolean("UseProjects"))
    _project->hide();

  _miscAmount->setAllowNegative(true);

  _commission->setValidator(omfgThis->percentVal());
  _weight->setValidator(omfgThis->weightVal());
  
  // some customers are creating scripts to show these widgets, probably shouldn't remove
  _paymentLit->hide();
  _payment->hide(); // Issue 9895:  if no objections over time, we should ultimately remove this. 

  _recurring->setParent(-1, "I");

  _miscChargeAccount->setType(GLCluster::cRevenue | GLCluster::cExpense);
}

invoice::~invoice()
{
  // no need to delete child widgets, Qt does it all for us
}

void invoice::languageChange()
{
  retranslateUi(this);
}

enum SetResponse invoice::set(const ParameterList &pParams)
{
  XWidget::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      setObjectName("invoice new");
      _mode = cNew;

      q.exec("SELECT NEXTVAL('invchead_invchead_id_seq') AS invchead_id;");
      if (q.first())
      {
        _invcheadid = q.value("invchead_id").toInt();
        _recurring->setParent(_invcheadid, "I");
      }
      else if (q.lastError().type() != QSqlError::NoError)
      {
	    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
	    return UndefinedError;
      }

      if ((_metrics->value("InvcNumberGeneration") == "A") ||
          (_metrics->value("InvcNumberGeneration") == "O"))
      {
        q.exec("SELECT fetchInvcNumber() AS number;");
        if (q.first())
        {
          _invoiceNumber->setText(q.value("number").toString());
          if (_metrics->value("InvcNumberGeneration") == "A")
            _invoiceNumber->setEnabled(false);
        }
        else if (q.lastError().type() != QSqlError::NoError)
        {
          systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
          return UndefinedError;
        }

        _cust->setFocus();
      }
      else
        sCheckInvoiceNumber();

      _orderDate->setDate(omfgThis->dbDate());
      _shipDate->setDate(omfgThis->dbDate());
      _invoiceDate->setDate(omfgThis->dbDate(), true);

      q.prepare("INSERT INTO invchead ("
				"    invchead_id, invchead_invcnumber, invchead_orderdate,"
                "    invchead_invcdate, invchead_cust_id, invchead_posted,"
				"    invchead_printed, invchead_commission, invchead_freight,"
				"    invchead_misc_amount, invchead_shipchrg_id "
				") VALUES ("
				"    :invchead_id, :invchead_invcnumber, :invchead_orderdate, "
				"    :invchead_invcdate, -1, false,"
				"    false, 0, 0,"
				"    0, -1"
				");");
      q.bindValue(":invchead_id",	 _invcheadid);
      q.bindValue(":invchead_invcnumber",_invoiceNumber->text().isEmpty() ?
                                               "TEMP" + QString(0 - _invcheadid)
                                             : _invoiceNumber->text());
      q.bindValue(":invchead_orderdate", _orderDate->date());
      q.bindValue(":invchead_invcdate",	 _invoiceDate->date());
      q.exec();
      if (q.lastError().type() != QSqlError::NoError)
      {
	    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
	    return UndefinedError;
      }

      connect(_cust,	    SIGNAL(valid(bool)), _new, SLOT(setEnabled(bool)));
      connect(_cust,        SIGNAL(valid(bool)), this, SLOT(populateCMInfo()));
      connect(_orderNumber, SIGNAL(lostFocus()), this, SLOT(populateCCInfo()));
    }
    else if (param.toString() == "edit")
    {

      param = pParams.value("invchead_id", &valid);
      if(valid)
        _invcheadid = param.toInt();

      setObjectName(QString("invoice edit %1").arg(_invcheadid));
      _mode = cEdit;

      _new->setEnabled(TRUE);
      _cust->setReadOnly(TRUE);

      _save->setFocus();
    }
    else if (param.toString() == "view")
    {
      setObjectName(QString("invoice view %1").arg(_invcheadid));
      _mode = cView;

      _invoiceNumber->setEnabled(FALSE);
      _orderNumber->setEnabled(FALSE);
      _invoiceDate->setEnabled(FALSE);
      _shipDate->setEnabled(FALSE);
      _orderDate->setEnabled(FALSE);
      _poNumber->setEnabled(FALSE);
      _cust->setReadOnly(TRUE);
      _salesrep->setEnabled(FALSE);
      _commission->setEnabled(FALSE);
      _taxzone->setEnabled(FALSE);
      _terms->setEnabled(FALSE);
      _terms->setType(XComboBox::Terms);
      _fob->setEnabled(FALSE);
      _shipVia->setEnabled(FALSE);
      _billToName->setEnabled(FALSE);
      _billToAddr->setEnabled(FALSE);
      _billToPhone->setEnabled(FALSE);
      _shipTo->setEnabled(FALSE);
      _shipToName->setEnabled(FALSE);
      _shipToAddr->setEnabled(FALSE);
      _shipToPhone->setEnabled(FALSE);
      _miscAmount->setEnabled(FALSE);
      _miscChargeDescription->setEnabled(FALSE);
      _miscChargeAccount->setReadOnly(TRUE);
      _freight->setEnabled(FALSE);
      _payment->setEnabled(FALSE);
      _notes->setReadOnly(TRUE);
      _edit->hide();
      _save->hide();
      _delete->hide();
      _project->setEnabled(false);
      _shipChrgs->setEnabled(FALSE);

      disconnect(_invcitem, SIGNAL(valid(bool)), _edit, SLOT(setEnabled(bool)));
      disconnect(_invcitem, SIGNAL(valid(bool)), _delete, SLOT(setEnabled(bool)));
      disconnect(_invcitem, SIGNAL(itemSelected(int)), _edit, SLOT(animateClick()));
      connect(_invcitem, SIGNAL(itemSelected(int)), _view, SLOT(animateClick()));

      _close->setFocus();
    }
  }

  param = pParams.value("cust_id", &valid);
  if(cNew == _mode && valid)
    _cust->setId(param.toInt());

  param = pParams.value("invchead_id", &valid);
  if (valid)
  {
    _invcheadid = param.toInt();
    populate();
    populateCMInfo();
    populateCCInfo();
  }

  return NoError;
}

void invoice::sClose()
{
  if (_mode == cNew)
  {
    q.prepare( "DELETE FROM invcitem "
               "WHERE (invcitem_invchead_id=:invchead_id);"
               "DELETE FROM invchead "
               "WHERE (invchead_id=:invchead_id);" );
    q.bindValue(":invchead_id", _invcheadid);
    q.exec();
    if (q.lastError().type() != QSqlError::NoError)
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
  }

  close();
}

void invoice::sPopulateCustomerInfo(int pCustid)
{
  if (pCustid != -1)
  {
    XSqlQuery cust;
    cust.prepare( "SELECT cust_name, COALESCE(cntct_addr_id,-1) AS addr_id, "
                  "       cust_salesrep_id, cust_commprcnt * 100 AS commission,"
                  "       cust_creditstatus, cust_terms_id, "
                  "       COALESCE(cust_taxzone_id, -1) AS cust_taxzone_id,"
                  "       COALESCE(cust_shipchrg_id, -1) AS cust_shipchrg_id,"
                  "       cust_ffshipto, cust_ffbillto, "
                  "       COALESCE(shipto_id, -1) AS shiptoid, "
                  "       cust_curr_id "
                  "FROM custinfo "
                  "  LEFT OUTER JOIN cntct ON (cust_cntct_id=cntct_id) "
                  "  LEFT OUTER JOIN shipto ON ( (shipto_cust_id=cust_id) "
                  "                         AND (shipto_default) ) "
                  "WHERE (cust_id=:cust_id);" );
    cust.bindValue(":cust_id", pCustid);
    cust.exec();
    if (cust.first())
    {
        _billToName->setText(cust.value("cust_name").toString());
        _billToAddr->setId(cust.value("addr_id").toInt());
	_salesrep->setId(cust.value("cust_salesrep_id").toInt());
	_commission->setDouble(cust.value("commission").toDouble());
	_terms->setId(cust.value("cust_terms_id").toInt());
	_custtaxzoneid = cust.value("cust_taxzone_id").toInt();
	_taxzone->setId(cust.value("cust_taxzone_id").toInt());
	_custCurrency->setId(cust.value("cust_curr_id").toInt());
        _shipChrgs->setId(cust.value("cust_shipchrg_id").toInt());

	bool ffBillTo = cust.value("cust_ffbillto").toBool();
        if (_mode != cView)
        {
          _billToName->setEnabled(ffBillTo);
          _billToAddr->setEnabled(ffBillTo);
          _billToPhone->setEnabled(ffBillTo);
        }

	setFreeFormShipto(cust.value("cust_ffshipto").toBool());
	if (cust.value("shiptoid").toInt() != -1)
	  populateShipto(cust.value("shiptoid").toInt());
	else
	{
          _shipTo->setId(-1);
	  _shipToName->clear();
	  _shipToAddr->clear();
	  _shipToPhone->clear();
	}
      }
      if (cust.lastError().type() != QSqlError::NoError)
      {
	systemError(this, cust.lastError().databaseText(), __FILE__, __LINE__);
	return;
      }
  }
  else
  {
    _salesrep->setCurrentIndex(-1);
    _commission->clear();
    _terms->setCurrentIndex(-1);
    _custtaxzoneid  = -1;
    _taxzoneidCache = -1;
    _taxzone->setCurrentIndex(-1);
  }
}

void invoice::populateShipto(int pShiptoid)
{
  if (pShiptoid != -1)
  {
    XSqlQuery shipto;
    shipto.prepare( "SELECT shipto_id, shipto_num, shipto_name, shipto_addr_id, "
                    "       cntct_phone, shipto_shipvia, shipto_salesrep_id, "
                    "       COALESCE(shipto_taxzone_id, -1) AS shipto_taxzone_id,"
                    "       COALESCE(shipto_shipchrg_id, -1) AS shipto_shipchrg_id,"
		    "       shipto_commission * 100 AS commission "
                    "FROM shiptoinfo LEFT OUTER JOIN "
		    "     cntct ON (shipto_cntct_id=cntct_id)"
                    "WHERE (shipto_id=:shipto_id);" );
    shipto.bindValue(":shipto_id", pShiptoid);
    shipto.exec();
    if (shipto.first())
    {
      _shipToAddr->blockSignals(true);
      _shipTo->blockSignals(true);
      _shipToName->blockSignals(true);
      _shipToPhone->blockSignals(true);

      _shipToName->setText(shipto.value("shipto_name"));
      _shipToAddr->setId(shipto.value("shipto_addr_id").toInt());
      _shipToPhone->setText(shipto.value("cntct_phone"));
      _shipTo->setId(shipto.value("shipto_id").toInt());

      _shipToAddr->blockSignals(false);
      _shipTo->blockSignals(false);
      _shipToName->blockSignals(false);
      _shipToPhone->blockSignals(false);

      _salesrep->setId(shipto.value("shipto_salesrep_id").toInt());
      _commission->setDouble(shipto.value("commission").toDouble());
      _shipVia->setText(shipto.value("shipto_shipvia"));
      _taxzone->setId(shipto.value("shipto_taxzone_id").toInt());
      _shipChrgs->setId(shipto.value("shipto_shipchrg_id").toInt());
    }
    else if (shipto.lastError().type() != QSqlError::NoError)
    {
      systemError(this, shipto.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }
  else
  {
    _shipTo->blockSignals(true);
    _shipTo->setId(-1);
    _shipTo->blockSignals(false);
    _shipToName->clear();
    _shipToAddr->clear();
    _shipToPhone->clear();
  }
}

void invoice::sCopyToShipto()
{
  _shipTo->blockSignals(true);
  _shipToAddr->blockSignals(true);
  _shipTo->setId(-1);
  _shipToName->setText(_billToName->text());
  _shipToAddr->setId(_billToAddr->id());
  _shipToPhone->setText(_billToPhone->text());
  _taxzone->setId(_custtaxzoneid);
  _shipTo->blockSignals(false);
  _shipToAddr->blockSignals(false);
}

void invoice::sSave()
{
  struct {
    bool	condition;
    QString	msg;
    QWidget*	widget;
  } error[] = {
    { _cust->id() <= 0,
      tr("<p>You must enter a Customer for this Invoice before saving it."),
      _cust
    },
    // TODO: add more error checks here?
    { true, "", NULL }
  };
  
  if (_total->localValue() < 0 )
  {
    QMessageBox::information(this, tr("Total Less than Zero"),
                             tr("<p>The Total must be a positive value.") );
    _cust->setFocus();
    return;
  }
  
  //Invoices must have atleast one line item.
  if (_invcitem->topLevelItemCount() <= 0 )
  {
    QMessageBox::information(this, tr("No Line Items"),
                             tr("<p>There must be at least one line item for an invoice.") );
    _new->setFocus();
    return;
  }
  
  //  We can't post a Misc. Charge without a Sales Account
  if ( (! _miscAmount->isZero()) && (!_miscChargeAccount->isValid()) )
  {
    QMessageBox::warning( this, tr("No Misc. Charge Account Number"),
                          tr("<p>You may not enter a Misc. Charge without "
                             "indicating the G/L Sales Account number for the "
                             "charge.  Please set the Misc. Charge amount to 0 "
                             "or select a Misc. Charge Sales Account." ) );
    _tabWidget->setCurrentIndex(_tabWidget->indexOf(lineItemsTab));
    _miscChargeAccount->setFocus();
    return;
  }


  int errIndex;
  for (errIndex = 0; ! error[errIndex].condition; errIndex++)
    ;
  if (! error[errIndex].msg.isEmpty())
  {
    QMessageBox::critical(this, tr("Cannot Save Invoice"), error[errIndex].msg);
    error[errIndex].widget->setFocus();
    return;
  }

  // save address info in case someone wants to use 'em again later
  // but don't make any global changes to the data and ignore errors
  _billToAddr->save(AddressCluster::CHANGEONE);
  _shipToAddr->save(AddressCluster::CHANGEONE);

  // finally save the invchead
  if (!save())
    return;

  omfgThis->sInvoicesUpdated(_invcheadid, TRUE);

  _invcheadid = -1;
  close();
}

bool invoice::save()
{
  RecurrenceWidget::RecurrenceChangePolicy cp = _recurring->getChangePolicy();
  if (cp == RecurrenceWidget::NoPolicy)
    return false;

  XSqlQuery rollbackq;
  rollbackq.prepare("ROLLBACK;");

  XSqlQuery begin("BEGIN;");

  q.prepare( "UPDATE invchead "
	     "SET invchead_cust_id=:invchead_cust_id,"
	     "    invchead_invcdate=:invchead_invcdate,"
	     "    invchead_shipdate=:invchead_shipdate,"
	     "    invchead_invcnumber=:invchead_invcnumber,"
	     "    invchead_ordernumber=:invchead_ordernumber,"
	     "    invchead_orderdate=:invchead_orderdate,"
	     "    invchead_ponumber=:invchead_ponumber,"
	     "    invchead_billto_name=:invchead_billto_name, invchead_billto_address1=:invchead_billto_address1,"
	     "    invchead_billto_address2=:invchead_billto_address2, invchead_billto_address3=:invchead_billto_address3,"
	     "    invchead_billto_city=:invchead_billto_city, invchead_billto_state=:invchead_billto_state,"
	     "    invchead_billto_zipcode=:invchead_billto_zipcode, invchead_billto_phone=:invchead_billto_phone,"
	     "    invchead_billto_country=:invchead_billto_country,"
	     "    invchead_shipto_id=:invchead_shipto_id,"
	     "    invchead_shipto_name=:invchead_shipto_name, invchead_shipto_address1=:invchead_shipto_address1,"
	     "    invchead_shipto_address2=:invchead_shipto_address2, invchead_shipto_address3=:invchead_shipto_address3,"
	     "    invchead_shipto_city=:invchead_shipto_city, invchead_shipto_state=:invchead_shipto_state,"
	     "    invchead_shipto_zipcode=:invchead_shipto_zipcode, invchead_shipto_phone=:invchead_shipto_phone,"
	     "    invchead_shipto_country=:invchead_shipto_country,"
	     "    invchead_salesrep_id=:invchead_salesrep_id,"
	     "    invchead_terms_id=:invchead_terms_id, invchead_commission=:invchead_commission,"
	     "    invchead_misc_amount=:invchead_misc_amount, invchead_misc_descrip=:invchead_misc_descrip,"
	     "    invchead_misc_accnt_id=:invchead_misc_accnt_id,"
	     "    invchead_freight=:invchead_freight,"
	     "    invchead_payment=:invchead_payment,"
	     "    invchead_curr_id=:invchead_curr_id,"
	     "    invchead_shipvia=:invchead_shipvia, invchead_fob=:invchead_fob, invchead_notes=:invchead_notes,"
             "    invchead_recurring_invchead_id=:invchead_recurring_invchead_id,"
	     "    invchead_prj_id=:invchead_prj_id, "
         "    invchead_shipchrg_id=:invchead_shipchrg_id, "
		 "    invchead_taxzone_id=:invchead_taxzone_id "
	     "WHERE (invchead_id=:invchead_id);" );

  q.bindValue(":invchead_id",			_invcheadid);
  q.bindValue(":invchead_invcnumber",		_invoiceNumber->text());
  q.bindValue(":invchead_cust_id",		_cust->id());
  q.bindValue(":invchead_invcdate",		_invoiceDate->date());
  q.bindValue(":invchead_shipdate",		_shipDate->date());
  q.bindValue(":invchead_orderdate",		_orderDate->date());
  q.bindValue(":invchead_ponumber",		_poNumber->text());
  q.bindValue(":invchead_billto_name",		_billToName->text());
  q.bindValue(":invchead_billto_address1",	_billToAddr->line1());
  q.bindValue(":invchead_billto_address2",	_billToAddr->line2());
  q.bindValue(":invchead_billto_address3",	_billToAddr->line3());
  q.bindValue(":invchead_billto_city",		_billToAddr->city());
  q.bindValue(":invchead_billto_state",		_billToAddr->state());
  q.bindValue(":invchead_billto_zipcode",	_billToAddr->postalCode());
  q.bindValue(":invchead_billto_country",	_billToAddr->country());
  q.bindValue(":invchead_billto_phone",		_billToPhone->text());
  q.bindValue(":invchead_shipto_id",		_shipTo->id());
  q.bindValue(":invchead_shipto_name",		_shipToName->text());
  q.bindValue(":invchead_shipto_address1",	_shipToAddr->line1());
  q.bindValue(":invchead_shipto_address2",	_shipToAddr->line2());
  q.bindValue(":invchead_shipto_address3",	_shipToAddr->line3());
  q.bindValue(":invchead_shipto_city",		_shipToAddr->city());
  q.bindValue(":invchead_shipto_state",		_shipToAddr->state());
  q.bindValue(":invchead_shipto_zipcode",	_shipToAddr->postalCode());
  q.bindValue(":invchead_shipto_country",	_shipToAddr->country());
  q.bindValue(":invchead_shipto_phone",		_shipToPhone->text());
  if(_taxzone->isValid())
    q.bindValue(":invchead_taxzone_id",		_taxzone->id());
  q.bindValue(":invchead_salesrep_id",		_salesrep->id());
  q.bindValue(":invchead_terms_id",		_terms->id());
  q.bindValue(":invchead_commission",	(_commission->toDouble() / 100.0));
  q.bindValue(":invchead_freight",	_freight->localValue());
  q.bindValue(":invchead_payment",	_payment->localValue());
  q.bindValue(":invchead_curr_id",	_custCurrency->id());
  q.bindValue(":invchead_misc_amount",	_miscAmount->localValue());
  q.bindValue(":invchead_misc_descrip",	_miscChargeDescription->text());
  q.bindValue(":invchead_misc_accnt_id",_miscChargeAccount->id());
  q.bindValue(":invchead_shipvia",	_shipVia->currentText());
  q.bindValue(":invchead_fob",		_fob->text());
  q.bindValue(":invchead_notes",	_notes->toPlainText());
  q.bindValue(":invchead_prj_id",	_project->id());
  q.bindValue(":invchead_shipchrg_id",	_shipChrgs->id());
  if(_recurring->isRecurring())
  {
    if(_recurring->parentId() != 0)
      q.bindValue(":invchead_recurring_invchead_id", _recurring->parentId());
    else
      q.bindValue(":invchead_recurring_invchead_id", _invcheadid);
  }

  if (_orderNumber->text().length())
    q.bindValue(":invchead_ordernumber", _orderNumber->text().toInt());
  
  q.exec();
  if (q.lastError().type() != QSqlError::NoError)
  {
    rollbackq.exec();
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return false;
  }

  QString errmsg;
  if (! _recurring->save(true, cp, &errmsg))
  {
    rollbackq.exec();
    systemError(this, errmsg, __FILE__, __LINE__);
    return false;
  }

  XSqlQuery commitq("COMMIT;");

  return true;
}

void invoice::sNew()
{
  if (!save())
    return;
	
  ParameterList params;
  params.append("mode", "new");
  params.append("invchead_id", _invcheadid);

  invoiceItem newdlg(this);
  newdlg.set(params);
  if (newdlg.exec() != XDialog::Rejected)
    sFillItemList();
}

void invoice::sEdit()
{
  if (!save())
    return;
	
  ParameterList params;
  params.append("mode", "edit");
  params.append("invchead_id", _invcheadid);
  params.append("invcitem_id", _invcitem->id());

  invoiceItem newdlg(this);
  newdlg.set(params);
  if (newdlg.exec() != XDialog::Rejected)
    sFillItemList();
}

void invoice::sView()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("invchead_id", _invcheadid);
  params.append("invcitem_id", _invcitem->id());

  invoiceItem newdlg(this);
  newdlg.set(params);
  newdlg.exec();
}

void invoice::sDelete()
{
  q.prepare( "DELETE FROM invcitem "
             "WHERE (invcitem_id=:invcitem_id);" );
  q.bindValue(":invcitem_id", _invcitem->id());
  q.exec();
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  sFillItemList();
}

void invoice::populate()
{
  q.prepare( "SELECT invchead.*,"
             "    COALESCE(invchead_taxzone_id, -1) AS taxzone_id,"
             "    COALESCE(cust_taxzone_id, -1) AS cust_taxzone_id,"
	         "    cust_ffbillto, cust_ffshipto, "
	         "    invchead_shipchrg_id, invchead_ordernumber, "
             "    COALESCE(invchead_shipchrg_id,-1) AS shipchrg_id "
             "FROM invchead LEFT OUTER JOIN cohead "
             "ON (cohead.cohead_number = invchead.invchead_ordernumber) "
             "JOIN custinfo ON (invchead_cust_id = cust_id) "
			 "WHERE (invchead_id=:invchead_id);" );
  q.bindValue(":invchead_id", _invcheadid);
  q.exec();
  if (q.first())
  {
    _loading = true;
    // We are setting the _taxzoneidCache here to the value that
    // sPopulateCustomerInfo is going to set the _taxzone to.
    // The reason for doing this is to prevent sPopulateCustomerInfo
    // from triggering the sTaxZoneChanged() function which does some
    // database manipulation that may be unnecessary or even harmful
    // and we intend to set the values a little further down and they
    // may differ.
    _taxzoneidCache = q.value("cust_taxzone_id").toInt(); 

    _cust->setId(q.value("invchead_cust_id").toInt());
    _custCurrency->setId(q.value("invchead_curr_id").toInt());

    _invoiceNumber->setText(q.value("invchead_invcnumber").toString());
    _invoiceNumber->setEnabled(false);
    _orderNumber->setText(q.value("invchead_ordernumber").toString());
    if (! _orderNumber->text().isEmpty() && _orderNumber->text().toInt() != 0)
	_custCurrency->setEnabled(FALSE);

    _invoiceDate->setDate(q.value("invchead_invcdate").toDate(), true);
    _orderDate->setDate(q.value("invchead_orderdate").toDate());
    _shipDate->setDate(q.value("invchead_shipdate").toDate());
    _poNumber->setText(q.value("invchead_ponumber").toString());
    _shipVia->setText(q.value("invchead_shipvia").toString());
    _fob->setText(q.value("invchead_fob").toString());
    _shipChrgs->setId(q.value("shipchrg_id").toInt());
    _freightCache=q.value("invchead_freight").toDouble();
    _freight->setLocalValue(q.value("invchead_freight").toDouble());

    if(q.value("invchead_recurring_invchead_id").toInt() != 0)
      _recurring->setParent(q.value("invchead_recurring_invchead_id").toInt(),
                          "I");
    else
      _recurring->setParent(_invcheadid, "I");

    _salesrep->setId(q.value("invchead_salesrep_id").toInt());
    _commission->setDouble(q.value("invchead_commission").toDouble() * 100);
    _taxzoneidCache = q.value("taxzone_id").toInt(); 
    _taxzone->setId(q.value("taxzone_id").toInt());
    _terms->setId(q.value("invchead_terms_id").toInt());
    _project->setId(q.value("invchead_prj_id").toInt());

    bool ffBillTo = q.value("cust_ffbillto").toBool();
    if (_mode != cView)
    {
      _billToName->setEnabled(ffBillTo);
      _billToAddr->setEnabled(ffBillTo);
      _billToPhone->setEnabled(ffBillTo);
    }

    _billToName->setText(q.value("invchead_billto_name").toString());
    _billToAddr->setLine1(q.value("invchead_billto_address1").toString());
    _billToAddr->setLine2(q.value("invchead_billto_address2").toString());
    _billToAddr->setLine3(q.value("invchead_billto_address3").toString());
    _billToAddr->setCity(q.value("invchead_billto_city").toString());
    _billToAddr->setState(q.value("invchead_billto_state").toString());
    _billToAddr->setPostalCode(q.value("invchead_billto_zipcode").toString());
    _billToAddr->setCountry(q.value("invchead_billto_country").toString());
    _billToPhone->setText(q.value("invchead_billto_phone").toString());

    setFreeFormShipto(q.value("cust_ffshipto").toBool());
    _shipToName->setText(q.value("invchead_shipto_name").toString());
    _shipToAddr->setLine1(q.value("invchead_shipto_address1").toString());
    _shipToAddr->setLine2(q.value("invchead_shipto_address2").toString());
    _shipToAddr->setLine3(q.value("invchead_shipto_address3").toString());
    _shipToAddr->setCity(q.value("invchead_shipto_city").toString());
    _shipToAddr->setState(q.value("invchead_shipto_state").toString());
    _shipToAddr->setPostalCode(q.value("invchead_shipto_zipcode").toString());
    _shipToAddr->setCountry(q.value("invchead_shipto_country").toString());
    _shipToPhone->setText(q.value("invchead_shipto_phone").toString());
    _shipTo->blockSignals(true);
    _shipTo->setId(q.value("invchead_shipto_id").toInt());
    _shipTo->blockSignals(false);
    
    _payment->setLocalValue(q.value("invchead_payment").toDouble());
    _miscChargeDescription->setText(q.value("invchead_misc_descrip"));
    _miscChargeAccount->setId(q.value("invchead_misc_accnt_id").toInt());
    _miscAmount->setLocalValue(q.value("invchead_misc_amount").toDouble());

    _notes->setText(q.value("invchead_notes").toString());

    _posted = q.value("invchead_posted").toBool();
    if (_posted)
    {
      _new->setEnabled(false);
      disconnect(_invcitem, SIGNAL(valid(bool)), _edit, SLOT(setEnabled(bool)));
      disconnect(_invcitem, SIGNAL(valid(bool)), _delete, SLOT(setEnabled(bool)));
      _invoiceNumber->setEnabled(false);
      _invoiceDate->setEnabled(false);
      _terms->setEnabled(false);
      _salesrep->setEnabled(false);
      _commission->setEnabled(false);
      _taxzone->setEnabled(false);
      _shipChrgs->setEnabled(false);
      _project->setEnabled(false);
      _miscChargeAccount->setEnabled(false);
      _miscAmount->setEnabled(false);
      _freight->setEnabled(false);
    }

    _loading = false;

    sFillItemList();
  }
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void invoice::sFillItemList()
{
  q.prepare( "SELECT invcitem_id, invcitem_linenumber,"
             "       formatSoItemNumber(invcitem_coitem_id) AS soitemnumber, "
             "       CASE WHEN (item_id IS NULL) THEN invcitem_number"
             "            ELSE item_number"
             "       END AS itemnumber,"
             "       CASE WHEN (item_id IS NULL) THEN invcitem_descrip"
             "            ELSE (item_descrip1 || ' ' || item_descrip2)"
             "       END AS itemdescription,"
             "       quom.uom_name AS qtyuom,"
             "       invcitem_ordered, invcitem_billed,"
             "       puom.uom_name AS priceuom,"
             "       invcitem_price,"
             "       round((invcitem_billed * invcitem_qty_invuomratio) * (invcitem_price / "
	           "            (CASE WHEN(item_id IS NULL) THEN 1 "
	           "			            ELSE invcitem_price_invuomratio END)), 2) AS extprice,"
             "       'qty' AS invcitem_ordered_xtnumericrole,"
             "       'qty' AS invcitem_billed_xtnumericrole,"
             "       'salesprice' AS invcitem_price_xtnumericrole,"
             "       'curr' AS extprice_xtnumericrole "
             "FROM invcitem LEFT OUTER JOIN item on (invcitem_item_id=item_id) "
             "  LEFT OUTER JOIN uom AS quom ON (invcitem_qty_uom_id=quom.uom_id)"
             "  LEFT OUTER JOIN uom AS puom ON (invcitem_price_uom_id=puom.uom_id)"
             "WHERE (invcitem_invchead_id=:invchead_id) "
             "ORDER BY invcitem_linenumber;" );
  q.bindValue(":invchead_id", _invcheadid);
  q.bindValue(":curr_id", _custCurrency->id());
  q.bindValue(":date", _orderDate->date());
  q.exec();
  if (q.lastError().type() != QSqlError::NoError)
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);

  _invcitem->clear();
  _invcitem->populate(q);

  //  Determine the subtotal
  q.prepare( "SELECT SUM( round(((invcitem_billed * invcitem_qty_invuomratio) * (invcitem_price /"
             "            CASE WHEN (item_id IS NULL) THEN 1"
             "                 ELSE invcitem_price_invuomratio"
             "            END)),2) ) AS subtotal "
             "FROM invcitem LEFT OUTER JOIN item ON (invcitem_item_id=item_id) "
             "WHERE (invcitem_invchead_id=:invchead_id);" );
  q.bindValue(":invchead_id", _invcheadid);
  q.exec();
  if (q.first())
    _subtotal->setLocalValue(q.value("subtotal").toDouble());
  else if (q.lastError().type() != QSqlError::NoError)
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);

  _custCurrency->setEnabled(_invcitem->topLevelItemCount() == 0);

  // TODO: Calculate the Freight weight here.
  sCalculateTax();
}

void invoice::sCalculateTotal()
{
  _total->setLocalValue(_subtotal->localValue() + _miscAmount->localValue() +
			_freight->localValue() + _tax->localValue());
  _balance->setLocalValue(_total->localValue() -
			  _allocatedCM->localValue() - _authCC->localValue() -
			  _outstandingCM->localValue());
  if (_balance->localValue() < 0)
    _balance->setLocalValue(0);
}

void invoice::closeEvent(QCloseEvent *pEvent)
{
  if ( (_mode == cNew) && (_invcheadid != -1) )
  {
    q.prepare( "DELETE FROM invcitem "
               "WHERE (invcitem_invchead_id=:invchead_id);"
               "SELECT releaseInvcNumber(:invoiceNumber);" );
    q.bindValue(":invchead_id", _invcheadid);
    q.bindValue(":invoiceNumber", _invoiceNumber->text().toInt());
    q.exec();
    if (q.lastError().type() != QSqlError::NoError)
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
  }

  XWidget::closeEvent(pEvent);
}

void invoice::sTaxDetail()
{
  if (_mode != cView)
  {
    if (!save())
	  return;
  }

  ParameterList params;
  params.append("order_id", _invcheadid);
  params.append("order_type", "I");

  if (_mode == cView || _posted)
    params.append("mode", "view");
  else if (_mode == cNew || _mode == cEdit)
    params.append("mode", "edit");

  taxBreakdown newdlg(this, "", TRUE);
  if (newdlg.set(params) == NoError)
  {
    newdlg.exec();
    sCalculateTax();
  }
}

void invoice::sCalculateTax()
{
  XSqlQuery taxq;
  taxq.prepare( "SELECT SUM(tax) AS tax "
                "FROM ("
                "SELECT ROUND(SUM(taxdetail_tax),2) AS tax "
                "FROM tax "
                " JOIN calculateTaxDetailSummary('I', :invchead_id, 'T') ON (taxdetail_tax_id=tax_id)"
	        "GROUP BY tax_id) AS data;" );
  taxq.bindValue(":invchead_id", _invcheadid);
  taxq.exec();
  if (taxq.first())
    _tax->setLocalValue(taxq.value("tax").toDouble());
  else if (taxq.lastError().type() != QSqlError::NoError)
  {
    systemError(this, taxq.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
  // changing _tax fires sCalculateTotal()
}

void invoice::setFreeFormShipto(bool pFreeForm)
{
  _ffShipto = pFreeForm;

  if (_mode != cView)
  {
    _copyToShipto->setEnabled(_ffShipto);
    _shipToName->setEnabled(_ffShipto);
    _shipToAddr->setEnabled(_ffShipto);
    _shipToPhone->setEnabled(_ffShipto);
  }
  else
    _copyToShipto->setEnabled(false);
}

void invoice::sShipToModified()
{
  _shipTo->blockSignals(true);
  _shipTo->setId(-1);
  _shipTo->setCustid(_cust->id());
  _shipTo->blockSignals(false);
}

void invoice::keyPressEvent( QKeyEvent * e )
{
#ifdef Q_WS_MAC
  if(e->key() == Qt::Key_N && (e->modifiers() & Qt::ControlModifier))
  {
    _new->animateClick();
    e->accept();
  }
  else if(e->key() == Qt::Key_E && (e->modifiers() & Qt::ControlModifier))
  {
    _edit->animateClick();
    e->accept();
  }
  if(e->isAccepted())
    return;
#endif
  e->ignore();
}

void invoice::newInvoice(int pCustid, QWidget *parent )
{
  // Check for an Item window in new mode already.
  QWidgetList list = omfgThis->windowList();
  for(int i = 0; i < list.size(); i++)
  {
    QWidget * w = list.at(i);
    if(QString::compare(w->objectName(), "invoice new")==0)
    {
      w->setFocus();
      if(omfgThis->showTopLevel())
      {
        w->raise();
        w->activateWindow();
      }
      return;
    }
  }

  // If none found then create one.
  ParameterList params;
  params.append("mode", "new");
  if(-1 != pCustid)
    params.append("cust_id", pCustid);

  invoice *newdlg = new invoice(parent);
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void invoice::editInvoice( int pId, QWidget *parent  )
{
  // Check for an Item window in edit mode for the specified invoice already.
  QString n = QString("invoice edit %1").arg(pId);
  QWidgetList list = omfgThis->windowList();
  for(int i = 0; i < list.size(); i++)
  {
    QWidget * w = list.at(i);
    if(QString::compare(w->objectName(), n)==0)
    {
      w->setFocus();
      if(omfgThis->showTopLevel())
      {
        w->raise();
        w->activateWindow();
      }
      return;
    }
  }

  // If none found then create one.
  ParameterList params;
  params.append("invchead_id", pId);
  params.append("mode", "edit");

  invoice *newdlg = new invoice(parent);
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void invoice::viewInvoice( int pId, QWidget *parent  )
{
  // Check for an Item window in edit mode for the specified invoice already.
  QString n = QString("invoice view %1").arg(pId);
  QWidgetList list = omfgThis->windowList();
  for(int i = 0; i < list.size(); i++)
  {
    QWidget * w = list.at(i);
    if(QString::compare(w->objectName(), n)==0)
    {
      w->setFocus();
      if(omfgThis->showTopLevel())
      {
        w->raise();
        w->activateWindow();
      }
      return;
    }
  }

  // If none found then create one.
  ParameterList params;
  params.append("invchead_id", pId);
  params.append("mode", "view");

  invoice *newdlg = new invoice(parent);
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void invoice::sCreditAllocate()
{
  ParameterList params;
  params.append("doctype", "I");
  params.append("invchead_id", _invcheadid);
  params.append("cust_id", _cust->id());
  params.append("total",  _total->localValue());
  params.append("balance",  (_total->localValue() - _allocatedCM->localValue()));
  params.append("curr_id",   _total->id());
  params.append("effective", _total->effective());

  allocateARCreditMemo newdlg(this, "", TRUE);
  if (newdlg.set(params) == NoError && newdlg.exec() == XDialog::Accepted)
  {
    populateCMInfo();
  }
}

void invoice::populateCMInfo()
{
  XSqlQuery cm;

  // Allocated C/M's
  cm.prepare("SELECT COALESCE(SUM(currToCurr(aropenalloc_curr_id, :curr_id,"
            "                               aropenalloc_amount, :effective)),0) AS amount"
            "  FROM ( "
            "  SELECT aropenalloc_curr_id, aropenalloc_amount"
            "    FROM cohead JOIN aropenalloc ON (aropenalloc_doctype='S' AND aropenalloc_doc_id=cohead_id)"
            "   WHERE (cohead_number=:cohead_number) "
            "  UNION ALL"
            "  SELECT aropenalloc_curr_id, aropenalloc_amount"
            "    FROM aropenalloc"
            "   WHERE (aropenalloc_doctype='I' AND aropenalloc_doc_id=:invchead_id)"
            "       ) AS data;");
  cm.bindValue(":invchead_id", _invcheadid);
  cm.bindValue(":cohead_number", _orderNumber->text());
  cm.bindValue(":curr_id",     _allocatedCM->id());
  cm.bindValue(":effective",   _allocatedCM->effective());
  cm.exec();
  if(cm.first())
    _allocatedCM->setLocalValue(cm.value("amount").toDouble());
  else if (cm.lastError().type() != QSqlError::NoError)
  {
    systemError(this, cm.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
  else
    _allocatedCM->setLocalValue(0.00);

  // Unallocated C/M's
  cm.prepare("SELECT SUM(amount) AS amount"
            "  FROM ( SELECT aropen_id, "
	    "                currToCurr(aropen_curr_id, :curr_id,"
            "                           noNeg(aropen_amount - aropen_paid - SUM(COALESCE(aropenalloc_amount,0))),"
	    "                           :effective) AS amount"
            "           FROM aropen LEFT OUTER JOIN aropenalloc ON (aropenalloc_aropen_id=aropen_id)"
            "          WHERE ( (aropen_cust_id=:cust_id)"
            "            AND   (aropen_doctype IN ('C', 'R'))"
            "            AND   (aropen_open))"
            "          GROUP BY aropen_id, aropen_amount, aropen_paid, aropen_curr_id) AS data; ");
  cm.bindValue(":cust_id",     _cust->id());
  cm.bindValue(":curr_id",     _outstandingCM->id());
  cm.bindValue(":effective",   _outstandingCM->effective());
  cm.exec();
  if(cm.first())
    _outstandingCM->setLocalValue(cm.value("amount").toDouble());
  else if (cm.lastError().type() != QSqlError::NoError)
  {
    systemError(this, cm.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
  else
    _outstandingCM->setLocalValue(0.00);
}

void invoice::populateCCInfo()
{
  XSqlQuery cc;
  int ccValidDays = _metrics->value("CCValidDays").toInt();
  if(ccValidDays < 1)
    ccValidDays = 7;

  if (_mode == cNew)
  {
    cc.prepare("SELECT COALESCE(SUM(currToCurr(payco_curr_id, :curr_id,"
	      "                               payco_amount, :effective)),0) AS amount"
	      "  FROM ccpay, payco, cohead"
	      " WHERE ( (ccpay_status = 'A')"
	      "   AND   (date_part('day', CURRENT_TIMESTAMP - ccpay_transaction_datetime) < :ccValidDays)"
	      "   AND   (payco_ccpay_id=ccpay_id)"
	      "   AND   (payco_cohead_id=cohead_id)"
	      "   AND   (cohead_number=:cohead_number) );");
    cc.bindValue(":cohead_number", _orderNumber->text());
  }
  else
  {
    cc.prepare("SELECT COALESCE(SUM(currToCurr(payco_curr_id, :curr_id,"
	      "                               payco_amount, :effective)),0) AS amount"
	      "  FROM ccpay, payco, cohead, invchead"
	      " WHERE ( (ccpay_status = 'A')"
	      "   AND   (date_part('day', CURRENT_TIMESTAMP - ccpay_transaction_datetime) < :ccValidDays)"
	      "   AND   (payco_ccpay_id=ccpay_id)"
	      "   AND   (payco_cohead_id=cohead_id)"
	      "   AND   (invchead_ordernumber=cohead_number)"
	      "   AND   (invchead_id=:invchead_id) ); ");
    cc.bindValue(":invchead_id", _invcheadid);
  }
  cc.bindValue(":ccValidDays", ccValidDays);
  cc.bindValue(":curr_id",     _authCC->id());
  cc.bindValue(":effective",   _authCC->effective());
  cc.exec();
  if(cc.first())
    _authCC->setLocalValue(cc.value("amount").toDouble());
  else if (cc.lastError().type() != QSqlError::NoError)
  {
    systemError(this, cc.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
  else
    _authCC->setLocalValue(0.00);
}

void invoice::sHandleShipchrg(int pShipchrgid)
{
  if ( (_mode == cView) || (_mode == cViewQuote) )
    _freight->setEnabled(FALSE);
  else
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
  }
}

void invoice::sTaxZoneChanged()
{
  if (_loading == false && _invcheadid != -1 && _taxzoneidCache != _taxzone->id())
  {
    if (!save())
	  return;
    _taxzoneidCache = _taxzone->id();
    sCalculateTax();
  }
}

void invoice::sFreightChanged()
{
  if (_loading == false && _invcheadid != -1 && _freightCache != _freight->localValue())
  {
    if (!save())
	  return;
    _freightCache = _freight->localValue();
    sCalculateTax();
  }   
}

bool invoice::sCheckInvoiceNumber()
{
  if (cNew == _mode)
  {
    if (_invoiceNumber->text().isEmpty())
    {
      QMessageBox::warning(this, tr("Enter Invoice Number"),
                           tr("<p>You must enter an Invoice Number before "
                              "you may continue."));
      _invoiceNumber->setFocus();
    }
    else
    {
      XSqlQuery checkq;
      checkq.prepare("SELECT invchead_invcnumber"
                     "  FROM invchead"
                     " WHERE ((invchead_invcnumber=:number)"
                     "   AND  (invchead_id != :id));");
      checkq.bindValue(":number", _invoiceNumber->text());
      checkq.bindValue(":id",     _invcheadid);
      checkq.exec();
      if (checkq.first())
      {
        QMessageBox::warning(this, tr("Duplicate Invoice Number"),
                             tr("<p>%1 is already used by another Invoice. "
                                "Please enter a different Invoice Number.")
                             .arg(_invoiceNumber->text()));
        _invoiceNumber->setFocus();
      }
      else if (checkq.lastError().type() != QSqlError::NoError)
        systemError(this, q.lastError().text(), __FILE__, __LINE__);
      else
      {
        _invoiceNumber->setEnabled(false);
        return true;
      }
    }
  }
  else
    qWarning("invoice::sHandleInvoiceNumber() called but mode isn't cNew");

  return false;
}
