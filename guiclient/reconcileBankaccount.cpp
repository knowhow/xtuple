/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "reconcileBankaccount.h"

#include <QApplication>
#include <QCursor>
#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include <metasql.h>
#include <parameter.h>

#include "mqlutil.h"
#include "bankAdjustment.h"
#include "storedProcErrorLookup.h"

reconcileBankaccount::reconcileBankaccount(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
    setupUi(this);

    connect(_addAdjustment, SIGNAL(clicked()),  this, SLOT(sAddAdjustment()));
    connect(_bankaccnt, SIGNAL(newID(int)),     this, SLOT(sBankaccntChanged()));
    connect(_cancel,	SIGNAL(clicked()),      this, SLOT(sCancel()));
    connect(_checks,    SIGNAL(itemDoubleClicked(QTreeWidgetItem*, int)), this, SLOT(sChecksToggleCleared()));
    connect(_endBal,	SIGNAL(lostFocus()),    this, SLOT(populate()));
    connect(_openBal,	SIGNAL(lostFocus()),    this, SLOT(populate()));
    connect(_receipts,	SIGNAL(itemDoubleClicked(QTreeWidgetItem*, int)), this, SLOT(sReceiptsToggleCleared()));
    connect(_reconcile,	SIGNAL(clicked()),      this, SLOT(sReconcile()));
    connect(_save,	    SIGNAL(clicked()),      this, SLOT(sSave()));
    connect(_update,	SIGNAL(clicked()),      this, SLOT(populate()));
    connect(_startDate, SIGNAL(newDate(QDate)), this, SLOT(sDateChanged()));
    connect(_endDate,   SIGNAL(newDate(QDate)), this, SLOT(sDateChanged()));
    connect(_checks,    SIGNAL(currentItemChanged(XTreeWidgetItem*, XTreeWidgetItem*)), this, SLOT(sChecksCloseEdit(XTreeWidgetItem*,XTreeWidgetItem*)));
    connect(_checks,    SIGNAL(itemClicked(XTreeWidgetItem*, int)), this, SLOT(sChecksOpenEdit(XTreeWidgetItem*, int)));
    connect(_receipts,  SIGNAL(currentItemChanged(XTreeWidgetItem*, XTreeWidgetItem*)), this, SLOT(sReceiptsCloseEdit(XTreeWidgetItem*,XTreeWidgetItem*)));
    connect(_receipts,  SIGNAL(itemClicked(XTreeWidgetItem*, int)), this, SLOT(sReceiptsOpenEdit(XTreeWidgetItem*, int)));

    _receipts->addColumn(tr("Cleared"),       _ynColumn * 2, Qt::AlignCenter );
    _receipts->addColumn(tr("Date"),            _dateColumn, Qt::AlignCenter );
    _receipts->addColumn(tr("Doc. Type"),     _ynColumn * 2, Qt::AlignCenter );
    _receipts->addColumn(tr("Doc. Number"),     _itemColumn, Qt::AlignLeft   );
    _receipts->addColumn(tr("Notes"),                    -1, Qt::AlignLeft   );
    _receipts->addColumn(tr("Currency"),    _currencyColumn, Qt::AlignCenter );
    _receipts->addColumn(tr("Exch. Rate"),  _bigMoneyColumn, Qt::AlignRight  );
    _receipts->addColumn(tr("Base Amount"), _bigMoneyColumn, Qt::AlignRight  );
    _receipts->addColumn(tr("Amount"),      _bigMoneyColumn, Qt::AlignRight  );
    
    _checks->addColumn(tr("Cleared"),       _ynColumn * 2, Qt::AlignCenter , true, "cleared");
    _checks->addColumn(tr("Date"),            _dateColumn, Qt::AlignCenter , true, "transdate");
    _checks->addColumn(tr("Doc. Type"),     _ynColumn * 2, Qt::AlignCenter , true, "doc_type");
    _checks->addColumn(tr("Doc. Number"),     _itemColumn, Qt::AlignLeft   , true, "doc_number");
    _checks->addColumn(tr("Notes"),                    -1, Qt::AlignLeft   , true, "notes");
    _checks->addColumn(tr("Currency"),    _currencyColumn, Qt::AlignCenter , true, "doc_curr");
    _checks->addColumn(tr("Exch. Rate"),  _bigMoneyColumn, Qt::AlignRight  , true, "doc_exchrate");
    _checks->addColumn(tr("Base Amount"), _bigMoneyColumn, Qt::AlignRight  , true, "base_amount");
    _checks->addColumn(tr("Amount"),      _bigMoneyColumn, Qt::AlignRight  , true, "amount");

    _clearedReceipts->setPrecision(omfgThis->moneyVal());
    _clearedChecks->setPrecision(omfgThis->moneyVal());
    _endBal2->setPrecision(omfgThis->moneyVal());
    _clearBal->setPrecision(omfgThis->moneyVal());
    _diffBal->setPrecision(omfgThis->moneyVal());
    
    _bankrecid = -1;	// do this before _bankaccnt->populate()
    _bankaccntid = -1;	// do this before _bankaccnt->populate()
    _datesAreOK = false;
    
    _bankaccnt->populate("SELECT bankaccnt_id,"
			 "       (bankaccnt_name || '-' || bankaccnt_descrip) "
			 "FROM bankaccnt "
			 "ORDER BY bankaccnt_name;");
    _currency->setLabel(_currencyLit);

    if (!_privileges->check("MaintainBankAdjustments"))
      _addAdjustment->setEnabled(FALSE);

    connect(omfgThis, SIGNAL(bankAdjustmentsUpdated(int, bool)), this, SLOT(populate()));
    connect(omfgThis, SIGNAL(checksUpdated(int, int, bool)), this, SLOT(populate()));
    connect(omfgThis, SIGNAL(cashReceiptsUpdated(int, bool)), this, SLOT(populate()));
    connect(omfgThis, SIGNAL(glSeriesUpdated()), this, SLOT(populate()));
}

