/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "implodeWo.h"

#include <QVariant>
#include <QMessageBox>
#include <QSqlError>
#include "inputManager.h"

implodeWo::implodeWo(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
  : XDialog(parent, name, modal, fl)
{
  setupUi(this);


  // signals and slots connections
  connect(_wo, SIGNAL(valid(bool)), _implode, SLOT(setEnabled(bool)));
  connect(_implode, SIGNAL(clicked()), this, SLOT(sImplode()));
  connect(_close, SIGNAL(clicked()), this, SLOT(reject()));

  _captive = TRUE;
  omfgThis->inputManager()->notify(cBCWorkOrder, this, _wo, SLOT(setId(int)));

  _wo->setType(cWoExploded);
}

implodeWo::~implodeWo()
{
  // no need to delete child widgets, Qt does it all for us
}

void implodeWo::languageChange()
{
  retranslateUi(this);
}

enum SetResponse implodeWo::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  _captive = TRUE;

  QVariant param;
  bool     valid;

  param = pParams.value("wo_id", &valid);
  if (valid)
  {
    _wo->setId(param.toInt());
    _implode->setFocus();
  }

  return NoError;
}

void implodeWo::sImplode()
{
  q.prepare( "SELECT wo_adhoc "
             "FROM wo "
             "WHERE ( (wo_adhoc)"
             " AND (wo_id=:wo_id) );" );
  q.bindValue(":wo_id", _wo->id());
  q.exec();
  if (q.first())
  {
    if ( QMessageBox::warning( this, tr("Adhoc Work Order"),
                               tr( "The Work Order you have selected to Implode is adhoc, meaning\n"
                                   "that its W/O Materials Requirements and/or W/O Operations lists\n"
                                   "have been manually modified.  If you Implode the selected Work Order\n"
                                   "then these modifications will be lost.\n"
                                   "Are you sure that you want to Implode the selected Work Order?"),
                               tr("&Yes"), tr("&No"), QString::null, 0, 1) == 1)
      return;
  }

  q.prepare("SELECT implodeWo(:wo_id, TRUE) AS result;");
  q.bindValue(":wo_id", _wo->id());
  q.exec();
  if (q.first() && q.value("result").toInt() < 0)
  {
    QString msg;
    if (q.value("result").toInt() == -1)
      msg = tr("The Work Order could not be imploded because time clock "
	       "entries exist for it.");
    else
      msg = tr("The Work Order could not be imploded (reason %1).")
	    .arg(q.value("result").toString());
    QMessageBox::information(this, tr("Work Order Not Imploded"), msg);
    return;
  }
  else if (q.lastError().type() != QSqlError::NoError)
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);

  omfgThis->sWorkOrdersUpdated(_wo->id(), TRUE);

  if (_captive)
    accept();
  else
  {
    _wo->setId(-1);
    _close->setText("&Close");
    _wo->setFocus();
  }
}

