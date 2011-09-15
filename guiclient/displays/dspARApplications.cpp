/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspARApplications.h"

#include <QAction>
#include <QMenu>
#include <QMessageBox>
#include <QSqlError>

#include <datecluster.h>

#include "arOpenItem.h"
#include "creditMemo.h"
#include "dspInvoiceInformation.h"

dspARApplications::dspARApplications(QWidget* parent, const char*, Qt::WFlags fl)
  : display(parent, "dspARApplications", fl)
{
  setupUi(optionsWidget());
  setWindowTitle(tr("A/R Applications"));
  setListLabel(tr("A/R Applications"));
  setReportName("ARApplications");
  setMetaSQLOptions("arApplications", "detail");

  _dates->setStartNull(tr("Earliest"), omfgThis->startOfTime(), true);
  _dates->setEndNull(tr("Latest"), omfgThis->endOfTime(), true);
    
  list()->addColumn(tr("Cust. #"),        _orderColumn, Qt::AlignCenter, true,  "cust_number" );
  list()->addColumn(tr("Customer"),                 -1, Qt::AlignLeft,   true,  "cust_name"   );
  list()->addColumn(tr("Post Date"),       _dateColumn, Qt::AlignCenter, true,  "arapply_postdate" );
  list()->addColumn(tr("Dist. Date"),      _dateColumn, Qt::AlignCenter, true,  "arapply_distdate" );
  list()->addColumn(tr("Source Doc Type"),          10, Qt::AlignCenter, true,  "arapply_source_doctype" );
  list()->addColumn(tr("Source"),	         _itemColumn, Qt::AlignCenter, true,  "doctype" );
  list()->addColumn(tr("Doc #"),          _orderColumn, Qt::AlignCenter, true,  "source" );
  list()->addColumn(tr("Apply-To Doc Type"),        10, Qt::AlignCenter, true,  "arapply_target_doctype" );
  list()->addColumn(tr("Apply-To"),        _itemColumn, Qt::AlignCenter, true,  "targetdoctype" );
  list()->addColumn(tr("Doc #"),          _orderColumn, Qt::AlignCenter, true,  "target" );
  list()->addColumn(tr("Amount"),         _moneyColumn, Qt::AlignRight,  true,  "arapply_applied"  );
  list()->addColumn(tr("Currency"),    _currencyColumn, Qt::AlignLeft,   true,  "currAbbr"   );
  list()->addColumn(tr("Base Amount"), _bigMoneyColumn, Qt::AlignRight,  true,  "base_applied"  );

  list()->hideColumn(4);
  list()->hideColumn(7);

  _allCustomers->setFocus();
}

void dspARApplications::languageChange()
{
  display::languageChange();
  retranslateUi(this);
}

