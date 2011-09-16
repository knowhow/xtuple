/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspInvoiceRegister.h"

#include <QAction>
#include <QMenu>
#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include "arOpenItem.h"
#include "creditMemo.h"
#include "dspInvoiceInformation.h"

dspInvoiceRegister::dspInvoiceRegister(QWidget* parent, const char*, Qt::WFlags fl)
  : display(parent, "dspInvoiceRegister", fl)
{
  setupUi(optionsWidget());
  setWindowTitle(tr("Invoice Register"));
  setListLabel(tr("Transactions"));
  setReportName("InvoiceRegister");
  setMetaSQLOptions("invoiceRegister", "detail");
  setUseAltId(true);

  list()->addColumn(tr("Date"),     _dateColumn, Qt::AlignCenter,true, "transdate");
  list()->addColumn(tr("Source"),  _orderColumn, Qt::AlignCenter,true, "gltrans_source");
  list()->addColumn(tr("Doc Type"),_orderColumn, Qt::AlignLeft,  true, "doctype");
  list()->addColumn(tr("Doc. #"),  _orderColumn, Qt::AlignCenter,true, "gltrans_docnumber");
  list()->addColumn(tr("Reference"),         -1, Qt::AlignLeft,  true, "notes");
  list()->addColumn(tr("Account"),  _itemColumn, Qt::AlignLeft,  true, "account");
  list()->addColumn(tr("Debit"),   _moneyColumn, Qt::AlignRight, true, "debit");
  list()->addColumn(tr("Credit"),  _moneyColumn, Qt::AlignRight, true, "credit");
  list()->setPopulateLinear();
}

void dspInvoiceRegister::languageChange()
{
  display::languageChange();
  retranslateUi(this);
}

enum SetResponse dspInvoiceRegister::set(const ParameterList &pParams)
{
  XWidget::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("accnt_id", &valid);
  if (valid)
  {
    _selectedAccount->setChecked(true);
    _account->setId(param.toInt());
  }

  param = pParams.value("startDate", &valid);
  if (valid)
    _dates->setStartDate(param.toDate());

  param = pParams.value("endDate", &valid);
  if (valid)
    _dates->setEndDate(param.toDate());

  param = pParams.value("period_id", &valid);
  if (valid)
  {
    q.prepare( "SELECT period_start, period_end "
               "FROM period "
               "WHERE (period_id=:period_id);" );
    q.bindValue(":period_id", param.toInt());
    q.exec();
    if (q.first())
    {
      _dates->setStartDate(q.value("period_start").toDate());
      _dates->setEndDate(q.value("period_end").toDate());
    }
    else if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return UndefinedError;
    }
  }

  if (pParams.inList("run"))
  {
    sFillList();
    return NoError_Run;
  }

  return NoError;
}

void dspInvoiceRegister::sPopulateMenu(QMenu *pMenu, QTreeWidgetItem*, int)
{
  QAction *menuItem;

  if (list()->altId() == 1)
  {
    menuItem = pMenu->addAction(tr("View Invoice..."), this, SLOT(sViewInvoice()));
    if (! _privileges->check("MaintainMiscInvoices") &&
        ! _privileges->check("ViewMiscInvoices"))
      menuItem->setEnabled(false);
  }

  else if (list()->altId() == 2)
  {
    menuItem = pMenu->addAction(tr("View Credit Memo..."), this, SLOT(sViewCreditMemo()));
    if (! _privileges->check("MaintainARMemos") &&
        ! _privileges->check("ViewARMemos"))
      menuItem->setEnabled(false);
  }
  else if (list()->altId() == 3)
  {
    menuItem = pMenu->addAction(tr("View Debit Memo..."), this, SLOT(sViewCreditMemo()));
    if (! _privileges->check("MaintainARMemos") &&
        ! _privileges->check("ViewARMemos"))
      menuItem->setEnabled(false);
  }
  else if (list()->altId() == 4)
  {
    menuItem = pMenu->addAction(tr("View Customer Deposit..."), this, SLOT(sViewCreditMemo()));
    if (! _privileges->check("MaintainARMemos") &&
        ! _privileges->check("ViewARMemos"))
      menuItem->setEnabled(false);
  }
}

