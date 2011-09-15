/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspPoItemReceivingsByDate.h"

#include <QMessageBox>
#include <QSqlError>

#include "guiclient.h"
#include "xtreewidget.h"

dspPoItemReceivingsByDate::dspPoItemReceivingsByDate(QWidget* parent, const char*, Qt::WFlags fl)
  : display(parent, "dspPoItemReceivingsByDate", fl)
{
  setupUi(optionsWidget());
  setWindowTitle(tr("Receipts and Returns by Date"));
  setListLabel(tr("Purchase Order &Receipts and Returns"));
  setReportName("ReceiptsReturnsByDate");
  setMetaSQLOptions("receivings", "detail");

  connect(_showVariances, SIGNAL(toggled(bool)), this, SLOT(sHandleVariance(bool)));

  _agent->setType(XComboBox::Agent);
  _agent->setText(omfgThis->username());

  list()->addColumn(tr("P/O #"),       _orderColumn, Qt::AlignRight, true, "ponumber");
  list()->addColumn(tr("Vendor"),      -1,           Qt::AlignLeft,  true, "vend_name");
  list()->addColumn(tr("Sched. Date"), _dateColumn,  Qt::AlignCenter,true, "duedate");
  list()->addColumn(tr("Recv. Date"),  _dateColumn,  Qt::AlignCenter,true, "recvdate");
  list()->addColumn(tr("Item Number"), _itemColumn,  Qt::AlignLeft,  true, "venditemnumber");
  list()->addColumn(tr("Rcvd/Rtnd"),   _qtyColumn,   Qt::AlignRight, true, "sense");
  list()->addColumn(tr("Qty."),        _qtyColumn,   Qt::AlignRight, true, "qty");
  if (_privileges->check("ViewCosts"))
  {
    list()->addColumn(tr("Purch. Cost"),_priceColumn, Qt::AlignRight,true, "purchcost");
    list()->addColumn(tr("Recv. Cost"), _priceColumn, Qt::AlignRight,true, "recvcost");
    list()->addColumn(tr("Value"),      _priceColumn, Qt::AlignRight,true, "value");
  }

  _showVariances->setEnabled(_privileges->check("ViewCosts"));

  _dates->setFocus();
  sHandleVariance(_showVariances->isChecked());
}

void dspPoItemReceivingsByDate::languageChange()
{
  display::languageChange();
  retranslateUi(this);
}

bool dspPoItemReceivingsByDate::setParams(ParameterList &pParams)
{
  if (!_dates->allValid())
  {
    QMessageBox::warning( this, tr("Enter Valid Dates"),
                          tr( "Please enter a valid Start and End Date." ) );
    _dates->setFocus();
    return false;
  }

  _warehouse->appendValue(pParams);
  _dates->appendValue(pParams);

  pParams.append("received", tr("Received"));
  pParams.append("returned", tr("Returned"));
  pParams.append("unvouchered", tr("Not Vouchered"));
  pParams.append("nonInv",   tr("NonInv - "));
  pParams.append("na",       tr("N/A"));

  if (_selectedPurchasingAgent->isChecked())
    pParams.append("agentUsername", _agent->currentText());

  if (_showVariances->isChecked())
    pParams.append("showVariances");

  if (_showUnvouchered->isChecked())
    pParams.append("showUnvouchered");

  pParams.append("includeFormatted"); // report only?

  return true;
}

void dspPoItemReceivingsByDate::sHandleVariance(bool pShowVariances)
{
  if (pShowVariances)
  {
    list()->showColumn(7);
    list()->showColumn(8);
    list()->showColumn(9);
  }
  else
  {
    list()->hideColumn(7);
    list()->hideColumn(8);
    list()->hideColumn(9);
  }
}

