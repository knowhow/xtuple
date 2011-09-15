/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "plannerCode.h"

#include <QVariant>
#include <QMessageBox>

plannerCode::plannerCode(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
  : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  if (_metrics->value("Application") != "PostBooks")
  {
    QButtonGroup* _explosionGroupInt = new QButtonGroup(this);
    _explosionGroupInt->addButton(_singleLevel);
    _explosionGroupInt->addButton(_multipleLevel);
    _singleLevel->setChecked(true);
    _explosionGroup->setEnabled(false);
  }
  else
  {
    _mrpexcpResched->hide();
    _mrpexcpDelete->hide();
    _autoExplode->hide();
    _explosionGroup->hide();
  }
  
  // signals and slots connections
  connect(_buttonBox, SIGNAL(accepted()), this, SLOT(sSave()));
  connect(_buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
  connect(_code, SIGNAL(lostFocus()), this, SLOT(sCheck()));
}

plannerCode::~plannerCode()
{
  // no need to delete child widgets, Qt does it all for us
}

void plannerCode::languageChange()
{
  retranslateUi(this);
}

enum SetResponse plannerCode::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("plancode_id", &valid);
  if (valid)
  {
    _plancodeid = param.toInt();
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
      _code->setFocus();
    }
    else if (param.toString() == "view")
    {
      _mode = cView;
      _code->setEnabled(FALSE);
      _description->setEnabled(FALSE);
      _mrpexcpResched->setEnabled(FALSE);
      _mrpexcpDelete->setEnabled(FALSE);
      _autoExplode->setEnabled(FALSE);
      _explosionGroup->setEnabled(FALSE);
      _buttonBox->clear();
      _buttonBox->addButton(QDialogButtonBox::Close);
      _buttonBox->setFocus();
    }
  }

  return NoError;
}

bool plannerCode::sCheck()
{
  _code->setText(_code->text().trimmed());
  if ((_mode == cNew) && (_code->text().length() != 0))
  {
    q.prepare( "SELECT plancode_id "
               "FROM plancode "
               "WHERE (UPPER(plancode_code)=UPPER(:plancode_code));" );
    q.bindValue(":plancode_code", _code->text());
    q.exec();
    if (q.first())
    {
      _plancodeid = q.value("plancode_id").toInt();
      _mode = cEdit;
      populate();

      _code->setEnabled(FALSE);
      return TRUE;
    }
  }
  return FALSE;
}

void plannerCode::sSave()
{
  _code->setText(_code->text().trimmed().toUpper());
  if (_code->text().length() == 0)
  {
    QMessageBox::information( this, tr("Invalid Planner Code"),
                              tr("You must enter a valid Code for this Planner.") );
    _code->setFocus();
    return;
  }

  if (_mode == cNew)
  {
    if (sCheck())
    {
      QMessageBox::warning( this, tr("Cannot Save Planner Code"),
                            tr("This Planner code already exists.  You have been placed in edit mode.") );
      return;
    }

    q.exec("SELECT NEXTVAL('plancode_plancode_id_seq') AS plancode_id");
    if (q.first())
      _plancodeid = q.value("plancode_id").toInt();
    else
    {
      systemError(this, tr("A System Error occurred at %1::%2.")
                        .arg(__FILE__)
                        .arg(__LINE__) );
      return;
    }

    q.prepare( "INSERT INTO plancode "
               "( plancode_id, plancode_code, plancode_name,"
               "  plancode_mpsexplosion, plancode_consumefcst,"
               "  plancode_mrpexcp_resched, plancode_mrpexcp_delete ) "
               "VALUES "
               "( :plancode_id, :plancode_code, :plancode_name,"
               "  :plancode_mpsexplosion, :plancode_consumefcst,"
               "  :plancode_mrpexcp_resched, :plancode_mrpexcp_delete );" );
  }
  else if (_mode == cEdit)
    q.prepare("SELECT plancode_id"
              "  FROM plancode"
              " WHERE((plancode_id != :plancode_id)"
              " AND (plancode_code = :plancode_code));");
    q.bindValue(":plancode_id", _plancodeid); 
    q.bindValue(":plancode_code", _code->text());
    q.exec();
    if(q.first())
    {
      QMessageBox::warning( this, tr("Cannot Save Planner Code"),
                            tr("You may not rename this Planner code with the entered name as it is in use by another Planner code.") );
      _code->setFocus();
      return;
    }

    q.prepare( "UPDATE plancode "
               "SET plancode_code=:plancode_code, plancode_name=:plancode_name,"
               "    plancode_mpsexplosion=:plancode_mpsexplosion,"
               "    plancode_consumefcst=:plancode_consumefcst,"
               "    plancode_mrpexcp_resched=:plancode_mrpexcp_resched, "
               "    plancode_mrpexcp_delete=:plancode_mrpexcp_delete "
               "WHERE (plancode_id=:plancode_id);" );

  q.bindValue(":plancode_id", _plancodeid);
  q.bindValue(":plancode_code", _code->text());
  q.bindValue(":plancode_name", _description->text().trimmed());
  q.bindValue(":plancode_consumefcst", false);
  q.bindValue(":plancode_mrpexcp_resched", QVariant(_mrpexcpResched->isChecked()));
  q.bindValue(":plancode_mrpexcp_delete", QVariant(_mrpexcpDelete->isChecked()));

  if (_autoExplode->isChecked())
  {
    if (_singleLevel->isChecked())
      q.bindValue(":plancode_mpsexplosion", "S");
    else
      q.bindValue(":plancode_mpsexplosion", "M");
  }
  else
    q.bindValue(":plancode_mpsexplosion", "N");

  q.exec();

  done(_plancodeid);
}

void plannerCode::populate()
{
  q.prepare( "SELECT * "
             "FROM plancode "
             "WHERE (plancode_id=:plancode_id);" );
  q.bindValue(":plancode_id", _plancodeid);
  q.exec();
  if (q.first())
  {
    _code->setText(q.value("plancode_code"));
    _description->setText(q.value("plancode_name"));
    _mrpexcpResched->setChecked(q.value("plancode_mrpexcp_resched").toBool());
    _mrpexcpDelete->setChecked(q.value("plancode_mrpexcp_delete").toBool());

    if (q.value("plancode_mpsexplosion").toString() == "N")
      _autoExplode->setChecked(FALSE);
    else
    {
      _autoExplode->setChecked(TRUE);

      if (q.value("plancode_mpsexplosion").toString() == "S")
        _singleLevel->setChecked(TRUE);
      else if (q.value("plancode_mpsexplosion").toString() == "M")
        _multipleLevel->setChecked(TRUE);
    }
  }
} 

