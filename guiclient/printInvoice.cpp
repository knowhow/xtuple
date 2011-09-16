/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "printInvoice.h"

#include <QApplication>
#include <QMessageBox>
#include <QSqlError>
#include <QValidator>
#include <QVariant>

#include <openreports.h>

#include "editICMWatermark.h"
#include "storedProcErrorLookup.h"
#include "distributeInventory.h"
#include "submitAction.h"

printInvoice::printInvoice(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_invoiceNumOfCopies, SIGNAL(valueChanged(int)), this, SLOT(sHandleCopies(int)));
  connect(_invoiceWatermarks, SIGNAL(itemSelected(int)), this, SLOT(sEditWatermark()));

  _captive = FALSE;
  _setup   = FALSE;
  _alert   = TRUE;
  _printer = new QPrinter(QPrinter::HighResolution);

  _cust->setReadOnly(TRUE);

  _invoiceWatermarks->addColumn( tr("Copy #"),      _dateColumn, Qt::AlignCenter );
  _invoiceWatermarks->addColumn( tr("Watermark"),   -1,          Qt::AlignLeft   );
  _invoiceWatermarks->addColumn( tr("Show Prices"), _dateColumn, Qt::AlignCenter );

  _invoiceNumOfCopies->setValue(_metrics->value("InvoiceCopies").toInt() + 1);
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

printInvoice::~printInvoice()
{
  if (_captive)	// see sPrint()
    orReport::endMultiPrint(_printer);

  if (_printer)
  {
    delete _printer;
    _printer = 0;
  }
}

void printInvoice::languageChange()
{
  retranslateUi(this);
}

enum SetResponse printInvoice::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  _captive = TRUE;

  QVariant param;
  bool     valid;

  param = pParams.value("invchead_id", &valid);
  if (valid)
  {
    _invcheadid = param.toInt();
    populate();
  }

  if (pParams.inList("print"))
  {
    sPrint();
    return NoError_Print;
  }

  if (pParams.inList("persistentPrint"))
  {
    _alert = FALSE;

    if (_setup)
    {
      sPrint();
      return NoError_Print;
    }
    else
      return Error_NoSetup;
  }

  return NoError;
}

