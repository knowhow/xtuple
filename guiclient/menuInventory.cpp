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
#include <QMenu>
#include <QMenuBar>
#include <QPixmap>
#include <QToolBar>

#include <parameter.h>

#include "guiclient.h"
#include "inputManager.h"

#include "itemSite.h"
#include "itemSites.h"

#include "adjustmentTrans.h"
#include "adjustInvValue.h"
#include "transferTrans.h"
#include "transferOrders.h"
#include "transferOrder.h"
#include "releaseTransferOrdersByAgent.h"
#include "scrapTrans.h"
#include "expenseTrans.h"
#include "transformTrans.h"
#include "resetQOHBalances.h"
#include "materialReceiptTrans.h"
#include "relocateInventory.h"

#include "reassignLotSerial.h"

#include "createCountTagsByParameterList.h"
#include "createCountTagsByItem.h"
#include "createCycleCountTags.h"
#include "countSlip.h"
#include "countTag.h"
#include "enterMiscCount.h"
#include "zeroUncountedCountTagsByWarehouse.h"
#include "postCountSlips.h"
#include "postCountTags.h"
#include "purgePostedCountSlips.h"
#include "purgePostedCounts.h"
#include "thawItemSitesByClassCode.h"

#include "packingListBatch.h"
#include "issueToShipping.h"
#include "maintainShipping.h"
#include "shipOrder.h"
#include "recallOrders.h"
#include "externalShippingList.h"

#include "enterPoReceipt.h"
#include "enterPoReturn.h"
#include "unpostedPoReceipts.h"

#include "printPackingList.h"
#include "printPackingListBatchByShipvia.h"
#include "printShippingForm.h"
#include "printShippingForms.h"
#include "printLabelsBySo.h"
#include "printLabelsByTo.h"
#include "printLabelsByInvoice.h"
#include "printLabelsByOrder.h"

#include "dspShipmentsBySalesOrder.h"
#include "dspSummarizedBacklogByWarehouse.h"
#include "dspShipmentsBySalesOrder.h"
#include "dspShipmentsByDate.h"
#include "dspShipmentsByShipment.h"

#include "dspFrozenItemSites.h"
#include "dspCountSlipEditList.h"
#include "dspCountTagEditList.h"
#include "dspCountSlipsByWarehouse.h"
#include "dspCountTagsByItem.h"
#include "dspCountTagsByWarehouse.h"
#include "dspCountTagsByClassCode.h"

#include "itemAvailabilityWorkbench.h"

#include "dspBacklog.h"
#include "dspValidLocationsByItem.h"
#include "dspQOH.h"
#include "dspQOHByLocation.h"
#include "dspInventoryLocator.h"
#include "dspSlowMovingInventoryByClassCode.h"
#include "dspExpiredInventoryByClassCode.h"
#include "dspInventoryAvailability.h"
#include "dspSubstituteAvailabilityByItem.h"
#include "dspInventoryHistory.h"
#include "dspDetailedInventoryHistoryByLotSerial.h"
#include "dspDetailedInventoryHistoryByLocation.h"
#include "dspUsageStatistics.h"
#include "dspTimePhasedUsageStatisticsByItem.h"

#include "printItemLabelsByClassCode.h"

#include "warehouses.h"
#include "warehouse.h"
#include "locations.h"

#include "dspUnbalancedQOHByClassCode.h"
#include "updateABCClass.h"
#include "updateCycleCountFrequency.h"
#include "updateItemSiteLeadTimes.h"
#include "updateReorderLevels.h"
#include "updateOUTLevelByItem.h"
#include "updateOUTLevels.h"
#include "updateOUTLevelsByClassCode.h"
#include "summarizeInvTransByClassCode.h"
#include "createItemSitesByClassCode.h"

#include "setup.h"

#include "menuInventory.h"

