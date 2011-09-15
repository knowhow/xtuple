/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspInventoryAvailabilityByWorkOrder.h"

#include <QAction>
#include <QMenu>
#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include "createCountTagsByItem.h"
#include "dspAllocations.h"
#include "dspInventoryHistory.h"
#include "dspOrders.h"
#include "dspRunningAvailability.h"
#include "dspSubstituteAvailabilityByItem.h"
#include "enterMiscCount.h"
#include "postMiscProduction.h"
#include "inputManager.h"
#include "purchaseOrder.h"
#include "purchaseRequest.h"
#include "workOrder.h"

dspInventoryAvailabilityByWorkOrder::dspInventoryAvailabilityByWorkOrder(QWidget* parent, const char*, Qt::WFlags fl)
  : display(parent, "dspInventoryAvailabilityByWorkOrder", fl)
{
  setupUi(optionsWidget());
  setWindowTitle(tr("Inventory Availability by Work Order"));
  setListLabel(tr("Work Order Material Availability"));
  setReportName("WOMaterialAvailabilityByWorkOrder");
  setMetaSQLOptions("inventoryAvailabilitybyWorkorder", "detail");
  setUseAltId(true);

  _wo->setType(cWoExploded | cWoIssued | cWoReleased);

  omfgThis->inputManager()->notify(cBCWorkOrder, this, _wo, SLOT(setId(int)));

  list()->addColumn(tr("WO/Item#"),    120, Qt::AlignLeft,  true, "woinvav_item_wo_number");
  list()->addColumn(tr("Description"),         -1, Qt::AlignLeft,  true, "woinvav_descrip");
  list()->addColumn(tr("UOM"),         _uomColumn, Qt::AlignCenter,true, "woinvav_uomname");
  list()->addColumn(tr("QOH"),         _qtyColumn, Qt::AlignRight, true, "woinvav_qoh");
  list()->addColumn(tr("This Alloc."), _qtyColumn, Qt::AlignRight, true, "woinvav_balance");
  list()->addColumn(tr("Total Alloc."),_qtyColumn, Qt::AlignRight, true, "woinvav_allocated");
  list()->addColumn(tr("Orders"),      _qtyColumn, Qt::AlignRight, true, "woinvav_ordered");
  list()->addColumn(tr("This Avail."), _qtyColumn, Qt::AlignRight, true, "woinvav_woavail");
  list()->addColumn(tr("Total Avail."),_qtyColumn, Qt::AlignRight, true, "woinvav_totalavail");
  list()->addColumn(tr("Type"),                 0, Qt::AlignLeft, false, "woinvav_type");

  connect(omfgThis, SIGNAL(workOrdersUpdated(int, bool)), this, SLOT(sFillList()));
}

void dspInventoryAvailabilityByWorkOrder::languageChange()
{
  display::languageChange();
  retranslateUi(this);
}

enum SetResponse dspInventoryAvailabilityByWorkOrder::set(const ParameterList &pParams)
{
  XWidget::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("wo_id", &valid);
  if (valid)
    _wo->setId(param.toInt());

  _onlyShowShortages->setChecked(pParams.inList("onlyShowShortages"));

  _onlyShowInsufficientInventory->setChecked(pParams.inList("onlyShowInsufficientInventory"));

  if (pParams.inList("run"))
  {
    sFillList();
    return NoError_Run;
  }

  return NoError;
}

bool dspInventoryAvailabilityByWorkOrder::setParams(ParameterList &params)
{
  if(!_wo->isValid())
  {
    QMessageBox::warning(this, tr("Invalid W/O Selected"),
                         tr("<p>You must specify a valid Work Order Number.") );
    _wo->setFocus();
    return false;
  }

  params.append("wo_id", _wo->id());

  if(_onlyShowShortages->isChecked())
    params.append("onlyShowShortages");

  if(_onlyShowInsufficientInventory->isChecked())
    params.append("onlyShowInsufficientInventory");

  if(_sumParentChild->isChecked())
      params.append("summarizedParentChild");

  if(_indentedWo->isChecked())
      params.append("IndentedParentChild");
  
  return true;
}

