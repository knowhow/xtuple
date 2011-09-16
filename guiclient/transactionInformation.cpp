/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "transactionInformation.h"

#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

transactionInformation::transactionInformation(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_save, SIGNAL(clicked()), this, SLOT(sSave()));

  _item->setReadOnly(TRUE);

  _transactionQty->setPrecision(omfgThis->qtyVal());
  _qohBefore->setPrecision(omfgThis->qtyVal());
  _qohAfter->setPrecision(omfgThis->qtyVal());

  if (!_metrics->boolean("MultiWhs"))
  {
    _warehouseLit->hide();
    _warehouse->hide();
  }
}

transactionInformation::~transactionInformation()
{
  // no need to delete child widgets, Qt does it all for us
}

void transactionInformation::languageChange()
{
  retranslateUi(this);
}

enum SetResponse transactionInformation::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("invhist_id", &valid);
  if (valid)
  {
    _invhistid = param.toInt();

    q.prepare( "SELECT *, "
               "       CASE WHEN (invhist_transtype IN ('EX', 'IM', 'SH', 'SI')) THEN (invhist_invqty * -1.0)"
               "            ELSE invhist_invqty"
               "       END AS adjinvqty "
               "FROM invhist "
               "WHERE (invhist_id=:invhist_id);" );
    q.bindValue(":invhist_id", _invhistid);
    q.exec();
    if (q.first())
    {
      _analyze->setChecked(q.value("invhist_analyze").toBool());
      _transactionType->setText(q.value("invhist_transtype").toString());
      _transactionDate->setDate(q.value("invhist_transdate").toDate());
      _createdDate->setDate(q.value("invhist_created").toDate());
      _username->setText(q.value("invhist_user").toString());
      _item->setItemsiteid(q.value("invhist_itemsite_id").toInt());
      _transactionQty->setText(formatQty(q.value("adjinvqty").toDouble()));
      _qohBefore->setText(formatQty(q.value("invhist_qoh_before").toDouble()));
      _qohAfter->setText(formatQty(q.value("invhist_qoh_after").toDouble()));
      _notes->setText(q.value("invhist_comments").toString());
    }
    else if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return UndefinedError;
    }
  }

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "edit")
      _mode = cEdit;
    else if (param.toString() == "view")
    {
      _mode = cView;

      _analyze->setEnabled(FALSE);

      _save->hide();
      _close->setText(tr("&Close"));
      _close->setFocus();
    }
  }

  return NoError;
}

void transactionInformation::sSave()
{
  q.prepare( "UPDATE invhist "
             "SET invhist_analyze=:invhist_analyze "
             "WHERE (invhist_id=:invhist_id);" );
  q.bindValue(":invhist_analyze", QVariant(_analyze->isChecked()));
  q.bindValue(":invhist_id", _invhistid);
  q.exec();
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  accept();
}
