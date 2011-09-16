/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "creditMemo.h"

#include <QCloseEvent>
#include <QMessageBox>
#include <QSqlError>
#include <QValidator>
#include <QVariant>

#include "creditMemoItem.h"
#include "invoiceList.h"
#include "storedProcErrorLookup.h"
#include "taxBreakdown.h"

creditMemo::creditMemo(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  connect(_memoNumber, SIGNAL(lostFocus()), this, SLOT(sCheckCreditMemoNumber()));
  connect(_copyToShipto, SIGNAL(clicked()), this, SLOT(sCopyToShipto()));
  connect(_delete, SIGNAL(clicked()), this, SLOT(sDelete()));
  connect(_edit, SIGNAL(clicked()), this, SLOT(sEdit()));
  connect(_invoiceList, SIGNAL(clicked()), this, SLOT(sInvoiceList()));
  connect(_new, SIGNAL(clicked()), this, SLOT(sNew()));
  connect(_shipTo, SIGNAL(newId(int)), this, SLOT(populateShipto(int)));
  connect(_save, SIGNAL(clicked()), this, SLOT(sSave()));
  connect(_taxLit, SIGNAL(leftClickedURL(const QString&)), this, SLOT(sTaxDetail()));
  connect(_subtotal, SIGNAL(valueChanged()), this, SLOT(sCalculateTotal()));
  connect(_tax, SIGNAL(valueChanged()), this, SLOT(sCalculateTotal()));
  connect(_miscCharge, SIGNAL(valueChanged()), this, SLOT(sCalculateTotal()));
  connect(_freight,	SIGNAL(valueChanged()),	this, SLOT(sFreightChanged()));
  connect(_taxzone,	SIGNAL(newID(int)),	this, SLOT(sTaxZoneChanged()));
  connect(_cust, SIGNAL(newCrmacctId(int)), _billToAddr, SLOT(setSearchAcct(int)));
  connect(_cust, SIGNAL(newCrmacctId(int)), _shipToAddr, SLOT(setSearchAcct(int)));
  connect(_cust, SIGNAL(newId(int)),        _shipTo,     SLOT(setCustid(int)));
  connect(_shipToName, SIGNAL(textChanged(QString)), this, SLOT(sConvertShipto()));
  connect(_shipToAddr, SIGNAL(changed()), this, SLOT(sConvertShipto()));

#ifndef Q_WS_MAC
  _invoiceList->setMaximumWidth(25);
#endif

  _custtaxzoneid	= -1;
  _freightCache         = 0;
  _taxzoneidCache       = -1;

  _memoNumber->setValidator(omfgThis->orderVal());
  _commission->setValidator(omfgThis->scrapVal());

  _currency->setLabel(_currencyLit);

  _shipTo->setNameVisible(false);
  _shipTo->setDescriptionVisible(false);

  _cmitem->addColumn(tr("#"),           _seqColumn,   Qt::AlignCenter, true,  "cmitem_linenumber" );
  _cmitem->addColumn(tr("Item"),        _itemColumn,  Qt::AlignLeft,   true,  "item_number"   );
  _cmitem->addColumn(tr("Description"), -1,           Qt::AlignLeft,   true,  "description"   );
  _cmitem->addColumn(tr("Site"),        _whsColumn,   Qt::AlignCenter, true,  "warehous_code" );
  _cmitem->addColumn(tr("Qty UOM"),     _uomColumn,   Qt::AlignLeft,   true,  "qtyuom"   );
  _cmitem->addColumn(tr("Returned"),    _qtyColumn,   Qt::AlignRight,  true,  "cmitem_qtyreturned"  );
  _cmitem->addColumn(tr("Credited"),    _qtyColumn,   Qt::AlignRight,  true,  "cmitem_qtycredit"  );
  _cmitem->addColumn(tr("Price UOM"),   _uomColumn,   Qt::AlignLeft,   true,  "priceuom"   );
  _cmitem->addColumn(tr("Price"),       _priceColumn, Qt::AlignRight,  true,  "cmitem_unitprice"  );
  _cmitem->addColumn(tr("Extended"),    _moneyColumn, Qt::AlignRight,  true,  "extprice"  );

  _miscChargeAccount->setType(GLCluster::cRevenue | GLCluster::cExpense);
}

creditMemo::~creditMemo()
{
  // no need to delete child widgets, Qt does it all for us
}

void creditMemo::languageChange()
{
  retranslateUi(this);
}

enum SetResponse creditMemo::set(const ParameterList &pParams)
{
  XWidget::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("cmhead_id", &valid);
  if (valid)
  {
    _cmheadid = param.toInt();
    populate();
  }