void printInvoice::sPrint()
{
  q.prepare( "SELECT invchead_invcnumber, invchead_printed, invchead_posted,"
             "       findCustomerForm(invchead_cust_id, 'I') AS reportname "
             "FROM invchead "
             "WHERE (invchead_id=:invchead_id);" );
  q.bindValue(":invchead_id", _invcheadid);
  q.exec();
  if (q.first())
  {
    XSqlQuery xrate;
    xrate.prepare("SELECT curr_rate "
		  "FROM curr_rate, invchead "
		  "WHERE ((curr_id=invchead_curr_id)"
		  "  AND  (invchead_id=:invchead_id)"
		  "  AND  (invchead_invcdate BETWEEN curr_effective AND curr_expires));");
    // if SUM becomes dependent on curr_id then move XRATE before it in the loop
    XSqlQuery sum;
    sum.prepare("SELECT invoiceTotal(:invchead_id) AS subtotal;");
    QString reportname = q.value("reportname").toString();
    QString invchead_invcnumber = q.value("invchead_invcnumber").toString();

    bool dosetup = (!_setup);
    if (dosetup)
    {
      bool userCanceled = false;
      if (orReport::beginMultiPrint(_printer, userCanceled) == false)
      {
	if(!userCanceled)
          systemError(this, tr("Could not initialize printing system for multiple reports."));
	return;
      }
    }

    for (int i = 0; i < _invoiceWatermarks->topLevelItemCount(); i++ )
    {
      QTreeWidgetItem *cursor = _invoiceWatermarks->topLevelItem(i);
      ParameterList params;
      params.append("invchead_id", _invcheadid);
      params.append("showcosts", ((cursor->text(2) == tr("Yes")) ? "TRUE" : "FALSE"));
      params.append("watermark", cursor->text(1));

      orReport report(reportname, params);
      if (!report.isValid())
        QMessageBox::critical( this, tr("Cannot Find Invoice Form"),
                               tr( "The Invoice Form '%1' cannot be found for Invoice #%2.\n"
                                   "This Invoice cannot be printed until a Customer Form Assignment is updated to remove any\n"
                                   "references to this Invoice Form or this Invoice Form is created." )
                               .arg(reportname)
                               .arg(invchead_invcnumber) );
          
      else
      {
        if (report.print(_printer, dosetup))
          dosetup = FALSE;
        else
        {
          systemError( this, tr("A Printing Error occurred at printInvoice::%1.")
                             .arg(__LINE__) );
	  if (!_captive)
	    orReport::endMultiPrint(_printer);
          return;
        }
      }
    }

    emit finishedPrinting(_invcheadid);

    if (!_captive)
      orReport::endMultiPrint(_printer);

    q.prepare( "UPDATE invchead "
               "SET invchead_printed=TRUE "
               "WHERE (invchead_id=:invchead_id);" );
    q.bindValue(":invchead_id", _invcheadid);
    q.exec();

    if (_alert)
      omfgThis->sInvoicesUpdated(_invcheadid, TRUE);

    if (_post->isChecked())
    {
      sum.bindValue(":invchead_id", _invcheadid);
      if (sum.exec() && sum.first() && sum.value("subtotal").toDouble() == 0)
      {
        if (QMessageBox::question(this, tr("Invoice Has Value 0"),
                                        tr("Invoice #%1 has a total value of 0.\n"
                                           "Would you like to post it anyway?")
					.arg(invchead_invcnumber),
				      QMessageBox::Yes,
				      QMessageBox::No | QMessageBox::Default)
		  == QMessageBox::No)
            return;
      }
      else if (sum.lastError().type() != QSqlError::NoError)
      {
        systemError(this, sum.lastError().databaseText(), __FILE__, __LINE__);
        return;
      }
      else if (sum.value("subtotal").toInt() != 0)
      {
        xrate.bindValue(":invchead_id", _invcheadid);
        xrate.exec();
        if (xrate.lastError().type() != QSqlError::NoError)
        {
          systemError(this, tr("System Error posting Invoice #%1\n%2")
                                  .arg(invchead_invcnumber)
				  .arg(xrate.lastError().databaseText()),
                          __FILE__, __LINE__);
          return;
        }
        else if (!xrate.first() || xrate.value("curr_rate").isNull())
        {
          systemError(this, tr("Could not post Invoice #%1 because of a missing exchange rate.")
                                  .arg(invchead_invcnumber));
          return;
        }
      }

      q.exec("BEGIN;");
      //TO DO:  Replace this method with commit that doesn't require transaction
      //block that can lead to locking issues
      XSqlQuery rollback;
      rollback.prepare("ROLLBACK;");

      q.prepare("SELECT postInvoice(:invchead_id) AS result;");
      q.bindValue(":invchead_id", _invcheadid);
      q.exec();
      if (q.first())
      {
        int result = q.value("result").toInt();
        if (result < 0)
        {
          rollback.exec();
          systemError( this, storedProcErrorLookup("postInvoice", result),
                    __FILE__, __LINE__);
          return;
        }
        else
        {
          if (distributeInventory::SeriesAdjust(result, this) == XDialog::Rejected)
          {
            rollback.exec();
            QMessageBox::information( this, tr("Post Invoice"), tr("Transaction Canceled") );
            return;
          }
        }
      }
      else if (q.lastError().type() != QSqlError::NoError)
      {
        systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
        rollback.exec();
        return;
      }

      q.exec("COMMIT;");
    }

    if (_captive)
      accept();
  }
  else if (q.lastError().type() != QSqlError::NoError)
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
}

void printInvoice::sHandleCopies(int pValue)
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

void printInvoice::sEditWatermark()
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

void printInvoice::populate()
{
  q.prepare( "SELECT invchead_invcnumber, invchead_cust_id, invchead_posted "
             "FROM invchead "
             "WHERE (invchead_id=:invchead_id);" );
  q.bindValue(":invchead_id", _invcheadid);
  q.exec();
  if (q.first())
  {
    _invoiceNum->setText(q.value("invchead_invcnumber").toString());
    _cust->setId(q.value("invchead_cust_id").toInt());
    if (q.value("invchead_posted").toBool())
    {
      _post->setChecked(FALSE);
      _post->hide();
    }
    else
      _post->show();
  }
}

int printInvoice::id()
{
  return _invcheadid;
}

bool printInvoice::isSetup()
{
  return _setup;
}

void printInvoice::setSetup(bool pSetup)
{
  _setup = pSetup;
}
