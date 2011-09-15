/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspPoItemReceivingsByVendor.h"

#include <QAction>
#include <QMenu>
#include <QMessageBox>
#include <QSqlError>

#include "enterPoitemReceipt.h"
#include "guiclient.h"
#include "poLiabilityDistrib.h"
#include "voucher.h"

dspPoItemReceivingsByVendor::dspPoItemReceivingsByVendor(QWidget* parent, const char*, Qt::WFlags fl)
  : display(parent, "dspPoItemReceivingsByVendor", fl)
{
  setupUi(optionsWidget());
  setWindowTitle(tr("Receipts and Returns by Vendor"));
  setListLabel(tr("Purchase Order &Receipts and Returns"));
  setReportName("ReceiptsReturnsByVendor");
  setMetaSQLOptions("receivings", "detail");
  setUseAltId(true);

  connect(_showVariances, SIGNAL(toggled(bool)), this, SLOT(sHandleVariance(bool)));

  _agent->setType(XComboBox::Agent);
  _agent->setText(omfgThis->username());

  list()->addColumn(tr("P/O #"),      _orderColumn, Qt::AlignRight, true, "ponumber");
  list()->addColumn(tr("Sched. Date"), _dateColumn, Qt::AlignCenter,true, "duedate");
  list()->addColumn(tr("Recv. Date"),  _dateColumn, Qt::AlignCenter,true, "recvdate");
  list()->addColumn(tr("Item Number"), _itemColumn, Qt::AlignLeft,  true, "venditemnumber");
  list()->addColumn(tr("Description"), -1,          Qt::AlignLeft,  true, "venditemdescrip");
  list()->addColumn(tr("Rcvd/Rtnd"),   _qtyColumn,  Qt::AlignRight, true, "sense");
  list()->addColumn(tr("Qty."),        _qtyColumn,  Qt::AlignRight, true, "qty");
  list()->addColumn(tr("Invoiced"),     _ynColumn,  Qt::AlignRight, false,"invoiced");
  if (_privileges->check("ViewCosts"))
  {
    list()->addColumn(tr("Purch. Cost"),_priceColumn, Qt::AlignRight,true, "purchcost");
    list()->addColumn(tr("Recv. Cost"), _priceColumn, Qt::AlignRight,true, "recvcost");
    list()->addColumn(tr("Value"),      _priceColumn, Qt::AlignRight,true, "value");
  }

  _dates->setStartNull(tr("Earliest"), omfgThis->startOfTime(), true);
  _dates->setEndNull(tr("Latest"),     omfgThis->endOfTime(),   true);

  _showVariances->setEnabled(_privileges->check("ViewCosts"));
  sHandleVariance(_showVariances->isChecked());
}

void dspPoItemReceivingsByVendor::languageChange()
{
  display::languageChange();
  retranslateUi(this);
}

bool dspPoItemReceivingsByVendor::setParams(ParameterList &pParams)
{
  if (!_vendor->isValid() && _vendor->isVisible())
  {
    QMessageBox::warning( this, tr("Enter Vendor Number"),
                          tr( "Please enter a valid Vendor Number." ) );
    _vendor->setFocus();
    return false;
  }

  if (!_dates->allValid())
  {
    QMessageBox::warning( this, tr("Enter Valid Dates"),
                          tr( "Please enter a valid Start and End Date." ) );
    _dates->setFocus();
    return false;
  }

  _warehouse->appendValue(pParams);
  _dates->appendValue(pParams);
  pParams.append("vend_id", _vendor->id());

  if (_selectedPurchasingAgent->isChecked())
    pParams.append("agentUsername", _agent->currentText());

  if (_showVariances->isChecked())
    pParams.append("showVariances");

  if (_showUnvouchered->isChecked())
    pParams.append("showUnvouchered");

  pParams.append("received", tr("Received"));
  pParams.append("returned", tr("Returned"));
  pParams.append("unvouchered", tr("Not Vouchered"));
  pParams.append("nonInv",   tr("NonInv - "));
  pParams.append("na",       tr("N/A"));

  pParams.append("includeFormatted"); // report only?

  return true;
}

