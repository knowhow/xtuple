/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1799-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspOrderActivityByProject.h"

#include <QAction>
#include <QMenu>
#include <QMessageBox>
#include <QVariant>

#include "guiclient.h"
#include "salesOrder.h"
#include "salesOrderItem.h"
#include "invoice.h"
#include "invoiceItem.h"
#include "workOrder.h"
#include "purchaseRequest.h"
#include "purchaseOrder.h"
#include "purchaseOrderItem.h"

dspOrderActivityByProject::dspOrderActivityByProject(QWidget* parent, const char*, Qt::WFlags fl)
  : display(parent, "dspOrderActivityByProject", fl)
{
  setupUi(optionsWidget());
  setWindowTitle(tr("Order Activity by Project"));
  setListLabel(tr("Orders"));
  setReportName("OrderActivityByProject");
  setMetaSQLOptions("orderActivityByProject", "detail");
  setUseAltId(true);

  _run = false;

  list()->addColumn(tr("Name"),        _itemColumn,  Qt::AlignLeft,   true,  "name"   );
  list()->addColumn(tr("Status"),      _orderColumn, Qt::AlignLeft,   true,  "status"   );
  list()->addColumn(tr("Item #"),      _itemColumn,  Qt::AlignLeft,   true,  "item"   );
  list()->addColumn(tr("Description"), -1          , Qt::AlignLeft,   true,  "descrip" );
  list()->addColumn(tr("Qty"),         _qtyColumn,   Qt::AlignRight,  true,  "qty"  );
  list()->addColumn(tr("UOM"),         _uomColumn,   Qt::AlignLeft,   true,  "uom"  );
  list()->addColumn(tr("Value"),      _qtyColumn,   Qt::AlignRight,  true,  "value"  );

  list()->setPopulateLinear(true);
}

void dspOrderActivityByProject::languageChange()
{
  display::languageChange();
  retranslateUi(this);
}

enum SetResponse dspOrderActivityByProject::set(const ParameterList &pParams)
{
  XWidget::set(pParams);

  QVariant param;
  bool     valid;

  param = pParams.value("prj_id", &valid);
  if (valid)
    _project->setId(param.toInt());

  if (pParams.inList("run"))
  {
    _run = true;
    return NoError_Run;
  }

  return NoError;
}

