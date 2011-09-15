/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspCashReceipts.h"

#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include <datecluster.h>
#include <openreports.h>
#include "arOpenItem.h"
#include "cashReceipt.h"
#include "storedProcErrorLookup.h"

dspCashReceipts::dspCashReceipts(QWidget* parent, const char*, Qt::WFlags fl)
  : display(parent, "dspCashReceipts", fl)
{
  setupUi(optionsWidget());
  setWindowTitle(tr("Cash Receipts"));
  setListLabel(tr("Cash Receipts"));
  setReportName("CashReceipts");
  setMetaSQLOptions("cashReceipts", "detail");
  setNewVisible(true);
  setUseAltId(true);

  connect(_applications, SIGNAL(toggled(bool)), list(), SLOT(clear()));

  _dates->setStartNull(tr("Earliest"), omfgThis->startOfTime(), TRUE);
  _dates->setStartDate(QDate().currentDate().addDays(-90));
  _dates->setEndNull(tr("Latest"), omfgThis->endOfTime(), TRUE);
  
  list()->addColumn(tr("Number"),      _orderColumn,    Qt::AlignLeft, true,  "cashrcpt_number" );
  list()->addColumn(tr("Source"),      _itemColumn,     Qt::AlignLeft,   true,  "source" );
  list()->addColumn(tr("Cust. #"),     _orderColumn,    Qt::AlignLeft, true,  "cust_number" );
  list()->addColumn(tr("Customer"),    -1,              Qt::AlignLeft,   true,  "cust_name"   );
  list()->addColumn(tr("Posted"),      _ynColumn,       Qt::AlignCenter, true,  "posted" );
  list()->addColumn(tr("Voided"),      _ynColumn,       Qt::AlignCenter, true,  "voided" );
  list()->addColumn(tr("Date"),        _dateColumn,     Qt::AlignCenter, true,  "postdate" );
  list()->addColumn(tr("Apply-To"),    -1,     Qt::AlignLeft, true,  "target" );
  list()->addColumn(tr("Amount"),      _moneyColumn, Qt::AlignRight,  true,  "applied"  );
  list()->addColumn(tr("Currency"),    _currencyColumn, Qt::AlignLeft,   true,  "currAbbr"   );
  list()->addColumn(tr("Base Amount"), _moneyColumn, Qt::AlignRight,  true,  "base_applied"  );
  
  newAction()->setEnabled(_privileges->check("MaintainCashReceipts"));
}

bool dspCashReceipts::setParams(ParameterList &pParams)
{
  if (!_dates->startDate().isValid())
  {
    QMessageBox::critical( this, tr("Enter Start Date"),
                           tr("You must enter a valid Start Date.") );
    _dates->setFocus();
    return false;
  }

  if (!_dates->endDate().isValid())
  {
    QMessageBox::critical( this, tr("Enter End Date"),
                           tr("You must enter a valid End Date.") );
    _dates->setFocus();
    return false;
  }

  _customerSelector->appendValue(pParams);
  _dates->appendValue(pParams);
  pParams.append("creditMemo", tr("Credit Memo"));
  pParams.append("debitMemo", tr("Debit Memo"));
  pParams.append("cashdeposit", tr("Customer Deposit"));
  pParams.append("invoice", tr("Invoice"));
  pParams.append("cash", tr("Cash"));
  pParams.append("check", tr("Check"));
  pParams.append("certifiedCheck", tr("Cert. Check"));
  pParams.append("masterCard", tr("Master Card"));
  pParams.append("visa", tr("Visa"));
  pParams.append("americanExpress", tr("AmEx"));
  pParams.append("discoverCard", tr("Discover"));
  pParams.append("otherCreditCard", tr("Other C/C"));
  pParams.append("wireTransfer", tr("Wire Trans."));
  pParams.append("other", tr("Other"));
  pParams.append("unapplied", tr("Customer Deposit"));
  pParams.append("unposted", tr("Unposted"));
  pParams.append("voided", tr("Voided"));
    
  if (_applications->isChecked())
  {
    list()->hideColumn("cashrcpt_number");
    pParams.append("LegacyDisplayMode");
  }
  else
    list()->showColumn("cashrcpt_number");
  pParams.append("includeFormatted");
  return true;
}

