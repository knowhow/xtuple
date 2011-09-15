/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "printPurchaseOrdersByAgent.h"

#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include <openreports.h>
#include <parameter.h>

#include "guiclient.h"

printPurchaseOrdersByAgent::printPurchaseOrdersByAgent(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));

  _agent->setType(XComboBox::Agent);
  _agent->setText(omfgThis->username());

  _vendorCopy->setChecked(_metrics->boolean("POVendor"));

  if (_metrics->value("POInternal").toInt() > 0)
  {
    _internalCopy->setChecked(TRUE);
    _numOfCopies->setValue(_metrics->value("POInternal").toInt());
  }
  else
  {
    _internalCopy->setChecked(FALSE);
    _numOfCopies->setEnabled(FALSE);
  }
}

printPurchaseOrdersByAgent::~printPurchaseOrdersByAgent()
{
  // no need to delete child widgets, Qt does it all for us
}

void printPurchaseOrdersByAgent::languageChange()
{
  retranslateUi(this);
}

void printPurchaseOrdersByAgent::sPrint()
{
  XSqlQuery markprinted;
  markprinted.prepare("UPDATE pohead"
                      "   SET pohead_printed=true"
                      " WHERE (pohead_id=:pohead_id);");
  XSqlQuery pohead;
  pohead.prepare( "SELECT pohead_id "
                  "FROM pohead "
                  "WHERE ( (pohead_agent_username=:username)"
                  " AND (pohead_status='O')"
                  " AND (NOT pohead_printed)"
                  " AND (pohead_saved) );" );
  pohead.bindValue(":username", _agent->currentText());
  pohead.exec();
  if (pohead.first())
  {
    QPrinter  *printer = new QPrinter(QPrinter::HighResolution);
    bool      setupPrinter = TRUE;

    bool userCanceled = false;
    if (orReport::beginMultiPrint(printer, userCanceled) == false)
    {
      if(!userCanceled)
        systemError(this, tr("Could not initialize printing system for multiple reports."));
      return;
    }

    do
    {
      if (_vendorCopy->isChecked())
      {
        ParameterList params;
        params.append("pohead_id", pohead.value("pohead_id").toInt());
        params.append("title", "Vendor Copy");

        orReport report("PurchaseOrder", params);
        if (report.isValid() && report.print(printer, setupPrinter))
	  setupPrinter = FALSE;
	else
	{
          report.reportError(this);
	  orReport::endMultiPrint(printer);
	  return;
	}
      }

      if (_internalCopy->isChecked())
      {
        for (int counter = _numOfCopies->value(); counter; counter--)
        {
          ParameterList params;
          params.append("pohead_id", pohead.value("pohead_id"));
          params.append("title", QString("Internal Copy #%1").arg(counter));

          orReport report("PurchaseOrder", params);
          if (report.isValid() && report.print(printer, setupPrinter))
	    setupPrinter = FALSE;
	  else
	  {
            report.reportError(this);
	    orReport::endMultiPrint(printer);
	    return;
	  }
        }
      }
      markprinted.bindValue(":pohead_id", pohead.value("pohead_id").toInt());
      markprinted.exec();
      if (markprinted.lastError().type() != QSqlError::NoError)
      {
	systemError(this, markprinted.lastError().databaseText(), __FILE__, __LINE__);
	orReport::endMultiPrint(printer);
	return;
      }

      emit finishedPrinting(pohead.value("pohead_id").toInt());
    }
    while (pohead.next());
    orReport::endMultiPrint(printer);
  }
  else if (pohead.lastError().type() != QSqlError::NoError)
  {
    systemError(this, pohead.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
  else
    QMessageBox::information( this, tr("No Purchase Orders to Print"),
                              tr("There are no posted, unprinted Purchase Orders entered by the selected Purchasing Agent to print.") );

  accept();
}

