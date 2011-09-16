/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspCheckRegister.h"

#include <QAction>
#include <QMenu>
#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include <openreports.h>
#include <parameter.h>
#include <xdateinputdialog.h>

#include "guiclient.h"
#include "mqlutil.h"
#include "storedProcErrorLookup.h"

dspCheckRegister::dspCheckRegister(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  connect(_query, SIGNAL(clicked()), this, SLOT(sFillList()));
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_check, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*)));
  connect(_vendRB,    SIGNAL(toggled(bool)), this, SLOT(sHandleButtons()));
  connect(_taxauthRB, SIGNAL(toggled(bool)), this, SLOT(sHandleButtons()));
  connect(_custRB,    SIGNAL(toggled(bool)), this, SLOT(sHandleButtons()));

  _bankaccnt->setType(XComboBox::APBankAccounts);

  _check->addColumn(tr("Void"),        _ynColumn,       Qt::AlignCenter, true,  "checkhead_void" );
  _check->addColumn(tr("Misc."),       _ynColumn,       Qt::AlignCenter, true,  "checkhead_misc" );
  _check->addColumn(tr("Prt'd"),       _ynColumn,       Qt::AlignCenter, true,  "checkhead_printed" );
  _check->addColumn(tr("Posted"),      _ynColumn,       Qt::AlignCenter, true,  "checkhead_posted" );
  _check->addColumn(tr("Chk./Vchr."),  _itemColumn,     Qt::AlignCenter, true,  "number" );
  _check->addColumn(tr("Recipient"),   -1,              Qt::AlignLeft,   true,  "description"   );
  _check->addColumn(tr("Check Date") , _dateColumn,     Qt::AlignCenter, true,  "checkdate" );
  _check->addColumn(tr("Amount"),      _moneyColumn,    Qt::AlignRight,  true,  "amount"  );
  _check->addColumn(tr("Currency"),    _currencyColumn, Qt::AlignRight,  true,  "currAbbr"  );
  _check->addColumn(tr("Base Amount"), _bigMoneyColumn, Qt::AlignRight,  true,  "base_amount"  );
  if (_metrics->boolean("ACHSupported") && _metrics->boolean("ACHEnabled"))
    _check->addColumn(tr("EFT Batch"), _orderColumn,    Qt::AlignRight,  true,  "checkhead_ach_batch");

  _check->sortByColumn(4);

  sHandleButtons();
  _recipGroup->setChecked(false);

  _total->setPrecision(omfgThis->moneyVal());
  if (omfgThis->singleCurrency())
  {
    _check->hideColumn("currAbbr");
    _totalCurr->hide();
  }
}

dspCheckRegister::~dspCheckRegister()
{
  // no need to delete child widgets, Qt does it all for us
}

void dspCheckRegister::languageChange()
{
  retranslateUi(this);
}

bool dspCheckRegister::setParams(ParameterList &pParams)
{
  if(!_dates->allValid())
  {
    QMessageBox::information( this, tr("Invalid Dates"),
			      tr("<p>Invalid dates specified. Please specify a "
				 "valid date range.") );
    _dates->setFocus();
    return false;
  }
  
  if(_recipGroup->isChecked())
  {
    pParams.append("recip", 100);
    if(_vendRB->isChecked())
    {
      pParams.append("recip_type_v", 100);
      if(_vend->isValid())
      {
        pParams.append("recip_id", _vend->id());
      }
    }
    if(_custRB->isChecked())
    {
      pParams.append("recip_type_c", 100);
      if(_cust->isValid())
      {
        pParams.append("recip_id", _cust->id());
      }
    }
    if(_taxauthRB->isChecked())
    {
      pParams.append("recip_type_t", 100);
      pParams.append("recip_id", _taxauth_2->id());
    }
  }

  if(_checkNumber->text() != "")
  {
    pParams.append("check_number", _checkNumber->text());
  }
  
  pParams.append("bankaccnt_id", _bankaccnt->id());
  _dates->appendValue(pParams);

  if(_showDetail->isChecked())
    pParams.append("showDetail");
  
  return true;
}

