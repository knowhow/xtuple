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
#include <QStatusBar>
#include <QPixmap>
#include <QMenu>
#include <QToolBar>

#include <parameter.h>

#include "guiclient.h"
#include "inputManager.h"

#include "item.h"
#include "items.h"
#include "copyItem.h"
#include "itemGroups.h"
#include "itemImages.h"
#include "lotSerial.h"
#include "bom.h"
#include "bomList.h"
#include "copyBOM.h"

#include "massReplaceComponent.h"
#include "massExpireComponent.h"

#include "maintainItemCosts.h"
#include "updateActualCostsByItem.h"
#include "updateActualCostsByClassCode.h"
#include "postCostsByItem.h"
#include "postCostsByClassCode.h"

#include "dspCostedSingleLevelBOM.h"
#include "dspCostedIndentedBOM.h"
#include "dspCostedSummarizedBOM.h"
#include "dspItemCostsByClassCode.h"
#include "dspItemCostSummary.h"
#include "dspItemCostHistory.h"

#include "costingElements.h"

#include "dspSingleLevelBOM.h"
#include "dspIndentedBOM.h"
#include "dspSummarizedBOM.h"
#include "dspSingleLevelWhereUsed.h"
#include "dspIndentedWhereUsed.h"
#include "dspPendingBOMChanges.h"
#include "dspCapacityUOMsByClassCode.h"
#include "dspCapacityUOMsByProductCategory.h"

#include "itemAvailabilityWorkbench.h"

#include "dspUndefinedManufacturedItems.h"
#include "dspUnusedPurchasedItems.h"
#include "dspInvalidBillsOfMaterials.h"
#include "reassignClassCodeByClassCode.h"
#include "reassignProductCategoryByProductCategory.h"

#include "setup.h"

#include "menuProducts.h"

#include "userPreferences.h"

