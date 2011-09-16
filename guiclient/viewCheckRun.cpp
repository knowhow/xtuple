/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "viewCheckRun.h"

#include <QSqlError>

#include <metasql.h>
#include <openreports.h>

#include "miscCheck.h"
#include "mqlutil.h"
#include "postCheck.h"
#include "postChecks.h"
#include "prepareCheckRun.h"
#include "printCheck.h"
#include "printChecks.h"
#include "storedProcErrorLookup.h"

viewCheckRun::viewCheckRun(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);  
  
  connect(_check, SIGNAL(currentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)), this, SLOT(sHandleItemSelection()));
  connect(_bankaccnt,     SIGNAL(newID(int)), this, SLOT(sFillList()));
  connect(_delete,         SIGNAL(clicked()), this, SLOT(sDelete()));
  connect(_edit,           SIGNAL(clicked()), this, SLOT(sEdit()));
  connect(_newMiscCheck,   SIGNAL(clicked()), this, SLOT(sNewMiscCheck()));
  connect(_prepareCheckRun,SIGNAL(clicked()), this, SLOT(sPrepareCheckRun()));
  connect(_replace,        SIGNAL(clicked()), this, SLOT(sReplace()));
  connect(_replaceAll,     SIGNAL(clicked()), this, SLOT(sReplaceAll()));
  connect(_vendorgroup,    SIGNAL(updated()), this, SLOT(sFillList()));
  connect(_vendorgroup,    SIGNAL(updated()), this, SLOT(sHandleVendorGroup()));
  connect(_void,           SIGNAL(clicked()), this, SLOT(sVoid()));

  _check->addColumn(tr("Void"),               _ynColumn, Qt::AlignCenter,true, "checkhead_void");
  _check->addColumn(tr("Misc."),              _ynColumn, Qt::AlignCenter,true, "checkhead_misc" );
  _check->addColumn(tr("Prt'd"),              _ynColumn, Qt::AlignCenter,true, "checkhead_printed" );
  _check->addColumn(tr("Chk./Voucher/RA #"),_itemColumn, Qt::AlignCenter,true, "number" );
  _check->addColumn(tr("Recipient/Invc./CM #"),      -1, Qt::AlignLeft,  true, "description"   );
  _check->addColumn(tr("Check Date") ,      _dateColumn, Qt::AlignCenter,true, "checkdate" );
  _check->addColumn(tr("Amount"),          _moneyColumn, Qt::AlignRight, true, "amount"  );
  _check->addColumn(tr("Currency"),     _currencyColumn, Qt::AlignLeft,  true, "currAbbr" );
  if (_metrics->boolean("ACHSupported") && _metrics->boolean("ACHEnabled"))
    _check->addColumn(tr("EFT Batch"),     _orderColumn, Qt::AlignLeft,  true, "checkhead_ach_batch" );

  if (omfgThis->singleCurrency())
      _check->hideColumn("curr_concat");

  connect(omfgThis, SIGNAL(checksUpdated(int, int, bool)), this, SLOT(sFillList(int)));

  sFillList(); 
}

viewCheckRun::~viewCheckRun()
{
  // no need to delete child widgets, Qt does it all for us
}

void viewCheckRun::languageChange()
{
  retranslateUi(this);
}

