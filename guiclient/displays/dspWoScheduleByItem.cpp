/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspWoScheduleByItem.h"

#include <QAction>
#include <QMenu>
#include <QMessageBox>
#include <QSqlError>

#include "guiclient.h"
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
#include "storedProcErrorLookup.h"
#include "workOrder.h"

dspWoScheduleByItem::dspWoScheduleByItem(QWidget* parent, const char*, Qt::WFlags fl)
  : display(parent, "dspWoScheduleByItem", fl)
{
  setupUi(optionsWidget());
  setWindowTitle(tr("Work Order Schedule by Item"));
  setListLabel(tr("Work Orders"));
  setReportName("WOScheduleByParameterList");
  setMetaSQLOptions("workOrderSchedule", "detail");
  setUseAltId(true);
  setAutoUpdateEnabled(true);

  _item->setType(ItemLineEdit::cGeneralManufactured | ItemLineEdit::cGeneralPurchased);
  _item->setDefaultType(ItemLineEdit::cGeneralManufactured);
  _dates->setStartNull(tr("Earliest"), omfgThis->startOfTime(), true);
  _dates->setEndNull(tr("Latest"), omfgThis->endOfTime(), true);

  list()->addColumn(tr("W/O #"),      -1,            Qt::AlignLeft,   true,  "wonumber"   );
  list()->addColumn(tr("Status"),     _statusColumn, Qt::AlignCenter, true,  "wo_status" );
  list()->addColumn(tr("Pri."),       _statusColumn, Qt::AlignCenter, true,  "wo_priority" );
  list()->addColumn(tr("Site"),       _whsColumn,    Qt::AlignCenter, true,  "warehous_code" );
  list()->addColumn(tr("Ordered"),    _qtyColumn,    Qt::AlignRight,  true,  "wo_qtyord"  );
  list()->addColumn(tr("Received"),   _qtyColumn,    Qt::AlignRight,  true,  "wo_qtyrcv"  );
  list()->addColumn(tr("Start Date"), _dateColumn,   Qt::AlignCenter, true,  "wo_startdate" );
  list()->addColumn(tr("Due Date"),   _dateColumn,   Qt::AlignCenter, true,  "wo_duedate" );
  
  connect(omfgThis, SIGNAL(workOrdersUpdated(int, bool)), this, SLOT(sFillList()));

  _item->setFocus();
}

void dspWoScheduleByItem::languageChange()
{
  display::languageChange();
  retranslateUi(this);
}

bool dspWoScheduleByItem::setParams(ParameterList &pParams)
{
  if (! _item->isValid())
  {
    QMessageBox::warning(this, tr("Missing Data"),
			 tr("You must enter a item."));
    _item->setFocus();
    return false;
  }
  if (! _dates->allValid())
  {
    QMessageBox::warning(this, tr("Missing Data"),
			 tr("You must enter both a start and end date."));
    _dates->setFocus();
    return false;
  }

  _warehouse->appendValue(pParams);
  _dates->appendValue(pParams);
  pParams.append("item_id", _item->id());

  if (_showOnlyRI->isChecked())
    pParams.append("showOnlyRI");

  if (_showOnlyTopLevel->isChecked())
    pParams.append("showOnlyTopLevel");

  return true;
}

void dspWoScheduleByItem::sView()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("wo_id", list()->id());

  workOrder *newdlg = new workOrder();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspWoScheduleByItem::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("wo_id", list()->id());

  workOrder *newdlg = new workOrder();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspWoScheduleByItem::sPostProduction()
{
  ParameterList params;
  params.append("wo_id", list()->id());

  postProduction newdlg(this, "", true);
  newdlg.set(params);
  newdlg.exec();
}

void dspWoScheduleByItem::sCorrectProductionPosting()
{
  ParameterList params;
  params.append("wo_id", list()->id());

  correctProductionPosting newdlg(this, "", true);
  newdlg.set(params);
  newdlg.exec();
}

void dspWoScheduleByItem::sReleaseWO()
{
  q.prepare("SELECT releaseWo(:wo_id, false);");
  q.bindValue(":wo_id", list()->id());
  q.exec();

  omfgThis->sWorkOrdersUpdated(list()->id(), true);
}

void dspWoScheduleByItem::sRecallWO()
{
  q.prepare("SELECT recallWo(:wo_id, false);");
  q.bindValue(":wo_id", list()->id());
  q.exec();

  omfgThis->sWorkOrdersUpdated(list()->id(), true);
}

