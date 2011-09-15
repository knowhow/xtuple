/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspPoItemsByDate.h"

#include <QAction>
#include <QMenu>
#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include <metasql.h>
#include <mqlutil.h>
#include <openreports.h>

#include "changePoitemQty.h"
#include "dspRunningAvailability.h"
#include "purchaseOrder.h"
#include "purchaseOrderItem.h"
#include "reschedulePoitem.h"

dspPoItemsByDate::dspPoItemsByDate(QWidget* parent, const char*, Qt::WFlags fl)
  : display(parent, "dspPoItemsByDate", fl)
{
  setupUi(optionsWidget());
  setWindowTitle(tr("Purchase Order Items by Date"));
  setListLabel(tr("Purchase Order Items"));
  setReportName("POLineItemsByDate");
  setMetaSQLOptions("poItems", "detail");
  setUseAltId(true);

  connect(_selectedPurchasingAgent, SIGNAL(toggled(bool)), _agent, SLOT(setEnabled(bool)));

  _dates->setStartNull(tr("Earliest"), omfgThis->startOfTime(), true);
  _dates->setStartCaption(tr("Starting Due Date:"));
  _dates->setEndNull(tr("Latest"), omfgThis->endOfTime(), true);
  _dates->setEndCaption(tr("Ending Due Date:"));

  _agent->setText(omfgThis->username());

  list()->addColumn(tr("P/O #"),       _orderColumn, Qt::AlignRight,  true,  "pohead_number"  );
  list()->addColumn(tr("Site"),        _whsColumn,   Qt::AlignCenter, true,  "warehous_code" );
  list()->addColumn(tr("Status"),      0,            Qt::AlignCenter, true,  "poitem_status" );
  list()->addColumn(tr("Status"),      _dateColumn,  Qt::AlignCenter, true,  "f_poitem_status" );
  list()->addColumn(tr("Vendor"),      _itemColumn,  Qt::AlignLeft,   true,  "vend_name"   );
  list()->addColumn(tr("Due Date"),    _dateColumn,  Qt::AlignCenter, true,  "poitem_duedate" );
  list()->addColumn(tr("Item Number"), _itemColumn,  Qt::AlignLeft,   true,  "itemnumber"   );
  list()->addColumn(tr("Description"), _itemColumn,  Qt::AlignLeft,   true,  "itemdescrip"   );
  list()->addColumn(tr("Vend. Item #"), _itemColumn, Qt::AlignLeft,   true,  "poitem_vend_item_number");
  list()->addColumn(tr("UOM"),         _uomColumn,   Qt::AlignCenter, true,  "itemuom" );
  list()->addColumn(tr("Vend. UOM"),   _uomColumn,   Qt::AlignCenter, true,  "poitem_vend_uom" );
  list()->addColumn(tr("Ordered"),     _qtyColumn,   Qt::AlignRight,  true,  "poitem_qty_ordered"  );
  list()->addColumn(tr("Received"),    _qtyColumn,   Qt::AlignRight,  true,  "poitem_qty_received"  );
  list()->addColumn(tr("Returned"),    _qtyColumn,   Qt::AlignRight,  true,  "poitem_qty_returned"  );
}

void dspPoItemsByDate::languageChange()
{
  display::languageChange();
  retranslateUi(this);
}

bool dspPoItemsByDate::setParams(ParameterList &params)
{
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

  params.append("byDate");

  _warehouse->appendValue(params);
  _dates->appendValue(params);

  if (_selectedPurchasingAgent->isChecked())
    params.append("agentUsername", _agent->currentText());

  if (_openItems->isChecked())
    params.append("openItems");
  else if (_closedItems->isChecked())
    params.append("closedItems");

  params.append("nonInv",	tr("NonInv - "));
  params.append("closed",	tr("Closed"));
  params.append("unposted",	tr("Unposted"));
  params.append("partial",	tr("Partial"));
  params.append("received",	tr("Received"));
  params.append("open",		tr("Open"));

  return true;
}

