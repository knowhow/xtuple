/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "returnAuthorization.h"

#include <QCloseEvent>
#include <QMessageBox>
#include <QSqlError>
#include <QValidator>
#include <QVariant>

#include <metasql.h>

#include "creditcardprocessor.h"
#include "enterPoitemReceipt.h"
#include "enterPoReceipt.h"
#include "mqlutil.h"
#include "returnAuthorizationItem.h"
#include "storedProcErrorLookup.h"
#include "taxBreakdown.h"
#include "freightBreakdown.h"
#include "printRaForm.h"

#include "salesOrder.h"
#include "salesOrderItem.h"
#include "dspShipmentsBySalesOrder.h"
#include "dspSalesOrderStatus.h"

returnAuthorization::returnAuthorization(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  _origso->setAllowedTypes(OrderLineEdit::Sales);
  _newso->setAllowedTypes(OrderLineEdit::Sales);
  _shipTo->setNameVisible(false);
  _shipTo->setDescriptionVisible(false);

  connect(_copyToShipto, SIGNAL(clicked()), this, SLOT(sCopyToShipto()));
  connect(_delete, SIGNAL(clicked()), this, SLOT(sDelete()));
  connect(_edit, SIGNAL(clicked()), this, SLOT(sEdit()));
  connect(_view, SIGNAL(clicked()), this, SLOT(sView()));
  connect(_new, SIGNAL(clicked()), this, SLOT(sNew()));
  connect(_shipTo, SIGNAL(newId(int)), this, SLOT(populateShipto(int)));
  connect(_save, SIGNAL(clicked()), this, SLOT(sSaveClick()));
  connect(_taxLit, SIGNAL(leftClickedURL(const QString&)), this, SLOT(sTaxDetail()));
  connect(_freightLit, SIGNAL(leftClickedURL(const QString&)), this, SLOT(sFreightDetail()));
  connect(_subtotal, SIGNAL(valueChanged()), this, SLOT(sCalculateTotal()));
  connect(_tax, SIGNAL(valueChanged()), this, SLOT(sCalculateTotal()));
  connect(_miscCharge, SIGNAL(valueChanged()), this, SLOT(sCalculateTotal()));
  connect(_freight, SIGNAL(valueChanged()), this, SLOT(sFreightChanged()));
  connect(_taxzone, SIGNAL(newID(int)), this, SLOT(sTaxZoneChanged()));
  connect(_warehouse, SIGNAL(newID(int)), this, SLOT(sRecvWhsChanged()));
  connect(_shipWhs, SIGNAL(newID(int)), this, SLOT(sShipWhsChanged()));
  connect(_origso, SIGNAL(newId(int,QString)), this, SLOT(sOrigSoChanged()));
  connect(_shipToAddr, SIGNAL(changed()), this, SLOT(sClearShiptoNumber()));
  connect(_shipToName, SIGNAL(textChanged(QString)), this, SLOT(sClearShiptoNumber()));
  connect(_disposition, SIGNAL(currentIndexChanged(int)), this, SLOT(sDispositionChanged()));
  connect(_creditBy, SIGNAL(currentIndexChanged(int)), this, SLOT(sCreditByChanged()));
  connect(_authorizeLine, SIGNAL(clicked()), this, SLOT(sAuthorizeLine()));
  connect(_clearAuthorization, SIGNAL(clicked()), this, SLOT(sClearAuthorization()));
  connect(_authorizeAll, SIGNAL(clicked()), this, SLOT(sAuthorizeAll()));
  connect(_enterReceipt, SIGNAL(clicked()), this, SLOT(sEnterReceipt()));
  connect(_receiveAll,   SIGNAL(clicked()), this, SLOT(sReceiveAll()));
  connect(_raitem,     SIGNAL(valid(bool)), this, SLOT(sHandleEnterReceipt(bool)));
  connect(_raitem,     SIGNAL(valid(bool)), this, SLOT(sHandleAction()));
  connect(_action, SIGNAL(clicked()), this, SLOT(sAction()));
  connect(omfgThis, SIGNAL(salesOrdersUpdated(int, bool)), this, SLOT(sHandleSalesOrderEvent(int, bool)));
  connect(_refund, SIGNAL(clicked()), this, SLOT(sRefund()));
  connect(_postReceipts, SIGNAL(clicked()), this, SLOT(sPostReceipts()));
  connect(_raitem, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*)), this, SLOT(sPopulateMenu(QMenu*, QTreeWidgetItem*)));
  connect(_authNumber, SIGNAL(textEdited(QString)), this, SLOT(sCheckNumber()));
  connect(_cust, SIGNAL(newCrmacctId(int)), _billToAddr, SLOT(setSearchAcct(int)));
  connect(_cust, SIGNAL(newCrmacctId(int)), _shipToAddr, SLOT(setSearchAcct(int)));
  connect(_cust, SIGNAL(newId(int)),        _shipTo,     SLOT(setCustid(int)));

  _commission->setValidator(omfgThis->percentVal());
  _newso->setReadOnly(true);

  _custtaxzoneid       = -1;
  _ignoreShiptoSignals = false;
  _ignoreSoSignals = false;
  _ignoreWhsSignals = false;
  _ffBillto = TRUE;
  _ffShipto = TRUE;
  _custEmail = FALSE;
  _saved = FALSE;
  _freightCache = 0;

  _authNumber->setValidator(omfgThis->orderVal());
  _comments->setType(Comments::ReturnAuth);

  _currency->setLabel(_currencyLit);

  _raitem->addColumn(tr("#"),             _seqColumn,   Qt::AlignCenter,true,  "f_linenumber");
  _raitem->addColumn(tr("Kit Seq. #"),    _seqColumn,   Qt::AlignRight, false, "raitem_subnumber");
  _raitem->addColumn(tr("Item"),          _itemColumn,  Qt::AlignLeft,   true, "item_number"   );
  _raitem->addColumn(tr("UOM"),           _statusColumn,Qt::AlignLeft,   true, "uom_name"   );
  _raitem->addColumn(tr("Description"),   -1,           Qt::AlignLeft,   true, "item_descrip"   );
  _raitem->addColumn(tr("Site"),          _whsColumn,   Qt::AlignCenter, true, "warehous_code" );
  _raitem->addColumn(tr("Status"),        _statusColumn,Qt::AlignCenter, true, "raitem_status" );
  _raitem->addColumn(tr("Disposition"),   _itemColumn,  Qt::AlignLeft,   true, "disposition"   );
  _raitem->addColumn(tr("Warranty"),      _qtyColumn,Qt::AlignCenter,    true, "raitem_warranty");
  _raitem->addColumn(tr("Sold"),          _qtyColumn,   Qt::AlignRight,  true, "oldcoitem_qtyshipped"  );
  _raitem->addColumn(tr("Authorized"),    _qtyColumn,   Qt::AlignRight,  true, "raitem_qtyauthorized"  );
  _raitem->addColumn(tr("Received"),      _qtyColumn,   Qt::AlignRight,  true, "raitem_qtyreceived"  );
  _raitem->addColumn(tr("To Receive"),    _qtyColumn,   Qt::AlignRight,  true, "raitem_qtytoreceive"  );
  _raitem->addColumn(tr("Shipped"),       _qtyColumn,   Qt::AlignRight,  true, "newcoitem_qtyshipped"  );
  _raitem->addColumn(tr("Credit Price"),  _priceColumn, Qt::AlignRight,  true, "raitem_unitprice"  );
  _raitem->addColumn(tr("Extended"),      _moneyColumn, Qt::AlignRight,  true, "raitem_extprice"  );
  _raitem->addColumn(tr("Credited"),      _moneyColumn, Qt::AlignRight,  true, "raitem_amtcredited"  );
  _raitem->addColumn(tr("Sale Price"),    _priceColumn, Qt::AlignRight,  true, "raitem_saleprice"  );
  _raitem->addColumn(tr("Extended"),      _moneyColumn, Qt::AlignRight,  true, "raitem_extsaleprice"  );
  _raitem->addColumn(tr("Net Due"),       _moneyColumn, Qt::AlignRight,  true, "raitem_netdue"  );
  _raitem->addColumn(tr("Orig. Order"),   _itemColumn,  Qt::AlignLeft,   true, "oldcohead_number"   );
  _raitem->addColumn(tr("New Order"),     _itemColumn,  Qt::AlignLeft,   true, "newcohead_number"   );
  _raitem->addColumn(tr("Sched. Date"),   _dateColumn,  Qt::AlignLeft,   true, "raitem_scheddate"   );
  _raitem->addColumn(tr("Item Type"),     _statusColumn,Qt::AlignLeft,  false, "item_type"   );

  _raitem->setSelectionMode(QAbstractItemView::ExtendedSelection);

  _authorizeLine->hide();
  _clearAuthorization->hide();
  _authorizeAll->hide();

  if (! _metrics->boolean("CCAccept") || !_privileges->check("ProcessCreditCards"))
  {
    _refund->hide();
    _CCCVVLit->hide();
    _CCCVV->hide();
  }
  
  if (! _metrics->boolean("CCAccept"))
    _creditBy->removeItem(3);
  
  _printRA->setChecked(_metrics->boolean("DefaultPrintRAOnSave"));

  _receiveAll->setEnabled(_privileges->check("EnterReceipts"));
  _postReceipts->setEnabled(_privileges->check("EnterReceipts"));

  //If not multi-warehouse hide whs controls
  if (!_metrics->boolean("MultiWhs"))
  {
    _warehouseLit->hide();
    _warehouse->hide();
    _shipWhs->hide();
    _shipWhsLit->hide();
  } 

  _miscChargeAccount->setType(GLCluster::cRevenue | GLCluster::cExpense);
}

