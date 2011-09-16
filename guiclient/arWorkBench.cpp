/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "arWorkBench.h"

#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include <stdlib.h>
#include <metasql.h>
#include "mqlutil.h"

#include "cashReceipt.h"
#include "creditcardprocessor.h"
#include "storedProcErrorLookup.h"
#include "xtreewidget.h"

arWorkBench::arWorkBench(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  _aritems = new dspAROpenItems(this, "_aritems", Qt::Widget);
  _aropenFrame->layout()->addWidget(_aritems);
  _aritems->setCloseVisible(false);
  _aritems->findChild<QWidget*>("_customerSelector")->hide();
  _aritems->queryAction()->setVisible(false);
  _aritems->findChild<QWidget*>("_asofGroup")->hide();
  _aritems->findChild<DLineEdit*>("_asOf")->setDate(omfgThis->endOfTime());
  _aritems->findChild<QWidget*>("_dateGroup")->hide();
  _aritems->findChild<QWidget*>("_showGroup")->hide();
  _aritems->findChild<QWidget*>("_printGroup")->hide();
  _aritems->findChild<QRadioButton*>("_dueDate")->click();
  
  _cctrans = new dspCreditCardTransactions(this, "_cctrans", Qt::Widget);
  _creditCardTab->layout()->addWidget(_cctrans);
  _cctrans->findChild<QWidget*>("_close")->hide();
  _cctrans->findChild<QWidget*>("_customerSelector")->hide();
  _cctrans->findChild<QWidget*>("_query")->hide();
  _cctrans->findChild<QWidget*>("_alltrans")->hide();
  _cctrans->findChild<QWidget*>("_pending")->hide();
  _cctrans->findChild<QWidget*>("_processed")->hide();
  _cctrans->findChild<XTreeWidget*>("_preauth")->hideColumn("type");
  _cctrans->findChild<XTreeWidget*>("_preauth")->hideColumn("status");
  
  connect(_query, SIGNAL(clicked()), this, SLOT(sFillList()));
  connect(_newCashrcpt, SIGNAL(clicked()), this, SLOT(sNewCashrcpt()));
  connect(_editCashrcpt, SIGNAL(clicked()), this, SLOT(sEditCashrcpt()));
  connect(_viewCashrcpt, SIGNAL(clicked()), this, SLOT(sViewCashrcpt()));
  connect(_deleteCashrcpt, SIGNAL(clicked()), this, SLOT(sDeleteCashrcpt()));
  connect(_postCashrcpt, SIGNAL(clicked()), this, SLOT(sPostCashrcpt()));
  connect(_cashrcpt, SIGNAL(populateMenu(QMenu*, QTreeWidgetItem*)),
          this, SLOT(sPopulateCashRcptMenu(QMenu*)));
  connect(_customerSelector, SIGNAL(newState(int)), this, SLOT(sClear()));
  connect(_customerSelector, SIGNAL(newCustId(int)), this, SLOT(sClear()));
  connect(_customerSelector, SIGNAL(newCustTypeId(int)), this, SLOT(sClear()));
  connect(_customerSelector, SIGNAL(newTypePattern(QString)), this, SLOT(sClear()));
  connect(_customerSelector, SIGNAL(newCustGroupId(int)), this, SLOT(sClear()));

  connect(_customerSelector, SIGNAL(newState(int)), 
          _aritems->findChild<CustomerSelector*>("_customerSelector"), SLOT(setState(int)));
  connect(_customerSelector, SIGNAL(newCustId(int)), 
          _aritems->findChild<CustomerSelector*>("_customerSelector"), SLOT(setCustId(int)));
  connect(_customerSelector, SIGNAL(newCustTypeId(int)), 
          _aritems->findChild<CustomerSelector*>("_customerSelector"), SLOT(setCustTypeId(int)));
  connect(_customerSelector, SIGNAL(newCustGroupId(int)), 
          _aritems->findChild<CustomerSelector*>("_customerSelector"), SLOT(setCustGroupId(int)));
  connect(_customerSelector, SIGNAL(newTypePattern(QString)), 
          _aritems->findChild<CustomerSelector*>("_customerSelector"), SLOT(setTypePattern(QString)));
  connect(_customerSelector, SIGNAL(newState(int)), 
          _cctrans->findChild<CustomerSelector*>("_customerSelector"), SLOT(setState(int)));
  connect(_customerSelector, SIGNAL(newCustId(int)), 
          _cctrans->findChild<CustomerSelector*>("_customerSelector"), SLOT(setCustId(int)));
  connect(_customerSelector, SIGNAL(newCustTypeId(int)), 
          _cctrans->findChild<CustomerSelector*>("_customerSelector"), SLOT(setCustTypeId(int)));
  connect(_customerSelector, SIGNAL(newCustGroupId(int)), 
          _cctrans->findChild<CustomerSelector*>("_customerSelector"), SLOT(setCustGroupId(int)));
  connect(_customerSelector, SIGNAL(newTypePattern(QString)), 
          _cctrans->findChild<CustomerSelector*>("_customerSelector"), SLOT(setTypePattern(QString)));
  connect(_debits, SIGNAL(clicked()), 
          _aritems->findChild<QRadioButton*>("_debits"), SLOT(click()));
  connect(_credits, SIGNAL(clicked()), 
          _aritems->findChild<QRadioButton*>("_credits"), SLOT(click()));
  connect(_both, SIGNAL(clicked()), 
          _aritems->findChild<QRadioButton*>("_both"), SLOT(click()));
  connect(_searchDocNum, SIGNAL(textChanged(const QString&)), this, SLOT(sSearchDocNumChanged()));

  _cashrcpt->addColumn(tr("Cust. #"),       _bigMoneyColumn, Qt::AlignLeft,  true, "cust_number");                                                                
  _cashrcpt->addColumn(tr("Name"),                       -1, Qt::AlignLeft,  true, "cust_name"); 
  _cashrcpt->addColumn(tr("Check/Doc. #"),     _orderColumn, Qt::AlignLeft,  true, "cashrcpt_docnumber");
  _cashrcpt->addColumn(tr("Bank Account"),     _orderColumn, Qt::AlignLeft,  true, "bankaccnt_name");
  _cashrcpt->addColumn(tr("Dist. Date"),        _dateColumn, Qt::AlignCenter,true, "cashrcpt_distdate");
  _cashrcpt->addColumn(tr("Funds Type"),    _bigMoneyColumn, Qt::AlignCenter,true, "cashrcpt_fundstype");
  _cashrcpt->addColumn(tr("Amount"),        _bigMoneyColumn, Qt::AlignRight, true, "cashrcpt_amount");
  _cashrcpt->addColumn(tr("Currency"),      _currencyColumn, Qt::AlignLeft,  true, "currabbr");
  
  if (_privileges->check("MaintainCashReceipts"))
  {
    connect(_cashrcpt, SIGNAL(valid(bool)), _editCashrcpt, SLOT(setEnabled(bool)));
    connect(_cashrcpt, SIGNAL(valid(bool)), _deleteCashrcpt, SLOT(setEnabled(bool)));
    connect(_cashrcpt, SIGNAL(valid(bool)), _postCashrcpt, SLOT(setEnabled(bool)));
  }
  else
  {
    _newCashrcpt->setEnabled(FALSE);
    connect(_cashrcpt, SIGNAL(itemSelected(int)), _viewCashrcpt, SLOT(animateClick()));
  }
  
  if(_privileges->check("PostCashReceipts"))
    connect(_cashrcpt, SIGNAL(itemSelected(int)), _editCashrcpt, SLOT(animateClick()));
  connect(omfgThis, SIGNAL(cashReceiptsUpdated(int, bool)), this, SLOT(sFillList()));

  if (omfgThis->singleCurrency())
    _cashrcpt->hideColumn(2);

  if (!_metrics->boolean("CCAccept") || !_privileges->check("ProcessCreditCards"))
    _tab->removeTab(_tab->indexOf(_creditCardTab));
}

