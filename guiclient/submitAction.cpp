/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "submitAction.h"

#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

submitAction::submitAction(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_submit, SIGNAL(clicked()), this, SLOT(sSubmit()));

  _action->setText("Unknown");
}

submitAction::~submitAction()
{
  // no need to delete child widgets, Qt does it all for us
}

void submitAction::languageChange()
{
  retranslateUi(this);
}

enum SetResponse submitAction::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  _params = pParams;

  QVariant param;
  bool     valid;

  param = pParams.value("action_name", &valid);
  if(valid)
      _action->setText(param.toString());
  
  param = pParams.value("responseEmail", &valid);
  if (valid)
    _email->setText(param.toString());
  else
  {
    q.prepare( "SELECT usr_email "
               "FROM report, usr "
               "WHERE (usr_username=getEffectiveXtUser());" );
    q.exec();
    if (q.first())
      _email->setText(q.value("usr_email").toString());
  }

  return NoError;
}

void submitAction::sSubmit()
{
  if (_email->text().trimmed().length() == 0)
  {
    QMessageBox::critical( this, tr("Cannot Submit Action"),
                           tr("You must indicate an Email address to which the completed Action response will be sent.") );
    _email->setFocus();
    return;
  }

  if(!_asap->isChecked() && !_date->isValid())
  {
    QMessageBox::critical( this, tr("Invalid Date"),
      tr("You must enter a valid date.") );
    return;
  }

  if (_asap->isChecked())
    q.prepare("SELECT xtbatch.submitActionToBatch(:action, :emailAddress, "
              "CURRENT_TIMESTAMP) AS batch_id;");
  else
  {
    q.prepare("SELECT xtbatch.submitActionToBatch(:action, :emailAddress, :scheduled) AS batch_id;");

    QDateTime scheduled;
    scheduled.setDate(_date->date());
    scheduled.setTime(_time->time());
    q.bindValue(":scheduled", scheduled);
  }

  q.bindValue(":action", _action->text());
  q.bindValue(":emailAddress", _email->text());
  q.exec();

  if (q.first())
  {
    int batchid = q.value("batch_id").toInt();

    q.prepare( "INSERT INTO xtbatch.batchparam "
               "( batchparam_batch_id, batchparam_order,"
               "  batchparam_name, batchparam_type, batchparam_value ) "
               "VALUES "
               "( :batchparam_batch_id, :batchparam_order,"
               "  :batchparam_name, :batchparam_type, :batchparam_value );" );
    for (int counter = 0; counter < _params.count(); counter++)
    {
      q.bindValue(":batchparam_batch_id", batchid);
      q.bindValue(":batchparam_order", (counter + 1));
      q.bindValue(":batchparam_name", _params.name(counter));
      QVariant v = _params.value(counter);
      q.bindValue(":batchparam_type", QVariant::typeToName(v.type()));
      q.bindValue(":batchparam_value", _params.value(counter).toString());
      q.exec();
    }
  }
  else if (q.lastError().type() != QSqlError::NoError)
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);

  accept();
}
