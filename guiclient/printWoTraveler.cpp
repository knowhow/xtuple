/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "printWoTraveler.h"

#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include <openreports.h>

#include "inputManager.h"
#include "storedProcErrorLookup.h"

#define DEBUG true

printWoTraveler::printWoTraveler(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_packingList,SIGNAL(toggled(bool)), this, SLOT(sHandlePrintButton()));
  connect(_pickList,   SIGNAL(toggled(bool)), this, SLOT(sHandlePrintButton()));
  connect(_print,          SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_wo,            SIGNAL(newId(int)), this, SLOT(sHandleOptions(int)));
  connect(_wo,           SIGNAL(valid(bool)), this, SLOT(sHandlePrintButton()));
  connect(_woLabel,    SIGNAL(toggled(bool)), this, SLOT(sHandlePrintButton()));

  _captive = FALSE;

  omfgThis->inputManager()->notify(cBCWorkOrder, this, _wo, SLOT(setId(int)));

  _wo->setType(cWoExploded | cWoReleased | cWoIssued);

  if (!_privileges->check("ReleaseWorkOrders"))
    _releaseWo->setEnabled(FALSE);
    
  _errorPrinting = false;
}

printWoTraveler::~printWoTraveler()
{
  // no need to delete child widgets, Qt does it all for us
}

void printWoTraveler::languageChange()
{
  retranslateUi(this);
}

enum SetResponse printWoTraveler::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  _captive = TRUE;

  QVariant param;
  bool     valid;

  param = pParams.value("wo_id", &valid);
  if (valid)
  {
    _wo->setId(param.toInt());
    _releaseWo->setFocus();
  }

  if (pParams.inList("print"))
  {
    sPrint();
    return NoError_Print;
  }

  return NoError;
}

