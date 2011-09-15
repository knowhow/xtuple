/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include <QAction>
#include <QMenuBar>
#include <QMenu>
#include <QToolBar>

#include <parameter.h>

#include "guiclient.h"

#include "workOrder.h"
#include "explodeWo.h"
#include "implodeWo.h"
#include "closeWo.h"

#include "printWoTraveler.h"
#include "printWoPickList.h"

#include "releaseWorkOrdersByPlannerCode.h"
#include "reprioritizeWo.h"
#include "rescheduleWo.h"
#include "changeWoQty.h"
#include "purgeClosedWorkOrders.h"

#include "woMaterialItem.h"
#include "workOrderMaterials.h"

#include "issueWoMaterialBatch.h"
#include "issueWoMaterialItem.h"
#include "returnWoMaterialBatch.h"
#include "returnWoMaterialItem.h"
#include "scrapWoMaterialFromWIP.h"

#include "postProduction.h"
#include "correctProductionPosting.h"
#include "postMiscProduction.h"

#include "dspWoSchedule.h"
#include "dspWoHistoryByItem.h"
#include "dspWoHistoryByNumber.h"
#include "dspWoHistoryByClassCode.h"
#include "dspWoMaterialsByItem.h"
#include "dspWoMaterialsByWorkOrder.h"
#include "dspInventoryAvailabilityByWorkOrder.h"
#include "dspPendingAvailability.h"
#include "dspJobCosting.h"
#include "dspMaterialUsageVarianceByBOMItem.h"
#include "dspMaterialUsageVarianceByItem.h"
#include "dspMaterialUsageVarianceByComponentItem.h"
#include "dspMaterialUsageVarianceByWorkOrder.h"
#include "dspMaterialUsageVarianceByWarehouse.h"

#include "printWoForm.h"

#include "setup.h"

#include "menuManufacture.h"

