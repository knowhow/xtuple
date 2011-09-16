/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "printInvoices.h"

#include <QMessageBox>
#include <QSqlError>
#include <QValidator>
#include <QVariant>

#include <openreports.h>

#include "editICMWatermark.h"
#include "submitAction.h"

printInvoices::printInvoices(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_invoiceNumOfCopies, SIGNAL(valueChanged(int)), this, SLOT(sHandleCopies(int)));
  connect(_invoiceWatermarks, SIGNAL(itemSelected(int)), this, SLOT(sEditWatermark()));

  _invoiceWatermarks->addColumn(tr("Copy #"),     _dateColumn, Qt::AlignCenter);
  _invoiceWatermarks->addColumn(tr("Watermark"),  -1,          Qt::AlignLeft  );
  _invoiceWatermarks->addColumn(tr("Show Prices"),_dateColumn, Qt::AlignCenter);

  _invoiceNumOfCopies->setValue(_metrics->value("InvoiceCopies").toInt());
  if (_invoiceNumOfCopies->value())
  {
    for (int i = 0; i < _invoiceWatermarks->topLevelItemCount(); i++)
    {
      _invoiceWatermarks->topLevelItem(i)->setText(1, _metrics->value(QString("InvoiceWatermark%1").arg(i)));
      _invoiceWatermarks->topLevelItem(i)->setText(2, ((_metrics->value(QString("InvoiceShowPrices%1").arg(i)) == "t") ? tr("Yes") : tr("No")));
    }
  }

  if(!_privileges->check("PostMiscInvoices"))
  {
    _post->setChecked(false);
    _post->setEnabled(false);
  }

  _print->setFocus();
}

printInvoices::~printInvoices()
{
  // no need to delete child widgets, Qt does it all for us
}

void printInvoices::languageChange()
{
  retranslateUi(this);
}

void printInvoices::sPrint()
{
  XSqlQuery invoices;
  invoices.prepare( "SELECT invchead_id, invchead_invcnumber, findCustomerForm(invchead_cust_id, 'I') AS reportname "
                    "FROM ( "
                    "  SELECT * FROM invchead WHERE (NOT invchead_printed)"
                    "  AND   (NOT invchead_posted)) AS data  " 
                    "WHERE (checkInvoiceSitePrivs(invchead_id) ) "
                    "ORDER BY invchead_ordernumber" );
  invoices.exec();
  if (invoices.first())
  {
    XSqlQuery local;
    QPrinter  printer(QPrinter::HighResolution);
    bool      setupPrinter = TRUE;
    bool userCanceled = false;
    if (orReport::beginMultiPrint(&printer, userCanceled) == false)
    {
      if(!userCanceled)
        systemError(this, tr("Could not initialize printing system for multiple reports."));
      return;
    }

    do
    {
      int invoiceNumber = invoices.value("invchead_invcnumber").toInt();
      if (invoiceNumber == 0)
      {
        local.prepare("SELECT fetchInvcNumber() AS invoicenumber;");
        local.exec();
        if (local.first())
        {
          invoiceNumber = local.value("invoicenumber").toInt();
          local.prepare( "UPDATE invchead "
                         "SET invchead_invcnumber=text(:invoicenumber) "
                         "WHERE (invchead_id=:invchead_id);" );
          local.bindValue(":invoicenumber", invoiceNumber);
          local.bindValue(":invchead_id", invoices.value("invchead_id").toInt());
          local.exec();
        }
        else
          systemError(this, tr("A System Error occurred at %1::%2.")
                            .arg(__FILE__)
                            .arg(__LINE__) );
      }

      message( tr("Printing Invoice #%1...")
               .arg(invoiceNumber) );

      for (int i = 0; i < _invoiceWatermarks->topLevelItemCount(); i++ )
      {
	QTreeWidgetItem *cursor = _invoiceWatermarks->topLevelItem(i);
        ParameterList params;
        params.append("invchead_id", invoices.value("invchead_id").toInt());
        params.append("showcosts", ((cursor->text(2) == tr("Yes")) ? "TRUE" : "FALSE"));
        params.append("watermark", cursor->text(1));

        orReport report(invoices.value("reportname").toString(), params);

        message( tr("Printing Invoice #%1...")
                 .arg(invoiceNumber) );

        if (!report.isValid())
        {
          QMessageBox::critical( this, tr("Cannot Find Invoice Form"),
                                 tr( "The Invoice Form '%1' cannot be found for Invoice #%2.\n"
                                     "This Invoice cannot be printed until a Customer Form Assignment is updated to remove any\n"
                                     "references to this Invoice Form or this Invoice Form is created." )
                                 .arg(invoices.value("reportname").toString())
                                 .arg(invoiceNumber) );
          
          resetMessage();
        }
        else
        {
          if (report.print(&printer, setupPrinter))
            setupPrinter = FALSE;
          else
          {
            systemError( this, tr("A Printing Error occurred at printInvoices::%1.")
                               .arg(__LINE__) );
	    orReport::endMultiPrint(&printer);
            return;
          }

          if (_post->isChecked())
          {
            message( tr("Posting Invoice #%1...")
                     .arg(invoiceNumber) );

            local.prepare("SELECT postInvoice(:invchead_id) AS result;");
            local.bindValue(":invchead_id", invoices.value("invchead_id").toInt());
            local.exec();
	    if (local.lastError().type() != QSqlError::NoError)
		systemError(this, local.lastError().databaseText(), __FILE__, __LINE__);
          }
        }
      }

      emit finishedPrinting(invoices.value("invchead_id").toInt());
    }
    while (invoices.next());
    orReport::endMultiPrint(&printer);

    resetMessage();

    if (!_post->isChecked())
    {
      if ( QMessageBox::information( this, tr("Mark Invoices as Printed?"),
                                     tr("Did all of the Invoices print correctly?"),
                                     tr("&Yes"), tr("&No"), QString::null, 0, 1) == 0)
        q.exec( "UPDATE invchead "
                "SET invchead_printed=TRUE "
                "WHERE (NOT invchead_printed);");

        omfgThis->sInvoicesUpdated(-1, TRUE);
    }
  }
  else if (invoices.lastError().type() != QSqlError::NoError)
    systemError(this, invoices.lastError().databaseText(), __FILE__, __LINE__);
  else
    QMessageBox::information( this, tr("No Invoices to Print"),
                              tr("There aren't any Invoices to print.") );

  accept();
}

void printInvoices::sHandleCopies(int pValue)
{
  if (_invoiceWatermarks->topLevelItemCount() > pValue)
    _invoiceWatermarks->takeTopLevelItem(_invoiceWatermarks->topLevelItemCount() - 1);
  else
  {
    for (int i = (_invoiceWatermarks->topLevelItemCount() + 1); i <= pValue; i++)
      new XTreeWidgetItem(_invoiceWatermarks,
			  _invoiceWatermarks->topLevelItem(_invoiceWatermarks->topLevelItemCount() - 1),
			  i, i, tr("Copy #%1").arg(i), "", tr("Yes"));
  }
}

void printInvoices::sEditWatermark()
{
  QTreeWidgetItem *cursor = _invoiceWatermarks->currentItem();
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