void dspPoItemsByDate::sPopulateMenu(QMenu *pMenu, QTreeWidgetItem *pSelected, int)
{
  QAction *menuItem;
  XTreeWidgetItem *item = dynamic_cast<XTreeWidgetItem*>(pSelected);

  if (item && item->rawValue("poitem_status") == "U")
  {
    menuItem = pMenu->addAction(tr("Edit Order..."), this, SLOT(sEditOrder()));
    menuItem->setEnabled(_privileges->check("MaintainPurchaseOrders"));
  }

  menuItem = pMenu->addAction(tr("View Order..."), this, SLOT(sViewOrder()));
  menuItem->setEnabled(_privileges->check("MaintainPurchaseOrders") ||
                       _privileges->check("ViewPurchaseOrders"));

  menuItem = pMenu->addAction(tr("Running Availability..."), this, SLOT(sRunningAvailability()));

  menuItem->setEnabled(_privileges->check("ViewInventoryAvailability"));

  pMenu->addSeparator();

  if (item && item->rawValue("poitem_status") == "U")
  {
    menuItem = pMenu->addAction(tr("Edit Item..."), this, SLOT(sEditItem()));
    menuItem->setEnabled(_privileges->check("MaintainPurchaseOrders"));
  }

  menuItem = pMenu->addAction(tr("View Item..."), this, SLOT(sViewItem()));
  menuItem->setEnabled(_privileges->check("MaintainPurchaseOrders") ||
                       _privileges->check("ViewPurchaseOrders"));

  if (item && item->rawValue("poitem_status") != "C")
  {
    menuItem = pMenu->addAction(tr("Reschedule..."), this, SLOT(sReschedule()));
    menuItem->setEnabled(_privileges->check("ReschedulePurchaseOrders"));

    menuItem = pMenu->addAction(tr("Change Qty..."), this, SLOT(sChangeQty()));
    menuItem->setEnabled(_privileges->check("ChangePurchaseOrderQty"));

    pMenu->addSeparator();
  }

  if (item && item->rawValue("poitem_status") == "O")
  {
    menuItem = pMenu->addAction(tr("Close Item..."), this, SLOT(sCloseItem()));
    menuItem->setEnabled(_privileges->check("MaintainPurchaseOrders"));
  }
  else if (item && item->rawValue("poitem_status") == "C")
  {
    menuItem = pMenu->addAction(tr("Open Item..."), this, SLOT(sOpenItem()));
    menuItem->setEnabled(_privileges->check("MaintainPurchaseOrders"));
  }
}

void dspPoItemsByDate::sRunningAvailability()
{
  XSqlQuery availq;
  availq.prepare("SELECT poitem_itemsite_id"
                 "  FROM poitem"
                 " WHERE (poitem_id=:poitemid); ");
  availq.bindValue(":poitemid", list()->altId());
  availq.exec();
  if (availq.first())
  {
    ParameterList params;
    params.append("itemsite_id", availq.value("poitem_itemsite_id").toInt());
    params.append("run");

    dspRunningAvailability *newdlg = new dspRunningAvailability();
    newdlg->set(params);
    omfgThis->handleNewWindow(newdlg);
  }
  else if (availq.lastError().type() != QSqlError::NoError)
  {
    systemError(this, availq.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void dspPoItemsByDate::sEditOrder()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("pohead_id", list()->id());

  purchaseOrder *newdlg = new purchaseOrder();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspPoItemsByDate::sViewOrder()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("pohead_id", list()->id());

  purchaseOrder *newdlg = new purchaseOrder();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspPoItemsByDate::sEditItem()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("poitem_id", list()->altId());

  purchaseOrderItem newdlg(this, "", true);
  newdlg.set(params);
  newdlg.exec();
}

void dspPoItemsByDate::sViewItem()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("poitem_id", list()->altId());

  purchaseOrderItem newdlg(this, "", true);
  newdlg.set(params);
  newdlg.exec();
}

void dspPoItemsByDate::sReschedule()
{
  ParameterList params;
  params.append("poitem_id", list()->altId());

  reschedulePoitem newdlg(this, "", true);
  if(newdlg.set(params) != UndefinedError)
    if (newdlg.exec() != XDialog::Rejected)
      sFillList();
}

void dspPoItemsByDate::sChangeQty()
{
  ParameterList params;
  params.append("poitem_id", list()->altId());

  changePoitemQty newdlg(this, "", true);
  newdlg.set(params);
  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void dspPoItemsByDate::sCloseItem()
{
  XSqlQuery closeq;
  closeq.prepare("UPDATE poitem"
                 "   SET poitem_status='C' "
                 " WHERE (poitem_id=:poitem_id);" );
  closeq.bindValue(":poitem_id", list()->altId());
  closeq.exec();
  if (closeq.lastError().type() != QSqlError::NoError)
  {
    systemError(this, closeq.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
  sFillList();
}

void dspPoItemsByDate::sOpenItem()
{
  XSqlQuery openq;
  openq.prepare("UPDATE poitem"
                "  SET poitem_status='O'"
                " WHERE (poitem_id=:poitem_id);" );
  openq.bindValue(":poitem_id", list()->altId());
  openq.exec();
  if (openq.lastError().type() != QSqlError::NoError)
  {
    systemError(this, openq.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
  sFillList();
}

