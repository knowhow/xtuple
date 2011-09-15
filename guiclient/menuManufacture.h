/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef menuManufacture_H
#define menuManufacture_H

#include <QObject>
#include <QPixmap>

class QToolBar;
class QMenu;
class GUIClient;

class menuManufacture : public QObject
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
    menuManufacture(GUIClient *);

  public slots:
    void sNewWorkOrder();
    void sExplodeWorkOrder();
    void sImplodeWorkOrder();
    void sCloseWorkOrder();

    void sPrintTraveler();
    void sPrintPickList();

    void sReleaseWorkOrdersByPlannerCode();
    void sReprioritizeWorkOrder();
    void sRescheduleWorkOrder();
    void sChangeWorkOrderQty();
    void sPurgeClosedWorkOrders();

    void sCreateWoMaterialRequirement();
    void sMaintainWoMaterials();
    void sIssueWoMaterialBatch();
    void sIssueWoMaterialItem();
    void sReturnWoMaterialBatch();
    void sReturnWoMaterialItem();
    void sScrapWoMaterialFromWo();
    void sPostProduction();
    void sPostMiscProduction();
    void sCorrectProductionPosting();

    void sDspWoSchedule();
    void sDspWoHistoryByItem();
    void sDspWoHistoryByNumber();
    void sDspWoHistoryByClassCode();
    void sDspWoMaterialsByComponentItem();
    void sDspWoMaterialsByWo();
    void sDspInventoryAvailabilityByWorkOrder();
    void sDspPendingAvailability();
    void sDspJobCosting();
    void sDspMaterialUsageVarianceByBOMItem();
    void sDspMaterialUsageVarianceByItem();
    void sDspMaterialUsageVarianceByComponentItem();
    void sDspMaterialUsageVarianceByWorkOrder();
    void sDspMaterialUsageVarianceByWarehouse();

    void sPrintWorkOrderForm();

    void sSetup();

  private:
    GUIClient *parent;

    QToolBar    *toolBar;
    QMenu  *mainMenu;
    QMenu  *ordersMenu;
    QMenu  *formsMenu;
    QMenu  *materialsMenu;
    QMenu  *materialsIssueMenu;
    QMenu  *materialsReturnMenu;
    QMenu  *transactionsMenu;
    QMenu  *reportsMenu;
    QMenu  *reportsHistoryMenu;
    QMenu  *reportsMatlReqMenu;
    QMenu  *reportsOperationsMenu;
    QMenu  *reportsWoTcMenu;
    QMenu  *reportsMatlUseVarMenu;
    QMenu  *utilitiesMenu;

    void	addActionsToMenu(actionProperties [], unsigned int);
};

#endif
