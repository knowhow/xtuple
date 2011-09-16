/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "configureIM.h"

#include <QSqlError>
#include <QVariant>
#include <QMessageBox>

#include "guiclient.h"
#include "editICMWatermark.h"
#include "storedProcErrorLookup.h"

configureIM::configureIM(QWidget* parent, const char* name, bool /*modal*/, Qt::WFlags fl)
    : XAbstractConfigure(parent, fl)
{
  setupUi(this);

  if (name)
    setObjectName(name);

  connect(_shipformWatermarks, SIGNAL(itemSelected(int)), this, SLOT(sEditShippingFormWatermark()));
  connect(_shipformNumOfCopies, SIGNAL(valueChanged(int)), this, SLOT(sHandleShippingFormCopies(int)));

  //Inventory
  //Disable multi-warehouse if PostBooks
  if (_metrics->value("Application") == "PostBooks")
    _multiWhs->hide();
  else
  {
    q.exec("SELECT * "
	   "FROM whsinfo");
    if (q.size() > 1)
    {
      _multiWhs->setCheckable(FALSE);
      _multiWhs->setTitle("Multiple Sites");
    }
    else
      _multiWhs->setChecked(_metrics->boolean("MultiWhs"));
      
    if (_metrics->value("TONumberGeneration") == "M")
      _toNumGeneration->setCurrentIndex(0); 
    else if (_metrics->value("TONumberGeneration") == "A")
      _toNumGeneration->setCurrentIndex(1);
    else if (_metrics->value("TONumberGeneration") == "O")
      _toNumGeneration->setCurrentIndex(2);

    _toNextNum->setValidator(omfgThis->orderVal());
    q.exec( "SELECT orderseq_number AS tonumber "
	    "FROM orderseq "
	    "WHERE (orderseq_name='ToNumber');" );
    if (q.first())
      _toNextNum->setText(q.value("tonumber").toString());
    else if (q.lastError().type() != QSqlError::NoError)
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);

    _enableToShipping->setChecked(_metrics->boolean("EnableTOShipping"));
    _transferOrderChangeLog->setChecked(_metrics->boolean("TransferOrderChangeLog"));
  }

  _eventFence->setValue(_metrics->value("DefaultEventFence").toInt());
  _itemSiteChangeLog->setChecked(_metrics->boolean("ItemSiteChangeLog"));
  _warehouseChangeLog->setChecked(_metrics->boolean("WarehouseChangeLog"));
  
  if (_metrics->boolean("PostCountTagToDefault"))
    _postToDefault->setChecked(TRUE);
  else
    _doNotPost->setChecked(TRUE);

  _defaultTransWhs->setId(_metrics->value("DefaultTransitWarehouse").toInt());

  QString countSlipAuditing = _metrics->value("CountSlipAuditing");
  if (countSlipAuditing == "N")
    _noSlipChecks->setChecked(TRUE);
  else if (countSlipAuditing == "W")
    _checkOnUnpostedWarehouse->setChecked(TRUE);
  else if (countSlipAuditing == "A")
    _checkOnUnposted->setChecked(TRUE);
  else if (countSlipAuditing == "X")
    _checkOnAllWarehouse->setChecked(TRUE);
  else if (countSlipAuditing == "B")
    _checkOnAll->setChecked(TRUE);
    
  QString avgCostingMethod = _metrics->value("CountAvgCostMethod");
  if (avgCostingMethod == "STD")
    _useStdCost->setChecked(TRUE);
  else if (avgCostingMethod == "ACT")
    _useActCost->setChecked(TRUE);
  else if (avgCostingMethod == "AVG")
    _useAvgCost->setChecked(TRUE);
  else
    _useStdCost->setChecked(TRUE);
    
  if(_metrics->value("Application") != "PostBooks")
  {
    q.exec("SELECT DISTINCT itemsite_controlmethod "
	      "FROM itemsite "
	      "WHERE (itemsite_controlmethod IN ('L','S'));");
    if (q.first())
    {
      _lotSerial->setChecked(TRUE);
      _lotSerial->setEnabled(FALSE);
    }
    else
      _lotSerial->setChecked(_metrics->boolean("LotSerialControl"));
  }
  else
    _lotSerial->hide();
  _setDefLoc->setChecked(_metrics->boolean("SetDefaultLocations"));

