/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "configureSO.h"

#include <QSqlError>
#include <QMessageBox>

#include "editICMWatermark.h"

configureSO::configureSO(QWidget* parent, const char* name, bool /*modal*/, Qt::WFlags fl)
    : XAbstractConfigure(parent, fl)
{
  setupUi(this);

  if (name)
    setObjectName(name);

  connect(_invoiceNumOfCopies, SIGNAL(valueChanged(int)), this, SLOT(sHandleInvoiceCopies(int)));
  connect(_creditMemoNumOfCopies, SIGNAL(valueChanged(int)), this, SLOT(sHandleCreditMemoCopies(int)));
  connect(_invoiceWatermarks, SIGNAL(itemSelected(int)), this, SLOT(sEditInvoiceWatermark()));
  connect(_creditLimit,          SIGNAL(editingFinished()), this, SLOT(sEditCreditLimit()));
  connect(_creditMemoWatermarks, SIGNAL(itemSelected(int)), this, SLOT(sEditCreditMemoWatermark()));
  connect(_askUpdatePrice, SIGNAL(toggled(bool)), _ignoreCustDisc, SLOT(setEnabled(bool)));

  _invoiceWatermarks->addColumn( tr("Copy #"),      _dateColumn, Qt::AlignCenter );
  _invoiceWatermarks->addColumn( tr("Watermark"),   -1,          Qt::AlignLeft   );
  _invoiceWatermarks->addColumn( tr("Show Prices"), _dateColumn, Qt::AlignCenter );

  _creditMemoWatermarks->addColumn( tr("Copy #"),      _dateColumn, Qt::AlignCenter );
  _creditMemoWatermarks->addColumn( tr("Watermark"),   -1,          Qt::AlignLeft   );
  _creditMemoWatermarks->addColumn( tr("Show Prices"), _dateColumn, Qt::AlignCenter );

  _nextSoNumber->setValidator(omfgThis->orderVal());
  _nextQuNumber->setValidator(omfgThis->orderVal());
  _nextRaNumber->setValidator(omfgThis->orderVal());
  _nextCmNumber->setValidator(omfgThis->orderVal());
  _nextInNumber->setValidator(omfgThis->orderVal());
  _creditLimit->setValidator(omfgThis->moneyVal());

  _orderNumGeneration->setMethod(_metrics->value("CONumberGeneration"));
  _quoteNumGeneration->setMethod(_metrics->value("QUNumberGeneration"));
  _creditMemoNumGeneration->setMethod(_metrics->value("CMNumberGeneration"));
  _invoiceNumGeneration->setMethod(_metrics->value("InvcNumberGeneration"));

  QString metric;
  metric = _metrics->value("InvoiceDateSource");
  if (metric == "scheddate")
    _invcScheddate->setChecked(true);
  else if (metric == "shipdate")
    _invcShipdate->setChecked(true);
  else
    _invcCurrdate->setChecked(true);

  q.exec( "SELECT sonumber.orderseq_number AS sonumber,"
          "       qunumber.orderseq_number AS qunumber,"
          "       cmnumber.orderseq_number AS cmnumber,"
          "       innumber.orderseq_number AS innumber "
          "FROM orderseq AS sonumber,"
          "     orderseq AS qunumber,"
          "     orderseq AS cmnumber,"
          "     orderseq AS innumber "
          "WHERE ( (sonumber.orderseq_name='SoNumber')"
          " AND (qunumber.orderseq_name='QuNumber')"
          " AND (cmnumber.orderseq_name='CmNumber')"
          " AND (innumber.orderseq_name='InvcNumber') );" );
  if (q.first())
  {
    _nextSoNumber->setText(q.value("sonumber"));
    _nextQuNumber->setText(q.value("qunumber"));
    _nextCmNumber->setText(q.value("cmnumber"));
    _nextInNumber->setText(q.value("innumber"));
  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  _allowDiscounts->setChecked(_metrics->boolean("AllowDiscounts"));
  _allowASAP->setChecked(_metrics->boolean("AllowASAPShipSchedules"));
  _customerChangeLog->setChecked(_metrics->boolean("CustomerChangeLog"));
  _salesOrderChangeLog->setChecked(_metrics->boolean("SalesOrderChangeLog"));
  _restrictCreditMemos->setChecked(_metrics->boolean("RestrictCreditMemos"));
  _autoSelectForBilling->setChecked(_metrics->boolean("AutoSelectForBilling"));
  _saveAndAdd->setChecked(_metrics->boolean("AlwaysShowSaveAndAdd"));
  _firmAndAdd->setChecked(_metrics->boolean("FirmSalesOrderPackingList"));
  _priceOverride->setChecked(_metrics->boolean("DisableSalesOrderPriceOverride"));
  _autoAllocateCM->setChecked(_metrics->boolean("AutoAllocateCreditMemos"));
  _hideSOMiscChrg->setChecked(_metrics->boolean("HideSOMiscCharge"));
  _enableSOShipping->setChecked(_metrics->boolean("EnableSOShipping"));
  _printSO->setChecked(_metrics->boolean("DefaultPrintSOOnSave"));
  _enablePromiseDate->setChecked(_metrics->boolean("UsePromiseDate"));
  _calcFreight->setChecked(_metrics->boolean("CalculateFreight"));
  _includePkgWeight->setChecked(_metrics->boolean("IncludePackageWeight"));
  _quoteafterSO->setChecked(_metrics->boolean("ShowQuotesAfterSO"));

  _invoiceNumOfCopies->setValue(_metrics->value("InvoiceCopies").toInt());
  if (_invoiceNumOfCopies->value())
  {
    for (int counter = 0; counter < _invoiceWatermarks->topLevelItemCount(); counter++)
    {
      _invoiceWatermarks->topLevelItem(counter)->setText(1, _metrics->value(QString("InvoiceWatermark%1").arg(counter)));
      _invoiceWatermarks->topLevelItem(counter)->setText(2, ((_metrics->value(QString("InvoiceShowPrices%1").arg(counter)) == "t") ? tr("Yes") : tr("No")));
    }
  }

  _creditMemoNumOfCopies->setValue(_metrics->value("CreditMemoCopies").toInt());
  if (_invoiceNumOfCopies->value())
  {
    for (int counter = 0; counter < _creditMemoWatermarks->topLevelItemCount(); counter++)
    {
      _creditMemoWatermarks->topLevelItem(counter)->setText(1, _metrics->value(QString("CreditMemoWatermark%1").arg(counter)));
      _creditMemoWatermarks->topLevelItem(counter)->setText(2, ((_metrics->value(QString("CreditMemoShowPrices%1").arg(counter)) == "t") ? tr("Yes") : tr("No")));
    }
  }

  _shipform->setId(_metrics->value("DefaultShipFormId").toInt());
  _shipvia->setId(_metrics->value("DefaultShipViaId").toInt());

  if (_metrics->value("DefaultBalanceMethod") == "B")
    _balanceMethod->setCurrentIndex(0);
  else if (_metrics->value("DefaultBalanceMethod") == "O")
    _balanceMethod->setCurrentIndex(1);

  _custtype->setId(_metrics->value("DefaultCustType").toInt());
  _salesrep->setId(_metrics->value("DefaultSalesRep").toInt());
  _terms->setId(_metrics->value("DefaultTerms").toInt());

  _partial->setChecked(_metrics->boolean("DefaultPartialShipments"));
  _backorders->setChecked(_metrics->boolean("DefaultBackOrders"));
  _freeFormShiptos->setChecked(_metrics->boolean("DefaultFreeFormShiptos"));

  _creditLimit->setText(_metrics->value("SOCreditLimit"));
  _creditRating->setText(_metrics->value("SOCreditRate"));

  if (_metrics->value("soPriceEffective") == "OrderDate")
    _priceOrdered->setChecked(true);
  else if (_metrics->value("soPriceEffective") == "ScheduleDate")
    _priceScheduled->setChecked(true);

  if(_metrics->value("UpdatePriceLineEdit").toInt() == 1)
    _dontUpdatePrice->setChecked(true);
  else if (_metrics->value("UpdatePriceLineEdit").toInt() == 2)
    _askUpdatePrice->setChecked(true);
  else if(_metrics->value("UpdatePriceLineEdit").toInt() == 3)
    _updatePrice->setChecked(true);
  _ignoreCustDisc->setChecked(_askUpdatePrice->isChecked() && _metrics->boolean("IgnoreCustDisc"));

  this->setWindowTitle("Sales Configuration");

  //Set status of Returns Authorization based on context
  if(_metrics->value("Application") == "PostBooks")
  {
    _authNumGenerationLit->setVisible(false);
    _returnAuthorizationNumGeneration->setVisible(false);
    _nextRaNumberLit->setVisible(false);
    _nextRaNumber->setVisible(false);
    _tab->removeTab(_tab->indexOf(_returns));
    _enableReturns->setChecked(false);
    _enableReservations->hide();
    _enableReservations->setChecked(false);
    _locationGroup->hide();
    _locationGroup->setChecked(false);
    _lowest->setChecked(false);
    _highest->setChecked(false);
    _alpha->setChecked(false);
  }
  else
  {
    q.exec("SELECT rahead_id FROM rahead LIMIT 1;");
    if (q.first())
      _enableReturns->setCheckable(false);
    else
      _enableReturns->setChecked(_metrics->boolean("EnableReturnAuth"));

    q.exec( "SELECT ranumber.orderseq_number AS ranumber "
            "FROM orderseq AS ranumber "
            "WHERE (ranumber.orderseq_name='RaNumber');" );
    if (q.first())
    {
      _nextRaNumber->setText(q.value("ranumber"));
    }
    else
      _nextRaNumber->setText("10000");

    metric = _metrics->value("RANumberGeneration");
    if (metric == "M")
      _returnAuthorizationNumGeneration->setCurrentIndex(0);
    else if (metric == "A")
      _returnAuthorizationNumGeneration->setCurrentIndex(1);
    else if (metric == "O")
      _returnAuthorizationNumGeneration->setCurrentIndex(2);

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

    _returnAuthChangeLog->setChecked(_metrics->boolean("ReturnAuthorizationChangeLog"));
    _printRA->setChecked(_metrics->boolean("DefaultPrintRAOnSave"));

    _enableReservations->setChecked(_metrics->boolean("EnableSOReservations"));

    _locationGroup->setChecked(_metrics->boolean("EnableSOReservationsByLocation"));
    if(_metrics->value("SOReservationLocationMethod").toInt() == 1)
      _lowest->setChecked(true);
    else if (_metrics->value("SOReservationLocationMethod").toInt() == 2)
      _highest->setChecked(true);
    else if(_metrics->value("SOReservationLocationMethod").toInt() == 3)
      _alpha->setChecked(true);
  }
  adjustSize();
}

configureSO::~configureSO()
{
  // no need to delete child widgets, Qt does it all for us
}

void configureSO::languageChange()
{
  retranslateUi(this);
}

bool configureSO::sSave()
{
  emit saving();

  const char *dispositionTypes[] = { "C", "R", "P", "V", "M", "" };
  const char *timingTypes[] = { "I", "R", "" };
  const char *creditMethodTypes[] = { "N", "M", "K", "C", "" };

  if ( (_metrics->boolean("EnableSOReservationsByLocation")) &&
       (!_locationGroup->isChecked()) )
  {
    if (QMessageBox::warning(this, tr("Reserve by Location Disabled"),
                             tr("<p>All existing location reservations will be removed. Are you sure you want to continue?"),
                             QMessageBox::Yes, QMessageBox::No | QMessageBox::Default) == QMessageBox::No)
    {
      return false;
    }
    else
    {
      q.prepare("DELETE FROM itemlocrsrv "
                " WHERE (itemlocrsrv_source='SO');");
      q.exec();
    }	
  }

  _metrics->set("ShowQuotesAfterSO", _quoteafterSO->isChecked());
  _metrics->set("AllowDiscounts", _allowDiscounts->isChecked());
  _metrics->set("AllowASAPShipSchedules", _allowASAP->isChecked());
  _metrics->set("CustomerChangeLog", _customerChangeLog->isChecked());
  _metrics->set("SalesOrderChangeLog", _salesOrderChangeLog->isChecked());
  _metrics->set("RestrictCreditMemos", _restrictCreditMemos->isChecked());
  _metrics->set("AutoSelectForBilling", _autoSelectForBilling->isChecked());
  _metrics->set("AlwaysShowSaveAndAdd", _saveAndAdd->isChecked());
  _metrics->set("FirmSalesOrderPackingList", _firmAndAdd->isChecked());
  _metrics->set("DisableSalesOrderPriceOverride", _priceOverride->isChecked());
  _metrics->set("AutoAllocateCreditMemos", _autoAllocateCM->isChecked());
  _metrics->set("HideSOMiscCharge", _hideSOMiscChrg->isChecked());
  _metrics->set("EnableSOShipping", _enableSOShipping->isChecked());
  _metrics->set("CONumberGeneration",   _orderNumGeneration->methodCode());
  _metrics->set("QUNumberGeneration",   _quoteNumGeneration->methodCode());
  _metrics->set("CMNumberGeneration",   _creditMemoNumGeneration->methodCode());
  _metrics->set("InvcNumberGeneration", _invoiceNumGeneration->methodCode());
  _metrics->set("DefaultShipFormId", _shipform->id());
  _metrics->set("DefaultShipViaId", _shipvia->id());
  _metrics->set("DefaultCustType", _custtype->id());
  _metrics->set("DefaultSalesRep", _salesrep->id());
  _metrics->set("DefaultTerms", _terms->id());
  _metrics->set("DefaultPartialShipments", _partial->isChecked());
  _metrics->set("DefaultBackOrders", _backorders->isChecked());
  _metrics->set("DefaultFreeFormShiptos", _freeFormShiptos->isChecked());
  _metrics->set("DefaultPrintSOOnSave", _printSO->isChecked());
  _metrics->set("UsePromiseDate", _enablePromiseDate->isChecked());
  _metrics->set("CalculateFreight", _calcFreight->isChecked());
  _metrics->set("IncludePackageWeight", _includePkgWeight->isChecked());
  _metrics->set("EnableReturnAuth", _enableReturns->isChecked());
  _metrics->set("EnableSOReservations", _enableReservations->isChecked());

  _metrics->set("EnableSOReservationsByLocation", _locationGroup->isChecked());
  //SOReservationLocationMethod are three Options Either 
  // Lowest quantity first,
  // Highest quantity first,
  // Alpha by Location Name
  if(_lowest->isChecked())
    _metrics->set("SOReservationLocationMethod", 1);
  else if (_highest->isChecked())
    _metrics->set("SOReservationLocationMethod", 2);
  else if(_alpha->isChecked())
    _metrics->set("SOReservationLocationMethod", 3);

  _metrics->set("SOCreditLimit", _creditLimit->text());
  _metrics->set("SOCreditRate", _creditRating->text());

  if (_priceOrdered->isChecked())
    _metrics->set("soPriceEffective", QString("OrderDate"));
  else if (_priceScheduled->isChecked())
    _metrics->set("soPriceEffective", QString("ScheduleDate"));
  else
    _metrics->set("soPriceEffective", QString("CurrentDate"));

  //UpdatePriceLineEdit are three Options Either 
  // Don't Update price
  // Ask to Update Price,
  // Update Price
  if(_dontUpdatePrice->isChecked())
    _metrics->set("UpdatePriceLineEdit", 1);
  else if (_askUpdatePrice->isChecked())
    _metrics->set("UpdatePriceLineEdit", 2);
  else if(_updatePrice->isChecked())
    _metrics->set("UpdatePriceLineEdit", 3);
  _metrics->set("IgnoreCustDisc", _askUpdatePrice->isChecked() && _ignoreCustDisc->isChecked());

  if(_invcScheddate->isChecked())
    _metrics->set("InvoiceDateSource", QString("scheddate"));
  else if(_invcShipdate->isChecked())
    _metrics->set("InvoiceDateSource", QString("shipdate"));
  else
    _metrics->set("InvoiceDateSource", QString("currdate"));

  _metrics->set("InvoiceCopies", _invoiceNumOfCopies->value());
  if (_invoiceNumOfCopies->value())
  {
    for (int counter = 0; counter < _invoiceWatermarks->topLevelItemCount(); counter++ )
    {
      _metrics->set(QString("InvoiceWatermark%1").arg(counter),  _invoiceWatermarks->topLevelItem(counter)->text(1));
      _metrics->set(QString("InvoiceShowPrices%1").arg(counter), _invoiceWatermarks->topLevelItem(counter)->text(2) == tr("Yes"));
    }
  }

  _metrics->set("CreditMemoCopies", _creditMemoNumOfCopies->value());
  if (_creditMemoNumOfCopies->value())
  {
    for (int counter = 0; counter < _creditMemoWatermarks->topLevelItemCount(); counter++)
    {
      _metrics->set(QString("CreditMemoWatermark%1").arg(counter), _creditMemoWatermarks->topLevelItem(counter)->text(1));
      _metrics->set(QString("CreditMemoShowPrices%1").arg(counter), (_creditMemoWatermarks->topLevelItem(counter)->text(2) == tr("Yes")));
    }
  }

  switch (_balanceMethod->currentIndex())
  {
  case 0:
    _metrics->set("DefaultBalanceMethod", QString("B"));
    break;

  case 1:
    _metrics->set("DefaultBalanceMethod", QString("O"));
    break;
  }

  q.prepare( "SELECT setNextSoNumber(:sonumber), setNextQuNumber(:qunumber),"
             "       setNextCmNumber(:cmnumber), setNextInvcNumber(:innumber);" );
  q.bindValue(":sonumber", _nextSoNumber->text().toInt());
  q.bindValue(":qunumber", _nextQuNumber->text().toInt());
  q.bindValue(":cmnumber", _nextCmNumber->text().toInt());
  q.bindValue(":innumber", _nextInNumber->text().toInt());
  q.exec();
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return false;
  }

  if (_enableReturns->isChecked() || !_enableReturns->isCheckable())
  {
    _metrics->set("DefaultRaDisposition", QString(dispositionTypes[_disposition->currentIndex()]));
    _metrics->set("DefaultRaTiming", QString(timingTypes[_timing->currentIndex()]));
    _metrics->set("DefaultRaCreditMethod", QString(creditMethodTypes[_creditBy->currentIndex()]));
    _metrics->set("ReturnAuthorizationChangeLog", _returnAuthChangeLog->isChecked());
    _metrics->set("DefaultPrintRAOnSave", _printRA->isChecked());
    _metrics->set("RANumberGeneration", _returnAuthorizationNumGeneration->methodCode());

    q.prepare( "SELECT setNextRaNumber(:ranumber);" );
    q.bindValue(":ranumber", _nextRaNumber->text().toInt());
    q.exec();
    if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return false;
    }
  }
  _metrics->set("EnableReturnAuth", (_enableReturns->isChecked() || !_enableReturns->isCheckable()));

  return true;
}

