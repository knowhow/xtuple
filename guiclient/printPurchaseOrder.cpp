/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "printPurchaseOrder.h"

#include <QMessageBox>
#include <QSqlError>
#include <QVariant>
#include <openreports.h>

printPurchaseOrder::printPurchaseOrder(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));

  _captive = false;

  _vendorCopy->setChecked(_metrics->boolean("POVendor"));

  if (_metrics->value("POInternal").toInt() > 0)
  {
    _internalCopy->setChecked(true);
    _numOfCopies->setValue(_metrics->value("POInternal").toInt());
  }
  else
  {
    _internalCopy->setChecked(false);
//    _numOfCopies->setEnabled(false);
  }
}

printPurchaseOrder::~printPurchaseOrder()
{
  // no need to delete child widgets, Qt does it all for us
}

void printPurchaseOrder::languageChange()
{
  retranslateUi(this);
}

enum SetResponse printPurchaseOrder::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  _captive = true;

  QVariant param;
  bool     valid;

  param = pParams.value("pohead_id", &valid);
  if (valid)
  {
    _po->setId(param.toInt());
    _print->setFocus();
  }

  return NoError;
}

void printPurchaseOrder::sPrint()
{
  bool alreadyPrinted = false;
  q.prepare("SELECT pohead_saved, pohead_printed FROM pohead WHERE pohead_id=:pohead_id");
  q.bindValue(":pohead_id", _po->id());
  q.exec();
  if(q.first())
  {
    alreadyPrinted = q.value("pohead_printed").toBool();
    if (q.value("pohead_saved").toBool() == false)
    {
      QMessageBox::warning( this, tr("Cannot Print P/O"),
                           tr("<p>The Purchase Order you are trying to print has "
                              "not been completed. Please wait until the "
                              "Purchase Order has been completely saved.") );
      return;
    }
  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  QPrinter  *printer = new QPrinter(QPrinter::HighResolution);
  bool      setupPrinter = true;
  bool userCanceled = false;
  if (orReport::beginMultiPrint(printer, userCanceled) == false)
  {
    if(!userCanceled)
      systemError(this,
                  tr("<p>Could not initialize printing system for multiple "
                     "reports."));
    return;
  }

  if (_vendorCopy->isChecked())
  {
    ParameterList params;
    params.append("pohead_id", _po->id());
    params.append("title", "Vendor Copy");

    orReport report("PurchaseOrder", params);
    if (report.isValid() && report.print(printer, setupPrinter))
      setupPrinter = false;
    else
    {
      report.reportError(this);
      return;
    }
  }

  if (_internalCopy->isChecked())
  {
    for (int counter = _numOfCopies->value(); counter; counter--)
    {
      ParameterList params;

      params.append("pohead_id", _po->id());
      params.append("title", QString("Internal Copy #%1").arg(counter));

      orReport report("PurchaseOrder", params);

      if (report.isValid() && report.print(printer, setupPrinter))
          setupPrinter = false;
      else
      {
        report.reportError(this);
	orReport::endMultiPrint(printer);
	return;
      }
    }
  }
  orReport::endMultiPrint(printer);

  if (!alreadyPrinted)
  {
    q.prepare( "UPDATE pohead "
               "SET pohead_printed=TRUE "
               "WHERE (pohead_id=:pohead_id);" );
    q.bindValue(":pohead_id", _po->id());
    q.exec();
    if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }

  if (_captive)
    accept();
  else
  {
    _po->setId(-1);
    _po->setFocus();
  }
}
