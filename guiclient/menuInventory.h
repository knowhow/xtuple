/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef menuInventory_h
#define menuInventory_h

#include <QObject>
#include <QMenu>

class QToolBar;
class QMenu;
class GUIClient;

class menuInventory : public QObject
{
  Q_OBJECT

  struct actionProperties {
    const char*		actionName;
    const QString	actionTitle;
    const char*		slot;
    QMenu*		menu;
    QString		priv;
    QPixmap		pixmap;
    QToolBar*		toolBar;
    bool		visible;
    const QString   toolTip;
  };

  public:
    menuInventory(GUIClient *);

  public slots:
    void sNewItemSite();
    void sItemSites();

    void sAdjustmentTrans();
    void sTransferTrans();
    void sNewTransferOrder();
    void sTransferOrders();
    void sReleaseTransferOrders();
    void sReceiptTrans();
    void sScrapTrans();
    void sExpenseTrans();
    void sTransformTrans();
    void sResetQOHBalances();
    void sRelocateInventory();

    void sReassignLotSerialNumber();

    void sCreateCountTagsByClassCode();
    void sCreateCountTagsByPlannerCode();
    void sCreateCountTagsByItem();
    void sCreateCycleCountTags();
    void sEnterCountSlip();
    void sEnterCountTags();
    void sEnterMiscCount();
    void sZeroUncountedTagsByWarehouse();
    void sThawItemSitesByClassCode();
    void sPostCountSlipsByWarehouse();
    void sPostCountTags();
    void sPurgePostedCountSlips();
    void sPurgePostedCountTags();
    
    void sPackingListBatch();
    void sIssueStockToShipping();
    void sShipOrders();
    void sRecallOrders();
    void sExternalShipping();
    void sDspShippingContents();

    void sEnterReceipt();
    void sEnterReturn();
    void sPostReceipts();

    void sPrintPackingLists();
    void sPrintPackingListBatchByShipvia();
    void sPrintShippingForm();
    void sPrintShippingForms();
    void sPrintShippingLabelsBySo();
    void sPrintShippingLabelsByInvoice();
    void sPrintReceivingLabelsByPo();
    void sPrintShippingLabelsByTo();

    void sAddRate();
    void sDspRatesByDestination();

    void sDspBacklog();
    void sDspSummarizedBacklogByWarehouse();
    void sDspShipmentsBySalesOrder();
    void sDspShipmentsByDate();
    void sDspShipmentsByShipment();

    void sDspItemAvailabilityWorkbench();

    void sDspFrozenItemSites();
    void sDspCountSlipEditList();
    void sDspCountTagEditList();
    void sDspCountSlipsByWarehouse();
    void sDspCountTagsByItem();
    void sDspCountTagsByWarehouse();
    void sDspCountTagsByClassCode();

    void sDspValidLocationsByItem();
    void sDspQOH();
    void sDspQOHByLocation();
    void sDspLocationLotSerialDetail();
    void sDspSlowMovingInventoryByClassCode();
    void sDspExpiredInventoryByClassCode();
    void sDspInventoryAvailability();
    void sDspSubstituteAvailabilityByRootItem();
    void sDspInventoryHistory();
    void sDspDetailedInventoryHistoryByLotSerial();
    void sDspDetailedInventoryHistoryByLocation();
    void sDspItemUsageStatistics();
    void sDspTimePhasedUsageStatisticsByItem();

    void sPrintItemLabelsByClassCode();

    void sWarehouses();
    void sWarehouseLocations();

    void sDspUnbalancedQOHByClassCode();
    void sUpdateABCClass();
    void sUpdateCycleCountFreq();
    void sUpdateItemSiteLeadTimes();
    void sUpdateReorderLevelByItem();
    void sUpdateReorderLevelsByPlannerCode();
    void sUpdateReorderLevelsByClassCode();
    void sUpdateOUTLevelByItem();
    void sUpdateOUTLevelsByPlannerCode();
    void sUpdateOUTLevelsByClassCode();
    void sSummarizeInvTransByClassCode();
    void sCreateItemSitesByClassCode();

    void sCatchLocationContents(int);
    void sCatchCountTag(int);

    void sSetup();

  private:
    GUIClient *parent;

    QToolBar   *toolBar;
    QMenu *mainMenu;
    QMenu *itemSitesMenu;
    QMenu *warehouseMenu;
    QMenu *transferOrderMenu;
    QMenu *transactionsMenu;
    QMenu *lotSerialControlMenu;
    QMenu *physicalMenu;
    QMenu *physicalCreateTagsMenu;
    QMenu *physicalReportsMenu;
    QMenu *physicalReportsSlipsMenu;
    QMenu *physicalReportsTagsMenu;
    QMenu *shippingMenu;
    QMenu *shippingReportsMenu;
    QMenu *shippingFormsMenu;
    QMenu *receivingMenu;
    QMenu *receivingFormsMenu;
    QMenu *formsMenu;
    QMenu *formsShipLabelsMenu;
    QMenu *graphsMenu;
    QMenu *reportsMenu;
    QMenu *reportsShipmentsMenu;
    QMenu *utilitiesMenu;
    QMenu *updateItemInfoMenu;
    QMenu *updateItemInfoReorderMenu;
    QMenu *updateItemInfoOutMenu;

    void	addActionsToMenu(actionProperties [], unsigned int);
};

#endif