arWorkBench::~arWorkBench()
{
  // no need to delete child widgets, Qt does it all for us
}

void arWorkBench::languageChange()
{
  retranslateUi(this);
}

enum SetResponse arWorkBench::set( const ParameterList & pParams )
{
  XWidget::set(pParams);
  QVariant param;
  bool    valid;
  
  param = pParams.value("cust_id", &valid);
  if (valid)
  {
    _customerSelector->setCustId(param.toInt());
    sFillList();
  }

  return NoError;
}

bool arWorkBench::setParams(ParameterList &params)
{
  _customerSelector->appendValue(params);
  params.append("invoice", tr("Invoice"));
  params.append("creditMemo", tr("Credit Memo"));
  params.append("debitMemo", tr("Debit Memo"));
  params.append("cashdeposit", tr("Customer Deposit"));
  params.append("other", tr("Other"));
  params.append("orderByDocDate");
  
  return true;
}


void arWorkBench::sFillList()
{ 
  if (_selectDate->currentIndex()==0)
  {
    _aritems->findChild<DateCluster*>("_dates")->setStartNull(tr("Earliest"), omfgThis->startOfTime(), TRUE);
    _aritems->findChild<DateCluster*>("_dates")->setEndNull(tr("Latest"), omfgThis->endOfTime(), TRUE);
  }
  else if (_selectDate->currentIndex()==1)
  {
    _aritems->findChild<DateCluster*>("_dates")->setStartNull(tr("Earliest"), omfgThis->startOfTime(), TRUE);
    _aritems->findChild<DateCluster*>("_dates")->setEndDate(_onOrBeforeDate->date());
  }
  else
  {
    _aritems->findChild<DateCluster*>("_dates")->setStartDate(_startDate->date());
    _aritems->findChild<DateCluster*>("_dates")->setEndDate(_endDate->date());
  }

  _aritems->findChild<XCheckBox*>("_unposted")->setChecked(_unposted->isChecked());
  
  _aritems->findChild<QWidget*>("_dateGroup")->hide();
  _aritems->findChild<QWidget*>("_dateGroup")->hide();
  _aritems->sFillList();
  sFillCashrcptList();
  _cctrans->sFillList();
}