void dspWoScheduleByItem::sExplodeWO()
{
  ParameterList params;
  params.append("wo_id", list()->id());

  explodeWo newdlg(this, "", true);
  newdlg.set(params);
  newdlg.exec();
}

void dspWoScheduleByItem::sImplodeWO()
{
  ParameterList params;
  params.append("wo_id", list()->id());

  implodeWo newdlg(this, "", true);
  newdlg.set(params);
  newdlg.exec();
}

void dspWoScheduleByItem::sDeleteWO()
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
      question = tr("The Work Order that you selected to delete is a child of "
		    "another Work Order.  If you delete the selected Work "
		    "Order then the Work Order Materials Requirements for the "
		    "Component Item will remain but the Work Order to relieve "
		    "that demand will not. Are you sure that you want to "
		    "delete the selected Work Order?" );
    else if (q.value("wo_ordtype") == "S")
      question = tr("The Work Order that you selected to delete was created to "
		    "satisfy a Sales Order demand.  If you delete the selected "
		    "Work Order then the Sales Order demand will remain but "
		    "the Work Order to relieve that demand will not. Are you "
		    "sure that you want to delete the selected Work Order?" );
    else
      question = tr("Are you sure that you want to delete the selected "
		    "Work Order?");

    if (QMessageBox::question(this, tr("Delete Work Order?"), "<p>" + question,
			      QMessageBox::No | QMessageBox::Default,
			      QMessageBox::Yes) == QMessageBox::No)
      return;

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
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }

    omfgThis->sWorkOrdersUpdated(list()->id(), true);
  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void dspWoScheduleByItem::sCloseWO()
{
  ParameterList params;
  params.append("wo_id", list()->id());

  closeWo newdlg(this, "", true);
  newdlg.set(params);
  newdlg.exec();
}

void dspWoScheduleByItem::sPrintTraveler()
{
  ParameterList params;
  params.append("wo_id", list()->id());

  printWoTraveler newdlg(this, "", true);
  newdlg.set(params);
  newdlg.exec();
}

void dspWoScheduleByItem::sReprioritizeWo()
{
  ParameterList params;
  params.append("wo_id", list()->id());

  reprioritizeWo newdlg(this, "", true);
  newdlg.set(params);
  newdlg.exec();
}

void dspWoScheduleByItem::sRescheduleWO()
{
  ParameterList params;
  params.append("wo_id", list()->id());

  rescheduleWo newdlg(this, "", true);
  newdlg.set(params);
  newdlg.exec();
}

void dspWoScheduleByItem::sChangeWOQty()
{
  ParameterList params;
  params.append("wo_id", list()->id());

  changeWoQty newdlg(this, "", true);
  newdlg.set(params);
  newdlg.exec();
}

