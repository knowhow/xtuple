/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include <QAction>
#include <QMenuBar>
#include <QMenu>
#include <QMessageBox>
#include <QPixmap>
#include <QToolBar>

#include <parameter.h>

#include "guiclient.h"

#include "salesOrder.h"
#include "openSalesOrders.h"
#include "quotes.h"

#include "returnAuthorization.h"
#include "openReturnAuthorizations.h"
#include "returnAuthorizationWorkbench.h"

#include "packingListBatch.h"
#include "printPackingList.h"

#include "uninvoicedShipments.h"
#include "selectShippedOrders.h"
#include "selectOrderForBilling.h"
#include "billingEditList.h"
#include "dspBillingSelections.h"
#include "createInvoices.h"
#include "unpostedInvoices.h"
#include "reprintInvoices.h"
#include "printInvoices.h"
#include "printInvoicesByShipvia.h"
#include "purgeInvoices.h"

#include "creditMemo.h"
#include "unpostedCreditMemos.h"
#include "creditMemoEditList.h"
#include "printCreditMemos.h"
#include "reprintCreditMemos.h"
#include "postCreditMemos.h"
#include "purgeCreditMemos.h"

#include "postInvoices.h"

#include "itemListPrice.h"
#include "updateListPricesByProductCategory.h"
#include "itemPricingSchedules.h"
#include "pricingScheduleAssignments.h"
#include "sales.h"
#include "updatePrices.h"

#include "dspPricesByItem.h"
#include "dspPricesByCustomer.h"
#include "dspPricesByCustomerType.h"
#include "dspFreightPricesByCustomer.h"
#include "dspFreightPricesByCustomerType.h"

#include "dspSalesOrdersByCustomer.h"
#include "dspSalesOrdersByItem.h"
#include "dspSalesOrdersByCustomerPO.h"
#include "dspSalesOrdersByParameterList.h"
#include "dspQuotesByCustomer.h"
#include "dspQuotesByItem.h"
#include "dspCustomersByCustomerType.h"
#include "dspCustomersByCharacteristic.h"
#include "dspInventoryAvailabilityByItem.h"
#include "dspInventoryAvailabilityBySalesOrder.h"
#include "dspInventoryAvailabilityByCustomerType.h"
#include "dspReservations.h"
#include "dspSalesOrderStatus.h"
#include "dspBacklogByItem.h"
#include "dspBacklogBySalesOrder.h"
#include "dspBacklogByCustomer.h"
#include "dspBacklogByParameterList.h"
#include "dspSummarizedBacklogByWarehouse.h"
#include "dspPartiallyShippedOrders.h"
#include "dspEarnedCommissions.h"
#include "dspBriefEarnedCommissions.h"
#include "dspTaxHistory.h"

#include "dspSalesHistoryByCustomer.h"
#include "dspSalesHistoryByBilltoName.h"
#include "dspSalesHistoryByShipTo.h"
#include "dspSalesHistoryByParameterList.h"
#include "dspSalesHistoryByItem.h"
#include "dspSalesHistoryBySalesrep.h"
#include "dspBriefSalesHistoryByCustomer.h"
#include "dspBriefSalesHistoryByCustomerType.h"
#include "dspBriefSalesHistoryBySalesRep.h"
#include "dspBookingsByCustomer.h"
#include "dspBookingsByCustomerGroup.h"
#include "dspBookingsByShipTo.h"
#include "dspBookingsByProductCategory.h"
#include "dspBookingsByItem.h"
#include "dspBookingsBySalesRep.h"
#include "dspSummarizedSalesByCustomer.h"
#include "dspSummarizedSalesByCustomerType.h"
#include "dspSummarizedSalesByCustomerByItem.h"
#include "dspSummarizedSalesByCustomerTypeByItem.h"
#include "dspSummarizedSalesByItem.h"
#include "dspSummarizedSalesBySalesRep.h"
#include "dspSummarizedSalesHistoryByShippingZone.h"
#include "dspTimePhasedBookingsByItem.h"
#include "dspTimePhasedBookingsByProductCategory.h"
#include "dspTimePhasedBookingsByCustomer.h"
#include "dspTimePhasedSalesByItem.h"
#include "dspTimePhasedSalesByProductCategory.h"
#include "dspTimePhasedSalesByCustomer.h"
#include "dspTimePhasedSalesByCustomerGroup.h"
#include "dspTimePhasedSalesByCustomerByItem.h"

#include "printSoForm.h"
#include "printRaForm.h"

#include "customer.h"
#include "customers.h"
#include "prospect.h"
#include "prospects.h"
#include "updateCreditStatusByCustomer.h"
#include "customerGroups.h"

#include "dspCustomerInformationExport.h"
#include "reassignCustomerTypeByCustomerType.h"
#include "characteristics.h"

#include "printSASpecialCalendarForm.h"
#include "archRestoreSalesHistory.h"
#include "allocateReservations.h"

#include "setup.h"

#include "menuSales.h"

