/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "selectPayments.h"
#include "mqlutil.h"

#include <QSqlError>

#include <metasql.h>
#include <openreports.h>
#include <parameter.h>
#include <QMessageBox>

#include "guiclient.h"
#include "selectBankAccount.h"
#include "selectPayment.h"
#include "storedProcErrorLookup.h"

selectPayments::selectPayments(QWidget* parent, const char* name, Qt::WFlags fl, bool pAutoFill)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  _ignoreUpdates = true;

  connect(_clear, SIGNAL(clicked()), this, SLOT(sClear()));
  connect(_clearAll, SIGNAL(clicked()), this, SLOT(sClearAll()));
  connect(_selectDate, SIGNAL(currentIndexChanged(int)), this, SLOT(sFillList()));
  connect(_startDate, SIGNAL(newDate(const QDate&)), this, SLOT(sFillList()));
  connect(_endDate, SIGNAL(newDate(const QDate&)), this, SLOT(sFillList()));
  connect(_onOrBeforeDate, SIGNAL(newDate(const QDate&)), this, SLOT(sFillList()));
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_select, SIGNAL(clicked()), this, SLOT(sSelect()));
  connect(_selectDiscount, SIGNAL(clicked()), this, SLOT(sSelectDiscount()));
  connect(_selectDue, SIGNAL(clicked()), this, SLOT(sSelectDue()));
  connect(_selectLine, SIGNAL(clicked()), this, SLOT(sSelectLine()));
  connect(_applyallcredits, SIGNAL(clicked()), this, SLOT(sApplyAllCredits()));
  connect(_vendorgroup, SIGNAL(updated()), this, SLOT(sFillList()));
  connect(_bankaccnt, SIGNAL(newID(int)), this, SLOT(sFillList()));
  connect(_apopen, SIGNAL(populateMenu(QMenu*, QTreeWidgetItem*, int)), this, SLOT(sPopulateMenu(QMenu*, QTreeWidgetItem*)));

  _bankaccnt->setType(XComboBox::APBankAccounts);

  _apopen->addColumn(tr("Vendor"),         -1,           Qt::AlignLeft  ,        true, "vendor" );
  _apopen->addColumn(tr("Doc. Type"),      _orderColumn, Qt::AlignCenter,        true, "doctype" );
  _apopen->addColumn(tr("Doc. #"),         _orderColumn, Qt::AlignRight ,        true, "apopen_docnumber" );
  _apopen->addColumn(tr("Inv. #"),         _orderColumn, Qt::AlignRight ,        true, "apopen_invcnumber" );
  _apopen->addColumn(tr("P/O #"),          _orderColumn, Qt::AlignRight ,        true, "apopen_ponumber" );
  _apopen->addColumn(tr("Due Date"),       _dateColumn,  Qt::AlignCenter,        true, "apopen_duedate" );
  _apopen->addColumn(tr("Doc. Date"),      _dateColumn,  Qt::AlignCenter,        false, "apopen_docdate" );
  _apopen->addColumn(tr("Amount"),         _moneyColumn, Qt::AlignRight ,        true, "amount" );
  _apopen->addColumn(tr("Amount (%1)").arg(CurrDisplay::baseCurrAbbr()), _moneyColumn, Qt::AlignRight, false, "base_amount"  );
  _apopen->addColumn(tr("Running Amount (%1)").arg(CurrDisplay::baseCurrAbbr()), _moneyColumn, Qt::AlignRight,         false, "running_amount" );
  _apopen->addColumn(tr("Selected"),       _moneyColumn, Qt::AlignRight ,        true, "selected" );
  _apopen->addColumn(tr("Selected (%1)").arg(CurrDisplay::baseCurrAbbr()), _moneyColumn, Qt::AlignRight, false, "base_selected"  );
  _apopen->addColumn(tr("Running Selected (%1)").arg(CurrDisplay::baseCurrAbbr()), _moneyColumn, Qt::AlignRight,       true, "running_selected"  );
  _apopen->addColumn(tr("Discount"),       _moneyColumn, Qt::AlignRight ,        true, "discount" );
  _apopen->addColumn(tr("Discount (%1)").arg(CurrDisplay::baseCurrAbbr()), _moneyColumn, Qt::AlignRight, false, "base_discount"  );
  _apopen->addColumn(tr("Currency"),       _currencyColumn, Qt::AlignLeft,       true, "curr_concat" );
  _apopen->addColumn(tr("Status"),         _currencyColumn, Qt::AlignCenter,     true, "apopen_status" );