menuManufacture::menuManufacture(GUIClient *Pparent) :
  QObject(Pparent)
{
  setObjectName("woModule");
  parent = Pparent;

  toolBar = new QToolBar(tr("Manufacture Tools"));
  toolBar->setObjectName("Manufacture Tools");
  toolBar->setIconSize(QSize(32, 32));
  parent->addToolBar(toolBar);

  mainMenu	 = new QMenu(parent);
  ordersMenu	 = new QMenu(parent);
  formsMenu = new QMenu(parent);
  materialsMenu	 = new QMenu(parent);
  materialsIssueMenu = new QMenu(parent);
  materialsReturnMenu = new QMenu(parent);
  transactionsMenu = new QMenu(parent);
  reportsMenu	 = new QMenu(parent);
  reportsHistoryMenu = new QMenu(parent);
  reportsMatlReqMenu = new QMenu(parent);
  reportsMatlUseVarMenu = new QMenu(parent);
  utilitiesMenu = new QMenu(parent);

  mainMenu->setObjectName("menu.manu");
  ordersMenu->setObjectName("menu.manu.orders");
  formsMenu->setObjectName("menu.manu.forms");
  materialsMenu->setObjectName("menu.manu.materials");
  materialsIssueMenu->setObjectName("menu.manu.materialsissue");
  materialsReturnMenu->setObjectName("menu.manu.materialsreturn");
  transactionsMenu->setObjectName("menu.manu.transactions");
  reportsMenu->setObjectName("menu.manu.reports");
  reportsHistoryMenu->setObjectName("menu.manu.reportshistory");
  reportsMatlReqMenu->setObjectName("menu.manu.reportsmatlreq");
  reportsMatlUseVarMenu->setObjectName("menu.manu.reportsmatlusevar");
  utilitiesMenu->setObjectName("menu.manu.utilities");

  actionProperties acts[] = {
    // Production | Control
    { "menu",                              tr("&Work Order"), (char*)ordersMenu,                       mainMenu,   "true",                  0, 0, true, NULL },
    { "wo.newWorkOrder",                       tr("&New..."), SLOT(sNewWorkOrder()),                   ordersMenu, "MaintainWorkOrders",    0, 0, true, NULL },
    { "separator",                                      NULL, NULL,                                    ordersMenu, "true",                  0, 0, true, NULL },
    { "wo.explodeWorkOrder",               tr("E&xplode..."), SLOT(sExplodeWorkOrder()),               ordersMenu, "ExplodeWorkOrders",     0, 0, true, NULL },
    { "wo.implodeWorkOrder",               tr("&Implode..."), SLOT(sImplodeWorkOrder()),               ordersMenu, "ImplodeWorkOrders",     0, 0, true, NULL },
    { "wo.releaseWorkOrdersByPlannerCode", tr("&Release..."), SLOT(sReleaseWorkOrdersByPlannerCode()), ordersMenu, "ReleaseWorkOrders",     0, 0, true, NULL },
    { "wo.closeWorkOrder",                   tr("&Close..."), SLOT(sCloseWorkOrder()),                 ordersMenu, "CloseWorkOrders",       0, 0, true, NULL },
    { "separator",                                      NULL, NULL,                                    ordersMenu, "true",                  0, 0, true, NULL },
    { "wo.reprioritizeWorkOrder",     tr("Re&prioritize..."), SLOT(sReprioritizeWorkOrder()),          ordersMenu, "ReprioritizeWorkOrders",0, 0, true, NULL },
    { "wo.rescheduleWorkOrder",         tr("Re&schedule..."), SLOT(sRescheduleWorkOrder()),            ordersMenu, "RescheduleWorkOrders",  0, 0, true, NULL },
    { "wo.changeWorkOrderQuantity",tr("Change &Quantity..."), SLOT(sChangeWorkOrderQty()),             ordersMenu, "ChangeWorkOrderQty",    0, 0, true, NULL },

    //  Production | W/O Materials
    { "menu",                               tr("&Materials"), (char*)materialsMenu,                 mainMenu,      "true",                0, 0, true, NULL },
    { "wo.createWoMaterialRequirement",        tr("&New..."), SLOT(sCreateWoMaterialRequirement()), materialsMenu, "MaintainWoMaterials", 0, 0, true, NULL },
    { "wo.maintainWoMaterialRequirements",tr("&Maintain..."), SLOT(sMaintainWoMaterials()),         materialsMenu, "MaintainWoMaterials", 0, 0, true, NULL },

    { "separator",                  NULL,                   NULL,                          mainMenu,          "true",  0, 0, true, NULL },
    
    //  Production | Transactions
    { "menu",                       tr("&Transactions"),    (char*)transactionsMenu,       mainMenu,           "true", 0, 0, true, NULL },
    
    //  Production |Transactions | Issue
    { "menu",                       tr("&Issue Material"),  (char*)materialsIssueMenu,     transactionsMenu,   "true",             0, 0, true, NULL },
    { "wo.issueWoMaterialBatch",    tr("&Batch..."),        SLOT(sIssueWoMaterialBatch()), materialsIssueMenu, "IssueWoMaterials", 0, 0, true, NULL },
    { "wo.issueWoMaterialItem",     tr("&Item..."),         SLOT(sIssueWoMaterialItem()),  materialsIssueMenu, "IssueWoMaterials", 0, 0, true, NULL },
    
    //  Production | Transactions | Return
    { "menu",                       tr("Ret&urn Material"),              (char*)materialsReturnMenu,       transactionsMenu,    "true",              0, 0, true, NULL },
    { "wo.returnWoMaterialBatch",   tr("&Batch..."),                     SLOT(sReturnWoMaterialBatch()),   materialsReturnMenu, "ReturnWoMaterials", 0, 0, true, NULL },
    { "wo.returnWoMaterialItem",    tr("&Item..."),                      SLOT(sReturnWoMaterialItem()),    materialsReturnMenu, "ReturnWoMaterials", 0, 0, true, NULL },
    { "wo.scrapWoMaterialFromWo",   tr("&Scrap..."),                     SLOT(sScrapWoMaterialFromWo()),   transactionsMenu,    "ScrapWoMaterials",  0, 0, true, NULL },
    { "separator",                  NULL,                                NULL,                             transactionsMenu,    "true",              0, 0, true, NULL },
    { "wo.postProduction",          tr("Post Productio&n..."),           SLOT(sPostProduction()),          transactionsMenu,    "PostProduction",    0, 0, true, NULL },
    { "wo.correctProductionPosting",tr("C&orrect Production Posting..."),SLOT(sCorrectProductionPosting()),transactionsMenu,    "PostProduction",    0, 0, true, NULL },
    { "wo.closeWorkOrder",          tr("&Close Work Order..."),          SLOT(sCloseWorkOrder()),          transactionsMenu,    "CloseWorkOrders",   0, 0, true, NULL },
    { "separator",                  NULL,                                NULL,                             transactionsMenu,    "true",              0, 0, true, NULL },
    { "wo.postMiscProduction",      tr("Post &Misc. Production..."),     SLOT(sPostMiscProduction()),      transactionsMenu,    "PostMiscProduction",0, 0, true, NULL },

    { "separator",              NULL,                           NULL,                     mainMenu, "true",     0, 0,   true, NULL },
    
    // Production | Forms
    { "menu",                    tr("&Forms"),                   (char*)formsMenu,           mainMenu,  "true",                    0, 0, true, NULL },
    { "wo.printTraveler",        tr("Print &Traveler..."),       SLOT(sPrintTraveler()),     formsMenu, "PrintWorkOrderPaperWork", 0, 0, true, NULL },
    { "wo.printPickList",        tr("Print &Pick List..."),      SLOT(sPrintPickList()),     formsMenu, "PrintWorkOrderPaperWork", 0, 0, true, NULL },
    { "separator",               NULL,                           NULL,                       formsMenu, "true",                    0, 0, true, NULL },
    { "wo.rptPrintWorkOrderForm",tr("Print &Work Order Form..."),SLOT(sPrintWorkOrderForm()),formsMenu, "PrintWorkOrderPaperWork", 0, 0, true, NULL },
    
    //  Production | Reports
    { "menu",                           tr("&Reports"), (char*)reportsMenu,     mainMenu,       "true", 0, 0,   true, NULL },
    
    //  Production | Reports | Schedule
    { "wo.dspWoSchedule",  tr("Work Order &Schedule"), SLOT(sDspWoSchedule()),reportsMenu, "MaintainWorkOrders ViewWorkOrders", QPixmap(":/images/dspWoScheduleByPlannerCode.png"), toolBar, true, tr("Work Order Schedule by Planner Code") },

    //  Production | Reports | Material Requirements
    { "menu",                                       tr("&Material Requirements"),(char*)reportsMatlReqMenu,              reportsMenu,        "true",                              0, 0, true, NULL },
    { "wo.dspWoMaterialRequirementsByWorkOrder",    tr("by &Work Order..."),     SLOT(sDspWoMaterialsByWo()),            reportsMatlReqMenu, "MaintainWorkOrders ViewWorkOrders", 0, 0, true, NULL },
    { "wo.dspWoMaterialRequirementsByComponentItem",tr("by &Component Item..."), SLOT(sDspWoMaterialsByComponentItem()), reportsMatlReqMenu, "MaintainWorkOrders ViewWorkOrders", 0, 0, true, NULL },

    { "wo.dspInventoryAvailabilityByWorkOrder",     tr("&Inventory Availability..."),       SLOT(sDspInventoryAvailabilityByWorkOrder()), reportsMenu, "ViewInventoryAvailability", 0, 0, true, NULL },
    { "wo.dspPendingWoMaterialAvailability",        tr("&Pending Material Availability..."),SLOT(sDspPendingAvailability()),              reportsMenu, "ViewInventoryAvailability", 0, 0, true, NULL },

    { "separator",                      NULL,   NULL,   reportsMenu,    "true", 0, 0,   true, NULL },
    
    //  Production | Reports | History
    { "menu",                       tr("&History"),          (char*)reportsHistoryMenu,        reportsMenu,        "true",                              0, 0, true, NULL },
    { "wo.dspWoHistoryByClassCode", tr("by &Class Code..."), SLOT(sDspWoHistoryByClassCode()), reportsHistoryMenu, "MaintainWorkOrders ViewWorkOrders", 0, 0, true, NULL },
    { "wo.dspWoHistoryByItem",      tr("by &Item..."),       SLOT(sDspWoHistoryByItem()),      reportsHistoryMenu, "MaintainWorkOrders ViewWorkOrders", 0, 0, true, NULL },
    { "wo.dspWoHistoryByNumber",    tr("by &W/O Number..."), SLOT(sDspWoHistoryByNumber()),    reportsHistoryMenu, "MaintainWorkOrders ViewWorkOrders", 0, 0, true, NULL },

    { "separator",        NULL,                   NULL,                   reportsMenu, "true",      0, 0, true, NULL },
    { "wo.dspJobCosting", tr("Work Order &Costing..."),  SLOT(sDspJobCosting()), reportsMenu, "ViewCosts", 0, 0, true, NULL },
    
    //  Production | Reports | Material Usage Variance
    { "menu",                                      tr("Material &Usage Variance"),(char*)reportsMatlUseVarMenu,                    reportsMenu,           "true",                  0, 0, true, NULL },
    { "wo.dspMaterialUsageVarianceByWarehouse",    tr("by &Site..."),             SLOT(sDspMaterialUsageVarianceByWarehouse()),    reportsMatlUseVarMenu, "ViewMaterialVariances", 0, 0, true, NULL },
    { "wo.dspMaterialUsageVarianceByItem",         tr("by &Item..."),             SLOT(sDspMaterialUsageVarianceByItem()),         reportsMatlUseVarMenu, "ViewMaterialVariances", 0, 0, true, NULL },
    { "wo.dspMaterialUsageVarianceByBOMItem",      tr("by &BOM Item..."),         SLOT(sDspMaterialUsageVarianceByBOMItem()),      reportsMatlUseVarMenu, "ViewMaterialVariances", 0, 0, true, NULL },
    { "wo.dspMaterialUsageVarianceByComponentItem",tr("by &Component Item..."),   SLOT(sDspMaterialUsageVarianceByComponentItem()),reportsMatlUseVarMenu, "ViewMaterialVariances", 0, 0, true, NULL },
    { "wo.dspMaterialUsageVarianceByWorkOrder",    tr("by &Work Order..."),       SLOT(sDspMaterialUsageVarianceByWorkOrder()),    reportsMatlUseVarMenu, "ViewMaterialVariances", 0, 0, true, NULL },

    { "separator",              NULL,                               NULL,                           mainMenu,      "true",            0, 0, true, NULL },
    { "menu",                   tr("&Utilities"),                   (char*)utilitiesMenu,           mainMenu,      "true",            0, 0, true, NULL },
    { "wo.purgeClosedWorkOrder",tr("Pur&ge Closed Work Orders..."), SLOT(sPurgeClosedWorkOrders()), utilitiesMenu, "PurgeWorkOrders", 0, 0, true, NULL },
    { "wo.setup",	tr("&Setup..."),	SLOT(sSetup()),	mainMenu,	NULL,	NULL,	NULL,	true, NULL}
  };

  addActionsToMenu(acts, sizeof(acts) / sizeof(acts[0]));

  parent->populateCustomMenu(mainMenu, "Manufacture");
  QAction * m = parent->menuBar()->addMenu(mainMenu);
  if(m)
    m->setText(tr("&Manufacture"));
}

