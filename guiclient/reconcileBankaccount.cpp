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

#include <parameter.h>

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
    
    _receipts->addColumn(tr("Cleared"),   _ynColumn * 2, Qt::AlignCenter );
    _receipts->addColumn(tr("Date"),        _dateColumn, Qt::AlignCenter );
    _receipts->addColumn(tr("Doc. Number"), _itemColumn, Qt::AlignLeft   );
    _receipts->addColumn(tr("Notes"),                -1, Qt::AlignLeft   );
    _receipts->addColumn(tr("Amount"),  _bigMoneyColumn, Qt::AlignRight  );
    
    _checks->addColumn(tr("Cleared"),   _ynColumn * 2, Qt::AlignCenter , true, "cleared");
    _checks->addColumn(tr("Date"),        _dateColumn, Qt::AlignCenter , true, "transdate");
    _checks->addColumn(tr("Doc. Number"), _itemColumn, Qt::AlignLeft   , true, "docnumber");
    _checks->addColumn(tr("Notes"),                -1, Qt::AlignLeft   , true, "notes");
    _checks->addColumn(tr("Amount"),  _bigMoneyColumn, Qt::AlignRight  , true, "amount");

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

  double begBal = _openBal->localValue();
  double endBal = _endBal->localValue();

  // calculate cleared balance
  q.prepare("SELECT round(:endBal - (:begBal + COALESCE(SUM(amount),0.0)), 2) AS diff_value"
            "  FROM ( SELECT currToLocal(bankaccnt_curr_id, gltrans_amount * -1, gltrans_date) AS amount"
            "           FROM bankaccnt, gltrans, bankrecitem"
            "          WHERE ((gltrans_accnt_id=bankaccnt_accnt_id)"
            "            AND (bankrecitem_source='GL')"
            "            AND (bankrecitem_source_id=gltrans_id)"
            "            AND (bankrecitem_bankrec_id=:bankrecid)"
            "            AND (bankrecitem_cleared)"
            "            AND (NOT gltrans_rec)"
            "            AND (bankaccnt_id=:bankaccntid) ) "
            "          UNION ALL"
            "         SELECT CASE WHEN(bankadjtype_iscredit=true) THEN (bankadj_amount * -1) ELSE bankadj_amount END AS amount"
            "           FROM bankadj, bankadjtype, bankrecitem"
            "          WHERE ( (bankrecitem_source='AD')"
            "            AND (bankrecitem_source_id=bankadj_id)"
            "            AND (bankrecitem_bankrec_id=:bankrecid)"
            "            AND (bankrecitem_cleared)"
            "            AND (bankadj_bankadjtype_id=bankadjtype_id)"
            "            AND (NOT bankadj_posted)"
            "            AND (bankadj_bankaccnt_id=:bankaccntid) ) ) AS data;");
  q.bindValue(":bankaccntid", _bankaccnt->id());
  q.bindValue(":bankrecid", _bankrecid);
  q.bindValue(":endBal", endBal);
  q.bindValue(":begBal", begBal);
  q.bindValue(":curr_id",   _currency->id());
  q.bindValue(":effective", _startDate->date());
  q.bindValue(":expires",   _endDate->date());
  q.exec();
  if(!q.exec() || !q.first())
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  if(q.value("diff_value").toDouble() != 0.0)
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
   currency) and the bankadj table (in the bank account's currency).
*/
void reconcileBankaccount::populate()
{
  qApp->setOverrideCursor(QCursor(Qt::WaitCursor));

  double begBal = _openBal->localValue();
  double endBal = _endBal->localValue();

  int currid = -1;

  // fill receipts list
  currid = _receipts->id();
  _receipts->clear();
  q.prepare("SELECT gltrans_id AS id, 1 AS altid,"
            "       jrnluse_use AS use, gltrans_journalnumber AS jrnlnum,"
            "       COALESCE(date(jrnluse_date), gltrans_date) AS f_jrnldate,"
            "       COALESCE(bankrecitem_cleared, FALSE) AS cleared,"
            "       gltrans_date AS f_date,"
            "       gltrans_docnumber AS docnumber,"
            "       gltrans_notes AS notes,"
            "       currToLocal(bankaccnt_curr_id, gltrans_amount, gltrans_date) * -1 AS amount,"
            "       COALESCE(date(jrnluse_date), gltrans_date) AS jrnldate,"
            "       gltrans_date AS sortdate "
            "  FROM (bankaccnt CROSS JOIN gltrans) LEFT OUTER JOIN bankrecitem "
            "    ON ((bankrecitem_source='GL') AND (bankrecitem_source_id=gltrans_id)"
            "        AND (bankrecitem_bankrec_id=:bankrecid)) "
            "       LEFT OUTER JOIN jrnluse ON (jrnluse_number=gltrans_journalnumber AND jrnluse_use='C/R')"
            " WHERE ((gltrans_accnt_id=bankaccnt_accnt_id)"
            "   AND (NOT gltrans_deleted) "
            "   AND (NOT gltrans_rec)"
            "   AND (gltrans_amount < 0)"
            "   AND (bankaccnt_id=:bankaccntid) ) "
            " UNION ALL "
            "SELECT bankadj_id AS id, 2 AS altid,"
            "       '' AS use, NULL AS jrnlnum, bankadj_date AS f_jrnldate,"
            "       COALESCE(bankrecitem_cleared, FALSE) AS cleared,"
            "       bankadj_date AS f_date,"
            "       bankadj_docnumber AS docnumber,"
            "       bankadjtype_name AS notes,"
            "       (CASE WHEN(bankadjtype_iscredit=true) THEN (bankadj_amount * -1) ELSE bankadj_amount END) AS amount,"
            "       bankadj_date AS jrnldate,"
            "       bankadj_date AS sortdate "
            "  FROM (bankadjtype CROSS JOIN bankadj) "
            "               LEFT OUTER JOIN bankrecitem ON ((bankrecitem_source='AD') "
            "                 AND (bankrecitem_source_id=bankadj_id) "
            "                 AND (bankrecitem_bankrec_id=:bankrecid)) "
            " WHERE ( (((bankadjtype_iscredit=false) AND (bankadj_amount > 0)) OR ((bankadjtype_iscredit=true) AND (bankadj_amount < 0))) "
            "   AND (bankadj_bankadjtype_id=bankadjtype_id) "
            "   AND (NOT bankadj_posted) "
            "   AND (bankadj_bankaccnt_id=:bankaccntid) ) "
            "ORDER BY jrnldate, jrnlnum, sortdate; ");
  q.bindValue(":bankaccntid", _bankaccnt->id());
  q.bindValue(":bankrecid", _bankrecid);
  q.exec();
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  int jrnlnum = 0;
  XTreeWidgetItem * parent = 0;
  XTreeWidgetItem * lastChild = 0;
  XTreeWidgetItem * last = 0;
  bool cleared = TRUE;
  double amount = 0.0;
  bool amountNull = true;
  while (q.next())
  {
    if(q.value("use").toString() == "C/R")
    {
      if(q.value("jrnlnum").toInt() != jrnlnum || (0 == parent))
      {
        if(parent != 0)
        {
          parent->setText(0, (cleared ? tr("Yes") : tr("No")));
          parent->setText(4, amountNull ? tr("?????") : formatMoney(amount));
        }
        jrnlnum = q.value("jrnlnum").toInt();
        last = new XTreeWidgetItem( _receipts, last,
          jrnlnum, 3, "", formatDate(q.value("f_jrnldate").toDate()), q.value("jrnlnum"));
        parent = last;
        cleared = true;
        amount = 0.0;
	amountNull = true;
        lastChild = 0;
      }
      cleared = (cleared && q.value("cleared").toBool());
      amount += q.value("amount").toDouble();
      amountNull = q.value("amount").isNull();
      
      lastChild = new XTreeWidgetItem( parent, lastChild,
        q.value("id").toInt(), q.value("altid").toInt(),
        (q.value("cleared").toBool() ? tr("Yes") : tr("No")),
        formatDate(q.value("f_date").toDate()), q.value("docnumber"),
        q.value("notes"),
	q.value("amount").isNull() ? tr("?????") : formatMoney(q.value("amount").toDouble()) );
    }
    else
    {
      if(parent != 0)
      {
        parent->setText(0, (cleared ? tr("Yes") : tr("No")));
        parent->setText(4, formatMoney(amount));
      }
      parent = 0;
      cleared = true;
      amount = 0.0;
      amountNull = true;
      lastChild = 0;
      last = new XTreeWidgetItem( _receipts, last,
        q.value("id").toInt(), q.value("altid").toInt(),
        (q.value("cleared").toBool() ? tr("Yes") : tr("No")),
        formatDate(q.value("f_date").toDate()), q.value("docnumber"),
        q.value("notes"),
	q.value("amount").isNull() ? tr("?????") : formatMoney(q.value("amount").toDouble()) );
    }
  }
  if(parent != 0)
  {
    parent->setText(0, (cleared ? tr("Yes") : tr("No")));
    parent->setText(4, amountNull ? tr("?????") : formatMoney(amount));
  }
  if(currid != -1)
    _receipts->setCurrentItem(_receipts->topLevelItem(currid));
  if(_receipts->currentItem())
    _receipts->scrollToItem(_receipts->currentItem());


  // fill receipts cleared value
  q.prepare("SELECT COALESCE(SUM(amount),0.0) AS cleared_amount"
            "  FROM ( SELECT currToLocal(bankaccnt_curr_id, gltrans_amount * -1, gltrans_date) AS amount"
            "           FROM bankaccnt, gltrans, bankrecitem"
            "          WHERE ((gltrans_accnt_id=bankaccnt_accnt_id)"
            "            AND (bankrecitem_source='GL')"
            "            AND (bankrecitem_source_id=gltrans_id)"
            "            AND (bankrecitem_bankrec_id=:bankrecid)"
            "            AND (bankrecitem_cleared)"
            "            AND (NOT gltrans_deleted)"
            "            AND (NOT gltrans_rec)"
            "            AND (gltrans_amount < 0)"
            "            AND (bankaccnt_id=:bankaccntid) ) "
            "          UNION ALL"
            "         SELECT CASE WHEN(bankadjtype_iscredit=true) THEN (bankadj_amount * -1) ELSE bankadj_amount END AS amount"
            "           FROM bankrecitem, bankadj, bankadjtype "
            "          WHERE ( (bankrecitem_source='AD')"
            "            AND (bankrecitem_source_id=bankadj_id)"
            "            AND (bankrecitem_bankrec_id=:bankrecid)"
            "            AND (bankrecitem_cleared)"
            "            AND (bankadj_bankadjtype_id=bankadjtype_id)"
            "            AND (NOT bankadj_posted)"
            "            AND (((bankadjtype_iscredit=false) AND (bankadj_amount > 0)) OR (bankadjtype_iscredit=true AND (bankadj_amount < 0))) "
            "            AND (bankadj_bankaccnt_id=:bankaccntid) ) ) AS data;");
  q.bindValue(":bankaccntid", _bankaccnt->id());
  q.bindValue(":bankrecid", _bankrecid);
  q.exec();
  if (q.first())
    _clearedReceipts->setDouble(q.value("cleared_amount").toDouble());
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  // fill checks list
  currid = _checks->id();
  _checks->clear();
  q.prepare("SELECT gltrans_id AS id, 1 AS altid,"
            "       COALESCE(bankrecitem_cleared, FALSE) AS cleared,"
            "       gltrans_date AS transdate,"
            "       gltrans_docnumber AS docnumber,"
            "       gltrans_notes AS notes,"
            "       currToLocal(bankaccnt_curr_id, gltrans_amount, gltrans_date) AS amount,"
            "       gltrans_date AS sortdate, "
            "       'curr' AS amount_xtnumericrole "
            "  FROM (bankaccnt CROSS JOIN gltrans) LEFT OUTER JOIN bankrecitem "
            "    ON ((bankrecitem_source='GL') AND (bankrecitem_source_id=gltrans_id)"
            "        AND (bankrecitem_bankrec_id=:bankrecid)) "
            " WHERE ((gltrans_accnt_id=bankaccnt_accnt_id)"
            "   AND (NOT gltrans_deleted)"
            "   AND (NOT gltrans_rec)"
            "   AND (gltrans_amount > 0)"
            "   AND (bankaccnt_id=:bankaccntid) ) "
            " UNION ALL "
            "SELECT bankadj_id AS id, 2 AS altid,"
            "       COALESCE(bankrecitem_cleared, FALSE) AS cleared,"
            "       bankadj_date AS transdate,"
            "       bankadj_docnumber AS docnumber,"
            "       bankadjtype_name AS notes,"
            "       CASE WHEN(bankadjtype_iscredit=false) THEN (bankadj_amount * -1) ELSE bankadj_amount END AS amount,"
            "       bankadj_date AS sortdate, "
            "       'curr' AS amount_xtnumericrole "
            "  FROM (bankadjtype CROSS JOIN bankadj) "
            "               LEFT OUTER JOIN bankrecitem ON ((bankrecitem_source='AD') "
            "                 AND (bankrecitem_source_id=bankadj_id) "
            "                 AND (bankrecitem_bankrec_id=:bankrecid)) "
            " WHERE ( (((bankadjtype_iscredit=true) AND (bankadj_amount > 0)) OR ((bankadjtype_iscredit=false) AND (bankadj_amount < 0))) "
            "   AND (bankadj_bankadjtype_id=bankadjtype_id) "
            "   AND (NOT bankadj_posted) "
            "   AND (bankadj_bankaccnt_id=:bankaccntid) ) "
            "ORDER BY sortdate; ");
  q.bindValue(":bankaccntid", _bankaccnt->id());
  q.bindValue(":bankrecid", _bankrecid);
  q.exec();
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
  _checks->populate(q, TRUE);

  if(currid != -1)
    _checks->setCurrentItem(_checks->topLevelItem(currid));
  if(_checks->currentItem())
    _checks->scrollToItem(_checks->currentItem());

  // fill checks cleared value
  q.prepare("SELECT COALESCE(SUM(amount),0.0) AS cleared_amount"
            "  FROM ( SELECT currToLocal(bankaccnt_curr_id, gltrans_amount, gltrans_date) AS amount"
            "           FROM bankaccnt, gltrans, bankrecitem"
            "          WHERE ((gltrans_accnt_id=bankaccnt_accnt_id)"
            "            AND (bankrecitem_source='GL')"
            "            AND (bankrecitem_source_id=gltrans_id)"
            "            AND (bankrecitem_bankrec_id=:bankrecid)"
            "            AND (bankrecitem_cleared)"
            "            AND (NOT gltrans_deleted)"
            "            AND (NOT gltrans_rec)"
            "            AND (gltrans_amount > 0)"
            "            AND (bankaccnt_id=:bankaccntid) ) "
            "          UNION ALL"
            "         SELECT CASE WHEN(bankadjtype_iscredit=false) THEN (bankadj_amount * -1) ELSE bankadj_amount END AS amount"
            "           FROM bankadj, bankadjtype, bankrecitem"
            "          WHERE ( (bankrecitem_source='AD')"
            "            AND (bankrecitem_source_id=bankadj_id)"
            "            AND (bankrecitem_bankrec_id=:bankrecid)"
            "            AND (bankrecitem_cleared)"
            "            AND (bankadj_bankadjtype_id=bankadjtype_id)"
            "            AND (NOT bankadj_posted)"
            "            AND (((bankadjtype_iscredit=true) AND (bankadj_amount > 0)) OR ((bankadjtype_iscredit=false) AND (bankadj_amount < 0)))"
            "            AND (bankadj_bankaccnt_id=:bankaccntid) ) ) AS data;");
  q.bindValue(":bankaccntid", _bankaccnt->id());
  q.bindValue(":bankrecid", _bankrecid);
  q.exec();
  if (q.first())
    _clearedChecks->setDouble(q.value("cleared_amount").toDouble());
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  // calculate cleared balance
  q.prepare("SELECT (COALESCE(SUM(amount),0.0) + :begBal) AS cleared_amount,"
            "       :endBal AS end_amount,"
            "       (:endBal - (:begBal + (COALESCE(SUM(amount),0.0)))) AS diff_amount,"
            "       round(:endBal - (:begBal + COALESCE(SUM(amount),0.0)), 2) AS diff_value"
            "  FROM ( SELECT currToLocal(bankaccnt_curr_id, gltrans_amount * -1, gltrans_date) AS amount"
            "           FROM bankaccnt, gltrans, bankrecitem"
            "          WHERE ((gltrans_accnt_id=bankaccnt_accnt_id)"
            "            AND (bankrecitem_source='GL')"
            "            AND (bankrecitem_source_id=gltrans_id)"
            "            AND (bankrecitem_bankrec_id=:bankrecid)"
            "            AND (bankrecitem_cleared)"
            "            AND (NOT gltrans_rec)"
            "            AND (bankaccnt_id=:bankaccntid) ) "
            "          UNION ALL"
            "         SELECT CASE WHEN(bankadjtype_iscredit=true) THEN (bankadj_amount * -1) ELSE bankadj_amount END AS amount"
            "           FROM bankadj, bankadjtype, bankrecitem"
            "          WHERE ( (bankrecitem_source='AD')"
            "            AND (bankrecitem_source_id=bankadj_id)"
            "            AND (bankrecitem_bankrec_id=:bankrecid)"
            "            AND (bankrecitem_cleared)"
            "            AND (bankadj_bankadjtype_id=bankadjtype_id)"
            "            AND (NOT bankadj_posted)"
            "            AND (bankadj_bankaccnt_id=:bankaccntid) ) ) AS data;");
  q.bindValue(":bankaccntid", _bankaccnt->id());
  q.bindValue(":bankrecid", _bankrecid);
  q.bindValue(":endBal", endBal);
  q.bindValue(":begBal", begBal);
  q.bindValue(":curr_id",   _currency->id());
  q.bindValue(":effective", _startDate->date());
  q.bindValue(":expires",   _endDate->date());
  q.exec();
  bool enableRec = FALSE;
  if(q.first())
  {
    _clearBal->setDouble(q.value("cleared_amount").toDouble());
    _endBal2->setDouble(q.value("end_amount").toDouble());
    _diffBal->setDouble(q.value("diff_amount").toDouble());

    QString stylesheet;

    if(q.value("diff_value").toDouble() == 0.0)
    {
      if(_startDate->isValid() && _endDate->isValid())
        enableRec = TRUE;
    }
    else
      stylesheet = QString("* { color: %1; }").arg(namedColor("error").name());

    _diffBal->setStyleSheet(stylesheet);
  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
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
  if(item->altId() == 3)
  {
    setto = item->text(0) == tr("No");
    for (int i = 0; i < item->childCount(); i++)
    {
      child = item->child(i);
      if(child->text(0) != (setto ? tr("Yes") : tr("No")))
      {
        q.prepare("SELECT toggleBankrecCleared(:bankrecid, :source, :sourceid) AS cleared");
        q.bindValue(":bankrecid", _bankrecid);
        q.bindValue(":sourceid", child->id());
        if(child->altId()==1)
          q.bindValue(":source", "GL");
        else if(child->altId()==2)
          q.bindValue(":source", "AD");
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
    q.prepare("SELECT toggleBankrecCleared(:bankrecid, :source, :sourceid) AS cleared");
    q.bindValue(":bankrecid", _bankrecid);
    q.bindValue(":sourceid", item->id());
    if(item->altId()==1)
      q.bindValue(":source", "GL");
    else if(item->altId()==2)
      q.bindValue(":source", "AD");
    q.exec();
    if(q.first())
    {
      item->setText(0, (q.value("cleared").toBool() ? tr("Yes") : tr("No") ));

      item = (XTreeWidgetItem*)item->QTreeWidgetItem::parent();
      if(item != 0 && item->altId() == 3)
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

  q.prepare("SELECT toggleBankrecCleared(:bankrecid, :source, :sourceid) AS cleared");
  q.bindValue(":bankrecid", _bankrecid);
  q.bindValue(":sourceid", item->id());
  if(item->altId()==1)
    q.bindValue(":source", "GL");
  else if(item->altId()==2)
    q.bindValue(":source", "AD");
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