//  if (omfgThis->singleCurrency())
//  {
//    _apopen->hideColumn("curr_concat");
//    _apopen->headerItem()->setText(11, tr("Running"));
//  }

  connect(omfgThis, SIGNAL(paymentsUpdated(int, int, bool)), this, SLOT(sFillList()));

  _ignoreUpdates = false;

  if(pAutoFill)
    sFillList();
}

selectPayments::~selectPayments()
{
  // no need to delete child widgets, Qt does it all for us
}

void selectPayments::languageChange()
{
  retranslateUi(this);
}

bool selectPayments::setParams(ParameterList &params)
{
  _vendorgroup->appendValue(params);

  return true;
}

void selectPayments::sPrint()
{
  ParameterList params;
  if (! setParams(params))
    return;

  orReport report("SelectPaymentsList", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void selectPayments::sSelectDue()
{
  ParameterList params;
  params.append("type", "P");

  int bankaccntid = _bankaccnt->id();
  if(bankaccntid == -1)
  {
    selectBankAccount newdlg(this, "", TRUE);
    newdlg.set(params);
    bankaccntid = newdlg.exec();
  }

  if (bankaccntid >= 0)
  {
    MetaSQLQuery mql("SELECT selectDueItemsForPayment("
                     "    <? if exists(\"vend_id\") ?> <? value(\"vend_id\") ?>"
                     "    <? else ?> vend_id <? endif ?>,"
                     "    <? value(\"bankaccnt_id\") ?>) AS result "
                     "<? if exists(\"vend_id\") ?>"
                     ";"
                     "<? elseif exists(\"vendtype_id\") ?>"
                     "FROM vendinfo "
                     "WHERE (vend_vendtype_id=<? value(\"vendtype_id\") ?>);"
                     "<? elseif exists(\"vendtype_pattern\") ?>"
                     "FROM vendinfo "
                     "WHERE (vend_vendtype_id IN (SELECT vendtype_id"
                     "                            FROM vendtype"
                     "                            WHERE (vendtype_code ~ <? value(\"vendtype_pattern\") ?>)));"
                     "<? else ?>"
                     "FROM vendinfo;"
                     "<? endif ?>");
    ParameterList params;
    if (! setParams(params))
        return;
    params.append("bankaccnt_id", bankaccntid);
    q = mql.toQuery(params);
    if (q.first())
    {
      int result = q.value("result").toInt();
      if (result < 0)
      {
        systemError(this, storedProcErrorLookup("selectDueItemsForPayment", result),
                    __FILE__, __LINE__);
        return;
      }
    }
    else if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }

    omfgThis->sPaymentsUpdated(-1, -1, TRUE);
  }
}

void selectPayments::sSelectDiscount()
{
  ParameterList params;
  params.append("type", "P");

  int bankaccntid = _bankaccnt->id();
  if(bankaccntid == -1)
  {
    selectBankAccount newdlg(this, "", TRUE);
    newdlg.set(params);
    bankaccntid = newdlg.exec();
  }

  if (bankaccntid >= 0)
  {
    MetaSQLQuery mql("SELECT selectDiscountItemsForPayment("
                     "    <? if exists(\"vend_id\") ?> <? value(\"vend_id\") ?>"
                     "    <? else ?> vend_id <? endif ?>,"
                     "    <? value(\"bankaccnt_id\") ?>) AS result "
                     "<? if exists(\"vend_id\") ?>"
                     ";"
                     "<? elseif exists(\"vendtype_id\") ?>"
                     "FROM vendinfo "
                     "WHERE (vend_vendtype_id=<? value(\"vendtype_id\") ?>);"
                     "<? elseif exists(\"vendtype_pattern\") ?>"
                     "FROM vendinfo "
                     "WHERE (vend_vendtype_id IN (SELECT vendtype_id"
                     "                            FROM vendtype"
                     "                            WHERE (vendtype_code ~ <? value(\"vendtype_pattern\") ?>)));"
                     "<? else ?>"
                     "FROM vendinfo;"
                     "<? endif ?>");
    ParameterList params;
    if (! setParams(params))
        return;
    params.append("bankaccnt_id", bankaccntid);
    q = mql.toQuery(params);
    if (q.first())
    {
      int result = q.value("result").toInt();
      if (result < 0)
      {
        systemError(this, storedProcErrorLookup("selectDiscountItemsForPayment", result),
                    __FILE__, __LINE__);
        return;
      }
    }
    else if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
    omfgThis->sPaymentsUpdated(-1, -1, TRUE);
  }
}

void selectPayments::sClearAll()
{
  switch (_vendorgroup->state())
  {
    case VendorGroup::All:
      q.prepare( "SELECT clearPayment(apselect_id) AS result "
                 "FROM apselect;" );
        break;
    case VendorGroup::Selected:
      q.prepare( "SELECT clearPayment(apselect_id) AS result "
                 "FROM apopen JOIN apselect ON (apselect_apopen_id=apopen_id) "
                 "WHERE (apopen_vend_id=:vend_id);" );
      break;
    case VendorGroup::SelectedType:
      q.prepare( "SELECT clearPayment(apselect_id) AS result "
                 "FROM vendinfo JOIN apopen ON (apopen_vend_id=vend_id) "
                 "              JOIN apselect ON (apselect_apopen_id=apopen_id) "
                 "WHERE (vend_vendtype_id=:vendtype_id) ;" );
      break;
    case VendorGroup::TypePattern:
      q.prepare( "SELECT clearPayment(apselect_id) AS result "
                 "FROM vendinfo JOIN apopen ON (apopen_vend_id=vend_id) "
                 "              JOIN apselect ON (apselect_apopen_id=apopen_id) "
                 "WHERE (vend_vendtype_id IN (SELECT vendtype_id"
                 "                            FROM vendtype"
                 "                            WHERE (vendtype_code ~ :vendtype_pattern)));" );
        break;
    }

  _vendorgroup->bindValue(q);
  q.exec();
  if (q.first())
  {
    int result = q.value("result").toInt();
    if (result < 0)
    {
      systemError(this, storedProcErrorLookup("clearPayment", result),
                  __FILE__, __LINE__);
      return;
    }
  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  omfgThis->sPaymentsUpdated(-1, -1, TRUE);
}

void selectPayments::sSelect()
{
  _ignoreUpdates = true;
  bool update = false;
  QList<XTreeWidgetItem*> list = _apopen->selectedItems();
  XTreeWidgetItem * cursor = 0;
  XSqlQuery slct;
  slct.prepare("SELECT apopen_status FROM apopen WHERE apopen_id=:apopen_id;");
  for(int i = 0; i < list.size(); i++)
  {
    cursor = (XTreeWidgetItem*)list.at(i);
    slct.bindValue(":apopen_id", cursor->id());
    slct.exec();
    if (slct.first())
    {
      if (slct.value("apopen_status").toString() == "H")
      {
        QMessageBox::critical( this, tr("Can not do Payment"), tr( "Item is On Hold" ) );
        return;
      }
      else
      {
        ParameterList params;
        params.append("apopen_id", cursor->id());

        if(_bankaccnt->id() != -1)
          params.append("bankaccnt_id", _bankaccnt->id());

        selectPayment newdlg(this, "", TRUE);
        newdlg.set(params);
        if(newdlg.exec() != XDialog::Rejected)
          update = true;
      }
    }
    else
    {
      QMessageBox::critical( this, tr("Can not do Payment"), tr( "apopen not found" ) );
      return;
    }
  }
  _ignoreUpdates = false;
  if(update)
    sFillList();
}
void selectPayments::sSelectLine()
{
  ParameterList params;
  params.append("type", "P");

  int bankaccntid = _bankaccnt->id();
  if(bankaccntid == -1)
  {
    selectBankAccount newdlg(this, "", TRUE);
    newdlg.set(params);
    bankaccntid = newdlg.exec();
  }

  if (bankaccntid != -1)
  {
    bool update = FALSE;
    QList<XTreeWidgetItem*> list = _apopen->selectedItems();
    XTreeWidgetItem * cursor = 0;
    q.prepare("SELECT selectPayment(:apopen_id, :bankaccnt_id) AS result;");
    XSqlQuery slctln;
    slctln.prepare( "SELECT apopen_status FROM apopen WHERE apopen_id=:apopen_id;");
    for(int i = 0; i < list.size(); i++)
    {
      cursor = (XTreeWidgetItem*)list.at(i);
      q.bindValue(":apopen_id", cursor->id());
      q.bindValue(":bankaccnt_id", bankaccntid);
          slctln.bindValue(":apopen_id", cursor->id());
      slctln.exec();
      if (slctln.first())
      {
        if (slctln.value("apopen_status").toString() != "H")
            {
      q.exec();
      if (q.first())
      {
        int result = q.value("result").toInt();
        if (result < 0)
        {
          systemError(this, cursor->text(0) + " " + cursor->text(2) + "\n" +
                            storedProcErrorLookup("selectPayment", result),
                      __FILE__, __LINE__);
          return;
        }
      }
      else if (q.lastError().type() != QSqlError::NoError)
      {
        systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
        return;
      }
                }
      update = TRUE;
    }
    }
    if(update)
      omfgThis->sPaymentsUpdated(-1, -1, TRUE);
  }
}

void selectPayments::sClear()
{
  bool update = FALSE;
  QList<XTreeWidgetItem*> list = _apopen->selectedItems();
  XTreeWidgetItem * cursor = 0;
  q.prepare("SELECT clearPayment(:apopen_id) AS result;");
  for(int i = 0; i < list.size(); i++)
  {
    cursor = (XTreeWidgetItem*)list.at(i);
    q.bindValue(":apopen_id", cursor->altId());
    q.exec();
    if (q.first())
    {
      int result = q.value("result").toInt();
      if (result < 0)
      {
        systemError(this, cursor->text(0) + " " + cursor->text(2) + "\n" +
                          storedProcErrorLookup("clearPayment", result),
                    __FILE__, __LINE__);
        return;
      }
    }
    else if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
    update = TRUE;
  }

  if(update)
    omfgThis->sPaymentsUpdated(-1, -1, TRUE);
}

void selectPayments::sApplyAllCredits()
{
  MetaSQLQuery mql = mqlLoad("selectPayments", "applyallcredits");
  ParameterList params;
  if (! setParams(params))
    return;
  q = mql.toQuery(params);
  if (q.first())
    sFillList();
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void selectPayments::sFillList()
{
  if(_ignoreUpdates)
    return;

//  if (_vendorgroup->isSelectedVend())
//    _apopen->showColumn(9);
//  else
//    _apopen->hideColumn(9);

  if ( (_selectDate->currentIndex() == 1 && !_onOrBeforeDate->isValid())  ||
        (_selectDate->currentIndex() == 2 && (!_startDate->isValid() || !_endDate->isValid())) )
    return;

  int _currid = -1;
  if (_bankaccnt->isValid())
  {
    q.prepare( "SELECT bankaccnt_curr_id "
               "FROM bankaccnt "
               "WHERE (bankaccnt_id=:bankaccnt_id);" );
    q.bindValue(":bankaccnt_id", _bankaccnt->id());
    q.exec();
    if (q.first())
      _currid = q.value("bankaccnt_curr_id").toInt();
  }

  ParameterList params;
  if (! setParams(params))
    return;
  params.append("voucher", tr("Voucher"));
  params.append("debitMemo", tr("Debit Memo"));
  if (_selectDate->currentIndex()==1)
    params.append("olderDate", _onOrBeforeDate->date());
  else if (_selectDate->currentIndex()==2)
  {
    params.append("startDate", _startDate->date());
    params.append("endDate", _endDate->date());
  }
  if (_currid >= 0)
    params.append("curr_id", _currid);

  MetaSQLQuery mql = mqlLoad("apOpenItems", "selectpayments");
  q = mql.toQuery(params);
  _apopen->populate(q,true);
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void selectPayments::sPopulateMenu(QMenu *pMenu,QTreeWidgetItem *selected)
{
  QString status(selected->text(1));
  QAction *menuItem;
  XSqlQuery menu;
  menu.prepare( "SELECT apopen_status FROM apopen WHERE apopen_id=:apopen_id;");
  menu.bindValue(":apopen_id", _apopen->id());
  menu.exec();
  if (menu.first())
  {
    if(menu.value("apopen_status").toString() == "O")
    {
      menuItem = pMenu->addAction(tr("On Hold"), this, SLOT(sOnHold()));
      menuItem->setEnabled(_privileges->check("EditAPOpenItem"));
    }
    if(menu.value("apopen_status").toString() == "H")
    {
      menuItem = pMenu->addAction(tr("Open"), this, SLOT(sOpen()));
      menuItem->setEnabled(_privileges->check("EditAPOpenItem"));
    }
  }
}

void selectPayments::sOpen()
{
  XSqlQuery open;
  open.prepare("UPDATE apopen SET apopen_status = 'O' WHERE apopen_id=:apopen_id;");
  open.bindValue(":apopen_id", _apopen->id());
  open.exec();
  sFillList();
}

void selectPayments::sOnHold()
{
  XSqlQuery selectpayment;
  selectpayment.prepare("SELECT * FROM apselect WHERE apselect_apopen_id = :apopen_id;");
  selectpayment.bindValue(":apopen_id", _apopen->id());
  selectpayment.exec();
  if (selectpayment.first())
  {
    QMessageBox::critical( this, tr("Can not change Status"),
                          tr("<p>You cannot set this item as On Hold. "
                             "This Item is already selected for payment."));
    return;
  }

  XSqlQuery onhold;
  onhold.prepare("UPDATE apopen SET apopen_status = 'H' WHERE apopen_id=:apopen_id;");
  onhold.bindValue(":apopen_id", _apopen->id());
  onhold.exec();
  sFillList();
}