menuSales::menuSales(GUIClient *pParent) :
  QObject(pParent)
{
  setObjectName("soModule");
  parent = pParent;

  toolBar = new QToolBar(tr("Sales Tools"));
  toolBar->setObjectName("Sales Tools");
  toolBar->setIconSize(QSize(32, 32));
  parent->addToolBar(toolBar);

  mainMenu = new QMenu(parent);
  quotesMenu = new QMenu(parent);
  ordersMenu = new QMenu(parent);
  billingMenu = new QMenu(parent);
  billingInvoicesMenu = new QMenu(parent);
  billingCreditMemosMenu= new QMenu(parent);
  billingFormsMenu= new QMenu(parent);
  returnsMenu= new QMenu(parent);
  lookupMenu = new QMenu(parent);
  lookupQuoteMenu = new QMenu(parent);
  lookupSoMenu = new QMenu(parent);
  formsMenu = new QMenu(parent);
  reportsMenu = new QMenu(parent);
  reportsCustomersMenu = new QMenu(parent);
  reportsInvAvailMenu = new QMenu(parent);
  reportsBacklogMenu = new QMenu(parent);
  analysisMenu = new QMenu(parent);
  analysisBookMenu = new QMenu(parent);
  analysisSumHistMenu = new QMenu(parent);
  analysisHistMenu = new QMenu(parent);
  analysisBrfHistMenu = new QMenu(parent);
  analysisTpBookMenu = new QMenu(parent);
  analysisTpHistMenu = new QMenu(parent);
  prospectMenu = new QMenu(parent);
  customerMenu = new QMenu(parent);
  pricingReportsMenu = new QMenu(parent);
  pricingMenu = new QMenu(parent);
  pricingUpdateMenu = new QMenu(parent);
  utilitiesMenu = new QMenu(parent);

  mainMenu->setObjectName("menu.sales");
  quotesMenu->setObjectName("menu.sales.quotes");
  ordersMenu->setObjectName("menu.sales.orders");
  billingMenu->setObjectName("menu.sales.billing");
  billingInvoicesMenu->setObjectName("menu.sales.billinginvoices");
  billingCreditMemosMenu->setObjectName("menu.sales.billingcreditmemos");
  billingFormsMenu->setObjectName("menu.sales.billingforms");
  returnsMenu->setObjectName("menu.sales.returns");
  lookupMenu->setObjectName("menu.sales.lookup");
  lookupQuoteMenu->setObjectName("menu.sales.lookupquote");
  lookupSoMenu->setObjectName("menu.sales.lookupso");
  formsMenu->setObjectName("menu.sales.forms");
  reportsMenu->setObjectName("menu.sales.reports");
  reportsCustomersMenu->setObjectName("menu.sales.reportscustomers");
  reportsInvAvailMenu->setObjectName("menu.sales.reportsinvavail");
  reportsBacklogMenu->setObjectName("menu.sales.reportsbacklog");
  analysisMenu->setObjectName("menu.sales.analysis");
  analysisBookMenu->setObjectName("menu.sales.analysisbook");
  analysisSumHistMenu->setObjectName("menu.sales.analysissumhist");
  analysisHistMenu->setObjectName("menu.sales.analysishist");
  analysisBrfHistMenu->setObjectName("menu.sales.analysisbrfhist");
  analysisTpBookMenu->setObjectName("menu.sales.analysistpbook");
  analysisTpHistMenu->setObjectName("menu.sales.analysistphist");
  prospectMenu->setObjectName("menu.sales.prospect");
  customerMenu->setObjectName("menu.sales.customer");
  pricingReportsMenu->setObjectName("menu.sales.pricingreports");
  pricingMenu->setObjectName("menu.sales.pricing");
  pricingUpdateMenu->setObjectName("menu.sales.pricingupdate");
  utilitiesMenu->setObjectName("menu.sales.utilities");

  actionProperties acts[] = {
    // Sales | Quotes
    { "menu",	tr("&Quote"),	(char*)quotesMenu,	mainMenu,	"true",	NULL, NULL, true, NULL },
    { "so.newQuote", tr("&New..."),	SLOT(sNewQuote()), quotesMenu, "MaintainQuotes",	NULL, NULL, true, NULL },
    { "so.listQuotes", tr("&List..."),	SLOT(sQuotes()), quotesMenu, "MaintainQuotes ViewQuotes",	NULL, NULL, true, NULL },
    
    // Sales | Sales Order
    { "menu",	tr("&Sales Order"),	(char*)ordersMenu,	mainMenu,	"true",	NULL, NULL, true, NULL },
    { "so.newSalesOrder", 	     tr("&New..."),		SLOT(sNewSalesOrder()),   ordersMenu, "MaintainSalesOrders", NULL, NULL,	 true, NULL },
    { "so.listOpenSalesOrders",      tr("&List Open..."),	SLOT(sOpenSalesOrders()), ordersMenu, "MaintainSalesOrders ViewSalesOrders",	QPixmap(":/images/listOpenSalesOrders.png"), toolBar,  true, tr("List Open Sales Orders") },

    // Sales | Billing
    { "menu",	tr("&Billing"),     (char*)billingMenu,		mainMenu,	"true",	NULL, NULL, true, NULL },
    
    // Sales | Billing | Invoice
    { "menu",	tr("&Invoice"),   (char*)billingInvoicesMenu,	billingMenu,	"true",	NULL, NULL, true, NULL },
    { "so.uninvoicedShipments",		     tr("&Uninvoiced Shipments..."),			SLOT(sUninvoicedShipments()), 		billingInvoicesMenu, "SelectBilling",	 QPixmap(":/images/uninvoicedShipments"), toolBar, true, tr("Uninvoiced Shipments") },
    { "separator",	NULL,	NULL,	billingInvoicesMenu,	"true",		NULL, NULL, true, NULL },
    { "so.selectAllShippedOrdersForBilling", tr("Select &All Shipped Orders for Billing..."),	SLOT(sSelectShippedOrdersForBilling()), billingInvoicesMenu, "SelectBilling",	NULL, NULL, true, NULL },
    { "so.selectOrderForBilling",	     tr("Select &Order for Billing..."),			SLOT(sSelectOrderForBilling()),		billingInvoicesMenu, "SelectBilling",	NULL, NULL, true, NULL },
    { "separator",	NULL,	NULL,	billingInvoicesMenu,	"true",		NULL, NULL, true, NULL },
    { "so.billingEditList",		     tr("&Billing Edit List..."),	SLOT(sBillingEditList()), billingInvoicesMenu, "SelectBilling",	NULL, NULL, true, NULL },
    { "so.dspBillingSelections",	     tr("Billing &Selections..."),	SLOT(sDspBillingSelections()), billingInvoicesMenu, "SelectBilling", QPixmap(":/images/billingSelections"), toolBar, true, tr("Billing Selections") },
    { "so.createInvoices",	     tr("&Create Invoices..."),	SLOT(sCreateInvoices()), billingInvoicesMenu, "SelectBilling",	NULL, NULL, true, NULL },
    { "separator",	NULL,	NULL,	billingInvoicesMenu,	"true",		NULL, NULL, true, NULL },
    { "so.listUnpostedInvoices",	     tr("&List Unposted Invoices..."),	SLOT(sUnpostedInvoices()), billingInvoicesMenu, "SelectBilling",	NULL, NULL,  true, NULL },
    { "so.postInvoices",		     tr("Post &Invoices..."),		SLOT(sPostInvoices()), billingInvoicesMenu, "PostMiscInvoices",	NULL, NULL, true, NULL },

    // Sales | Billing | Credit Memo
    { "menu",	tr("&Credit Memo"), (char*)billingCreditMemosMenu,	billingMenu,	"true",	NULL, NULL, true, NULL },
    { "so.newCreditMemo",		     tr("&New..."),		SLOT(sNewCreditMemo()), billingCreditMemosMenu, "MaintainCreditMemos",	NULL, NULL, true, NULL },
    { "so.listUnpostedCreditMemos",	     tr("&List Unposted..."),	SLOT(sUnpostedCreditMemos()), billingCreditMemosMenu, "MaintainCreditMemos ViewCreditMemos",	NULL, NULL, true, NULL },
    { "so.creditMemoEditList",		     tr("&Edit List..."),	SLOT(sCreditMemoEditList()), billingCreditMemosMenu, "MaintainCreditMemos ViewCreditMemos",	NULL, NULL, true, NULL },
    { "separator",	NULL,	NULL,	billingCreditMemosMenu,	"true",		NULL, NULL, true, NULL },
    { "so.postCreditMemos",		     tr("&Post..."),	SLOT(sPostCreditMemos()), billingCreditMemosMenu, "PostARDocuments",	NULL, NULL, true, NULL },

    { "separator",	NULL,	NULL,	billingMenu,	"true",		NULL, NULL, true, NULL },
    
    // Sales | Billing | Forms
    { "menu",	tr("&Forms"), (char*)billingFormsMenu,	billingMenu,	"true",	NULL, NULL, true, NULL },
    { "so.printInvoices",		     tr("&Print Invoices..."),		SLOT(sPrintInvoices()), billingFormsMenu, "PrintInvoices",	NULL, NULL, true, NULL },
    { "so.printInvoicesByShipvia",	     tr("Print Invoices by Ship &Via..."),	SLOT(sPrintInvoicesByShipvia()), billingFormsMenu, "PrintInvoices",	NULL, NULL, true, NULL },
    { "so.reprintInvoices",		     tr("&Re-Print Invoices..."),	SLOT(sReprintInvoices()), billingFormsMenu, "PrintInvoices",	NULL, NULL, true, NULL },
    { "separator",	NULL,	NULL,	billingFormsMenu,	"true",		NULL, NULL, true , NULL },
    { "so.printCreditMemos",		     tr("Print &Credit Memos..."),	SLOT(sPrintCreditMemos()), billingFormsMenu, "PrintCreditMemos",	NULL, NULL, true, NULL },
    { "so.reprintCreditMemos",		     tr("Re-Print Credit &Memos..."),	SLOT(sReprintCreditMemos()), billingFormsMenu, "PrintCreditMemos",	NULL, NULL, true, NULL },

    // Sales | Returns
    { "menu",	tr("&Return"),	(char*)returnsMenu,	mainMenu, "true",	NULL, NULL,  _metrics->boolean("EnableReturnAuth"), NULL },
    { "so.newReturn", tr("&New..."),	SLOT(sNewReturn()), returnsMenu, "MaintainReturns",	NULL, NULL, true, NULL },
    { "so.openReturns", tr("&List Open..."),	SLOT(sOpenReturns()), returnsMenu, "MaintainReturns ViewReturns",	NULL, NULL, true, NULL },
    { "separator",	NULL,	NULL,	returnsMenu,	"true",		NULL, NULL, true , NULL },
    { "so.returnsWorkbench", tr("&Workbench..."),	SLOT(sReturnsWorkbench()), returnsMenu, "MaintainReturns ViewReturns",	NULL, NULL, true, NULL },

    { "separator",	NULL,	NULL,	mainMenu,	"true",		NULL, NULL, true, NULL },
    
    // Sales | Lookup
    { "menu",	tr("&Lookup"),           (char*)lookupMenu,	mainMenu,	"true",	NULL, NULL, true, NULL },
    
    // Sales | Lookup | Quote Lookup
    { "menu",	tr("&Quote"),           (char*)lookupQuoteMenu,	lookupMenu,	"true",	NULL, NULL, true, NULL },
    { "so.dspQuoteLookupByCustomer", tr("by &Customer..."),	SLOT(sDspQuoteLookupByCustomer()), lookupQuoteMenu, "ViewQuotes",	NULL, NULL, true, NULL },
    { "so.dspQuoteOrderLookupByItem", tr("by &Item..."),	SLOT(sDspQuoteLookupByItem()), lookupQuoteMenu, "ViewQuotes",	NULL, NULL, true, NULL },
    
    // Sales | Lookup | Sales Order Lookup
    { "menu",	tr("&Sales Order"),           (char*)lookupSoMenu,	lookupMenu,	"true",	NULL, NULL, true, NULL },
    { "so.dspSalesOrderLookupByCustomerType", tr("by Customer &Type..."),	SLOT(sDspOrderLookupByCustomerType()), lookupSoMenu, "ViewSalesOrders",	NULL, NULL, true, NULL },
    { "so.dspSalesOrderLookupByCustomer", tr("by &Customer..."),	SLOT(sDspOrderLookupByCustomer()), lookupSoMenu, "ViewSalesOrders",	NULL, NULL, true, NULL },
    { "so.dspSalesOrderLookupByCustomerPO", tr("by Customer &PO..."),	SLOT(sDspOrderLookupByCustomerPO()), lookupSoMenu, "ViewSalesOrders",	NULL, NULL, true, NULL },
    { "so.dspSalesOrderLookupByItem", tr("by &Item..."),	SLOT(sDspOrderLookupByItem()), lookupSoMenu, "ViewSalesOrders",	NULL, NULL, true, NULL },
    
    { "separator",	NULL,	NULL,	lookupMenu,	"true",		NULL, NULL, true, NULL }, 
    { "so.dspSalesOrderStatus", tr("Sales Order S&tatus..."),	SLOT(sDspSalesOrderStatus()), lookupMenu, "ViewSalesOrders",	NULL, NULL, true, NULL },   
  
    // Sales | Forms
    { "menu",	tr("&Forms"),           (char*)formsMenu,	mainMenu,	"true",	NULL, NULL, true, NULL },
    { "so.printSalesOrderForm", tr("Print Sales &Order Form..."),	SLOT(sPrintSalesOrderForm()), formsMenu, "MaintainSalesOrders ViewSalesOrders",	NULL, NULL, true, NULL },
    { "so.printReturnAuthForm", tr("Print &Return Auth. Form..."),	SLOT(sPrintReturnAuthForm()), formsMenu, "MaintainReturns",	NULL, NULL, _metrics->boolean("EnableReturnAuth"), NULL },
    { "separator",	NULL,	NULL,	formsMenu,	"true",		NULL, NULL, true , NULL }, 
    { "so.packingListBatch", tr("Packing &List Batch..."),	SLOT(sPackingListBatch()), formsMenu, "MaintainPackingListBatch ViewPackingListBatch",	NULL, NULL, true, NULL },
    { "so.printPackingList", tr("&Print Packing List..."),	SLOT(sPrintPackingList()), formsMenu, "PrintPackingLists",	NULL, NULL, true, NULL },


    // Sales | Reports
    { "menu",	tr("&Reports"),           (char*)reportsMenu,	mainMenu,	"true",	NULL, NULL, true, NULL },
    { "so.dspSummarizedBacklogByWarehouse", tr("Su&mmarized Backlog..."),	SLOT(sDspSummarizedBacklogByWarehouse()), reportsMenu, "ViewSalesOrders",	QPixmap(":/images/dspSummarizedBacklogByWhse.png"), toolBar,  true, tr("Summarized Backlog") },

    // Sales | Reports | Backlog
    { "menu",	tr("&Backlog"),           (char*)reportsBacklogMenu,	reportsMenu,	"true",	NULL, NULL, true, NULL },
    { "so.dspBacklogByCustomerType", tr("by Customer &Type..."),	SLOT(sDspBacklogByCustomerType()), reportsBacklogMenu, "ViewSalesOrders",	NULL, NULL, true, NULL },
    { "so.dspBacklogByCustomerGroup", tr("by Customer &Group..."),	SLOT(sDspBacklogByCustomerGroup()), reportsBacklogMenu, "ViewSalesOrders",	NULL, NULL, true, NULL },
    { "so.dspBacklogByCustomer", tr("by &Customer..."),	SLOT(sDspBacklogByCustomer()), reportsBacklogMenu, "ViewSalesOrders", NULL, NULL,	 true, NULL },
    { "separator",	NULL,	NULL,	reportsBacklogMenu,	"true",		NULL, NULL, true, NULL },
    { "so.dspBacklogBySalesOrder", tr("by Sales &Order..."),	SLOT(sDspBacklogBySalesOrder()), reportsBacklogMenu, "ViewSalesOrders",	NULL, NULL, true, NULL },
    { "separator",	NULL,	NULL,	reportsBacklogMenu,	"true",		NULL, NULL, true, NULL },
    { "so.dspBacklogByProductCategory", tr("by &Product Category..."),	SLOT(sDspBacklogByProductCategory()), reportsBacklogMenu, "ViewSalesOrders",	NULL, NULL, true, NULL },
    { "so.dspBacklogByItem", tr("by &Item..."),	SLOT(sDspBacklogByItem()), reportsBacklogMenu, "ViewSalesOrders", NULL, NULL,	 true, NULL },
    
    { "so.dspPartiallyShippedOrders", tr("&Partially Shipped Orders..."),	SLOT(sDspPartiallyShippedOrders()), reportsMenu, "ViewSalesOrders",	NULL, NULL, true, NULL },
    { "so.dspReservations", tr("Reservations by Item..."),	SLOT(sDspReservations()), reportsMenu, "ViewInventoryAvailability",	NULL, NULL, true, NULL },    
    { "separator",	NULL,	NULL,	reportsMenu,	"true",		NULL, NULL, true, NULL },   
    
    // Sales | Reports | Inventory Availability
    { "menu",	tr("&Inventory Availability"),           (char*)reportsInvAvailMenu,	reportsMenu,	"true",	NULL, NULL, true, NULL },
    { "so.dspInventoryAvailabilityBySalesOrder", tr("by Sales &Order..."),	SLOT(sDspInventoryAvailabilityBySalesOrder()), reportsInvAvailMenu, "ViewInventoryAvailability",	NULL, NULL, true, NULL },
    { "so.dspInventoryAvailabilityByItem", tr("by &Item..."),	SLOT(sDspInventoryAvailabilityByItem()), reportsInvAvailMenu, "ViewInventoryAvailability",	NULL, NULL, true, NULL },    
    { "so.dspInventoryAvailabilityByCustomerType", tr("by &Customer Type..."),	SLOT(sDspInventoryAvailabilityByCustomerType()), reportsInvAvailMenu, "ViewInventoryAvailability",	NULL, NULL, true, NULL },    

    { "separator",	NULL,	NULL,	reportsMenu,	"true",		NULL, NULL, true, NULL },
    { "so.dspEarnedCommissions", tr("&Earned Commissions..."),	SLOT(sDspEarnedCommissions()), reportsMenu, "ViewCommissions",	NULL, NULL, true, NULL },
    { "so.dspBriefEarnedCommissions", tr("B&rief Earned Commissions..."),	SLOT(sDspBriefEarnedCommissions()), reportsMenu, "ViewCommissions",	NULL, NULL, true, NULL },
    { "so.dspTaxHistory", tr("Tax History..."),	SLOT(sDspTaxHistory()), reportsMenu, "ViewTaxReconciliations",	NULL, NULL, true, NULL },

    { "separator",	NULL,	NULL,	reportsMenu,	"true",		NULL, NULL, true, NULL },
    
    // Sales | Reports | Customers
    { "menu",	tr("&Customers"),           (char*)reportsCustomersMenu,	reportsMenu,	"true",	NULL, NULL, true, NULL },
    { "so.dspCustomersByCustomerType", tr("by Customer &Type..."),	SLOT(sDspCustomersByCusttype()), reportsCustomersMenu, "MaintainCustomerMasters ViewCustomerMasters",	NULL, NULL, true, NULL },
    { "so.dspCustomersByCharacteristic", tr("by C&haracteristic..."),	SLOT(sDspCustomersByCharacteristic()), reportsCustomersMenu, "MaintainCustomerMasters ViewCustomerMasters",	NULL, NULL, true, NULL },

    // Sales | Analysis
    { "menu",	tr("&Analysis"),           (char*)analysisMenu,	mainMenu,	"true",	NULL, NULL, true, NULL },

    // Sales | Analysis | Bookings
    { "menu",	tr("&Bookings"),           (char*)analysisBookMenu,	analysisMenu,	"true",	NULL, NULL, true, NULL },
    { "sa.dspBookingsBySalesRep", tr("by &Sales Rep..."), SLOT(sDspBookingsBySalesRep()), analysisBookMenu, "ViewSalesOrders", NULL, NULL, true , NULL },
    { "separator",	NULL,	NULL,	analysisBookMenu,	"true",		NULL, NULL, true, NULL },
    { "sa.dspBookingsByCustomerGroup", tr("by Customer &Group..."), SLOT(sDspBookingsByCustomerGroup()), analysisBookMenu, "ViewSalesOrders", NULL, NULL, true , NULL },
    { "sa.dspBookingsByCustomer", tr("by &Customer..."), SLOT(sDspBookingsByCustomer()), analysisBookMenu, "ViewSalesOrders", NULL, NULL, true , NULL },
    { "sa.dspBookingsByShipTo", tr("by S&hip-To..."), SLOT(sDspBookingsByShipTo()), analysisBookMenu, "ViewSalesOrders", NULL, NULL, true , NULL },
    { "separator",	NULL,	NULL,	analysisBookMenu,	"true",		NULL, NULL, true, NULL },
    { "sa.dspBookingsByProductCategory", tr("by &Product Category..."), SLOT(sDspBookingsByProductCategory()), analysisBookMenu, "ViewSalesOrders", NULL, NULL, true , NULL },
    { "sa.dspBookingsByItem", tr("by &Item..."), SLOT(sDspBookingsByItem()), analysisBookMenu, "ViewSalesOrders", NULL, NULL, true , NULL },

    // Sales | Analysis | Time Phased Bookings
    { "menu",	tr("Time-Phased B&ookings"),           (char*)analysisTpBookMenu,	analysisMenu,	"true",	NULL, NULL, true, NULL },
    { "sa.dspTimePhasedBookingsByCustomer", tr("by &Customer..."), SLOT(sDspTimePhasedBookingsByCustomer()), analysisTpBookMenu, "ViewSalesOrders", NULL, NULL, true , NULL },
    { "separator",	NULL,	NULL,	analysisTpBookMenu,	"true",		NULL, NULL, true, NULL },
    { "sa.dspTimePhasedBookingsByProductCategory", tr("by &Product Category..."), SLOT(sDspTimePhasedBookingsByProductCategory()), analysisTpBookMenu, "ViewSalesOrders", NULL, NULL, true , NULL },
    { "sa.dspTimePhasedBookingsByItem", tr("by &Item..."), SLOT(sDspTimePhasedBookingsByItem()), analysisTpBookMenu, "ViewSalesOrders", NULL, NULL, true , NULL },
    
    { "separator",	NULL,	NULL,	analysisMenu,	"true",		NULL, NULL, true, NULL },
    
    // Sales | Analysis | Sales History
    { "menu",	tr("Sales &History"),           (char*)analysisHistMenu,	analysisMenu,	"true",	NULL, NULL, true, NULL },
    { "sa.dspSalesHistoryBySalesRep", tr("by &Sales Rep..."), SLOT(sDspSalesHistoryBySalesRep()), analysisHistMenu, "ViewSalesHistory", NULL, NULL, true , NULL },
    { "separator",	NULL,	NULL,	analysisHistMenu,	"true",		NULL, NULL, true, NULL },
    { "sa.dspSalesHistoryByCustomerType", tr("by Customer &Type..."), SLOT(sDspSalesHistoryByCustomerType()), analysisHistMenu, "ViewSalesHistory", NULL, NULL, true , NULL },
    { "sa.dspSalesHistoryByCustomerGroup", tr("by Customer &Group..."), SLOT(sDspSalesHistoryByCustomerGroup()), analysisHistMenu, "ViewSalesHistory", NULL, NULL, true , NULL },
    { "sa.dspSalesHistoryByCustomer", tr("by &Customer..."), SLOT(sDspSalesHistoryByCustomer()), analysisHistMenu, "ViewSalesHistory", NULL, NULL, true , NULL },
    { "sa.dspSalesHistoryByBillToName", tr("by &Bill-To Name..."), SLOT(sDspSalesHistoryByBilltoName()), analysisHistMenu, "ViewSalesHistory", NULL, NULL, true , NULL },
    { "sa.dspSalesHistoryByShipTo", tr("by S&hip-To..."), SLOT(sDspSalesHistoryByShipTo()), analysisHistMenu, "ViewSalesHistory", NULL, NULL, true , NULL },
    { "separator",	NULL,	NULL,	analysisHistMenu,	"true",		NULL, NULL, true, NULL },
    { "sa.dspSalesHistoryByProductCategory", tr("by &Product Category..."), SLOT(sDspSalesHistoryByProductCategory()), analysisHistMenu, "ViewSalesHistory", NULL, NULL, true , NULL },
    { "sa.dspSalesHistoryByItem", tr("by &Item..."), SLOT(sDspSalesHistoryByItem()), analysisHistMenu, "ViewSalesHistory", NULL, NULL, true , NULL },

    // Sales | Analysis | Brief Sales History
    { "menu",	tr("Brie&f Sales History"),           (char*)analysisBrfHistMenu,	analysisMenu,	"true",	NULL, NULL, true, NULL },
    { "sa.dspBriefSalesHistoryBySalesRep", tr("by &Sales Rep..."), SLOT(sDspBriefSalesHistoryBySalesRep()), analysisBrfHistMenu, "ViewSalesHistory", NULL, NULL, true , NULL },
    { "separator",	NULL,	NULL,	analysisBrfHistMenu,	"true",		NULL, NULL, true, NULL },
    { "sa.dspBriefSalesHistoryByCustomerType", tr("by Customer &Type..."), SLOT(sDspBriefSalesHistoryByCustomerType()), analysisBrfHistMenu, "ViewSalesHistory", NULL, NULL, true , NULL },
    { "sa.dspBriefSalesHistoryByCustomer", tr("by &Customer..."), SLOT(sDspBriefSalesHistoryByCustomer()), analysisBrfHistMenu, "ViewSalesHistory", NULL, NULL, true , NULL },
    
    // Sales | Analysis | Summarized Sales History
    { "menu",	tr("Summari&zed Sales History"),           (char*)analysisSumHistMenu,	analysisMenu,	"true",	NULL, NULL, true, NULL },
    { "sa.dspSummarizedSalesHistoryBySalesRep", tr("by &Sales Rep..."), SLOT(sDspSummarizedSalesBySalesRep()), analysisSumHistMenu, "ViewSalesHistory", NULL, NULL, true , NULL },
    { "sa.dspSummarizedSalesHistoryByShippingZoneByItem", tr("by Shipping &Zone by Item..."), SLOT(sDspSummarizedSalesHistoryByShippingZone()), analysisSumHistMenu, "ViewSalesHistory", NULL, NULL, true , NULL },
    { "separator",	NULL,	NULL,	analysisSumHistMenu,	"true",		NULL, NULL, true, NULL },
    { "sa.dspSummarizedSalesHistoryByCustomerType", tr("by Customer &Type..."), SLOT(sDspSummarizedSalesByCustomerType()), analysisSumHistMenu, "ViewSalesHistory", NULL, NULL, true , NULL },
    { "sa.dspSummarizedSalesHistoryByCustomer", tr("by &Customer..."), SLOT(sDspSummarizedSalesByCustomer()), analysisSumHistMenu, "ViewSalesHistory", NULL, NULL, true , NULL },
    { "sa.dspSummarizedSalesHistoryByCustomerTypeByItem", tr("by Customer T&ype by Item..."), SLOT(sDspSummarizedSalesByCustomerTypeByItem()), analysisSumHistMenu, "ViewSalesHistory", NULL, NULL, true , NULL },
    { "sa.dspSummarizedSalesHistoryByCustomerByItem", tr("by C&ustomer by Item..."), SLOT(sDspSummarizedSalesByCustomerByItem()), analysisSumHistMenu, "ViewSalesHistory", NULL, NULL, true , NULL },
    { "separator",	NULL,	NULL,	analysisSumHistMenu,	"true",		NULL, NULL, true, NULL },
    { "sa.dspSummarizedSalesHistoryByItem", tr("by &Item..."), SLOT(sDspSummarizedSalesByItem()), analysisSumHistMenu, "ViewSalesHistory", NULL, NULL, true , NULL },

    // Sales | Analysis | Time-Phased Sales History
    { "menu",	tr("Time &Phased Sales History"),           (char*)analysisTpHistMenu,	analysisMenu,	"true",	NULL, NULL, true, NULL },
    { "sa.dspTimePhasedSalesHistoryByCustomerGroup", tr("by Customer &Group..."), SLOT(sDspTimePhasedSalesByCustomerGroup()), analysisTpHistMenu, "ViewSalesHistory+ViewCustomerPrices", NULL, NULL, true , NULL },
    { "sa.dspTimePhasedSalesHistoryByCustomer", tr("by &Customer..."), SLOT(sDspTimePhasedSalesByCustomer()), analysisTpHistMenu, "ViewSalesHistory+ViewCustomerPrices", NULL, NULL, true , NULL },
    { "sa.dspTimePhasedSalesHistoryByCustomerByItem", tr("by C&ustomer by Item..."), SLOT(sDspTimePhasedSalesByCustomerByItem()), analysisTpHistMenu, "ViewSalesHistory+ViewCustomerPrices", NULL, NULL, true , NULL },
    { "separator",	NULL,	NULL,	analysisTpHistMenu,	"true",		NULL, NULL, true, NULL },
    { "sa.dspTimePhasedSalesHistoryByProductCategory", tr("by &Product Category..."), SLOT(sDspTimePhasedSalesByProductCategory()), analysisTpHistMenu, "ViewSalesHistory+ViewCustomerPrices", NULL, NULL, true , NULL },
    { "sa.dspTimePhasedSalesHistoryByItem", tr("by &Item..."), SLOT(sDspTimePhasedSalesByItem()), analysisTpHistMenu, "ViewSalesHistory+ViewCustomerPrices", NULL, NULL, true , NULL },

    // Sales | Analysis
    { "sa.rptPrintSASpecialCalendarForm", tr("Print S/A Special Calendar Form..."), SLOT(sPrintSASpecialCalendarForm()), analysisMenu, "ViewSalesHistory", NULL, NULL, true, NULL },

    { "separator",	NULL,	NULL,	mainMenu,	"true",		NULL, NULL, true, NULL },

    // Sales | Prospect
    { "menu",	tr("&Prospect"),       (char*)prospectMenu,	mainMenu,	"true",	NULL, NULL, true, NULL },
    { "so.enterNewProspect", tr("&New..."),	SLOT(sNewProspect()), prospectMenu, "MaintainProspectMasters",	NULL, NULL, true, NULL },
    { "so.prospects", tr("&List..."),	SLOT(sProspects()), prospectMenu, "MaintainProspectMasters ViewProspectMasters",	NULL, NULL, true, NULL },

    // Sales | Customer
    { "menu",	tr("&Customer"),       (char*)customerMenu,	mainMenu,	"true",	NULL, NULL, true, NULL },
    { "so.enterNewCustomer", tr("&New..."),	SLOT(sNewCustomer()), customerMenu, "MaintainCustomerMasters",	NULL, NULL, true, NULL },
    { "so.customers", tr("&List..."),	SLOT(sCustomers()), customerMenu, "MaintainCustomerMasters ViewCustomerMasters",	NULL, NULL, true, NULL },
    { "separator",	NULL,	NULL,	customerMenu,	"true",		NULL, NULL, true, NULL },
    { "so.customerWorkbench", tr("&Workbench..."),	SLOT(sCustomerWorkbench()), customerMenu, "MaintainCustomerMasters ViewCustomerMasters",	QPixmap(":/images/customerInformationWorkbench.png"), toolBar,  true, tr("Customer Workbench") }, 
    { "separator",	NULL,	NULL,	customerMenu,	"true",		NULL, NULL, true, NULL },
    { "so.customerGroups", tr("&Groups..."),	SLOT(sCustomerGroups()), customerMenu, "MaintainCustomerGroups ViewCustomerGroups",	NULL, NULL, true, NULL },
   
    // Sales | Pricing
    { "menu",	tr("Pricing"),       (char*)pricingMenu,	mainMenu,	"true",	NULL, NULL, true, NULL },
    { "so.itemListPrice", tr("Item &List Price..."),	SLOT(sItemListPrice()), pricingMenu, "MaintainListPrices ViewListPrices",	NULL, NULL, true, NULL },
    { "so.updateListPricesByProductCategory", tr("&Update List Prices..."),	SLOT(sUpdateListPricesByProductCategory()), pricingMenu, "MaintainListPrices",	NULL, NULL, true, NULL },
    { "separator",	NULL,	NULL,	pricingMenu,	"true",		NULL, NULL, true, NULL },
    { "so.pricingSchedules", tr("Pricing &Schedules..."),	SLOT(sPricingSchedules()), pricingMenu, "MaintainListPrices ViewListPrices",	NULL, NULL, true, NULL },
    { "so.updatePricingSchedules", tr("Update Pricing Schedules..."),	SLOT(sUpdatePrices()), pricingMenu, "UpdatePricingSchedules",	NULL, NULL, true, NULL },
    { "so.pricingScheduleAssignments", tr("Pricing Schedule Assi&gnments..."),	SLOT(sPricingScheduleAssignments()), pricingMenu, "AssignPricingSchedules",	NULL, NULL, true, NULL },
    { "so.sales", tr("S&ales..."),	SLOT(sSales()), pricingMenu, "CreateSales",	NULL, NULL, true, NULL },
    { "separator",	NULL,	NULL,	pricingMenu,	"true",		NULL, NULL, true, NULL },

    // Sales | Pricing | Reports
    { "menu",	tr("&Reports"),	(char*)pricingReportsMenu,	pricingMenu,	"true",	NULL, NULL, true, NULL },
    { "so.dspPricesByCustomerType", tr("Item Prices by Customer &Type..."),	SLOT(sDspPricesByCustomerType()), pricingReportsMenu, "ViewCustomerPrices", NULL, NULL,	 true, NULL },
    { "so.dspPricesByCustomer", tr("Item Prices by &Customer..."),	SLOT(sDspPricesByCustomer()), pricingReportsMenu, "ViewCustomerPrices", NULL, NULL,	 true, NULL },
    { "so.dspPricesByItem", tr("Item Prices by &Item..."),	SLOT(sDspPricesByItem()), pricingReportsMenu, "ViewCustomerPrices", NULL, NULL,	 true, NULL },
    { "separator",	NULL,	NULL,	pricingReportsMenu,	"true",		NULL, NULL, true, NULL },
    { "so.dspFreightPricesByCustomerType", tr("Freight Prices by Customer &Type..."),	SLOT(sDspFreightPricesByCustomerType()), pricingReportsMenu, "ViewCustomerPrices", NULL, NULL,	 true, NULL },
    { "so.dspFreightPricesByCustomer", tr("Freight Prices by &Customer..."),	SLOT(sDspFreightPricesByCustomer()), pricingReportsMenu, "ViewCustomerPrices", NULL, NULL,	 true, NULL },
    
    { "separator",	NULL,	NULL,	mainMenu,	"true",		NULL, NULL, true, NULL },

    { "menu",	tr("&Utilities"),         (char*)utilitiesMenu,	mainMenu,	"true",	NULL, NULL, true, NULL },
    { "so.customerInformationExport", tr("&Customer Information Export..."),	SLOT(sDspCustomerInformationExport()), utilitiesMenu, "MaintainCustomerMasters",	NULL, NULL, true, NULL },
    { "so.reassignCustomerTypeByCustomerType", tr("&Reassign Customer Type by Customer Type..."),	SLOT(sReassignCustomerTypeByCustomerType()), utilitiesMenu, "MaintainCustomerMasters",	NULL, NULL, true, NULL },
    { "so.updateCreditStatusByCustomer", tr("&Update Credit Status by Customer..."),	SLOT(sUpdateCreditStatusByCustomer()), utilitiesMenu, "MaintainCustomerMasters UpdateCustomerCreditStatus",	NULL, NULL, true, NULL },
    { "separator",	NULL,	NULL,	utilitiesMenu,	"true",		NULL, NULL, true, NULL },
    { "so.purgeInvoices",		     tr("Purge &Invoices..."),		SLOT(sPurgeInvoices()), utilitiesMenu, "PurgeInvoices",	NULL, NULL, true, NULL },
    { "so.purgeCreditMemos",		     tr("Purge Credit &Memos..."),	SLOT(sPurgeCreditMemos()), utilitiesMenu, "PurgeCreditMemos",	NULL, NULL, true, NULL },
    { "separator",	NULL,	NULL,	utilitiesMenu,	"true",		NULL, NULL, _metrics->boolean("EnableSOReservations"), NULL },
    { "so.allocateReservations", tr("Allocate Reser&vations..."), SLOT(sAllocateReservations()), utilitiesMenu, "MaintainReservations", NULL, NULL, _metrics->boolean("EnableSOReservations") , NULL },
    { "separator",	NULL,	NULL,	utilitiesMenu,	"true",		NULL, NULL, true, NULL },
    { "sa.archieveSalesHistory", tr("&Archive Sales History..."), SLOT(sArchiveSalesHistory()), utilitiesMenu, "ArchiveSalesHistory", NULL, NULL, true , NULL },
    { "sa.restoreSalesHistory", tr("Restore &Sales History..."), SLOT(sRestoreSalesHistory()), utilitiesMenu, "RestoreSalesHistory", NULL, NULL, true , NULL },

    { "so.setup",	tr("&Setup..."),	SLOT(sSetup()),	mainMenu,	NULL,	NULL,	NULL,	true, NULL	},

  };

  addActionsToMenu(acts, sizeof(acts) / sizeof(acts[0]));

  parent->populateCustomMenu(mainMenu, "Sales");
  QAction * m = parent->menuBar()->addMenu(mainMenu);
  if(m)
    m->setText(tr("S&ales"));
}