void configureSO::sHandleInvoiceCopies(int pValue)
{
  if (_invoiceWatermarks->topLevelItemCount() > pValue)
    _invoiceWatermarks->takeTopLevelItem(_invoiceWatermarks->topLevelItemCount() - 1);
  else
  {
    XTreeWidgetItem *last = static_cast<XTreeWidgetItem*>(_invoiceWatermarks->topLevelItem(_invoiceWatermarks->topLevelItemCount() - 1));
    for (int counter = (_invoiceWatermarks->topLevelItemCount()); counter <= pValue; counter++)
      last = new XTreeWidgetItem(_invoiceWatermarks, last, counter, QVariant(tr("Copy #%1").arg(counter)), QVariant(""), QVariant(tr("Yes")));
  }
}

void configureSO::sHandleCreditMemoCopies(int pValue)
{
  if (_creditMemoWatermarks->topLevelItemCount() > pValue)
    _creditMemoWatermarks->takeTopLevelItem(_creditMemoWatermarks->topLevelItemCount() - 1);
  else
  {
    XTreeWidgetItem *last = static_cast<XTreeWidgetItem*>(_creditMemoWatermarks->topLevelItem(_creditMemoWatermarks->topLevelItemCount() - 1));
    for (int counter = (_creditMemoWatermarks->topLevelItemCount()); counter <= pValue; counter++)
      last = new XTreeWidgetItem(_creditMemoWatermarks, last, counter, QVariant(tr("Copy #%1").arg(counter)), QVariant(""), QVariant(tr("Yes")));
  }
}

