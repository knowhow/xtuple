/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "arCreditMemoApplication.h"

#include <QMessageBox>
#include <QSqlError>
#include <QVariant>
#include <QValidator>

arCreditMemoApplication::arCreditMemoApplication(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_buttonBox, SIGNAL(accepted()), this, SLOT(sSave()));
  connect(_buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

  _cust->setReadOnly(TRUE);
  adjustSize();
}

arCreditMemoApplication::~arCreditMemoApplication()
{
  // no need to delete child widgets, Qt does it all for us
}

void arCreditMemoApplication::languageChange()
{
  retranslateUi(this);
}

enum SetResponse arCreditMemoApplication::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("sourceAropenid", &valid);
  if (valid)
    _sourceAropenid = param.toInt();

  param = pParams.value("targetAropenid", &valid);
  if (valid)
  {
    _targetAropenid = param.toInt();
    populate();
  }

  return NoError;
}

void arCreditMemoApplication::sSave()
{
  double amountToApply = _amountToApply->localValue();

  // check to make sure the amount being applied does not exceed
  // the balance due on the target item.
  q.prepare( "SELECT ROUND(currToCurr(aropen_curr_id, :curr_id,"
             "      (aropen_amount - aropen_paid - calcpendingarapplications(aropen_id)), current_date), 2) AS balance "
             "  FROM aropen "
             " WHERE (aropen_id=:aropen_id);");
  q.bindValue(":aropen_id", _targetAropenid);
  q.bindValue(":curr_id",   _amountToApply->id());
  q.exec();
  double targetBalance = 0.0;
  if(q.first())
    targetBalance = q.value("balance").toDouble();
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
  if(amountToApply > targetBalance)
  {
    QMessageBox::warning(this, tr("Invalid Application"),
      tr("You may not apply more than the balance due to this Document.") );
    return;
  }

  // check to make sure the mount being applied does not exceed
  // the remaining balance of the source item.
  q.prepare( "SELECT round((aropen_amount - aropen_paid) - "
	     "	COALESCE(SUM(currToCurr(arcreditapply_curr_id, "
	     "				aropen_curr_id, "
	     "				arcreditapply_amount, "
             "				aropen_docdate)), 0), 2) - COALESCE(prepared,0.0) - COALESCE(cashapplied,0.0) AS available "
             "FROM aropen LEFT OUTER JOIN arcreditapply "
             "  ON ((arcreditapply_source_aropen_id=aropen_id) "
             "  AND (arcreditapply_target_aropen_id<>:targetAropenid)) "
             "       LEFT OUTER JOIN (SELECT aropen_id AS prepared_aropen_id,"
             "                               SUM(checkitem_amount + checkitem_discount) AS prepared"
             "                          FROM checkhead JOIN checkitem ON (checkitem_checkhead_id=checkhead_id)"
             "                                     JOIN aropen ON (checkitem_aropen_id=aropen_id)"
             "                         WHERE ((NOT checkhead_posted)"
             "                           AND  (NOT checkhead_void))"
             "                         GROUP BY aropen_id) AS sub1"
             "         ON (prepared_aropen_id=aropen_id)"
             "       LEFT OUTER JOIN (SELECT aropen_id AS cash_aropen_id,"
             "                               SUM(cashrcptitem_amount + cashrcptitem_discount) * -1.0 AS cashapplied"
             "                          FROM cashrcpt JOIN cashrcptitem ON (cashrcptitem_cashrcpt_id=cashrcpt_id)"
             "                                     JOIN aropen ON (cashrcptitem_aropen_id=aropen_id)"
             "                         WHERE (NOT cashrcpt_posted)"
             "                         GROUP BY aropen_id ) AS sub2"
             "         ON (cash_aropen_id=aropen_id)"
             "WHERE (aropen_id=:sourceAropenid) "
             "GROUP BY aropen_amount, aropen_paid, prepared, cashapplied;" );
  q.bindValue(":sourceAropenid", _sourceAropenid);
  q.bindValue(":targetAropenid", _targetAropenid);
  q.exec();
  double sourceBalance = 0.0;
  if(q.first())
    sourceBalance = q.value("available").toDouble();
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
  if(amountToApply > sourceBalance)
  {
    QMessageBox::warning(this, tr("Invalid Application"),
      tr("You may not apply more than the amount available to apply for this Credit Memo.") );
    return;
  }

  q.prepare( "SELECT arcreditapply_id "
             "FROM arcreditapply "
             "WHERE ( (arcreditapply_source_aropen_id=:sourceAropenid)"
             " AND (arcreditapply_target_aropen_id=:targetAropenid) );" );
  q.bindValue(":sourceAropenid", _sourceAropenid);
  q.bindValue(":targetAropenid", _targetAropenid);
  q.exec();
  if (q.first())
  {
    int arcreditapplyid = q.value("arcreditapply_id").toInt();

    q.prepare( "UPDATE arcreditapply "
               "SET arcreditapply_amount=:arcreditapply_amount, "
	       "    arcreditapply_curr_id = :arcreditapply_curr_id "
               "WHERE (arcreditapply_id=:arcreditapply_id);" );
    q.bindValue(":arcreditapply_id", arcreditapplyid);
  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
  else
  {
    q.prepare( "INSERT INTO arcreditapply "
               "( arcreditapply_source_aropen_id, arcreditapply_target_aropen_id, "
	       " arcreditapply_amount, arcreditapply_curr_id ) "
               "VALUES "
               "( :sourceAropenid, :targetAropenid, "
	       "  :arcreditapply_amount, :arcreditapply_curr_id );" );
    q.bindValue(":sourceAropenid", _sourceAropenid);
    q.bindValue(":targetAropenid", _targetAropenid);
  }

  q.bindValue(":arcreditapply_amount", amountToApply);
  q.bindValue(":arcreditapply_curr_id", _amountToApply->id());
  q.exec();
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  accept();
}

void arCreditMemoApplication::populate()
{
  q.prepare( "SELECT aropen_cust_id, aropen_docnumber, aropen_doctype,"
             "       aropen_docdate, aropen_duedate, "
             "       aropen_amount, "
             "       aropen_paid, "
             "       calcpendingarapplications(aropen_id) AS pending, "
             "       (aropen_amount - aropen_paid - calcpendingarapplications(aropen_id)) AS f_balance, "
             "       aropen_curr_id "
             "FROM aropen "
             "WHERE (aropen_id=:aropen_id);" );
  q.bindValue(":aropen_id", _targetAropenid);
  q.exec();
  if (q.first())
  {
    _cust->setId(q.value("aropen_cust_id").toInt());
    _docNumber->setText(q.value("aropen_docnumber").toString());
    _docType->setText(q.value("aropen_doctype").toString());
    _docDate->setDate(q.value("aropen_docdate").toDate(), true);
    _dueDate->setDate(q.value("aropen_duedate").toDate());
    _targetAmount->set(q.value("aropen_amount").toDouble(),
		       q.value("aropen_curr_id").toInt(),
		       q.value("aropen_docdate").toDate(), false);
    _targetPaid->setLocalValue(q.value("aropen_paid").toDouble());
    _targetPending->setLocalValue(q.value("pending").toDouble());
    _targetBalance->setLocalValue(q.value("f_balance").toDouble());
  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  q.prepare( "SELECT COALESCE(arcreditapply_curr_id,aropen_curr_id) AS curr_id,"
             "       currToCurr(aropen_curr_id,"
             "                 COALESCE(arcreditapply_curr_id,aropen_curr_id),"
             "                 aropen_amount - aropen_paid, aropen_docdate) - "
             "		COALESCE(SUM(arcreditapply_amount), 0) - COALESCE(prepared,0.0) - COALESCE(cashapplied,0.0) AS available,"
             "       current_date AS docdate "
             "FROM aropen LEFT OUTER JOIN arcreditapply ON (arcreditapply_source_aropen_id=aropen_id) "
    	     "       LEFT OUTER JOIN (SELECT aropen_id AS prepared_aropen_id,"
             "                               SUM(checkitem_amount + checkitem_discount) AS prepared"
             "                          FROM checkhead JOIN checkitem ON (checkitem_checkhead_id=checkhead_id)"
             "                                     JOIN aropen ON (checkitem_aropen_id=aropen_id)"
             "                         WHERE ((NOT checkhead_posted)"
             "                           AND  (NOT checkhead_void))"
             "                         GROUP BY aropen_id) AS sub1 "
             "         ON (prepared_aropen_id=aropen_id)"
             "       LEFT OUTER JOIN (SELECT aropen_id AS cash_aropen_id,"
             "                               SUM(cashrcptitem_amount + cashrcptitem_discount) * -1.0 AS cashapplied"
             "                          FROM cashrcpt JOIN cashrcptitem ON (cashrcptitem_cashrcpt_id=cashrcpt_id)"
             "                                     JOIN aropen ON (cashrcptitem_aropen_id=aropen_id)"
             "                         WHERE ((NOT cashrcpt_posted)"
             "                           AND  (NOT cashrcpt_void))"
             "                         GROUP BY aropen_id ) AS sub2"
             "         ON (cash_aropen_id=aropen_id)"
             "WHERE (aropen_id=:aropen_id) "
             "GROUP BY aropen_amount, aropen_paid, aropen_docdate,"
                 "         arcreditapply_curr_id, aropen_curr_id, prepared, cashapplied;" );
  q.bindValue(":aropen_id", _sourceAropenid);
  q.exec();
  if (q.first())
  {
    _availableToApply->set(q.value("available").toDouble(),
		           q.value("curr_id").toInt(),
		           q.value("docdate").toDate(), false);
  }
  else if (q.lastError().type() != QSqlError::NoError)
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);

  q.prepare( "SELECT currToCurr(arcreditapply_curr_id, :curr_id, "
	     "                  arcreditapply_amount, :effective) AS arcreditapply_amount "
             "FROM arcreditapply "
             "WHERE ( (arcreditapply_source_aropen_id=:source_aropen_id)"
             " AND (arcreditapply_target_aropen_id=:target_aropen_id) );" );
  q.bindValue(":source_aropen_id", _sourceAropenid);
  q.bindValue(":target_aropen_id", _targetAropenid);
  q.bindValue(":curr_id", _amountToApply->id());
  q.bindValue(":effective", _amountToApply->effective());
  q.exec();
  if (q.first())
    _amountToApply->setLocalValue(q.value("arcreditapply_amount").toDouble());
  else if (q.lastError().type() != QSqlError::NoError)
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
}