void menuSales::addActionsToMenu(actionProperties acts[], unsigned int numElems)
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

//  Orders
void menuSales::sNewSalesOrder()
{
  salesOrder::newSalesOrder(-1);
}

void menuSales::sOpenSalesOrders()
{
  omfgThis->handleNewWindow(new openSalesOrders());
}

void menuSales::sPackingListBatch()
{
  omfgThis->handleNewWindow(new packingListBatch());
}

void menuSales::sPrintPackingList()
{
  printPackingList(parent, "", TRUE).exec();
}

void menuSales::sNewQuote()
{
  ParameterList params;
  params.append("mode", "newQuote");

  salesOrder *newdlg = new salesOrder();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void menuSales::sQuotes()
{
  omfgThis->handleNewWindow(new quotes());
}

//  Billing
void menuSales::sUninvoicedShipments()
{
  omfgThis->handleNewWindow(new uninvoicedShipments());
}

void menuSales::sSelectShippedOrdersForBilling()
{
  selectShippedOrders(parent, "", TRUE).exec();
}

void menuSales::sSelectOrderForBilling()
{
  ParameterList params;
  params.append("mode", "new");

  selectOrderForBilling *newdlg = new selectOrderForBilling();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void menuSales::sBillingEditList()
{
  omfgThis->handleNewWindow(new billingEditList());
}

void menuSales::sDspBillingSelections()
{
  omfgThis->handleNewWindow(new dspBillingSelections());
}

void menuSales::sCreateInvoices()
{
  createInvoices(parent, "", TRUE).exec();
}

void menuSales::sUnpostedInvoices()
{
  omfgThis->handleNewWindow(new unpostedInvoices());
}

void menuSales::sPrintInvoices()
{
  printInvoices(parent, "", TRUE).exec();
}

void menuSales::sPrintInvoicesByShipvia()
{
  printInvoicesByShipvia(parent, "", TRUE).exec();
}

void menuSales::sReprintInvoices()
{
  reprintInvoices(parent, "", TRUE).exec();
}

void menuSales::sPostInvoices()
{
  postInvoices(parent, "", TRUE).exec();
}

void menuSales::sPurgeInvoices()
{
  purgeInvoices(parent, "", TRUE).exec();
}


void menuSales::sNewCreditMemo()
{
  ParameterList params;
  params.append("mode", "new");

  creditMemo *newdlg = new creditMemo();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void menuSales::sUnpostedCreditMemos()
{
  omfgThis->handleNewWindow(new unpostedCreditMemos());
}

void menuSales::sCreditMemoEditList()
{
  omfgThis->handleNewWindow(new creditMemoEditList());
}

void menuSales::sPrintCreditMemos()
{
  printCreditMemos(parent, "", TRUE).exec();
}

void menuSales::sReprintCreditMemos()
{
  reprintCreditMemos(parent, "", TRUE).exec();
}

void menuSales::sPostCreditMemos()
{
  postCreditMemos(parent, "", TRUE).exec();
}

void menuSales::sPurgeCreditMemos()
{
  purgeCreditMemos(parent, "", TRUE).exec();
}

void menuSales::sNewReturn()
{
  ParameterList params;
  params.append("mode", "new");

  returnAuthorization *newdlg = new returnAuthorization();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void menuSales::sOpenReturns()
{
  ParameterList params;
  params.append("run");
  openReturnAuthorizations* win = new openReturnAuthorizations();
  win->set(params);
  omfgThis->handleNewWindow(win);
}

void menuSales::sReturnsWorkbench()
{
  omfgThis->handleNewWindow(new returnAuthorizationWorkbench());
}

//  S/O | Item Pricing
void menuSales::sItemListPrice()
{
  itemListPrice newdlg(parent, "", TRUE);
  newdlg.exec();
}

void menuSales::sUpdateListPricesByProductCategory()
{
  updateListPricesByProductCategory newdlg(parent, "", TRUE);
  newdlg.exec();
}

void menuSales::sPricingSchedules()
{
  omfgThis->handleNewWindow(new itemPricingSchedules());
}

void menuSales::sPricingScheduleAssignments()
{
  omfgThis->handleNewWindow(new pricingScheduleAssignments());
}

void menuSales::sSales()
{
  omfgThis->handleNewWindow(new sales());
}

void menuSales::sUpdatePrices()
{
  updatePrices(parent, "", TRUE).exec();
}

void menuSales::sDspPricesByItem()
{
  omfgThis->handleNewWindow(new dspPricesByItem());
}

void menuSales::sDspPricesByCustomer()
{
  omfgThis->handleNewWindow(new dspPricesByCustomer());
}

void menuSales::sDspPricesByCustomerType()
{
  omfgThis->handleNewWindow(new dspPricesByCustomerType());
}

void menuSales::sDspFreightPricesByCustomer()
{
  omfgThis->handleNewWindow(new dspFreightPricesByCustomer());
}

void menuSales::sDspFreightPricesByCustomerType()
{
  omfgThis->handleNewWindow(new dspFreightPricesByCustomerType());
}

void menuSales::sDspCustomersByCusttype()
{
  omfgThis->handleNewWindow(new dspCustomersByCustomerType());
}

void menuSales::sDspCustomersByCharacteristic()
{
  omfgThis->handleNewWindow(new dspCustomersByCharacteristic());
}

void menuSales::sDspSalesOrderStatus()
{
  omfgThis->handleNewWindow(new dspSalesOrderStatus());
}

void menuSales::sDspInventoryAvailabilityByItem()
{
  omfgThis->handleNewWindow(new dspInventoryAvailabilityByItem());
}

void menuSales::sDspInventoryAvailabilityBySalesOrder()
{
  omfgThis->handleNewWindow(new dspInventoryAvailabilityBySalesOrder());
}

void menuSales::sDspInventoryAvailabilityByCustomerType()
{
  omfgThis->handleNewWindow(new dspInventoryAvailabilityByCustomerType());
}

void menuSales::sDspReservations()
{
  omfgThis->handleNewWindow(new dspReservations());
}

void menuSales::sDspOrderLookupByCustomer()
{
  omfgThis->handleNewWindow(new dspSalesOrdersByCustomer());
}

void menuSales::sDspOrderLookupByCustomerType()
{
  ParameterList params;
  params.append("custtype");

  dspSalesOrdersByParameterList *newdlg = new dspSalesOrdersByParameterList();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void menuSales::sDspOrderLookupByItem()
{
  omfgThis->handleNewWindow(new dspSalesOrdersByItem());
}

void menuSales::sDspOrderLookupByCustomerPO()
{
  omfgThis->handleNewWindow(new dspSalesOrdersByCustomerPO());
}

void menuSales::sDspQuoteLookupByCustomer()
{
  omfgThis->handleNewWindow(new dspQuotesByCustomer());
}

void menuSales::sDspQuoteLookupByItem()
{
  omfgThis->handleNewWindow(new dspQuotesByItem());
}

void menuSales::sDspBacklogByItem()
{
  omfgThis->handleNewWindow(new dspBacklogByItem());
}

void menuSales::sDspBacklogBySalesOrder()
{
  omfgThis->handleNewWindow(new dspBacklogBySalesOrder());
}

void menuSales::sDspBacklogByCustomer()
{
  omfgThis->handleNewWindow(new dspBacklogByCustomer());
}

void menuSales::sDspBacklogByCustomerType()
{
  ParameterList params;
  params.append("custtype");

  dspBacklogByParameterList *newdlg = new dspBacklogByParameterList();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void menuSales::sDspBacklogByCustomerGroup()
{
  ParameterList params;
  params.append("custgrp");

  dspBacklogByParameterList *newdlg = new dspBacklogByParameterList();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void menuSales::sDspBacklogByProductCategory()
{
  ParameterList params;
  params.append("prodcat");

  dspBacklogByParameterList *newdlg = new dspBacklogByParameterList();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void menuSales::sDspSummarizedBacklogByWarehouse()
{
  omfgThis->handleNewWindow(new dspSummarizedBacklogByWarehouse());
}

void menuSales::sDspPartiallyShippedOrders()
{
  omfgThis->handleNewWindow(new dspPartiallyShippedOrders());
}

void menuSales::sDspEarnedCommissions()
{
  omfgThis->handleNewWindow(new dspEarnedCommissions());
}

void menuSales::sDspBriefEarnedCommissions()
{
  omfgThis->handleNewWindow(new dspBriefEarnedCommissions());
}

void menuSales::sDspTaxHistory()
{
  omfgThis->handleNewWindow(new dspTaxHistory());
}


void menuSales::sDspSalesHistoryByCustomer()
{
  omfgThis->handleNewWindow(new dspSalesHistoryByCustomer());
}

void menuSales::sDspSalesHistoryByBilltoName()
{
  omfgThis->handleNewWindow(new dspSalesHistoryByBilltoName());
}

void menuSales::sDspSalesHistoryByShipTo()
{
  omfgThis->handleNewWindow(new dspSalesHistoryByShipTo());
}

void menuSales::sDspSalesHistoryByItem()
{
  omfgThis->handleNewWindow(new dspSalesHistoryByItem());
}

void menuSales::sDspSalesHistoryBySalesRep()
{
  omfgThis->handleNewWindow(new dspSalesHistoryBySalesrep());
}

void menuSales::sDspSalesHistoryByProductCategory()
{
  ParameterList params;
  params.append("prodcat");

  dspSalesHistoryByParameterList *newdlg = new dspSalesHistoryByParameterList();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void menuSales::sDspSalesHistoryByCustomerType()
{
  ParameterList params;
  params.append("custtype");

  dspSalesHistoryByParameterList *newdlg = new dspSalesHistoryByParameterList();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void menuSales::sDspSalesHistoryByCustomerGroup()
{
  ParameterList params;
  params.append("custgrp");

  dspSalesHistoryByParameterList *newdlg = new dspSalesHistoryByParameterList();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void menuSales::sDspBriefSalesHistoryByCustomer()
{
  omfgThis->handleNewWindow(new dspBriefSalesHistoryByCustomer());
}

void menuSales::sDspBriefSalesHistoryByCustomerType()
{
  omfgThis->handleNewWindow(new dspBriefSalesHistoryByCustomerType());
}

void menuSales::sDspBriefSalesHistoryBySalesRep()
{
  omfgThis->handleNewWindow(new dspBriefSalesHistoryBySalesRep());
}

void menuSales::sDspBookingsByCustomer()
{
  omfgThis->handleNewWindow(new dspBookingsByCustomer());
}

void menuSales::sDspBookingsByCustomerGroup()
{
  omfgThis->handleNewWindow(new dspBookingsByCustomerGroup());
}

void menuSales::sDspBookingsByShipTo()
{
  omfgThis->handleNewWindow(new dspBookingsByShipTo());
}

void menuSales::sDspBookingsByItem()
{
  omfgThis->handleNewWindow(new dspBookingsByItem());
}

void menuSales::sDspBookingsByProductCategory()
{
  omfgThis->handleNewWindow(new dspBookingsByProductCategory());
}

void menuSales::sDspBookingsBySalesRep()
{
  omfgThis->handleNewWindow(new dspBookingsBySalesRep());
}

void menuSales::sDspSummarizedSalesByCustomer()
{
  omfgThis->handleNewWindow(new dspSummarizedSalesByCustomer());
}

void menuSales::sDspSummarizedSalesByCustomerType()
{
  omfgThis->handleNewWindow(new dspSummarizedSalesByCustomerType());
}

void menuSales::sDspSummarizedSalesByCustomerByItem()
{
  omfgThis->handleNewWindow(new dspSummarizedSalesByCustomerByItem());
}

void menuSales::sDspSummarizedSalesByCustomerTypeByItem()
{
  omfgThis->handleNewWindow(new dspSummarizedSalesByCustomerTypeByItem());
}

void menuSales::sDspSummarizedSalesByItem()
{
  omfgThis->handleNewWindow(new dspSummarizedSalesByItem());
}

void menuSales::sDspSummarizedSalesBySalesRep()
{
  omfgThis->handleNewWindow(new dspSummarizedSalesBySalesRep());
}

void menuSales::sDspSummarizedSalesHistoryByShippingZone()
{
  omfgThis->handleNewWindow(new dspSummarizedSalesHistoryByShippingZone());
}

void menuSales::sDspTimePhasedBookingsByItem()
{
  omfgThis->handleNewWindow(new dspTimePhasedBookingsByItem());
}

void menuSales::sDspTimePhasedBookingsByProductCategory()
{
  omfgThis->handleNewWindow(new dspTimePhasedBookingsByProductCategory());
}

void menuSales::sDspTimePhasedBookingsByCustomer()
{
  omfgThis->handleNewWindow(new dspTimePhasedBookingsByCustomer());
}

void menuSales::sDspTimePhasedSalesByItem()
{
  omfgThis->handleNewWindow(new dspTimePhasedSalesByItem());
}

void menuSales::sDspTimePhasedSalesByProductCategory()
{
  omfgThis->handleNewWindow(new dspTimePhasedSalesByProductCategory());
}

void menuSales::sDspTimePhasedSalesByCustomer()
{
  omfgThis->handleNewWindow(new dspTimePhasedSalesByCustomer());
}

void menuSales::sDspTimePhasedSalesByCustomerGroup()
{
  omfgThis->handleNewWindow(new dspTimePhasedSalesByCustomerGroup());
}

void menuSales::sDspTimePhasedSalesByCustomerByItem()
{
  omfgThis->handleNewWindow(new dspTimePhasedSalesByCustomerByItem());
}

void menuSales::sPrintSalesOrderForm()
{
  printSoForm(parent, "", TRUE).exec();
}

void menuSales::sPrintReturnAuthForm()
{
  printRaForm(parent, "", TRUE).exec();
}


//  Master Information
void menuSales::sNewCustomer()
{
  ParameterList params;
  params.append("mode", "new");

  customer *newdlg = new customer();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void menuSales::sCustomerWorkbench()
{
  ParameterList params;
  if (_privileges->check("MaintainCustomerMasters"))
    params.append("mode", "edit");
  else
    params.append("mode", "view");
    
  customer *newdlg = new customer();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void menuSales::sCustomers()
{
  omfgThis->handleNewWindow(new customers());
}

void menuSales::sUpdateCreditStatusByCustomer()
{
  updateCreditStatusByCustomer(parent, "", TRUE).exec();
}

void menuSales::sCustomerGroups()
{
  omfgThis->handleNewWindow(new customerGroups());
}

void menuSales::sNewProspect()
{
  ParameterList params;
  params.append("mode", "new");

  prospect *newdlg = new prospect();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void menuSales::sProspects()
{
  omfgThis->handleNewWindow(new prospects());
}

void menuSales::sDspCustomerInformationExport()
{
  omfgThis->handleNewWindow(new dspCustomerInformationExport());
}

void menuSales::sReassignCustomerTypeByCustomerType()
{
  reassignCustomerTypeByCustomerType(parent, "", TRUE).exec();
}

void menuSales::sArchiveSalesHistory()
{
  ParameterList params;
  params.append("archieve");

  archRestoreSalesHistory newdlg(parent, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void menuSales::sRestoreSalesHistory()
{
  ParameterList params;
  params.append("restore");

  archRestoreSalesHistory newdlg(parent, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void menuSales::sPrintSASpecialCalendarForm()
{
  printSASpecialCalendarForm(parent, "", TRUE).exec();
}

void menuSales::sAllocateReservations()
{
  allocateReservations(parent, "", TRUE).exec();
}


void menuSales::sSetup()
{
  ParameterList params;
  params.append("module", Xt::SalesModule);

  setup newdlg(parent);
  newdlg.set(params);
  newdlg.exec();
}
