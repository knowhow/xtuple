/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspAPApplications.h"

#include <QAction>
#include <QMenu>
#include <QMessageBox>
#include <QSqlError>

#include "apOpenItem.h"
#include "check.h"
#include "voucher.h"

dspAPApplications::dspAPApplications(QWidget* parent, const char*, Qt::WFlags fl)
  : display(parent, "dspAPApplications", fl)
{
  setupUi(optionsWidget());
  setWindowTitle(tr("A/P Applications"));
  setListLabel(tr("Accounts Payable Applications"));
  setReportName("APApplications");
  setMetaSQLOptions("apApplications", "detail");

  _dates->setStartNull(tr("Earliest"), omfgThis->startOfTime(), true);
  _dates->setEndNull(tr("Latest"),     omfgThis->endOfTime(),   true);
    
  list()->addColumn(tr("Vend. #"),    _orderColumn, Qt::AlignLeft,  true, "vend_number");
  list()->addColumn(tr("Vendor"),               -1, Qt::AlignLeft,  true, "vend_name");
  list()->addColumn(tr("Post Date"),   _dateColumn, Qt::AlignCenter,true, "apapply_postdate");
  list()->addColumn(tr("Source"),      _itemColumn, Qt::AlignCenter,true, "apapply_source_doctype");
  list()->addColumn(tr("Doc #"),      _orderColumn, Qt::AlignRight, true, "apapply_source_docnumber");
  list()->addColumn(tr("Apply-To"),    _itemColumn, Qt::AlignCenter,true, "apapply_target_doctype");
  list()->addColumn(tr("Doc #"),      _orderColumn, Qt::AlignRight, true, "apapply_target_docnumber");
  list()->addColumn(tr("Amount"),     _moneyColumn, Qt::AlignRight, true, "apapply_amount");
  list()->addColumn(tr("Currency"),_currencyColumn, Qt::AlignLeft,  true, "currAbbr");
  list()->addColumn(tr("Amount (in %1)").arg(CurrDisplay::baseCurrAbbr()),_moneyColumn, Qt::AlignRight, true, "base_applied");

  _vendorgroup->setFocus();
}

void dspAPApplications::languageChange()
{
  display::languageChange();
  retranslateUi(this);
}

