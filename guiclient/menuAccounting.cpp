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
#include <QToolBar>

#include <parameter.h>

#include "guiclient.h"

//AP
#include "purchaseOrder.h"
#include "unpostedPurchaseOrders.h"
#include "printPurchaseOrder.h"
#include "postPurchaseOrder.h"

#include "dspUninvoicedReceivings.h"
#include "voucher.h"
#include "miscVoucher.h"
#include "openVouchers.h"
#include "voucheringEditList.h"
#include "postVouchers.h"

#include "selectPayments.h"
#include "selectedPayments.h"
#include "prepareCheckRun.h"
#include "viewCheckRun.h"
#include "miscCheck.h"
#include "printCheck.h"
#include "printChecks.h"
#include "voidChecks.h"
#include "postCheck.h"
#include "postChecks.h"
#include "apWorkBench.h"

#include "unappliedAPCreditMemos.h"
#include "apOpenItem.h"

#include "dspVendorAPHistory.h"
#include "dspCheckRegister.h"
#include "dspAPApplications.h"
#include "dspVoucherRegister.h"
#include "dspAPOpenItemsByVendor.h"
#include "dspTimePhasedOpenAPItems.h"

// AR
#include "invoice.h"
#include "listRecurringInvoices.h"
#include "unpostedInvoices.h"
#include "printInvoices.h"
#include "reprintInvoices.h"
#include "postInvoices.h"
#include "purgeInvoices.h"

#include "cashReceipt.h"
#include "cashReceiptsEditList.h"
#include "postCashReceipts.h"

#include "unappliedARCreditMemos.h"
#include "arOpenItem.h"

#include "arWorkBench.h"

#include "dspCustomerARHistory.h"
#include "dspCashReceipts.h"
#include "dspARApplications.h"
#include "dspInvoiceInformation.h"
#include "dspAROpenItems.h"
#include "dspTimePhasedOpenARItems.h"
#include "dspInvoiceRegister.h"
#include "dspDepositsRegister.h"
#include "printJournal.h"
#include "printStatementByCustomer.h"
#include "printStatementsByCustomerType.h"

// GL
#include "glTransaction.h"
#include "glSeries.h"
#include "unpostedGlSeries.h"

#include "standardJournal.h"
#include "standardJournals.h"
#include "standardJournalGroups.h"
#include "postStandardJournal.h"
#include "postStandardJournalGroup.h"
#include "postJournals.h"
#include "dspStandardJournalHistory.h"

#include "financialLayouts.h"
#include "financialLayout.h"
#include "dspFinancialReport.h"

#include "dspGLTransactions.h"
#include "dspSummarizedGLTransactions.h"
#include "dspGLSeries.h"
#include "dspTrialBalances.h"
#include "dspJournals.h"

#include "companies.h"
#include "profitCenters.h"
#include "subaccounts.h"
#include "accountNumbers.h"
#include "subAccntTypes.h"
#include "accountingPeriods.h"
#include "accountingYearPeriods.h"
#include "taxCodes.h"
#include "taxclasses.h"
#include "taxTypes.h"
#include "taxZones.h"
#include "taxAuthorities.h"
#include "taxAssignments.h"
#include "taxRegistrations.h"
#include "dspTaxHistory.h"

#include "reconcileBankaccount.h"
#include "bankAdjustment.h"
#include "bankAdjustmentEditList.h"
#include "dspBankrecHistory.h"
#include "dspSummarizedBankrecHistory.h"

#include "budgets.h"
#include "maintainBudget.h"
#include "forwardUpdateAccounts.h"
#include "duplicateAccountNumbers.h"
#include "vendors.h"
#include "bankAccounts.h"
#include "checkFormats.h"

#include "customers.h"

#include "updateLateCustCreditStatus.h"
#include "createRecurringInvoices.h"
#include "syncCompanies.h"

#include "setup.h"

#include "menuAccounting.h"

