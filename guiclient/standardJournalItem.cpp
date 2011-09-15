/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "standardJournalItem.h"

#include <QVariant>
#include <QMessageBox>
#include <QValidator>

standardJournalItem::standardJournalItem(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  // signals and slots connections
  connect(_save, SIGNAL(clicked()), this, SLOT(sSave()));
  connect(_close, SIGNAL(clicked()), this, SLOT(reject()));
}

standardJournalItem::~standardJournalItem()
{
  // no need to delete child widgets, Qt does it all for us
}

void standardJournalItem::languageChange()
{
  retranslateUi(this);
}

enum SetResponse standardJournalItem::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("stdjrnl_id", &valid);
  if (valid)
    _stdjrnlid = param.toInt();

  param = pParams.value("stdjrnlitem_id", &valid);
  if (valid)
  {
    _stdjrnlitemid = param.toInt();
    populate();
  }

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      _mode = cNew;
      _account->setFocus();
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;
      _save->setFocus();
    }
    else if (param.toString() == "view")
    {
      _mode = cView;
      _account->setEnabled(FALSE);
      _amount->setEnabled(FALSE);
      _senseGroup->setEnabled(FALSE);
      _notes->setEnabled(FALSE);
      _close->setText(tr("&Close"));
      _save->hide();

      _close->setFocus();
    }
  }

  return NoError;
}

void standardJournalItem::sSave()
{
  double amount = _amount->baseValue();
  if (_debit->isChecked())
    amount *= -1;

  if (_amount->isZero())
  {
    QMessageBox::warning( this, tr("Incomplete Data"),
      tr("You must enter an amount value.") );
    _amount->setFocus();
    return;
  }

  if(!_account->isValid())
  {
    QMessageBox::warning( this, tr("Incomplete Data"),
      tr("You must enter an account.") );
    return;
  }

  if (! _amount->isBase() &&
      QMessageBox::question(this, tr("G/L Transaction Not In Base Currency"),
                          tr("G/L transactions are recorded in the base currency.\n"
                          "Do you wish to convert %1 %2 at the current rate?")
                          .arg(_amount->localValue()).arg(_amount->currAbbr()),
                          QMessageBox::Yes|QMessageBox::Escape,
                          QMessageBox::No |QMessageBox::Default) != QMessageBox::Yes)
  {
        _amount->setFocus();
        return;
  }

  if (_mode == cNew)
  {
    q.exec("SELECT NEXTVAL('stdjrnlitem_stdjrnlitem_id_seq') AS stdjrnlitem_id;");
    if (q.first())
      _stdjrnlitemid = q.value("stdjrnlitem_id").toInt();
    else
      systemError(this, tr("A System Error occurred at %1::%2.")
                        .arg(__FILE__)
                        .arg(__LINE__) );

    q.prepare( "INSERT INTO stdjrnlitem "
               "( stdjrnlitem_id, stdjrnlitem_stdjrnl_id, stdjrnlitem_accnt_id,"
               "  stdjrnlitem_amount, stdjrnlitem_notes ) "
               "VALUES "
               "( :stdjrnlitem_id, :stdjrnlitem_stdjrnl_id, :stdjrnlitem_accnt_id,"
               "  :stdjrnlitem_amount, :stdjrnlitem_notes );" );
  }
  else if (_mode == cEdit)
    q.prepare( "UPDATE stdjrnlitem "
               "SET stdjrnlitem_accnt_id=:stdjrnlitem_accnt_id,"
               "    stdjrnlitem_amount=:stdjrnlitem_amount,"
               "    stdjrnlitem_notes=:stdjrnlitem_notes "
               "WHERE (stdjrnlitem_id=:stdjrnlitem_id);" );
 
  q.bindValue(":stdjrnlitem_id", _stdjrnlitemid);
  q.bindValue(":stdjrnlitem_stdjrnl_id", _stdjrnlid);
  q.bindValue(":stdjrnlitem_accnt_id", _account->id());
  q.bindValue(":stdjrnlitem_amount", amount);
  q.bindValue(":stdjrnlitem_notes", _notes->toPlainText().trimmed());
  q.exec();

  done(_stdjrnlitemid);
}

void standardJournalItem::populate()
{
  q.prepare( "SELECT stdjrnlitem_accnt_id, stdjrnlitem_notes,"
             "       ABS(stdjrnlitem_amount) AS amount,"
             "       (stdjrnlitem_amount < 0) AS debit "
             "FROM stdjrnlitem "
             "WHERE (stdjrnlitem_id=:stdjrnlitem_id);" );
  q.bindValue(":stdjrnlitem_id", _stdjrnlitemid);
  q.exec();
  if (q.first())
  {
    _amount->setBaseValue(q.value("amount").toDouble());
    if (q.value("debit").toBool())
      _debit->setChecked(TRUE);
    else
      _credit->setChecked(TRUE);

    _account->setId(q.value("stdjrnlitem_accnt_id").toInt());
    _notes->setText(q.value("stdjrnlitem_notes").toString());
  }
}