menuInventory::menuInventory(GUIClient *Pparent) :
  QObject(Pparent)
{
  setObjectName("imModule");
  parent = Pparent;

  toolBar = new QToolBar(tr("Inventory Tools"));
  toolBar->setObjectName("Inventory Tools");
  toolBar->setIconSize(QSize(32, 32));
  parent->addToolBar(toolBar);

  mainMenu                 = new QMenu(parent);
  itemSitesMenu            = new QMenu(parent);
  warehouseMenu            = new QMenu(parent);
  transferOrderMenu        = new QMenu(parent);
  transactionsMenu         = new QMenu(parent);
  lotSerialControlMenu     = new QMenu(parent);
  physicalMenu             = new QMenu(parent);
  physicalCreateTagsMenu   = new QMenu(parent);
  physicalReportsMenu      = new QMenu(parent);
  physicalReportsTagsMenu  = new QMenu(parent);
  shippingMenu             = new QMenu(parent);
  shippingReportsMenu      = new QMenu(parent);
  shippingFormsMenu        = new QMenu(parent);
  receivingMenu            = new QMenu(parent);
  receivingFormsMenu       = new QMenu(parent);
  formsMenu                = new QMenu(parent);
  formsShipLabelsMenu      = new QMenu(parent);
  reportsMenu              = new QMenu(parent);
  reportsShipmentsMenu     = new QMenu(parent);
  updateItemInfoMenu       = new QMenu(parent);
  updateItemInfoReorderMenu= new QMenu(parent);
  updateItemInfoOutMenu    = new QMenu(parent);
  utilitiesMenu            = new QMenu(parent);

  mainMenu->setObjectName("menu.im");
  warehouseMenu->setObjectName("menu.im.warehouse");
  transferOrderMenu->setObjectName("menu.im.transferorder");
  transactionsMenu->setObjectName("menu.im.transactions");
  lotSerialControlMenu->setObjectName("menu.im.lotserialcontrol");
  physicalMenu->setObjectName("menu.im.physical");
  physicalCreateTagsMenu->setObjectName("menu.im.physicalcreatetags");
  physicalReportsMenu->setObjectName("menu.im.physicalreports");
  physicalReportsTagsMenu->setObjectName("menu.im.physicalreportstags");
  shippingMenu->setObjectName("menu.im.shipping");
  shippingReportsMenu->setObjectName("menu.im.shippingreports");
  shippingFormsMenu->setObjectName("menu.im.shippingforms");
  receivingMenu->setObjectName("menu.im.receiving");
  receivingFormsMenu->setObjectName("menu.im.receivingforms");
  formsMenu->setObjectName("menu.im.forms");
  formsShipLabelsMenu->setObjectName("menu.im.formsshiplabels");
  reportsMenu->setObjectName("menu.im.reports");
  reportsShipmentsMenu->setObjectName("menu.im.reportsshipments");
  updateItemInfoMenu->setObjectName("menu.im.updateiteminfo");
  updateItemInfoReorderMenu->setObjectName("menu.im.updateiteminforeorder");
  updateItemInfoOutMenu->setObjectName("menu.im.updateiteminfoout");
  utilitiesMenu->setObjectName("menu.im.utilities");

  //Handle single vs. multi-warehouse scenario
  const char *whsModule;
  QString whsLabel;
  if (_metrics->boolean("MultiWhs"))
  {
    whsModule="im.warehouses";
    whsLabel=tr("&List...");
  }
  else
  {
    whsModule = "im.warehouse"; // must be char*
    whsLabel = tr("&Maintain...");
  }

  actionProperties acts[] = {
    //  Inventory |  Transfer Orders
    // TODO: switch between visibility of Inter-Warehouse Transfer and Transfer Orders based on config param?
    { "menu",                     tr("Transfer &Order"),     (char*)transferOrderMenu,      mainMenu,          "true",                                      NULL, NULL, _metrics->boolean("MultiWhs"), NULL},
    { "im.interWarehouseTransfer",tr("&New..."),             SLOT(sNewTransferOrder()),     transferOrderMenu, "MaintainTransferOrders",                    NULL, NULL, _metrics->boolean("MultiWhs"), NULL },
    { "im.transferOrders",        tr("&List Open..."),       SLOT(sTransferOrders()),       transferOrderMenu, "ViewTransferOrders MaintainTransferOrders", NULL, NULL, _metrics->boolean("MultiWhs"), NULL },
    { "separator",                NULL,                      NULL,                          transferOrderMenu, "true",                                      NULL, NULL, true,                          NULL},
    { "im.releaseTransferOrders", tr("&Release by Agent..."),SLOT(sReleaseTransferOrders()),transferOrderMenu, "ReleaseTransferOrders",                     NULL, NULL, _metrics->boolean("MultiWhs"), NULL },

    //  Inventory | Physical Inventory
    { "menu",                           tr("&Physical Inventory"),             (char*)physicalMenu,                  mainMenu,               "true",            NULL, NULL, true, NULL },

    //  Inventory | Physical Inventory | Create Count Tags
    { "menu",                           tr("&Create Count Tags"),              (char*)physicalCreateTagsMenu,        physicalMenu,           "true",            NULL, NULL, true, NULL },
    { "im.createCountTagsByPlannerCode",tr("by &Planner Code..."),             SLOT(sCreateCountTagsByPlannerCode()),physicalCreateTagsMenu, "IssueCountTags",  NULL, NULL, true, NULL },
    { "im.createCountTagsByClassCode",  tr("by &Class Code..."),               SLOT(sCreateCountTagsByClassCode()),  physicalCreateTagsMenu, "IssueCountTags",  NULL, NULL, true, NULL },
    { "im.createCountTagsByItem",       tr("by &Item..."),                     SLOT(sCreateCountTagsByItem()),       physicalCreateTagsMenu, "IssueCountTags",  NULL, NULL, true, NULL },
    { "im.createCycleCountTags",        tr("Create C&ycle Count Tags..."),     SLOT(sCreateCycleCountTags()),        physicalMenu,           "IssueCountTags",  NULL, NULL, true, NULL },
    { "separator",                      NULL,                                  NULL,                                 physicalMenu,           "true",            NULL, NULL, true, NULL },
    { "im.enterCountSlip",              tr("Enter Count &Slip..."),            SLOT(sEnterCountSlip()),              physicalMenu,           "EnterCountSlips", NULL, NULL, true, NULL },
    { "im.enterCountTag",               tr("Enter Count &Tag..."),             SLOT(sEnterCountTags()),              physicalMenu,           "EnterCountTags",  NULL, NULL, true, NULL },
    { "im.enterMiscInventoryCount",     tr("Enter &Misc. Inventory Count..."), SLOT(sEnterMiscCount()),              physicalMenu,           "EnterMiscCounts", NULL, NULL, true, NULL },
    { "im.zeroUncountedCountTags",      tr("&Zero Uncounted Count Tags..."),   SLOT(sZeroUncountedTagsByWarehouse()),physicalMenu,           "ZeroCountTags",   NULL, NULL, true, NULL },
    { "separator",                      NULL,                                  NULL,                                 physicalMenu,           "true",            NULL, NULL, true, NULL },
    { "im.thawItemSitesByClassCode",    tr("Tha&w Item Sites..."),             SLOT(sThawItemSitesByClassCode()),    physicalMenu,           "ThawInventory",   NULL, NULL, true, NULL },
    { "separator",                      NULL,                                  NULL,                                 physicalMenu,           "true",            NULL, NULL, true, NULL },
    { "im.postCountSlips",              tr("Post Count S&lips..."),            SLOT(sPostCountSlipsByWarehouse()),   physicalMenu,           "PostCountSlips",  NULL, NULL, true, NULL },
    { "im.postCountTags",               tr("Post Count T&ags..."),             SLOT(sPostCountTags()),               physicalMenu,           "PostCountTags",   NULL, NULL, true, NULL },
    { "im.purgeCountSlips",             tr("&Purge Posted Count Slips..."),    SLOT(sPurgePostedCountSlips()),       physicalMenu,           "PurgeCountSlips", NULL, NULL, true, NULL },
    { "im.purgeCountTags",              tr("P&urge Posted Count Tags..."),     SLOT(sPurgePostedCountTags()),        physicalMenu,           "PurgeCountTags",  NULL, NULL, true, NULL },
    { "separator",                      NULL,                                  NULL,                                 physicalMenu,           "true",            NULL, NULL, true, NULL },      

    //  Inventory | Physical Inventory | Reports
    { "menu",                           tr("&Reports"),                   (char*)physicalReportsMenu,       physicalMenu,        "true",                NULL, NULL, true, NULL },
    { "im.dspFrozenItemSites",          tr("&Frozen Item Sites..."),      SLOT(sDspFrozenItemSites()),      physicalReportsMenu, "ViewItemSites",       NULL, NULL, true, NULL },
    { "separator",                      NULL,                             NULL,                             physicalReportsMenu, "true",                NULL, NULL, true, NULL },
    { "im.dspCountSlipEditList",        tr("Count S&lip Edit List..."),   SLOT(sDspCountSlipEditList()),    physicalReportsMenu, "ViewCountTags",       NULL, NULL, true, NULL },
    { "im.dspCountTagEditList",         tr("Count Ta&g Edit List..."),    SLOT(sDspCountTagEditList()),     physicalReportsMenu, "ViewCountTags",       NULL, NULL, true, NULL },
    { "separator",                      NULL,                             NULL,                             physicalReportsMenu, "true",                NULL, NULL, true, NULL },
    { "im.dspCountSlipsByWarehouse",    tr("Count &Slips..."),            SLOT(sDspCountSlipsByWarehouse()),physicalReportsMenu, "ViewCountTags",       NULL, NULL, true, NULL },

    //  Inventory | Physical Inventory | Reports | Count Tags
    { "menu",                           tr("Count &Tags"),       (char*)physicalReportsTagsMenu,   physicalReportsMenu,     "ViewCountTags",  NULL, NULL, true, NULL },
    { "im.dspCountTagsByItem",          tr("by &Item..."),       SLOT(sDspCountTagsByItem()),      physicalReportsTagsMenu, "ViewCountTags",  NULL, NULL, true, NULL },
    { "im.dspCountTagsByWarehouse",     tr("by Sit&e..."),       SLOT(sDspCountTagsByWarehouse()), physicalReportsTagsMenu, "ViewCountTags",  NULL, NULL, true, NULL },
    { "im.dspCountTagsByClassCode",     tr("by &Class Code..."), SLOT(sDspCountTagsByClassCode()), physicalReportsTagsMenu, "ViewCountTags",  NULL, NULL, true, NULL },

    { "separator",       NULL,                             NULL,                  mainMenu,      "true",          NULL, NULL, true, NULL },

    //  Inventory | Receiving
    { "menu",            tr("R&eceiving"),                 (char*)receivingMenu,  mainMenu,      "true",          NULL, NULL, true, NULL },
    { "sr.enterReceipt", tr("&New Receipt..."),            SLOT(sEnterReceipt()), receivingMenu, "EnterReceipts", NULL, NULL, true, NULL },
    { "sr.postReceipts", tr("&List Unposted Receipts..."), SLOT(sPostReceipts()), receivingMenu, "EnterReceipts", QPixmap(":/images/postReceipts.png"), toolBar,  true, tr("List Unposted Receipts") },
    { "separator",       NULL,                             NULL,                  receivingMenu, "true",          NULL, NULL, true, NULL },
    { "sr.enterReturn",  tr("Purchase Order &Return..."),  SLOT(sEnterReturn()),  receivingMenu, "EnterReturns",  NULL, NULL, true, NULL },
    { "separator",       NULL,                             NULL,                  receivingMenu, "true",          NULL, NULL, true, NULL },

    //  Inventory | Receiving | Forms
    { "menu",                        tr("&Forms"),                     (char*)receivingFormsMenu,         receivingMenu,      "true",         NULL, NULL, true, NULL },
    { "sr.printReceivingLabelsByPo", tr("&Print Receiving Labels..."), SLOT(sPrintReceivingLabelsByPo()), receivingFormsMenu, "ViewShipping", NULL, NULL, true, NULL },

    //  Inventory | Shipping
    { "menu",                        tr("&Shipping"),                      (char*)shippingMenu,           mainMenu,     "true",                 NULL, NULL, true, NULL },
    { "sr.issueToShipping",          tr("&Issue to Shipping..."),          SLOT(sIssueStockToShipping()), shippingMenu, "IssueStockToShipping", QPixmap(":/images/issueStockToShipping.png"), toolBar,  true, tr("Issue to Shipping") },
    { "sr.maintainShippingContents", tr("&Maintain Shipping Contents..."), SLOT(sDspShippingContents()),  shippingMenu, "ViewShipping",         NULL, NULL, true, NULL },
    { "separator",                   NULL,                                 NULL,                          shippingMenu, "true",                 NULL, NULL, true, NULL },
    { "sr.shipOrder",                tr("&Ship Order..."),                 SLOT(sShipOrders()),           shippingMenu, "ShipOrders",           NULL, NULL, true, NULL },
    { "sr.recallOrdersToShipping",   tr("R&ecall Orders to Shipping..."),  SLOT(sRecallOrders()),         shippingMenu, "RecallOrders",         NULL, NULL, true, NULL },

    { "separator",                   NULL,                                 NULL,                          shippingMenu,  "true",                NULL, NULL,  true, NULL },

    // Inventory | Shipping | Forms
    { "menu",                              tr("&Forms"),                                   (char*)shippingFormsMenu,                shippingMenu,      "true",                                          NULL, NULL, true, NULL },
    { "sr.packingListBatch",               tr("Packing List &Batch..."),                   SLOT(sPackingListBatch()),               shippingFormsMenu, "MaintainPackingListBatch ViewPackingListBatch", NULL, NULL, true, NULL },
    { "sr.printPackingListBatchByShipvia", tr("Print Packing List Batch by Ship &Via..."), SLOT(sPrintPackingListBatchByShipvia()), shippingFormsMenu, "PrintPackingLists",  NULL, NULL, true, NULL },
    { "sr.printPackingList",               tr("&Packing List..."),                         SLOT(sPrintPackingLists()),              shippingFormsMenu, "PrintPackingLists",  NULL, NULL, true, NULL },
    { "separator",                         NULL,                                           NULL,                                    shippingFormsMenu, "true",               NULL, NULL, true, NULL },
    { "sr.printShippingForm",              tr("&Shipping Form..."),                        SLOT(sPrintShippingForm()),              shippingFormsMenu, "PrintBillsOfLading", NULL, NULL, true, NULL },
    { "sr.printShippingForms",             tr("S&hipping Forms..."),                       SLOT(sPrintShippingForms()),             shippingFormsMenu, "PrintBillsOfLading", NULL, NULL, true, NULL },

    // Inventory | Shipping | Forms | Shipping Labels
    { "menu",                            tr("Shipping &Labels"),  (char*)formsShipLabelsMenu,            shippingFormsMenu,   "true",               NULL, NULL, true, NULL },
    { "sr.printShippingLabelsBySo",      tr("by &Sales Order..."),      SLOT(sPrintShippingLabelsBySo()),      formsShipLabelsMenu, "ViewShipping",       NULL, NULL, true, NULL },
    { "sr.printShippingLabelsByTo",      tr("by &Transfer Order..."),      SLOT(sPrintShippingLabelsByTo()),      formsShipLabelsMenu, "ViewTransferOrders", NULL, NULL, true, NULL },
    { "sr.printShippingLabelsByInvoice", tr("by &Invoice..."),    SLOT(sPrintShippingLabelsByInvoice()), formsShipLabelsMenu, "ViewShipping",       NULL, NULL, true, NULL },

    // Inventory | Shipping | Reports
    { "menu",                               tr("&Reports"),                (char*)shippingReportsMenu,               shippingMenu,        "true",            NULL, NULL, true, NULL },

    // Inventory | Shipping | Reports | Backlog
    { "sr.dspBacklog",                tr("&Backlog..."),             SLOT(sDspBacklog()),                shippingReportsMenu,  "ViewSalesOrders", NULL, NULL, true, NULL },

    { "separator", NULL, NULL, shippingReportsMenu, "true", NULL, NULL, true, NULL },

    // Inventory | Shipping | Reports | Shipments
    { "menu",                        tr("&Shipments"),         (char*)reportsShipmentsMenu,       shippingReportsMenu,  "true",         NULL, NULL, true, NULL },
    { "sr.dspShipmentsBySalesOrder", tr("by Sales &Order..."), SLOT(sDspShipmentsBySalesOrder()), reportsShipmentsMenu, "ViewShipping", NULL, NULL, true, NULL },
    { "sr.dspShipmentsByDate",       tr("by &Date..."),        SLOT(sDspShipmentsByDate()),       reportsShipmentsMenu, "ViewShipping", NULL, NULL, true, NULL },
    { "sr.dspShipmentsByShipment",   tr("by &Shipment..."),    SLOT(sDspShipmentsByShipment()),   reportsShipmentsMenu, "ViewShipping", NULL, NULL, true, NULL },

    //  Inventory |  Transactions
    { "menu",                           tr("&Transactions"),         (char*)transactionsMenu,    mainMenu,         "true",                      NULL, NULL, true, NULL },
    { "im.miscellaneousAdjustment",     tr("&Adjustment..."),        SLOT(sAdjustmentTrans()),   transactionsMenu, "CreateAdjustmentTrans",     NULL, NULL, true, NULL },

    { "im.scrap",                       tr("&Scrap..."),             SLOT(sScrapTrans()),        transactionsMenu, "CreateScrapTrans",          NULL, NULL, true, NULL },
    { "im.expense",                     tr("E&xpense..."),           SLOT(sExpenseTrans()),      transactionsMenu, "CreateExpenseTrans",        NULL, NULL, true, NULL },
    { "im.materialReceipt",             tr("&Material Receipt..."),  SLOT(sReceiptTrans()),      transactionsMenu, "CreateReceiptTrans",        NULL, NULL, true, NULL },
    { "im.transform",                   tr("Trans&form..."),         SLOT(sTransformTrans()),    transactionsMenu, "CreateTransformTrans",      NULL, NULL, _metrics->boolean("Transforms"), NULL },
    { "separator",                      NULL,                        NULL,                       transactionsMenu, "true",                      NULL, NULL, true, NULL },
    { "im.resetQOHBalances",            tr("&Reset QOH Balances..."),SLOT(sResetQOHBalances()),  transactionsMenu, "CreateAdjustmentTrans",     NULL, NULL, true, NULL },
    { "separator",                      NULL,                        NULL,                       transactionsMenu, "true",                      NULL, NULL, true, NULL },
    { "im.interWarehouseTransfer",      tr("&Site Transfer..."),     SLOT(sTransferTrans()),     transactionsMenu, "CreateInterWarehouseTrans", NULL, NULL, _metrics->boolean("MultiWhs"), NULL },
    { "im.relocateInventory",           tr("Re&locate Inventory..."),SLOT(sRelocateInventory()), transactionsMenu, "RelocateInventory",         NULL, NULL, true, NULL },

    {  "separator",                     NULL,                        NULL,                       mainMenu,         "true", NULL, NULL, true, NULL },

    // Inventory | Forms
    { "menu",                           tr("&Forms"),                (char*)formsMenu,                    mainMenu,  "true",            NULL, NULL, true, NULL },
    { "im.printItemLabelsByClassCode",  tr("Print &Item Labels..."), SLOT(sPrintItemLabelsByClassCode()), formsMenu, "ViewItemSites",   NULL, NULL, true, NULL },

    //  Inventory | Reports
    { "menu",                           tr("&Reports"),                   (char*)reportsMenu,                   mainMenu,       "true", NULL, NULL, true, NULL },

    { "im.dspValidLocationsByItem",     tr("&Valid Locations by Item..."),SLOT(sDspValidLocationsByItem()),     reportsMenu,          "ViewLocations",  NULL, NULL, true, NULL },
    {  "separator",                     NULL,                             NULL,                                 reportsMenu,          "true",           NULL, NULL, true, NULL },

    //  Inventory | Reports | Quantities On Hand
    { "im.dspQOH",                    tr("&Quantities On Hand..."),                             SLOT(sDspQOH()),                reportsMenu, "ViewQOH",      NULL, NULL, true, NULL },
    { "im.dspQOHByLocation",          tr("Quantities On Hand By &Location..."),                 SLOT(sDspQOHByLocation()),      reportsMenu, "ViewQOH",      NULL, NULL, true, NULL },

    { "im.dspLocationLotSerialDetail",tr("&Location/Lot/Serial # Detail..."),SLOT(sDspLocationLotSerialDetail()),       reportsMenu,    "ViewQOH",      NULL, NULL, _metrics->boolean("LotSerialControl"), NULL },
    { "im.dspExpiredInventory",       tr("&Expired Inventory..."),           SLOT(sDspExpiredInventoryByClassCode()),   reportsMenu,    "ViewQOH",      NULL, NULL, _metrics->boolean("LotSerialControl"), NULL },

    { "im.dspSlowMovingInventory",    tr("Slow &Moving Inventory..."),       SLOT(sDspSlowMovingInventoryByClassCode()),reportsMenu,    "ViewQOH",      NULL, NULL, true, NULL },
    {  "separator",                   NULL,                                  NULL,                                      reportsMenu,    "true",         NULL, NULL, true, NULL },

    //  Inventory| Reports | Inventory Availability
    { "im.dspInventoryAvailability",       tr("Inventory &Availability..."), SLOT(sDspInventoryAvailability()), reportsMenu, "ViewInventoryAvailability", QPixmap(":/images/dspInventoryAvailabilityByPlannerCode.png"), toolBar, true, tr("Inventory Availability by Planner Code") },
    { "im.dspSubstituteAvailabilityByRootItem",         tr("&Substitute Availability..."),       SLOT(sDspSubstituteAvailabilityByRootItem()), reportsMenu, "ViewInventoryAvailability",        NULL, NULL, true, NULL },
    {  "separator",                   NULL,                                  NULL,                                      reportsMenu,    "true",         NULL, NULL, true, NULL },

    //  Inventory| Reports | Inventory History
    { "im.dspInventoryHistory",                   tr("&History..."), SLOT(sDspInventoryHistory()), reportsMenu, "ViewInventoryHistory", NULL, NULL, true, NULL },
    { "im.dspDetailedInventoryHistoryByLocation",                  tr("History by Lo&cation..."), SLOT(sDspDetailedInventoryHistoryByLocation()), reportsMenu, "ViewInventoryHistory", NULL, NULL, true, NULL },
    { "im.dspDetailedInventoryHistoryByLot/SerialNumber",      tr("History by &Lot/Serial #..."), SLOT(sDspDetailedInventoryHistoryByLotSerial()),reportsMenu, "ViewInventoryHistory", NULL, NULL, _metrics->boolean("LotSerialControl"), NULL },
    { "im.dspItemUsageStatistics",                          tr("&Usage Statistics..."), SLOT(sDspItemUsageStatistics()), reportsMenu, "ViewInventoryHistory", NULL, NULL, true, NULL },

    { "im.dspTimePhasedItemUsageStatisticsByItem",tr("Time &Phased Usage Statistics..."), SLOT(sDspTimePhasedUsageStatisticsByItem()),reportsMenu, "ViewInventoryHistory",       NULL, NULL, true, NULL },

    { "separator",                      NULL,                             NULL,                         mainMenu, "true", NULL, NULL, true, NULL },

    // Inventory | Warehouse
    { "menu",                   tr("&Site"),            (char*)warehouseMenu,        mainMenu,      "true",                                             NULL, NULL, true, NULL },
    { whsModule,                whsLabel,               SLOT(sWarehouses()),         warehouseMenu, "MaintainWarehouses ViewWarehouses",                NULL, NULL, true, NULL },
    { "im.warehousesLocations", tr("L&ocations..."),    SLOT(sWarehouseLocations()), warehouseMenu, "MaintainLocations ViewLocations",                  NULL, NULL, true, NULL },

    // Inventory | Item Site
    { "menu",                           tr("&Item Site"),       (char*)itemSitesMenu,   mainMenu,       "true",              NULL, NULL, true, NULL },
    { "im.newItemSite",                 tr("&New..."),          SLOT(sNewItemSite()),   itemSitesMenu,  "MaintainItemSites", NULL, NULL, true, NULL },
    { "im.listItemSites",               tr("&List..."),         SLOT(sItemSites()),     itemSitesMenu,  "MaintainItemSites ViewItemSites", QPixmap(":/images/itemSites.png"), toolBar, true, tr("List Item Sites") },                                                  
    { "separator", NULL, NULL, itemSitesMenu,   "true", NULL, NULL, true, NULL },
    { "im.itemAvailabilityWorkbench",   tr("&Workbench..."),    SLOT(sDspItemAvailabilityWorkbench()),  itemSitesMenu, "ViewItemAvailabilityWorkbench", QPixmap(":/images/itemAvailabilityWorkbench.png"), toolBar, true, tr("Item Availability Workbench") },

    //  Inventory | Lot/Serial Control
    { "menu",                           tr("&Lot/Serial Control"),      (char*)lotSerialControlMenu,    mainMenu, "true",       NULL, NULL, _metrics->boolean("LotSerialControl") , NULL },
    { "im.dspLocationLotSerialDetail",  tr("&Location Detail..."),      SLOT(sDspLocationLotSerialDetail()), lotSerialControlMenu, "ViewQOH",   NULL, NULL,  _metrics->boolean("LotSerialControl"), NULL },
    { "im.dspDetailedInventoryHistoryByLot/SerialNumber", tr("&Detailed Inventory History..."), SLOT(sDspDetailedInventoryHistoryByLotSerial()), lotSerialControlMenu, "ViewInventoryHistory",  NULL, NULL, _metrics->boolean("LotSerialControl"), NULL },
    { "separator",                      NULL,                           NULL,   lotSerialControlMenu,   "true", NULL, NULL,  _metrics->boolean("LotSerialControl") , NULL },

    { "im.reassignLotSerialNumber",     tr("&Reassign Lot/Serial #..."),        SLOT(sReassignLotSerialNumber()), lotSerialControlMenu, "ReassignLotSerial",    NULL, NULL, _metrics->boolean("LotSerialControl"), NULL },

    { "separator", NULL, NULL, mainMenu,        "true", NULL, NULL, true, NULL },

    // Inventory | Utilities
    { "menu",                                     tr("&Utilities"),                       (char*)utilitiesMenu,                     mainMenu,       "true",                    NULL, NULL, true, NULL },
    { "im.dspUnbalancedQOHByClassCode",           tr("U&nbalanced QOH..."),               SLOT(sDspUnbalancedQOHByClassCode()),     utilitiesMenu,  "ViewItemSites",           NULL, NULL, true, NULL },
    { "im.adjustInvValue",                        tr("Adjust Avg. Cost Value..."),        SLOT(sAdjustInvValue()),                  utilitiesMenu,  "CreateAdjustmentTrans ViewCosts", NULL, NULL, _metrics->boolean("AllowAvgCostMethod"), NULL },
    { "separator",                                NULL,                                   NULL,                                     utilitiesMenu,  "true",                    NULL, NULL, true, NULL },

    // Inventory | Utilities | Update Item Controls
    { "menu",                                     tr("&Update Item Controls"),            (char*)updateItemInfoMenu,                utilitiesMenu,      "true",                NULL, NULL, true, NULL },
    { "im.updateABCClass",                        tr("&ABC Class..."),                    SLOT(sUpdateABCClass()),                  updateItemInfoMenu, "UpdateABCClass",      NULL, NULL, true, NULL },
    { "im.updateCycleCountFrequency",             tr("&Cycle Count Frequency..."),        SLOT(sUpdateCycleCountFreq()),            updateItemInfoMenu, "UpdateCycleCountFreq",NULL, NULL, true, NULL },
    { "im.updateItemSiteLeadTimes",               tr("&Item Site Lead Times..."),         SLOT(sUpdateItemSiteLeadTimes()),         updateItemInfoMenu, "UpdateLeadTime",      NULL, NULL, true, NULL },

    //  Inventory | Utilities | Update Item Controls | Update Reorder Levels
    { "menu",                                     tr("&Reorder Levels"),                  (char*)updateItemInfoReorderMenu,         updateItemInfoMenu,       "UpdateReorderLevels", NULL, NULL, true, NULL },
    { "im.updateReorderLevelsByItem",             tr("by &Item..."),                      SLOT(sUpdateReorderLevelByItem()),        updateItemInfoReorderMenu,"UpdateReorderLevels", NULL, NULL, true, NULL },
    { "im.updateReorderLevelsByPlannerCode",      tr("by &Planner Code..."),              SLOT(sUpdateReorderLevelsByPlannerCode()),updateItemInfoReorderMenu,"UpdateReorderLevels", NULL, NULL, true, NULL },
    { "im.updateReorderLevelsByClassCode",        tr("by &Class Code..."),                SLOT(sUpdateReorderLevelsByClassCode()),  updateItemInfoReorderMenu,"UpdateReorderLevels", NULL, NULL, true, NULL },

    //  Inventory | Utilities | Update Item Controls | Update Order Up To Levels
    { "menu",                                     tr("&Order Up To Levels"),              (char*)updateItemInfoOutMenu,             updateItemInfoMenu,    "UpdateOUTLevels",  NULL, NULL, true, NULL },
    { "im.updateOrderUpToLevelsByItem",           tr("by &Item..."),                      SLOT(sUpdateOUTLevelByItem()),            updateItemInfoOutMenu, "UpdateOUTLevels",  NULL, NULL, true, NULL },
    { "im.updateOrderUpToLevelsByPlannerCode",    tr("by &Planner Code..."),              SLOT(sUpdateOUTLevelsByPlannerCode()),    updateItemInfoOutMenu, "UpdateOUTLevels",  NULL, NULL, true, NULL },
    { "im.updateOrderUpToLevelsByClassCode",      tr("by &Class Code..."),                SLOT(sUpdateOUTLevelsByClassCode()),      updateItemInfoOutMenu, "UpdateOUTLevels",  NULL, NULL, true, NULL },

    { "im.summarizeTransactionHistoryByClassCode",tr("Summarize &Transaction History..."),SLOT(sSummarizeInvTransByClassCode()),    utilitiesMenu, "SummarizeInventoryTransactions", NULL, NULL, true, NULL },
    { "im.createItemSitesByClassCode",            tr("&Create Item Sites..."),            SLOT(sCreateItemSitesByClassCode()),      utilitiesMenu, "MaintainItemSites",              NULL, NULL, true, NULL },
    { "separator",                                NULL,                                   NULL,                                     utilitiesMenu, "true",                           NULL, NULL, true, NULL },
    { "sr.externalShipping",          tr("Maintain E&xternal Shipping Records..."),       SLOT(sExternalShipping()),                utilitiesMenu, "MaintainExternalShipping", NULL, NULL, true, NULL },
    // Setup
    { "im.setup",	                          tr("&Setup..."),                        SLOT(sSetup()),                           mainMenu,	   "true",                     NULL, NULL, true, NULL}

  };

  addActionsToMenu(acts, sizeof(acts) / sizeof(acts[0]));



#if 0
  mainMenu->insertItem(tr("&Graphs"),                 graphsMenu           );
#endif

  parent->populateCustomMenu(mainMenu, "Inventory");
  QAction * m = parent->menuBar()->addMenu(mainMenu);
  if(m)
    m->setText(tr("&Inventory"));

//  Create connections to the module specific inputManager SIGNALS
  parent->inputManager()->notify(cBCLocationContents, this, this, SLOT(sCatchLocationContents(int)));
  parent->inputManager()->notify(cBCCountTag, this, this, SLOT(sCatchCountTag(int)));
}