void dspCashReceipts::sPopulateMenu(QMenu * pMenu, QTreeWidgetItem *, int)
{
  QAction *menuItem = 0;
  
  if (list()->id() > -1)
  {
    // Cash Receipt              
    if (!list()->currentItem()->rawValue("posted").toBool() && !list()->currentItem()->rawValue("voided").toBool())
    {
      menuItem = pMenu->addAction(tr("Edit Cash Receipt..."), this, SLOT(sEditCashrcpt()));
      menuItem->setEnabled(_privileges->check("MaintainCashReceipts"));
    }

    menuItem = pMenu->addAction(tr("View Cash Receipt..."), this, SLOT(sViewCashrcpt()));
    menuItem->setEnabled(_privileges->check("ViewCashReceipts") || _privileges->check("MaintainCashReceipts"));

    if (!list()->currentItem()->rawValue("voided").toBool())
    {      
      if (!list()->currentItem()->rawValue("posted").toBool())
      {   
        menuItem = pMenu->addAction(tr("Post Cash Receipt"), this, SLOT(sPostCashrcpt()));
        menuItem->setEnabled(_privileges->check("PostCashReceipts"));
      }
      else if (!list()->altId())
      {
        menuItem = pMenu->addAction(tr("Void Posted Cash Receipt"), this, SLOT(sReversePosted()));
        menuItem->setEnabled(_privileges->check("ReversePostedCashReceipt"));
      }
    }

    // Open Item
    if (list()->currentItem()->id("target") > -1 )
    {
      pMenu->addSeparator();
      menuItem = pMenu->addAction(tr("Edit Receivable Item..."), this, SLOT(sEditAropen()));
      menuItem->setEnabled(_privileges->check("EditAROpenItem"));
      menuItem = pMenu->addAction(tr("View Receivable Item..."), this, SLOT(sViewAropen()));
      menuItem->setEnabled(_privileges->check("ViewAROpenItems") || _privileges->check("EditAROpenItem"));
    }
  }
}

void dspCashReceipts::sEditAropen()
{   
  ParameterList params;
  params.append("mode", "edit");
  params.append("aropen_id", list()->currentItem()->id("target"));
  arOpenItem newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void dspCashReceipts::sViewAropen()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("aropen_id", list()->currentItem()->id("target"));
  arOpenItem newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void dspCashReceipts::sNew()
{
  sNewCashrcpt();
}

void dspCashReceipts::sNewCashrcpt()
{
  ParameterList params;
  params.append("mode", "new");
  if (_customerSelector->isSelectedCust())
    params.append("cust_id", _customerSelector->custId());

  cashReceipt *newdlg = new cashReceipt(this);
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspCashReceipts::sEditCashrcpt()
{    
  ParameterList params;
  params.append("mode", "edit");
  params.append("cashrcpt_id", list()->currentItem()->id("source"));

  cashReceipt *newdlg = new cashReceipt(this);
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspCashReceipts::sViewCashrcpt()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("cashrcpt_id", list()->currentItem()->id("source"));

  cashReceipt *newdlg = new cashReceipt(this);
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspCashReceipts::sPostCashrcpt()
{
  int journalNumber = -1;

  XSqlQuery tx;
  tx.exec("BEGIN;");
  q.exec("SELECT fetchJournalNumber('C/R') AS journalnumber;");
  if (q.first())
    journalNumber = q.value("journalnumber").toInt();
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  q.prepare("SELECT postCashReceipt(:cashrcpt_id, :journalNumber) AS result;");
  q.bindValue(":cashrcpt_id", list()->currentItem()->id("source"));
  q.bindValue(":journalNumber", journalNumber);
  q.exec();
  if (q.first())
  {
    int result = q.value("result").toInt();
    if (result < 0)
    {
      systemError(this, storedProcErrorLookup("postCashReceipt", result),
                  __FILE__, __LINE__);
      tx.exec("ROLLBACK;");
      return;
    }
  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    tx.exec("ROLLBACK;");
    return;
  }
    
  tx.exec("COMMIT;");
  omfgThis->sCashReceiptsUpdated(list()->currentItem()->id("source"), TRUE);
  sFillList();
}

void dspCashReceipts::sReversePosted()
{
  if (QMessageBox::warning(this, tr("Reverse Entire Posting?"),
                                  tr("<p>This will reverse all applications related "
                                     "to this cash receipt.  Are you sure you want "
                                     "to do this?"),
                                     QMessageBox::Yes | QMessageBox::Default,
                                     QMessageBox::No  | QMessageBox::Escape) == QMessageBox::Yes)
  {                     
    q.prepare("SELECT reverseCashReceipt(:cashrcpt_id, fetchJournalNumber('C/R')) AS result;");
    q.bindValue(":cashrcpt_id", list()->currentItem()->id("source"));
    q.exec();
    if (q.first())
    {
      int result = q.value("result").toInt();
      if (result < 0)
      {
        systemError(this, storedProcErrorLookup("reverseCashReceipt", result),
                         __FILE__, __LINE__);
        return;
      }
    }
    else if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
    sFillList();
  }
}
