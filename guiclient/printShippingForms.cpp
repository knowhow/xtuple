/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "printShippingForms.h"

#include <QMessageBox>
#include <QVariant>

#include <openreports.h>
#include "editICMWatermark.h"

printShippingForms::printShippingForms(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_shipformNumOfCopies, SIGNAL(valueChanged(int)), this, SLOT(sHandleShippingFormCopies(int)));
  connect(_shipformWatermarks, SIGNAL(itemSelected(int)), this, SLOT(sEditShippingFormWatermark()));

  _captive = FALSE;

  _shipformWatermarks->addColumn( tr("Copy #"),      _dateColumn, Qt::AlignCenter );
  _shipformWatermarks->addColumn( tr("Watermark"),   -1,          Qt::AlignLeft   );
  _shipformWatermarks->addColumn( tr("Show Prices"), _dateColumn, Qt::AlignCenter );

  _shipformNumOfCopies->setValue(_metrics->value("ShippingFormCopies").toInt());
  if (_shipformNumOfCopies->value())
  {
    for (int i = 0; i < _shipformWatermarks->topLevelItemCount(); i++)
    {
      _shipformWatermarks->topLevelItem(i)->setText(1, _metrics->value(QString("ShippingFormWatermark%1").arg(i)));
      _shipformWatermarks->topLevelItem(i)->setText(2, ((_metrics->boolean(QString("ShippingFormShowPrices%1").arg(i))) ? tr("Yes") : tr("No")));
    }
  }

  if (_preferences->boolean("XCheckBox/forgetful"))
  {
    _printNew->setChecked(true);
    _printDirty->setChecked(true);
  }
}

printShippingForms::~printShippingForms()
{
    // no need to delete child widgets, Qt does it all for us
}

void printShippingForms::languageChange()
{
    retranslateUi(this);
}

enum SetResponse printShippingForms::set(const ParameterList & pParams)
{
  XDialog::set(pParams);
  _captive = TRUE;
  return NoError;
}

void printShippingForms::sHandleShippingFormCopies(int pValue)
{
  if (_shipformWatermarks->topLevelItemCount() > pValue)
    _shipformWatermarks->takeTopLevelItem(_shipformWatermarks->topLevelItemCount() - 1);
  else
  {
    for (int i = (_shipformWatermarks->topLevelItemCount() + 1); i <= pValue; i++ )
      new XTreeWidgetItem(_shipformWatermarks,
			  _shipformWatermarks->topLevelItem(_shipformWatermarks->topLevelItemCount() - 1),
			  i, i, tr("Copy #%1").arg(i), "", tr("Yes"));
  }
}

void printShippingForms::sEditShippingFormWatermark()
{
  QTreeWidgetItem *cursor = _shipformWatermarks->currentItem();
  if (cursor)
  {
    ParameterList params;
    params.append("watermark", cursor->text(1));
    params.append("showPrices", (cursor->text(2) == tr("Yes")));

    editICMWatermark newdlg(this, "", TRUE);
    newdlg.set(params);
    if (newdlg.exec() == XDialog::Accepted)
    {
      cursor->setText(1, newdlg.watermark());
      cursor->setText(2, ((newdlg.showPrices()) ? tr("Yes") : tr("No")));
    }
  }
}

void printShippingForms::sPrint()
{
  if (!_printNew->isChecked() && !_printDirty->isChecked())
  {
    QMessageBox::warning( this, tr("Cannot Print Shipping Forms"),
                          tr("You must indicate if you wish to print Shipping Forms for New and/or Changed Shipments.") );
    return;
  }

  QString sql( "SELECT shiphead_id, shipform_report_name AS report_name "
               "FROM shiphead, cohead, shipform "
               "WHERE ( (NOT shiphead_shipped)"
               " AND (shiphead_order_id=cohead_id)"
	       " AND (shiphead_order_type='SO')"
               " AND (shipform_id=COALESCE(shiphead_shipform_id, cohead_shipform_id))"
               " AND (shiphead_sfstatus IN (" );

  if (_printNew->isChecked())
  {
    sql += "'N'";

    if (_printDirty->isChecked())
      sql += ", 'D'";
  }
  else if (_printDirty->isChecked())
    sql += "'D'";
  sql += ")) ) "
         "ORDER BY shiphead_id;";

  XSqlQuery reports;
  reports.exec(sql);
  if (reports.first())
  {
    QPrinter printer(QPrinter::HighResolution);

    bool     setupPrinter = TRUE;
    bool userCanceled = false;
    if (orReport::beginMultiPrint(&printer, userCanceled) == false)
    {
      if(!userCanceled)
        systemError(this, tr("Could not initialize printing system for multiple reports."));
      return;
    }

    do
    {
      for (int i = 0; i < _shipformWatermarks->topLevelItemCount(); i++ )
      {
        ParameterList params;

        params.append("cosmisc_id", reports.value("shiphead_id").toInt());
        params.append("shiphead_id", reports.value("shiphead_id").toInt());

        params.append("watermark", _shipformWatermarks->topLevelItem(i)->text(1));

#if 0
        params.append("shipchrg_id", _shipchrg->id());

#endif

        if (_shipformWatermarks->topLevelItem(i)->text(2) == tr("Yes"))
          params.append("showcosts");

        orReport report(reports.value("report_name").toString(), params);
        if (report.print(&printer, setupPrinter))
          setupPrinter = FALSE;
        else
        {
          report.reportError(this);
	  orReport::endMultiPrint(&printer);
          return;
        }
      }

      XSqlQuery setStatus;
      setStatus.prepare( "UPDATE shiphead "
                         "SET shiphead_sfstatus='P' "
                         "WHERE (shiphead_id=:shiphead_id);" );
      setStatus.bindValue(":shiphead_id", reports.value("shiphead_id").toInt());
      setStatus.exec();
    }
    while (reports.next());
    orReport::endMultiPrint(&printer);

    if (_captive)
      accept();
  }
  else
    QMessageBox::warning( this, tr("Cannot Print Shipping Forms"),
                          tr("There are no New or Changed Shipments for which Shipping Forms should be printed.") );

}
