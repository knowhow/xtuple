/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef menuProducts_h
#define menuProducts_h

#include <QObject>
#include <QMenu>

class QToolBar;
class QMenu;
class GUIClient;

class menuProducts : public QObject
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
    menuProducts(GUIClient *);

  private slots:
    void sNewItem();
    void sItems();
    void sLotSerial();
    void sCopyItem();
    void sItemGroups();
    void sItemImages();

    void sNewBOM();
    void sBOMs();
    void sCopyBOM();
    void sMassReplaceComponent();
    void sMassExpireComponent();

    void sMaintainItemCosts();
    void sUpdateActualCostsByItem();
    void sUpdateActualCostsByClassCode();
    void sPostActualCostsByItem();
    void sPostActualCostsByClassCode();
    void sUpdateStandardCostsByItem();
    void sUpdateStandardCostsByClassCode();
    void sDspCostedSingleLevelBOM();
    void sDspCostedIndentedBOM();
    void sDspCostedSummarizedBOM();
    void sDspItemCostsByClassCode();
    void sDspItemCostSummary();
    void sDspItemCostHistory();

    void sUserCostingElements();

    void sDspItemsByClassCode();
    void sDspItemsByProductCategory();
    void sDspItemsByCharacteristic();
    void sDspSingleLevelBOM();
    void sDspIndentedBOM();
    void sDspSummarizedBOM();
    void sDspSingleLevelWhereUsed();
    void sDspIndentedWhereUsed();
    void sDspPendingBOMChanges();
    void sDspCapacityUOMsByClassCode();
    void sDspCapacityUOMsByProductCategory();
    void sDspItemAvailabilityWorkbench();

    void sDspUnusedPurchasedItems();
    void sDspUndefinedManufacturedItems();
    void sDspInvalidBillsOfMaterials();
    void sReassignClassCodeByClassCode();
    void sReassignProductCategoryByProductCategory();

    void sSetup();

    void sPreferences();

  private:
    GUIClient *parent;

    QToolBar *toolBar;
    QMenu *mainMenu;
    QMenu *itemsMenu;
    QMenu *bomMenu;
    QMenu *costingMenu;
    QMenu *costingUpdActMenu;
    QMenu *costingPostActMenu;
    QMenu *costingUpdStdMenu;
    QMenu *costingReportsMenu;
    QMenu *costingReportsCostedMenu;
    QMenu *costingReportsItemCostsMenu;
    QMenu *reportsMenu;
    QMenu *reportsItemsMenu;
    QMenu *reportsBomsMenu;
    QMenu *reportsWhereUsdMenu;
    QMenu *reportsCapUomMenu;
    QMenu *utilitiesMenu;
    
    void	addActionsToMenu(actionProperties [], unsigned int);
};
		
#endif
