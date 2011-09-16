/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "applyDiscount.h"
#include "voucher.h"
#include "miscVoucher.h"

#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

applyDiscount::applyDiscount(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_buttonBox, SIGNAL(accepted()), this, SLOT(sApply()));
  connect(_buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

  _discprcnt->setPrecision(omfgThis->percentVal());

  _apopenid = -1;
  adjustSize();
}

applyDiscount::~applyDiscount()
{
  // no need to delete child widgets, Qt does it all for us
}

void applyDiscount::languageChange()
{
  retranslateUi(this);
}

enum SetResponse applyDiscount::set( const ParameterList & pParams )
{
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("apopen_id", &valid);
  if (valid)
  {
    _apopenid = param.toInt();
    populate();
  }

  param = pParams.value("curr_id", &valid);
  if (valid)
    _amount->setId(param.toInt());

  param = pParams.value("amount", &valid);
  if (valid)
    _amount->setLocalValue(param.toDouble());

  
  return NoError;
}

void applyDiscount::sApply()
{
  accept();
}

void applyDiscount::populate()
{
  q.prepare("SELECT (vend_number|| '-' || vend_name) as f_vend,"
            "       CASE WHEN (apopen_doctype='V') THEN 'Voucher'"
            "            WHEN (apopen_doctype='D') THEN 'Debit Memo'"
            "            ELSE apopen_doctype"
            "       END AS f_doctype,"
            "       apopen_docnumber,"
            "       apopen_docdate, "
            "       (terms_code|| '-' || terms_descrip) AS f_terms,"
            "       determineDiscountDate(apopen_terms_id, apopen_docdate) AS discdate,"
            "       terms_discprcnt,"
            "       apopen_amount, apopen_discountable_amount, apopen_curr_id, applied, "
            "       noNeg(apopen_discountable_amount *"
            "             CASE WHEN (CURRENT_DATE <= (apopen_docdate + terms_discdays)) THEN terms_discprcnt"
            "             ELSE 0.0 END - applied) AS amount,"
            "       ((apopen_docdate + terms_discdays) < CURRENT_DATE) AS past"
            "  FROM apopen LEFT OUTER JOIN terms ON (apopen_terms_id=terms_id),"
            "       vend, "
            "       (SELECT COALESCE(SUM(apapply_amount),0) AS applied"
            "          FROM apapply, apopen"
            "         WHERE ((apapply_target_apopen_id=:apopen_id)"
            "           AND  (apapply_source_apopen_id=apopen_id)"
            "           AND  (apopen_discount)) ) AS data"
            " WHERE ((apopen_vend_id=vend_id)"
            "   AND  (apopen_id=:apopen_id)); ");
  q.bindValue(":apopen_id", _apopenid);
  q.exec();

  if(q.first())
  {
    _vend->setText(q.value("f_vend").toString());

    _doctype->setText(q.value("f_doctype").toString());
    _docnum->setText(q.value("apopen_docnumber").toString());
    _docdate->setDate(q.value("apopen_docdate").toDate());

    _terms->setText(q.value("f_terms").toString());
    _discdate->setDate(q.value("discdate").toDate());

    if(q.value("past").toBool())
    {
      QPalette tmpPalette = _discdate->palette();
      tmpPalette.setColor(QPalette::HighlightedText, namedColor("error"));
      _discdate->setPalette(tmpPalette);
      _discdate->setForegroundRole(QPalette::HighlightedText);  // why doesn't the date turn ERROR?
      _discdateLit->setPalette(tmpPalette);
      _discdateLit->setForegroundRole(QPalette::HighlightedText);
    }

    _discprcnt->setDouble(q.value("terms_discprcnt").toDouble() * 100);

    _owed->setLocalValue(q.value("apopen_amount").toDouble());
    _discountableOwed->setLocalValue(q.value("apopen_discountable_amount").toDouble());
    _applieddiscounts->setLocalValue(q.value("applied").toDouble());

    _amount->set(q.value("amount").toDouble(),
		 q.value("apopen_curr_id").toInt(), 
		 q.value("apopen_docdate").toDate(), false);
  }
  else if (q.lastError().type() != QSqlError::NoError)
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
}

void applyDiscount::sViewVoucher()
{
  q.prepare("SELECT vohead_id, COALESCE(pohead_id, -1) AS pohead_id"
            "  FROM apopen, vohead LEFT OUTER JOIN pohead ON (vohead_pohead_id=pohead_id)"
            " WHERE((vohead_number=apopen_docnumber)"
            "   AND (apopen_id=:apopen_id));");
  q.bindValue(":apopen_id", _apopenid);
  q.exec();
  if(q.first())
  {
    ParameterList params;
    params.append("mode", "view");
    params.append("vohead_id", q.value("vohead_id").toInt());
  
    if (q.value("pohead_id").toInt() == -1)
    {
      miscVoucher *newdlg = new miscVoucher();
      newdlg->set(params);
      omfgThis->handleNewWindow(newdlg);
    }
    else
    {
      voucher *newdlg = new voucher();
      newdlg->set(params);
      omfgThis->handleNewWindow(newdlg);
    }
  }
  else
    systemError( this, q.lastError().databaseText(), __FILE__, __LINE__);
}


