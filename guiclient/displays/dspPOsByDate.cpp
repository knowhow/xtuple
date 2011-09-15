/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspPOsByDate.h"

#include <QMessageBox>

#include "guiclient.h"
#include "purchaseOrder.h"

dspPOsByDate::dspPOsByDate(QWidget* parent, const char*, Qt::WFlags fl)
  : display(parent, "dspPOsByDate", fl)
{
  setupUi(optionsWidget());
  setWindowTitle(tr("Purchase Orders by Date"));
  setListLabel(tr("Purchase Orders"));
  setReportName("POsByDate");
  setMetaSQLOptions("purchaseOrders", "detail");

  _dates->setStartNull(tr("Earliest"), omfgThis->startOfTime(), true);
  _dates->setStartCaption(tr("Starting Order Date:"));
  _dates->setEndNull(tr("Latest"), omfgThis->endOfTime(), true);
  _dates->setEndCaption(tr("Ending Order Date:"));

  _agent->setType(XComboBox::Agent);
  _agent->setText(omfgThis->username());

  list()->addColumn(tr("P/O #"),       _orderColumn, Qt::AlignRight, true, "pohead_number");
  list()->addColumn(tr("Site"),        _whsColumn,   Qt::AlignCenter,true, "warehousecode");
  list()->addColumn(tr("Status"),      _dateColumn,  Qt::AlignCenter,true, "poitem_status");
  list()->addColumn(tr("Vendor"),      _itemColumn,  Qt::AlignLeft,  true, "vend_name");
  list()->addColumn(tr("Order Date"),  _dateColumn,  Qt::AlignCenter,true, "pohead_orderdate");
  list()->addColumn(tr("Due Date"),    _dateColumn,  Qt::AlignCenter,true, "minDueDate");
}

void dspPOsByDate::languageChange()
{
  display::languageChange();
  retranslateUi(this);
}

bool dspPOsByDate::setParams(ParameterList &pParams)
{
  pParams.append("byDate");
  if (!_dates->startDate().isValid())
  {
    QMessageBox::warning( this, tr("Enter Start Date"),
                          tr( "Please enter a valid Start Date." ) );
    _dates->setFocus();
    return false;
  }

  if (!_dates->endDate().isValid())
  {
    QMessageBox::warning( this, tr("Enter End Date"),
                          tr( "Please eneter a valid End Date." ) );
    _dates->setFocus();
    return false;
  }

  _dates->appendValue(pParams);
  _warehouse->appendValue(pParams);

  if (_selectedPurchasingAgent->isChecked())
    pParams.append("agentUsername", _agent->currentText());

  if (_showClosed->isChecked())
    pParams.append("showClosed");

  pParams.append("closed",      tr("Closed"));
  pParams.append("unposted",    tr("Unposted"));
  pParams.append("partial",     tr("Partial"));
  pParams.append("received",    tr("Received"));
  pParams.append("open",        tr("Open"));

  return true;
}

void dspPOsByDate::sPopulateMenu(QMenu *pMenu, QTreeWidgetItem *pSelected, int)
{
  QAction *menuItem;

  if (dynamic_cast<XTreeWidgetItem*>(pSelected) &&
      dynamic_cast<XTreeWidgetItem*>(pSelected)->rawValue("poitem_status").toString() == "U")
  {
    menuItem = pMenu->addAction(tr("Edit Order..."), this, SLOT(sEditOrder()));
    menuItem->setEnabled(_privileges->check("MaintainPurchaseOrders"));
  }

  menuItem = pMenu->addAction(tr("View Order..."), this, SLOT(sViewOrder()));
  menuItem->setEnabled(_privileges->check("MaintainPurchaseOrders") ||
                       _privileges->check("ViewPurchaseOrders"));
}

void dspPOsByDate::sEditOrder()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("pohead_id", list()->id());

  purchaseOrder *newdlg = new purchaseOrder();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspPOsByDate::sViewOrder()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("pohead_id", list()->id());

  purchaseOrder *newdlg = new purchaseOrder();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

