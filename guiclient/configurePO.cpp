/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "configurePO.h"

#include <QValidator>
#include <QVariant>

configurePO::configurePO(QWidget* parent, const char* name, bool /*modal*/, Qt::WFlags fl)
    : XAbstractConfigure(parent, fl)
{
  setupUi(this);

  if (name)
    setObjectName(name);

  connect(_internalCopy, SIGNAL(toggled(bool)), _numOfCopies, SLOT(setEnabled(bool)));

  XSqlQuery configq;
  if (_metrics->value("Application") == "PostBooks")
  {  
    _enableDropShip->hide();
    _billDropShip->hide();
  }
  else
  {
    configq.prepare("SELECT pohead_id FROM pohead "
	          "WHERE ( (pohead_dropship = 'TRUE') "
              "  AND (pohead_status = 'O') )");
    configq.exec();
    if (configq.first())
    {
      _enableDropShip->setChecked(TRUE);
	  _enableDropShip->setEnabled(FALSE);
	  _billDropShip->setChecked(_metrics->boolean("BillDropShip"));
	  _billDropShip->setEnabled(FALSE);

    }
    else
	{
      _enableDropShip->setChecked(_metrics->boolean("EnableDropShipments"));
	  _billDropShip->setChecked(_metrics->boolean("BillDropShip"));
	}
  }

  if (_metrics->value("PONumberGeneration") == "M")
    _orderNumGeneration->setCurrentIndex(0);
  else if (_metrics->value("PONumberGeneration") == "A")
    _orderNumGeneration->setCurrentIndex(1);
  else if (_metrics->value("PONumberGeneration") == "O")
    _orderNumGeneration->setCurrentIndex(2);

#if 0
  if (_metrics->value("VoucherNumberGeneration") == "M")
    _voucherNumGeneration->setCurrentIndex(0);
  else if (_metrics->value("VoucherNumberGeneration") == "A")
    _voucherNumGeneration->setCurrentIndex(1);
  else if (_metrics->value("VoucherNumberGeneration") == "O")
    _voucherNumGeneration->setCurrentIndex(2);
#endif
//  ToDo
 
  if (_metrics->value("PrNumberGeneration") == "M")
    _prNumberGeneration->setCurrentIndex(0);
  else if (_metrics->value("PrNumberGeneration") == "A")
    _prNumberGeneration->setCurrentIndex(1);
  else if (_metrics->value("PrNumberGeneration") == "O")
    _prNumberGeneration->setCurrentIndex(2);

  _nextPoNumber->setValidator(omfgThis->orderVal());
  _nextVcNumber->setValidator(omfgThis->orderVal());
  _nextPrNumber->setValidator(omfgThis->orderVal());

  configq.exec( "SELECT ponumber.orderseq_number AS ponumber,"
          "       vcnumber.orderseq_number AS vcnumber,"
          "       prnumber.orderseq_number AS prnumber "
          "FROM orderseq AS ponumber,"
          "     orderseq AS vcnumber,"
          "     orderseq AS prnumber "
          "WHERE ( (ponumber.orderseq_name='PoNumber')"
          " AND (vcnumber.orderseq_name='VcNumber')"
          " AND (prnumber.orderseq_name='PrNumber') );" );
  if (configq.first())
  {
    _nextPoNumber->setText(configq.value("ponumber").toString());
    _nextVcNumber->setText(configq.value("vcnumber").toString());
    _nextPrNumber->setText(configq.value("prnumber").toString());
  }

  _poChangeLog->setChecked(_metrics->boolean("POChangeLog"));
  _vendorChangeLog->setChecked(_metrics->boolean("VendorChangeLog"));
  _earliestPO->setChecked(_metrics->boolean("UseEarliestAvailDateOnPOItem"));
  _printPO->setChecked(_metrics->boolean("DefaultPrintPOOnSave"));

  _vendorCopy->setChecked(_metrics->boolean("POVendor"));
  _requirePoitemStdCost->setChecked(_metrics->boolean("RequireStdCostForPOItem"));
  _notes->setChecked(_metrics->boolean("CopyPRtoPOItem"));
  if (_metrics->value("POInternal").toInt() > 0)
  {
    _internalCopy->setChecked(TRUE);
    _numOfCopies->setValue(_metrics->value("POInternal").toInt());
  }
  else
  {
    _internalCopy->setChecked(FALSE);
    _numOfCopies->setEnabled(FALSE);
  }

  _defaultShipVia->setText(_metrics->value("DefaultPOShipVia"));

  this->setWindowTitle("Purchase Configuration");
}

configurePO::~configurePO()
{
  // no need to delete child widgets, Qt does it all for us
}

void configurePO::languageChange()
{
  retranslateUi(this);
}

bool configurePO::sSave()
{
  emit saving();

  if (_orderNumGeneration->currentIndex() == 0)
    _metrics->set("PONumberGeneration", QString("M"));
  else if (_orderNumGeneration->currentIndex() == 1)
    _metrics->set("PONumberGeneration", QString("A"));
  else if (_orderNumGeneration->currentIndex() == 2)
    _metrics->set("PONumberGeneration", QString("O"));

  _metrics->set("VoucherNumberGeneration", QString("A"));

  if (_prNumberGeneration->currentIndex() == 0)
    _metrics->set("PrNumberGeneration", QString("M"));
  if (_prNumberGeneration->currentIndex() == 1)
    _metrics->set("PrNumberGeneration", QString("A"));
  if (_prNumberGeneration->currentIndex() == 2)
    _metrics->set("PrNumberGeneration", QString("O"));

  _metrics->set("POChangeLog", _poChangeLog->isChecked());
  _metrics->set("VendorChangeLog", _vendorChangeLog->isChecked());
  _metrics->set("UseEarliestAvailDateOnPOItem", _earliestPO->isChecked());
  _metrics->set("DefaultPrintPOOnSave", _printPO->isChecked());
  _metrics->set("EnableDropShipments",  _enableDropShip->isChecked());
  _metrics->set("BillDropShip",         _billDropShip->isChecked());
 
  _metrics->set("POVendor", _vendorCopy->isChecked());
  _metrics->set("RequireStdCostForPOItem", _requirePoitemStdCost->isChecked());
  _metrics->set("CopyPRtoPOItem", _notes->isChecked());
  _metrics->set("POInternal", ((_internalCopy->isChecked()) ? _numOfCopies->value() : 0) );
  _metrics->set("DefaultPOShipVia", _defaultShipVia->text().trimmed());

  XSqlQuery configq;
  configq.prepare("SELECT setNextPoNumber(:poNumber), setNextVcNumber(:vcNumber), setNextPrNumber(:prNumber);");
  configq.bindValue(":poNumber", _nextPoNumber->text().toInt());
  configq.bindValue(":vcNumber", _nextVcNumber->text().toInt());
  configq.bindValue(":prNumber", _nextPrNumber->text().toInt());
  configq.exec();

  return true;
}

