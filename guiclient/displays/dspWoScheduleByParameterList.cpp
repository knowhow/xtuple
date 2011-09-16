/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspWoScheduleByParameterList.h"

#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include "bom.h"
#include "changeWoQty.h"
#include "closeWo.h"
#include "correctProductionPosting.h"
#include "dspInventoryAvailabilityByWorkOrder.h"
#include "dspRunningAvailability.h"
#include "dspWoMaterialsByWorkOrder.h"
#include "explodeWo.h"
#include "implodeWo.h"
#include "issueWoMaterialItem.h"
#include "postProduction.h"
#include "printWoTraveler.h"
#include "reprioritizeWo.h"
#include "rescheduleWo.h"
#include "salesOrderInformation.h"
#include "storedProcErrorLookup.h"
#include "workOrder.h"

dspWoScheduleByParameterList::dspWoScheduleByParameterList(QWidget* parent, const char*, Qt::WFlags fl)
  : display(parent, "dspWoScheduleByParameterList", fl)
{
  setupUi(optionsWidget());
  setNewVisible(true);
  setWindowTitle(tr("Work Order Schedule"));
  setListLabel(tr("Work Orders"));
  setReportName("WOScheduleByParameterList");
  setMetaSQLOptions("workOrderSchedule", "parameterlist");
  setUseAltId(true);
  setAutoUpdateEnabled(true);

  _dates->setStartNull(tr("Earliest"), omfgThis->startOfTime(), true);
  _dates->setEndNull(tr("Latest"), omfgThis->endOfTime(), true);

  list()->addColumn(tr("parentType"),  0,             Qt::AlignCenter, true,  "wo_ordtype" );
  list()->addColumn(tr("W/O #"),       _orderColumn,  Qt::AlignLeft,   true,  "wonumber"   );
  list()->addColumn(tr("Status"),      _statusColumn, Qt::AlignCenter, true,  "wo_status" );
  list()->addColumn(tr("Pri."),        _statusColumn, Qt::AlignCenter, false,  "wo_priority" );
  list()->addColumn(tr("Site"),        _whsColumn,    Qt::AlignCenter, true,  "warehous_code" );
  list()->addColumn(tr("Item Number"), _itemColumn,   Qt::AlignLeft,   true,  "item_number"   );
  list()->addColumn(tr("Description"), -1,            Qt::AlignLeft,   true,  "itemdescrip"   );
  list()->addColumn(tr("UOM"),         _uomColumn,    Qt::AlignCenter, true,  "uom_name" );
  list()->addColumn(tr("Ordered"),     _qtyColumn,    Qt::AlignRight,  true,  "wo_qtyord"  );
  list()->addColumn(tr("Received"),    _qtyColumn,    Qt::AlignRight,  true,  "wo_qtyrcv"  );
  list()->addColumn(tr("Start Date"),  _dateColumn,   Qt::AlignRight,  true,  "wo_startdate"  );
  list()->addColumn(tr("Due Date"),    _dateColumn,   Qt::AlignRight,  true,  "wo_duedate"  );
  list()->addColumn(tr("Condition"),   _dateColumn,   Qt::AlignLeft,   true,  "condition"   );

  connect(omfgThis, SIGNAL(workOrdersUpdated(int, bool)), this, SLOT(sFillList()));
}

void dspWoScheduleByParameterList::languageChange()
{
  display::languageChange();
  retranslateUi(this);
}

enum SetResponse dspWoScheduleByParameterList::set(const ParameterList &pParams)
{
  XWidget::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("classcode", &valid);
  if (valid)
  {
    _parameter->setType(ParameterGroup::ClassCode);
    setWindowTitle(tr("Work Order Schedule by Class Code"));
  }

  param = pParams.value("plancode", &valid);
  if (valid)
  {
    _parameter->setType(ParameterGroup::PlannerCode);
    setWindowTitle(tr("Work Order Schedule by Planner Code"));
  }

  param = pParams.value("plancode_id", &valid);
  if (valid)
    _parameter->setId(param.toInt());

  param = pParams.value("itemgrp", &valid);
  if (valid)
  {
    _parameter->setType(ParameterGroup::ItemGroup);
    setWindowTitle(tr("Work Order Schedule by Item Group"));
  }

  param = pParams.value("wrkcnt", &valid);
  if (valid)
  {
    _parameter->setType(ParameterGroup::WorkCenter);
    setWindowTitle(tr("Work Order Schedule by Work Center"));
  }