returnAuthorization::~returnAuthorization()
{
    // no need to delete child widgets, Qt does it all for us
}

void returnAuthorization::languageChange()
{
    retranslateUi(this);
}

enum SetResponse returnAuthorization::set(const ParameterList &pParams)
{
  XWidget::set(pParams);
  QVariant param;
  bool     valid;
  QString metric;

  param = pParams.value("mode", &valid);
  if (valid)
  {
    _mode = cNew;

    if (param.toString() == "new")
    {
      q.prepare("SELECT NEXTVAL('rahead_rahead_id_seq') AS rahead_id;");
      q.exec();
      if (q.first())
      {
        _raheadid = q.value("rahead_id").toInt();
        _comments->setId(_raheadid);
      }
      else if (q.lastError().type() != QSqlError::NoError)
      {
        systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
        return UndefinedError;
      }
      _cust->setFocus();      
      setNumber();
      _authDate->setDate(omfgThis->dbDate(), true);

      q.prepare("INSERT INTO rahead ("
                "    rahead_id, rahead_number, rahead_authdate"
                ") VALUES ("
                "    :rahead_id, COALESCE(:rahead_number,0), :rahead_authdate"
                ");");
      q.bindValue(":rahead_id", _raheadid);
      if (!_authNumber->text().isEmpty())
        q.bindValue(":rahead_number", _authNumber->text());
      q.exec();
      if (q.lastError().type() != QSqlError::NoError)
      {
        systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
        return UndefinedError;
      }

      metric = _metrics->value("DefaultRaDisposition");
      if (metric == "C")
        _disposition->setCurrentIndex(0);
      else if (metric == "R")
        _disposition->setCurrentIndex(1);
      else if (metric == "P")
        _disposition->setCurrentIndex(2);
      else if (metric == "V")
        _disposition->setCurrentIndex(3);
      else if (metric == "M")
        _disposition->setCurrentIndex(4);
      else
        _disposition->setCurrentIndex(5);

      metric = _metrics->value("DefaultRaTiming");
      if (metric == "I")
        _timing->setCurrentIndex(0);
      else if (metric == "R")
        _timing->setCurrentIndex(1);
      else
        _timing->setCurrentIndex(2);

      metric = _metrics->value("DefaultRaCreditMethod");
      if (metric == "N")
        _creditBy->setCurrentIndex(0);
      else if (metric == "M")
        _creditBy->setCurrentIndex(1);
      else if (metric == "K")
        _creditBy->setCurrentIndex(2);
      else if (metric == "C")
        _creditBy->setCurrentIndex(3);
      else
        _creditBy->setCurrentIndex(4);

      connect(_cust, SIGNAL(newId(int)), this, SLOT(sPopulateCustomerInfo()));
      connect(_cust, SIGNAL(valid(bool)), _new, SLOT(setEnabled(bool)));
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;

      _authNumber->setEnabled(FALSE);
      _save->setFocus();
      _cancel->setText("&Close");
      
      connect(_authNumber, SIGNAL(lostFocus()), this, SLOT(sCheckAuthorizationNumber()));
    }
    else if (param.toString() == "view")
    {
      _mode = cView;
      disconnect(_raitem, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*)), this, SLOT(sPopulateMenu(QMenu*, QTreeWidgetItem*)));
    }
  }

  param = pParams.value("incdt_id", &valid);
  if (valid)
    _incident->setId(param.toInt());

  param = pParams.value("cust_id", &valid);
  if(cNew == _mode && valid)
    _cust->setId(param.toInt());

  param = pParams.value("sohead_id", &valid);
  if (cNew == _mode && valid)
    _origso->setId(param.toInt());

  param = pParams.value("rahead_id", &valid);
  if (valid)
  {
    _raheadid = param.toInt();
    _comments->setId(_raheadid);
    populate();
  }

  if (_mode == cView)
  {
      _authNumber->setEnabled(FALSE);
      _authDate->setEnabled(FALSE);
      _expireDate->setEnabled(FALSE);
      _salesRep->setEnabled(FALSE);
      _commission->setEnabled(FALSE);
      _taxzone->setEnabled(FALSE);
      _rsnCode->setEnabled(FALSE);
      _disposition->setEnabled(FALSE);
      _timing->setEnabled(FALSE);
      _creditBy->setEnabled(FALSE);

      _origso->setEnabled(FALSE);
      _newso->setEnabled(FALSE);
      _incident->setEnabled(FALSE);
      _project->setEnabled(FALSE);

      _cust->setReadOnly(TRUE);
      _billToName->setEnabled(FALSE);
      _billToAddr->setEnabled(FALSE);

      _customerPO->setEnabled(FALSE);

      _miscCharge->setEnabled(FALSE);
      _miscChargeDescription->setEnabled(FALSE);
      _miscChargeAccount->setReadOnly(TRUE);
      _freight->setEnabled(FALSE);
      _notes->setEnabled(FALSE);
      _comments->setEnabled(FALSE);
      _copyToShipto->setEnabled(FALSE);
      _shipTo->setEnabled(FALSE);
      _shipToName->setEnabled(FALSE);
      _shipToAddr->setEnabled(FALSE);
      _currency->setEnabled(FALSE);
      _warehouse->setEnabled(FALSE);
      _shipWhs->setEnabled(FALSE);
      _save->hide();
      _new->hide();
      _delete->hide();
      _edit->hide();
      _action->hide();
      _authorizeLine->hide();
      _clearAuthorization->hide();
      _authorizeAll->hide();
      _enterReceipt->hide();
      _receiveAll->hide();
      _postReceipts->hide();

      _cancel->setText("&Close");
      _cancel->setFocus();
  }
  
  return NoError;
}

void returnAuthorization::setNumber()
{
  if ( (_metrics->value("RANumberGeneration") == "A") || 
       (_metrics->value("RANumberGeneration") == "O")   )
  {
    q.prepare("SELECT fetchRaNumber() AS ranumber;");
    q.exec();
    if (q.first())
    {
      _authNumber->setText(q.value("ranumber").toString());

      if (_metrics->value("RANumberGeneration") == "A")
        _authNumber->setEnabled(FALSE);
    }
    else if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }
  else
    _authNumber->setFocus();
}