void configureSO::sEditInvoiceWatermark()
{
  QList<XTreeWidgetItem*>selected = _invoiceWatermarks->selectedItems();
  for (int counter = 0; counter < selected.size(); counter++)
  {
    XTreeWidgetItem *cursor = static_cast<XTreeWidgetItem*>(selected[counter]);
    ParameterList params;
    params.append("watermark", cursor->text(1));
    params.append("showPrices", (cursor->text(2) == tr("Yes")));

    editICMWatermark newdlg(this, "", TRUE);
    newdlg.set(params);
    if (newdlg.exec() == XDialog::Accepted)
    {
      cursor->setText(1, newdlg.watermark());
      cursor->setText(2, ((newdlg.showPrices()) ? tr("Yes") : tr("No")));
    }
  }
}


void configureSO::sEditCreditMemoWatermark()
{
  QList<XTreeWidgetItem*>selected = _creditMemoWatermarks->selectedItems();
  for (int counter = 0; counter < selected.size(); counter++)
  {
    XTreeWidgetItem *cursor = static_cast<XTreeWidgetItem*>(selected[counter]);
    ParameterList params;
    params.append("watermark", cursor->text(1));
    params.append("showPrices", (cursor->text(2) == tr("Yes")));

    editICMWatermark newdlg(this, "", TRUE);
    newdlg.set(params);
    if (newdlg.exec() == XDialog::Accepted)
    {
      cursor->setText(1, newdlg.watermark());
      cursor->setText(2, ((newdlg.showPrices()) ? tr("Yes") : tr("No")));
    }
  }
}

void configureSO::sEditCreditLimit()
{
  _creditLimit->setDouble((double)qRound(_creditLimit->toDouble()));
}
