/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "printStatementByCustomer.h"

#include <QVariant>
#include <QMessageBox>
#include <openreports.h>

printStatementByCustomer::printStatementByCustomer(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
  : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  _asOf->setDate(omfgThis->dbDate(), true);
  // signals and slots connections
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_close, SIGNAL(clicked()), this, SLOT(reject()));
  connect(_cust, SIGNAL(valid(bool)), _print, SLOT(setEnabled(bool)));

  _captive = FALSE;

  _cust->setFocus();
}

printStatementByCustomer::~printStatementByCustomer()
{
  // no need to delete child widgets, Qt does it all for us
}

void printStatementByCustomer::languageChange()
{
  retranslateUi(this);
}

enum SetResponse printStatementByCustomer::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  _captive = TRUE;

  QVariant param;
  bool     valid;

  param = pParams.value("cust_id", &valid);
  if (valid)
    _cust->setId(param.toInt());

  if (pParams.inList("print"))
  {
    sPrint();
    return NoError_Print;
  }

  return NoError;
}

void printStatementByCustomer::sPrint()
{
  if (!_cust->isValid())
  {
    QMessageBox::warning( this, tr("Enter a Valid Customer Number"),
                          tr("You must enter a valid Customer Number for this Statement.") );
    _cust->setFocus();
    return;
  }
  q.prepare("SELECT * FROM araging (:asofDate, true) "
            "WHERE (araging_cust_id = :cust_id);"); 
  q.bindValue(":cust_id", _cust->id());
  q.bindValue(":asofDate", _asOf->date());
  q.exec();
  if(q.first())
  {
    q.prepare("SELECT findCustomerForm(:cust_id, 'S') AS _reportname;");
    q.bindValue(":cust_id", _cust->id());
    q.exec();
    if (q.first())
    {
      ParameterList params;
      params.append("invoice", tr("Invoice"));
      params.append("debit", tr("Debit Memo"));
      params.append("credit", tr("Credit Memo"));
      params.append("deposit", tr("Deposit"));
      params.append("cust_id", _cust->id());
      params.append("asofdate", _asOf->date());

      orReport report(q.value("_reportname").toString(), params);
      if (report.isValid())
        report.print();
      else
      {
        report.reportError(this);
        reject();
      }

      if (_captive)
        accept();
      else
      {
        _close->setText(tr("&Close"));
        _cust->setId(-1);
        _cust->setFocus();
      }
    }
    else
      systemError(this, tr("A System Error occurred at %1::%2.")
                      .arg(__FILE__)
                      .arg(__LINE__) );
  }
  else
    QMessageBox::warning( this, tr("No Statement to Print"),
                          tr("No statement is available for the specified "   
                                        "Customer and Asof Date.") );
}