void dspCheckRegister::sPrint()
{
  ParameterList params;
  if (! setParams(params))
    return;

  orReport report("CheckRegister", params);
  if(report.isValid())
    report.print();
  else
    report.reportError(this);
}

void dspCheckRegister::sFillList()
{
  MetaSQLQuery mql = mqlLoad("checkRegister", "detail");

  ParameterList params;
  if (!setParams(params))
    return;
  
  q = mql.toQuery(params);
  _check->populate(q, true);
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  if(_showDetail->isChecked())
    _check->expandAll();
	
  QString tots("SELECT SUM(currToCurr(checkhead_curr_id, bankaccnt_curr_id,"
               "           checkhead_amount, checkhead_checkdate)) AS amount, "
               "       currConcat(bankaccnt_curr_id) AS currAbbr "
               "  FROM bankaccnt LEFT OUTER JOIN"
               "       checkhead ON ((bankaccnt_id=checkhead_bankaccnt_id)"
               "                AND  (checkhead_checkdate BETWEEN <? value(\"startDate\") ?> AND <? value(\"endDate\") ?> ) "
               "                AND  (NOT checkhead_void)"
               " <? if exists(\"check_number\") ?>"
               " AND   (CAST(checkhead_number AS text) ~ <? value(\"check_number\") ?>)"
               " <? endif ?>"
               " <? if exists(\"recip\") ?>"
               " <? if exists(\"recip_type_v\") ?>"
               " AND   (checkhead_recip_type = 'V' )"
               " <? endif ?>"
               " <? if exists(\"recip_type_c\") ?>"
               " AND   (checkhead_recip_type = 'C' )"
               " <? endif ?>"
               " <? if exists(\"recip_type_t\") ?>"
               " AND   (checkhead_recip_type = 'T' )"
               " <? endif ?>"
               " <? if exists(\"recip_id\") ?>"
               " AND   (checkhead_recip_id = <? value(\"recip_id\") ?> )"
               " <? endif ?>"
               " <? endif ?>)"
               "       LEFT OUTER JOIN "
               "       checkrecip ON ((checkhead_recip_id=checkrecip_id) "
               "                 AND  (checkhead_recip_type=checkrecip_type)) "
               " WHERE(bankaccnt_id=<? value(\"bankaccnt_id\") ?>)  "
               "GROUP BY bankaccnt_curr_id;" );
  MetaSQLQuery totm(tots);
  q = totm.toQuery(params);	// reused from above
  if(q.first())
  {
    _total->setDouble(q.value("amount").toDouble());
    _totalCurr->setText(q.value("currAbbr").toString());
  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void dspCheckRegister::sPopulateMenu( QMenu * pMenu )
{
  QAction *menuItem;

  if(_check->altId() == 1)
  {
    menuItem = pMenu->addAction(tr("Void Posted Check..."), this, SLOT(sVoidPosted()));
    menuItem->setEnabled(_privileges->check("VoidPostedAPCheck"));
  }
}

void dspCheckRegister::sVoidPosted()
{
  ParameterList params;

  XDateInputDialog newdlg(this, "", TRUE);
  params.append("label", tr("On what date did you void this check?"));
  newdlg.set(params);
  int returnVal = newdlg.exec();
  if (returnVal == XDialog::Accepted)
  {
    QDate voidDate = newdlg.getDate();
    q.prepare("SELECT voidPostedCheck(:check_id, fetchJournalNumber('AP-CK'),"
	      "                         DATE :voidDate) AS result;");
    q.bindValue(":check_id", _check->id());
    q.bindValue(":voidDate", voidDate);
    q.exec();
    if (q.first())
    {
      int result = q.value("result").toInt();
      if (result < 0)
      {
	systemError(this, storedProcErrorLookup("voidPostedCheck", result),
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
  sFillList();
}

void dspCheckRegister::sHandleButtons()
{
  if (_vendRB->isChecked())
  {
    _widgetStack->setCurrentIndex(0);
  }
  else if (_custRB->isChecked())
  {
    _widgetStack->setCurrentIndex(1);
  }
  else
  {
    _widgetStack->setCurrentIndex(2);
  }
}