void menuManufacture::addActionsToMenu(actionProperties acts[], unsigned int numElems)
{
  QAction * m = 0;
  for (unsigned int i = 0; i < numElems; i++)
  {
    if (! acts[i].visible)
    {
      continue;
    }
    else if (acts[i].actionName == QString("menu"))
    {
      m = acts[i].menu->addMenu((QMenu*)(acts[i].slot));
      if(m)
        m->setText(acts[i].actionTitle);
    }
    else if (acts[i].actionName == QString("separator"))
    {
      acts[i].menu->addSeparator();
    }
    else if ((acts[i].toolBar != NULL) && (!acts[i].toolTip.isEmpty()))
    {
      new Action( parent,
                  acts[i].actionName,
                  acts[i].actionTitle,
                  this,
                  acts[i].slot,
                  acts[i].menu,
                  acts[i].priv,
                  (acts[i].pixmap),
                  acts[i].toolBar,
                  acts[i].toolTip) ;
    }
    else if (acts[i].toolBar != NULL)
    {
      new Action( parent,
                  acts[i].actionName,
                  acts[i].actionTitle,
                  this,
                  acts[i].slot,
                  acts[i].menu,
                  acts[i].priv,
                  (acts[i].pixmap),
                  acts[i].toolBar,
                  acts[i].actionTitle) ;
    }
    else
    {
      new Action( parent,
                  acts[i].actionName,
                  acts[i].actionTitle,
                  this,
                  acts[i].slot,
                  acts[i].menu,
                  acts[i].priv ) ;
    }
  }
}