void dspAPApplications::sViewCheck()
{
  int checkid = list()->currentItem()->id("apapply_source_docnumber");
  if ((checkid == -1) || (checkid == 0))
  {
    XSqlQuery countq;
    countq.prepare("SELECT COUNT(*) AS count "
                 "FROM checkhead "
                 "JOIN checkitem ON (checkhead_id=checkitem_checkhead_id) "
                 "WHERE ((checkhead_number=:number)"
                 "   AND (checkitem_amount=:amount));");
    countq.bindValue(":number", list()->currentItem()->text("apapply_source_docnumber"));
    countq.bindValue(":amount", list()->currentItem()->rawValue("apapply_amount"));
    countq.exec();
    if (countq.first())
    {
      if (countq.value("count").toInt() > 1)
      {
        QMessageBox::warning(this, tr("Check Look-Up Failed"),
                             tr("Found multiple checks with this check number."));
        return;
      }
      else if (countq.value("count").toInt() < 1)
      {
        QMessageBox::warning(this, tr("Check Look-Up Failed"),
                             tr("Could not find the record for this check."));
        return;
      }
    }
    else if (countq.lastError().type() != QSqlError::NoError)
    {
      systemError(this, countq.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }

    XSqlQuery chkq;
    chkq.prepare("SELECT checkhead_id "
                 "FROM checkhead "
                 "JOIN checkitem ON (checkhead_id=checkitem_checkhead_id) "
                 "WHERE ((checkhead_number=:number)"
                 "   AND (checkitem_amount=:amount));");
    chkq.bindValue(":number", list()->currentItem()->text("apapply_source_docnumber"));
    chkq.bindValue(":amount", list()->currentItem()->rawValue("apapply_amount"));
    chkq.exec();
    if (chkq.first())
      checkid = chkq.value("checkhead_id").toInt();
    else if (chkq.lastError().type() != QSqlError::NoError)
    {
      systemError(this, chkq.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }

  ParameterList params;
  params.append("checkhead_id", checkid);
  check *newdlg = new check(this, "check");
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspAPApplications::sViewCreditMemo()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("apopen_id", list()->id("apapply_source_docnumber"));
  params.append("docType",   "creditMemo");
  apOpenItem newdlg(this, "", true);
  newdlg.set(params);
  newdlg.exec();
}

void dspAPApplications::sViewDebitMemo()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("apopen_id", list()->id("apapply_target_docnumber"));
  params.append("docType", "debitMemo");
  apOpenItem newdlg(this, "", true);
  newdlg.set(params);
  newdlg.exec();
}

void dspAPApplications::sViewVoucher()
{
  ParameterList params;
  params.append("mode",      "view");
  params.append("vohead_id", list()->id("apapply_target_docnumber"));
  voucher *newdlg = new voucher(this, "voucher");
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspAPApplications::sPopulateMenu(QMenu* pMenu, QTreeWidgetItem*, int)
{
  QAction *menuItem;

  if (list()->currentItem()->rawValue("apapply_source_doctype") == "C")
  {
    menuItem = pMenu->addAction(tr("View Source Credit Memo..."), this, SLOT(sViewCreditMemo()));
    menuItem->setEnabled(_privileges->check("MaintainAPMemos") ||
                         _privileges->check("ViewAPMemos"));
  }
  else if (list()->currentItem()->rawValue("apapply_source_doctype") == "K")
  {
    menuItem = pMenu->addAction(tr("View Source Check..."), this, SLOT(sViewCheck()));
    menuItem->setEnabled(_privileges->check("MaintainPayments"));
  }

  if (list()->currentItem()->rawValue("apapply_target_doctype") == "D")
  {
    menuItem = pMenu->addAction(tr("View Apply-To Debit Memo..."), this, SLOT(sViewDebitMemo()));
    menuItem->setEnabled(_privileges->check("MaintainAPMemos") ||
                         _privileges->check("ViewAPMemos"));
  }
  else if (list()->currentItem()->rawValue("apapply_target_doctype") == "V")
  {
    menuItem = pMenu->addAction(tr("View Apply-To Voucher..."), this, SLOT(sViewVoucher()));
    menuItem->setEnabled(_privileges->check("MaintainVouchers") ||
                         _privileges->check("ViewVouchers"));
  }
}

bool dspAPApplications::setParams(ParameterList & params)
{
  if (! _vendorgroup->isValid())
  {
    QMessageBox::warning( this, tr("Select Vendor"),
                          tr("You must select the Vendor(s) whose A/R Applications you wish to view.") );
    _vendorgroup->setFocus();
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

  if ( !_showChecks->isChecked() && !_showCreditMemos->isChecked())
  {
    if (windowFlags() & (Qt::Window | Qt::Dialog))
      QMessageBox::critical( this, tr("Select Document Type"),
                             tr("You must indicate which Document Type(s) you wish to view.") );
    _showChecks->setFocus();
    return false;
  }
  
  if (_showChecks->isChecked() && _showCreditMemos->isChecked())
    params.append("doctypeList", "'C', 'K'");
  else if (_showChecks->isChecked())
    params.append("doctypeList", "'K'");
  else if (_showCreditMemos->isChecked())
    params.append("doctypeList", "'C'");
  if (_showChecks->isChecked())
    params.append("showChecks");
  if (_showCreditMemos->isChecked())
    params.append("showCreditMemos");

  _dates->appendValue(params);
  params.append("creditMemo", tr("Credit Memo"));
  params.append("debitMemo",  tr("Debit Memo"));
  params.append("check",      tr("Check"));
  params.append("voucher",    tr("Voucher"));

  _vendorgroup->appendValue(params);

  return true;
}
