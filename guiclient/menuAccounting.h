/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef menuAccounting_h
#define menuAccounting_h

#include <QObject>
#include <QPixmap>

class QToolBar;
class QMenu;
class GUIClient;

class menuAccounting : public QObject
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
    menuAccounting(GUIClient *);

  public slots:
    // AP
    void sEnterPurchaseOrder();
    void sUnpostedPurchaseOrders();
    void sPrintPurchaseOrder();
    void sPostPurchaseOrder();

    void sDspUninvoicedReceipts();
    void sEnterVoucher();
    void sEnterMiscVoucher();
    void sUnpostedVouchers();
    void sVoucheringEditList();
    void sPostVouchers();

    void sSelectPayments();
    void sSelectedPayments();
    void sPrepareCheckRun();
    void sCreateMiscCheck();
    void sViewCheckRun();
    void sPrintCheck();
    void sPrintCheckRun();
    void sVoidCheckRun();
    void sPostCheck();
    void sPostChecks();

    void sApWorkBench();

    void sEnterMiscApCreditMemo();
    void sUnappliedApCreditMemos();
    void sEnterMiscApDebitMemo();

    void sDspVendorHistory();
    void sDspCheckRegister();
    void sDspVoucherRegister();
    void sDspAPApplications();
    void sDspAPOpenItemsByVendor();
    void sDspTimePhasedOpenAPItems();
    
    // AR
    void sCreateInvoice();
    void sRecurringInvoices();
    void sUnpostedInvoices();
    void sPrintInvoices();
    void sReprintInvoices();
    void sPostInvoices();
    void sPurgeInvoices();

    void sEnterCashReceipt();
    void sCashReceiptEditList();
    void sPostCashReceipts();

    void sEnterMiscArCreditMemo();
    void sUnappliedArCreditMemos();
    void sEnterMiscArDebitMemo();

    void sArWorkBench();
    void sDspCustomerHistory();
    void sDspCashReceipts();
    void sDspARApplications();
    void sDspInvoiceInformation();
    void sDspAROpenItems();
    void sDspTimePhasedOpenItems();
    void sDspInvoiceRegister();
    void sDspDepositsRegister();
    void sRptSalesJournal();
    void sRptCreditMemoJournal();
    void sRptPayablesJournal();
    void sRptCheckJournal();

    // GL
    void sSimpleEntry();
    void sSeriesEntry();
    void sUnpostedEntries();

    void sEnterStandardJournal();
    void sStandardJournals();
    void sStandardJournalGroups();
    void sPostStandardJournal();
    void sPostStandardJournalGroup();
    void sPostJournals();

    void sFinancialReports();
    void sViewFinancialReport();
    void sNewFinancialReport();

    void sDspGLTransactions();
    void sDspSummarizedGLTransactions();
    void sDspGLSeries();
    void sDspStandardJournalHistory();
    void sDspTrialBalances();
    void sDspJournals();

    void sCompanies();
    void sProfitCenters();
    void sSubaccounts();
    void sAccountNumbers();
    void sDuplicateAccountNumbers();
    void sSubAccntTypes();
    void sAccountingPeriods();
    void sAccountingYearPeriods();
    void sTaxCodes();
    void sTaxTypes();
    void sTaxAuthorities();
    void sTaxZones();
    void sTaxClasses();
    void sTaxAssignments();
    void sTaxRegistrations();
    void sDspTaxHistory();

    void sReconcileBankaccount();
    void sEnterAdjustment();
    void sAdjustmentEditList();
    void sDspBankrecHistory();
    void sDspSummarizedBankrecHistory();

    void sBudgets();
    void sMaintainBudget();

    void sForwardUpdateAccounts();

    void sVendors();
    void sCheckFormats();
    
    void sPrintStatementByCustomer();
    void sPrintStatementsByCustomerType();

    void sCustomers();

    void sUpdateLateCustCreditStatus();
    void sCreateRecurringInvoices();
    void sSyncCompanies();

    void sSetup();

  private:
    GUIClient *parent;

    QToolBar   *toolBar;
    QMenu    *mainMenu;
    QMenu    *apMenu;
    QMenu    *apPurchaseMenu;
    QMenu    *apVoucherMenu;
    QMenu    *apPaymentsMenu;
    QMenu    *apMemosMenu;
    QMenu    *apFormsMenu;
    QMenu    *apReportsMenu;
    QMenu    *arMenu;
    QMenu    *arInvoicesMenu;
    QMenu    *arMemosMenu;
    QMenu    *arCashReceiptsMenu;
    QMenu    *arFormsMenu;
    QMenu    *arReportsMenu;
    QMenu    *glMenu;
    QMenu    *glReportsMenu;
    QMenu    *glEnterTransactionMenu;
    QMenu    *glStandardJournalsMenu;
    QMenu    *financialReportsMenu;
    QMenu    *calendarMenu;
    QMenu    *coaMenu;
    QMenu    *budgetMenu;
    QMenu    *taxMenu;
    QMenu    *taxReportsMenu;
    QMenu    *bankrecMenu;
    QMenu    *bankrecDisplaysMenu;
    QMenu    *bankrecReportsMenu;
    QMenu    *utilitiesMenu;
    
    void	addActionsToMenu(actionProperties [], unsigned int);
};

#endif