bool returnAuthorization::sSave(bool partial)
{
  const char *dispositionTypes[] = { "C", "R", "P", "V", "M", "" };
  const char *timingTypes[] = { "I", "R", "" };
  const char *creditMethods[] = { "N", "M", "K", "C", "" };

  QString disposition = QString(dispositionTypes[_disposition->currentIndex()]);
  QString timing = QString(timingTypes[_timing->currentIndex()]);
  QString creditBy = QString(creditMethods[_creditBy->currentIndex()]);
  
  if ( !partial && (disposition.isEmpty()) )
  {
    QMessageBox::warning( this, tr("Invalid Disposition"),
                         tr("<p>You must enter a Disposition." ) );
    _disposition->setFocus();
    return false;
  }
  
  if ( !partial && (timing.isEmpty()) )
  {
    QMessageBox::warning( this, tr("Invalid Timing"),
                         tr("<p>You must enter a Timing." ) );
    _timing->setFocus();
    return false;
  }
  
  if ( !partial && (creditBy.isEmpty()) )
  {
    QMessageBox::warning( this, tr("Invalid Credit Method"),
                         tr("<p>You must enter a Credit Method." ) );
    _creditBy->setFocus();
    return false;
  }
  
  if (_authNumber->text().isEmpty())
  {
    if(partial && !isVisible())
      return false;
    QMessageBox::critical( this, tr("Cannot Save Return Authorization"),
      tr("You may not save this Return Authorization until you have entered a valid Authorization Number.") );
    _authNumber->setFocus();
    return false;
  }

  if ( !partial && (disposition == "C") && (creditBy == "N") )
  {
    QMessageBox::warning( this, tr("Invalid Credit Method"),
                         tr("<p>You may not enter a Disposition of Credit "
                            "and a Credit By of None." ) );
    _creditBy->setFocus();
    return false;
  }
  
  if ( ! _miscCharge->isZero() && (!_miscChargeAccount->isValid()) )
  {
    QMessageBox::warning( this, tr("No Misc. Charge Account Number"),
                         tr("<p>You may not enter a Misc. Charge without "
                            "indicating the G/L Sales Account number for the "
                            "charge. Please set the Misc. Charge amount to 0 "
                            "or select a Misc. Charge Sales Account." ) );
    _returnAuthInformation->setCurrentIndex(_returnAuthInformation->indexOf(_lineItemsPage));
    _miscChargeAccount->setFocus();
    return false;
  }
  
  if (!partial && _raitem->topLevelItemCount() == 0)
  {
    QMessageBox::warning( this, tr("Create Line Items for this Order"),
                          tr("<p>You must create at least one Line Item for "
                          "this Return Authorization before you may save it."));
    _new->setFocus();
    return FALSE;
  }

  if (_total->localValue() < 0 )
  {
    QMessageBox::information(this, tr("Total Less than Zero"),
                             tr("<p>The Total must be a positive value.") );
    _cust->setFocus();
    return false;
  }

  if ( !partial && !_warehouse->isValid() )
  {
    QMessageBox::warning( this, tr("Invalid Receiving Site"),
                         tr("<p>You must enter a valid Receiving Site." ) );
    _warehouse->setFocus();
    return false;
  }

  if ( !partial && !_shipWhs->isValid() )
  {
    QMessageBox::warning( this, tr("Invalid Shipping Site"),
                         tr("<p>You must enter a valid Shipping Site." ) );
    _shipWhs->setFocus();
    return false;
  }

  // save address info in case someone wants to use 'em again later
  // but don't make any global changes to the data and ignore errors
  _billToAddr->save(AddressCluster::CHANGEONE);
  _shipToAddr->save(AddressCluster::CHANGEONE);

  q.prepare( "UPDATE rahead "
             "   SET rahead_cust_id=:rahead_cust_id,rahead_number=:rahead_number,"
             "       rahead_authdate=:rahead_authdate,rahead_expiredate=:rahead_expiredate,"
             "       rahead_salesrep_id=:rahead_salesrep_id, rahead_commission=:rahead_commission,"
             "       rahead_taxzone_id=:rahead_taxzone_id,rahead_rsncode_id=:rahead_rsncode_id,"
             "       rahead_disposition=:rahead_disposition,rahead_timing=:rahead_timing,"
             "       rahead_creditmethod=:rahead_creditmethod,rahead_orig_cohead_id=:rahead_orig_cohead_id,"
             "       rahead_incdt_id=:rahead_incdt_id,rahead_prj_id=:rahead_prj_id,"
             "       rahead_billtoname=:rahead_billtoname, rahead_billtoaddress1=:rahead_billtoaddress1,"
             "       rahead_billtoaddress2=:rahead_billtoaddress2, rahead_billtoaddress3=:rahead_billtoaddress3,"
             "       rahead_billtocity=:rahead_billtocity, rahead_billtostate=:rahead_billtostate,"
             "       rahead_billtozip=:rahead_billtozip,rahead_billtocountry=:rahead_billtocountry,"
             "       rahead_shipto_id=:rahead_shipto_id,"
             "       rahead_shipto_name=:rahead_shipto_name, rahead_shipto_address1=:rahead_shipto_address1,"
             "       rahead_shipto_address2=:rahead_shipto_address2, rahead_shipto_address3=:rahead_shipto_address3,"
             "       rahead_shipto_city=:rahead_shipto_city, rahead_shipto_state=:rahead_shipto_state,"
             "       rahead_shipto_zipcode=:rahead_shipto_zipcode,rahead_shipto_country=:rahead_shipto_country,"
             "       rahead_custponumber=:rahead_custponumber,rahead_notes=:rahead_notes, "
             "       rahead_misc_accnt_id=:rahead_misc_accnt_id,rahead_misc=:rahead_misc, "
             "       rahead_misc_descrip=:rahead_misc_descrip, rahead_curr_id=:rahead_curr_id,"
             "       rahead_freight=:rahead_freight, rahead_printed=:rahead_printed,"
             "       rahead_warehous_id=:rahead_warehous_id, rahead_cohead_warehous_id=:rahead_cohead_warehous_id "
             " WHERE(rahead_id=:rahead_id);" );

  q.bindValue(":rahead_id", _raheadid);
  q.bindValue(":rahead_number", _authNumber->text().toInt());
  q.bindValue(":rahead_authdate", _authDate->date());
  q.bindValue(":rahead_expiredate", _expireDate->date());
  if (_salesRep->isValid() && _cust->isValid())
    q.bindValue(":rahead_salesrep_id", _salesRep->id());
  q.bindValue(":rahead_commission", (_commission->toDouble() / 100));
  if (_taxzone->isValid())
    q.bindValue(":rahead_taxzone_id", _taxzone->id());
  if (_rsnCode->id() > 0)
    q.bindValue(":rahead_rsncode_id", _rsnCode->id());
  if (!disposition.isEmpty())
    q.bindValue(":rahead_disposition", disposition);
  if (!timing.isEmpty())
    q.bindValue(":rahead_timing", timing);
  if (!creditBy.isEmpty())
    q.bindValue(":rahead_creditmethod", creditBy);
  if (_origso->isValid())
    q.bindValue(":rahead_orig_cohead_id", _origso->id());
  if (_newso->isValid())
    q.bindValue(":rahead_new_cohead_id", _newso->id());
  if (_incident->isValid())
    q.bindValue(":rahead_incdt_id", _incident->id());
  if (_project->isValid())
    q.bindValue(":rahead_prj_id", _project->id());
  if (_cust->isValid())
    q.bindValue(":rahead_cust_id", _cust->id());
  q.bindValue(":rahead_billtoname", _billToName->text().trimmed());
  q.bindValue(":rahead_billtoaddress1", _billToAddr->line1());
  q.bindValue(":rahead_billtoaddress2", _billToAddr->line2());
  q.bindValue(":rahead_billtoaddress3", _billToAddr->line3());
  q.bindValue(":rahead_billtocity",     _billToAddr->city());
  q.bindValue(":rahead_billtostate",    _billToAddr->state());
  q.bindValue(":rahead_billtozip",      _billToAddr->postalCode());
  q.bindValue(":rahead_billtocountry",  _billToAddr->country());
  if (_shipTo->id() > 0)
    q.bindValue(":rahead_shipto_id",    _shipTo->id());
  q.bindValue(":rahead_shipto_name", _shipToName->text().trimmed());
  q.bindValue(":rahead_shipto_address1", _shipToAddr->line1());
  q.bindValue(":rahead_shipto_address2", _shipToAddr->line2());
  q.bindValue(":rahead_shipto_address3", _shipToAddr->line3());
  q.bindValue(":rahead_shipto_city",     _shipToAddr->city());
  q.bindValue(":rahead_shipto_state",    _shipToAddr->state());
  q.bindValue(":rahead_shipto_zipcode",  _shipToAddr->postalCode());
  q.bindValue(":rahead_shipto_country",  _shipToAddr->country());
  q.bindValue(":rahead_custponumber", _customerPO->text().trimmed());
  q.bindValue(":rahead_notes", _notes->toPlainText());
  q.bindValue(":rahead_misc", _miscCharge->localValue());
  if (_miscChargeAccount->id() != -1)
    q.bindValue(":rahead_misc_accnt_id", _miscChargeAccount->id());
  q.bindValue(":rahead_misc_descrip", _miscChargeDescription->text());
  q.bindValue(":rahead_curr_id", _currency->id());
  q.bindValue(":rahead_freight", _freight->localValue());
  q.bindValue(":rahead_warehous_id", _warehouse->id());
  q.bindValue(":rahead_cohead_warehous_id", _shipWhs->id());

  q.exec();
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return false;
  }

  omfgThis->sReturnAuthorizationsUpdated();
  omfgThis->sProjectsUpdated(_project->id());
  _saved = TRUE;
  _comments->setId(_raheadid);
  
  connect(_authNumber, SIGNAL(lostFocus()), this, SLOT(sCheckAuthorizationNumber()));
  
  emit saved(_raheadid);

  return true;
}

void returnAuthorization::sPostReceipts()
{
   enterPoReceipt::post("RA", _raheadid);
   sFillList();
   _mode = cEdit;
   _cancel->setText("&Close");
}

void returnAuthorization::sSaveClick()
{
  if (sSave(false))
  {
    if (_printRA->isChecked())
    {
      ParameterList params;
      params.append("rahead_id", _raheadid);

      printRaForm newdlgS(this, "", true);
      newdlgS.set(params);
      newdlgS.exec();

/*   TO DO: This isn't going to work right now because the EDI profile is specific to S/O.
     We really need to rearchitect the EDI profiles for customers to one to many
     With profile types for Invoice, S/O, R/A etc...
     
      if (_custEmail && _metrics->boolean("EnableBatchManager"))
      {
        deliverReturnAuthorization newdlgD(this, "", true);
        newdlgD.set(params);
        newdlgD.exec();
      }
  */
    }
    _raheadid=-1;

    close();
  }
}

