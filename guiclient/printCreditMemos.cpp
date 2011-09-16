/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "printCreditMemos.h"

#include <QMessageBox>
#include <QSqlError>
#include <QValidator>
#include <QVariant>

#include <parameter.h>
#include <openreports.h>

#include "editICMWatermark.h"
#include "guiclient.h"
#include "storedProcErrorLookup.h"
#include "distributeInventory.h"

printCreditMemos::printCreditMemos(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_creditMemoNumOfCopies, SIGNAL(valueChanged(int)), this, SLOT(sHandleCopies(int)));
  connect(_creditMemoWatermarks, SIGNAL(itemSelected(int)), this, SLOT(sEditWatermark()));

  _creditMemoWatermarks->addColumn( tr("Copy #"),      _dateColumn, Qt::AlignCenter );
  _creditMemoWatermarks->addColumn( tr("Watermark"),   -1,          Qt::AlignLeft   );
  _creditMemoWatermarks->addColumn( tr("Show Prices"), _dateColumn, Qt::AlignCenter );
  
  _creditMemoNumOfCopies->setValue(_metrics->value("CreditMemoCopies").toInt() + 1);
  if (_creditMemoNumOfCopies->value())
  {
    for (int i = 0; i < _creditMemoWatermarks->topLevelItemCount(); i++)
    {
      _creditMemoWatermarks->topLevelItem(i)->setText(1, _metrics->value(QString("CreditMemoWatermark%1").arg(i)));
      _creditMemoWatermarks->topLevelItem(i)->setText(2, ((_metrics->value(QString("CreditMemoShowPrices%1").arg(i)) == "t") ? tr("Yes") : tr("No")));
    }
  }

  if(!_privileges->check("PostARDocuments"))
  {
    _post->setChecked(false);
    _post->setEnabled(false);
  }
  
  _print->setFocus();
}

printCreditMemos::~printCreditMemos()
{
    // no need to delete child widgets, Qt does it all for us
}

void printCreditMemos::languageChange()
{
    retranslateUi(this);
}

void printCreditMemos::sPrint()
{
  XSqlQuery cmhead( "SELECT cmhead_id, cmhead_number,"
                    "       findCustomerForm(cmhead_cust_id, 'C') AS _reportname "
                    "FROM ( SELECT cmhead_id, cmhead_number, cmhead_cust_id "
                    "       FROM cmhead "
                    "       WHERE ( (NOT cmhead_hold)"
                    "         AND   (NOT COALESCE(cmhead_printed,false)) ) ) AS data "
                    "WHERE (checkCreditMemoSitePrivs(cmhead_id));" );
  if (cmhead.first())
  {
    QPrinter printer(QPrinter::HighResolution);
    bool     setupPrinter  = TRUE;
    bool userCanceled = false;
    if (orReport::beginMultiPrint(&printer, userCanceled) == false)
    {
      if(!userCanceled)
        systemError(this, tr("Could not initialize printing system for multiple reports."));
      return;
    }

    do
    {
      for (int i = 0; i < _creditMemoWatermarks->topLevelItemCount(); i++ )
      {
        ParameterList params;

        params.append("cmhead_id", cmhead.value("cmhead_id").toInt());
        params.append("showcosts", ((_creditMemoWatermarks->topLevelItem(i)->text(2) == tr("Yes")) ? "TRUE" : "FALSE"));
        params.append("watermark", _creditMemoWatermarks->topLevelItem(i)->text(1));

        orReport report(cmhead.value("_reportname").toString(), params);
        if (!report.isValid())
          QMessageBox::critical( this, tr("Cannot Find Credit Memo Form"),
                                 tr("<p>The Credit Memo Form '%1' for Credit "
                                    "Memo #%2 cannot be found. This Credit "
                                    "Memo cannot be printed until Customer "
                                    "Form Assignments are updated to remove "
                                    "any references to this Credit Memo Form "
                                    "or this Credit Memo Form is created." )
                                 .arg(cmhead.value("_reportname").toString())
                                 .arg(cmhead.value("cmhead_number").toString()) );
        else
        {
          if (report.print(&printer, setupPrinter))
            setupPrinter = FALSE;
          else
          {
            systemError( this, tr("A Printing Error occurred at printCreditMemos::%1.")
                               .arg(__LINE__) );
	    orReport::endMultiPrint(&printer);
            return;
          }
        }

        if ( (_post->isChecked()) && (i == 0) )
        {
          q.exec("BEGIN;");
          //TO DO:  Replace this method with commit that doesn't require transaction
          //block that can lead to locking issues
          XSqlQuery rollback;
          rollback.prepare("ROLLBACK;");

          q.prepare("SELECT postCreditMemo(:cmhead_id, 0) AS result;");
          q.bindValue(":cmhead_id", cmhead.value("cmhead_id").toInt());
          q.exec();
          q.first();
          int result = q.value("result").toInt();
          if (result < 0)
          {
            rollback.exec();
            systemError( this, storedProcErrorLookup("postCreditMemo", result),
                  __FILE__, __LINE__);
          }
          else if (q.lastError().type() != QSqlError::NoError)
          {
            systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
            rollback.exec();
          }
          else
          {
            if (distributeInventory::SeriesAdjust(result, this) == XDialog::Rejected)
            {
              rollback.exec();
              QMessageBox::information( this, tr("Post Credit Memo"), tr("Transaction Canceled") );
            }

            q.exec("COMMIT;");
          }
        }
      }

      emit finishedPrinting(cmhead.value("cmhead_id").toInt());
    }
    while (cmhead.next());
    orReport::endMultiPrint(&printer);

    if (QMessageBox::question(this, tr("Mark Credit Memos as Printed?"),
                              tr("<p>Did all of the Credit Memos print "
                                 "correctly?"),
                              QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes)
      XSqlQuery().exec( "UPDATE cmhead "
                        "SET cmhead_printed=TRUE "
                        "WHERE (NOT COALESCE(cmhead_printed,false));" );

    omfgThis->sCreditMemosUpdated();
  }
  else
    QMessageBox::information( this, tr("No Credit Memos to Print"),
                              tr("There aren't any Credit Memos to print.") );

  accept();
}

void printCreditMemos::sHandleCopies(int pValue)
{
  if (_creditMemoWatermarks->topLevelItemCount() > pValue)
    _creditMemoWatermarks->takeTopLevelItem(_creditMemoWatermarks->topLevelItemCount() - 1);
  else
  {
    for (int i = (_creditMemoWatermarks->topLevelItemCount() + 1); i <= pValue; i++)
      new XTreeWidgetItem(_creditMemoWatermarks,
			  _creditMemoWatermarks->topLevelItem(_creditMemoWatermarks->topLevelItemCount() - 1),
			  i, i,
			  tr("Copy #%1").arg(i), "", tr("Yes"));
  }
}

void printCreditMemos::sEditWatermark()
{
  QTreeWidgetItem *cursor = _creditMemoWatermarks->currentItem();
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