menuProducts::menuProducts(GUIClient *Pparent) :
 QObject(Pparent)
{
  setObjectName("productMenu");

  parent = Pparent;

  toolBar = new QToolBar(tr("Products Tools"));
  toolBar->setObjectName("Products Tools");
  toolBar->setIconSize(QSize(32, 32));
  parent->addToolBar(toolBar);
    
  mainMenu	= new QMenu(parent);
  itemsMenu	= new QMenu(parent);
  bomMenu	= new QMenu(parent);
  costingMenu = new QMenu(parent);
  costingUpdActMenu = new QMenu(parent);
  costingPostActMenu = new QMenu(parent);
  costingUpdStdMenu = new QMenu(parent);
  costingReportsMenu = new QMenu(parent);
  costingReportsCostedMenu = new QMenu(parent);
  costingReportsItemCostsMenu = new QMenu(parent);
  reportsMenu = new QMenu(parent);
  reportsBomsMenu = new QMenu(parent);
  reportsWhereUsdMenu = new QMenu(parent);
  reportsCapUomMenu = new QMenu(parent);
  utilitiesMenu = new QMenu(parent);

  mainMenu->setObjectName("menu.prod");
  itemsMenu->setObjectName("menu.prod.items");
  bomMenu->setObjectName("menu.prod.bom");
  costingMenu->setObjectName("menu.prod.costing");
  costingUpdActMenu->setObjectName("menu.prod.costingupdact");
  costingPostActMenu->setObjectName("menu.prod.costingpostact");
  costingUpdStdMenu->setObjectName("menu.prod.costingupdstd");
  costingReportsMenu->setObjectName("menu.prod.costingreports");
  costingReportsCostedMenu->setObjectName("menu.prod.costingreportscosted");
  costingReportsItemCostsMenu->setObjectName("menu.prod.costingreportsitemcosts");
  reportsMenu->setObjectName("menu.prod.reports");
  reportsBomsMenu->setObjectName("menu.prod.reportsboms");
  reportsWhereUsdMenu->setObjectName("menu.prod.reportswhereusd");
  reportsCapUomMenu->setObjectName("menu.prod.reportscapuom");
  utilitiesMenu->setObjectName("menu.prod.utilities");

  actionProperties acts[] = {
  
#ifdef Q_WS_MACX
  { "sys.preferences",              tr("P&references..."),                SLOT(sPreferences()),              mainMenu, "MaintainPreferencesSelf MaintainPreferencesOthers",  NULL,   NULL,   true, NULL },
#endif

  // Product | Reports
  { "menu",	tr("&Reports"), (char*)reportsMenu,	mainMenu, "true", NULL, NULL, true , NULL },
  
  { "separator", NULL, NULL, reportsMenu,	"true", NULL, NULL, true , NULL },
  
  // Product | Reports | BOMs
  { "menu",	tr("&Bills of Materials"), (char*)reportsBomsMenu,	reportsMenu, "true", NULL, NULL, true , NULL },
  { "pd.dspSingleLevelBOM", tr("&Single Level..."), SLOT(sDspSingleLevelBOM()), reportsBomsMenu, "ViewBOMs", NULL, NULL, true , NULL },
  { "pd.dspIndentedBOM", tr("&Indented..."), SLOT(sDspIndentedBOM()), reportsBomsMenu, "ViewBOMs", NULL, NULL, true , NULL },
  { "pd.dspSummarizedBOM", tr("Summari&zed..."), SLOT(sDspSummarizedBOM()), reportsBomsMenu, "ViewBOMs", NULL, NULL, true , NULL },
  
  // Product | Reports | Where Used
  { "menu",	tr("&Where Used"), (char*)reportsWhereUsdMenu,	reportsMenu, "true", NULL, NULL, true , NULL },
  { "pd.dspSingleLevelWhereUsed", tr("&Single Level..."), SLOT(sDspSingleLevelWhereUsed()), reportsWhereUsdMenu, "ViewBOMs", NULL, NULL, true , NULL },
  { "pd.dspIndentedWhereUsed", tr("&Indented..."), SLOT(sDspIndentedWhereUsed()), reportsWhereUsdMenu, "ViewBOMs", NULL, NULL, true , NULL },

  { "pd.dspPendingBOMChanges", tr("&Pending BOM Changes..."), SLOT(sDspPendingBOMChanges()), reportsMenu, "ViewBOMs", NULL, NULL, true , NULL },
  { "separator", NULL, NULL, reportsMenu,	"true", NULL, NULL, true , NULL },

  // Product | Reports | Capacity UOMs
  { "menu",	tr("Capacity &UOMs"), (char*)reportsCapUomMenu,	reportsMenu, "true", NULL, NULL, true , NULL },
  { "pd.dspCapacityUOMsByProductCategory", tr("by &Product Category..."), SLOT(sDspCapacityUOMsByProductCategory()), reportsCapUomMenu, "MaintainItemMasters ViewItemMasters", NULL, NULL, true , NULL },
  { "pd.dspCapacityUOMsByClassCode", tr("by &Class Code..."), SLOT(sDspCapacityUOMsByClassCode()), reportsCapUomMenu, "MaintainItemMasters ViewItemMasters", NULL, NULL, true , NULL },
  
  {  "separator", NULL, NULL, mainMenu,	"true", NULL, NULL, true , NULL },

  // Product | Items
  { "menu",	tr("&Item"), (char*)itemsMenu,	mainMenu, "true", NULL, NULL, true , NULL },
  { "pd.enterNewItem", tr("&New..."), SLOT(sNewItem()), itemsMenu, "MaintainItemMasters", NULL, NULL, true , NULL },
  { "pd.listItems", tr("&List..."), SLOT(sItems()), itemsMenu, "MaintainItemMasters ViewItemMasters", QPixmap(":/images/items.png"), toolBar, true , tr("List Items") },
  { "pd.copyItem", tr("&Copy..."), SLOT(sCopyItem()), itemsMenu, "MaintainItemMasters" , NULL, NULL, true, NULL },
  { "separator", NULL, NULL, itemsMenu,	"true", NULL, NULL, true , NULL },
  { "pd.itemAvailabilityWorkbench", tr("&Workbench..."), SLOT(sDspItemAvailabilityWorkbench()), itemsMenu, "ViewItemAvailabilityWorkbench", NULL, NULL, true , NULL },
  { "separator", NULL, NULL, itemsMenu,	"true", NULL, NULL, true , NULL },
  { "pd.itemGroups", tr("&Groups..."), SLOT(sItemGroups()), itemsMenu, "MaintainItemGroups ViewItemGroups", NULL, NULL, true , NULL },
  { "pd.itemImages", tr("&Images..."), SLOT(sItemImages()), itemsMenu, "MaintainItemMasters ViewItemMasters", NULL, NULL, true , NULL },

  // Product | Bill of Materials
  { "menu",	tr("Bill Of Ma&terials"), (char*)bomMenu,	mainMenu, "true", NULL, NULL, true , NULL },
  { "pd.enterNewBOM", tr("&New..."), SLOT(sNewBOM()), bomMenu, "MaintainBOMs", NULL, NULL, true , NULL },
  { "pd.listBOMs", tr("&List..."), SLOT(sBOMs()), bomMenu, "MaintainBOMs ViewBOMs", QPixmap(":/images/boms.png"), toolBar, true , tr("List Bill of Materials") },
  { "pd.copyBOM", tr("&Copy..."), SLOT(sCopyBOM()), bomMenu, "MaintainBOMs", NULL, NULL, true , NULL },
  { "separator", NULL, NULL, bomMenu,	"true", NULL, NULL, true , NULL },
  { "pd.massReplaceComponentItem", tr("Mass &Replace..."), SLOT(sMassReplaceComponent()), bomMenu, "MaintainBOMs", NULL, NULL, true , NULL },
  { "pd.massExpireComponentItem", tr("Mass E&xpire..."), SLOT(sMassExpireComponent()),  bomMenu, "MaintainBOMs", NULL, NULL, true , NULL },

  // Product | Costing
  { "menu",	tr("&Costing"), (char*)costingMenu,	mainMenu, "true", NULL, NULL, true , NULL },
  { "pd.maintainItemCosts", tr("&Maintain Item Costs..."), SLOT(sMaintainItemCosts()), costingMenu, "ViewCosts", NULL, NULL, true , NULL },
  
  // Product | Lot Serial
  { "pd.lotSerial", tr("&Lot/Serial..."), SLOT(sLotSerial()), mainMenu, "true", NULL, NULL, _metrics->boolean("LotSerialControl"), NULL }, 

  { "separator", NULL, NULL, costingMenu,	"true", NULL, NULL, true , NULL },
  
  // Product | Costing | Update Actual Costs
  { "menu",	tr("Update &Actual Costs"), (char*)costingUpdActMenu,	costingMenu, "true", NULL, NULL, true , NULL },
  { "pd.updateActualCostsByItem", tr("by &Item..."), SLOT(sUpdateActualCostsByItem()), costingUpdActMenu, "UpdateActualCosts", NULL, NULL, true , NULL },
  { "pd.updateActualCostsByClassCode", tr("by &Class Code..."), SLOT(sUpdateActualCostsByClassCode()), costingUpdActMenu, "UpdateActualCosts", NULL, NULL, true , NULL },

  // Product | Costing | Post Actual Costs
  { "menu",	tr("&Post Actual Costs"), (char*)costingPostActMenu,	costingMenu, "true", NULL, NULL, true , NULL },
  { "pd.postActualCostsByItem", tr("by &Item..."), SLOT(sPostActualCostsByItem()), costingPostActMenu, "PostActualCosts", NULL, NULL, true , NULL },
  { "pd.postActualCostsByClassCode", tr("by &Class Code..."), SLOT(sPostActualCostsByClassCode()), costingPostActMenu, "PostActualCosts", NULL, NULL, true , NULL },

  // Product | Costing | Post Standard Costs
  { "menu",	tr("Post &Standard Costs"), (char*)costingUpdStdMenu,	costingMenu, "true", NULL, NULL, true , NULL },
  { "pd.postStandardCostsByItem", tr("by &Item..."), SLOT(sUpdateStandardCostsByItem()), costingUpdStdMenu, "PostStandardCosts", NULL, NULL, true , NULL },
  { "pd.postStandardCostsByClassCode", tr("by &Class Code..."), SLOT(sUpdateStandardCostsByClassCode()), costingUpdStdMenu, "PostStandardCosts", NULL, NULL, true , NULL },

  { "separator", NULL, NULL, costingMenu,	"true", NULL, NULL, true , NULL },

  //  Product | Costing | Reports
  { "menu",	tr("&Reports"), (char*)costingReportsMenu,	costingMenu, "true", NULL, NULL, true , NULL },
  
  //  Product | Costing | Reports | Costed BOM
  { "menu",	tr("&Costed BOM"), (char*)costingReportsCostedMenu,	costingReportsMenu, "true", NULL, NULL, true , NULL },
  { "pd.dspCostedSingleLevelBOM", tr("&Single Level..."), SLOT(sDspCostedSingleLevelBOM()),costingReportsCostedMenu, "ViewCosts", NULL, NULL, true , NULL },
  { "pd.dspCostedIndentedBOM", tr("&Indented..."), SLOT(sDspCostedIndentedBOM()), costingReportsCostedMenu, "ViewCosts", NULL, NULL, true , NULL },
  { "pd.dspCostedSummarizedBOM", tr("Summari&zed..."), SLOT(sDspCostedSummarizedBOM()), costingReportsCostedMenu, "ViewCosts", NULL, NULL, true , NULL },
  
  //  Product | Costing | Reports | Item Costs
  { "menu",	tr("&Item Costs"), (char*)costingReportsItemCostsMenu,	costingReportsMenu, "true", NULL, NULL, true , NULL },
  { "pd.dspItemCostsByClassCode", tr("by &Class Code..."), SLOT(sDspItemCostsByClassCode()), costingReportsItemCostsMenu, "ViewCosts", NULL, NULL, true , NULL },
  { "pd.dspItemCostsSummary", tr("&Summary..."), SLOT(sDspItemCostSummary()), costingReportsItemCostsMenu, "ViewCosts", NULL, NULL, true , NULL },
  { "pd.dspItemCostsHistory", tr("&History..."), SLOT(sDspItemCostHistory()), costingReportsItemCostsMenu, "ViewCosts", NULL, NULL, true , NULL },

  { "separator", NULL, NULL, costingMenu,	"true", NULL, NULL, true , NULL },
  { "pd.userDefinedCostingElements", tr("&User-Defined Costing Elements..."), SLOT(sUserCostingElements()), costingMenu, "MaintainUserCostingElements", NULL, NULL, true , NULL },

  { "separator", NULL, NULL, mainMenu,	"true", NULL, NULL, true , NULL },

  //  Produt | Utilies
  { "menu",	tr("&Utilities"), (char*)utilitiesMenu, mainMenu, "true", NULL, NULL, true , NULL },
  { "pd.dspUnusedPurchasedItems", tr("Unused &Purchased Items..."), SLOT(sDspUnusedPurchasedItems()), utilitiesMenu, "ViewBOMs", NULL, NULL, true , NULL },
  { "pd.dspUndefinedManufacturedItems", tr("Undefined &Manufactured Items..."), SLOT(sDspUndefinedManufacturedItems()), utilitiesMenu, "ViewBOMs ViewBOOs", NULL, NULL, true , NULL },
  { "pd.dspBillsOfMaterialsWithoutComponentItemSites", tr("Bills of Ma&terials without Component Item Sites..."), SLOT(sDspInvalidBillsOfMaterials()), utilitiesMenu, "ViewBOMs", NULL, NULL, true , NULL },
  { "separator", NULL, NULL, utilitiesMenu,	"true", NULL, NULL, true , NULL },
  { "pd.reassignClassCodeByClassCode", tr("Reassign &Class Codes..."), SLOT(sReassignClassCodeByClassCode()), utilitiesMenu, "MaintainItemMasters", NULL, NULL, true , NULL },
  { "pd.reassignProductCategoryByProductCategory", tr("&Reassign Product Categories..."), SLOT(sReassignProductCategoryByProductCategory()), utilitiesMenu, "MaintainItemMasters", NULL, NULL, true , NULL },

  // Setup
  { "pd.setup",	    tr("&Setup..."),	  SLOT(sSetup()),     mainMenu,	"true",	NULL, NULL,  true, NULL}

  };

  addActionsToMenu(acts, sizeof(acts) / sizeof(acts[0]));

  parent->populateCustomMenu(mainMenu, "Products");
  QAction * m = parent->menuBar()->addMenu(mainMenu);
  if(m)
    m->setText(tr("Produc&ts"));
}