menuAccounting::menuAccounting(GUIClient *Pparent) :
 QObject(Pparent)
{
  setObjectName("wmModule");
  parent = Pparent;

  toolBar = new QToolBar(tr("Accounting Tools"));
  toolBar->setObjectName("Accounting Tools");
  toolBar->setIconSize(QSize(32, 32));
  parent->addToolBar(toolBar);


  mainMenu		= new QMenu(parent);
  apMenu = new QMenu(parent);
  apPurchaseMenu = new QMenu(parent);
  apVoucherMenu = new QMenu(parent);
  apMemosMenu = new QMenu(parent);
  apPaymentsMenu = new QMenu(parent);
  apFormsMenu = new QMenu(parent);
  apReportsMenu = new QMenu(parent);
  arMenu = new QMenu(parent);
  arInvoicesMenu = new QMenu(parent);
  arMemosMenu = new QMenu(parent);
  arCashReceiptsMenu = new QMenu(parent);
  arFormsMenu = new QMenu(parent);
  arReportsMenu = new QMenu(parent);
  glMenu = new QMenu(parent);
  glEnterTransactionMenu = new QMenu(parent);
  glStandardJournalsMenu = new QMenu(parent);
  bankrecMenu = new QMenu(parent);
  bankrecReportsMenu = new QMenu(parent);
  financialReportsMenu = new QMenu(parent);
  glReportsMenu = new QMenu(parent);
  calendarMenu = new QMenu(parent);
  coaMenu = new QMenu(parent);
  budgetMenu = new QMenu(parent);
  taxMenu = new QMenu(parent);
  taxReportsMenu = new QMenu(parent);
  utilitiesMenu = new QMenu(parent);

  mainMenu->setObjectName("menu.accnt");
  apMenu->setObjectName("menu.accnt.ap");
  apPurchaseMenu->setObjectName("menu.accnt.appurchase");
  apVoucherMenu->setObjectName("menu.accnt.apvoucher");
  apMemosMenu->setObjectName("menu.accnt.apmemos");
  apPaymentsMenu->setObjectName("menu.accnt.appayments");
  apFormsMenu->setObjectName("menu.accnt.apforms");
  apReportsMenu->setObjectName("menu.accnt.apreports");
  arMenu->setObjectName("menu.accnt.ar");
  arInvoicesMenu->setObjectName("menu.accnt.arinvoices");
  arMemosMenu->setObjectName("menu.accnt.armemos");
  arCashReceiptsMenu->setObjectName("menu.accnt.arcashreceipts");
  arFormsMenu->setObjectName("menu.accnt.arforms");
  arReportsMenu->setObjectName("menu.accnt.arreports");
  glMenu->setObjectName("menu.accnt.gl");
  glReportsMenu->setObjectName("menu.accnt.gl.glreports");
  glEnterTransactionMenu->setObjectName("menu.accnt.glentertransaction");
  glStandardJournalsMenu->setObjectName("menu.accnt.glstandardjournals");
  bankrecMenu->setObjectName("menu.accnt.bankrec");
  bankrecReportsMenu->setObjectName("menu.accnt.bankrec.bankrecreports");
  financialReportsMenu->setObjectName("menu.accnt.financialreports");
  calendarMenu->setObjectName("menu.accnt.calendar");
  coaMenu->setObjectName("menu.accnt.coa");
  budgetMenu->setObjectName("menu.accnt.budget");
  taxMenu->setObjectName("menu.accnt.tax");
  taxMenu->setObjectName("menu.accnt.tax.taxreports");
  utilitiesMenu->setObjectName("menu.accnt.utilities");

  actionProperties acts[] = { 
    // Accounting | Accounts Payable
    { "menu", tr("Accounts &Payable"), (char*)apMenu,	mainMenu, "true", NULL, NULL, true, NULL },
    
    // Accounting | Accaunts Payable | Purchase Orders
    { "menu", tr("Purchase &Order"), (char*)apPurchaseMenu, apMenu, "true", NULL, NULL, true, NULL },
    { "ap.enterPurchaseOrder", tr("&New..."), SLOT(sEnterPurchaseOrder()), apPurchaseMenu, "MaintainPurchaseOrders", NULL, NULL, true , NULL },
    { "ap.listUnpostedPurchaseOrders", tr("&List Open..."), SLOT(sUnpostedPurchaseOrders()), apPurchaseMenu, "MaintainPurchaseOrders ViewPurchaseOrders", NULL, NULL, true , NULL },
    { "separator", NULL, NULL, apPurchaseMenu, "true", NULL, NULL, true, NULL },
    { "ap.postPurchaseOrder", tr("&Release..."), SLOT(sPostPurchaseOrder()), apPurchaseMenu, "ReleasePurchaseOrders", NULL, NULL, true , NULL },

    // Accounting | Accaunts Payable | Vouchers
    { "menu", tr("&Voucher"), (char*)apVoucherMenu, apMenu, "true", NULL, NULL, true, NULL },
    { "ar.enterNewVoucher", tr("&New..."), SLOT(sEnterVoucher()), apVoucherMenu, "MaintainVouchers", NULL, NULL, true , NULL },
    { "ar.enterNewMiscVoucher", tr("New &Miscellaneous..."), SLOT(sEnterMiscVoucher()), apVoucherMenu, "MaintainVouchers", NULL, NULL, true , NULL },
    { "ar.listUnpostedVouchers", tr("&List Unposted..."), SLOT(sUnpostedVouchers()), apVoucherMenu, "MaintainVouchers ViewVouchers", QPixmap(":/images/listUnpostedVouchers.png"), toolBar, true , tr("List Unposted Vouchers") },
    { "separator", NULL, NULL, apVoucherMenu, "true", NULL, NULL, true, NULL },
    { "ar.postVouchers", tr("&Post..."), SLOT(sPostVouchers()), apVoucherMenu, "PostVouchers", NULL, NULL, true , NULL },

    // Accounting | Accaunts Payable | Memos
    { "menu", tr("&Memos"), (char*)apMemosMenu, apMenu, "true", NULL, NULL, true, NULL },
    { "ap.enterMiscCreditMemo", tr("&New Misc. Credit Memo..."), SLOT(sEnterMiscApCreditMemo()), apMemosMenu, "MaintainAPMemos", NULL, NULL, true , NULL },
    { "ap.unapplidCreditMemo", tr("&List Unapplied Credit Memos..."), SLOT(sUnappliedApCreditMemos()), apMemosMenu, "MaintainAPMemos ViewAPMemos", NULL, NULL, true , NULL },
    { "separator", NULL, NULL, apMemosMenu, "true", NULL, NULL, true, NULL },
    { "ap.enterMiscDebitMemo", tr("New &Misc. Debit Memo..."), SLOT(sEnterMiscApDebitMemo()), apMemosMenu, "MaintainAPMemos", NULL, NULL, true , NULL },

    // Accounting | Accaunts Payable |  Payments
    { "menu", tr("&Payments"), (char*)apPaymentsMenu, apMenu, "true", NULL, NULL, true, NULL },
    { "ap.selectPayments", tr("&Select..."), SLOT(sSelectPayments()), apPaymentsMenu, "MaintainPayments", NULL, NULL, true , tr("Select Payments") },
    { "ap.listSelectPayments", tr("&List Selected..."), SLOT(sSelectedPayments()), apPaymentsMenu, "MaintainPayments", NULL, NULL, true , NULL },
    { "separator", NULL, NULL, apPaymentsMenu, "true", NULL, NULL, true, NULL },
    { "ap.prepareCheckRun", tr("&Prepare Check Run..."), SLOT(sPrepareCheckRun()), apPaymentsMenu, "MaintainPayments", NULL, NULL, true , NULL },
    { "ap.createMiscCheck", tr("Create &Miscellaneous Check..."), SLOT(sCreateMiscCheck()), apPaymentsMenu, "MaintainPayments", NULL, NULL, true , NULL },
    { "ap.viewCheckRun", tr("Vie&w Check Run..."), SLOT(sViewCheckRun()), apPaymentsMenu, "MaintainPayments", NULL, NULL, true , NULL },
    { "separator", NULL, NULL, apPaymentsMenu, "true", NULL, NULL, true, NULL },
    { "ap.voidCheckRun", tr("&Void Check Run..."), SLOT(sVoidCheckRun()), apPaymentsMenu, "MaintainPayments", NULL, NULL, true , NULL },
    { "separator", NULL, NULL, apPaymentsMenu, "true", NULL, NULL, true, NULL },
    { "ap.postCheck", tr("Post &Check..."), SLOT(sPostCheck()), apPaymentsMenu, "PostPayments", NULL, NULL, true , NULL },
    { "ap.postChecks", tr("P&ost Checks..."), SLOT(sPostChecks()), apPaymentsMenu, "PostPayments", NULL, NULL, true , NULL },
                       
    { "separator", NULL, NULL, apMenu, "true", NULL, NULL, true, NULL },
    { "ap.workbench", tr("&Workbench..."), SLOT(sApWorkBench()), apMenu, "MaintainPayments MaintainAPMemos", QPixmap(":/images/viewCheckRun.png"), toolBar, true, tr("Payables Workbench") },
    { "separator", NULL, NULL, apMenu, "true", NULL, NULL, true, NULL },
    
    // Accounting | Accaunts Payable | Forms
    { "menu", tr("&Forms"), (char*)apFormsMenu, apMenu, "true", NULL, NULL, true, NULL },
    { "ap.printPurchaseOrder", tr("Print Purchase &Order..."), SLOT(sPrintPurchaseOrder()), apFormsMenu, "PrintPurchaseOrders", NULL, NULL, true , NULL },
    { "separator", NULL, NULL, apFormsMenu, "true", NULL, NULL, true, NULL },
    { "ap.printCheck", tr("Print &Check..."), SLOT(sPrintCheck()), apFormsMenu, "MaintainPayments", NULL, NULL, true , NULL },
    { "ap.printCheckRun", tr("Print Check &Run..."), SLOT(sPrintCheckRun()), apFormsMenu, "MaintainPayments", NULL, NULL, true , NULL },
    
    // Accounting | Accaunts Payable |  Reports
    { "menu", tr("&Reports"), (char*)apReportsMenu, apMenu, "true", NULL, NULL, true, NULL },
    { "ap.uninvoicedReceipts", tr("&Uninvoiced Receipts and Returns..."), SLOT(sDspUninvoicedReceipts()), apReportsMenu, "ViewUninvoicedReceipts MaintainUninvoicedReceipts", NULL, NULL, true , NULL },
    { "ap.voucheringEditList", tr("Vouchering &Edit List..."), SLOT(sVoucheringEditList()), apReportsMenu, "MaintainVouchers ViewVouchers", NULL, NULL, true , NULL },
    { "separator", NULL, NULL, apReportsMenu, "true", NULL, NULL, true, NULL },
    { "ap.dspOpenAPItemsByVendor", tr("Open &Payables..."), SLOT(sDspAPOpenItemsByVendor()), apReportsMenu, "ViewAPOpenItems", NULL, NULL, true , NULL },
    { "ap.dspAPAging", tr("&Aging..."), SLOT(sDspTimePhasedOpenAPItems()), apReportsMenu, "ViewAPOpenItems", QPixmap(":/images/apAging.png"), toolBar, true , tr("Payables Aging") },
    { "separator", NULL, NULL, apReportsMenu, "true", NULL, NULL, true, NULL },
    { "ap.dspCheckRegister", tr("&Check Register..."), SLOT(sDspCheckRegister()), apReportsMenu, "MaintainPayments", NULL, NULL, true , NULL },
    { "ap.dspVoucherRegister", tr("&Voucher Register..."), SLOT(sDspVoucherRegister()), apReportsMenu, "MaintainVouchers ViewVouchers", NULL, NULL, true , NULL },
    { "separator", NULL, NULL, apReportsMenu, "true", NULL, NULL, true, NULL },
    { "ap.dspAPApplications", tr("&Applications..."), SLOT(sDspAPApplications()), apReportsMenu, "ViewAPOpenItems", NULL, NULL, true , NULL },
    { "separator", NULL, NULL, apReportsMenu, "true", NULL, NULL, true, NULL },
    { "ap.dspVendorHistory", tr("Vendor &History..."), SLOT(sDspVendorHistory()), apReportsMenu, "ViewAPOpenItems", NULL, NULL, true , NULL },
    { "separator", NULL, NULL, apReportsMenu, "true", NULL, NULL, true, NULL },
    { "ap.rptPayablesJournal", tr("Pa&yables Journal..."), SLOT(sRptPayablesJournal()), apReportsMenu, "PrintAPJournals", NULL, NULL, true , NULL },
    { "ap.rptCheckJournal", tr("Check &Journal..."), SLOT(sRptCheckJournal()), apReportsMenu, "PrintAPJournals", NULL, NULL, true , NULL },
    
    { "separator", NULL, NULL, apMenu, "true", NULL, NULL, true, NULL },
    { "ap.vendors", tr("Ve&ndors..."), SLOT(sVendors()), apMenu, "MaintainVendors ViewVendors", NULL, NULL, true , NULL },
    
    // Accounting | Accounts Receivable
    { "menu", tr("Accounts Recei&vable"),	(char*)arMenu,	mainMenu, "true", NULL, NULL, true, NULL },
  
    // Accounting | Accounts Receivable | Invoices
    { "menu", tr("&Invoice"), (char*)arInvoicesMenu,	arMenu, "true",	 NULL, NULL, true, NULL },
    { "ar.createInvoice", tr("&New..."), SLOT(sCreateInvoice()), arInvoicesMenu, "MaintainMiscInvoices", NULL, NULL, true , NULL },
    { "ar.listRecurringInvoices", tr("&List Recurring Invoices..."),	SLOT(sRecurringInvoices()), arInvoicesMenu, "SelectBilling",	NULL, NULL,  true, NULL },
    { "ar.listUnpostedInvoices", tr("&List Unposted..."), SLOT(sUnpostedInvoices()), arInvoicesMenu, "SelectBilling", QPixmap(":/images/unpostedInvoices.png"), toolBar, true , tr("List Unposted Invoices") },
    { "separator", NULL, NULL, arInvoicesMenu, "true", NULL, NULL, true, NULL },
    { "ar.postInvoices", tr("&Post..."), SLOT(sPostInvoices()), arInvoicesMenu, "PostMiscInvoices", NULL, NULL, true , NULL },

    // Accounting | Accounts Receivable | Memos
    { "menu", tr("&Memos"), (char*)arMemosMenu,	arMenu, "true",	 NULL, NULL, true, NULL },
    { "ar.enterMiscCreditMemo", tr("&New Misc. Credit Memo..."), SLOT(sEnterMiscArCreditMemo()), arMemosMenu, "MaintainARMemos", NULL, NULL, true , NULL },
    { "ar.unapplidCreditMemo", tr("&List Unapplied Credit Memos..."), SLOT(sUnappliedArCreditMemos()), arMemosMenu, "MaintainARMemos ViewARMemos", NULL, NULL, true , NULL },
    { "separator", NULL, NULL, arMemosMenu, "true", NULL, NULL, true, NULL },
    { "ar.enterMiscDebitMemo", tr("New &Misc. Debit Memo..."), SLOT(sEnterMiscArDebitMemo()), arMemosMenu, "MaintainARMemos", NULL, NULL, true , NULL },

    // Accounting | Accounts Receivable | Cash Receipts
    { "menu", tr("C&ash Receipt"), (char*)arCashReceiptsMenu,	arMenu, "true",	 NULL, NULL, true, NULL },
    { "ar.enterCashReceipt", tr("&New..."), SLOT(sEnterCashReceipt()), arCashReceiptsMenu, "MaintainCashReceipts", NULL, NULL, true , NULL },
    { "ar.cashReceiptEditList", tr("&Edit List..."), SLOT(sCashReceiptEditList()), arCashReceiptsMenu, "MaintainCashReceipts ViewCashReceipt", QPixmap(":/images/editCashReceipts.png"), toolBar, true , tr("Cash Receipt Edit List") },
    { "ar.postCashReceipts", tr("&Post..."), SLOT(sPostCashReceipts()), arCashReceiptsMenu, "PostCashReceipts", NULL, NULL, true , NULL },

    { "separator", NULL, NULL, arMenu, "true", NULL, NULL, true, NULL },
    { "ar.arWorkBench", tr("&Workbench..."), SLOT(sArWorkBench()), arMenu, "ViewAROpenItems" , QPixmap(":/images/arWorkbench.png"), toolBar, true , tr("Receivables Workbench") },

    { "separator", NULL, NULL, arMenu, "true", NULL, NULL, true, NULL },
    // Accounting | Accounts Receivable | Forms
    { "menu", tr("&Forms"), (char*)arFormsMenu,	arMenu, "true",	 NULL, NULL, true, NULL },
    { "ar.printInvoices", tr("Print &Invoices..."), SLOT(sPrintInvoices()), arFormsMenu, "PrintInvoices", NULL, NULL, true , NULL },
    { "ar.reprintInvoices", tr("&Re-Print Invoices..."), SLOT(sReprintInvoices()), arFormsMenu, "PrintInvoices", NULL, NULL, true , NULL },
    { "separator", NULL, NULL, arFormsMenu, "true", NULL, NULL, true, NULL },
    { "ar.printStatementByCustomer", tr("Print S&tatement by Customer..."), SLOT(sPrintStatementByCustomer()), arFormsMenu, "ViewAROpenItems", NULL, NULL, true , NULL },
    { "ar.printStatementsByCustomerType", tr("Print State&ments by Customer Type..."), SLOT(sPrintStatementsByCustomerType()), arFormsMenu, "ViewAROpenItems", NULL, NULL, true , NULL },

    // Accounting | Accounts Receivable | Reports
    { "menu", tr("&Reports"), (char*)arReportsMenu,	arMenu, "true",	 NULL, NULL, true, NULL },
    { "ar.dspInvoiceInformation", tr("&Invoice Information..."), SLOT(sDspInvoiceInformation()), arReportsMenu, "ViewAROpenItems", NULL, NULL, true , NULL },
    { "separator", NULL, NULL, arReportsMenu, "true", NULL, NULL, true, NULL },
    { "ar.dspOpenItems", tr("&Open Receivables..."), SLOT(sDspAROpenItems()), arReportsMenu, "ViewAROpenItems", NULL, NULL, true , NULL },
    { "ar.dspARAging", tr("A&ging..."), SLOT(sDspTimePhasedOpenItems()), arReportsMenu, "ViewAROpenItems", QPixmap(":/images/arAging.png"), toolBar, true , tr("Receivables Aging") },
    { "separator", NULL, NULL, arReportsMenu, "true", NULL, NULL, true, NULL }, 
    { "ar.dspInvoiceRegister", tr("In&voice Register..."), SLOT(sDspInvoiceRegister()), arReportsMenu, "ViewInvoiceRegister", NULL, NULL, true , NULL },
    { "ar.dspCashReceipts", tr("Cash &Receipts..."), SLOT(sDspCashReceipts()), arReportsMenu, "ViewAROpenItems", NULL, NULL, true , NULL },
    { "ar.dspARApplications", tr("&Applications..."), SLOT(sDspARApplications()), arReportsMenu, "ViewAROpenItems", NULL, NULL, true , NULL },
    { "ar.dspDepositsRegister", tr("&Deposits Register..."), SLOT(sDspDepositsRegister()), arReportsMenu, "ViewDepositsRegister", NULL, NULL, true , NULL },
    { "separator", NULL, NULL, arReportsMenu, "true", NULL, NULL, true, NULL },
    { "ar.dspCustomerHistory", tr("Customer &History..."), SLOT(sDspCustomerHistory()), arReportsMenu, "ViewAROpenItems", NULL, NULL, true , NULL },
    { "separator", NULL, NULL, arReportsMenu, "true", NULL, NULL, true, NULL },
    { "ar.rptSalesJournal", tr("Sales &Journal..."), SLOT(sRptSalesJournal()), arReportsMenu, "PrintARJournals", NULL, NULL, true , NULL },
    { "ar.rptCreditMemoJournal", tr("Credit &Memo Journal..."), SLOT(sRptCreditMemoJournal()), arReportsMenu, "PrintARJournals", NULL, NULL, true , NULL },

    { "separator", NULL, NULL, arMenu, "true", NULL, NULL, true, NULL },
    { "ar.customers", tr("&Customers..."), SLOT(sCustomers()), arMenu, "MaintainCustomerMasters ViewCustomerMasters", NULL, NULL, true , NULL },
  
    // Accounting | G/L
    { "menu",		    tr("General &Ledger"),		(char*)glMenu,	mainMenu,		"true",					NULL, NULL, true, NULL },
    
    // Accounting | G/L | Journals
    { "menu",		    tr("&Journal Entry"),		(char*)glEnterTransactionMenu,	glMenu,		"true",					NULL, NULL, true, NULL },
    { "gl.simpleEntry",	    tr("S&imple..."),	SLOT(sSimpleEntry()),		glEnterTransactionMenu,	"PostJournalEntries", NULL, NULL, true, NULL },
    { "gl.seriesEntry",     tr("&Series..."),	SLOT(sSeriesEntry()),		glEnterTransactionMenu,	"PostJournalEntries", NULL, NULL, true, NULL },
    { "separator",	    NULL,				NULL,			        glEnterTransactionMenu,   "true",					NULL, NULL, true, NULL },
    { "gl.unpostedEntries", tr("&List Unposted..."), SLOT(sUnpostedEntries()),	glEnterTransactionMenu,	"PostJournalEntries", QPixmap(":/images/journalEntries.png"), toolBar,  true, tr("List Unposted Journal Entries") },

    // Accounting | G/L | Standard Journals
    { "menu",			     tr("&Standard Journals"),		   (char*)glStandardJournalsMenu,	     glMenu,		   "true",					      NULL, NULL, true, NULL },
    { "gl.enterNewStandardJournal",  tr("&New..."),  SLOT(sEnterStandardJournal()),    glStandardJournalsMenu, "MaintainStandardJournals",     NULL, NULL, true, NULL },
    { "gl.listStandardJournals",     tr("&List..."),	   SLOT(sStandardJournals()),	     glStandardJournalsMenu, "MaintainStandardJournals",     NULL, NULL, true, NULL },
    { "gl.listStandardJournalGroups",tr("List &Groups..."),SLOT(sStandardJournalGroups()),   glStandardJournalsMenu, "MaintainStandardJournalGroups", NULL, NULL, true, NULL },
    { "separator",		     NULL,				   NULL,			     glStandardJournalsMenu, "true",					      NULL, NULL, true, NULL },
    { "gl.postStandardJournal",	     tr("&Post..."),	   SLOT(sPostStandardJournal()),     glStandardJournalsMenu, "PostStandardJournals",	      NULL, NULL, true, NULL },
    { "gl.postStandardJournalGroup", tr("Post G&roup..."), SLOT(sPostStandardJournalGroup()),glStandardJournalsMenu, "PostStandardJournalGroups",    NULL, NULL, true, NULL },

    { "separator",		     NULL,				   NULL,  glMenu, "true",					      NULL, NULL, _metrics->boolean("UseJournals"), NULL },
    { "gl.postJournals",   tr("&Post Journals to Ledger..."),	   SLOT(sPostJournals()), glMenu, "PostJournals",	      NULL, NULL, _metrics->boolean("UseJournals"), NULL },


    { "separator", NULL, NULL, glMenu, "true", NULL, NULL, true, NULL },
    
    // Accounting | General Ledger |  Reports
    { "menu", tr("&Reports"), (char*)glReportsMenu, glMenu, "true", NULL, NULL, true, NULL },
    { "gl.dspGLTransactions",		tr("&Transactions..."),		SLOT(sDspGLTransactions()),		glReportsMenu, "ViewGLTransactions",	NULL, NULL, true, NULL },
    { "gl.dspSummarizedGLTransactions",	tr("Su&mmarized Transactions..."),	SLOT(sDspSummarizedGLTransactions()),	glReportsMenu, "ViewGLTransactions",	NULL, NULL, true, NULL },
    { "gl.dspGLSeries",			tr("&Series..."),			SLOT(sDspGLSeries()),			glReportsMenu, "ViewGLTransactions",	NULL, NULL, true, NULL },
    { "gl.dspStandardJournalHistory",	tr("Standard &Journal History..."),	SLOT(sDspStandardJournalHistory()),	glReportsMenu, "ViewGLTransactions",	NULL, NULL, true, NULL },
    { "separator", NULL, NULL, glReportsMenu, "true", NULL, NULL, _metrics->boolean("UseJournals"), NULL },
    { "gl.dspJournals",	tr("Journals..."),	SLOT(sDspJournals()),	glReportsMenu, "ViewJournals",	NULL, NULL, _metrics->boolean("UseJournals"), NULL },

    { "menu",			tr("&Bank Reconciliation"), 	(char*)bankrecMenu,		mainMenu,    "true",						NULL, NULL, true, NULL },
    { "gl.reconcileBankaccnt",	tr("&Reconcile..."),SLOT(sReconcileBankaccount()),	bankrecMenu, "MaintainBankRec", QPixmap(":/images/bankReconciliation.png"), toolBar,  true, tr("Reconcile Bank Account") },
    { "separator",		NULL,				NULL,				bankrecMenu, "true",						NULL, NULL, true, NULL },
    { "gl.enterAdjustment",	tr("&New Adjustment..."),	SLOT(sEnterAdjustment()),	bankrecMenu, "MaintainBankAdjustments",	NULL, NULL, true, NULL },
    { "gl.adjustmentEditList",	tr("Adjustment Edit &List..."),	SLOT(sAdjustmentEditList()),	bankrecMenu, "MaintainBankAdjustments ViewBankAdjustments", NULL, NULL, true, NULL },
 
    { "separator", NULL, NULL, bankrecMenu, "true", NULL, NULL, true, NULL },
 
    // Accounting | Bank Reconciliation |  Reports
    { "menu", tr("&Reports"), (char*)bankrecReportsMenu, bankrecMenu, "true", NULL, NULL, true, NULL },
    { "gl.dspBankrecHistory",		tr("&History"),		SLOT(sDspBankrecHistory()),		bankrecReportsMenu, "ViewBankRec",		NULL, NULL, true, NULL },
    { "gl.dspSummarizedBankrecHistory",	tr("Summari&zed History"),	SLOT(sDspSummarizedBankrecHistory()),	bankrecReportsMenu, "ViewBankRec",		NULL, NULL, true, NULL },

    { "separator",		  NULL,					NULL,					mainMenu,		"true",					       NULL, NULL, true, NULL },
    
    // Accounting | Statements
    { "menu",			  tr("Financial &Statements"),		(char*)financialReportsMenu,		mainMenu,			"true",					       NULL, NULL, true, NULL },
    { "gl.createFinancialReports",tr("&New Financial Report..."),	SLOT(sNewFinancialReport()),		financialReportsMenu,		"MaintainFinancialLayouts", NULL, NULL, true, NULL },
    { "gl.editFinancialReports",  tr("&List Financial Reports..."),	SLOT(sFinancialReports()),		financialReportsMenu,		"MaintainFinancialLayouts", NULL, NULL, true, NULL },
    { "separator",		  NULL,					NULL,					financialReportsMenu,		"true",					       NULL, NULL, true, NULL },
    { "gl.dspTrialBalances",	  tr("View &Trial Balances..."),		SLOT(sDspTrialBalances()),		financialReportsMenu,		"ViewTrialBalances",	   QPixmap(":/images/viewTrialBalance.png"), toolBar,  true, NULL },
    { "gl.viewFinancialReport",	  tr("View &Financial Report..."),	SLOT(sViewFinancialReport()),		financialReportsMenu,		"ViewFinancialReports",   QPixmap(":/images/viewFinancialReport.png"), toolBar, true, NULL },

    { "separator",		  NULL,					NULL,					mainMenu,		"true",					       NULL, NULL, true, NULL },
    
    // Accounting | Fiscal Calendar
    { "menu", tr("&Fiscal Calendar"), (char*)calendarMenu, mainMenu,	"true",	NULL, NULL, true, NULL },
    { "gl.accountingYearPeriods",	tr("Fiscal &Years..."),	SLOT(sAccountingYearPeriods()),	calendarMenu,	"MaintainAccountingPeriods",	NULL, NULL, true, NULL },
    { "gl.accountingPeriods",	tr("Accounting &Periods..."),	SLOT(sAccountingPeriods()),	calendarMenu,	"MaintainAccountingPeriods",	NULL, NULL, true, NULL },
    
    // Accounting | Account
    { "menu", tr("&Account"), (char*)coaMenu, mainMenu,	"true",	NULL, NULL, true, NULL },
    { "gl.accountNumbers",	tr("&Chart of Accounts..."),	SLOT(sAccountNumbers()), coaMenu,	"MaintainChartOfAccounts",	NULL, NULL, true, NULL },
    { "gl.companies",		tr("C&ompanies..."),		SLOT(sCompanies()),		coaMenu,	"MaintainChartOfAccounts", NULL, NULL, (_metrics->value("GLCompanySize").toInt() > 0), NULL },
    { "gl.profitCenterNumber",	tr("&Profit Center Numbers..."),	SLOT(sProfitCenters()),	coaMenu,	"MaintainChartOfAccounts", NULL, NULL, (_metrics->value("GLProfitSize").toInt() > 0), NULL },
    { "gl.subaccountNumbers",	tr("&Subaccount Numbers..."),	SLOT(sSubaccounts()), coaMenu,	"MaintainChartOfAccounts", NULL, NULL, (_metrics->value("GLSubaccountSize").toInt() > 0), NULL },
    { "gl.subAccntTypes",	tr("Su&baccount Types..."),	SLOT(sSubAccntTypes()),	coaMenu,	"MaintainChartOfAccounts",	NULL, NULL, true, NULL },

    // Accounting | Budget
    { "menu", tr("Bu&dget"), (char*)budgetMenu, mainMenu,	"true",	NULL, NULL, true, NULL },
    { "gl.maintainBudget",	tr("&New Budget..."),	SLOT(sMaintainBudget()), budgetMenu,	"MaintainBudgets",	NULL, NULL, true, NULL },
    { "gl.maintainBudgets",	tr("&List Budgets..."),	SLOT(sBudgets()),	 budgetMenu,	"MaintainBudgets ViewBudgets",	NULL, NULL, true, NULL },

    // Accounting | Tax
    { "menu", tr("&Tax"), (char*)taxMenu, mainMenu,	"true",	NULL, NULL, true, NULL },
    { "gl.taxAuthorities",	tr("Tax &Authorities..."),	SLOT(sTaxAuthorities()),	taxMenu,	"MaintainTaxAuthorities ViewTaxAuthorities", NULL, NULL, true, NULL },
    { "gl.taxZones",		tr("Tax &Zones..."),		SLOT(sTaxZones()),		taxMenu,	"MaintainTaxZones ViewTaxZones",             NULL, NULL, true, NULL }, 
    { "gl.taxClasses",		tr("Tax &Classes..."),		SLOT(sTaxClasses()),		taxMenu,	"MaintainTaxClasses ViewTaxClasses",         NULL, NULL, true, NULL }, 
    { "gl.taxCodes",		tr("Tax &Codes..."),		SLOT(sTaxCodes()),		taxMenu,	"MaintainTaxCodes ViewTaxCodes",             NULL, NULL, true, NULL },
    { "gl.taxTypes",		tr("Tax &Types..."),		SLOT(sTaxTypes()),		taxMenu,	"MaintainTaxTypes ViewTaxTypes",             NULL, NULL, true, NULL },
    { "gl.taxAssignments",	tr("Tax Assi&gnments..."),	SLOT(sTaxAssignments()),	taxMenu,	"MaintainTaxAssignments ViewTaxAssignments", NULL, NULL, true, NULL },
    { "gl.taxRegistatrions",	tr("Tax &Registrations..."),	SLOT(sTaxRegistrations()),	taxMenu,	"MaintainChartOfAccounts",                   NULL, NULL, true, NULL },
    { "separator",		NULL,			        NULL,			        taxMenu,	"true",	                                     NULL, NULL, true, NULL },
 
    // Accounting | Tax | Reports
    { "menu",			tr("&Reports"),	                (char*)taxReportsMenu,		taxMenu,	"true",			    NULL, NULL, true, NULL },
    { "gl.dspTaxHistory",	tr("&Tax History..."),           SLOT(sDspTaxHistory()),        taxReportsMenu, "ViewTaxReconciliations",   NULL, NULL, true, NULL },
     
    { "separator",		  NULL,					NULL,					mainMenu,		"true",					       NULL, NULL, true, NULL },

    // Accounting | Utilities
    { "menu",				tr("&Utilities"),			(char*)utilitiesMenu,		mainMenu,	"true",	NULL, NULL, true, NULL },
    { "gl.forwardUpdateAccounts",	tr("&Forward Update Accounts..."),	SLOT(sForwardUpdateAccounts()),	utilitiesMenu,	"ViewTrialBalances",	NULL, NULL, _metrics->boolean("ManualForwardUpdate"), NULL },
    { "gl.duplicateAccountNumbers",      tr("&Duplicate Account Numbers..."),  SLOT(sDuplicateAccountNumbers()), utilitiesMenu,  "MaintainChartOfAccounts", NULL, NULL, true, NULL },
    { "separator",		  NULL,					NULL,					utilitiesMenu,		"true",					       NULL, NULL, true, NULL },
    { "so.purgeInvoices", tr("Purge &Invoices..."), SLOT(sPurgeInvoices()), utilitiesMenu, "PurgeInvoices", NULL, NULL, true , NULL },
    { "ar.updateLateCustCreditStatus", tr("&Update Late Customer Credit Status..."), SLOT(sUpdateLateCustCreditStatus()), utilitiesMenu, "UpdateCustomerCreditStatus", NULL, NULL, _metrics->boolean("AutoCreditWarnLateCustomers"), NULL },
    { "ar.createRecurringInvoices", tr("&Create Recurring Invoices..."), SLOT(sCreateRecurringInvoices()), utilitiesMenu, "MaintainMiscInvoices", NULL, NULL, true, NULL },
    { "separator",		  NULL,					NULL,					utilitiesMenu,		"true",					       NULL, NULL, _metrics->boolean("MultiCompanyFinancialConsolidation"), NULL },
    { "gl.syncCompanies",           tr("&Synchronize Companies"),        SLOT(sSyncCompanies()),           utilitiesMenu, "SynchronizeCompanies", NULL, NULL, _metrics->boolean("MultiCompanyFinancialConsolidation"), NULL },

    { "gl.setup",	tr("&Setup..."),	SLOT(sSetup()),	mainMenu,	NULL,	NULL,	NULL,	true,	NULL},

  };

  addActionsToMenu(acts, sizeof(acts) / sizeof(acts[0]));
  
  parent->populateCustomMenu(mainMenu, "Accounting");
  QAction * m = parent->menuBar()->addMenu(mainMenu);
  if(m)
    m->setText(tr("Accountin&g"));
}