void dspInventoryAvailabilityByWorkOrder::sPopulateMenu(QMenu *pMenu, QTreeWidgetItem *selected, int)
{
      XTreeWidgetItem * item = (XTreeWidgetItem*)selected;
      QAction *menuItem;

      menuItem = pMenu->addAction(tr("View Inventory History..."), this, SLOT(sViewHistory()));
      if (!_privileges->check("ViewInventoryHistory"))
        menuItem->setEnabled(false);

      pMenu->addSeparator();

      menuItem = pMenu->addAction("View Allocations...", this, SLOT(sViewAllocations()));
      if (item->rawValue("woinvav_allocated").toDouble() == 0.0)
        menuItem->setEnabled(false);

      menuItem = pMenu->addAction("View Orders...", this, SLOT(sViewOrders()));
      if (item->rawValue("woinvav_ordered").toDouble() == 0.0)
        menuItem->setEnabled(false);

      menuItem = pMenu->addAction("Running Availability...", this, SLOT(sRunningAvailability()));

      pMenu->addSeparator();
      
	  q.prepare( "SELECT itemsite_posupply as result "
				 "FROM itemsite "
				 "WHERE (itemsite_id=:womatl_id);" );
	  q.bindValue(":womatl_id", list()->id());
	  q.exec();
	  if (q.first())
	  {
		  if (q.value("result").toBool())
		  {
			menuItem = pMenu->addAction(tr("Create P/R..."), this, SLOT(sCreatePR()));
			if (!_privileges->check("MaintainPurchaseRequests"))
			  menuItem->setEnabled(false);

			menuItem = pMenu->addAction("Create P/O...", this, SLOT(sCreatePO()));
			if (!_privileges->check("MaintainPurchaseOrders"))
			  menuItem->setEnabled(false);

			pMenu->addSeparator();
		  }
	  }
	  q.prepare( "SELECT itemsite_wosupply as result "
				 "FROM itemsite "
				 "WHERE (itemsite_id=:womatl_id);" );
	  q.bindValue(":womatl_id", list()->id());
	  q.exec();
	  if (q.first())
	  {
		  if (q.value("result").toBool())
		  {
			if(list()->altId() != -1)
			{
			  menuItem = pMenu->addAction("Create W/O...", this, SLOT(sCreateWO()));
			  if (!_privileges->check("MaintainWorkOrders"))
				menuItem->setEnabled(false);
			}
			menuItem = pMenu->addAction(tr("Post Misc. Production..."), this, SLOT(sPostMiscProduction()));
			if (!_privileges->check("PostMiscProduction"))
			  menuItem->setEnabled(false);

			pMenu->addSeparator();
		  }
	  }
//      }

      menuItem = pMenu->addAction("View Substitute Availability...", this, SLOT(sViewSubstituteAvailability()));

      pMenu->addSeparator();

      menuItem = pMenu->addAction("Issue Count Tag...", this, SLOT(sIssueCountTag()));
      if (!_privileges->check("IssueCountTags"))
        menuItem->setEnabled(false);

      menuItem = pMenu->addAction(tr("Enter Misc. Inventory Count..."), this, SLOT(sEnterMiscCount()));
      if (!_privileges->check("EnterMiscCounts"))
        menuItem->setEnabled(false);

}

