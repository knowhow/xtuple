/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "apCreditMemoApplication.h"

#include <QMessageBox>
#include <QSqlError>
#include <QVariant>
#include <QValidator>

#include "storedProcErrorLookup.h"

apCreditMemoApplication::apCreditMemoApplication(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_buttonBox, SIGNAL(accepted()), this, SLOT(sSave()));
  connect(_buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

  _vend->setReadOnly(TRUE);
  adjustSize();
}

apCreditMemoApplication::~apCreditMemoApplication()
{
    // no need to delete child widgets, Qt does it all for us
}

void apCreditMemoApplication::languageChange()
{
    retranslateUi(this);
}

enum SetResponse apCreditMemoApplication::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("sourceApopenid", &valid);
  if (valid)
    _sourceApopenid = param.toInt();

  param = pParams.value("targetApopenid", &valid);
  if (valid)
  {
    _targetApopenid = param.toInt();
    populate();
  }

  return NoError;
}

void apCreditMemoApplication::sSave()
{
  q.prepare("SELECT createAPCreditMemoApplication(:source, :target, "
	    "                                   :amount, :curr_id) AS result;");
  q.bindValue(":source",  _sourceApopenid);
  q.bindValue(":target",  _targetApopenid);
  q.bindValue(":amount",  _amountToApply->localValue());
  q.bindValue(":curr_id", _amountToApply->id());

  q.exec();
  if (q.first())
  {
    int result = q.value("result").toInt();
    if (result < 0)
    {
      systemError(this, storedProcErrorLookup("createApCreditMemoApplication",
					      result), __FILE__, __LINE__);
      return;
    }
  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  accept();
}

void apCreditMemoApplication::populate()
{
  q.prepare( "SELECT apopen_vend_id, apopen_docnumber, apopen_doctype,"
             "       apopen_docdate, apopen_duedate,"
             "       apopen_amount,"
             "       apopen_paid,"
             "       (apopen_amount - apopen_paid - COALESCE(selected,0.0) - COALESCE(prepared,0.0)) AS f_balance, "
	     "       apopen_curr_id "
             "  FROM apopen"
             "       LEFT OUTER JOIN (SELECT apopen_id AS selected_apopen_id,"
             "                             SUM(currToCurr(apselect_curr_id, apopen_curr_id, apselect_amount + apselect_discount, apselect_date)) AS selected"
             "                        FROM apselect JOIN apopen ON (apselect_apopen_id=apopen_id)"
             "                       WHERE (apopen_id=:apopen_id)"
             "                       GROUP BY apopen_id) AS sub1"
             "         ON (apopen_id=selected_apopen_id)"
             "       LEFT OUTER JOIN (SELECT apopen_id AS prepared_apopen_id,"
             "                               SUM(currToCurr(checkitem_curr_id, apopen_curr_id, checkitem_amount + checkitem_discount, checkitem_docdate)) AS prepared"
             "                          FROM checkhead JOIN checkitem ON (checkitem_checkhead_id=checkhead_id)"
             "                                     JOIN apopen ON (checkitem_apopen_id=apopen_id)"
             "                         WHERE ((NOT checkhead_posted)"
             "                           AND  (NOT checkhead_void)"
             "                           AND  (apopen_id=:apopen_id))"
             "                         GROUP BY apopen_id) AS sub2"
             "         ON (prepared_apopen_id=apopen_id)"
             " WHERE (apopen_id=:apopen_id);" );
  q.bindValue(":apopen_id", _targetApopenid);
  q.exec();
  if (q.first())
  {
    _vend->setId(q.value("apopen_vend_id").toInt());
    _docNumber->setText(q.value("apopen_docnumber").toString());
    _docType->setText(q.value("apopen_doctype").toString());
    _docDate->setDate(q.value("apopen_docdate").toDate(), true);
    _dueDate->setDate(q.value("apopen_duedate").toDate());
    _targetAmount->set(q.value("apopen_amount").toDouble(), 
		       q.value("apopen_curr_id").toInt(),
		       q.value("apopen_docdate").toDate(), false);
    _targetPaid->setLocalValue(q.value("apopen_paid").toDouble());
    _targetBalance->setLocalValue(q.value("f_balance").toDouble());
  }
  else if (q.lastError().type() != QSqlError::NoError)
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);

  q.prepare( "SELECT COALESCE(apcreditapply_curr_id,apopen_curr_id) AS curr_id,"
	     "       currToCurr(apopen_curr_id,"
	     "		        COALESCE(apcreditapply_curr_id,apopen_curr_id),"
	     "		        apopen_amount - apopen_paid, apopen_docdate) -"
	     "		   COALESCE(SUM(apcreditapply_amount), 0) AS available,"
             "       apopen_docdate "
             "FROM apopen LEFT OUTER JOIN apcreditapply ON (apcreditapply_source_apopen_id=apopen_id) "
             "WHERE (apopen_id=:apopen_id) "
             "GROUP BY apopen_amount, apopen_paid, apopen_docdate,"
	     "         apcreditapply_curr_id, apopen_curr_id;" );
  q.bindValue(":apopen_id", _sourceApopenid);
  q.exec();
  if (q.first())
  {
    _availableToApply->set(q.value("available").toDouble(), 
		           q.value("curr_id").toInt(),
		           q.value("apopen_docdate").toDate(), false);
  }
  else if (q.lastError().type() != QSqlError::NoError)
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);

  q.prepare( "SELECT currToCurr(apcreditapply_curr_id, :curr_id, "
	     "                  apcreditapply_amount, :effective) AS apcreditapply_amount "
             "FROM apcreditapply "
             "WHERE ( (apcreditapply_source_apopen_id=:source_apopen_id)"
             " AND (apcreditapply_target_apopen_id=:target_apopen_id) );" );
  q.bindValue(":source_apopen_id", _sourceApopenid);
  q.bindValue(":target_apopen_id", _targetApopenid);
  q.bindValue(":curr_id", _amountToApply->id());
  q.bindValue(":effective", _amountToApply->effective());
  q.exec();
  if (q.first())
    _amountToApply->setLocalValue(q.value("apcreditapply_amount").toDouble());
  else if (q.lastError().type() != QSqlError::NoError)
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
}