void viewCheckRun::sVoid()
{
  q.prepare( "SELECT checkhead_bankaccnt_id, voidCheck(checkhead_id) AS result "
             "FROM checkhead "
             "WHERE (checkhead_id=:checkhead_id);" );
  q.bindValue(":checkhead_id", _check->id());
  q.exec();
  if (q.first())
  {
    int result = q.value("result").toInt();
    if (result < 0)
    {
      systemError(this, storedProcErrorLookup("voidCheck", result), __FILE__, __LINE__);
      return;
    }
    omfgThis->sChecksUpdated(q.value("checkhead_bankaccnt_id").toInt(), _check->id(), TRUE);
  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void viewCheckRun::sDelete()
{
  q.prepare( "SELECT checkhead_bankaccnt_id, deleteCheck(checkhead_id) AS result "
             "FROM checkhead "
             "WHERE (checkhead_id=:checkhead_id);" );
  q.bindValue(":checkhead_id", _check->id());
  q.exec();
  if (q.first())
  {
    int result = q.value("result").toInt();
    if (result < 0)
    {
      systemError(this, storedProcErrorLookup("deleteCheck", result), __FILE__, __LINE__);
      return;
    }
    omfgThis->sChecksUpdated(q.value("checkhead_bankaccnt_id").toInt(), _check->id(), TRUE);
  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void viewCheckRun::sNewMiscCheck()
{
  ParameterList params;
  params.append("new");
  params.append("bankaccnt_id", _bankaccnt->id());
  if (_vendorgroup->isSelectedVend())
    params.append("vend_id", _vendorgroup->vendId());

  miscCheck *newdlg = new miscCheck(this, "_miscCheck", Qt::Dialog);
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void viewCheckRun::sEdit()
{
  ParameterList params;
  params.append("edit");
  params.append("check_id", _check->id());

  miscCheck *newdlg = new miscCheck();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void viewCheckRun::sReplace()
{
  q.prepare( "SELECT checkhead_bankaccnt_id, replaceVoidedCheck(:check_id) AS result "
             "FROM checkhead "
             "WHERE (checkhead_id=:check_id);" );
  q.bindValue(":check_id", _check->id());
  q.exec();
  if (q.first())
  {
    int result = q.value("result").toInt();
    if (result < 0)
    {
      systemError(this, storedProcErrorLookup("replaceVoidedCheck", result), __FILE__, __LINE__);
      return;
    }
    omfgThis->sChecksUpdated( q.value("checkhead_bankaccnt_id").toInt(),
                                q.value("result").toInt(), TRUE);
  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void viewCheckRun::sReplaceAll()
{
  q.prepare("SELECT replaceAllVoidedChecks(:bankaccnt_id) AS result;");
  q.bindValue(":bankaccnt_id", _bankaccnt->id());
  q.exec();
  if (q.first())
  {
    int result = q.value("result").toInt();
    if (result < 0)
    {
      systemError(this, storedProcErrorLookup("replaceAllVoidedChecks", result), __FILE__, __LINE__);
      return;
    }
    omfgThis->sChecksUpdated(_bankaccnt->id(), -1, TRUE);
  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void viewCheckRun::sPrint()
{
  ParameterList params;
  params.append("check_id", _check->id());

  printCheck *newdlg = new printCheck();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg, Qt::WindowModal);
}

void viewCheckRun::sPost()
{
  ParameterList params;
  params.append("check_id", _check->id());

  postCheck newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void viewCheckRun::sHandleItemSelection()
{
  XTreeWidgetItem *selected = _check->currentItem();
  bool select = false;

  if (! selected)
  {
    _void->setEnabled(FALSE);
    _delete->setEnabled(FALSE);
    _replace->setEnabled(FALSE);
    select = true;

    _edit->setEnabled(FALSE);

    return;
  }

  if (selected->rawValue("checkhead_void").toBool())
  {
    _void->setEnabled(FALSE);
    _delete->setEnabled(TRUE);
    _replace->setEnabled(TRUE);

    _edit->setEnabled(FALSE);
  }
  else if (! selected->rawValue("checkhead_void").isNull() &&
           ! selected->rawValue("checkhead_void").toBool())
  {
    // This was not allowing voiding of ACH checks. No strong case could be
    // made to disallow this since ACH is manuall at this time. Should that
    // requirement change this is the original line
    //_void->setEnabled(selected->rawValue("checkhead_ach_batch").isNull());
    _void->setEnabled(true);
    _delete->setEnabled(FALSE);
    _replace->setEnabled(FALSE);
    select = selected->rawValue("checkhead_ach_batch").isNull();

    _edit->setEnabled(selected->rawValue("checkhead_misc").toBool() &&
                      ! selected->rawValue("checkhead_printed").toBool());
  }
  
  QMenu * printMenu = new QMenu;
  if (select)
    printMenu->addAction(tr("Selected Check..."), this, SLOT(sPrint()));
  if (_vendorgroup->isAll())
    printMenu->addAction(tr("Check Run..."), this, SLOT(sPrintCheckRun()));
  printMenu->addAction(tr("Edit List"), this, SLOT(sPrintEditList()));
  _print->setMenu(printMenu); 

  QMenu * postMenu = new QMenu;
  if (selected->rawValue("checkhead_printed").toBool() &&
      _privileges->check("PostPayments"))
    postMenu->addAction(tr("Selected Check..."), this, SLOT(sPost()));
  if (_vendorgroup->isAll())
    postMenu->addAction(tr("All Checks..."), this, SLOT(sPostChecks()));
  _postCheck->setMenu(postMenu); 
}

void viewCheckRun::sFillList(int pBankaccntid)
{
  if (pBankaccntid == _bankaccnt->id())
    sFillList();
}

void viewCheckRun::sFillList()
{
  QMenu * printMenu = new QMenu;
  if (_vendorgroup->isAll())
    printMenu->addAction(tr("Check Run..."), this, SLOT(sPrintCheckRun()));
  printMenu->addAction(tr("Edit List"),   this, SLOT(sPrintEditList()));
  _print->setMenu(printMenu);   

  QMenu * postMenu = new QMenu;
  if (_vendorgroup->isAll())
    postMenu->addAction(tr("Post All..."), this, SLOT(sPostChecks()));
  _postCheck->setMenu(postMenu); 
  
  MetaSQLQuery mql = mqlLoad("checkRegister", "detail");
  ParameterList params;
  params.append("bankaccnt_id", _bankaccnt->id());
  params.append("showTotal");
  params.append("newOnly");
  params.append("showDetail");
  _vendorgroup->appendValue(params);
  q = mql.toQuery(params);
  _check->populate(q);
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void viewCheckRun::sPrintEditList()
{
  ParameterList params;
  params.append("bankaccnt_id", _bankaccnt->id()); 
  _vendorgroup->appendValue(params);
    
  orReport report("ViewAPCheckRunEditList", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void viewCheckRun::sPrintCheckRun()
{
  ParameterList params;
  params.append("bankaccnt_id", _bankaccnt->id()); 

  printChecks newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.setWindowModality(Qt::WindowModal);
  newdlg.exec();
}

void viewCheckRun::sPostChecks()
{
  ParameterList params;
  params.append("bankaccnt_id", _bankaccnt->id()); 

  postChecks newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.setWindowModality(Qt::WindowModal);
  newdlg.exec();
}

void viewCheckRun::sHandleVendorGroup()
{
  _replaceAll->setEnabled(_vendorgroup->isAll());
}

void viewCheckRun::sPrepareCheckRun()
{
  ParameterList params;
  params.append("bankaccnt_id", _bankaccnt->id()); 

  prepareCheckRun newdlg(this, "", true);
  newdlg.set(params);
  newdlg.exec();
}
