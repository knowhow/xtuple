/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef menuSales_h
#define menuSales_h

#include <QObject>
#include <QPixmap>

class QToolBar;
class QMenu;
class GUIClient;

class menuSales : public QObject
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
    menuSales(GUIClient *);

  public slots:
    void sNewSalesOrder();
    void sOpenSalesOrders();
    void sNewQuote();
    void sQuotes();

    void sPackingListBatch();
    void sPrintPackingList();
    
    void sUninvoicedShipments();
    void sSelectShippedOrdersForBilling();
    void sSelectOrderForBilling();
    void sBillingEditList();
    void sDspBillingSelections();
    void sCreateInvoices();
    void sUnpostedInvoices();
    void sPrintInvoices();
    void sPrintInvoicesByShipvia();
    void sReprintInvoices();
    void sPostInvoices();
    void sPurgeInvoices();

    void sNewReturn();
    void sOpenReturns();
    void sReturnsWorkbench();

    void sNewCreditMemo();
    void sUnpostedCreditMemos();
    void sCreditMemoEditList();
    void sPrintCreditMemos();
    void sReprintCreditMemos();
    void sPostCreditMemos();
    void sPurgeCreditMemos();

    void sItemListPrice();
    void sUpdateListPricesByProductCategory();
    void sPricingSchedules();
    void sPricingScheduleAssignments();
    void sSales();
    void sUpdatePrices();
    void sDspPricesByItem();
    void sDspPricesByCustomer();
    void sDspPricesByCustomerType();
    void sDspFreightPricesByCustomer();
    void sDspFreightPricesByCustomerType();

    void sDspOrderLookupByCustomer();
    void sDspOrderLookupByCustomerType();
    void sDspOrderLookupByItem();
    void sDspOrderLookupByCustomerPO();
    void sDspQuoteLookupByCustomer();
    void sDspQuoteLookupByItem();
    void sDspInventoryAvailabilityByItem();
    void sDspInventoryAvailabilityBySalesOrder();
    void sDspInventoryAvailabilityByCustomerType();
    void sDspReservations();
    void sDspCustomersByCusttype();
    void sDspCustomersByCharacteristic();
    void sDspSalesOrderStatus();
    void sDspBacklogByItem();
    void sDspBacklogBySalesOrder();
    void sDspBacklogByCustomer();
    void sDspBacklogByCustomerType();
    void sDspBacklogByCustomerGroup();
    void sDspBacklogByProductCategory();
    void sDspSummarizedBacklogByWarehouse();
    void sDspPartiallyShippedOrders();
    void sDspEarnedCommissions();
    void sDspBriefEarnedCommissions();
    void sDspTaxHistory();
    
    void sDspSalesHistoryByCustomer();
    void sDspSalesHistoryByBilltoName();
    void sDspSalesHistoryByShipTo();
    void sDspSalesHistoryByItem();
    void sDspSalesHistoryBySalesRep();
    void sDspSalesHistoryByProductCategory();
    void sDspSalesHistoryByCustomerType();
    void sDspSalesHistoryByCustomerGroup();
    void sDspBriefSalesHistoryByCustomer();
    void sDspBriefSalesHistoryByCustomerType();
    void sDspBriefSalesHistoryBySalesRep();
    void sDspBookingsByCustomer();
    void sDspBookingsByCustomerGroup();
    void sDspBookingsByShipTo();
    void sDspBookingsByItem();
    void sDspBookingsByProductCategory();
    void sDspBookingsBySalesRep();
    void sDspSummarizedSalesByCustomer();
    void sDspSummarizedSalesByCustomerType();
    void sDspSummarizedSalesByCustomerByItem();
    void sDspSummarizedSalesByCustomerTypeByItem();
    void sDspSummarizedSalesByItem();
    void sDspSummarizedSalesBySalesRep();
    void sDspSummarizedSalesHistoryByShippingZone();
    void sDspTimePhasedBookingsByItem();
    void sDspTimePhasedBookingsByProductCategory();
    void sDspTimePhasedBookingsByCustomer();
    void sDspTimePhasedSalesByItem();
    void sDspTimePhasedSalesByProductCategory();
    void sDspTimePhasedSalesByCustomer();
    void sDspTimePhasedSalesByCustomerGroup();
    void sDspTimePhasedSalesByCustomerByItem();

    void sPrintSalesOrderForm();
    void sPrintReturnAuthForm();
    void sPrintSASpecialCalendarForm();

    void sNewCustomer();
    void sCustomers();
    void sCustomerWorkbench();
    void sUpdateCreditStatusByCustomer();
    void sCustomerGroups();
    void sNewProspect();
    void sProspects();

    void sDspCustomerInformationExport();
    void sReassignCustomerTypeByCustomerType();
    
    void sArchiveSalesHistory();
    void sRestoreSalesHistory();

    void sAllocateReservations();

    void sSetup();

  private:
    GUIClient *parent;
		
    QToolBar   *toolBar;
    QMenu *mainMenu;
    QMenu *quotesMenu;
    QMenu *ordersMenu;
    QMenu *packingListsMenu;
    QMenu *billingMenu;
    QMenu *billingInvoicesMenu;
    QMenu *billingCreditMemosMenu;
    QMenu *billingFormsMenu;
	QMenu *returnsMenu;
    QMenu *pricingMenu;
    QMenu *pricingReportsMenu;
    QMenu *pricingUpdateMenu;
    QMenu *prospectMenu;
    QMenu *customerMenu;
    QMenu *lookupMenu;
    QMenu *lookupQuoteMenu;
    QMenu *lookupSoMenu;
    QMenu *formsMenu;
    QMenu *analysisMenu;
    QMenu *analysisBookMenu;
    QMenu *analysisSumHistMenu;
    QMenu *analysisHistMenu;
    QMenu *analysisBrfHistMenu;
    QMenu *analysisTpBookMenu;
    QMenu *analysisTpHistMenu;
    QMenu *reportsMenu;
    QMenu *reportsCustomersMenu;
    QMenu *reportsInvAvailMenu;
    QMenu *reportsBacklogMenu;
    QMenu *utilitiesMenu;
    
    void	addActionsToMenu(actionProperties [], unsigned int);
};

#endif

