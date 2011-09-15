/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "printCreditMemo.h"

#include <QApplication>
#include <QMessageBox>
#include <QSqlError>
#include <QStatusBar>
#include <QValidator>
#include <QVariant>

#include <openreports.h>
#include "editICMWatermark.h"
#include "storedProcErrorLookup.h"
#include "distributeInventory.h"

printCreditMemo::printCreditMemo(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
    setupUi(this);

    connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
    connect(_close, SIGNAL(clicked()), this, SLOT(reject()));
    connect(_numberOfCopies, SIGNAL(valueChanged(int)), this, SLOT(sHandleCopies(int)));
    connect(_watermarks, SIGNAL(itemSelected(int)), this, SLOT(sEditWatermark()));

    _captive = FALSE;
    _setup   = FALSE;
    _alert   = TRUE;

    _cust->setReadOnly(TRUE);

    _watermarks->addColumn( tr("Copy #"),      _dateColumn, Qt::AlignCenter );
    _watermarks->addColumn( tr("Watermark"),   -1,          Qt::AlignLeft   );
    _watermarks->addColumn( tr("Show Prices"), _dateColumn, Qt::AlignCenter );

    _numberOfCopies->setValue(_metrics->value("CreditMemoCopies").toInt() + 1);
    if (_numberOfCopies->value())
    {
      for (int i = 0; i < _watermarks->topLevelItemCount(); i++)
      {
	_watermarks->topLevelItem(i)->setText(1, _metrics->value(QString("CreditMemoWatermark%1").arg(i)));
	_watermarks->topLevelItem(i)->setText(2, ((_metrics->value(QString("CreditMemoShowPrices%1").arg(i)) == "t") ? tr("Yes") : tr("No")));
      }
    }

    if(!_privileges->check("PostARDocuments"))
    {
      _post->setChecked(false);
      _post->setEnabled(false);
    }

    _print->setFocus();
}

printCreditMemo::~printCreditMemo()
{
  if (_captive)	// see sPrint()
    orReport::endMultiPrint(&_printer);
}

void printCreditMemo::languageChange()
{
    retranslateUi(this);
}

enum SetResponse printCreditMemo::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  _captive = TRUE;

  QVariant param;
  bool     valid;

  param = pParams.value("cmhead_id", &valid);
  if (valid)
  {
    _cmheadid = param.toInt();
    populate();
  }

  param = pParams.value("posted", &valid);
  if (valid)
    _post->hide();

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

void printCreditMemo::sPrint()
{
  q.prepare( "SELECT cmhead_number, cmhead_printed, cmhead_posted,"
             "       findCustomerForm(cmhead_cust_id, 'C') AS reportname "
             "FROM cmhead "
             "WHERE (cmhead_id=:cmhead_id);" );
  q.bindValue(":cmhead_id", _cmheadid);
  q.exec();
  if (q.first())
  {
    QString reportname = q.value("reportname").toString();
    QString cmheadnumber = q.value("cmhead_number").toString();

    bool dosetup = (!_setup);
    if (dosetup)
    {
      bool userCanceled = false;
      if (orReport::beginMultiPrint(&_printer, userCanceled) == false)
      {
        if(!userCanceled)
	  systemError(this, tr("Could not initialize printing system for multiple reports."));
	return;
      }
    }


    for (int i = 0; i < _watermarks->topLevelItemCount(); i++ )
    {
      ParameterList params;
      params.append("cmhead_id", _cmheadid);
      params.append("showcosts", ((_watermarks->topLevelItem(i)->text(2) == tr("Yes")) ? "TRUE" : "FALSE"));
      params.append("watermark", _watermarks->topLevelItem(i)->text(1));

      orReport report(reportname, params);
      if (!report.isValid())
        QMessageBox::critical( this, tr("Cannot Find Credit Memo Form"),
                               tr( "The Credit Memo Form '%1' cannot be found for Credit Memo #%2.\n"
                                   "This Credit Memo cannot be printed until a Customer Form Assignment is updated to remove any\n"
                                   "references to this Credit Memo Form or this Credit Memo Form is created." )
                               .arg(reportname)
                               .arg(cmheadnumber) );
          
      else
      {
        if (report.print(&_printer, dosetup))
          dosetup = FALSE;
        else
        {
          systemError( this, tr("A Printing Error occurred at printCreditMemo::%1.")
                             .arg(__LINE__) );
	  if (!_captive)
	    orReport::endMultiPrint(&_printer);
          return;
        }
      }
    }

    q.prepare( "UPDATE cmhead "
               "SET cmhead_printed=TRUE "
               "WHERE (cmhead_id=:cmhead_id);" );
    q.bindValue(":cmhead_id", _cmheadid);
    q.exec();
    if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }

    if (_alert)
      omfgThis->sCreditMemosUpdated();

    if (_post->isChecked())
    {
      q.exec("BEGIN;");
      //TO DO:  Replace this method with commit that doesn't require transaction
      //block that can lead to locking issues
      XSqlQuery rollback;
      rollback.prepare("ROLLBACK;");

      q.prepare("SELECT postCreditMemo(:cmhead_id, 0) AS result;");
      q.bindValue(":cmhead_id", _cmheadid);
      q.exec();
      q.first();
      int result = q.value("result").toInt();
      if (result < 0)
      {
        rollback.exec();
        systemError( this, storedProcErrorLookup("postCreditMemo", result),
              __FILE__, __LINE__);
        return;
      }
      else if (q.lastError().type() != QSqlError::NoError)
      {
        systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
        rollback.exec();
        return;
      }
      else
      {
        if (distributeInventory::SeriesAdjust(result, this) == XDialog::Rejected)
        {
          rollback.exec();
          QMessageBox::information( this, tr("Post Credit Memo"), tr("Transaction Canceled") );
          return;
        }

        q.exec("COMMIT;");
      }
    }

    emit finishedPrinting(_cmheadid);
  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  if (_captive)
    accept();
  else
    orReport::endMultiPrint(&_printer);
}

void printCreditMemo::sHandleCopies(int pValue)
{
  if (_watermarks->topLevelItemCount() > pValue)
    _watermarks->takeTopLevelItem(_watermarks->topLevelItemCount() - 1);
  else
  {
    for (int i = (_watermarks->topLevelItemCount() + 1); i <= pValue; i++)
      new XTreeWidgetItem(_watermarks,
			  _watermarks->topLevelItem(_watermarks->topLevelItemCount() - 1),
			  i, i,
			  tr("Copy #%1").arg(i), "", tr("Yes"));
  }
}

void printCreditMemo::sEditWatermark()
{
  QTreeWidgetItem *cursor = _watermarks->currentItem();
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

void printCreditMemo::populate()
{
  q.prepare( "SELECT cmhead_number, cmhead_cust_id, cmhead_posted "
             "FROM cmhead "
             "WHERE (cmhead_id=:cmhead_id);" );
  q.bindValue(":cmhead_id", _cmheadid);
  q.exec();
  if (q.first())
  {
    _number->setText(q.value("cmhead_number").toString());
    _cust->setId(q.value("cmhead_cust_id").toInt());
    if (q.value("cmhead_posted").toBool())
    {
      _post->setChecked(FALSE);
      _post->hide();
    }
    else
      _post->show();
      
  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

bool printCreditMemo::isSetup()
{
  return _setup;
}

void printCreditMemo::setSetup(bool pSetup)
{
  _setup = pSetup;
}