// Shipping and Receiving
  QString metric = _metrics->value("ShipmentNumberGeneration");
  if (metric == "A")
    _shipmentNumGeneration->setCurrentIndex(0);

  _nextShipmentNum->setValidator(omfgThis->orderVal());
  q.exec("SELECT setval('shipment_number_seq', nextval('shipment_number_seq') -1); "
         "SELECT currval('shipment_number_seq') AS shipment_number;");
  if (q.first())
    _nextShipmentNum->setText(q.value("shipment_number"));
  else if (q.lastError().type() != QSqlError::NoError)
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);

  _shipformWatermarks->addColumn( tr("Copy #"),      _dateColumn, Qt::AlignCenter );
  _shipformWatermarks->addColumn( tr("Watermark"),   -1,          Qt::AlignLeft   );
  _shipformWatermarks->addColumn( tr("Show Prices"), _dateColumn, Qt::AlignCenter );

  _shipformNumOfCopies->setValue(_metrics->value("ShippingFormCopies").toInt());

  if (_shipformNumOfCopies->value())
  {
    for (int counter = 0; counter < _shipformWatermarks->topLevelItemCount(); counter++)
    {
      QTreeWidgetItem *cursor = _shipformWatermarks->topLevelItem(counter);
      cursor->setText(1, _metrics->value(QString("ShippingFormWatermark%1").arg(counter)));
      cursor->setText(2, ((_metrics->boolean(QString("ShippingFormShowPrices%1").arg(counter))) ? tr("Yes") : tr("No")));
    }
  }

  _kitInheritCOS->setChecked(_metrics->boolean("KitComponentInheritCOS"));
  _disallowReceiptExcess->setChecked(_metrics->boolean("DisallowReceiptExcessQty"));
  _warnIfReceiptDiffers->setChecked(_metrics->boolean("WarnIfReceiptQtyDiffers"));
  _recordPpvOnReceipt->setChecked(_metrics->boolean("RecordPPVonReceipt"));

  _tolerance->setValidator(omfgThis->percentVal());
  _tolerance->setText(_metrics->value("ReceiptQtyTolerancePct"));

  _costAvg->setChecked(_metrics->boolean("AllowAvgCostMethod"));
  _costStd->setChecked(_metrics->boolean("AllowStdCostMethod"));
  _costJob->setChecked(_metrics->boolean("AllowJobCostMethod"));

  q.prepare("SELECT count(*) AS result FROM itemsite WHERE(itemsite_costmethod='A');");
  q.exec();
  if(q.first() && q.value("result").toInt() > 0)
  {
    _costAvg->setChecked(true);
    _costAvg->setEnabled(false);
  }

  q.prepare("SELECT count(*) AS result FROM itemsite WHERE(itemsite_costmethod='S');");
  q.exec();
  if(q.first() && q.value("result").toInt() > 0)
  {
    _costStd->setChecked(true);
    _costStd->setEnabled(false);
  }

  if(!_costAvg->isChecked() && !_costStd->isChecked())
    _costStd->isChecked();
  
  // Jobs at this time should always be checked and disabled
  // when this is changed in the future this should be replaced with
  // similar code checks as for Avg and Std cost
  _costJob->setChecked(true);
  _costJob->setEnabled(false);
    
  this->setWindowTitle("Inventory Configuration");

  // Requires Consolidated shipping package
  _shipByGroup->hide();

  adjustSize();
}

configureIM::~configureIM()
{
    // no need to delete child widgets, Qt does it all for us
}

void configureIM::languageChange()
{
    retranslateUi(this);
}