  param = pParams.value("mode", &valid);
  if (valid)
  {
    _mode = cNew;

    if (param.toString() == "new")
    {
      q.prepare("SELECT NEXTVAL('cmhead_cmhead_id_seq') AS cmhead_id;");
      q.exec();
      if (q.first())
        _cmheadid = q.value("cmhead_id").toInt();
      else if (q.lastError().type() != QSqlError::NoError)
      {
	systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
	return UndefinedError;
      }

      setNumber();
      _memoDate->setDate(omfgThis->dbDate(), true);

      q.prepare("INSERT INTO cmhead ("
		"    cmhead_id, cmhead_number, cmhead_docdate, cmhead_posted"
		") VALUES ("
		"    :cmhead_id, :cmhead_number, :cmhead_docdate, false"
		");");
      q.bindValue(":cmhead_id",		_cmheadid);
      q.bindValue(":cmhead_number",	(!_memoNumber->text().isEmpty() ? _memoNumber->text() : QString("tmp%1").arg(_cmheadid)));
      q.bindValue(":cmhead_docdate",	_memoDate->date());
      q.exec();
      if (q.lastError().type() != QSqlError::NoError)
      {
	systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
	return UndefinedError;
      }

      connect(_cust, SIGNAL(newId(int)), this, SLOT(sPopulateCustomerInfo()));
      connect(_cust, SIGNAL(valid(bool)), _new, SLOT(setEnabled(bool)));
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;

      _memoNumber->setEnabled(FALSE);
      _cust->setReadOnly(TRUE);
      _invoiceNumber->setEnabled(FALSE);
      _invoiceList->hide();

      _new->setEnabled(TRUE);

      _save->setFocus();
    }
    else if (param.toString() == "view")
    {
      _mode = cView;

      _memoNumber->setEnabled(FALSE);
      _memoDate->setEnabled(FALSE);
      _cust->setReadOnly(TRUE);
      _invoiceNumber->setEnabled(FALSE);
      _salesRep->setEnabled(FALSE);
      _commission->setEnabled(FALSE);

      _billtoName->setEnabled(FALSE);
      _billToAddr->setEnabled(FALSE);

      _taxzone->setEnabled(FALSE);
      _rsnCode->setEnabled(FALSE);
      _customerPO->setEnabled(FALSE);
      _hold->setEnabled(FALSE);
      _miscCharge->setEnabled(FALSE);
      _miscChargeDescription->setEnabled(FALSE);
      _miscChargeAccount->setReadOnly(TRUE);
      _freight->setEnabled(FALSE);
      _comments->setEnabled(FALSE);
      _invoiceList->hide();
      _shipTo->setEnabled(FALSE);
      _shipToName->setEnabled(FALSE);
      _shipToAddr->setEnabled(FALSE);
      _currency->setEnabled(FALSE);
      _save->hide();
      _new->hide();
      _delete->hide();
      _edit->setText(tr("&View"));
      disconnect(_edit, SIGNAL(clicked()), this, SLOT(sEdit()));
      connect(_edit, SIGNAL(clicked()), this, SLOT(sView()));

      _close->setFocus();
    }
  }

  param = pParams.value("cust_id", &valid);
  if(cNew == _mode && valid)
    _cust->setId(param.toInt());

  return NoError;
}

void creditMemo::setNumber()
{
  if ( (_metrics->value("CMNumberGeneration") == "A") || 
       (_metrics->value("CMNumberGeneration") == "O")   )
  {
    q.prepare("SELECT fetchCmNumber() AS cmnumber;");
    q.exec();
    if (q.first())
    {
      _memoNumber->setText(q.value("cmnumber").toString());

      if (_metrics->value("CMNumberGeneration") == "A")
        _memoNumber->setEnabled(FALSE);
    }
    else if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }
  else if (_metrics->value("CMNumberGeneration") == "S")
  {
    q.prepare("SELECT fetchSoNumber() AS cmnumber;");
    q.exec();
    if (q.first())
    {
      _memoNumber->setText(q.value("cmnumber").toString());
      _memoNumber->setEnabled(FALSE);
    }
    else if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }
  else
    _memoNumber->setFocus();
}