void dspOrderActivityByProject::sPopulateMenu(QMenu * pMenu, QTreeWidgetItem*, int)
{
  QAction *menuItem;

  if(list()->altId() == 15)
  {
    menuItem = pMenu->addAction(tr("Edit Quote..."), this, SLOT(sEdit()));
    menuItem->setEnabled(_privileges->check("MaintainQuotes"));

    menuItem = pMenu->addAction(tr("View Quote..."), this, SLOT(sView()));
    menuItem->setEnabled(_privileges->check("MaintainQuotes") ||
                         _privileges->check("ViewQuotes"));
  }

  if(list()->altId() == 17)
  {
    menuItem = pMenu->addAction(tr("Edit Quote Item..."), this, SLOT(sEdit()));
    menuItem->setEnabled(_privileges->check("MaintainQuotes"));

    menuItem = pMenu->addAction(tr("View Quote Item..."), this, SLOT(sView()));
    menuItem->setEnabled(_privileges->check("MaintainQuotes") ||
                         _privileges->check("ViewQuotes"));
  }

  if(list()->altId() == 25)
  {
    menuItem = pMenu->addAction(tr("Edit Sales Order..."), this, SLOT(sEdit()));
    menuItem->setEnabled(_privileges->check("MaintainSalesOrders"));

    menuItem = pMenu->addAction(tr("View Sales Order..."), this, SLOT(sView()));
    menuItem->setEnabled(_privileges->check("MaintainSalesOrders") ||
                         _privileges->check("ViewSalesOrders"));
  }

  if(list()->altId() == 27)
  {
    menuItem = pMenu->addAction(tr("Edit Sales Order Item..."), this, SLOT(sEdit()));
    menuItem->setEnabled(_privileges->check("MaintainSalesOrders"));

    menuItem = pMenu->addAction(tr("View Sales Order Item..."), this, SLOT(sView()));
    menuItem->setEnabled(_privileges->check("MaintainSalesOrders") ||
                         _privileges->check("ViewSalesOrders"));
  }

  if(list()->altId() == 35)
  {
    menuItem = pMenu->addAction(tr("Edit Invoice..."), this, SLOT(sEdit()));
    menuItem->setEnabled(_privileges->check("MaintainMiscInvoices"));

    menuItem = pMenu->addAction(tr("View Invoice..."), this, SLOT(sView()));
    menuItem->setEnabled(_privileges->check("MaintainMiscInvoices") ||
                         _privileges->check("ViewMiscInvoices"));
  }

  if(list()->altId() == 37)
  {
    menuItem = pMenu->addAction(tr("Edit Invoice Item..."), this, SLOT(sEdit()));
    menuItem->setEnabled(_privileges->check("MaintainMiscInvoices"));

    menuItem = pMenu->addAction(tr("View Invoice Item..."), this, SLOT(sView()));
    menuItem->setEnabled(_privileges->check("MaintainMiscInvoices") ||
                         _privileges->check("ViewMiscInvoices"));
  }

  if(list()->altId() == 45)
  {
    menuItem = pMenu->addAction(tr("Edit Work Order..."), this, SLOT(sEdit()));
    menuItem->setEnabled(_privileges->check("MaintainWorkOrders"));

    menuItem = pMenu->addAction(tr("View Work Order..."), this, SLOT(sView()));
    menuItem->setEnabled(_privileges->check("MaintainWorkOrders") ||
                         _privileges->check("ViewWorkOrders"));
  }

  if(list()->altId() == 55)
  {
    menuItem = pMenu->addAction(tr("View Purchase Request..."), this, SLOT(sView()));
    menuItem->setEnabled(_privileges->check("MaintainPurchaseRequests") ||
                         _privileges->check("ViewPurchaseRequests"));
  }

  if(list()->altId() == 65)
  {
    menuItem = pMenu->addAction(tr("Edit Purchase Order..."), this, SLOT(sEdit()));
    menuItem->setEnabled(_privileges->check("MaintainPurchaseOrders"));

    menuItem = pMenu->addAction(tr("View Purchase Order..."), this, SLOT(sView()));
    menuItem->setEnabled(_privileges->check("MaintainPurchaseOrders") ||
                         _privileges->check("ViewPurchaseOrders"));
  }

  if(list()->altId() == 67)
  {
    menuItem = pMenu->addAction(tr("Edit Purchase Order Item..."), this, SLOT(sEdit()));
    menuItem->setEnabled(_privileges->check("MaintainPurchaseOrders"));

    menuItem = pMenu->addAction(tr("View Purchase Order Item..."), this, SLOT(sView()));
    menuItem->setEnabled(_privileges->check("MaintainPurchaseOrders") ||
                         _privileges->check("ViewPurchaseOrders"));
  }

}

void dspOrderActivityByProject::sEdit()
{
  ParameterList params;

  if(list()->altId() == 15)
  {
    params.append("mode", "editQuote");
    params.append("quhead_id", list()->id());

    salesOrder *newdlg = new salesOrder(this);
    newdlg->set(params);
    omfgThis->handleNewWindow(newdlg);
  }
  else if(list()->altId() == 17)
  {
    params.append("mode", "editQuote");
    params.append("soitem_id", list()->id());

    salesOrderItem newdlg(this);
    newdlg.set(params);
    newdlg.exec();
  }
  else if(list()->altId() == 25)
  {
    salesOrder::editSalesOrder(list()->id(), false, this);
  }
  else if(list()->altId() == 27)
  {
    params.append("mode", "edit");
    params.append("soitem_id", list()->id());

    salesOrderItem newdlg(this);
    newdlg.set(params);
    newdlg.exec();
  }
  else if(list()->altId() == 35)
  {
    invoice::editInvoice(list()->id(), this);
  }
  else if(list()->altId() == 37)
  {
    params.append("mode", "edit");
    params.append("invcitem_id", list()->id());

    invoiceItem newdlg(this);
    newdlg.set(params);
    newdlg.exec();
  }
  else if(list()->altId() == 45)
  {
    params.append("mode", "edit");
    params.append("wo_id", list()->id());

    workOrder *newdlg = new workOrder(this);
    newdlg->set(params);
    omfgThis->handleNewWindow(newdlg);
  }
  else if(list()->altId() == 65)
  {
    params.append("mode", "edit");
    params.append("pohead_id", list()->id());

    purchaseOrder *newdlg = new purchaseOrder(this);
    newdlg->set(params);
    omfgThis->handleNewWindow(newdlg);
  }
  else if(list()->altId() == 67)
  {
    ParameterList params;
    params.append("mode", "edit");
    params.append("poitem_id", list()->id());

    purchaseOrderItem newdlg(this, "", true);
    newdlg.set(params);
    newdlg.exec();
  }
}

