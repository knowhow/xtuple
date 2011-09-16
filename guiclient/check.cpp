/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "check.h"

#include <QSqlError>
#include <QVariant>

#include <metasql.h>

check::check(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  _items->addColumn(tr("P/O"),        _orderColumn, Qt::AlignRight, true, "checkitem_ponumber");
  _items->addColumn(tr("Voucher"),    _orderColumn, Qt::AlignRight, true, "checkitem_vouchernumber");
  _items->addColumn(tr("Invoice"),    _orderColumn, Qt::AlignRight, true, "checkitem_invcnumber");
  _items->addColumn(tr("C/M"),        _orderColumn, Qt::AlignRight, true, "checkitem_cmnumber");
  if (_metrics->boolean("EnableReturnAuth"))
    _items->addColumn(tr("R/A"),      _orderColumn, Qt::AlignRight, true, "checkitem_ranumber");
  _items->addColumn(tr("A/P Doc."),   _orderColumn, Qt::AlignRight, true, "apopen_docnumber");
  _items->addColumn(tr("A/R Doc."),   _orderColumn, Qt::AlignRight, true, "aropen_docnumber");
  _items->addColumn(tr("Doc. Date"),   _dateColumn, Qt::AlignCenter,true, "checkitem_docdate");
  _items->addColumn(tr("Amount"),     _moneyColumn, Qt::AlignRight, true, "checkitem_amount");
  _items->addColumn(tr("Discount"),    _itemColumn, Qt::AlignRight, true, "checkitem_discount");
  _items->addColumn(tr("Amount (in %1)").arg(CurrDisplay::baseCurrAbbr()),
                                      _moneyColumn, Qt::AlignRight, true, "baseamount");

  _achBatchNumber->setVisible(_metrics->boolean("ACHSupported") && _metrics->boolean("ACHEnabled"));

  _checkid = -1;
}

check::~check()
{
  // no need to delete child widgets, Qt does it all for us
}

void check::languageChange()
{
  retranslateUi(this);
}

enum SetResponse check::set(const ParameterList &pParams)
{
  XWidget::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("checkhead_id", &valid);
  if (valid)
  {
    _checkid = param.toInt();
    sPopulate();
  }

  return NoError;
}

void check::sPopulate()
{
  MetaSQLQuery mql("SELECT * "
                   "FROM checkhead"
                   "     JOIN checkrecip ON (checkhead_recip_id=checkrecip_id"
                   "                 AND checkhead_recip_type=checkrecip_type)"
                   "WHERE (checkhead_id=<? value(\"checkid\")?>);");
  ParameterList params;
  params.append("checkid", _checkid);
  q = mql.toQuery(params);
  if (q.first())
  {
    _checkNumber->setText(q.value("checkhead_number").toString());
    _bankaccnt->setId(q.value("checkhead_bankaccnt_id").toInt());
    _recipNumber->setText(q.value("checkrecip_number").toString());
    _recipName->setText(q.value("checkrecip_name").toString());
    _amount->set(q.value("checkhead_amount").toDouble(),
                 q.value("checkhead_curr_id").toInt(),
                 q.value("checkhead_checkdate").toDate());
    _checkdate->setDate(q.value("checkhead_checkdate").toDate());
    _achBatchNumber->setText(q.value("checkhead_ach_batch").toString());
    _recipType->setText(q.value("checkrecip_type").toString());
    _void->setChecked(q.value("checkhead_void").toBool());
    _replaced->setChecked(q.value("checkhead_replaced").toBool());
    _deleted->setChecked(q.value("checkhead_deleted").toBool());
    _posted->setChecked(q.value("checkhead_posted").toBool());
    _misc->setChecked(q.value("checkhead_misc").toBool());
    _expcat->setId(q.value("checkhead_expcat_id").toInt());
    _for->setText(q.value("checkhead_for").toString());
    _notes->setPlainText(q.value("checkhead_notes").toString());
    sFillList();
  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
  else
    clear();
}

void check::clear()
{
  _checkNumber->setText("");
  _bankaccnt->setId(-1);
  _recipNumber->setText("");
  _recipName->setText("");
  _checkdate->clear();
  _achBatchNumber->setText("");
  _recipType->setText("");
  _void->setChecked(false);
  _replaced->setChecked(false);
  _deleted->setChecked(false);
  _posted->setChecked(false);
  _misc->setChecked(false);
  _expcat->setId(-1);
  _for->setText("");
  _notes->setPlainText("");
  _items->clear();
}

void check::sFillList()
{
  if (_misc->isChecked())
    _items->clear();
  else
  {
    MetaSQLQuery mql("SELECT checkitem.*,"
                     "       apopen_docnumber, aropen_docnumber,"
                     "       currToBase(<? value(\"curr_id\") ?>, checkitem_amount,"
                     "                  checkitem_docdate) AS baseamount,"
                     "       'curr' AS checkitem_amount_xtnumericrole,"
                     "       'curr' AS checkitem_discount_xtnumericrole,"
                     "       'curr' AS baseamount_xtnumericrole "
                     "FROM checkitem"
                     "     LEFT OUTER JOIN apopen ON (checkitem_apopen_id=apopen_id)"
                     "     LEFT OUTER JOIN aropen ON (checkitem_aropen_id=aropen_id) "
                     "WHERE (checkitem_checkhead_id=<? value(\"checkid\") ?>);");
    ParameterList params;
    params.append("checkid", _checkid);
    params.append("curr_id", _amount->id());
    q = mql.toQuery(params);
    _items->populate(q, true);
    if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }
}