void dspWoScheduleByItem::sViewWomatl()
{
  ParameterList params;
  params.append("wo_id", list()->id());
  params.append("run");

  dspWoMaterialsByWorkOrder *newdlg = new dspWoMaterialsByWorkOrder();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspWoScheduleByItem::sInventoryAvailabilityByWorkOrder()
{
  ParameterList params;
  params.append("wo_id", list()->id());
  params.append("run");

  dspInventoryAvailabilityByWorkOrder *newdlg = new dspInventoryAvailabilityByWorkOrder();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspWoScheduleByItem::sPopulateMenu(QMenu *pMenu, QTreeWidgetItem *selected, int)
{
  QString  status(selected->text(1));
  QAction *menuItem;

  menuItem = pMenu->addAction(tr("Edit W/O"), this, SLOT(sEdit()));
  if (!_privileges->check("MaintainWorkOrders"))
    menuItem->setEnabled(false);
  menuItem = pMenu->addAction(tr("View W/O"), this, SLOT(sView()));

  pMenu->addSeparator();

  if (status == "E")
  {
    menuItem = pMenu->addAction(tr("Release W/O"), this, SLOT(sReleaseWO()));
    if (!_privileges->check("ReleaseWorkOrders"))
      menuItem->setEnabled(false);
  }
  else if (status == "R")
  {
    menuItem = pMenu->addAction(tr("Recall W/O"), this, SLOT(sRecallWO()));
    if (!_privileges->check("RecallWorkOrders"))
      menuItem->setEnabled(false);
  }

  if ((status == "E") || (status == "R") || (status == "I"))
  {
    menuItem = pMenu->addAction(tr("Post Production..."), this, SLOT(sPostProduction()));
    if (!_privileges->check("PostProduction"))
      menuItem->setEnabled(false);

    if (status != "E")
    {
      menuItem = pMenu->addAction(tr("Correct Production Posting..."), this, SLOT(sCorrectProductionPosting()));
      if (!_privileges->check("PostProduction"))
        menuItem->setEnabled(false);
    }

    pMenu->addSeparator();
  }

  if (status == "O")
  {
    menuItem = pMenu->addAction(tr("Explode W/O..."), this, SLOT(sExplodeWO()));
    if (!_privileges->check("ExplodeWorkOrders"))
      menuItem->setEnabled(false);
  }
  else if (status == "E")
  {
    menuItem = pMenu->addAction(tr("Implode W/O..."), this, SLOT(sImplodeWO()));
    if (!_privileges->check("ImplodeWorkOrders"))
      menuItem->setEnabled(false);
  }

  if ((status == "O") || (status == "E"))
  {
    menuItem = pMenu->addAction(tr("Delete W/O..."), this, SLOT(sDeleteWO()));
    if (!_privileges->check("DeleteWorkOrders"))
      menuItem->setEnabled(false);
  }
  else
  {
    menuItem = pMenu->addAction(tr("Close W/O..."), this, SLOT(sCloseWO()));
    if (!_privileges->check("CloseWorkOrders"))
      menuItem->setEnabled(false);
  }

  pMenu->addSeparator();

  if ((status == "E") || (status == "R") || (status == "I"))
  {
    menuItem = pMenu->addAction(tr("View W/O Material Requirements..."), this, SLOT(sViewWomatl()));
    if (!_privileges->check("ViewWoMaterials"))
      menuItem->setEnabled(false);

    menuItem = pMenu->addAction(tr("Inventory Availability by Work Order..."), this, SLOT(sInventoryAvailabilityByWorkOrder()));
    if (!_privileges->check("ViewInventoryAvailability"))
      menuItem->setEnabled(false);

    pMenu->addSeparator();

    menuItem = pMenu->addAction(tr("Print Traveler..."), this, SLOT(sPrintTraveler()));
    if (!_privileges->check("PrintWorkOrderPaperWork"))
      menuItem->setEnabled(false);

    pMenu->addSeparator();

    menuItem = pMenu->addAction(tr("Issue Material Item..."), this, SLOT(sIssueWoMaterialItem()));
    menuItem->setEnabled(_privileges->check("IssueWoMaterials"));
  }

  if ((status == "O") || (status == "E"))
  {
    pMenu->addSeparator();

    menuItem = pMenu->addAction(tr("Reprioritize W/O..."), this, SLOT(sReprioritizeWo()));
    if (!_privileges->check("ReprioritizeWorkOrders"))
      menuItem->setEnabled(false);

    menuItem = pMenu->addAction(tr("Reschedule W/O..."), this, SLOT(sRescheduleWO()));
    if (!_privileges->check("RescheduleWorkOrders"))
      menuItem->setEnabled(false);

    menuItem = pMenu->addAction(tr("Change W/O Quantity..."), this, SLOT(sChangeWOQty()));
    if (!_privileges->check("ChangeWorkOrderQty"))
      menuItem->setEnabled(false);
  }

  pMenu->addSeparator();
  
  menuItem = pMenu->addAction(tr("View Bill of Materials..."), this, SLOT(sViewBOM()));
  menuItem->setEnabled(_privileges->check("ViewBOMs"));

  pMenu->addSeparator();

  menuItem = pMenu->addAction(tr("Running Availability..."), this, SLOT(sDspRunningAvailability()));
}

void dspWoScheduleByItem::sIssueWoMaterialItem()
{
  issueWoMaterialItem newdlg(this);
  ParameterList params;
  params.append("wo_id", list()->id());
  if (newdlg.set(params) == NoError)
    newdlg.exec();
}

void dspWoScheduleByItem::sDspRunningAvailability()
{
  ParameterList params;
  params.append("itemsite_id", list()->altId());
  params.append("run");

  dspRunningAvailability *newdlg = new dspRunningAvailability();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspWoScheduleByItem::sViewBOM()
{
  q.prepare("SELECT itemsite_item_id FROM itemsite WHERE (itemsite_id=:id);");
  q.bindValue(":id", list()->altId());
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