void menuManufacture::sNewWorkOrder()
{
  ParameterList params;
  params.append("mode", "new");

  workOrder *newdlg = new workOrder();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void menuManufacture::sExplodeWorkOrder()
{
  explodeWo(parent, "", TRUE).exec();
}

void menuManufacture::sImplodeWorkOrder()
{
  implodeWo(parent, "", TRUE).exec();
}

void menuManufacture::sPrintTraveler()
{
  printWoTraveler(parent, "", TRUE).exec();
}

void menuManufacture::sPrintPickList()
{
  printWoPickList(parent, "", TRUE).exec();
}

void menuManufacture::sCloseWorkOrder()
{
  closeWo(parent, "", TRUE).exec();
}

void menuManufacture::sReleaseWorkOrdersByPlannerCode()
{
  releaseWorkOrdersByPlannerCode(parent, "", TRUE).exec();
}

void menuManufacture::sReprioritizeWorkOrder()
{
  reprioritizeWo(parent, "", TRUE).exec();
}

void menuManufacture::sRescheduleWorkOrder()
{
  rescheduleWo(parent, "", TRUE).exec();
}

void menuManufacture::sChangeWorkOrderQty()
{
  changeWoQty(parent, "", TRUE).exec();
}

void menuManufacture::sPurgeClosedWorkOrders()
{
  purgeClosedWorkOrders(parent, "", TRUE).exec();
}

void menuManufacture::sCreateWoMaterialRequirement()
{
  ParameterList params;
  params.append("mode", "new");

  woMaterialItem newdlg(parent, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void menuManufacture::sMaintainWoMaterials()
{
  omfgThis->handleNewWindow(new workOrderMaterials());
}

void menuManufacture::sIssueWoMaterialBatch()
{
  issueWoMaterialBatch(parent, "", TRUE).exec();
}

void menuManufacture::sIssueWoMaterialItem()
{
  issueWoMaterialItem(parent, "", TRUE).exec();
}

void menuManufacture::sReturnWoMaterialBatch()
{
  returnWoMaterialBatch(parent, "", TRUE).exec();
}

void menuManufacture::sReturnWoMaterialItem()
{
  returnWoMaterialItem(parent, "", TRUE).exec();
}

void menuManufacture::sScrapWoMaterialFromWo()
{
  ParameterList params;
  if (_privileges->check("PostProduction"))
    params.append("canPostProd", true);

  scrapWoMaterialFromWIP newdlg(parent, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void menuManufacture::sPostProduction()
{
  postProduction(parent, "", TRUE).exec();
}

void menuManufacture::sCorrectProductionPosting()
{
  correctProductionPosting(parent, "", TRUE).exec();
}

void menuManufacture::sPostMiscProduction()
{
  postMiscProduction(parent, "", TRUE).exec();
}

void menuManufacture::sDspWoHistoryByItem()
{
  omfgThis->handleNewWindow(new dspWoHistoryByItem());
}

void menuManufacture::sDspWoHistoryByNumber()
{
  omfgThis->handleNewWindow(new dspWoHistoryByNumber());
}

void menuManufacture::sDspWoHistoryByClassCode()
{
  omfgThis->handleNewWindow(new dspWoHistoryByClassCode());
}

void menuManufacture::sDspWoMaterialsByComponentItem()
{
  omfgThis->handleNewWindow(new dspWoMaterialsByItem());
}

void menuManufacture::sDspWoMaterialsByWo()
{
  omfgThis->handleNewWindow(new dspWoMaterialsByWorkOrder());
}

void menuManufacture::sDspInventoryAvailabilityByWorkOrder()
{
  omfgThis->handleNewWindow(new dspInventoryAvailabilityByWorkOrder());
}

void menuManufacture::sDspPendingAvailability()
{
  omfgThis->handleNewWindow(new dspPendingAvailability());
}

void menuManufacture::sDspWoSchedule()
{
  omfgThis->handleNewWindow(new dspWoSchedule());
}

void menuManufacture::sDspJobCosting()
{
  omfgThis->handleNewWindow(new dspJobCosting());
}

void menuManufacture::sDspMaterialUsageVarianceByBOMItem()
{
  omfgThis->handleNewWindow(new dspMaterialUsageVarianceByBOMItem());
}

void menuManufacture::sDspMaterialUsageVarianceByItem()
{
  omfgThis->handleNewWindow(new dspMaterialUsageVarianceByItem());
}

void menuManufacture::sDspMaterialUsageVarianceByComponentItem()
{
  omfgThis->handleNewWindow(new dspMaterialUsageVarianceByComponentItem());
}

void menuManufacture::sDspMaterialUsageVarianceByWorkOrder()
{
  omfgThis->handleNewWindow(new dspMaterialUsageVarianceByWorkOrder());
}

void menuManufacture::sDspMaterialUsageVarianceByWarehouse()
{
  omfgThis->handleNewWindow(new dspMaterialUsageVarianceByWarehouse());
}

void menuManufacture::sPrintWorkOrderForm()
{
  printWoForm(parent, "", TRUE).exec();
}

void menuManufacture::sSetup()
{
  ParameterList params;
  params.append("module", Xt::ManufactureModule);

  setup newdlg(parent);
  newdlg.set(params);
  newdlg.exec();
}