void arWorkBench::sClear()
{
  _aritems->list()->clear();
  _cashrcpt->clear();
  _cctrans->findChild<XTreeWidget*>("_preauth")->clear();
}

void arWorkBench::sFillCashrcptList()
{
  MetaSQLQuery mql = mqlLoad("unpostedCashReceipts", "detail");
  ParameterList params;
  setParams(params);
  params.append("check", tr("Check"));
  params.append("certifiedCheck", tr("Certified Check"));
  params.append("masterCard", tr("Master Card"));
  params.append("visa", tr("Visa"));
  params.append("americanExpress", tr("American Express"));
  params.append("discoverCard", tr("Discover Card"));
  params.append("otherCreditCard", tr("Other Credit Card"));
  params.append("cash", tr("Cash"));
  params.append("wireTransfer", tr("Wire Transfer"));
  params.append("other", tr("Other"));
  q = mql.toQuery(params);
  _cashrcpt->populate(q);
}

void arWorkBench::sNewCashrcpt()
{
  ParameterList params;
  params.append("mode", "new");
  if (_customerSelector->isSelectedCust())
    params.append("cust_id", _customerSelector->custId());

  cashReceipt *newdlg = new cashReceipt();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void arWorkBench::sEditCashrcpt()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("cashrcpt_id", _cashrcpt->id());

  cashReceipt *newdlg = new cashReceipt();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void arWorkBench::sViewCashrcpt()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("cashrcpt_id", _cashrcpt->id());

  cashReceipt *newdlg = new cashReceipt();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void arWorkBench::sDeleteCashrcpt()
{
  q.prepare("SELECT deleteCashrcpt(:cashrcpt_id) AS result;");
  q.bindValue(":cashrcpt_id", _cashrcpt->id());
  q.exec();
  if (q.first())
  {
    int result = q.value("result").toInt();
    if (result < 0)
    {
      systemError(this, storedProcErrorLookup("deleteCashrcpt", result));
      return;
    }
  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
  sFillCashrcptList();
}

void arWorkBench::sPostCashrcpt()
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

  QList<XTreeWidgetItem*> selected = _cashrcpt->selectedItems();
  for (int i = 0; i < selected.size(); i++)
  {
    q.prepare("SELECT postCashReceipt(:cashrcpt_id, :journalNumber) AS result;");
    q.bindValue(":cashrcpt_id", ((XTreeWidgetItem*)(selected[i]))->id());
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
  }
  tx.exec("COMMIT;");
  sFillList();
}

void arWorkBench::sPopulateCashRcptMenu(QMenu *pMenu)
{
  QAction *menuItem;

  menuItem = pMenu->addAction(tr("Edit Cash Receipt..."), this, SLOT(sEditCashrcpt()));
  if (! _privileges->check("MaintainCashReceipts") &&
      ! _privileges->check("ViewCashReceipts"))
    menuItem->setEnabled(false);

  menuItem = pMenu->addAction(tr("View Cash Receipt..."), this, SLOT(sViewCashrcpt()));
  if (! _privileges->check("ViewCashReceipts"))
    menuItem->setEnabled(false);

  menuItem = pMenu->addAction(tr("Delete Cash Receipt..."), this, SLOT(sDeleteCashrcpt()));
  if (! _privileges->check("MaintainCashReceipts"))
    menuItem->setEnabled(false);

  pMenu->addSeparator();

  menuItem = pMenu->addAction(tr("Post Cash Receipt..."), this, SLOT(sPostCashrcpt()));
  if (! _privileges->check("PostCashReceipts"))
    menuItem->setEnabled(false);
}

void arWorkBench::sSearchDocNumChanged()
{
  XTreeWidget *aropen = _aritems->list();
  QString sub = _searchDocNum->text().trimmed();
  if(sub.isEmpty())
    return;

  QList<XTreeWidgetItem*> list = aropen->findItems(sub, Qt::MatchFixedString|Qt::MatchCaseSensitive, 4);
  if(list.isEmpty())
    list = aropen->findItems(sub, Qt::MatchFixedString|Qt::MatchStartsWith|Qt::MatchCaseSensitive, 4);

  if(!list.isEmpty())
  {
    aropen->setCurrentItem(list.at(0));
    aropen->scrollTo(aropen->currentIndex());
  }
}