void menuProducts::addActionsToMenu(actionProperties acts[], unsigned int numElems)
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
      m = acts[i].menu->addSeparator();
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

void menuProducts::sNewItem()
{
  item::newItem();
}

void menuProducts::sItems()
{
  omfgThis->handleNewWindow(new items());
}

void menuProducts::sCopyItem()
{
  copyItem(parent, "", TRUE).exec();
}

void menuProducts::sItemGroups()
{
  omfgThis->handleNewWindow(new itemGroups());
}

void menuProducts::sItemImages()
{
  omfgThis->handleNewWindow(new itemImages());
}

void menuProducts::sNewBOM()
{
  ParameterList params;
  params.append("mode", "new");

  BOM *newdlg = new BOM();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void menuProducts::sBOMs()
{
  omfgThis->handleNewWindow(new bomList());
}

void menuProducts::sCopyBOM()
{
  copyBOM(parent, "", TRUE).exec();
}

void menuProducts::sMassReplaceComponent()
{
  omfgThis->handleNewWindow(new massReplaceComponent());
}

void menuProducts::sMassExpireComponent()
{
  omfgThis->handleNewWindow(new massExpireComponent());
}

//  Costing
void menuProducts::sMaintainItemCosts()
{
  omfgThis->handleNewWindow(new maintainItemCosts());
}

void menuProducts::sUpdateActualCostsByItem()
{
  ParameterList params;
  params.append("costtype", "actual");

  updateActualCostsByItem *newdlg = new updateActualCostsByItem(parent, "", TRUE);
  newdlg->set(params);
  newdlg->exec();
}

void menuProducts::sUpdateActualCostsByClassCode()
{
  ParameterList params;
  params.append("costtype", "actual");

  updateActualCostsByClassCode *newdlg = new updateActualCostsByClassCode(parent, "", TRUE);
  newdlg->set(params);
  newdlg->exec();
}

void menuProducts::sPostActualCostsByItem()
{
  postCostsByItem(parent, "", TRUE).exec();
}

void menuProducts::sPostActualCostsByClassCode()
{
  postCostsByClassCode(parent, "", TRUE).exec();
}

void menuProducts::sUpdateStandardCostsByItem()
{
  ParameterList params;
  params.append("costtype", "standard");

  updateActualCostsByItem *newdlg = new updateActualCostsByItem(parent, "", TRUE);
  newdlg->set(params);
  newdlg->exec();
}

void menuProducts::sUpdateStandardCostsByClassCode()
{
  ParameterList params;
  params.append("costtype", "standard");

  updateActualCostsByClassCode *newdlg = new updateActualCostsByClassCode(parent, "", TRUE);
  newdlg->set(params);
  newdlg->exec();
}

void menuProducts::sDspCostedSingleLevelBOM()
{
  omfgThis->handleNewWindow(new dspCostedSingleLevelBOM());
}

void menuProducts::sDspCostedIndentedBOM()
{
  omfgThis->handleNewWindow(new dspCostedIndentedBOM());
}

void menuProducts::sDspCostedSummarizedBOM()
{
  omfgThis->handleNewWindow(new dspCostedSummarizedBOM());
}

void menuProducts::sDspItemCostsByClassCode()
{
  omfgThis->handleNewWindow(new dspItemCostsByClassCode());
}

void menuProducts::sDspItemCostSummary()
{
  omfgThis->handleNewWindow(new dspItemCostSummary());
}

void menuProducts::sDspItemCostHistory()
{
  omfgThis->handleNewWindow(new dspItemCostHistory());
}

void menuProducts::sUserCostingElements()
{
  omfgThis->handleNewWindow(new costingElements());
}

void menuProducts::sLotSerial()
{
  lotSerial newdlg(parent, "", TRUE);
  newdlg.exec();
}

//  Displays
void menuProducts::sDspSingleLevelBOM()
{
  omfgThis->handleNewWindow(new dspSingleLevelBOM());
}

void menuProducts::sDspIndentedBOM()
{
  omfgThis->handleNewWindow(new dspIndentedBOM());
}

void menuProducts::sDspSummarizedBOM()
{
  omfgThis->handleNewWindow(new dspSummarizedBOM());
}

void menuProducts::sDspSingleLevelWhereUsed()
{
  omfgThis->handleNewWindow(new dspSingleLevelWhereUsed());
}

void menuProducts::sDspIndentedWhereUsed()
{
  omfgThis->handleNewWindow(new dspIndentedWhereUsed());
}

void menuProducts::sDspPendingBOMChanges()
{
  omfgThis->handleNewWindow(new dspPendingBOMChanges());
}

void menuProducts::sDspCapacityUOMsByClassCode()
{
  omfgThis->handleNewWindow(new dspCapacityUOMsByClassCode());
}

void menuProducts::sDspCapacityUOMsByProductCategory()
{
  omfgThis->handleNewWindow(new dspCapacityUOMsByProductCategory());
}

void menuProducts::sDspItemAvailabilityWorkbench()
{
  omfgThis->handleNewWindow(new itemAvailabilityWorkbench());
}

//  Utilities
void menuProducts::sDspUnusedPurchasedItems()
{
  omfgThis->handleNewWindow(new dspUnusedPurchasedItems());
}

void menuProducts::sDspUndefinedManufacturedItems()
{
  omfgThis->handleNewWindow(new dspUndefinedManufacturedItems());
}

void menuProducts::sDspInvalidBillsOfMaterials()
{
  omfgThis->handleNewWindow(new dspInvalidBillsOfMaterials());
}

void menuProducts::sReassignClassCodeByClassCode()
{
  reassignClassCodeByClassCode(parent, "", TRUE).exec();
}

void menuProducts::sReassignProductCategoryByProductCategory()
{
  reassignProductCategoryByProductCategory(parent, "", TRUE).exec();
}

void menuProducts::sSetup()
{
  ParameterList params;
  params.append("module", Xt::ProductsModule);

  setup newdlg(parent);
  newdlg.set(params);
  newdlg.exec();
}

void menuProducts::sPreferences()
{
  userPreferences(parent, "", true).exec();
}