void dspOrderActivityByProject::sView()
{
  ParameterList params;

  if(list()->altId() == 15)
  {
    params.append("mode", "viewQuote");
    params.append("quhead_id", list()->id());

    salesOrder *newdlg = new salesOrder(this);
    newdlg->set(params);
    omfgThis->handleNewWindow(newdlg);
  }
  else if(list()->altId() == 17)
  {
    params.append("mode", "viewQuote");
    params.append("soitem_id", list()->id());

    salesOrderItem newdlg(this);
    newdlg.set(params);
    newdlg.exec();
  }
  else if(list()->altId() == 25)
  {
    salesOrder::viewSalesOrder(list()->id(), false);
  }
  else if(list()->altId() == 27)
  {
    params.append("mode", "view");
    params.append("soitem_id", list()->id());

    salesOrderItem newdlg(this);
    newdlg.set(params);
    newdlg.exec();
  }
  else if(list()->altId() == 35)
  {
    invoice::viewInvoice(list()->id(), this);
  }
  else if(list()->altId() == 37)
  {
    params.append("mode", "view");
    params.append("invcitem_id", list()->id());

    invoiceItem newdlg(this);
    newdlg.set(params);
    newdlg.exec();
  }
  else if(list()->altId() == 45)
  {
    params.append("mode", "view");
    params.append("wo_id", list()->id());

    workOrder *newdlg = new workOrder(this);
    newdlg->set(params);
    omfgThis->handleNewWindow(newdlg);
  }
  else if(list()->altId() == 55)
  {
    params.append("mode", "view");
    params.append("pr_id", list()->id());

    purchaseRequest newdlg(this, "", true);
    newdlg.set(params);
    newdlg.exec();
  }
  else if(list()->altId() == 65)
  {
    params.append("mode", "view");
    params.append("pohead_id", list()->id());

    purchaseOrder *newdlg = new purchaseOrder(this);
    newdlg->set(params);
    omfgThis->handleNewWindow(newdlg);
  }
  else if(list()->altId() == 67)
  {
    ParameterList params;
    params.append("mode", "view");
    params.append("poitem_id", list()->id());

    purchaseOrderItem newdlg(this, "", true);
    newdlg.set(params);
    newdlg.exec();
  }
}

bool dspOrderActivityByProject::setParams(ParameterList &params)
{
  if(_project->id() == -1)
  {
    QMessageBox::warning(this, tr("Project Required"),
      tr("You must specify a Project."));
    return false;
  }

  if (!_showWo->isChecked() &&
      !_showPo->isChecked() &&
      !_showSo->isChecked())
    return false;

  params.append("prj_id", _project->id());
  
  params.append("so", tr("Sales Order"));
  params.append("wo", tr("Work Order"));
  params.append("po", tr("Purchase Order"));
  params.append("pr", tr("Purchase Request"));
  params.append("sos", tr("Sales Orders"));
  params.append("wos", tr("Work Orders"));
  params.append("pos", tr("Purchase Orders"));
  params.append("prs", tr("Purchase Requests"));
  params.append("quote", tr("Quote"));
  params.append("quotes", tr("Quotes"));
  params.append("invoice", tr("Invoice"));
  params.append("invoices", tr("Invoices"));

  params.append("open", tr("Open"));
  params.append("closed", tr("Closed"));
  params.append("converted", tr("Converted"));
  params.append("canceled", tr("Canceled"));
  params.append("expired", tr("Expired"));
  params.append("unposted", tr("Unposted"));
  params.append("posted", tr("Posted"));
  params.append("exploded", tr("Exploded"));
  params.append("released", tr("Released"));
  params.append("inprocess", tr("In Process"));
  params.append("unreleased", tr("Unreleased"));
  params.append("total", tr("Total"));

  if(_showSo->isChecked())
    params.append("showSo");

  if(_showWo->isChecked())
    params.append("showWo");

  if(_showPo->isChecked())
    params.append("showPo");

  return true;
}

void dspOrderActivityByProject::sFillList()
{
  display::sFillList();
  list()->expandAll();
}

void dspOrderActivityByProject::showEvent(QShowEvent *event)
{
  display::showEvent(event);

  if (_run)
    sFillList();
}