void dspInventoryAvailabilityByWorkOrder::sViewHistory()
{
  ParameterList params;
  params.append("itemsite_id", list()->id());

  dspInventoryHistory *newdlg = new dspInventoryHistory();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspInventoryAvailabilityByWorkOrder::sViewAllocations()
{
  q.prepare( "SELECT womatl_duedate "
             "FROM womatl "
             "WHERE (womatl_itemsite_id=:womatl_itemsite_id);" );
  q.bindValue(":womatl_itemsite_id", list()->id());
  q.exec();
  if (q.first())
  {
    ParameterList params;
    params.append("itemsite_id", list()->id());
    params.append("byDate", q.value("womatl_duedate"));
    params.append("run");

    dspAllocations *newdlg = new dspAllocations();
    newdlg->set(params);
    omfgThis->handleNewWindow(newdlg);
  }
}

void dspInventoryAvailabilityByWorkOrder::sViewOrders()
{
  q.prepare( "SELECT womatl_duedate "
             "FROM womatl "
             "WHERE (womatl_itemsite_id=:womatl_itemsite_id);" );
  q.bindValue(":womatl_itemsite_id", list()->id());
  q.exec();
  if (q.first())
  {
    ParameterList params;
    params.append("itemsite_id", list()->id());
    params.append("byDate", q.value("womatl_duedate"));
    params.append("run");

    dspOrders *newdlg = new dspOrders();
    newdlg->set(params);
    omfgThis->handleNewWindow(newdlg);
  }
}

void dspInventoryAvailabilityByWorkOrder::sRunningAvailability()
{
  ParameterList params;
  params.append("itemsite_id", list()->id());
  params.append("run");

  dspRunningAvailability *newdlg = new dspRunningAvailability();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspInventoryAvailabilityByWorkOrder::sViewSubstituteAvailability()
{
  q.prepare( "SELECT womatl_duedate "
             "FROM womatl "
             "WHERE (womatl_itemsite_id=:womatl_itemsite_id);" );
  q.bindValue(":womatl_itemsite_id", list()->id());
  q.exec();
  if (q.first())
  {
    ParameterList params;
    params.append("itemsite_id", list()->id());
    params.append("byDate", q.value("womatl_duedate"));
    params.append("run");

    dspSubstituteAvailabilityByItem *newdlg = new dspSubstituteAvailabilityByItem();
    newdlg->set(params);
    omfgThis->handleNewWindow(newdlg);
  }
//  ToDo
}

void dspInventoryAvailabilityByWorkOrder::sCreatePR()
{
  int currentAltId = 0;
  ParameterList params;
  params.append("mode", "new");
  params.append("itemsite_id", list()->id());

  purchaseRequest newdlg(this, "", true);
  newdlg.set(params);
  newdlg.exec();

  q.prepare("SELECT womatl_id FROM womatl WHERE womatl_itemsite_id = :womatl_itemsite_id");
  q.bindValue(":womatl_itemsite_id", list()->id());
  q.exec();
  if (q.first())
    currentAltId =  q.value("womatl_id").toInt();
  int currentId = list()->id();
  sFillList();
  list()->setId(currentId,currentAltId);
}

void dspInventoryAvailabilityByWorkOrder::sCreatePO()
{
  int currentAltId = 0;
  ParameterList params;
  params.append("mode", "new");
  params.append("itemsite_id", list()->id());

  purchaseOrder *newdlg = new purchaseOrder();
  if(newdlg->set(params) == NoError)
    omfgThis->handleNewWindow(newdlg);

  q.prepare("SELECT womatl_id FROM womatl WHERE womatl_itemsite_id = :womatl_itemsite_id");
  q.bindValue(":womatl_itemsite_id", list()->id());
  q.exec();
  if (q.first())
    currentAltId =  q.value("womatl_id").toInt();
  int currentId = list()->id();
  sFillList();
  list()->setId(currentId,currentAltId);
}

void dspInventoryAvailabilityByWorkOrder::sCreateWO()
{
  int currentAltId = 0;
  ParameterList params;
  params.append("mode", "new");
  params.append("itemsite_id", list()->id());

  workOrder *newdlg = new workOrder();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);

  q.prepare("SELECT womatl_id FROM womatl WHERE womatl_itemsite_id = :womatl_itemsite_id");
  q.bindValue(":womatl_itemsite_id", list()->id());
  q.exec();
  if (q.first())
    currentAltId =  q.value("womatl_id").toInt();
  int currentId = list()->id();
  sFillList();
  list()->setId(currentId,currentAltId);
}

void dspInventoryAvailabilityByWorkOrder::sPostMiscProduction()
{
  int currentAltId = 0;
  ParameterList params;
  params.append("itemsite_id", list()->id());

  postMiscProduction newdlg(this, "", true);
  newdlg.set(params);
  newdlg.exec();

  q.prepare("SELECT womatl_id FROM womatl WHERE womatl_itemsite_id = :womatl_itemsite_id");
  q.bindValue(":womatl_itemsite_id", list()->id());
  q.exec();
  if (q.first())
    currentAltId =  q.value("womatl_id").toInt();
  int currentId = list()->id();
  sFillList();
  list()->setId(currentId,currentAltId);
}

void dspInventoryAvailabilityByWorkOrder::sIssueCountTag()
{
  ParameterList params;
  params.append("itemsite_id", list()->id());

  createCountTagsByItem newdlg(this, "", true);
  newdlg.set(params);
  newdlg.exec();
}

void dspInventoryAvailabilityByWorkOrder::sEnterMiscCount()
{  
  int currentAltId = 0;
  ParameterList params;
  params.append("itemsite_id", list()->id());
  
  enterMiscCount newdlg(this, "", true);
  newdlg.set(params);
  newdlg.exec();

  q.prepare("SELECT womatl_id FROM womatl WHERE womatl_itemsite_id = :womatl_itemsite_id");
  q.bindValue(":womatl_itemsite_id", list()->id());
  q.exec();
  if (q.first())
    currentAltId =  q.value("womatl_id").toInt();
  int currentId = list()->id();
  sFillList();
  list()->setId(currentId,currentAltId);
}

void dspInventoryAvailabilityByWorkOrder::sFillList()
{
  display::sFillList();
  if(_indentedWo->isChecked())
    list()->expandAll();
}
