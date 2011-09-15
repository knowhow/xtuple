/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "reprioritizeWo.h"

#include <QMessageBox>
#include <QVariant>

reprioritizeWo::reprioritizeWo(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_reprioritize, SIGNAL(clicked()), this, SLOT(sReprioritize()));
  connect(_wo, SIGNAL(newId(int)), this, SLOT(sPopulateWoInfo(int)));

  _captive = FALSE;

  _wo->setType(cWoOpen | cWoExploded);

  if (_preferences->boolean("XCheckBox/forgetful"))
    _changeChildren->setChecked(true);
}

reprioritizeWo::~reprioritizeWo()
{
  // no need to delete child widgets, Qt does it all for us
}

void reprioritizeWo::languageChange()
{
  retranslateUi(this);
}

enum SetResponse reprioritizeWo::set(const ParameterList &pParams)
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
    _new->setFocus();
  }

  return NoError;
}

void reprioritizeWo::sReprioritize()
{
  if (_wo->status() == 'R')
  {
    QMessageBox::warning( this, tr("Cannot Reschedule Released W/O"),
                          tr( "The selected Work Order has been Released.\n"
                              "You must Recall this Work Order before Rescheduling it." ) );
    return;
  }

  q.prepare("SELECT reprioritizeWo(:wo_id, :newPriority, :reprioritizeChildren);");
  q.bindValue(":wo_id", _wo->id());
  q.bindValue(":newPriority", _new->value());
  q.bindValue(":reprioritizeChildren", QVariant(_changeChildren->isChecked()));
  q.exec();

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


void reprioritizeWo::sPopulateWoInfo(int pWoid)
{
  if (pWoid != -1)
  {
    q.prepare( "SELECT wo_priority "
               "FROM wo "
               "WHERE (wo_id=:wo_id);" );
    q.bindValue(":wo_id", pWoid);
    q.exec();
    if (q.first())
    {
      _current->setText(q.value("wo_priority").toString());
      _new->setValue(q.value("wo_priority").toInt());
      return;
    }
    else
      systemError(this, tr("A System Error occurred at %1::%2.")
                        .arg(__FILE__)
                        .arg(__LINE__) );
  }

  _current->clear();
  _new->setValue(1);
}