void creditMemo::sSave()
{
  //  Make sure that all of the required field have been populated
  if (_memoNumber->text().length() == 0)
  {
    QMessageBox::warning( this, tr("Invalid Memo # Entered"),
                          tr( "<p>You must enter a valid Memo # for this Credit "
                          "Memo before you may save it." ) );
    _memoNumber->setFocus();
    return;
  }

  if ( _mode == cNew &&
       ( (_metrics->value("CMNumberGeneration") == "O") || 
         (_metrics->value("CMNumberGeneration") == "M")   ) )
  {
    XSqlQuery query;
    query.prepare( "SELECT cmhead_id "
                   "FROM cmhead "
                   "WHERE (cmhead_number=:cmhead_number)"
                   " AND (cmhead_id !=:cmhead_id);" );
    query.bindValue(":cmhead_number", _memoNumber->text());
    query.bindValue(":cmhead_id", _cmheadid);
    query.exec();
    if (query.first())
    {
      QMessageBox::warning( this, tr("Invalid Memo # Entered"),
                            tr( "<p>The Memo # is already in use.  "
                            "You must enter an unused Memo # for this Credit "
                            "Memo before you may save it." ) );
      _memoNumber->setFocus();
      return;
    }
  }

  if (!_cust->isValid())
  {
    QMessageBox::information(this, tr("Select a Customer"),
                             tr("Please select a Customer before continuing.") );
    _cust->setFocus();
    return;
  }

  if ( ! _miscCharge->isZero() && (!_miscChargeAccount->isValid()) )
  {
    QMessageBox::warning( this, tr("No Misc. Charge Account Number"),
                         tr("<p>You may not enter a Misc. Charge without "
			    "indicating the G/L Sales Account number for the "
			    "charge. Please set the Misc. Charge amount to 0 "
			    "or select a Misc. Charge Sales Account." ) );
    _creditMemoInformation->setCurrentIndex(_creditMemoInformation->indexOf(_itemsTab));
    _miscChargeAccount->setFocus();
    return;
  }
  
  if (_total->localValue() < 0 )
  {
    QMessageBox::information(this, tr("Total Less than Zero"),
                             tr("<p>The Total must be a positive value.") );
    _cust->setFocus();
    return;
  }

  // save address info in case someone wants to use 'em again later
  // but don't make any global changes to the data and ignore errors
  _billToAddr->save(AddressCluster::CHANGEONE);
  _shipToAddr->save(AddressCluster::CHANGEONE);
  
  // finally save the cmhead
  if (!save())
    return;
	
  omfgThis->sCreditMemosUpdated();

  _cmheadid = -1;
  close();
}

bool creditMemo::save()
{
  q.prepare( "UPDATE cmhead "
	     "SET cmhead_invcnumber=:cmhead_invcnumber, cmhead_cust_id=:cmhead_cust_id,"
       "    cmhead_number=:cmhead_number,"
	     "    cmhead_custponumber=:cmhead_custponumber, cmhead_hold=:cmhead_hold,"
	     "    cmhead_billtoname=:cmhead_billtoname, cmhead_billtoaddress1=:cmhead_billtoaddress1,"
	     "    cmhead_billtoaddress2=:cmhead_billtoaddress2, cmhead_billtoaddress3=:cmhead_billtoaddress3,"
	     "    cmhead_billtocity=:cmhead_billtocity, cmhead_billtostate=:cmhead_billtostate,"
	     "    cmhead_billtozip=:cmhead_billtozip,"
	     "    cmhead_billtocountry=:cmhead_billtocountry,"
	     "    cmhead_shipto_id=:cmhead_shipto_id,"
	     "    cmhead_shipto_name=:cmhead_shipto_name, cmhead_shipto_address1=:cmhead_shipto_address1,"
	     "    cmhead_shipto_address2=:cmhead_shipto_address2, cmhead_shipto_address3=:cmhead_shipto_address3,"
	     "    cmhead_shipto_city=:cmhead_shipto_city, cmhead_shipto_state=:cmhead_shipto_state,"
	     "    cmhead_shipto_zipcode=:cmhead_shipto_zipcode,"
	     "    cmhead_shipto_country=:cmhead_shipto_country,"
	     "    cmhead_docdate=:cmhead_docdate,"
	     "    cmhead_salesrep_id=:cmhead_salesrep_id, cmhead_commission=:cmhead_commission,"
	     "    cmhead_misc=:cmhead_misc, cmhead_misc_accnt_id=:cmhead_misc_accnt_id,"
	     "    cmhead_misc_descrip=:cmhead_misc_descrip,"
	     "    cmhead_freight=:cmhead_freight,"
	     "    cmhead_taxzone_id=:cmhead_taxzone_id,"
	     "    cmhead_comments=:cmhead_comments, "
	     "    cmhead_rsncode_id=:cmhead_rsncode_id, "
             "    cmhead_curr_id=:cmhead_curr_id, "
             "    cmhead_prj_id=:cmhead_prj_id "
	     "WHERE (cmhead_id=:cmhead_id);" );

  q.bindValue(":cmhead_id", _cmheadid);
  q.bindValue(":cmhead_cust_id", _cust->id());
  q.bindValue(":cmhead_number", _memoNumber->text());
  q.bindValue(":cmhead_invcnumber", _invoiceNumber->invoiceNumber());
  q.bindValue(":cmhead_custponumber", _customerPO->text().trimmed());
  q.bindValue(":cmhead_billtoname", _billtoName->text().trimmed());
  q.bindValue(":cmhead_billtoaddress1",	_billToAddr->line1());
  q.bindValue(":cmhead_billtoaddress2",	_billToAddr->line2());
  q.bindValue(":cmhead_billtoaddress3",	_billToAddr->line3());
  q.bindValue(":cmhead_billtocity",	_billToAddr->city());
  q.bindValue(":cmhead_billtostate",	_billToAddr->state());
  q.bindValue(":cmhead_billtozip",	_billToAddr->postalCode());
  q.bindValue(":cmhead_billtocountry",	_billToAddr->country());
  if (_shipTo->id() > 0)
    q.bindValue(":cmhead_shipto_id",	_shipTo->id());
  q.bindValue(":cmhead_shipto_name", _shipToName->text().trimmed());
  q.bindValue(":cmhead_shipto_address1", _shipToAddr->line1());
  q.bindValue(":cmhead_shipto_address2", _shipToAddr->line2());
  q.bindValue(":cmhead_shipto_address3", _shipToAddr->line3());
  q.bindValue(":cmhead_shipto_city",	 _shipToAddr->city());
  q.bindValue(":cmhead_shipto_state",	 _shipToAddr->state());
  q.bindValue(":cmhead_shipto_zipcode",	 _shipToAddr->postalCode());
  q.bindValue(":cmhead_shipto_country",	 _shipToAddr->country());
  q.bindValue(":cmhead_docdate", _memoDate->date());
  q.bindValue(":cmhead_comments", _comments->toPlainText());
  q.bindValue(":cmhead_salesrep_id", _salesRep->id());
  q.bindValue(":cmhead_rsncode_id", _rsnCode->id());
  q.bindValue(":cmhead_hold",       QVariant(_hold->isChecked()));
  q.bindValue(":cmhead_commission", (_commission->toDouble() / 100));
  q.bindValue(":cmhead_misc", _miscCharge->localValue());
  q.bindValue(":cmhead_misc_accnt_id", _miscChargeAccount->id());
  q.bindValue(":cmhead_misc_descrip", _miscChargeDescription->text());
  q.bindValue(":cmhead_freight", _freight->localValue());
  if (_taxzone->isValid())
    q.bindValue(":cmhead_taxzone_id",	_taxzone->id());
  q.bindValue(":cmhead_curr_id", _currency->id());
  if (_project->isValid())
    q.bindValue(":cmhead_prj_id", _project->id());
  q.exec();
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return false;
  }

  return true;
}