void dspInvoiceRegister::sViewCreditMemo()
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
            "  AND  (aropen_doctype=:doctype)"
            ") ORDER BY type LIMIT 1;");
  q.bindValue(":docnum", list()->currentItem()->text(3));
  
  if(list()->altId()==1)
    q.bindValue(":doctype", "I");
  else if(list()->altId()==2)
    q.bindValue(":doctype", "C");
  else if(list()->altId()==3)
    q.bindValue(":doctype", "D");
  else if(list()->altId()==4)
    q.bindValue(":doctype", "R");
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
    if (list()->altId() == 2)
      QMessageBox::information(this, tr("Credit Memo Not Found"),
                               tr("<p>The Credit Memo #%1 could not be found.")
                               .arg(list()->currentItem()->text(3)));
    else if (list()->altId() == 3)
      QMessageBox::information(this, tr("Debit Memo Not Found"),
                               tr("<p>The Debit Memo #%1 could not be found.")
                               .arg(list()->currentItem()->text(3)));
    else
      QMessageBox::information(this, tr("Document Not Found"),
                               tr("<p>The Document #%1 could not be found.")
                               .arg(list()->currentItem()->text(3)));
    return;
  }
}

void dspInvoiceRegister::sViewInvoice()
{
  ParameterList params;
  params.append("invoiceNumber", list()->currentItem()->text(3));

  dspInvoiceInformation* newdlg = new dspInvoiceInformation();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

bool dspInvoiceRegister::setParams(ParameterList &params)
{
  if(!_dates->allValid())
  {
    QMessageBox::warning(this, tr("Invalid Date Range"),
      tr("You must specify a valid date range.") );
    return false;
  }

  _dates->appendValue(params);

  if (_selectedAccount->isChecked())
    params.append("accnt_id", _account->id());

  params.append("invoice",      tr("Invoice"));
  params.append("creditmemo",   tr("Credit Memo"));
  params.append("debitmemo",    tr("Debit Memo"));
  params.append("cashdeposit",  tr("Customer Deposit"));

  return true;
}

void dspInvoiceRegister::sFillList()
{
  display::sFillList();
  list()->expandAll();

  // calculate subtotals and grand total for debit and credit columns and add rows for them
  double debittotal = 0.0;
  double credittotal = 0.0;
  for (int i = 0; i < list()->topLevelItemCount(); i++)
  {
    double debitsum = 0.0;
    double creditsum = 0.0;
    XTreeWidgetItem *item = 0;
    for (int j = 0; j < list()->topLevelItem(i)->childCount(); j++)
    {
      item = list()->topLevelItem(i)->child(j);
//      qDebug("in loop @ %d %p", j, item);
      if (item)
      {
        debitsum += item->rawValue("debit").toDouble();
        creditsum += item->rawValue("credit").toDouble();
        debittotal += item->rawValue("debit").toDouble();
        credittotal += item->rawValue("credit").toDouble();
      }
    }
    if (item)
    {
//      qDebug("adding subtotal %p", item);
      item = new XTreeWidgetItem(list()->topLevelItem(i), -1, -1, tr("Subtotal"));
      item->setData(list()->column("debit"),  Qt::EditRole, formatMoney(debitsum));
      item->setData(list()->column("credit"), Qt::EditRole, formatMoney(creditsum));
    }
  }
  XTreeWidgetItem *item = new XTreeWidgetItem(list(), -1, -1, tr("Total"));
  item->setData(list()->column("debit"),  Qt::EditRole, formatMoney(debittotal));
  item->setData(list()->column("credit"), Qt::EditRole, formatMoney(credittotal));
}
