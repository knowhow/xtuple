/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef menuPurchase_h
#define menuPurchase_h

#include <QObject>
#include <QMenu>

class QToolBar;
class QMenu;
class GUIClient;

class menuPurchase : public QObject
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
    menuPurchase(GUIClient *);

  public slots:
    void sNewPurchaseOrder();
    void sPurchaseOrderEditList();
    void sPrintPurchaseOrder();
    void sPrintPurchaseOrdersByAgent();
    void sPostPurchaseOrder();
    void sPostPurchaseOrdersByAgent();
    void sClosePurchaseOrder();
    void sReschedulePoitem();
    void sChangePoitemQty();
    void sAddPoComment();

    void sDspUninvoicedReceipts();
    void sEnterVoucher();
    void sEnterMiscVoucher();
    void sUnpostedVouchers();
    void sVoucheringEditList();
    void sPostVouchers();

    void sNewItemSource();
    void sItemSources();

    void sDspPurchaseReqsByItem();
    void sDspPurchaseReqsByPlannerCode();
    void sDspItemSitesByPlannerCode();
    void sDspPOsByDate();
    void sDspPOsByVendor();
    void sDspPoItemsByVendor();
    void sDspPoItemsByItem();
    void sDspPoItemsByDate();
    void sDspPoHistory();
    void sDspItemSourcesByVendor();
    void sDspItemSourcesByItem();
    void sDspBuyCard();
    void sDspReceiptsReturnsByVendor();
    void sDspReceiptsReturnsByItem();
    void sDspReceiptsReturnsByDate();
    void sDspPriceVariancesByVendor();
    void sDspPriceVariancesByItem();
    void sDspPoDeliveryDateVariancesByVendor();
    void sDspPoDeliveryDateVariancesByItem();
    void sDspRejectedMaterialByVendor();

    void sPrintPOForm();
    void sPrintVendorForm();

    void sNewVendor();
    void sVendors();
    void sVendorWorkBench();

    void sItemsWithoutItemSources();
    void sAssignItemToPlannerCode();
    void sAssignClassCodeToPlannerCode();

    void sSetup();

  private:
    GUIClient *parent;

    QToolBar *toolBar;
    QMenu *mainMenu;
    QMenu *requestMenu;
    QMenu *ordersMenu;
    QMenu *vouchersMenu;
    QMenu *itemSourcesMenu;
    QMenu *vendorMenu;
    QMenu *formsMenu;
    QMenu *reportsMenu;
    QMenu *reportsPoMenu;
    QMenu *reportsPoItemsMenu;
    QMenu *reportsItemSrcMenu;
    QMenu *reportsRcptRtrnMenu;
    QMenu *reportsPriceVarMenu;
    QMenu *reportsDelvVarMenu;
    QMenu *utilitiesMenu;

    void	addActionsToMenu(actionProperties [], unsigned int);
};

#endif
