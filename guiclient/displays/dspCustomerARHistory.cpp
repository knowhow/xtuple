/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspCustomerARHistory.h"

#include <QAction>
#include <QMenu>
#include <QMessageBox>
#include <QVariant>

#include "arOpenItem.h"

dspCustomerARHistory::dspCustomerARHistory(QWidget* parent, const char* name, Qt::WFlags fl)
  : display(parent, "dspCustomerARHistory", fl)
{
  setupUi(optionsWidget());
  setWindowTitle(tr("Customer History"));
  setListLabel(tr("Customer History"));
  setReportName("CustomerARHistory");
  setMetaSQLOptions("arHistory", "detail");
  setUseAltId(true);

  list()->setRootIsDecorated(true);
  list()->addColumn(tr("Open"),          _dateColumn,     Qt::AlignCenter, true,  "open" );
  list()->addColumn(tr("Doc. Type"),     _itemColumn,     Qt::AlignCenter, true,  "documenttype" );
  list()->addColumn(tr("Doc. #"),        _orderColumn,    Qt::AlignRight,  true,  "docnumber"  );
  list()->addColumn(tr("Doc. Date"),     _dateColumn,     Qt::AlignCenter, true,  "docdate" );
  list()->addColumn(tr("Due/Dist Date"), _dateColumn,     Qt::AlignCenter, true,  "duedate" );
  list()->addColumn(tr("Amount"),        _moneyColumn,    Qt::AlignRight,  true,  "amount"  );
  list()->addColumn(tr("Balance"),       _moneyColumn,    Qt::AlignRight,  true,  "balance"  );
  list()->addColumn(tr("Currency"),      _currencyColumn, Qt::AlignCenter, true,  "currAbbr"  );
  list()->addColumn(tr("Base Balance"),  _bigMoneyColumn, Qt::AlignRight,  true,  "base_balance"  );

  _cust->setFocus();
}

void dspCustomerARHistory::languageChange()
{
  display::languageChange();
  retranslateUi(this);
}

enum SetResponse dspCustomerARHistory::set(const ParameterList &pParams)
{
  XWidget::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("cust_id", &valid);
  if (valid)
    _cust->setId(param.toInt());

  param = pParams.value("startDate", &valid);
  if (valid)
    _dates->setStartDate(param.toDate());

  param = pParams.value("endDate", &valid);
  if (valid)
    _dates->setEndDate(param.toDate());

  if (pParams.inList("run"))
  {
    sFillList();
    return NoError_Run;
  }

  return NoError;
}

void dspCustomerARHistory::sPopulateMenu(QMenu *pMenu, QTreeWidgetItem *pSelected, int)
{
  QAction *menuItem;

  if (((XTreeWidgetItem *)pSelected)->id() != -1)
  {
    menuItem = pMenu->addAction(tr("Edit..."), this, SLOT(sEdit()));
    menuItem->setEnabled(_privileges->check("EditAROpenItem"));

    pMenu->addAction(tr("View..."), this, SLOT(sView()));
  }
}

void dspCustomerARHistory::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("aropen_id", list()->id());

  arOpenItem newdlg(this, "", true);
  newdlg.set(params);
  newdlg.exec();
}

void dspCustomerARHistory::sView()
{
  ParameterList params;
  params.append("mode", "view");

  if (list()->id() == -1)
    params.append("aropen_id", list()->altId());
  else
    params.append("aropen_id", list()->id());

  arOpenItem newdlg(this, "", true);
  newdlg.set(params);
  newdlg.exec();
}

bool dspCustomerARHistory::setParams(ParameterList &params)
{
  if (!_cust->isValid())
  {
    if (isVisible())
    {
      QMessageBox::warning( this, tr("Enter Customer Number"),
                            tr("Please enter a valid Customer Number.") );
      _cust->setFocus();
    }
    return false;
  }

  if (!_dates->startDate().isValid())
  {
    if (isVisible())
    {
      QMessageBox::warning( this, tr("Enter Start Date"),
                            tr("Please enter a valid Start Date.") );
      _dates->setFocus();
    }
    return false;
  }

  if (!_dates->endDate().isValid())
  {
    if (isVisible())
    {
      QMessageBox::warning( this, tr("Enter End Date"),
                            tr("Please enter a valid End Date.") );
      _dates->setFocus();
    }
    return false;
  }

  _dates->appendValue(params);
  params.append("invoice", tr("Invoice"));
  params.append("zeroinvoice", tr("Zero Invoice"));
  params.append("creditMemo", tr("Credit Memo"));
  params.append("debitMemo", tr("Debit Memo"));
  params.append("check", tr("Check"));
  params.append("certifiedCheck", tr("Certified Check"));
  params.append("masterCard", tr("Master Card"));
  params.append("visa", tr("Visa"));
  params.append("americanExpress", tr("American Express"));
  params.append("discoverCard", tr("Discover Card"));
  params.append("otherCreditCard", tr("Other Credit Card"));
  params.append("cash", tr("Cash"));
  params.append("wireTransfer", tr("Wire Transfer"));
  params.append("cashdeposit", tr("Customer Deposit"));
  params.append("other", tr("Other"));
  params.append("cust_id", _cust->id());

  return true;
}