  param = pParams.value("warehous_id", &valid);
  if (valid)
    _warehouse->setId(param.toInt());

  param = pParams.value("startDate", &valid);
  if (valid)
    _dates->setStartDate(param.toDate());
  
  param = pParams.value("endDate", &valid);
  if (valid)
    _dates->setEndDate(param.toDate()); 
    
  if (pParams.inList("run"))
  {
    sFillList();
    return NoError_Run;
  }   

  return NoError;
}

bool dspWoScheduleByParameterList::setParams(ParameterList &pParams)
{
  _warehouse->appendValue(pParams);
  _parameter->appendValue(pParams);
  _dates->appendValue(pParams);

  QStringList statusList;
  if (_open->isChecked())
    statusList.append("O");
  if (_exploded->isChecked())
    statusList.append("E");
  if (_released->isChecked())
    statusList.append("R");
  if (_inprocess->isChecked())
    statusList.append("I");
  if (!statusList.count())
    return false;
  pParams.append("status_list", statusList);

  if (_showOnlyTopLevel->isChecked())
    pParams.append("showOnlyTopLevel");

  return true;
}

void dspWoScheduleByParameterList::sView()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("wo_id", list()->id());

  workOrder *newdlg = new workOrder();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspWoScheduleByParameterList::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("wo_id", list()->id());

  workOrder *newdlg = new workOrder();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspWoScheduleByParameterList::sPostProduction()
{
  ParameterList params;
  params.append("wo_id", list()->id());

  postProduction newdlg(this, "", true);
  newdlg.set(params);
  newdlg.exec();
}

void dspWoScheduleByParameterList::sCorrectProductionPosting()
{
  ParameterList params;
  params.append("wo_id", list()->id());

  correctProductionPosting newdlg(this, "", true);
  newdlg.set(params);
  newdlg.exec();
}

void dspWoScheduleByParameterList::sReleaseWO()
{
  q.prepare("SELECT releaseWo(:wo_id, false);");
  q.bindValue(":wo_id", list()->id());
  q.exec();

  omfgThis->sWorkOrdersUpdated(list()->id(), true);
}

void dspWoScheduleByParameterList::sRecallWO()
{
  q.prepare("SELECT recallWo(:wo_id, false);");
  q.bindValue(":wo_id", list()->id());
  q.exec();

  omfgThis->sWorkOrdersUpdated(list()->id(), true);
}

void dspWoScheduleByParameterList::sExplodeWO()
{
  ParameterList params;
  params.append("wo_id", list()->id());

  explodeWo newdlg(this, "", true);
  newdlg.set(params);
  newdlg.exec();
}

void dspWoScheduleByParameterList::sImplodeWO()
{
  ParameterList params;
  params.append("wo_id", list()->id());

  implodeWo newdlg(this, "", true);
  newdlg.set(params);
  newdlg.exec();
}