void creditMemo::sInvoiceList()
{
  ParameterList params;
  params.append("cust_id", _cust->id());
  params.append("invoiceNumber", _invoiceNumber->invoiceNumber());

  invoiceList newdlg(this, "", TRUE);
  newdlg.set(params);
  int invoiceid = newdlg.exec();

  if (invoiceid != 0)
  {

    XSqlQuery sohead;
    sohead.prepare( "SELECT invchead.* "
                    "FROM invchead "
                    "WHERE (invchead_id=:invcid) "
                    "LIMIT 1;" );
    sohead.bindValue(":invcid", invoiceid);
    sohead.exec();
    if (sohead.first())
    {
      _cust->setEnabled(FALSE);
      _billtoName->setEnabled(FALSE);
      _billToAddr->setEnabled(FALSE);

      _cust->setId(sohead.value("invchead_cust_id").toInt());
      _billtoName->setText(sohead.value("invchead_billto_name"));
      _billToAddr->setLine1(sohead.value("invchead_billto_address1").toString());
      _billToAddr->setLine2(sohead.value("invchead_billto_address2").toString());
      _billToAddr->setLine3(sohead.value("invchead_billto_address3").toString());
      _billToAddr->setCity(sohead.value("invchead_billto_city").toString());
      _billToAddr->setState(sohead.value("invchead_billto_state").toString());
      _billToAddr->setPostalCode(sohead.value("invchead_billto_zipcode").toString());
      _billToAddr->setCountry(sohead.value("invchead_billto_country").toString());

      _shipTo->setEnabled(FALSE);
      _shipToName->setEnabled(FALSE);
      _shipToAddr->setEnabled(FALSE);
      _ignoreShiptoSignals = TRUE;
      _shipToName->setText(sohead.value("invchead_shipto_name"));
      _shipToAddr->setLine1(sohead.value("invchead_shipto_address1").toString());
      _shipToAddr->setLine2(sohead.value("invchead_shipto_address2").toString());
      _shipToAddr->setLine3(sohead.value("invchead_shipto_address3").toString());
      _shipToAddr->setCity(sohead.value("invchead_shipto_city").toString());
      _shipToAddr->setState(sohead.value("invchead_shipto_state").toString());
      _shipToAddr->setPostalCode(sohead.value("invchead_shipto_zipcode").toString());
      _shipToAddr->setCountry(sohead.value("invchead_shipto_country").toString());
      _ignoreShiptoSignals = FALSE;

      _invoiceNumber->setInvoiceNumber(sohead.value("invchead_invcnumber").toString());
      _salesRep->setId(sohead.value("invchead_salesrep_id").toInt());
      _commission->setDouble(sohead.value("invchead_commission").toDouble() * 100);

      _taxzoneidCache = sohead.value("invchead_taxzone_id").toInt();
      _taxzone->setId(sohead.value("invchead_taxzone_id").toInt());
      _customerPO->setText(sohead.value("invchead_ponumber"));
      _project->setId(sohead.value("invchead_prj_id").toInt());
    }
    else if (sohead.lastError().type() != QSqlError::NoError)
    {
      systemError(this, sohead.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }
}

void creditMemo::populateShipto(int pShiptoid)
{
  if (pShiptoid != -1)
  {
    XSqlQuery query;
    query.prepare( "SELECT * "
                   "FROM shiptoinfo "
                   "WHERE (shipto_id=:shipto_id);" );
    query.bindValue(":shipto_id", pShiptoid);
    query.exec();
    if (query.first())
    {
      _ignoreShiptoSignals = true;

      _shipTo->setId(query.value("shipto_id").toInt());
      _shipToName->setText(query.value("shipto_name"));
      _shipToAddr->setId(query.value("shipto_addr_id").toInt());

      _ignoreShiptoSignals = false;

      _taxzoneidCache = query.value("shipto_taxzone_id").toInt();
      _taxzone->setId(query.value("shipto_taxzone_id").toInt());
      _salesRep->setId(query.value("shipto_salesrep_id").toInt());
      _commission->setDouble(query.value("shipto_commission").toDouble() * 100);
    }
    else if (query.lastError().type() != QSqlError::NoError)
    {
      systemError(this, query.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }
  else
  {
    _shipTo->setId(-1);
    _shipToName->clear();
    _shipToAddr->clear();
  }
}

void creditMemo::sPopulateCustomerInfo()
{
  if (!_invoiceNumber->isValid())
  {
    if (_cust->isValid())
    {
      XSqlQuery query;
      query.prepare( "SELECT cust_salesrep_id,"
                     "       cust_commprcnt,"
                     "       cust_taxzone_id, cust_curr_id, "
                     "       cust_name, cntct_addr_id, "
                     "       cust_ffshipto, cust_ffbillto, "
                     "       COALESCE(shipto_id, -1) AS shiptoid "
                     "FROM custinfo LEFT OUTER JOIN cntct ON (cust_cntct_id=cntct_id) "
					 "              LEFT OUTER JOIN shipto ON ((shipto_cust_id=cust_id) "
					 "                                     AND (shipto_default)) "
                     "WHERE (cust_id=:cust_id);" );
      query.bindValue(":cust_id", _cust->id());
      query.exec();
      if (query.first())
      {
        _ffShipto = query.value("cust_ffshipto").toBool();
        _copyToShipto->setEnabled(_ffShipto);
		_shipToName->setEnabled(_ffShipto);
		_shipToAddr->setEnabled(_ffShipto);

        _salesRep->setId(query.value("cust_salesrep_id").toInt());
        _commission->setDouble(query.value("cust_commprcnt").toDouble() * 100);

        _taxzoneidCache = query.value("cust_taxzone_id").toInt();
        _custtaxzoneid = query.value("cust_taxzone_id").toInt();
        _taxzone->setId(query.value("cust_taxzone_id").toInt());
        _currency->setId(query.value("cust_curr_id").toInt());

        _billtoName->setText(query.value("cust_name"));
        _billToAddr->setId(query.value("cntct_addr_id").toInt());

        bool ffBillTo;
        if ( (_mode == cNew) || (_mode == cEdit) )
          ffBillTo = query.value("cust_ffbillto").toBool();
        else
          ffBillTo = FALSE;

        _billtoName->setEnabled(ffBillTo);
        _billToAddr->setEnabled(ffBillTo);
	  
        if ((cNew == _mode) && (query.value("shiptoid").toInt() != -1))
          populateShipto(query.value("shiptoid").toInt());
      }
      else if (query.lastError().type() != QSqlError::NoError)
      {
        systemError(this, query.lastError().databaseText(), __FILE__, __LINE__);
        return;
      }
    }
    else
    {
      _salesRep->setCurrentIndex(-1);
      _taxzoneidCache = -1;
      _custtaxzoneid	= -1;
      _taxzone->setId(-1);

      _shipToName->setEnabled(_ffShipto);
      _shipToAddr->setEnabled(_ffShipto);
      _shipTo->setId(-1);
      _shipToName->clear();
      _shipToAddr->clear();
    }
  }
}

void creditMemo::sCheckCreditMemoNumber()
{
  if ( (_memoNumber->text().length()) && _mode != cNew &&
       ( (_metrics->value("CMNumberGeneration") == "O") || 
         (_metrics->value("CMNumberGeneration") == "M")   ) )
  {
    _memoNumber->setEnabled(FALSE);

    XSqlQuery query;
    query.prepare( "SELECT cmhead_id, cmhead_posted "
                   "FROM cmhead "
                   "WHERE (cmhead_number=:cmhead_number)" );
    query.bindValue(":cmhead_number", _memoNumber->text());
    query.exec();
    if (query.first())
    {
      _cmheadid = query.value("cmhead_id").toInt();

      _cust->setReadOnly(TRUE);

      populate();
     
      if (query.value("cmhead_posted").toBool())
      {
        _memoDate->setEnabled(FALSE);
        _invoiceNumber->setEnabled(FALSE);
        _salesRep->setEnabled(FALSE);
        _commission->setEnabled(FALSE);
        _taxzone->setEnabled(FALSE);
        _customerPO->setEnabled(FALSE);
        _hold->setEnabled(FALSE);
        _miscCharge->setEnabled(FALSE);
        _miscChargeDescription->setEnabled(FALSE);
        _miscChargeAccount->setReadOnly(TRUE);
        _freight->setEnabled(FALSE);
        _comments->setReadOnly(TRUE);
        _invoiceList->hide();
        _shipTo->setEnabled(FALSE);
        _shipToName->setEnabled(FALSE);
        _cmitem->setEnabled(FALSE);
        _save->hide();
        _new->hide();
        _delete->hide();
        _edit->hide();

        _mode = cView;
      }
      else
        _mode = cEdit;
    }
    else if (query.lastError().type() != QSqlError::NoError)
    {
      systemError(this, query.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }
}

void creditMemo::sConvertShipto()
{
  if (!_ignoreShiptoSignals)
  {
//  Convert the captive shipto to a free-form shipto
    _shipTo->blockSignals(true);
    _shipTo->setId(-1);
    _shipTo->setCustid(_cust->id());
    _shipTo->blockSignals(false);
  }
}

void creditMemo::sCopyToShipto()
{
  _shipTo->setId(-1);
  _shipToName->setText(_billtoName->text());
  if (_billToAddr->id() > 0)
    _shipToAddr->setId(_billToAddr->id());
  else
  {
    _shipToAddr->setLine1(_billToAddr->line1());
    _shipToAddr->setLine2(_billToAddr->line2());
    _shipToAddr->setLine3(_billToAddr->line1());
    _shipToAddr->setCity(_billToAddr->city());
    _shipToAddr->setState(_billToAddr->state());
    _shipToAddr->setPostalCode(_billToAddr->postalCode());
    _shipToAddr->setCountry(_billToAddr->country());
  }

  _taxzoneidCache = _custtaxzoneid;
  _taxzone->setId(_custtaxzoneid);
}

void creditMemo::sNew()
{
  if (!save())
    return;
	
  ParameterList params;
  params.append("mode", "new");
  params.append("cmhead_id", _cmheadid);

  creditMemoItem newdlg(this, "", TRUE);
  newdlg.set(params);
  
  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void creditMemo::sEdit()
{
  if (!save())
    return;
	
  ParameterList params;
  params.append("mode", "edit");
  params.append("cmhead_id", _cmheadid);
  params.append("cmitem_id", _cmitem->id());

  creditMemoItem newdlg(this, "", TRUE);
  newdlg.set(params);
  
  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void creditMemo::sView()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("cmhead_id", _cmheadid);
  params.append("cmitem_id", _cmitem->id());

  creditMemoItem newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void creditMemo::sDelete()
{
  q.prepare( "SELECT cmhead_posted "
             "FROM cmitem, cmhead "
             "WHERE ( (cmitem_cmhead_id=cmhead_id)"
             " AND (cmitem_id=:cmitem_id) );" );
  q.bindValue(":cmitem_id", _cmitem->id());
  q.exec();
  if (q.first())
  {
    if (q.value("cmhead_posted").toBool())
    {
      QMessageBox::information(this, "Line Item cannot be delete",
                               tr("<p>This Credit Memo has been Posted and "
				"this cannot be modified.") );
      return;
    }
  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  if (QMessageBox::question(this, "Delete current Line Item?",
                            tr("<p>Are you sure that you want to delete the "
			       "current Line Item?"),
			    QMessageBox::Yes | QMessageBox::Default,
			    QMessageBox::No | QMessageBox::Escape) == QMessageBox::Yes)
  {
    q.prepare( "DELETE FROM cmitem "
               "WHERE (cmitem_id=:cmitem_id);" );
    q.bindValue(":cmitem_id", _cmitem->id());
    q.exec();
    if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }

    sFillList();
  }
}

void creditMemo::sFillList()
{
  q.prepare( "SELECT cmitem_id, cmitem_linenumber, item_number,"
             "       (item_descrip1 || ' ' || item_descrip2) AS description,"
             "       warehous_code, quom.uom_name AS qtyuom,"
             "       cmitem_qtyreturned, cmitem_qtycredit,"
             "       puom.uom_name AS priceuom,"
             "       cmitem_unitprice,"
             "       (cmitem_qtycredit * cmitem_qty_invuomratio) *"
             "        (cmitem_unitprice / cmitem_price_invuomratio) AS extprice,"
             "       'qty' AS cmitem_qtyreturned_xtnumericrole,"
             "       'qty' AS cmitem_qtycredit_xtnumericrole,"
             "       'salesprice' AS cmitem_unitprice_xtnumericrole,"
             "       'curr' AS extprice_xtnumericrole "
             "FROM cmitem, itemsite, item, warehous, uom AS quom, uom AS puom "
             "WHERE ( (cmitem_itemsite_id=itemsite_id)"
             " AND (cmitem_qty_uom_id=quom.uom_id)"
             " AND (cmitem_price_uom_id=puom.uom_id)"
             " AND (itemsite_item_id=item_id)"
             " AND (itemsite_warehous_id=warehous_id)"
             " AND (cmitem_cmhead_id=:cmhead_id) ) "
             "ORDER BY cmitem_linenumber;" );
  q.bindValue(":cmhead_id", _cmheadid);
  q.exec();
  if (q.lastError().type() != QSqlError::NoError)
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);

  _cmitem->populate(q);
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  sCalculateSubtotal();
  sCalculateTax();

  _currency->setEnabled(_cmitem->topLevelItemCount() == 0);
}

void creditMemo::sCalculateSubtotal()
{
//  Determine the subtotal and line item tax
  XSqlQuery query;
  query.prepare( "SELECT SUM(round((cmitem_qtycredit * cmitem_qty_invuomratio) * (cmitem_unitprice / cmitem_price_invuomratio), 2)) AS subtotal "
                 "FROM cmitem, itemsite, item "
                 "WHERE ( (cmitem_itemsite_id=itemsite_id)"
                 " AND (itemsite_item_id=item_id)"
                 " AND (cmitem_cmhead_id=:cmhead_id) );" );
  query.bindValue(":cmhead_id", _cmheadid);
  query.exec();
  if (query.first())
    _subtotal->setLocalValue(query.value("subtotal").toDouble());
}

void creditMemo::sCalculateTotal()
{
  _total->setLocalValue(_subtotal->localValue() +
			_tax->localValue() +
			_freight->localValue() +
			_miscCharge->localValue() );
}

void creditMemo::populate()
{
  XSqlQuery cmhead;
  cmhead.prepare( "SELECT cmhead.*,"
                  "       cust_name, cust_ffbillto, cust_ffshipto "
                  "FROM cust, cmhead "
                  "WHERE ( (cmhead_cust_id=cust_id)"
                  " AND (cmhead_id=:cmhead_id) );" );
  cmhead.bindValue(":cmhead_id", _cmheadid);
  cmhead.exec();
  if (cmhead.first())
  {
    if (cmhead.value("cmhead_posted").toBool())
      _status->setText(tr("Posted"));
    else
      _status->setText(tr("Unposted"));

    _cust->setId(cmhead.value("cmhead_cust_id").toInt());
    _shipTo->setId(cmhead.value("cmhead_shipto_id").toInt());

    _salesRep->setId(cmhead.value("cmhead_salesrep_id").toInt());
    _commission->setDouble(cmhead.value("cmhead_commission").toDouble() * 100);
    // do taxauth first so we can overwrite the result of the signal cascade
    _taxzoneidCache = cmhead.value("cmhead_taxzone_id").toInt();
    if (!cmhead.value("cmhead_taxzone_id").isNull() && cmhead.value("cmhead_taxzone_id").toInt() != -1)
      _taxzone->setId(cmhead.value("cmhead_taxzone_id").toInt());

    _memoNumber->setText(cmhead.value("cmhead_number"));
    _memoDate->setDate(cmhead.value("cmhead_docdate").toDate(), true);
    _customerPO->setText(cmhead.value("cmhead_custponumber"));
    _hold->setChecked(cmhead.value("cmhead_hold").toBool());

    _currency->setId(cmhead.value("cmhead_curr_id").toInt());
    _freightCache = cmhead.value("cmhead_freight").toDouble();
    _freight->setLocalValue(cmhead.value("cmhead_freight").toDouble());

    _miscCharge->setLocalValue(cmhead.value("cmhead_misc").toDouble());
    _miscChargeDescription->setText(cmhead.value("cmhead_misc_descrip"));
    _miscChargeAccount->setId(cmhead.value("cmhead_misc_accnt_id").toInt());

    _comments->setText(cmhead.value("cmhead_comments").toString());

    bool ffBillTo = cmhead.value("cust_ffbillto").toBool();
    _billtoName->setEnabled(ffBillTo);
    _billToAddr->setEnabled(ffBillTo);

    _billtoName->setText(cmhead.value("cmhead_billtoname"));
    _billToAddr->setLine1(cmhead.value("cmhead_billtoaddress1").toString());
    _billToAddr->setLine2(cmhead.value("cmhead_billtoaddress2").toString());
    _billToAddr->setLine3(cmhead.value("cmhead_billtoaddress3").toString());
    _billToAddr->setCity(cmhead.value("cmhead_billtocity").toString());
    _billToAddr->setState(cmhead.value("cmhead_billtostate").toString());
    _billToAddr->setPostalCode(cmhead.value("cmhead_billtozip").toString());
    _billToAddr->setCountry(cmhead.value("cmhead_billtocountry").toString());

    _ffShipto = cmhead.value("cust_ffshipto").toBool();
    _shipToName->setEnabled(_ffShipto);
	_shipToAddr->setEnabled(_ffShipto);

    _shipToName->setText(cmhead.value("cmhead_shipto_name"));
    _shipToAddr->setLine1(cmhead.value("cmhead_shipto_address1").toString());
    _shipToAddr->setLine2(cmhead.value("cmhead_shipto_address2").toString());
    _shipToAddr->setLine3(cmhead.value("cmhead_shipto_address3").toString());
    _shipToAddr->setCity(cmhead.value("cmhead_shipto_city").toString());
    _shipToAddr->setState(cmhead.value("cmhead_shipto_state").toString());
    _shipToAddr->setPostalCode(cmhead.value("cmhead_shipto_zipcode").toString());
    _shipToAddr->setCountry(cmhead.value("cmhead_shipto_country").toString());

    if (!cmhead.value("cmhead_rsncode_id").isNull() && cmhead.value("cmhead_rsncode_id").toInt() != -1)
      _rsnCode->setId(cmhead.value("cmhead_rsncode_id").toInt());

    if (! cmhead.value("cmhead_invcnumber").toString().isEmpty())
      _invoiceNumber->setInvoiceNumber(cmhead.value("cmhead_invcnumber").toString());

    if (!cmhead.value("cmhead_prj_id").isNull())
      _project->setId(cmhead.value("cmhead_prj_id").toInt());

    sCalculateTax();
  }
  else if (cmhead.lastError().type() != QSqlError::NoError)
  {
    systemError(this, cmhead.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  sFillList();
}

void creditMemo::closeEvent(QCloseEvent *pEvent)
{
  if ( (_mode == cNew) && (_cmheadid != -1) )
  {
    q.prepare("SELECT deleteCreditMemo(:cmhead_id) AS result;");
    q.bindValue(":cmhead_id", _cmheadid);
    q.exec();
    if (q.first())
      ; // TODO: add error checking when function returns int instead of boolean
    else if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }

    if ( (_metrics->value("CMNumberGeneration") == "A") || 
         (_metrics->value("CMNumberGeneration") == "O")   )
      q.prepare("SELECT releaseCmNumber(:number) AS result;");
    else if (_metrics->value("CMNumberGeneration") == "S")
      q.prepare("SELECT releaseSoNumber(:number) AS result;");

    q.bindValue(":number", _memoNumber->text());
    q.exec();
    if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }

  XWidget::closeEvent(pEvent);
}

void creditMemo::sTaxDetail()
{
  if (!save())
    return;

  ParameterList params;
  params.append("order_id",	_cmheadid);
  params.append("order_type",	"CM");
  params.append("sense", -1);

  if(cView == _mode)
    params.append("mode", "view");
  else
    params.append("mode", "edit");

  taxBreakdown newdlg(this, "", TRUE);
  if (newdlg.set(params) == NoError)
  {
    newdlg.exec();
    populate();
  }
}

void creditMemo::sCalculateTax()
{
  XSqlQuery taxq;
  taxq.prepare( "SELECT SUM(tax) * -1 AS tax "
                "FROM ("
                "SELECT ROUND(SUM(taxdetail_tax),2) AS tax "
                "FROM tax "
                " JOIN calculateTaxDetailSummary('CM', :cmhead_id, 'T') ON (taxdetail_tax_id=tax_id)"
	        "GROUP BY tax_id) AS data;" );
  taxq.bindValue(":cmhead_id", _cmheadid);
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

void creditMemo::sTaxZoneChanged()
{
  if (_cmheadid != -1 && _taxzoneidCache != _taxzone->id())
  {
    if (!save())
      return;
    _taxzoneidCache = _taxzone->id();
    sCalculateTax();
  }
}

void creditMemo::sFreightChanged()
{
  if (_cmheadid != -1 && _freightCache != _freight->localValue())
  {
    if (!save())
      return;
    _freightCache = _freight->localValue();
    sCalculateTax();
    sCalculateTotal();
  }   
}

