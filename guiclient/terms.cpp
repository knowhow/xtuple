/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "terms.h"

#include <QVariant>
#include <QMessageBox>
#include <QValidator>

terms::terms(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_proximo, SIGNAL(toggled(bool)), _cutOffDay, SLOT(setEnabled(bool)));
  connect(_buttonBox, SIGNAL(accepted()), this, SLOT(sSave()));
  connect(_days, SIGNAL(toggled(bool)), this, SLOT(sTypeChanged()));
  connect(_buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
  connect(_code, SIGNAL(lostFocus()), this, SLOT(sCheck()));
  
  _discountPercent->setValidator(omfgThis->percentVal());
}

terms::~terms()
{
  // no need to delete child widgets, Qt does it all for us
}

void terms::languageChange()
{
  retranslateUi(this);
}

enum SetResponse terms::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("terms_id", &valid);
  if (valid)
  {
    _termsid = param.toInt();
    populate();
  }

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      _mode = cNew;
      _code->setFocus();
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;
      _description->setFocus();
    }
    else if (param.toString() == "view")
    {
      _mode = cView;

      _code->setEnabled(FALSE);
      _description->setEnabled(FALSE);
      _typeGroup->setEnabled(FALSE);
      _ap->setEnabled(FALSE);
      _ar->setEnabled(FALSE);
      _dueDays->setEnabled(FALSE);
      _discountDays->setEnabled(FALSE);
      _discountPercent->setEnabled(FALSE);
      _cutOffDay->setEnabled(FALSE);
      _buttonBox->clear();
      _buttonBox->addButton(QDialogButtonBox::Close);
      _buttonBox->setFocus();
    }
  }

  return NoError;
}

bool terms::sCheck()
{
  _code->setText(_code->text().trimmed());
  if ( (_mode == cNew) && (_code->text().length()) )
  {
    q.prepare( "SELECT terms_id "
               "FROM terms "
               "WHERE (UPPER(terms_code)=UPPER(:terms_code));" );
    q.bindValue(":terms_code", _code->text());
    q.exec();
    if (q.first())
    {
      _termsid = q.value("terms_id").toInt();
      _mode = cEdit;
      populate();

      _code->setEnabled(FALSE);
      return TRUE;
    }
  }
  return FALSE;
}

void terms::sSave()
{
  if(_code->text().trimmed().isEmpty())
  {
    QMessageBox::warning(this, tr("Cannot Save Terms"),
      tr("You must specify a code for the Terms."));
    _code->setFocus();
    return;
  }

  if (_mode == cNew)
  {
    if (sCheck())
    {
      QMessageBox::warning( this, tr("Cannot Save Terms Code"),
                            tr("This Terms code already exists.  You have been placed in edit mode.") );
      return;
    }

    q.exec("SELECT NEXTVAL('terms_terms_id_seq') AS _terms_id");
    if (q.first())
      _termsid = q.value("_terms_id").toInt();
    else
    {
      systemError(this, tr("A System Error occurred at %1::%2.")
                        .arg(__FILE__)
                        .arg(__LINE__) );
      return;
    }

    q.prepare( "INSERT INTO terms "
               "( terms_id, terms_code, terms_descrip, terms_type,"
               "  terms_ap, terms_ar,"
               "  terms_duedays, terms_discdays, terms_discprcnt, terms_cutoffday ) "
               "VALUES "
               "( :terms_id, :terms_code, :terms_descrip, :terms_type,"
               "  :terms_ap, :terms_ar,"
               "  :terms_duedays, :terms_discdays, :terms_discprcnt, :terms_cutoffday );" );
  }
  else if (_mode == cEdit)
  {
    q.prepare( "SELECT terms_id "
               "FROM terms "
               "WHERE ( (UPPER(terms_code)=UPPER(:terms_code))"
               " AND (terms_id<>:terms_id) );" );
    q.bindValue(":terms_id", _termsid);
    q.bindValue(":terms_code", _code->text().trimmed());
    q.exec();
    if (q.first())
    {
      QMessageBox::warning( this, tr("Cannot Save Terms"),
                            tr("You may not rename this Terms code with the entered name as it is in use by another Terms code.") );
      return;
    }

    q.prepare( "UPDATE terms "
               "SET terms_code=:terms_code, terms_descrip=:terms_descrip, terms_type=:terms_type,"
               "    terms_ap=:terms_ap, terms_ar=:terms_ar,"
               "    terms_duedays=:terms_duedays, terms_discdays=:terms_discdays,"
               "    terms_discprcnt=:terms_discprcnt, terms_cutoffday=:terms_cutoffday "
               "WHERE (terms_id=:terms_id);" );
  }

  if (_days->isChecked())
    q.bindValue(":terms_type", "D");
  else
    q.bindValue(":terms_type", "P");

  q.bindValue(":terms_id", _termsid);
  q.bindValue(":terms_code", _code->text().trimmed());
  q.bindValue(":terms_descrip", _description->text().trimmed());
  q.bindValue(":terms_ap", QVariant(_ap->isChecked()));
  q.bindValue(":terms_ar", QVariant(_ar->isChecked()));
  q.bindValue(":terms_duedays", _dueDays->value());
  q.bindValue(":terms_discdays", _discountDays->value());
  q.bindValue(":terms_discprcnt", (_discountPercent->toDouble() / 100.0));
  q.bindValue(":terms_cutoffday", _cutOffDay->value());
  q.exec();

  done(_termsid);
}

void terms::sTypeChanged()
{
  if (_days->isChecked())
  {
    _dueDaysLit->setText(tr("Due Days:"));
    _discountDaysLit->setText(tr("Discnt. Days:"));

    _dueDays->setMinimum(0);
    _dueDays->setMaximum(999);
    _discountDays->setMinimum(0);
    _discountDays->setMaximum(999);
  }
  else
  {
    _dueDaysLit->setText(tr("Due Day:"));
    _discountDaysLit->setText(tr("Discnt. Day:"));

    _dueDays->setMinimum(1);
    _dueDays->setMaximum(31);
    _discountDays->setMinimum(1);
    _discountDays->setMaximum(31);
    _cutOffDay->setMaximum(31);
  }
}

void terms::populate()
{
  q.prepare( "SELECT terms_code, terms_descrip, terms_type,"
             "       terms_ap, terms_ar,"
             "       terms_duedays, terms_discdays, terms_cutoffday,"
             "       terms_discprcnt "
             "FROM terms "
             "WHERE (terms_id=:terms_id);" );
  q.bindValue(":terms_id", _termsid);
  q.exec();
  if (q.first())
  {
    _code->setText(q.value("terms_code").toString());
    _description->setText(q.value("terms_descrip").toString());
    _ap->setChecked(q.value("terms_ap").toBool());
    _ar->setChecked(q.value("terms_ar").toBool());
    _dueDays->setValue(q.value("terms_duedays").toInt());
    _discountPercent->setText(q.value("terms_discprcnt").toDouble() * 100);
    _discountDays->setValue(q.value("terms_discdays").toInt());

    if (q.value("terms_type").toString() == "D")
    {
      _days->setChecked(TRUE);
      if (_mode == cEdit)
        _cutOffDay->setEnabled(FALSE);
    }
    else if (q.value("terms_type").toString() == "P")
    {
      _proximo->setChecked(TRUE);
      _cutOffDay->setValue(q.value("terms_cutoffday").toInt());
    }
  }
}