void printWoTraveler::sHandleOptions(int pWoid)
{
  if (pWoid != -1)
  {
    XSqlQuery check;

    check.prepare( "SELECT womatl_id "
                   "FROM womatl "
                   "WHERE (womatl_wo_id=:wo_id) "
                   "LIMIT 1;" );
    check.bindValue(":wo_id", pWoid);
    check.exec();
    if (check.lastError().type() != QSqlError::NoError)
    {
      systemError(this, check.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
    if (check.first())
    {
      _pickList->setEnabled(true);
      _pickList->setForgetful(false);
      _pickList->setObjectName("_pickList");
    }
    else
    {
      _pickList->setEnabled(false);
      _pickList->setForgetful(true);
      _pickList->setChecked(false);
    }
  
    check.prepare( "SELECT wo_id "
                   "FROM wo "
                   "WHERE ( (wo_ordtype='S')"
                   " AND (wo_id=:wo_id) );" );
    check.bindValue(":wo_id", pWoid);
    check.exec();
    if (check.lastError().type() != QSqlError::NoError)
    {
      systemError(this, check.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
    if (check.first())
    {
      _packingList->setEnabled(true);
      _packingList->setForgetful(false);
      _packingList->setObjectName("_packingList");
    }
    else
    {
      _packingList->setEnabled(false);
      _packingList->setForgetful(true);
      _packingList->setChecked(false);
    }
    
  }
}

void printWoTraveler::sPrint()
{
  if (DEBUG) qDebug("printWoTraveler::sPrint() entered");

  QPrinter  printer(QPrinter::HighResolution);
  bool      setupPrinter = TRUE;
  bool      userCanceled = false;

  _errorPrinting = false;

  if (orReport::beginMultiPrint(&printer, userCanceled) == false)
  {
    if(!userCanceled)
      systemError(this, tr("Could not initialize printing system for multiple reports."));
    return;
  }

  if (DEBUG) qDebug("printWoTraveler::sPrint() multiprint started");

  if (_pickList->isChecked())
  {
    ParameterList params;
    params.append("wo_id", _wo->id());

    orReport report("PickList", params);
    if (report.isValid() && report.print(&printer, setupPrinter))
      setupPrinter = FALSE;
    else
    {
      report.reportError(this);
      _errorPrinting = true;
    }
  }

  if (DEBUG) qDebug("printWoTraveler::sPrint() PickList handled");

  if (_woLabel->isChecked() && ! _errorPrinting)
  {
    XSqlQuery query;
    query.prepare( "SELECT wo_id, CAST(wo_qtyord AS INTEGER) AS wo_qtyord_int "
                   "FROM wo "
                   " WHERE (wo_id=:wo_id);" );
    query.bindValue(":wo_id", _wo->id());
    query.exec();
    if (query.first())
    {
      ParameterList params;
      params.append("wo_id", query.value("wo_id"));
      params.append("labelTo", query.value("wo_qtyord_int"));

      orReport report("WOLabel", params);
      if (report.isValid() && report.print(&printer, setupPrinter))
	setupPrinter = FALSE;
      else
      {
	report.reportError(this);
        _errorPrinting = true;
      }
    }
    else if (query.lastError().type() != QSqlError::NoError)
    {
      systemError(this, query.lastError().databaseText(), __FILE__, __LINE__);
      _errorPrinting = true;
    }
  }

  if (DEBUG) qDebug("printWoTraveler::sPrint() WOLabel handled");

  if (_packingList->isChecked() && ! _errorPrinting)
  {
    XSqlQuery query;
    query.prepare( "SELECT cohead_id, findCustomerForm(cohead_cust_id, 'L') AS reportname "
                   "FROM cohead, coitem, wo "
                   "WHERE ( (coitem_cohead_id=cohead_id)"
                   " AND (wo_ordid=coitem_id)"
                   " AND (wo_ordtype='S')"
                   " AND (wo_id=:wo_id) );" );
    query.bindValue(":wo_id", _wo->id());
    query.exec();
    if (query.first())
    {
      ParameterList params;
      params.append("sohead_id", query.value("cohead_id"));
      params.append("head_id",  query.value("cohead_id"));
      params.append("head_type",  "SO");
      if (_metrics->boolean("MultiWhs"))
	params.append("MultiWhs");

      orReport report(query.value("reportname").toString(), params);
      if (report.isValid() && report.print(&printer, setupPrinter))
	setupPrinter = FALSE;
      else
      {
	report.reportError(this);
        _errorPrinting = true;
      }
    }
    else if (query.lastError().type() != QSqlError::NoError)
    {
      systemError(this, query.lastError().databaseText(), __FILE__, __LINE__);
      _errorPrinting = true;
    }
  }

  if (DEBUG) qDebug("printWoTraveler::sPrint() PackingList handled");

  if (! _errorPrinting)
  {
    emit finishedPrinting(&printer);
  }

  if (DEBUG) qDebug("printWoTraveler::sPrint() finishedPrinting handled");

  orReport::endMultiPrint(&printer);

  if (_errorPrinting)
    return;

  if (_releaseWo->isChecked())
  {
    XSqlQuery release;
    release.prepare("SELECT releaseWo(:wo_id, FALSE) AS result;");
    release.bindValue(":wo_id", _wo->id());
    release.exec();
    if (release.first())
    {
      int result = release.value("result").toInt();
      if (result < 0)
      {
        systemError(this, storedProcErrorLookup("releaseWo", result),
                    __FILE__, __LINE__);
        return;
      }
    }
    else if (release.lastError().type() != QSqlError::NoError)
    {
      systemError(this, release.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }

    omfgThis->sWorkOrdersUpdated(_wo->id(), TRUE);
  }

  if (_captive)
    close();
  else
  {
    _errorPrinting = false;
    _wo->setId(-1);
    _wo->setFocus();
  }
}

void printWoTraveler::sHandlePrintButton()
{
  _print->setEnabled( _wo->isValid() &&
                     (_pickList->isChecked() || _packingList->isChecked()
                      || _woLabel->isChecked()) );
}