void dspWoScheduleByParameterList::sDeleteWO()
{
  q.prepare( "SELECT wo_ordtype "
             "FROM wo "
             "WHERE (wo_id=:wo_id);" );
  q.bindValue(":wo_id", list()->id());
  q.exec();
  if (q.first())
  {
    QString question;
    if (q.value("wo_ordtype") == "W")
      question = tr("<p>The Work Order that you selected to delete is a child "
		    "of another Work Order.  If you delete the selected Work "
		    "Order then the Work Order Materials Requirements for the "
		    "Component Item will remain but the Work Order to relieve "
		    "that demand will not. Are you sure that you want to "
		    "delete the selected Work Order?" );
    else if (q.value("wo_ordtype") == "S")
      question = tr("<p>The Work Order that you selected to delete was created "
		    "to satisfy Sales Order demand. If you delete the selected "
		    "Work Order then the Sales Order demand will remain but "
		    "the Work Order to relieve that demand will not. Are you "
		    "sure that you want to delete the selected Work Order?" );
    else
      question = tr("<p>Are you sure that you want to delete the selected "
		    "Work Order?");
    if (QMessageBox::question(this, tr("Delete Work Order?"),
                              question,
                              QMessageBox::Yes,
                              QMessageBox::No | QMessageBox::Default) == QMessageBox::No)
    {
      return;
    }

    q.prepare("SELECT deleteWo(:wo_id, true) AS returnVal;");
    q.bindValue(":wo_id", list()->id());
    q.exec();

    if (q.first())
    {
      int result = q.value("returnVal").toInt();
      if (result < 0)
      {
	systemError(this, storedProcErrorLookup("deleteWo", result));
	return;
      }
    }
    else if (q.lastError().type() != QSqlError::NoError)
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);

    omfgThis->sWorkOrdersUpdated(-1, true);
  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void dspWoScheduleByParameterList::sCloseWO()
{
  ParameterList params;
  params.append("wo_id", list()->id());

  closeWo newdlg(this, "", true);
  newdlg.set(params);
  newdlg.exec();
}

void dspWoScheduleByParameterList::sPrintTraveler()
{
  ParameterList params;
  params.append("wo_id", list()->id());

  printWoTraveler newdlg(this, "", true);
  newdlg.set(params);
  newdlg.exec();
}

void dspWoScheduleByParameterList::sViewWomatl()
{
  ParameterList params;
  params.append("wo_id", list()->id());
  params.append("run");

  dspWoMaterialsByWorkOrder *newdlg = new dspWoMaterialsByWorkOrder();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspWoScheduleByParameterList::sInventoryAvailabilityByWorkOrder()
{
  ParameterList params;
  params.append("wo_id", list()->id());
  params.append("run");

  dspInventoryAvailabilityByWorkOrder *newdlg = new dspInventoryAvailabilityByWorkOrder();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspWoScheduleByParameterList::sReprioritizeWo()
{
  ParameterList params;
  params.append("wo_id", list()->id());

  reprioritizeWo newdlg(this, "", true);
  newdlg.set(params);
  newdlg.exec();
}

void dspWoScheduleByParameterList::sRescheduleWO()
{
  ParameterList params;
  params.append("wo_id", list()->id());

  rescheduleWo newdlg(this, "", true);
  newdlg.set(params);
  newdlg.exec();
}

void dspWoScheduleByParameterList::sChangeWOQty()
{
  ParameterList params;
  params.append("wo_id", list()->id());

  changeWoQty newdlg(this, "", true);
  newdlg.set(params);
  newdlg.exec();
}

void dspWoScheduleByParameterList::sViewParentSO()
{
  ParameterList params;
  params.append("soitem_id", list()->altId());

  salesOrderInformation newdlg(this, "", true);
  newdlg.set(params);
  newdlg.exec();
}

void dspWoScheduleByParameterList::sViewParentWO()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("wo_id", list()->altId());

  workOrder *newdlg = new workOrder();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspWoScheduleByParameterList::sPopulateMenu(QMenu *pMenu,  QTreeWidgetItem *selected, int)
{
  QString  status(selected->text(2));
  QAction *menuItem;

  menuItem = pMenu->addAction(tr("Edit W/O"), this, SLOT(sEdit()));
  menuItem->setEnabled(_privileges->check("MaintainWorkOrders"));

  menuItem = pMenu->addAction(tr("View W/O"), this, SLOT(sView()));

  pMenu->addSeparator();

  if (status == "E")
  {
    menuItem = pMenu->addAction(tr("Release W/O"), this, SLOT(sReleaseWO()));
    menuItem->setEnabled(_privileges->check("ReleaseWorkOrders"));
  }
  else if (status == "R")
  {
    menuItem = pMenu->addAction(tr("Recall W/O"), this, SLOT(sRecallWO()));
    menuItem->setEnabled(_privileges->check("RecallWorkOrders"));
  }

  if ((status == "E") || (status == "R") || (status == "I"))
  {
    menuItem = pMenu->addAction(tr("Post Production..."), this, SLOT(sPostProduction()));
    menuItem->setEnabled(_privileges->check("PostProduction"));

    if (status != "E")
    {
      menuItem = pMenu->addAction(tr("Correct Production Posting..."), this, SLOT(sCorrectProductionPosting()));
      menuItem->setEnabled(_privileges->check("PostProduction"));
    }

    pMenu->addSeparator();
  }

  if (status == "O")
  {
    menuItem = pMenu->addAction(tr("Explode W/O..."), this, SLOT(sExplodeWO()));
    menuItem->setEnabled(_privileges->check("ExplodeWorkOrders"));
  }
  else if (status == "E")
  {
    menuItem = pMenu->addAction(tr("Implode W/O..."), this, SLOT(sImplodeWO()));
    menuItem->setEnabled(_privileges->check("ImplodeWorkOrders"));
  }

  if ((status == "O") || (status == "E"))
  {
    menuItem = pMenu->addAction(tr("Delete W/O..."), this, SLOT(sDeleteWO()));
    menuItem->setEnabled(_privileges->check("DeleteWorkOrders"));
  }
  else
  {
    menuItem = pMenu->addAction(tr("Close W/O..."), this, SLOT(sCloseWO()));
    menuItem->setEnabled(_privileges->check("CloseWorkOrders"));
  }

  pMenu->addSeparator();

  if ((status == "E") || (status == "R") || (status == "I"))
  {
    menuItem = pMenu->addAction(tr("View W/O Material Requirements..."), this, SLOT(sViewWomatl()));
    menuItem->setEnabled(_privileges->check("ViewWoMaterials"));
      
    menuItem = pMenu->addAction(tr("Inventory Availability by Work Order..."), this, SLOT(sInventoryAvailabilityByWorkOrder()));
    menuItem->setEnabled(_privileges->check("ViewInventoryAvailability"));

    pMenu->addSeparator();

    menuItem = pMenu->addAction(tr("Print Traveler..."), this, SLOT(sPrintTraveler()));
    menuItem->setEnabled(_privileges->check("PrintWorkOrderPaperWork"));

    pMenu->addSeparator();

    menuItem = pMenu->addAction(tr("Issue Material Item..."), this, SLOT(sIssueWoMaterialItem()));
    menuItem->setEnabled(_privileges->check("IssueWoMaterials"));
  }

  if ((status == "O") || (status == "E"))
  {
    pMenu->addSeparator();

    menuItem = pMenu->addAction(tr("Reprioritize W/O..."), this, SLOT(sReprioritizeWo()));
    menuItem->setEnabled(_privileges->check("ReprioritizeWorkOrders"));

    menuItem = pMenu->addAction(tr("Reschedule W/O..."), this, SLOT(sRescheduleWO()));
    menuItem->setEnabled(_privileges->check("RescheduleWorkOrders"));

    menuItem = pMenu->addAction(tr("Change W/O Quantity..."), this, SLOT(sChangeWOQty()));
    menuItem->setEnabled(_privileges->check("ChangeWorkOrderQty"));
  }

  pMenu->addSeparator();
  
  menuItem = pMenu->addAction(tr("View Bill of Materials..."), this, SLOT(sViewBOM()));
  menuItem->setEnabled(_privileges->check("ViewBOMs"));

  pMenu->addSeparator();

  menuItem = pMenu->addAction(tr("Running Availability..."), this, SLOT(sDspRunningAvailability()));

  if (list()->altId() != -1)
  {
    if (selected->text(0) == "S")
    {
      pMenu->addSeparator();
      menuItem = pMenu->addAction(tr("View Parent Sales Order Information..."), this, SLOT(sViewParentSO()));
    }
    else if (selected->text(0) == "W")
    {
      pMenu->addSeparator();
      menuItem = pMenu->addAction(tr("View Parent Work Order Information..."), this, SLOT(sViewParentWO()));
    }
  }
}

void dspWoScheduleByParameterList::sIssueWoMaterialItem()
{
  issueWoMaterialItem newdlg(this);
  ParameterList params;
  params.append("wo_id", list()->id());
  if (newdlg.set(params) == NoError)
    newdlg.exec();
}

void dspWoScheduleByParameterList::sDspRunningAvailability()
{
  q.prepare("SELECT wo_itemsite_id FROM wo WHERE (wo_id=:id);");
  q.bindValue(":id", list()->id());
  q.exec();
  if (q.first())
  {
    ParameterList params;
    params.append("itemsite_id", q.value("wo_itemsite_id"));
    params.append("run");

    dspRunningAvailability *newdlg = new dspRunningAvailability();
    newdlg->set(params);
    omfgThis->handleNewWindow(newdlg);
  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void dspWoScheduleByParameterList::sViewBOM()
{
  q.prepare("SELECT itemsite_item_id "
	    "FROM wo, itemsite "
	    "WHERE ((wo_itemsite_id=itemsite_id)"
	    "  AND  (wo_id=:id));");
  q.bindValue(":id", list()->id());
  q.exec();
  if (q.first())
  {
    ParameterList params;
    params.append("item_id", q.value("itemsite_item_id"));
    params.append("mode", "view");

    BOM *newdlg = new BOM();
    newdlg->set(params);
    omfgThis->handleNewWindow(newdlg);
  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void dspWoScheduleByParameterList::sNew()
{
    ParameterList params;
    params.append("mode", "new");
    workOrder *newdlg = new workOrder();
    newdlg->set(params);
    omfgThis->handleNewWindow(newdlg);
}

