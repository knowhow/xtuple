/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "applyARDiscount.h"

#include <QSqlError>
#include <QVariant>

applyARDiscount::applyARDiscount(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_buttonBox, SIGNAL(accepted()), this, SLOT(sApply()));
  connect(_buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

  _discprcnt->setPrecision(omfgThis->percentVal());

  _aropenid = -1;
  adjustSize();
}

applyARDiscount::~applyARDiscount()
{
  // no need to delete child widgets, Qt does it all for us
}

void applyARDiscount::languageChange()
{
  retranslateUi(this);
}

enum SetResponse applyARDiscount::set( const ParameterList & pParams )
{
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("aropen_id", &valid);
  if (valid)
  {
    _aropenid = param.toInt();
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

void applyARDiscount::sApply()
{
  accept();
}

void applyARDiscount::populate()
{
  q.prepare("SELECT cust_name, aropen_docnumber, aropen_docdate,"
            "       CASE WHEN (aropen_doctype='I') THEN :invoice "
            "            WHEN (aropen_doctype='C') THEN :creditmemo "
            "            ELSE aropen_doctype "
            "       END AS f_doctype, "
            "       (terms_code || '-' || terms_descrip) AS f_terms, "
            "       determineDiscountDate(terms_id, aropen_docdate) AS discdate, "
            "       terms_discprcnt, applied, "
            "       aropen_amount,aropen_curr_id, "
            "       (determineDiscountDate(terms_id, aropen_docdate) < CURRENT_DATE) AS past "
            "FROM aropen LEFT OUTER JOIN terms ON (aropen_terms_id=terms_id) "
            "            JOIN custinfo ON (cust_id=aropen_cust_id), "
            "     ( SELECT COALESCE(SUM(arapply_applied),0) AS applied "
            "       FROM arapply, aropen "
            "       WHERE ((arapply_target_aropen_id=:aropen_id) "
            "         AND  (arapply_source_aropen_id=aropen_id) "
            "         AND  (aropen_discount)) ) AS data "
            "WHERE (aropen_id=:aropen_id);");
  q.bindValue(":aropen_id", _aropenid);
  q.bindValue(":invoice", tr("Invoice"));
  q.bindValue(":creditmemo", tr("Credit Memo"));
  q.exec();

  if(q.first())
  {
    _cust->setText(q.value("cust_name").toString());

    _doctype->setText(q.value("f_doctype").toString());
    _docnum->setText(q.value("aropen_docnumber").toString());
    _docdate->setDate(q.value("aropen_docdate").toDate());

    _terms->setText(q.value("f_terms").toString());
    _discdate->setDate(q.value("discdate").toDate());

    if(q.value("past").toBool())
    {
      QPalette tmpPalette = _discdate->palette();
      tmpPalette.setColor(QPalette::HighlightedText, namedColor("error"));
      _discdate->setPalette(tmpPalette);
      _discdate->setForegroundRole(QPalette::HighlightedText);
      _discdateLit->setPalette(tmpPalette);
      _discdateLit->setForegroundRole(QPalette::HighlightedText);
    }

    _discprcnt->setDouble(q.value("terms_discprcnt").toDouble() * 100);

    _owed->setLocalValue(q.value("aropen_amount").toDouble());
    _applieddiscounts->setLocalValue(q.value("applied").toDouble());
  }

  else if (q.lastError().type() != QSqlError::NoError)
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
}