void returnAuthorization::sOrigSoChanged()
{
  if (_origso->isValid())
  {
    _authorizeLine->show();
    _clearAuthorization->show();
    _authorizeAll->show();
  }
  else
  {
    _authorizeLine->hide();
    _clearAuthorization->hide();
    _authorizeAll->hide();
  }
  if (_origso->isValid())
  {
    q.prepare("SELECT rahead_number FROM rahead,raitem "
              " WHERE((rahead_orig_cohead_id=:cohead_id) "
              "   AND (rahead_id != :rahead_id) "
              "   AND (raitem_rahead_id=rahead_id) "
              "   AND (raitem_status = 'O') "
              "   AND (raitem_qtyauthorized > 0)); ");
    q.bindValue(":cohead_id",_origso->id());
    q.bindValue(":rahead_id",_raheadid);
    q.exec();
    if (q.first())
    {
      QMessageBox::critical(this, tr("Invalid Sales Order"),
          tr("This sales order is already linked to open return authorization %1.").arg(q.value("rahead_number").toString())  );
      _origso->setId(-1);
      if (_cust->isValid())
        _origso->setCustId(_cust->id());
      else
        _origso->setCustId(-1);
      return;
    }
  }
  if (!_ignoreSoSignals) 
  {
    if (_origso->isValid())
    {
      if ( !_warehouse->isValid() )
      {
        QMessageBox::warning( this, tr("Invalid Receiving Site"),
                             tr("<p>You must enter a valid Receiving Site." ) );
        _warehouse->setFocus();
        _origso->setId(-1);
        return;
      }

      if ( !_shipWhs->isValid() )
      {
        QMessageBox::warning( this, tr("Invalid Shipping Site"),
                             tr("<p>You must enter a valid Shipping Site." ) );
        _shipWhs->setFocus();
        _origso->setId(-1);
        return;
      }

      XSqlQuery sohead;
      sohead.prepare( "SELECT cohead.*,custinfo.*, custtype_code, "
                      "       cohead_commission, "
                      "       shipto_num "
                      "FROM cohead "
                      "  LEFT OUTER JOIN shiptoinfo ON (cohead_shipto_id=shipto_id), "
                      "  custinfo, custtype "
                      "WHERE ((cohead_id=:cohead_id) "
                      "AND (cohead_cust_id=cust_id) "
                      "AND (cust_custtype_id=custtype_id) ) "
                      "LIMIT 1;" );
      sohead.bindValue(":cohead_id", _origso->id());
      sohead.exec();
      if (sohead.first())
      {
        _salesRep->setId(sohead.value("cohead_salesrep_id").toInt());
        _commission->setDouble(sohead.value("cohead_commission").toDouble() * 100);

        _taxzone->setId(sohead.value("cohead_taxzone_id").toInt());
        _customerPO->setText(sohead.value("cohead_custponumber"));

        _project->setId(sohead.value("cohead_prj_id").toInt());

        _cust->setEnabled(FALSE);

        _cust->setId(sohead.value("cohead_cust_id").toInt());
        _custType->setText(sohead.value("custtype_code").toString());
        _billToName->setText(sohead.value("cohead_billtoname"));
        _billToAddr->setLine1(sohead.value("cohead_billtoaddress1").toString());
        _billToAddr->setLine2(sohead.value("cohead_billtoaddress2").toString());
        _billToAddr->setLine3(sohead.value("cohead_billtoaddress3").toString());
        _billToAddr->setCity(sohead.value("cohead_billtocity").toString());
        _billToAddr->setState(sohead.value("cohead_billtostate").toString());
        _billToAddr->setPostalCode(sohead.value("cohead_billtozipcode").toString());
        _billToAddr->setCountry(sohead.value("cohead_billtocountry").toString());
        if ( (_mode == cNew) || (_mode == cEdit) )
          _ffBillto = sohead.value("cust_ffbillto").toBool();
        else
          _ffBillto = FALSE;
        _billToName->setEnabled(_ffBillto);
        _billToAddr->setEnabled(_ffBillto);

        _ignoreShiptoSignals = true;
        _shipTo->setId(sohead.value("cohead_shipto_id").toInt());
        _shipToName->setText(sohead.value("cohead_shiptoname"));
        _shipToAddr->setLine1(sohead.value("cohead_shiptoaddress1").toString());
        _shipToAddr->setLine2(sohead.value("cohead_shiptoaddress2").toString());
        _shipToAddr->setLine3(sohead.value("cohead_shiptoaddress3").toString());
        _shipToAddr->setCity(sohead.value("cohead_shiptocity").toString());
        _shipToAddr->setState(sohead.value("cohead_shiptostate").toString());
        _shipToAddr->setPostalCode(sohead.value("cohead_shiptozipcode").toString());
        _shipToAddr->setCountry(sohead.value("cohead_shiptocountry").toString());
        if ( (_mode == cNew) || (_mode == cEdit) )
          _ffShipto = sohead.value("cust_ffshipto").toBool();
        else
          _ffShipto = FALSE;
        _copyToShipto->setEnabled(_ffShipto);
        _shipToName->setEnabled(_ffShipto);
        _shipTo->setEnabled(_ffShipto);
        _shipToAddr->setEnabled(_ffShipto);
        _ignoreShiptoSignals = FALSE;

        sSave(true);
        sFillList();
      }
      else if (sohead.lastError().type() != QSqlError::NoError)
      {
        systemError(this, sohead.lastError().databaseText(), __FILE__, __LINE__);
        _origso->setId(-1);
        return;
      }
    }
    sRecalcFreight();
  }
}

