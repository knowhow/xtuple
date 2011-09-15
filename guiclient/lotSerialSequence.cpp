/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "lotSerialSequence.h"

#include <QVariant>
#include <QMessageBox>

lotSerialSequence::lotSerialSequence(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
  : XDialog(parent, name, modal, fl)
{
  setupUi(this);
  
  // signals and slots connections
  connect(_buttonBox, SIGNAL(accepted()), this, SLOT(sSave()));
  connect(_buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
  connect(_number, SIGNAL(lostFocus()), this, SLOT(sCheck()));
  connect(_prefix, SIGNAL(editingFinished()), this, SLOT(sHandleExample()));
  connect(_nextValue, SIGNAL(editingFinished()), this, SLOT(sHandleExample()));
  connect(_length, SIGNAL(editingFinished()), this, SLOT(sHandleExample()));
  connect(_suffix, SIGNAL(editingFinished()), this, SLOT(sHandleExample()));


  _nextValue->setValidator(omfgThis->orderVal());
}

lotSerialSequence::~lotSerialSequence()
{
  // no need to delete child widgets, Qt does it all for us
}

void lotSerialSequence::languageChange()
{
  retranslateUi(this);
}

enum SetResponse lotSerialSequence::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("lsseq_id", &valid);
  if (valid)
  {
    _lsseqid = param.toInt();
    populate();
  }

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      _mode = cNew;
      _number->setFocus();
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;
      _number->setFocus();
    }
    else if (param.toString() == "view")
    {
      _mode = cView;
      _number->setEnabled(FALSE);
      _description->setEnabled(FALSE);
      _prefix->setEnabled(FALSE);
      _nextValue->setEnabled(FALSE);
      _length->setEnabled(FALSE);
      _suffix->setEnabled(FALSE);
      _buttonBox->clear();
      _buttonBox->addButton(QDialogButtonBox::Close);
      _buttonBox->setFocus();
    }
  }

  return NoError;
}

bool lotSerialSequence::sCheck()
{
  _number->setText(_number->text().trimmed());
  if ((_mode == cNew) && (_number->text().length() != 0))
  {
    q.prepare( "SELECT lsseq_id "
               "FROM lsseq "
               "WHERE (UPPER(lsseq_number)=UPPER(:lsseq_number));" );
    q.bindValue(":lsseq_number", _number->text());
    q.exec();
    if (q.first())
    {
      _lsseqid = q.value("lsseq_id").toInt();
      _mode = cEdit;
      populate();

      _number->setEnabled(FALSE);
      return TRUE;
    }
  }
  return FALSE;
}

void lotSerialSequence::sSave()
{
  _number->setText(_number->text().trimmed().toUpper());
  if (_number->text().length() == 0)
  {
    QMessageBox::information( this, tr("Invalid Sequence"),
                              tr("You must enter a valid Code for this Sequence.") );
    _number->setFocus();
    return;
  }

  if (_mode == cNew)
  {
    if (sCheck())
    {
      QMessageBox::warning( this, tr("Cannot Save Sequence"),
                            tr("This Sequence number already exists.  You have been placed in edit mode.") );
      return;
    }

    q.exec("SELECT NEXTVAL('lsseq_lsseq_id_seq') AS lsseq_id");
    if (q.first())
      _lsseqid = q.value("lsseq_id").toInt();
    else
    {
      systemError(this, tr("A System Error occurred at %1::%2.")
                        .arg(__FILE__)
                        .arg(__LINE__) );
      return;
    }

    q.prepare( "INSERT INTO lsseq "
               "( lsseq_id, lsseq_number) "
               "VALUES "
               "( :lsseq_id, :lsseq_number);");
  }
  else if (_mode == cEdit)
    q.prepare("SELECT lsseq_id"
              "  FROM lsseq"
              " WHERE((lsseq_id != :lsseq_id)"
              " AND (lsseq_number = :lsseq_number));");

  q.bindValue(":lsseq_id", _lsseqid);
  q.bindValue(":lsseq_number", _number->text());
  q.exec();
  if(q.first())
  {
    QMessageBox::warning( this, tr("Cannot Save Sequence Number"),
                          tr("You may not rename this Sequence number with the entered name as it is in use by another Planner code.") );
    _number->setFocus();
    return;
  }

  q.prepare( "UPDATE lsseq "
             "SET lsseq_number=:lsseq_number, lsseq_descrip=:lsseq_descrip,"
             "    lsseq_prefix=:lsseq_prefix,lsseq_seqlen=:lsseq_seqlen, "
             "    lsseq_suffix=:lsseq_suffix "
             "WHERE (lsseq_id=:lsseq_id);"
             "SELECT setval(:sequence, :next_value - 1);");

  q.bindValue(":lsseq_id", _lsseqid);
  q.bindValue(":lsseq_number", _number->text());
  q.bindValue(":lsseq_descrip", _description->text().trimmed());
  q.bindValue(":lsseq_prefix", _prefix->text());
  q.bindValue(":lsseq_seqlen", _length->value());
  q.bindValue(":lsseq_suffix", _suffix->text());
  q.bindValue(":next_value", _nextValue->text());
  q.bindValue(":sequence", QString("lsseq_number_seq_%1").arg(_lsseqid));
  q.exec();

  done(_lsseqid);
}

void lotSerialSequence::populate()
{
  q.prepare( "SELECT * "
             "FROM lsseq "
             "WHERE (lsseq_id=:lsseq_id);" );
  q.bindValue(":lsseq_id", _lsseqid);
  q.exec();
  if (q.first())
  {
    _number->setText(q.value("lsseq_number"));
    _description->setText(q.value("lsseq_descrip"));
    _prefix->setText(q.value("lsseq_prefix"));
    _length->setValue(q.value("lsseq_seqlen").toInt());
    _suffix->setText(q.value("lsseq_suffix"));

    q.prepare("SELECT last_value + 1 AS next_number "
              "FROM lsseq_number_seq_:lsseq_id;");
    q.bindValue(":lsseq_id", _lsseqid);
    q.exec();
    if (q.first())
      _nextValue->setText(q.value("next_number"));
    sHandleExample();
  }
}

void lotSerialSequence::sHandleExample()
{
  _example->setText(_prefix->text() + _nextValue->text().rightJustified(_length->value(),QChar('0')) + _suffix->text());
} 