void menuInventory::addActionsToMenu(actionProperties acts[], unsigned int numElems)
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

void menuInventory::sNewItemSite()
{
  ParameterList params;
  params.append("mode", "new");

  itemSite newdlg(parent, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void menuInventory::sItemSites()
{
  omfgThis->handleNewWindow(new itemSites());
}

void menuInventory::sAdjustmentTrans()
{
  ParameterList params;
  params.append("mode", "new");

  adjustmentTrans *newdlg = new adjustmentTrans();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void menuInventory::sAdjustInvValue()
{
  ParameterList params;
  params.append("mode", "new");

  adjustInvValue *newdlg = new adjustInvValue();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void menuInventory::sTransferTrans()
{
  ParameterList params;
  params.append("mode", "new");

  transferTrans *newdlg = new transferTrans();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void menuInventory::sNewTransferOrder()
{
  transferOrder::newTransferOrder(-1, -1);
}

void menuInventory::sTransferOrders()
{
  omfgThis->handleNewWindow(new transferOrders());
}

void menuInventory::sReleaseTransferOrders()
{
  omfgThis->handleNewWindow(new releaseTransferOrdersByAgent());
}

void menuInventory::sReceiptTrans()
{
  ParameterList params;
  params.append("mode", "new");

  materialReceiptTrans *newdlg = new materialReceiptTrans();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void menuInventory::sTransformTrans()
{
  ParameterList params;
  params.append("mode", "new");

  transformTrans *newdlg = new transformTrans();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void menuInventory::sScrapTrans()
{
  ParameterList params;
  params.append("mode", "new");

  scrapTrans *newdlg = new scrapTrans();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void menuInventory::sExpenseTrans()
{
  ParameterList params;
  params.append("mode", "new");

  expenseTrans *newdlg = new expenseTrans();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void menuInventory::sResetQOHBalances()
{
  resetQOHBalances(parent, "", TRUE).exec();
}

void menuInventory::sRelocateInventory()
{
  relocateInventory(parent, "", TRUE).exec();
}


//  Lot/Serial Control
void menuInventory::sReassignLotSerialNumber()
{
  reassignLotSerial newdlg(parent, "", TRUE);
  newdlg.exec();
}


void menuInventory::sCreateCountTagsByClassCode()
{
  ParameterList params;
  params.append("classcode");

  createCountTagsByParameterList newdlg(parent, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void menuInventory::sCreateCountTagsByPlannerCode()
{
  ParameterList params;
  params.append("plancode");

  createCountTagsByParameterList newdlg(parent, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void menuInventory::sCreateCountTagsByItem()
{
  createCountTagsByItem(parent, "", TRUE).exec();
}

void menuInventory::sCreateCycleCountTags()
{
  createCycleCountTags(parent, "", TRUE).exec();
}

void menuInventory::sEnterCountSlip()
{
  ParameterList params;
  params.append("mode", "new");

  countSlip newdlg(parent, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void menuInventory::sEnterCountTags()
{
  ParameterList params;
  params.append("mode", "new");

  countTag newdlg(parent, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void menuInventory::sEnterMiscCount()
{
  enterMiscCount(parent, "", TRUE).exec();
}

void menuInventory::sZeroUncountedTagsByWarehouse()
{
  zeroUncountedCountTagsByWarehouse(parent, "", TRUE).exec();
}

void menuInventory::sThawItemSitesByClassCode()
{
  thawItemSitesByClassCode(parent, "", TRUE).exec();
}

void menuInventory::sPostCountSlipsByWarehouse()
{
  postCountSlips(parent, "", TRUE).exec();
}

void menuInventory::sPostCountTags()
{
  postCountTags(parent, "", TRUE).exec();
}

void menuInventory::sPurgePostedCountSlips()
{
  purgePostedCountSlips(parent, "", TRUE).exec();
}

void menuInventory::sPurgePostedCountTags()
{
  purgePostedCounts(parent, "", TRUE).exec();
}


void menuInventory::sPackingListBatch()
{
  omfgThis->handleNewWindow(new packingListBatch());
}

void menuInventory::sDspShippingContents()
{
  omfgThis->handleNewWindow(new maintainShipping());
}

void menuInventory::sIssueStockToShipping()
{
  omfgThis->handleNewWindow(new issueToShipping());
}

void menuInventory::sPrintPackingLists()
{
  printPackingList(parent, "", TRUE).exec();
}

void menuInventory::sPrintPackingListBatchByShipvia()
{
  printPackingListBatchByShipvia(parent, "", TRUE).exec();
}

void menuInventory::sPrintShippingForm()
{
  printShippingForm(parent).exec();
}

void menuInventory::sPrintShippingForms()
{
  printShippingForms(parent, "", TRUE).exec();
}

void menuInventory::sPrintShippingLabelsBySo()
{
  printLabelsBySo(parent, "", TRUE).exec();
}

void menuInventory::sPrintShippingLabelsByTo()
{
  printLabelsByTo(parent, "", TRUE).exec();
}

void menuInventory::sPrintShippingLabelsByInvoice()
{
  printLabelsByInvoice(parent, "", TRUE).exec();
}

void menuInventory::sPrintReceivingLabelsByPo()
{
  printLabelsByOrder(parent, "", TRUE).exec();
}


void menuInventory::sShipOrders()
{
  shipOrder(parent, "", TRUE).exec();
}

void menuInventory::sRecallOrders()
{
  omfgThis->handleNewWindow(new recallOrders());
}

void menuInventory::sEnterReceipt()
{
  ParameterList params;
  params.append("mode", "new");

  enterPoReceipt *newdlg = new enterPoReceipt();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void menuInventory::sEnterReturn()
{
  ParameterList params;
  params.append("mode", "new");

  enterPoReturn *newdlg = new enterPoReturn();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void menuInventory::sPostReceipts()
{
  //ParameterList params;

  unpostedPoReceipts *newdlg = new unpostedPoReceipts();
  //newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void menuInventory::sAddRate()
{
}

void menuInventory::sDspRatesByDestination()
{
}

void menuInventory::sDspBacklog()
{
  omfgThis->handleNewWindow(new dspBacklog());
}

void menuInventory::sDspSummarizedBacklogByWarehouse()
{
  omfgThis->handleNewWindow(new dspSummarizedBacklogByWarehouse());
}

void menuInventory::sDspShipmentsBySalesOrder()
{
  omfgThis->handleNewWindow(new dspShipmentsBySalesOrder());
}

void menuInventory::sDspShipmentsByDate()
{
  omfgThis->handleNewWindow(new dspShipmentsByDate());
}

void menuInventory::sDspShipmentsByShipment()
{
  omfgThis->handleNewWindow(new dspShipmentsByShipment());
}


void menuInventory::sDspFrozenItemSites()
{
  omfgThis->handleNewWindow(new dspFrozenItemSites());
}

void menuInventory::sDspCountSlipEditList()
{
  omfgThis->handleNewWindow(new dspCountSlipEditList());
}

void menuInventory::sDspCountTagEditList()
{
  omfgThis->handleNewWindow(new dspCountTagEditList());
}

void menuInventory::sDspCountSlipsByWarehouse()
{
  omfgThis->handleNewWindow(new dspCountSlipsByWarehouse());
}

void menuInventory::sDspCountTagsByItem()
{
  omfgThis->handleNewWindow(new dspCountTagsByItem());
}

void menuInventory::sDspCountTagsByWarehouse()
{
  omfgThis->handleNewWindow(new dspCountTagsByWarehouse());
}

void menuInventory::sDspCountTagsByClassCode()
{
  omfgThis->handleNewWindow(new dspCountTagsByClassCode());
}

void menuInventory::sDspItemAvailabilityWorkbench()
{
  omfgThis->handleNewWindow(new itemAvailabilityWorkbench());
}

void menuInventory::sDspValidLocationsByItem()
{
  omfgThis->handleNewWindow(new dspValidLocationsByItem());
}

void menuInventory::sDspQOH()
{
  omfgThis->handleNewWindow(new dspQOH());
}

void menuInventory::sDspQOHByLocation()
{
  omfgThis->handleNewWindow(new dspQOHByLocation());
}

void menuInventory::sDspLocationLotSerialDetail()
{
  omfgThis->handleNewWindow(new dspInventoryLocator());
}

void menuInventory::sDspSlowMovingInventoryByClassCode()
{
  omfgThis->handleNewWindow(new dspSlowMovingInventoryByClassCode());
}

void menuInventory::sDspExpiredInventoryByClassCode()
{
  omfgThis->handleNewWindow(new dspExpiredInventoryByClassCode());
}

void menuInventory::sDspInventoryAvailability()
{
  omfgThis->handleNewWindow(new dspInventoryAvailability());
}

void menuInventory::sDspSubstituteAvailabilityByRootItem()
{
  omfgThis->handleNewWindow(new dspSubstituteAvailabilityByItem());
}

void menuInventory::sDspInventoryHistory()
{
  omfgThis->handleNewWindow(new dspInventoryHistory());
}

void menuInventory::sDspDetailedInventoryHistoryByLotSerial()
{
  omfgThis->handleNewWindow(new dspDetailedInventoryHistoryByLotSerial());
}

void menuInventory::sDspDetailedInventoryHistoryByLocation()
{
  omfgThis->handleNewWindow(new dspDetailedInventoryHistoryByLocation());
}

void menuInventory::sDspItemUsageStatistics()
{
  omfgThis->handleNewWindow(new dspUsageStatistics());
}

void menuInventory::sDspTimePhasedUsageStatisticsByItem()
{
  omfgThis->handleNewWindow(new dspTimePhasedUsageStatisticsByItem());
}

void menuInventory::sPrintItemLabelsByClassCode()
{
  printItemLabelsByClassCode(parent, "", TRUE).exec();
}


//  Master Information
void menuInventory::sWarehouses()
{
  if (_metrics->boolean("MultiWhs"))
    omfgThis->handleNewWindow(new warehouses());
  else
  {
    omfgThis->handleNewWindow(new warehouse());
  }
}

void menuInventory::sWarehouseLocations()
{
  omfgThis->handleNewWindow(new locations());
}

//  Utilities
void menuInventory::sDspUnbalancedQOHByClassCode()
{
  omfgThis->handleNewWindow(new dspUnbalancedQOHByClassCode());
}

void menuInventory::sUpdateABCClass()
{
  updateABCClass(parent, "", TRUE).exec();
}

void menuInventory::sUpdateCycleCountFreq()
{
  updateCycleCountFrequency(parent, "", TRUE).exec();
}

void menuInventory::sUpdateItemSiteLeadTimes()
{
  updateItemSiteLeadTimes(parent, "", TRUE).exec();
}

void menuInventory::sUpdateReorderLevelByItem()
{
  ParameterList params;
  params.append("item");

  updateReorderLevels *newdlg = new updateReorderLevels();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void menuInventory::sUpdateReorderLevelsByPlannerCode()
{
  ParameterList params;
  params.append("plancode");

  updateReorderLevels *newdlg = new updateReorderLevels();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void menuInventory::sUpdateReorderLevelsByClassCode()
{
  ParameterList params;
  params.append("classcode");

  updateReorderLevels *newdlg = new updateReorderLevels();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void menuInventory::sUpdateOUTLevelByItem()
{
  updateOUTLevelByItem(parent, "", TRUE).exec();
}

void menuInventory::sUpdateOUTLevelsByPlannerCode()
{
  updateOUTLevels(parent, "", TRUE).exec();
}

void menuInventory::sUpdateOUTLevelsByClassCode()
{
  updateOUTLevelsByClassCode(parent, "", TRUE).exec();
}

void menuInventory::sSummarizeInvTransByClassCode()
{
  summarizeInvTransByClassCode(parent, "", TRUE).exec();
}

void menuInventory::sCreateItemSitesByClassCode()
{
  createItemSitesByClassCode(parent, "", TRUE).exec();
}

//  inputManager SIGNAL handlers
void menuInventory::sCatchLocationContents(int pLocationid)
{
  ParameterList params;
  params.append("location_id", pLocationid);
  params.append("run");

  dspQOHByLocation *newdlg = new dspQOHByLocation();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void menuInventory::sCatchCountTag(int pCnttagid)
{
  ParameterList params;
  params.append("cnttag_id", pCnttagid);
  params.append("mode", "edit");

  countTag newdlg(parent, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void menuInventory::sExternalShipping()
{
  omfgThis->handleNewWindow(new externalShippingList());
}

void menuInventory::sSetup()
{
  ParameterList params;
  params.append("module", Xt::InventoryModule);

  setup newdlg(parent);
  newdlg.set(params);
  newdlg.exec();
}