void returnAuthorization::populateShipto(int pShiptoid)
{
  if (pShiptoid != -1)
  {
    XSqlQuery query;
    query.prepare( "SELECT shipto_id, shipto_num, shipto_name,"
                   " shipto_addr_id, shipto_taxzone_id, "
                   " shipto_salesrep_id, shipto_commission "
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
      _taxzone->setId(query.value("shipto_taxzone_id").toInt());
      _salesRep->setId(query.value("shipto_salesrep_id").toInt());
      _commission->setDouble(query.value("shipto_commission").toDouble() * 100);
      _ignoreShiptoSignals = false;
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

void returnAuthorization::sPopulateCustomerInfo()
{
    if (_cust->isValid())
    {
      _origso->setCustId(_cust->id());
      
      XSqlQuery query;
      query.prepare( "SELECT custtype_code, cust_salesrep_id,"
                     "       cust_commprcnt,"
                     "       cust_taxzone_id, cust_curr_id, "
                     "       cust_name, cntct_addr_id, "
                     "       cust_ffshipto, cust_ffbillto, "
                     "       COALESCE(shipto_id, -1) AS shiptoid, "
                     "       cust_soemaildelivery "
                     "FROM custinfo "
                     "  LEFT OUTER JOIN shiptoinfo ON ((cust_id=shipto_cust_id) "
                     "                             AND (shipto_default)) "
                     "  LEFT OUTER JOIN cntct ON (cust_cntct_id=cntct_id), "
                     "  custtype "
                     "WHERE ( (cust_id=:cust_id) "
                     "AND (custtype_id=cust_custtype_id) );" );
      query.bindValue(":cust_id", _cust->id());
      query.exec();
      if (query.first())
      {
        _custType->setText(query.value("custtype_code").toString());
        _salesRep->setId(query.value("cust_salesrep_id").toInt());
        _commission->setDouble(query.value("cust_commprcnt").toDouble() * 100);

        _custtaxzoneid = query.value("cust_taxzone_id").toInt();
        _taxzone->setId(query.value("cust_taxzone_id").toInt());
        _currency->setId(query.value("cust_curr_id").toInt());

        _billToName->setText(query.value("cust_name"));
        _billToAddr->setId(query.value("cntct_addr_id").toInt());

        if ( (_mode == cNew) || (_mode == cEdit) )
          _ffBillto = query.value("cust_ffbillto").toBool();
        else
          _ffBillto = FALSE;
        _billToName->setEnabled(_ffBillto);
        _billToAddr->setEnabled(_ffBillto);

        if ( (_mode == cNew) || (_mode == cEdit) )
          _ffShipto = query.value("cust_ffshipto").toBool();
        else
          _ffShipto = FALSE;
        _copyToShipto->setEnabled(_ffShipto);
        _shipToName->setEnabled(_ffShipto);
        _shipTo->setEnabled(_ffShipto);
        _shipToAddr->setEnabled(_ffShipto);
        _custEmail = query.value("cust_soemaildelivery").toBool();
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
      _origso->setCustId(-1);
      _salesRep->setCurrentIndex(-1);
      _taxzone->setId(-1);
      _custtaxzoneid = -1;
      _billToName->setEnabled(TRUE);
      _billToAddr->setEnabled(TRUE);
      _billToName->clear();
      _billToAddr->clear();
      _shipTo->setEnabled(TRUE);
      _shipToName->setEnabled(TRUE);
      _shipToAddr->setEnabled(TRUE);
      _shipTo->setId(-1);
      _shipToName->clear();
      _shipToAddr->clear();
    }
}

void returnAuthorization::sCheckAuthorizationNumber()
{
  if ( (_authNumber->text().length()) &&
       ( (_metrics->value("RANumberGeneration") == "O") || 
         (_metrics->value("RANumberGeneration") == "M")   ) )
  {
    _authNumber->setEnabled(FALSE);

    XSqlQuery query;
    query.prepare( "SELECT rahead_id "
                   "FROM rahead "
                   "WHERE (rahead_number=:rahead_number)" );
    query.bindValue(":rahead_number", _authNumber->text());
    query.exec();
    if (query.first())
    {
      _raheadid = query.value("rahead_id").toInt();

      if (_cust->id() > 0)
        _cust->setReadOnly(TRUE);

      populate();

      _mode = cEdit;
    }
    else if (query.lastError().type() != QSqlError::NoError)
    {
      systemError(this, query.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }
}

void returnAuthorization::sClearShiptoNumber()
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

void returnAuthorization::sCopyToShipto()
{
  _shipTo->setId(-1);
  _shipToName->setText(_billToName->text());
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

  _taxzone->setId(_custtaxzoneid);
}

void returnAuthorization::sNew()
{
  if (sSave(true))
  {
    ParameterList params;
    params.append("mode", "new");
    params.append("rahead_id", _raheadid);
    params.append("orig_cohead_id", _origso->id());
    if (_warehouse->isValid())
      params.append("warehous_id", _warehouse->id());
    if (_shipWhs->isValid())
      params.append("shipwarehous_id", _shipWhs->id());

    returnAuthorizationItem newdlg(this, "", TRUE);
    newdlg.set(params);
  
    if (newdlg.exec() != XDialog::Rejected)
    {
      populate();
      sRecalcFreight();
    }
  }
}

void returnAuthorization::sEdit()
{
  if (sSave(true))
  {
    bool fill;
    fill = FALSE;
    QList<XTreeWidgetItem*> selected = _raitem->selectedItems();
    for (int i = 0; i < selected.size(); i++)
    {
      ParameterList params;
      params.append("raitem_id", ((XTreeWidgetItem*)(selected[i]))->id());
      params.append("rahead_id", _raheadid);
      params.append("orig_cohead_id", _origso->id());

      if (_mode==cView || ((XTreeWidgetItem*)(selected[i]))->altId() == -1)
        params.append("mode", "view");
      else if (((XTreeWidgetItem*)(selected[i]))->rawValue("item_type") == "K")
        params.append("mode", "view");
      else
        params.append("mode", "edit");

      returnAuthorizationItem newdlg(this, "", TRUE);
      newdlg.set(params);
  
      if (newdlg.exec() != XDialog::Rejected)
        fill = TRUE;
    }
    if (fill)
    {
      populate();
      sRecalcFreight();
    }
  }
}

void returnAuthorization::sView()
{
  QList<XTreeWidgetItem*> selected = _raitem->selectedItems();
  for (int i = 0; i < selected.size(); i++)
  {
    ParameterList params;
    params.append("raitem_id", ((XTreeWidgetItem*)(selected[i]))->id());
    params.append("rahead_id", _raheadid);
    params.append("orig_cohead_id", _origso->id());
    params.append("mode", "view");

    returnAuthorizationItem newdlg(this, "", TRUE);
    newdlg.set(params);
  
    newdlg.exec();
  }
}

void returnAuthorization::sDelete()
{
  if (QMessageBox::question(this, "Delete current Line Item?",
                            tr("<p>Are you sure that you want to delete the "
                               "the selected Line Item(s)?"),
                            QMessageBox::Yes | QMessageBox::Default,
                            QMessageBox::No | QMessageBox::Escape) == QMessageBox::Yes)
  {
    QList<XTreeWidgetItem*> selected = _raitem->selectedItems();
    for (int i = 0; i < selected.size(); i++)
    {
      q.prepare( "DELETE FROM raitem "
                 "WHERE (raitem_id=:raitem_id);" );
      q.bindValue(":raitem_id", ((XTreeWidgetItem*)(selected[i]))->id());
      q.exec();
      if (q.lastError().type() != QSqlError::NoError)
      {
        systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
        return;
      }
    }
    populate();
    sRecalcFreight();
  }
}

void returnAuthorization::sFillList()
{
  q.prepare( "SELECT raitem_id, "
             "       CASE WHEN (raitem_status='C')     THEN -1"
             "            WHEN (raitem_disposition='C') THEN 0"
             "            WHEN (raitem_disposition='R') THEN 1"
             "            WHEN (raitem_disposition='P') THEN 2"
             "            WHEN (raitem_disposition='V') THEN 3"
             "            WHEN (raitem_disposition='S') THEN 4"
             "            ELSE -1 END AS disposition_code,"
             "       formatRaLineNumber(raitem_id) AS f_linenumber, raitem_subnumber,"
             "       item_number, uom_name, item_type,"
             "       (item_descrip1 || ' ' || item_descrip2) AS item_descrip, warehous_code,"
             "       raitem_status, "
             "       CASE WHEN (raitem_disposition='C') THEN :credit "
             "            WHEN (raitem_disposition='R') THEN :return "
             "            WHEN (raitem_disposition='P') THEN :replace "
             "            WHEN (raitem_disposition='V') THEN :service "
             "            WHEN (raitem_disposition='S') THEN :ship "
             "            ELSE 'Error' END AS disposition, "
             "       raitem_warranty,"
             "       COALESCE(oc.coitem_qtyshipped,0) AS oldcoitem_qtyshipped,"
             "       raitem_qtyauthorized,"
             "       raitem_qtyreceived,"
             "       qtyToReceive('RA', raitem_id) AS raitem_qtytoreceive,"
             "       COALESCE(nc.coitem_qtyshipped,0) AS newcoitem_qtyshipped,"
             "       raitem_unitprice,"
             "       round((raitem_qtyauthorized * raitem_qty_invuomratio) * (raitem_unitprice / raitem_price_invuomratio),2) AS raitem_extprice,"
             "       raitem_amtcredited,"
             "       raitem_saleprice,"
             "       round((raitem_qtyauthorized * raitem_qty_invuomratio) * (raitem_saleprice / raitem_price_invuomratio),2) AS raitem_extsaleprice,"
             "       round((raitem_qtyauthorized * raitem_qty_invuomratio) * (raitem_unitprice / raitem_price_invuomratio),2) - "
             "       round((raitem_qtyauthorized * raitem_qty_invuomratio) * (raitem_saleprice / raitem_price_invuomratio),2) AS raitem_netdue,"
             "       och.cohead_number::text || '-' || oc.coitem_linenumber::text AS oldcohead_number, "
             "       COALESCE(och.cohead_id,-1) AS oldcohead_number_xtidrole, "
             "       nch.cohead_number::text || '-' || nc.coitem_linenumber::text AS newcohead_number, "
             "       COALESCE(nch.cohead_id,-1) AS newcohead_number_xtidrole, "
             "       raitem_scheddate, "
             "       'qty' AS oldcoitem_qtyshipped_xtnumericrole, "
             "       'qty' AS raitem_qtyauthorized_xtnumericrole, "
             "       'qty' AS raitem_qtyreceived_xtnumericrole, "
             "       'qty' AS raitem_qtytoreceive_xtnumericrole, "
             "       'qty' AS newcoitem_qtyshipped_xtnumericrole, "
             "       'salesprice' AS raitem_unitprice_xtnumericrole, "
             "       'extprice' AS raitem_extprice_xtnumericrole, "
             "       'curr' AS raitem_amtcredited_xtnumericrole, "
             "       'salesprice' AS raitem_saleprice_xtnumericrole, "
             "       'extprice' AS raitem_extsaleprice_xtnumericrole, "
             "       'extprice' AS raitem_netdue_xtnumericrole, "
             "        :na AS raitem_scheddate_xtnullrole,"
             "       CASE WHEN raitem_subnumber = 0 THEN 0"
             "            ELSE 1 END AS xtindentrole "
             "FROM raitem "
             " LEFT OUTER JOIN coitem oc ON (raitem_orig_coitem_id=oc.coitem_id) "
             " LEFT OUTER JOIN cohead och ON (och.cohead_id=oc.coitem_cohead_id) "
             " LEFT OUTER JOIN coitem nc ON (raitem_new_coitem_id=nc.coitem_id) "
             " LEFT OUTER JOIN cohead nch ON (nch.cohead_id=nc.coitem_cohead_id),"
             " itemsite, item, whsinfo, uom "
             "WHERE ( (raitem_itemsite_id=itemsite_id)"
             " AND (itemsite_item_id=item_id)"
             " AND (itemsite_warehous_id=warehous_id)"
             " AND (raitem_rahead_id=:rahead_id) "
             " AND (raitem_qty_uom_id=uom_id) ) "
             "ORDER BY raitem_linenumber, raitem_subnumber;" );
  q.bindValue(":rahead_id", _raheadid);
  q.bindValue(":credit", tr("Credit"));
  q.bindValue(":return", tr("Return"));
  q.bindValue(":replace", tr("Replace"));
  q.bindValue(":service", tr("Service"));
  q.bindValue(":ship", tr("Ship"));
  q.bindValue(":na", tr("N/A"));
  q.exec();
  _raitem->populate(q, true);
  if ((q.first()) && (_mode == cEdit))
  {
    _cust->setDisabled(_cust->isValid());
  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
  
  sCalculateSubtotal();
  sCalculateNetDue();
  sCalculateTax();

  _currency->setEnabled((_raitem->topLevelItemCount() == 0) && (_mode == cEdit));

  //Disable order changes if any order qty authorized
  q.prepare("SELECT raitem_id "
            "FROM raitem "
            "WHERE ( (raitem_rahead_id=:rahead_id) "
            "AND (raitem_orig_coitem_id IS NOT NULL) "
            "AND (raitem_qtyauthorized > 0) );");
  q.bindValue(":rahead_id", _raheadid);
  q.exec();
  _origso->setEnabled((!q.first()) && (_mode == cEdit || _mode == cNew));

  //Disable changes if any transactions
  q.prepare("SELECT raitem_id "
            "FROM raitem "
            "  LEFT OUTER JOIN coitem ON (raitem_new_coitem_id=coitem_id) "
            "WHERE ( (raitem_rahead_id=:rahead_id) "
            "AND (((raitem_qtyreceived + COALESCE(coitem_qtyshipped,0) + "
            " raitem_amtcredited) > 0 OR raitem_status = 'C')) );");
  q.bindValue(":rahead_id", _raheadid);
  q.exec();
  if (q.first())
  {
    _salesRep->setEnabled(false);
    _commission->setEnabled(false);
    _taxzone->setEnabled(false);
    _disposition->setEnabled(false);
    _timing->setEnabled(false);
    _cust->setEnabled(false);
    _billToName->setEnabled(false);
    _billToAddr->setEnabled(false);
  }
  else
  {
    _salesRep->setEnabled(true);
    _commission->setEnabled(true);
    _taxzone->setEnabled(true);
    _disposition->setEnabled(true);
    _timing->setEnabled(true);
    _cust->setEnabled(!_origso->isValid());
    _billToName->setEnabled(true);
    _billToAddr->setEnabled(true);
  }
  _comments->refresh();
  sCreditByChanged();
}

void returnAuthorization::sCalculateSubtotal()
{
//  Determine the subtotal and line item tax
  XSqlQuery query;
  query.prepare( "SELECT SUM(round((raitem_qtyauthorized * raitem_qty_invuomratio) * (raitem_unitprice / raitem_price_invuomratio),2)) AS subtotal,"
               "       SUM(round((raitem_qtyauthorized * raitem_qty_invuomratio) * stdCost(item_id),2)) AS totalcost "
               "FROM raitem, itemsite, item "
               "WHERE ( (raitem_rahead_id=:rahead_id)"
               " AND (raitem_itemsite_id=itemsite_id)"
               " AND (itemsite_item_id=item_id) );" );
  query.bindValue(":rahead_id", _raheadid);
  query.exec();
  if (query.first())
    _subtotal->setLocalValue(query.value("subtotal").toDouble());
}

void returnAuthorization::sCalculateNetDue()
{
  XSqlQuery query;
  query.prepare( "SELECT SUM(round( ((raitem_qtyauthorized * raitem_qty_invuomratio) * (raitem_unitprice / raitem_price_invuomratio)) - "
               "                    ((raitem_qtyauthorized * raitem_qty_invuomratio) * (raitem_saleprice / raitem_price_invuomratio)),2)) AS netdue "
               "FROM raitem, itemsite, item "
               "WHERE ( (raitem_rahead_id=:rahead_id)"
               " AND (raitem_itemsite_id=itemsite_id)"
               " AND (itemsite_item_id=item_id) );" );
  query.bindValue(":rahead_id", _raheadid);
  query.exec();
  if (query.first())
    _netdue->setLocalValue(query.value("netdue").toDouble());
}

void returnAuthorization::sCalculateTotal()
{
  _total->setLocalValue(_subtotal->localValue() +
                        _tax->localValue() +
                        _freight->localValue() +
                        _miscCharge->localValue() );
}

void returnAuthorization::populate()
{
  XSqlQuery rahead;
  rahead.prepare( "SELECT rahead.*, custtype_code, "
                  "       cust_name, cust_ffbillto, cust_ffshipto, shipto_num, "
                  "       rahead_commission "
                  "FROM rahead "
                  "  LEFT OUTER JOIN shiptoinfo ON (rahead_shipto_id=shipto_id) "
                  "  LEFT OUTER JOIN custinfo ON (rahead_cust_id=cust_id) "
                  "  LEFT OUTER JOIN custtype ON (cust_custtype_id=custtype_id) "
                  "WHERE (rahead_id=:rahead_id);" );
  rahead.bindValue(":rahead_id", _raheadid);
  rahead.exec();
  if (rahead.first())
  {
    _authNumber->setText(rahead.value("rahead_number"));
    _authDate->setDate(rahead.value("rahead_authdate").toDate(), true);
    _expireDate->setDate(rahead.value("rahead_expiredate").toDate());

    _salesRep->setId(rahead.value("rahead_salesrep_id").toInt());
    _commission->setDouble(rahead.value("rahead_commission").toDouble() * 100);
    // do taxzone first so we can overwrite the result of the signal cascade
    _taxzone->setId(rahead.value("rahead_taxzone_id").toInt());
    if (!rahead.value("rahead_rsncode_id").isNull() && rahead.value("rahead_rsncode_id").toInt() != -1)
      _rsnCode->setId(rahead.value("rahead_rsncode_id").toInt());

    if (rahead.value("rahead_timing").toString() == "I")
      _timing->setCurrentIndex(0);
    else
      _timing->setCurrentIndex(1);

    if (rahead.value("rahead_creditmethod").toString() == "N")
      _creditBy->setCurrentIndex(0);
    else if (rahead.value("rahead_creditmethod").toString() == "M")
      _creditBy->setCurrentIndex(1);
    else if (rahead.value("rahead_creditmethod").toString() == "K")
      _creditBy->setCurrentIndex(2);
    else if (rahead.value("rahead_creditmethod").toString() == "C")
      _creditBy->setCurrentIndex(3);

    _cust->setId(rahead.value("rahead_cust_id").toInt());
    _custType->setText(rahead.value("custtype_code").toString());
    _ignoreSoSignals = TRUE;
    _origso->setId(rahead.value("rahead_orig_cohead_id").toInt());
    _newso->setId(rahead.value("rahead_new_cohead_id").toInt(),"SO");
    _ignoreSoSignals = FALSE;
    _incident->setId(rahead.value("rahead_incdt_id").toInt());
    _project->setId(rahead.value("rahead_prj_id").toInt());

    _ffBillto = rahead.value("cust_ffbillto").toBool();
    _billToName->setEnabled(_ffBillto);
    _billToAddr->setEnabled(_ffBillto);

    _billToName->setText(rahead.value("rahead_billtoname"));
    _billToAddr->setLine1(rahead.value("rahead_billtoaddress1").toString());
    _billToAddr->setLine2(rahead.value("rahead_billtoaddress2").toString());
    _billToAddr->setLine3(rahead.value("rahead_billtoaddress3").toString());
    _billToAddr->setCity(rahead.value("rahead_billtocity").toString());
    _billToAddr->setState(rahead.value("rahead_billtostate").toString());
    _billToAddr->setPostalCode(rahead.value("rahead_billtozip").toString());
    _billToAddr->setCountry(rahead.value("rahead_billtocountry").toString());

    _ffShipto = rahead.value("cust_ffshipto").toBool();
    _shipToName->setEnabled(_ffShipto);
    _shipToAddr->setEnabled(_ffShipto);
    _copyToShipto->setEnabled(_ffShipto);

    _ignoreShiptoSignals = true;
    _shipTo->setId(rahead.value("rahead_shipto_id").toInt());
    _shipToName->setText(rahead.value("rahead_shipto_name"));
    _shipToAddr->setLine1(rahead.value("rahead_shipto_address1").toString());
    _shipToAddr->setLine2(rahead.value("rahead_shipto_address2").toString());
    _shipToAddr->setLine3(rahead.value("rahead_shipto_address3").toString());
    _shipToAddr->setCity(rahead.value("rahead_shipto_city").toString());
    _shipToAddr->setState(rahead.value("rahead_shipto_state").toString());
    _shipToAddr->setPostalCode(rahead.value("rahead_shipto_zipcode").toString());
    _shipToAddr->setCountry(rahead.value("rahead_shipto_country").toString());
    _ignoreShiptoSignals = false;

    _customerPO->setText(rahead.value("rahead_custponumber"));

    _currency->setId(rahead.value("rahead_curr_id").toInt());
    _freightCache = rahead.value("rahead_freight").toDouble();
    _freight->setLocalValue(rahead.value("rahead_freight").toDouble());

    _ignoreWhsSignals = TRUE;
    _warehouse->setId(rahead.value("rahead_warehous_id").toInt());
    _shipWhs->setId(rahead.value("rahead_cohead_warehous_id").toInt());
    _ignoreWhsSignals = FALSE;

    _miscCharge->setLocalValue(rahead.value("rahead_misc").toDouble());
    _miscChargeDescription->setText(rahead.value("rahead_misc_descrip"));
    _miscChargeAccount->setId(rahead.value("rahead_misc_accnt_id").toInt());

    _notes->setText(rahead.value("rahead_notes").toString());

    if (rahead.value("rahead_headcredited").toBool())
    {
      _freight->setEnabled(FALSE);
      _miscCharge->setEnabled(FALSE);
      _miscChargeDescription->setEnabled(FALSE);
      _miscChargeAccount->setEnabled(FALSE);
    }

    _ignoreSoSignals = TRUE;
    if (rahead.value("rahead_disposition").toString() == "C")
      sDispositionChanged();
    else if (rahead.value("rahead_disposition").toString() == "R")
      _disposition->setCurrentIndex(1);
    else if (rahead.value("rahead_disposition").toString() == "P")
      _disposition->setCurrentIndex(2);
    else if (rahead.value("rahead_disposition").toString() == "V")
      _disposition->setCurrentIndex(3);
    else if (rahead.value("rahead_disposition").toString() == "M")
      _disposition->setCurrentIndex(4);
    _ignoreSoSignals = FALSE;
    _saved = TRUE;
    sFillList();
  }
  else if (rahead.lastError().type() != QSqlError::NoError)
  {
    systemError(this, rahead.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void returnAuthorization::closeEvent(QCloseEvent *pEvent)
{
  if ( (_mode == cNew) && (_raheadid != -1) )
  {
    q.prepare( "UPDATE raitem SET raitem_qtyauthorized=0 "
               "WHERE (raitem_rahead_id=:rahead_id); "
               "SELECT importcoitemstora(:rahead_id,NULL);"
               "DELETE FROM rahead "
               "WHERE (rahead_id=:rahead_id);" );
    q.bindValue(":rahead_id", _raheadid);
    q.exec();
    if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
    else
    {
      omfgThis->sReturnAuthorizationsUpdated();
      omfgThis->sProjectsUpdated(_project->id());
    }

    if ( (_metrics->value("RANumberGeneration") == "A") || 
         (_metrics->value("RANumberGeneration") == "O")   )
    {
      q.prepare("SELECT releaseRaNumber(:number) AS result;");
      q.bindValue(":number", _authNumber->text());
      q.exec();
      if (q.lastError().type() != QSqlError::NoError)
      {
        systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
        return;
      }
    }
  }

  XWidget::closeEvent(pEvent);
}

void returnAuthorization::sTaxDetail()
{
  if (!sSave(true))
    return;

  ParameterList params;
  params.append("order_id", _raheadid);
  params.append("order_type", "RA");
  // mode => view since there are no fields to hold modified tax data
  if (_mode == cView)
    params.append("mode", "view");

  taxBreakdown newdlg(this, "", TRUE);
  if (newdlg.set(params) == NoError && newdlg.exec() == XDialog::Accepted)
    populate();

}

void returnAuthorization::sCalculateTax()
{  
  XSqlQuery taxq;
  taxq.prepare( "SELECT calcRATaxAmt(:rahead_id) AS tax;" );

  taxq.bindValue(":rahead_id", _raheadid);
  taxq.exec();
  if (taxq.first())
    _tax->setLocalValue(taxq.value("tax").toDouble());
  else if (taxq.lastError().type() != QSqlError::NoError)
  {
    systemError(this, taxq.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }              
}

void returnAuthorization::sTaxZoneChanged()
{
  if (_saved)
    sSave(true);
  sCalculateTax();
}

void returnAuthorization::sRecvWhsChanged()
{
  if (!_ignoreWhsSignals)
  { 
    if ( (_raheadid == -1) || (!_warehouse->isValid()) )
      return;

    XSqlQuery whsq;
    whsq.prepare("SELECT raitem_id "
                 "FROM raitem JOIN itemsite ON (itemsite_id=raitem_itemsite_id) "
                 "WHERE ( (raitem_rahead_id=:rahead_id)"
                 "  AND   (itemsite_warehous_id <> :warehous_id) )"
                 "LIMIT 1;");
    whsq.bindValue(":rahead_id", _raheadid);
    whsq.bindValue(":warehous_id", _warehouse->id());
    whsq.exec();
    if (whsq.first())
      QMessageBox::information(this, tr("Receiving Site Warning"),
                                     tr("This Return Authorization has line items with a different Receiving Site. "
                                        "You may need to review the line items."));
  }
}

void returnAuthorization::sShipWhsChanged()
{
  if (!_ignoreWhsSignals)
  { 
    if ( (_raheadid == -1) || (!_shipWhs->isValid()) )
      return;

    XSqlQuery whsq;
    whsq.prepare("SELECT raitem_id "
                 "FROM raitem JOIN itemsite ON (itemsite_id=raitem_coitem_itemsite_id) "
                 "WHERE ( (raitem_rahead_id=:rahead_id)"
                 "  AND   (itemsite_warehous_id <> :warehous_id) )"
                 "LIMIT 1;");
    whsq.bindValue(":rahead_id", _raheadid);
    whsq.bindValue(":warehous_id", _shipWhs->id());
    whsq.exec();
    if (whsq.first())
      QMessageBox::information(this, tr("Shipping Site Warning"),
                                     tr("This Return Authorization has line items with a different Shipping Site. "
                                        "You may need to review the line items."));
  }
}

void returnAuthorization::sDispositionChanged()
{
  _new->setEnabled(_cust->isValid() || 
              (_disposition->currentIndex() == 1 && _creditBy->currentIndex() == 0));
  
  bool enableReceipt = _privileges->check("EnterReceipts") &&
                      (_disposition->currentIndex() != 0);

  _receiveAll->setEnabled(enableReceipt);
  _postReceipts->setEnabled(enableReceipt);

  if (_disposition->currentIndex() == 0)
  {
    _timing->setCurrentIndex(0);
    _timing->setEnabled(FALSE);
    if (_creditBy->currentIndex() == 0)
      _creditBy->setCurrentIndex(1);
  }
  else
    _timing->setEnabled(TRUE);

  _refund->setEnabled(_creditBy->currentIndex() == 3);

  if (!_ignoreSoSignals) 
  {
// Save the change so that disposition of raitems is changed
    sSave(true);
    sFillList();
    sRecalcFreight();
  }
}

void returnAuthorization::sCreditByChanged()
{
  if (_creditBy->currentIndex() == 0 && _total->localValue() > 0)
  {
    QMessageBox::information(this, tr("Credit By 'None' not allowed"),
                          tr("<p>This Return Authorization has authorized "
                             "credit amounts. You may not set the Credit By "
                             "to 'None' unless all credit amounts are zero."));
    _creditBy->setCurrentIndex(1);
  }
  else if (_creditBy->currentIndex() == 0  || _total->localValue() == 0)
  {
    _currency->setEnabled(false);
    _miscChargeDescription->setEnabled(false);
    _miscChargeAccount->setEnabled(false);
    _miscCharge->setEnabled(false);
    _freight->setEnabled(false);
  }
  else
  {
    _currency->setEnabled(true);
    _miscChargeDescription->setEnabled(true);
    _miscChargeAccount->setEnabled(true);
    _miscCharge->setEnabled(true);
    _freight->setEnabled(true);
  }
}

void returnAuthorization::sAuthorizeLine()
{
  QList<XTreeWidgetItem*> selected = _raitem->selectedItems();
  for (int i = 0; i < selected.size(); i++)
  {
    QString sql ( "SELECT authReturnItem(:raitem_id) AS result;" );
    q.prepare(sql);
    q.bindValue(":raitem_id",  ((XTreeWidgetItem*)(selected[i]))->id());
    q.exec();
    if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
    sRecalcFreight();
  }

  if (_newso->isValid())
    omfgThis->sSalesOrdersUpdated(_newso->id());
    
  populate();
}

void returnAuthorization::sClearAuthorization()
{
  QList<XTreeWidgetItem*> selected = _raitem->selectedItems();
  for (int i = 0; i < selected.size(); i++)
  {
        q.prepare ( "SELECT clearReturnItem(:raitem_id) AS result;" );
        q.bindValue(":raitem_id",  ((XTreeWidgetItem*)(selected[i]))->id());
        q.exec();
        if (q.lastError().type() != QSqlError::NoError)
        {
           systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
           return;
        }
  }

  if (_newso->isValid())
    omfgThis->sSalesOrdersUpdated(_newso->id());

  populate();
  sRecalcFreight();
}

void returnAuthorization::sAuthorizeAll()
{
  QString sql ( "SELECT authReturnItem(raitem_id) AS result "
                "FROM raitem "
                "WHERE (raitem_rahead_id=:raitem_rahead_id);" );
  q.prepare(sql);
  q.bindValue(":raitem_rahead_id",  _raheadid);
  q.exec();
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  if (_newso->isValid())
    omfgThis->sSalesOrdersUpdated(_newso->id());

  populate();
  sRecalcFreight();
}

void returnAuthorization::sEnterReceipt()
{
  ParameterList params;
  q.prepare("SELECT * "
            "FROM recv "
            "WHERE ((recv_orderitem_id=:id)"
            "  AND  (recv_order_type = 'RA')"
            "  AND  (NOT recv_posted));");
  q.bindValue(":id", _raitem->id());
  q.exec();
  if (q.first())
  {
    params.append("recv_id", q.value("recv_id"));
    params.append("mode", "edit");
  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
  else
  {
    params.append("lineitem_id", _raitem->id());
    params.append("order_type",  "RA");
    params.append("mode", "new");
  }

  enterPoitemReceipt newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void returnAuthorization::sReceiveAll()
{
  ParameterList params;
  params.append("orderid",   _raheadid);
  params.append("ordertype", "RA");
  params.append("nonInventory", tr("Non-Inventory"));
  params.append("na",           tr("N/A"));
  params.append("EnableReturnAuth", TRUE);
  if (_metrics->boolean("MultiWhs"))
    params.append("MultiWhs");
  MetaSQLQuery recvm = mqlLoad("receipt", "receiveAll");
  q = recvm.toQuery(params);

  while (q.next())
  {
    int result = q.value("result").toInt();
    if (result < 0)
    {
      systemError(this, storedProcErrorLookup("enterReceipt", result),
                  __FILE__, __LINE__);
      return;
    }
    omfgThis->sPurchaseOrderReceiptsUpdated();
  }
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  sFillList();
}

void returnAuthorization::sHandleEnterReceipt(bool p)
{
  if (p)
    _enterReceipt->setEnabled(_raitem->altId() == 1 ||
                              _raitem->altId() == 2 ||
                              _raitem->altId() == 3);
  else
    _enterReceipt->setEnabled(false);
}

void returnAuthorization::sHandleAction()
{
  QList<XTreeWidgetItem*> selected = _raitem->selectedItems();
  if (selected.size() > 1)
  {
    _action->setText("Close");
    _action->setEnabled(FALSE);
  }
  else if (_raitem->altId() == -1)
  {
    _action->setText("Open");
    _edit->setEnabled(FALSE);
  }
  else
  {
    _action->setText("Close");
    _edit->setEnabled(TRUE);
  }
}

void returnAuthorization::sAction()
{
  q.prepare("UPDATE raitem SET "
            "raitem_status = :status "
            "WHERE (raitem_id=:raitem_id); ");
  q.bindValue(":raitem_id", _raitem->id());
  if (_raitem->altId() == -1)
    q.bindValue(":status",QString("O"));
  else
    q.bindValue(":status",QString("C"));
  q.exec();
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
  else
    sFillList();
}

void returnAuthorization::sHandleSalesOrderEvent(int pSoheadid, bool)
{
  if ((pSoheadid == _origso->id()) || (pSoheadid == _newso->id()))
  {
    sFillList();
    sRecalcFreight();
  }
}

void returnAuthorization::sRefund()
{
  if (! sSave(true))
    return;

  q.exec("BEGIN;");

  bool _post = _disposition->currentIndex() == 0 &&
               _timing->currentIndex() == 0 &&
               _creditBy->currentIndex() == 3;

  q.prepare("SELECT createRaCreditMemo(:rahead_id,:post) AS result;");
  q.bindValue(":rahead_id", _raheadid);
  q.bindValue(":post",      QVariant(_post));
  q.exec();
  if (q.first())
  {
    int result = q.value("result").toInt();
    if (result < 0)
    {
      systemError(this, storedProcErrorLookup("createRaCreditMemo", result), __FILE__, __LINE__);
      q.exec("ROLLBACK;");
      return;
    }

    ParameterList ccp;
    ccp.append("cmhead_id", result);
    MetaSQLQuery ccm = mqlLoad("creditMemoCreditCards", "detail");
    XSqlQuery ccq = ccm.toQuery(ccp);
    if (ccq.first())
    {
      int ccpayid = ccq.value("ccpay_id").toInt();
      QMessageBox::information( this, tr("New Credit Memo Created"),
                                tr("<p>A new CreditMemo has been created and "
                                   "assigned #%1")
                                   .arg(ccq.value("cmhead_number").toString()));
      CreditCardProcessor *cardproc = CreditCardProcessor::getProcessor();
      if (! cardproc)
        QMessageBox::critical(this, tr("Credit Card Processing Error"),
                              CreditCardProcessor::errorMsg());
      else
      {
        QString docnum = ccq.value("cmhead_number").toString();
        QString refnum = ccq.value("cohead_number").toString();
        QString reftype;
        int refid = -1; // TODO: set this to some appropriate value
        if(_post)
        {
          reftype = "aropen";
          q.prepare("SELECT aropen_id FROM aropen WHERE aropen_doctype='C' AND aropen_docnumber=:cmhead_number;");
          q.bindValue(":cmhead_number", docnum);
          q.exec();
          if(q.first())
            refid = q.value("aropen_id").toInt();
          else
          {
            systemError(this, ccq.lastError().databaseText(), __FILE__, __LINE__);
            q.exec("ROLLBACK;");
            return;
          }
        }
        int returnValue = cardproc->credit(ccq.value("ccard_id").toInt(),
                                       (_CCCVV->text().isEmpty()) ? -1 :
                                                        _CCCVV->text().toInt(),
                                       ccq.value("total").toDouble(),
                                       ccq.value("tax_in_cmcurr").toDouble(),
                                       ccq.value("cmhead_tax_id").isNull(),
                                       ccq.value("cmhead_freight").toDouble(),
                                       0,
                                       ccq.value("cmhead_curr_id").toInt(),
                                       docnum, refnum, ccpayid, reftype, refid);
        if (returnValue < 0)
        {
          QMessageBox::critical(this, tr("Credit Card Processing Error"),
                                cardproc->errorMsg());
          q.exec("ROLLBACK;");
          return;
        }
        else if (returnValue > 0)
          QMessageBox::warning(this, tr("Credit Card Processing Warning"),
                               cardproc->errorMsg());
        else if (! cardproc->errorMsg().isEmpty())
          QMessageBox::information(this, tr("Credit Card Processing Note"),
                               cardproc->errorMsg());
      }
    }
    else if (ccq.lastError().type() != QSqlError::NoError)
    {
      systemError(this, ccq.lastError().databaseText(), __FILE__, __LINE__);
      q.exec("ROLLBACK;");
      return;
    }
    else
    {
      QMessageBox::critical(this, tr("Credit Card Processing Error"),
                            tr("Could not find a Credit Card to use for "
                               "this Credit transaction."));
      q.exec("ROLLBACK;");
      return;
    }
  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    q.exec("ROLLBACK;");
    return;
  }
  q.exec("COMMIT;");
}

void returnAuthorization::sPopulateMenu( QMenu * pMenu,  QTreeWidgetItem *selected)
{
  QAction *menuItem;
  menuItem = pMenu->addAction(tr("Edit Line..."), this, SLOT(sEdit()));
  if (((XTreeWidgetItem *)selected)->rawValue("raitem_status").toString() == "O")
    menuItem = pMenu->addAction(tr("Close Line..."), this, SLOT(sAction()));
  if (((XTreeWidgetItem *)selected)->rawValue("raitem_status").toString() == "C")
    menuItem = pMenu->addAction(tr("Open Line..."), this, SLOT(sAction()));
  menuItem = pMenu->addAction(tr("Delete Line..."), this, SLOT(sDelete()));
  pMenu->addSeparator();

  if (((XTreeWidgetItem *)selected)->id("oldcohead_number") > -1)
  {
    pMenu->addAction(tr("View Original Order..."), this, SLOT(sViewOrigOrder()));
    menuItem->setEnabled(_privileges->check("ViewSalesOrders"));
  }
  
  pMenu->addSeparator();

  if (((XTreeWidgetItem *)selected)->id("newcohead_number") > -1)
  {
    menuItem = pMenu->addAction(tr("Edit New Order..."), this, SLOT(sEditNewOrder()));
    menuItem->setEnabled(_privileges->check("MaintainSalesOrders"));

    pMenu->addAction(tr("View New Order..."), this, SLOT(sViewNewOrder()));
    menuItem->setEnabled(_privileges->check("ViewSalesOrders"));

    pMenu->addSeparator();

    menuItem = pMenu->addAction(tr("Edit New Order Line..."), this, SLOT(sEditNewOrderLine()));
    menuItem->setEnabled(_privileges->check("MaintainSalesOrders"));

    pMenu->addAction(tr("View New Order Line..."), this, SLOT(sViewNewOrderLine()));
    menuItem->setEnabled(_privileges->check("ViewSalesOrders"));
  
    pMenu->addSeparator();

    pMenu->addAction(tr("New Order Shipment Status..."), this, SLOT(sShipmentStatus()));
    pMenu->addAction(tr("New Order Shipments..."), this, SLOT(sShipment()));
  
  }
}

void returnAuthorization::sViewOrigOrder()
{
  salesOrder::viewSalesOrder(_origso->id());
}

void returnAuthorization::sEditNewOrder()
{
  salesOrder::editSalesOrder(_newso->id(), false);
}

void returnAuthorization::sViewNewOrder()
{
  salesOrder::viewSalesOrder(_newso->id());
}

void returnAuthorization::sEditNewOrderLine()
{
  q.prepare("SELECT coitem_id, cohead_number, "
            "  cohead_curr_id, cohead_orderdate "
            "FROM raitem, cohead, coitem "
            "WHERE ((raitem_id=:raitem_id) "
            "AND (raitem_new_coitem_id=coitem_id) "
            "AND (coitem_cohead_id=cohead_id));");
  q.bindValue(":raitem_id", _raitem->id());
  q.exec();
  if (q.first())
  {
    ParameterList params;
    params.append("soitem_id", q.value("coitem_id").toInt());
    params.append("cust_id", _cust->id());
    params.append("orderNumber", q.value("cohead_number").toString());
    params.append("curr_id", q.value("cohead_curr_id").toInt());
    params.append("orderDate", q.value("cohead_orderdate").toDate());
    params.append("mode", "edit");

    salesOrderItem newdlg(this);
    newdlg.set(params);
    newdlg.exec();
  }
}

void returnAuthorization::sViewNewOrderLine()
{
  q.prepare("SELECT coitem_id, cohead_number, "
            "  cohead_curr_id, cohead_orderdate "
            "FROM raitem, cohead, coitem "
            "WHERE ((raitem_id=:raitem_id) "
            "AND (raitem_new_coitem_id=coitem_id) "
            "AND (coitem_cohead_id=cohead_id));");
  q.bindValue(":raitem_id", _raitem->id());
  q.exec();
  if (q.first())
  {
    ParameterList params;
    params.append("soitem_id", q.value("coitem_id").toInt());
    params.append("cust_id", _cust->id());
    params.append("orderNumber", q.value("cohead_number").toString());
    params.append("curr_id", q.value("cohead_curr_id").toInt());
    params.append("orderDate", q.value("cohead_orderdate").toDate());
    params.append("mode", "view");

    salesOrderItem newdlg(this);
    newdlg.set(params);
    newdlg.exec();
  }
}

void returnAuthorization::sShipmentStatus()
{
  ParameterList params;
  params.append("sohead_id", _newso->id());
  params.append("run");

  dspSalesOrderStatus *newdlg = new dspSalesOrderStatus();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void returnAuthorization::sShipment()
{
  ParameterList params;
  params.append("sohead_id", _newso->id() );

  dspShipmentsBySalesOrder* newdlg = new dspShipmentsBySalesOrder();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void returnAuthorization::sCheckNumber()
{
  q.prepare( "SELECT rahead_id "
                 "FROM rahead "
                 "WHERE (rahead_number=:rahead_number);" );
  q.bindValue(":rahead_number", _authNumber->text());
  q.exec();
  if (q.first())
  {
    _mode = cEdit;
    _raheadid = q.value("rahead_id").toInt();
    populate();
    _authNumber->setEnabled(FALSE);
    _cust->setReadOnly(TRUE);
  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void returnAuthorization::sRecalcFreight()
{
    q.prepare("SELECT SUM(freightdata_total) AS freight "
              "FROM freightDetail('RA', :head_id, :cust_id, :shipto_id, :orderdate, :shipvia, :curr_id);");
    q.bindValue(":head_id", _raheadid);
    q.bindValue(":cust_id", _cust->id());
    q.bindValue(":shipto_id", _shipTo->id());
    q.bindValue(":orderdate", _authDate->date());
    q.bindValue(":shipvia", "");
    q.bindValue(":curr_id", _currency->id());
    q.exec();
    if (q.first())
    {
      _freight->setLocalValue(q.value("freight").toDouble());
      _freight->setEnabled(FALSE);
    }
    else if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
}

void returnAuthorization::sFreightChanged()
{
  if (_freight->localValue() != _freightCache)
  {
    sSave(true);   
    _freightCache = _freight->localValue();
    sCalculateTax();
    sCalculateTotal();
  }
}

void returnAuthorization::sFreightDetail()
{
  ParameterList params;
  params.append("calcfreight", true);
  params.append("order_type", "RA");
  params.append("order_id", _raheadid);
  params.append("document_number", _authNumber->text());
  params.append("cust_id", _cust->id());
  params.append("shipto_id", _shipTo->id());
  params.append("orderdate", _authDate->date());
  params.append("shipvia", "");
  params.append("curr_id", _currency->id());

  // mode => view since there are no fields to hold modified freight data
  params.append("mode", "view");

  freightBreakdown newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}