void menuAccounting::addActionsToMenu(actionProperties acts[], unsigned int numElems)
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
    else if ((acts[i].toolBar != NULL) && (acts[i].toolBar != NULL))
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
                  acts[i].actionTitle);
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

//  Purchase Orders
void menuAccounting::sEnterPurchaseOrder()
{
  ParameterList params;
  params.append("mode", "new");

  purchaseOrder *newdlg = new purchaseOrder();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void menuAccounting::sUnpostedPurchaseOrders()
{
  omfgThis->handleNewWindow(new unpostedPurchaseOrders());
}

void menuAccounting::sPrintPurchaseOrder()
{
  printPurchaseOrder(parent, "", TRUE).exec();
}

void menuAccounting::sPostPurchaseOrder()
{
  postPurchaseOrder(parent, "", TRUE).exec();
}

//  Vouchers
void menuAccounting::sDspUninvoicedReceipts()
{
  omfgThis->handleNewWindow(new dspUninvoicedReceivings());
}

void menuAccounting::sEnterVoucher()
{
  ParameterList params;
  params.append("mode", "new");

  voucher *newdlg = new voucher();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void menuAccounting::sEnterMiscVoucher()
{
  ParameterList params;
  params.append("mode", "new");

  miscVoucher *newdlg = new miscVoucher();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void menuAccounting::sUnpostedVouchers()
{
  omfgThis->handleNewWindow(new openVouchers());
}

void menuAccounting::sVoucheringEditList()
{
  omfgThis->handleNewWindow(new voucheringEditList());
}

void menuAccounting::sPostVouchers()
{
  postVouchers(parent, "", TRUE).exec();
}


//  Payments
void menuAccounting::sSelectPayments()
{
  omfgThis->handleNewWindow(new selectPayments());
}

void menuAccounting::sSelectedPayments()
{
  omfgThis->handleNewWindow(new selectedPayments());
}

void menuAccounting::sCreateMiscCheck()
{
  ParameterList params;
  params.append("new");

  miscCheck *newdlg = new miscCheck();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void menuAccounting::sPrepareCheckRun()
{
  prepareCheckRun(parent, "", TRUE).exec();
}

void menuAccounting::sViewCheckRun()
{
  omfgThis->handleNewWindow(new viewCheckRun());
}

void menuAccounting::sPrintCheck()
{
  omfgThis->handleNewWindow(new printCheck(0, "printCheckFromAccountingMenu"));
}

void menuAccounting::sPrintCheckRun()
{
  printChecks *newdlg = new printChecks(parent, "", false);
  newdlg->setWindowModality(Qt::WindowModal);
  newdlg->exec();
}

void menuAccounting::sVoidCheckRun()
{
  voidChecks newdlg(parent, "", TRUE);
  newdlg.exec();
}

void menuAccounting::sPostCheck()
{
  postCheck(parent, "", TRUE).exec();
}

void menuAccounting::sPostChecks()
{
  postChecks(parent, "", TRUE).exec();
}

void menuAccounting::sApWorkBench()
{
  omfgThis->handleNewWindow(new apWorkBench());
}

//  Memos
void menuAccounting::sEnterMiscApCreditMemo()
{
  ParameterList params;
  params.append("mode", "new");
  params.append("docType", "creditMemo");

  apOpenItem newdlg(parent, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void menuAccounting::sUnappliedApCreditMemos()
{
  omfgThis->handleNewWindow(new unappliedAPCreditMemos());
}

void menuAccounting::sEnterMiscApDebitMemo()
{
  ParameterList params;
  params.append("mode", "new");
  params.append("docType", "debitMemo");

  apOpenItem newdlg(parent, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}


// AP Displays
void menuAccounting::sDspVendorHistory()
{
  omfgThis->handleNewWindow(new dspVendorAPHistory());
}

void menuAccounting::sDspCheckRegister()
{
  omfgThis->handleNewWindow(new dspCheckRegister());
}

void menuAccounting::sDspVoucherRegister()
{
  omfgThis->handleNewWindow(new dspVoucherRegister());
}

void menuAccounting::sDspAPApplications()
{
  omfgThis->handleNewWindow(new dspAPApplications());
}

void menuAccounting::sDspAPOpenItemsByVendor()
{
  omfgThis->handleNewWindow(new dspAPOpenItemsByVendor());
}

void menuAccounting::sDspTimePhasedOpenAPItems()
{
  omfgThis->handleNewWindow(new dspTimePhasedOpenAPItems());
}

void menuAccounting::sCreateInvoice()
{
  invoice::newInvoice(-1);
}

void menuAccounting::sRecurringInvoices()
{
  omfgThis->handleNewWindow(new listRecurringInvoices());
}

void menuAccounting::sUnpostedInvoices()
{
  omfgThis->handleNewWindow(new unpostedInvoices());
}

void menuAccounting::sReprintInvoices()
{
  reprintInvoices(parent, "", TRUE).exec();
}

void menuAccounting::sPrintInvoices()
{
  printInvoices(parent, "", TRUE).exec();
}

void menuAccounting::sPostInvoices()
{
  postInvoices(parent, "", TRUE).exec();
}

void menuAccounting::sPurgeInvoices()
{
  purgeInvoices(parent, "", TRUE).exec();
}



//  Cash Receipts
void menuAccounting::sEnterCashReceipt()
{
  ParameterList params;
  params.append("mode", "new");

  cashReceipt *newdlg = new cashReceipt();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void menuAccounting::sCashReceiptEditList()
{
  omfgThis->handleNewWindow(new cashReceiptsEditList());
}

void menuAccounting::sPostCashReceipts()
{
  postCashReceipts(parent, "", TRUE).exec();
}


//  Memos
void menuAccounting::sEnterMiscArCreditMemo()
{
  ParameterList params;
  params.append("mode", "new");
  params.append("docType", "creditMemo");

  arOpenItem newdlg(parent, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void menuAccounting::sUnappliedArCreditMemos()
{
  omfgThis->handleNewWindow(new unappliedARCreditMemos());
}

void menuAccounting::sEnterMiscArDebitMemo()
{
  ParameterList params;
  params.append("mode", "new");
  params.append("docType", "debitMemo");

  arOpenItem newdlg(parent, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

//  Displays
void menuAccounting::sArWorkBench()
{
  omfgThis->handleNewWindow(new arWorkBench());
}

void menuAccounting::sDspCustomerHistory()
{
  omfgThis->handleNewWindow(new dspCustomerARHistory());
}

void menuAccounting::sDspCashReceipts()
{
  omfgThis->handleNewWindow(new dspCashReceipts());
}

void menuAccounting::sDspARApplications()
{
  omfgThis->handleNewWindow(new dspARApplications());
}

void menuAccounting::sDspInvoiceInformation()
{
  omfgThis->handleNewWindow(new dspInvoiceInformation());
}

void menuAccounting::sDspInvoiceRegister()
{
  omfgThis->handleNewWindow(new dspInvoiceRegister());
}

void menuAccounting::sDspDepositsRegister()
{
  omfgThis->handleNewWindow(new dspDepositsRegister());
}

void menuAccounting::sRptSalesJournal()
{
  ParameterList params;
  params.append("type", SalesJournal);

  printJournal newdlg(parent, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void menuAccounting::sRptCreditMemoJournal()
{
  ParameterList params;
  params.append("type", CreditMemoJournal);

  printJournal newdlg(parent, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void menuAccounting::sRptPayablesJournal()
{
  ParameterList params;
  params.append("type", PayablesJournal);

  printJournal newdlg(parent, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void menuAccounting::sRptCheckJournal()
{
  ParameterList params;
  params.append("type", CheckJournal);

  printJournal newdlg(parent, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void menuAccounting::sDspAROpenItems()
{
  omfgThis->handleNewWindow(new dspAROpenItems());
}

void menuAccounting::sDspTimePhasedOpenItems()
{
  omfgThis->handleNewWindow(new dspTimePhasedOpenARItems());
}


// General Ledger
void menuAccounting::sEnterStandardJournal()
{
  ParameterList params;
  params.append("mode", "new");

  standardJournal newdlg(parent, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void menuAccounting::sStandardJournals()
{
  omfgThis->handleNewWindow(new standardJournals());
}

void menuAccounting::sStandardJournalGroups()
{
  omfgThis->handleNewWindow(new standardJournalGroups());
}

void menuAccounting::sPostStandardJournal()
{
  postStandardJournal(parent, "", TRUE).exec();
}

void menuAccounting::sPostStandardJournalGroup()
{
  postStandardJournalGroup(parent, "", TRUE).exec();
}

void menuAccounting::sPostJournals()
{
  omfgThis->handleNewWindow(new postJournals());
}

void menuAccounting::sSimpleEntry()
{
  ParameterList params;
  params.append("mode", "new");

  glTransaction newdlg(parent, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void menuAccounting::sSeriesEntry()
{
  ParameterList params;
  params.append("mode", "new");

  glSeries newdlg(parent, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void menuAccounting::sUnpostedEntries()
{
  omfgThis->handleNewWindow(new unpostedGlSeries());
}

//  Financial Layouts
void menuAccounting::sFinancialReports()
{
  omfgThis->handleNewWindow(new financialLayouts());
}

void menuAccounting::sViewFinancialReport()
{
  omfgThis->handleNewWindow(new dspFinancialReport());
}

void menuAccounting::sNewFinancialReport()
{
  ParameterList params;
  params.append("mode", "new");

  financialLayout newdlg(parent, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}


//  Displays
void menuAccounting::sDspGLTransactions()
{
  omfgThis->handleNewWindow(new dspGLTransactions());
}

void menuAccounting::sDspSummarizedGLTransactions()
{
  omfgThis->handleNewWindow(new dspSummarizedGLTransactions());
}

void menuAccounting::sDspGLSeries()
{
  omfgThis->handleNewWindow(new dspGLSeries());
}

void menuAccounting::sDspJournals()
{
  omfgThis->handleNewWindow(new dspJournals());
}

void menuAccounting::sDspStandardJournalHistory()
{
  omfgThis->handleNewWindow(new dspStandardJournalHistory());
}

void menuAccounting::sDspTrialBalances()
{
  omfgThis->handleNewWindow(new dspTrialBalances());
}

//  Master Information
void menuAccounting::sCompanies()
{
  omfgThis->handleNewWindow(new companies());
}

void menuAccounting::sProfitCenters()
{
  omfgThis->handleNewWindow(new profitCenters());
}

void menuAccounting::sSubaccounts()
{
  omfgThis->handleNewWindow(new subaccounts());
}

void menuAccounting::sAccountNumbers()
{
  omfgThis->handleNewWindow(new accountNumbers());
}

void menuAccounting::sDuplicateAccountNumbers()
{
  omfgThis->handleNewWindow(new duplicateAccountNumbers());
}

void menuAccounting::sSubAccntTypes()
{
  omfgThis->handleNewWindow(new subAccntTypes());
}

void menuAccounting::sAccountingPeriods()
{
  omfgThis->handleNewWindow(new accountingPeriods());
}

void menuAccounting::sAccountingYearPeriods()
{
  omfgThis->handleNewWindow(new accountingYearPeriods());
}

void menuAccounting::sReconcileBankaccount()
{
  omfgThis->handleNewWindow(new reconcileBankaccount());
}

void menuAccounting::sEnterAdjustment()
{
  ParameterList params;
  params.append("mode", "new");

  bankAdjustment *newdlg = new bankAdjustment();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void menuAccounting::sAdjustmentEditList()
{
  omfgThis->handleNewWindow(new bankAdjustmentEditList());
}

void menuAccounting::sTaxAuthorities()
{
  omfgThis->handleNewWindow(new taxAuthorities());
}

void menuAccounting::sTaxZones()
{
  omfgThis->handleNewWindow(new taxZones());
}

void menuAccounting::sTaxClasses()
{
  omfgThis->handleNewWindow(new taxClasses());
}

void menuAccounting::sTaxCodes()
{
  omfgThis->handleNewWindow(new taxCodes());
}

void menuAccounting::sTaxTypes()
{
  omfgThis->handleNewWindow(new taxTypes());
}

void menuAccounting::sTaxAssignments()
{
  omfgThis->handleNewWindow(new taxAssignments());
}

void menuAccounting::sTaxRegistrations()
{
  omfgThis->handleNewWindow(new taxRegistrations());
}

void menuAccounting::sDspTaxHistory()
{
  omfgThis->handleNewWindow(new dspTaxHistory());
}

void menuAccounting::sDspBankrecHistory()
{
  omfgThis->handleNewWindow(new dspBankrecHistory());
}

void menuAccounting::sDspSummarizedBankrecHistory()
{
  omfgThis->handleNewWindow(new dspSummarizedBankrecHistory());
}

void menuAccounting::sBudgets()
{
  omfgThis->handleNewWindow(new budgets());
}

void menuAccounting::sMaintainBudget()
{
  ParameterList params;
  params.append("mode", "new");

  maintainBudget *newdlg = new maintainBudget();
  newdlg->set(params);

  omfgThis->handleNewWindow(newdlg);
}

void menuAccounting::sForwardUpdateAccounts()
{
  forwardUpdateAccounts(parent, "", TRUE).exec();
}

void menuAccounting::sVendors()
{
  omfgThis->handleNewWindow(new vendors());
}

void menuAccounting::sCheckFormats()
{
  omfgThis->handleNewWindow(new checkFormats());
}

void menuAccounting::sPrintStatementByCustomer()
{
  printStatementByCustomer(parent, "", TRUE).exec();
}

void menuAccounting::sPrintStatementsByCustomerType()
{
  printStatementsByCustomerType(parent, "", TRUE).exec();
}

void menuAccounting::sCustomers()
{
  omfgThis->handleNewWindow(new customers());
}

void menuAccounting::sUpdateLateCustCreditStatus()
{
  updateLateCustCreditStatus newdlg(parent, "", TRUE);
  newdlg.exec();
}

void menuAccounting::sCreateRecurringInvoices()
{
  createRecurringInvoices newdlg(parent, "", TRUE);
  newdlg.exec();
}

void menuAccounting::sSyncCompanies()
{
  omfgThis->handleNewWindow(new syncCompanies());
}


void menuAccounting::sSetup()
{
  ParameterList params;
  params.append("module", Xt::AccountingModule);

  setup newdlg(parent);
  newdlg.set(params);
  newdlg.exec();
}