void dspARApplications::sViewCreditMemo()
{
  ParameterList params;
  params.append("mode", "view");

  q.prepare("SELECT 1 AS type, cmhead_id AS id "
	    "FROM cmhead "
	    "WHERE (cmhead_number=:docnum) "
	    "UNION "
	    "SELECT 2 AS type, aropen_id AS id "
	    "FROM aropen "
	    "WHERE ((aropen_docnumber=:docnum)"
	    "  AND (aropen_doctype IN ('C', 'R')) "
	    ") ORDER BY type LIMIT 1;");
  q.bindValue(":docnum", list()->currentItem()->text(6));
  q.exec();
  if (q.first())
  {
    if (q.value("type").toInt() == 1)
    {
      params.append("cmhead_id", q.value("id"));
      creditMemo* newdlg = new creditMemo();
      newdlg->set(params);
      omfgThis->handleNewWindow(newdlg);
    }
    else if (q.value("type").toInt() == 2)
    {
      params.append("aropen_id", q.value("id"));
      params.append("docType", "creditMemo");
      arOpenItem newdlg(this, "", true);
      newdlg.set(params);
      newdlg.exec();
    }
  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
  else
  {
    QMessageBox::information(this, tr("Credit Memo Not Found"),
			     tr("<p>The Credit Memo #%1 could not be found.")
			     .arg(list()->currentItem()->text(6)));
    return;
  }
}

void dspARApplications::sViewDebitMemo()
{
  ParameterList params;

  params.append("mode", "view");
  q.prepare("SELECT aropen_id "
	    "FROM aropen "
	    "WHERE ((aropen_docnumber=:docnum) AND (aropen_doctype='D'));");
  q.bindValue(":docnum", list()->currentItem()->text(9));
  q.exec();
  if (q.first())
  {
    params.append("aropen_id", q.value("aropen_id"));
    params.append("docType", "debitMemo");
    arOpenItem newdlg(this, "", true);
    newdlg.set(params);
    newdlg.exec();
  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void dspARApplications::sViewInvoice()
{
  ParameterList params;

  params.append("mode", "view");
  params.append("invoiceNumber", list()->currentItem()->text(9));
  dspInvoiceInformation* newdlg = new dspInvoiceInformation();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspARApplications::sPopulateMenu(QMenu* pMenu, QTreeWidgetItem*, int)
{
  QAction *menuItem;

  if (list()->currentItem()->text(4) == "C")
  {
    menuItem = pMenu->addAction(tr("View Source Credit Memo..."), this, SLOT(sViewCreditMemo()));
    if (! _privileges->check("MaintainARMemos") &&
	! _privileges->check("ViewARMemos"))
      menuItem->setEnabled(false);
  }

  if (list()->currentItem()->text(7) == "D")
  {
    menuItem = pMenu->addAction(tr("View Apply-To Debit Memo..."), this, SLOT(sViewDebitMemo()));
    if (! _privileges->check("MaintainARMemos") &&
	! _privileges->check("ViewARMemos"))
      menuItem->setEnabled(false);
  }
  else if (list()->currentItem()->text(7) == "I")
  {
    menuItem = pMenu->addAction(tr("View Apply-To Invoice..."), this, SLOT(sViewInvoice()));
    if (! _privileges->check("MaintainMiscInvoices") &&
	! _privileges->check("ViewMiscInvoices"))
      menuItem->setEnabled(false);
  }
}

bool dspARApplications::setParams(ParameterList & params)
{
  if ( (_selectedCustomer->isChecked()) && (!_cust->isValid()) )
  {
    QMessageBox::warning( this, tr("Select Customer"),
                          tr("You must select a Customer whose A/R Applications you wish to view.") );
    _cust->setFocus();
    return false;
  }

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

  if ( (!_cashReceipts->isChecked()) && (!_creditMemos->isChecked()) )
  {
    QMessageBox::critical( this, tr("Select Document Type"),
                           tr("You must indicate which Document Type(s) you wish to view.") );
    _cashReceipts->setFocus();
    return false;
  }
  
  if (_cashReceipts->isChecked())
    params.append("includeCashReceipts");

  if (_creditMemos->isChecked())
    params.append("includeCreditMemos");

  _dates->appendValue(params);
  params.append("creditMemo", tr("C/M"));
  params.append("debitMemo", tr("D/M"));
  params.append("cashdeposit", tr("Cash Deposit"));
  params.append("invoice", tr("Invoice"));
  params.append("cash", tr("C/R"));
  params.append("check", tr("Check"));
  params.append("certifiedCheck", tr("Cert. Check"));
  params.append("masterCard", tr("M/C"));
  params.append("visa", tr("Visa"));
  params.append("americanExpress", tr("AmEx"));
  params.append("discoverCard", tr("Discover"));
  params.append("otherCreditCard", tr("Other C/C"));
  params.append("cash", tr("Cash"));
  params.append("wireTransfer", tr("Wire Trans."));
  params.append("other", tr("Other"));
  params.append("apcheck", tr("A/P Check"));

  if (_selectedCustomer->isChecked())
    params.append("cust_id", _cust->id());
  else if (_selectedCustomerType->isChecked())
    params.append("custtype_id", _customerTypes->id());
  else if (_customerTypePattern->isChecked())
    params.append("custtype_pattern", _customerType->text());

  return true; 
}
