/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspPurchaseReqsByItem.h"

#include <QAction>
#include <QMenu>
#include <QVariant>
#include <QMessageBox>

#include "guiclient.h"

#include "dspRunningAvailability.h"
#include "purchaseOrder.h"

dspPurchaseReqsByItem::dspPurchaseReqsByItem(QWidget* parent, const char*, Qt::WFlags fl)
  : display(parent, "dspPurchaseReqsByItem", fl)
{
  setupUi(optionsWidget());
  setWindowTitle(tr("Purchase Requests by Item"));
  setListLabel(tr("Purchase Requests"));
  setReportName("PurchaseRequestsByItem");
  setMetaSQLOptions("purchase", "purchaserequests");
  setUseAltId(true);

  list()->addColumn(tr("P/R #"),        _orderColumn,  Qt::AlignLeft,   true,  "pr_number");
  list()->addColumn(tr("Sub #"),        _orderColumn,  Qt::AlignLeft,   true,  "pr_subnumber");
  list()->addColumn(tr("Status"),       _statusColumn, Qt::AlignCenter, true,  "pr_status" );
  list()->addColumn(tr("Parent Order"), -1,            Qt::AlignLeft,   true,  "parent"   );
  list()->addColumn(tr("Create Date"),        -1,    Qt::AlignLeft,  true,  "pr_createdate"  );
  list()->addColumn(tr("Due Date"),     _dateColumn,   Qt::AlignCenter, true,  "pr_duedate" );
  list()->addColumn(tr("Qty."),         _qtyColumn,    Qt::AlignRight,  true,  "pr_qtyreq"  );
  list()->addColumn(tr("Notes"),        -1,    Qt::AlignLeft,  true,  "pr_releasenote"  );

  connect(omfgThis, SIGNAL(purchaseRequestsUpdated()), this, SLOT(sFillList()));
}

void dspPurchaseReqsByItem::languageChange()
{
  display::languageChange();
  retranslateUi(this);
}

bool dspPurchaseReqsByItem::setParams(ParameterList &params)
{
  if (! _item->isValid())
  {
    QMessageBox::information(this, tr("Item Required"),
                             tr("<p>Item is required."));
    _item->setFocus();
    return false;
  }

  _warehouse->appendValue(params);
  params.append("item_id", _item->id());
  params.append("manual", tr("Manual"));
  params.append("other",  tr("Other"));

  return true;
}

void dspPurchaseReqsByItem::sPopulateMenu(QMenu *pMenu, QTreeWidgetItem *, int)
{
  QAction *menuItem;

  menuItem = pMenu->addAction(tr("Running Availability..."), this, SLOT(sDspRunningAvailability()));
  menuItem->setEnabled(_privileges->check("ViewInventoryAvailability"));

  pMenu->addSeparator();

  menuItem = pMenu->addAction(tr("Release P/R..."), this, SLOT(sRelease()));
  menuItem->setEnabled(_privileges->check("MaintainPurchaseOrders"));

  menuItem = pMenu->addAction(tr("Delete P/R..."), this, SLOT(sDelete()));
  menuItem->setEnabled(_privileges->check("MaintainPurchaseRequests"));
}

void dspPurchaseReqsByItem::sDspRunningAvailability()
{
  ParameterList params;
  params.append("itemsite_id", list()->altId());
  params.append("run");

  dspRunningAvailability *newdlg = new dspRunningAvailability();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspPurchaseReqsByItem::sRelease()
{
  ParameterList params;
  params.append("mode", "releasePr");
  params.append("pr_id", list()->id());

  purchaseOrder *newdlg = new purchaseOrder();
  if(newdlg->set(params) == NoError)
    omfgThis->handleNewWindow(newdlg);
  else
    delete newdlg;

  sFillList();
  omfgThis->sPurchaseRequestsUpdated();
}

void dspPurchaseReqsByItem::sDelete()
{
  q.prepare("SELECT deletePr(:pr_id) AS _result;");
  q.bindValue(":pr_id", list()->id());
  q.exec();

  sFillList();
  omfgThis->sPurchaseRequestsUpdated();
}