reconcileBankaccount::~reconcileBankaccount()
{
    // no need to delete child widgets, Qt does it all for us
}

void reconcileBankaccount::languageChange()
{
    retranslateUi(this);
}

void reconcileBankaccount::sCancel()
{
  if(_bankrecid != -1)
  {
    q.prepare("SELECT count(*) AS num"
	      "  FROM bankrec"
	      " WHERE (bankrec_id=:bankrecid); ");
    q.bindValue(":bankrecid", _bankrecid);
    q.exec();
    if (q.first() && q.value("num").toInt() > 0)
    {
      if (QMessageBox::question(this, tr("Cancel Bank Reconciliation?"),
				tr("<p>Are you sure you want to Cancel this Bank "
				   "Reconciliation and delete all of the Cleared "
				   "notations for this time period?"),
				 QMessageBox::Yes,
				 QMessageBox::No | QMessageBox::Default) == QMessageBox::No)
      {
	return;
      }

      q.prepare( "SELECT deleteBankReconciliation(:bankrecid) AS result;" );
      q.bindValue(":bankrecid", _bankrecid);
      q.exec();
      if (q.first())
      {
	int result = q.value("result").toInt();
	if (result < 0)
	{
	  systemError(this, storedProcErrorLookup("deleteBankReconciliation", result),
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
  }
  close();
}

bool reconcileBankaccount::sSave(bool closeWhenDone)
{
  q.prepare("SELECT count(*) AS num"
            "  FROM bankrec"
            " WHERE (bankrec_id=:bankrecid); ");
  q.bindValue(":bankrecid", _bankrecid);
  q.exec();
  if (q.first() && q.value("num").toInt() > 0)
    q.prepare("UPDATE bankrec"
              "   SET bankrec_bankaccnt_id=:bankaccntid,"
              "       bankrec_opendate=:startDate,"
              "       bankrec_enddate=:endDate,"
              "       bankrec_openbal=:openbal,"
              "       bankrec_endbal=:endbal "
              " WHERE (bankrec_id=:bankrecid); ");
  else if (q.value("num").toInt() == 0)
    q.prepare("INSERT INTO bankrec "
              "(bankrec_id, bankrec_bankaccnt_id,"
              " bankrec_opendate, bankrec_enddate,"
              " bankrec_openbal, bankrec_endbal) "
              "VALUES "
              "(:bankrecid, :bankaccntid,"
              " :startDate, :endDate,"
              " :openbal, :endbal); ");
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return false;
  }

  q.bindValue(":bankrecid", _bankrecid);
  q.bindValue(":bankaccntid", _bankaccntid);
  q.bindValue(":startDate", _startDate->date());
  q.bindValue(":endDate", _endDate->date());
  q.bindValue(":openbal", _openBal->localValue());
  q.bindValue(":endbal", _endBal->localValue());
  q.exec();
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, tr("<p>There was an error creating records to reconcile "
			 "this account: <br><pre>%1</pre>")
			.arg(q.lastError().databaseText()), __FILE__, __LINE__);
    return false;
  }

  if (closeWhenDone)
    return close();

  return true;
}

void reconcileBankaccount::sReconcile()
{
  if(_bankrecid == -1)
  {
    QMessageBox::critical( this, tr("Cannot Reconcile Account"),
      tr("<p>There was an error trying to reconcile this account. "
         "Please contact your Systems Administrator.") );
    return;
  }

  if (!_startDate->isValid())
  {
    QMessageBox::warning( this, tr("Missing Opening Date"),
      tr("<p>No Opening Date was specified for this reconciliation. Please specify an Opening Date.") );
    _startDate->setFocus();
    return;
  }

  if (!_endDate->isValid())
  {
    QMessageBox::warning( this, tr("Missing Ending Date"),
      tr("<p>No Ending Date was specified for this reconciliation. Please specify an Ending Date.") );
    _endDate->setFocus();
    return;
  }

  if (_endDate->date() < _startDate->date())
  {
    QMessageBox::warning( this, tr("Invalid End Date"),
                           tr("The end date cannot be earlier than the start date.") );
    _endDate->setFocus();
    return;
  }

  if(!_datesAreOK)
  {
    QMessageBox::critical( this, tr("Dates already reconciled"),
                tr("The date range you have entered already has "
                   "reconciled dates in it. Please choose a different "
                   "date range.") );
    _startDate->setFocus();
    _datesAreOK = false;
    return;
  }

  double begBal = _openBal->localValue();
  double endBal = _endBal->localValue();

  // calculate cleared balance
  MetaSQLQuery mbal = mqlLoad("bankrec", "clearedbalance");
  ParameterList params;
  params.append("bankaccntid", _bankaccnt->id());
  params.append("bankrecid", _bankrecid);
  params.append("endBal", endBal);
  params.append("begBal", begBal);
  params.append("curr_id",   _currency->id());
  params.append("effective", _startDate->date());
  params.append("expires",   _endDate->date());
  XSqlQuery bal = mbal.toQuery(params);
  if(!bal.first())
  {
    systemError(this, bal.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  if(bal.value("diff_value").toDouble() != 0.0)
  {
    QMessageBox::critical( this, tr("Balances Do Not Match"),
      tr("The cleared amounts do not balance with the specified\n"
         "beginning and ending balances.\n"
         "Please correct this before continuing.") );
    return;
  }

  if (! sSave(false))
    return;

  q.prepare("SELECT postBankReconciliation(:bankrecid) AS result;");
  q.bindValue(":bankrecid", _bankrecid);
  q.exec();
  if (q.first())
  {
    int result = q.value("result").toInt();
    if (result < 0)
    {
      systemError(this, storedProcErrorLookup("postBankReconciliation", result),
		  __FILE__, __LINE__);
      return;
    }
    _bankrecid = -1;
    close();
  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

/* 
   Note that the SELECTs here are UNIONs of the gltrans table (in the base
   currency), sltrans table (in the base currency) and the bankadj table
   (in the bank account's currency).
*/
void reconcileBankaccount::populate()
{
  qApp->setOverrideCursor(QCursor(Qt::WaitCursor));

  double begBal = _openBal->localValue();
  double endBal = _endBal->localValue();

  int currid = -1;

  ParameterList params;
  params.append("bankaccntid", _bankaccnt->id());
  params.append("bankrecid", _bankrecid);

  // fill receipts list
  currid = _receipts->id();
  _receipts->clear();
  MetaSQLQuery mrcp = mqlLoad("bankrec", "receipts");
  XSqlQuery rcp = mrcp.toQuery(params);
  if (rcp.lastError().type() != QSqlError::NoError)
  {
    systemError(this, rcp.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  int jrnlnum = 0;
  XTreeWidgetItem * parent = 0;
  XTreeWidgetItem * lastChild = 0;
  XTreeWidgetItem * last = 0;
  bool cleared = TRUE;
  double amount = 0.0;
  bool amountNull = true;
  while (rcp.next())
  {
    if(rcp.value("use").toString() == "C/R")
    {
      if(rcp.value("jrnlnum").toInt() != jrnlnum || (0 == parent))
      {
        if(parent != 0)
        {
          parent->setText(0, (cleared ? tr("Yes") : tr("No")));
          parent->setText(8, amountNull ? tr("?????") : formatMoney(amount));
        }
        jrnlnum = rcp.value("jrnlnum").toInt();
        last = new XTreeWidgetItem( _receipts, last,
          jrnlnum, 9, "", formatDate(rcp.value("f_jrnldate").toDate()), tr("JS"), rcp.value("jrnlnum"));
        parent = last;
        cleared = true;
        amount = 0.0;
	amountNull = true;
        lastChild = 0;
      }
      cleared = (cleared && rcp.value("cleared").toBool());
      amount += rcp.value("amount").toDouble();
      amountNull = rcp.value("amount").isNull();
      
      lastChild = new XTreeWidgetItem( parent, lastChild,
        rcp.value("id").toInt(), rcp.value("altid").toInt(),
        (rcp.value("cleared").toBool() ? tr("Yes") : tr("No")),
        formatDate(rcp.value("f_date").toDate()), rcp.value("doc_type"), rcp.value("docnumber"),
        rcp.value("notes"),
        rcp.value("doc_curr"),
        rcp.value("doc_exchrate").isNull() ? tr("?????") : formatUOMRatio(rcp.value("doc_exchrate").toDouble()),
        rcp.value("base_amount").isNull() ? tr("?????") : formatMoney(rcp.value("base_amount").toDouble()),
        rcp.value("amount").isNull() ? tr("?????") : formatMoney(rcp.value("amount").toDouble()) );
    }
    else
    {
      if(parent != 0)
      {
        parent->setText(0, (cleared ? tr("Yes") : tr("No")));
        parent->setText(8, formatMoney(amount));
      }
      parent = 0;
      cleared = true;
      amount = 0.0;
      amountNull = true;
      lastChild = 0;
      last = new XTreeWidgetItem( _receipts, last,
        rcp.value("id").toInt(), rcp.value("altid").toInt(),
        (rcp.value("cleared").toBool() ? tr("Yes") : tr("No")),
        formatDate(rcp.value("f_date").toDate()), rcp.value("doc_type"), rcp.value("docnumber"),
        rcp.value("notes"),
        rcp.value("doc_curr"),
        rcp.value("doc_exchrate").isNull() ? tr("?????") : formatUOMRatio(rcp.value("doc_exchrate").toDouble()),
        rcp.value("base_amount").isNull() ? tr("?????") : formatMoney(rcp.value("base_amount").toDouble()),
        rcp.value("amount").isNull() ? tr("?????") : formatMoney(rcp.value("amount").toDouble()) );
    }
  }
  if(parent != 0)
  {
    parent->setText(0, (cleared ? tr("Yes") : tr("No")));
    parent->setText(8, amountNull ? tr("?????") : formatMoney(amount));
  }

  disconnect(_receipts, SIGNAL(itemChanged(XTreeWidgetItem*, int)), this, SLOT(sReceiptsItemChanged(XTreeWidgetItem*, int)));
  connect(_receipts, SIGNAL(itemChanged(XTreeWidgetItem*, int)), this, SLOT(sReceiptsItemChanged(XTreeWidgetItem*, int)));

  if(currid != -1)
    _receipts->setCurrentItem(_receipts->topLevelItem(currid));
  if(_receipts->currentItem())
    _receipts->scrollToItem(_receipts->currentItem());

  // fill checks list
  currid = _checks->id();
  _checks->clear();
  MetaSQLQuery mchk = mqlLoad("bankrec", "checks");
  XSqlQuery chk = mchk.toQuery(params);
  if (chk.lastError().type() != QSqlError::NoError)
  {
    systemError(this, chk.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
  _checks->populate(chk, TRUE);

  disconnect(_checks, SIGNAL(itemChanged(XTreeWidgetItem*, int)), this, SLOT(sChecksItemChanged(XTreeWidgetItem*, int)));
  connect(_checks, SIGNAL(itemChanged(XTreeWidgetItem*, int)), this, SLOT(sChecksItemChanged(XTreeWidgetItem*, int)));

  if(currid != -1)
    _checks->setCurrentItem(_checks->topLevelItem(currid));
  if(_checks->currentItem())
    _checks->scrollToItem(_checks->currentItem());

  params.append("summary", true);

  // fill receipts cleared value
  rcp = mrcp.toQuery(params);
  if (rcp.first())
    _clearedReceipts->setDouble(rcp.value("cleared_amount").toDouble());
  else if (rcp.lastError().type() != QSqlError::NoError)
  {
    systemError(this, rcp.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  // fill checks cleared value
  chk = mchk.toQuery(params);
  if (chk.first())
    _clearedChecks->setDouble(chk.value("cleared_amount").toDouble());
  else if (chk.lastError().type() != QSqlError::NoError)
  {
    systemError(this, chk.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  // calculate cleared balance
  MetaSQLQuery mbal = mqlLoad("bankrec", "clearedbalance");
  params.append("endBal", endBal);
  params.append("begBal", begBal);
  params.append("curr_id",   _currency->id());
  params.append("effective", _startDate->date());
  params.append("expires",   _endDate->date());
  XSqlQuery bal = mbal.toQuery(params);
  bool enableRec = FALSE;
  if(bal.first())
  {
    _clearBal->setDouble(bal.value("cleared_amount").toDouble());
    _endBal2->setDouble(bal.value("end_amount").toDouble());
    _diffBal->setDouble(bal.value("diff_amount").toDouble());

    QString stylesheet;

    if(bal.value("diff_value").toDouble() == 0.0)
    {
      if(_startDate->isValid() && _endDate->isValid())
        enableRec = TRUE;
    }
    else
      stylesheet = QString("* { color: %1; }").arg(namedColor("error").name());

    _diffBal->setStyleSheet(stylesheet);
  }
  else if (bal.lastError().type() != QSqlError::NoError)
  {
    systemError(this, bal.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
  //_reconcile->setEnabled(enableRec);

  qApp->restoreOverrideCursor();
}

void reconcileBankaccount::sAddAdjustment()
{
  ParameterList params;
  params.append("mode", "new");

  params.append("bankaccnt_id", _bankaccnt->id());

  bankAdjustment *newdlg = new bankAdjustment();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void reconcileBankaccount::sReceiptsToggleCleared()
{
  XTreeWidgetItem *item = (XTreeWidgetItem*)_receipts->currentItem();
  XTreeWidgetItem *child = 0;
  bool setto = true;

  if(0 == item)
    return;

  _receipts->scrollToItem(item);
  if(item->altId() == 9)
  {
    setto = item->text(0) == tr("No");
    for (int i = 0; i < item->childCount(); i++)
    {
      child = item->child(i);
      if(child->text(0) != (setto ? tr("Yes") : tr("No")))
      {
        q.prepare("SELECT toggleBankrecCleared(:bankrecid, :source, :sourceid, :currrate) AS cleared");
        q.bindValue(":bankrecid", _bankrecid);
        q.bindValue(":sourceid", child->id());
        if(child->altId()==1)
          q.bindValue(":source", "GL");
        else if(child->altId()==2)
          q.bindValue(":source", "SL");
        else if(child->altId()==3)
          q.bindValue(":source", "AD");
        q.bindValue(":currrate", child->text(6).toDouble());
        q.exec();
        if(q.first())
          child->setText(0, (q.value("cleared").toBool() ? tr("Yes") : tr("No") ));
	else if (q.lastError().type() != QSqlError::NoError)
	{
	  systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
	  return;
	}
      }
    }
    item->setText(0, (setto ? tr("Yes") : tr("No")));
  }
  else
  {
    q.prepare("SELECT toggleBankrecCleared(:bankrecid, :source, :sourceid, :currrate) AS cleared");
    q.bindValue(":bankrecid", _bankrecid);
    q.bindValue(":sourceid", item->id());
    if(item->altId()==1)
      q.bindValue(":source", "GL");
    else if(item->altId()==2)
      q.bindValue(":source", "SL");
    else if(item->altId()==3)
      q.bindValue(":source", "AD");
    q.bindValue(":currrate", item->text(6).toDouble());
    q.exec();
    if(q.first())
    {
      item->setText(0, (q.value("cleared").toBool() ? tr("Yes") : tr("No") ));

      item = (XTreeWidgetItem*)item->QTreeWidgetItem::parent();
      if(item != 0 && item->altId() == 9)
      {
        setto = true;
	for (int i = 0; i < item->childCount(); i++)
        {
          setto = (setto && (item->child(i)->text(0) == tr("Yes")));
        }
        item->setText(0, (setto ? tr("Yes") : tr("No")));
      }
    }
    else
    {
      populate();
      if (q.lastError().type() != QSqlError::NoError)
      {
	systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
	return;
      }
    }
  }
}

void reconcileBankaccount::sChecksToggleCleared()
{
  XTreeWidgetItem *item = (XTreeWidgetItem*)_checks->currentItem();

  if(0 == item)
    return;

  _checks->scrollToItem(item);

  q.prepare("SELECT toggleBankrecCleared(:bankrecid, :source, :sourceid, :currrate) AS cleared");
  q.bindValue(":bankrecid", _bankrecid);
  q.bindValue(":sourceid", item->id());
  if(item->altId()==1)
    q.bindValue(":source", "GL");
  else if(item->altId()==2)
    q.bindValue(":source", "SL");
  else if(item->altId()==3)
    q.bindValue(":source", "AD");
  q.bindValue(":currrate", item->text(6).toDouble());
  q.exec();
  if(q.first())
    item->setText(0, (q.value("cleared").toBool() ? tr("Yes") : tr("No") ));
  else
  {
    populate();
    if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }
}

void reconcileBankaccount::sBankaccntChanged()
{
  if(_bankrecid != -1)
  {
    q.prepare("SELECT count(*) AS num"
	          "  FROM bankrecitem"
	          " WHERE (bankrecitem_bankrec_id=:bankrecid); ");
    q.bindValue(":bankrecid", _bankrecid);
    q.exec();
    if (q.first() && q.value("num").toInt() > 0)
    {
      if (QMessageBox::question(this, tr("Save Bank Reconciliation?"),
				                      tr("<p>Do you want to save this Bank Reconciliation?"),
				                QMessageBox::No,
				                QMessageBox::Yes | QMessageBox::Default) == QMessageBox::Yes)
      {
	    sSave(false);
      }
      else
	  {
        q.prepare( "SELECT deleteBankReconciliation(:bankrecid) AS result;" );
        q.bindValue(":bankrecid", _bankrecid);
        q.exec();
        if (q.first())
        {
	      int result = q.value("result").toInt();
	      if (result < 0)
	      {
	        systemError(this, storedProcErrorLookup("deleteBankReconciliation", result),
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
	}
  }

  _bankaccntid = _bankaccnt->id();
  XSqlQuery accntq;
  accntq.prepare("SELECT bankaccnt_curr_id "
            "FROM bankaccnt WHERE bankaccnt_id = :accntId;");
  accntq.bindValue(":accntId", _bankaccnt->id());
  accntq.exec();
  if (accntq.first())
    _currency->setId(accntq.value("bankaccnt_curr_id").toInt());
  else if (accntq.lastError().type() != QSqlError::NoError)
  {
    systemError(this, accntq.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  accntq.prepare("SELECT * FROM bankrec "
		 "WHERE ((bankrec_bankaccnt_id=:accntId)"
		 "  AND  (NOT bankrec_posted));");
  accntq.bindValue(":accntId", _bankaccnt->id());
  accntq.exec();
  if (accntq.first())
  {
    _bankrecid = accntq.value("bankrec_id").toInt();
    _startDate->setDate(accntq.value("bankrec_opendate").toDate(), true);
    _endDate->setDate(accntq.value("bankrec_enddate").toDate(), true);
    _openBal->setLocalValue(accntq.value("bankrec_openbal").toDouble());
    _endBal->setLocalValue(accntq.value("bankrec_endbal").toDouble());
  }
  else if (accntq.lastError().type() != QSqlError::NoError)
  {
    systemError(this, accntq.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
  else
  {
    accntq.prepare("SELECT NEXTVAL('bankrec_bankrec_id_seq') AS bankrec_id");
    accntq.exec();
    if (accntq.first())
      _bankrecid = accntq.value("bankrec_id").toInt();
    else if (accntq.lastError().type() != QSqlError::NoError)
    {
      systemError(this, accntq.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
    
    accntq.prepare("SELECT bankrec_enddate + 1 AS startdate, "
                   " bankrec_endbal AS openbal "
                   "FROM bankrec "
                   "WHERE (bankrec_bankaccnt_id=:accntId) "
                   "ORDER BY bankrec_enddate DESC "
                   "LIMIT 1");
    accntq.bindValue(":accntId", _bankaccnt->id());
    accntq.exec();
    if (accntq.first())
    {
      _startDate->setDate(accntq.value("startdate").toDate());
      _openBal->setLocalValue(accntq.value("openbal").toDouble());
    }
    else
    {
      _startDate->clear();
      _openBal->clear();
    }
    if (accntq.lastError().type() != QSqlError::NoError)
    {
      systemError(this, accntq.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }

  populate();
}

void reconcileBankaccount::sDateChanged()
{
  q.prepare("SELECT TRUE AS reconciled "
            "FROM bankrec "
            "WHERE ((bankrec_bankaccnt_id = :bankaccnt_id) "
            "AND (bankrec_posted) "
            "AND (bankrec_opendate <= :end_date) "
            "AND (bankrec_enddate >= :start_date)) "
            "GROUP BY bankrec_bankaccnt_id");
  q.bindValue(":bankaccnt_id", _bankaccnt->id());
  q.bindValue(":end_date", _endDate->date().toString(Qt::ISODate));
  q.bindValue(":start_date", _startDate->date().toString(Qt::ISODate));

  q.exec();
  if(q.first())
  {
    QMessageBox::critical( this, tr("Dates already reconciled"),
	        tr("The date range you have entered already has "
	           "reconciled dates in it. Please choose a different "
	           "date range.") );
    _startDate->setFocus();
    _datesAreOK = false;
    return;
  }
  else if(q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    _datesAreOK = false;
	return;
  }
  else
  {
	_datesAreOK = true;
  }
}

void reconcileBankaccount::sChecksOpenEdit(XTreeWidgetItem *item, const int col)
{
  if (col==6)
  {
    _checks->openPersistentEditor(item,col);
    _checks->editItem(item,col);
  }
}

void reconcileBankaccount::sChecksCloseEdit(XTreeWidgetItem * /*current*/, XTreeWidgetItem *previous)
{
  _checks->closePersistentEditor(previous,6);
}

void reconcileBankaccount::sChecksItemChanged(XTreeWidgetItem *item, const int col)
{
  // Only positive numbers allowed
  if (col==6)
  {
    if (item->data(col,Qt::EditRole).toDouble() < 0)
      item->setData(col,Qt::EditRole,0);
    else
      item->setData(col,Qt::EditRole,item->data(col,Qt::EditRole).toDouble());
  }
}

void reconcileBankaccount::sReceiptsOpenEdit(XTreeWidgetItem *item, const int col)
{
  if (col==6)
  {
    _receipts->openPersistentEditor(item,col);
    _receipts->editItem(item,col);
  }
}

void reconcileBankaccount::sReceiptsCloseEdit(XTreeWidgetItem * /*current*/, XTreeWidgetItem *previous)
{
  _receipts->closePersistentEditor(previous,6);
}

void reconcileBankaccount::sReceiptsItemChanged(XTreeWidgetItem *item, const int col)
{
  // Only positive numbers allowed
  if (col==6)
  {
    if (item->data(col,Qt::EditRole).toDouble() < 0)
      item->setData(col,Qt::EditRole,0);
    else
      item->setData(col,Qt::EditRole,item->data(col,Qt::EditRole).toDouble());
  }
}