void dspPoItemReceivingsByVendor::sPopulateMenu(QMenu *menu, QTreeWidgetItem *, int)
{
  QAction *markInvoiced = 0;
  QAction *correctReceipt = 0;
  QAction *createVoucher = 0;
  switch (list()->altId())
  {
    case 1:     // id = recv_id
      markInvoiced   = new QAction(tr("Mark As Invoiced..."),  this);
      correctReceipt = new QAction(tr("Correct Receiving..."), this);

      markInvoiced->setEnabled(_privileges->check("MaintainUninvoicedReceipts")
                               && ! list()->currentItem()->rawValue("invoiced").isNull()
                               && ! list()->currentItem()->rawValue("invoiced").toBool());
      correctReceipt->setEnabled(_privileges->check("MaintainUninvoicedReceipts"));
      connect(markInvoiced, SIGNAL(triggered()), this, SLOT(sMarkAsInvoiced()));
      connect(correctReceipt, SIGNAL(triggered()), this, SLOT(sCorrectReceiving()));

      menu->addAction(markInvoiced);
      menu->addAction(correctReceipt);
      break;
    case 2:     // id = poreject_id
      break;
    case 3:     // id = unvouchered pohead_id
      createVoucher = new QAction(tr("Create Voucher..."),  this);
      createVoucher->setEnabled(_privileges->check("MaintainVouchers"));
      connect(createVoucher, SIGNAL(triggered()), this, SLOT(sCreateVoucher()));
      menu->addAction(createVoucher);
      break;
    default:
      break;
  }
}

void dspPoItemReceivingsByVendor::sHandleVariance(bool pShowVariances)
{
  if (pShowVariances)
  {
    list()->showColumn("purchcost");
    list()->showColumn("recvcost");
    list()->showColumn("value");
  }
  else
  {
    list()->hideColumn("purchcost");
    list()->hideColumn("recvcost");
    list()->hideColumn("value");
  }
}

void dspPoItemReceivingsByVendor::sCorrectReceiving()
{
  if (enterPoitemReceipt::correctReceipt(list()->id(), this) != XDialog::Rejected)
    sFillList();
}

void dspPoItemReceivingsByVendor::sCreateVoucher()
{
  ParameterList params;
  params.append("mode",      "new");
  params.append("pohead_id", list()->id());

  voucher *newdlg = new voucher();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspPoItemReceivingsByVendor::sMarkAsInvoiced()
{
  if (list()->currentItem()->rawValue("invoiced").toBool()
      || list()->currentItem()->rawValue("invoiced").isNull())
    return;

  bool update = true;
  
  if (recvHasValue())
  {
    ParameterList params;
    params.append("porecv_id", list()->id());  
    poLiabilityDistrib newdlg(this, "", true);
    newdlg.set(params);
    if (newdlg.exec() == XDialog::Rejected)
      update = false;
  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
  if (update)
  {
    q.prepare("UPDATE recv "
	      "SET recv_invoiced=true "
	      "WHERE (recv_id=:porecv_id); ");
    q.bindValue(":porecv_id",list()->id());
    q.exec();
    if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
    sFillList();
  }
}

bool dspPoItemReceivingsByVendor::recvHasValue()
{
  bool hasValue = false;

  if (_privileges->check("ViewCosts"))
    hasValue = list()->currentItem()->rawValue("value").toDouble() != 0;
  else
  {
    XSqlQuery valq;
    valq.prepare("SELECT * "
                 "FROM recv "
                 "WHERE ((recv_value <> 0)"
                 "   AND (recv_id=:recv_id));");
    valq.bindValue(":recv_id",list()->id());
    valq.exec();
    if (valq.first())
      hasValue = true;
    else if (valq.lastError().type() != QSqlError::NoError)
    {
      systemError(this, valq.lastError().databaseText(), __FILE__, __LINE__);
      return false;
    }
  }

  return hasValue;
}
