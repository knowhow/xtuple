/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "rescheduleWo.h"

#include <QMessageBox>
#include <QVariant>

rescheduleWo::rescheduleWo(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_reschedule, SIGNAL(clicked()), this, SLOT(sReschedule()));

  _captive = FALSE;

  _wo->setType(cWoOpen | cWoExploded);
  _cmnttype->setType(XComboBox::AllCommentTypes);

  if (_preferences->boolean("XCheckBox/forgetful"))
    _changeChildren->setChecked(true);
  _commentGroup->setEnabled(_postComment->isChecked());
}

rescheduleWo::~rescheduleWo()
{
  // no need to delete child widgets, Qt does it all for us
}

void rescheduleWo::languageChange()
{
  retranslateUi(this);
}

enum SetResponse rescheduleWo::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  _captive = TRUE;

  QVariant param;
  bool     valid;

  param = pParams.value("wo_id", &valid);
  if (valid)
  {
    _wo->setId(param.toInt());
    _wo->setReadOnly(TRUE);
    _newStartDate->setFocus();
  }

  return NoError;
}

void rescheduleWo::sReschedule()
{
  if ((_newStartDate->isValid()) && (_newDueDate->isValid()))
  {
    if (_wo->status() == 'R')
    {
      QMessageBox::warning( this, tr("Cannot Reschedule Released W/O"),
                            tr( "The selected Work Order has been Released.\n"
                                "You must Recall this Work Order before Rescheduling it." ) );
      return;
    }

    q.prepare("SELECT changeWoDates(:wo_id, :startDate, :dueDate, :rescheduleChildren);");
    q.bindValue(":wo_id", _wo->id());
    q.bindValue(":startDate", _newStartDate->date());
    q.bindValue(":dueDate", _newDueDate->date());
    q.bindValue(":rescheduleChildren", QVariant(_changeChildren->isChecked()));
    q.exec();

    if (_postComment->isChecked())
    {
      q.prepare("SELECT postComment(:cmnttype_id, 'W', :wo_id, :comment) AS _result");
      q.bindValue(":cmnttype_id", _cmnttype->id());
      q.bindValue(":wo_id", _wo->id());
      q.bindValue(":comment", _comment->toPlainText());
      q.exec();
    }

    omfgThis->sWorkOrdersUpdated(_wo->id(), TRUE);

    if (_captive)
      close();
    else
    {
      _wo->setId(-1);
      _close->setText(tr("&Close"));
      _wo->setFocus();
    }
  }
  else
  {
    QMessageBox::warning( this, tr("Valid Dates Required"),
      tr("You must specify a valid Start/Due date.") );
  }
}