bool configureIM::sSave()
{
  emit saving();

  if(!_costAvg->isChecked() && !_costStd->isChecked())
  { 
    QMessageBox::warning(this, tr("No Cost selected"),
                         tr("<p>You must have checked Standard Cost, "
                            "Average Cost or both before saving."));
    return false;
  }

  // Inventory
  _metrics->set("DefaultEventFence", _eventFence->value());
  _metrics->set("ItemSiteChangeLog", _itemSiteChangeLog->isChecked());
  _metrics->set("WarehouseChangeLog", _warehouseChangeLog->isChecked());
  _metrics->set("PostCountTagToDefault", _postToDefault->isChecked());
  _metrics->set("MultiWhs", ((!_multiWhs->isCheckable()) || (_multiWhs->isChecked())));
  _metrics->set("LotSerialControl", _lotSerial->isChecked());
  _metrics->set("SetDefaultLocations", _setDefLoc->isChecked());
  _metrics->set("AllowAvgCostMethod", _costAvg->isChecked());
  _metrics->set("AllowStdCostMethod", _costStd->isChecked());
  _metrics->set("AllowJobCostMethod", _costJob->isChecked());
  
  if (_toNumGeneration->currentIndex() == 0)
    _metrics->set("TONumberGeneration", QString("M"));
  else if (_toNumGeneration->currentIndex() == 1)
    _metrics->set("TONumberGeneration", QString("A"));
  else if (_toNumGeneration->currentIndex() == 2)
    _metrics->set("TONumberGeneration", QString("O"));

  if (_metrics->boolean("MultiWhs"))
  {
    q.prepare("SELECT setNextNumber('ToNumber', :next) AS result;");
    q.bindValue(":next", _toNextNum->text());
    q.exec();
    if (q.first())
    {
      int result = q.value("result").toInt();
      if (result < 0)
      {
        systemError(this, storedProcErrorLookup("setNextNumber", result), __FILE__, __LINE__);
        _toNextNum->setFocus();
        return false;
      }
    }
    else if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      _toNextNum->setFocus();
      return false;
    }

    _metrics->set("DefaultTransitWarehouse", _defaultTransWhs->id());
    _metrics->set("EnableTOShipping",	     _enableToShipping->isChecked());
    _metrics->set("TransferOrderChangeLog",  _transferOrderChangeLog->isChecked());
  }

  if (_noSlipChecks->isChecked())
    _metrics->set("CountSlipAuditing", QString("N"));
  else if (_checkOnUnpostedWarehouse->isChecked())
    _metrics->set("CountSlipAuditing", QString("W"));
  else if (_checkOnUnposted->isChecked())
    _metrics->set("CountSlipAuditing", QString("A"));
  else if (_checkOnAllWarehouse->isChecked())
    _metrics->set("CountSlipAuditing", QString("X"));
  else if (_checkOnAll->isChecked())
    _metrics->set("CountSlipAuditing", QString("B"));
    
  if (_useStdCost->isChecked())
    _metrics->set("CountAvgCostMethod", QString("STD"));
  else if (_useActCost->isChecked())
    _metrics->set("CountAvgCostMethod", QString("ACT"));
  else if (_useAvgCost->isChecked())
    _metrics->set("CountAvgCostMethod", QString("AVG"));

  //Shipping and Receiving
  const char *numberGenerationTypes[] = { "A" };

  _metrics->set("ShipmentNumberGeneration", QString(numberGenerationTypes[_shipmentNumGeneration->currentIndex()]));

  _metrics->set("ShippingFormCopies", _shipformNumOfCopies->value());

  if (_shipformNumOfCopies->value())
  {
    for (int counter = 0; counter < _shipformWatermarks->topLevelItemCount(); counter++)
    {
      QTreeWidgetItem *cursor = _shipformWatermarks->topLevelItem(counter);
      _metrics->set(QString("ShippingFormWatermark%1").arg(counter), cursor->text(1));
      _metrics->set(QString("ShippingFormShowPrices%1").arg(counter), (cursor->text(2) == tr("Yes")));
    }
  }

  _metrics->set("KitComponentInheritCOS", _kitInheritCOS->isChecked());
  _metrics->set("DisallowReceiptExcessQty", _disallowReceiptExcess->isChecked());
  _metrics->set("WarnIfReceiptQtyDiffers", _warnIfReceiptDiffers->isChecked());
  _metrics->set("ReceiptQtyTolerancePct", _tolerance->text());
  _metrics->set("RecordPPVonReceipt", _recordPpvOnReceipt->isChecked());

  q.prepare("SELECT setval('shipment_number_seq', :shipmentnumber);");
  q.bindValue(":shipmentnumber", _nextShipmentNum->text().toInt());
  q.exec();
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    _nextShipmentNum->setFocus();
    return false;
  }

  return true;
}


void configureIM::sHandleShippingFormCopies(int pValue)
{
  if (_shipformWatermarks->topLevelItemCount() > pValue)
    _shipformWatermarks->takeTopLevelItem(_shipformWatermarks->topLevelItemCount() - 1);
  else
  {
    for (unsigned int counter = (_shipformWatermarks->topLevelItemCount() + 1); counter <= (unsigned int)pValue; counter++)
      new XTreeWidgetItem(_shipformWatermarks,
			  (XTreeWidgetItem*)(_shipformWatermarks->topLevelItem(_shipformWatermarks->topLevelItemCount() - 1)),
			  counter,
			  QVariant(tr("Copy #%1").arg(counter)), "", tr("Yes"));
  }
}

void configureIM::sEditShippingFormWatermark()
{
  XTreeWidgetItem *cursor = (XTreeWidgetItem*)(_shipformWatermarks->currentItem());

  if (cursor)
  {
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
