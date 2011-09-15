/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "accountNumbers.h"

#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include <metasql.h>
#include <mqlutil.h>
#include <openreports.h>

#include "accountNumber.h"
#include "storedProcErrorLookup.h"

accountNumbers::accountNumbers(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

//  (void)statusBar();

  connect(_account,        SIGNAL(valid(bool)), this, SLOT(sHandleButtons()));
  connect(_delete,           SIGNAL(clicked()), this, SLOT(sDelete()));
  connect(_edit,             SIGNAL(clicked()), this, SLOT(sEdit()));
  connect(_new,              SIGNAL(clicked()), this, SLOT(sNew()));
  connect(_print,            SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_showExternal, SIGNAL(toggled(bool)), this, SLOT(sBuildList()));
  connect(_showInactive, SIGNAL(toggled(bool)), this, SLOT(sFillList()));

  connect(omfgThis, SIGNAL(configureGLUpdated()), this, SLOT(sBuildList()));

  _showExternal->setVisible(_metrics->boolean("MultiCompanyFinancialConsolidation"));

  _externalCol = -1;

  sBuildList();
}

accountNumbers::~accountNumbers()
{
  // no need to delete child widgets, Qt does it all for us
}

void accountNumbers::languageChange()
{
  retranslateUi(this);
}

void accountNumbers::sDelete()
{
  q.prepare("SELECT deleteAccount(:accnt_id) AS result;");
  q.bindValue(":accnt_id", _account->id());
  q.exec();
  if (q.first())
  {
    int result = q.value("result").toInt();
    if (result < 0)
    {
      systemError(this, storedProcErrorLookup("deleteAccount", result), __FILE__, __LINE__);
      return;
    }
    sFillList();
  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void accountNumbers::sNew()
{
  ParameterList params;
  params.append("mode", "new");

  accountNumber newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void accountNumbers::sEdit()
{
  ParameterList params;
  // don't allow editing external accounts
  if (_externalCol >= 0 &&
      _account->currentItem()->rawValue("company_external").toBool())
    params.append("mode", "view");
  else
    params.append("mode", "edit");
  params.append("accnt_id", _account->id());

  accountNumber newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

bool accountNumbers::setParams(ParameterList &pParams)
{
  if (_metrics->boolean("MultiCompanyFinancialConsolidation") &&
      _showExternal->isChecked())
    pParams.append("showExternal");
  if (_showInactive->isChecked())
    pParams.append("showInactive");

  pParams.append("asset",     tr("Asset"));
  pParams.append("expense",   tr("Expense"));
  pParams.append("liability", tr("Liability"));
  pParams.append("equity",    tr("Equity"));
  pParams.append("revenue",   tr("Revenue"));

  return true;
}

void accountNumbers::sPrint()
{
  ParameterList params;
  if (! setParams(params))
    return;

  orReport report("AccountNumberMasterList");
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void accountNumbers::sFillList()
{
  ParameterList params;
  if (! setParams(params))
    return;

  bool ok = true;
  QString errorString;
  MetaSQLQuery mql = MQLUtil::mqlLoad("accountNumbers", "detail", errorString, &ok);
  if(!ok)
  {
    systemError(this, errorString, __FILE__, __LINE__);
    return;
  }
  q = mql.toQuery(params);

  _account->populate(q);
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void accountNumbers::sBuildList()
{
  _account->setColumnCount(0);
  _externalCol = -1;

  if (_metrics->value("GLCompanySize").toInt() > 0)
  {
    _account->addColumn(tr("Company"),       50, Qt::AlignCenter, true, "accnt_company");
    _externalCol++;
  }

  if (_metrics->value("GLProfitSize").toInt() > 0)
  {
    _account->addColumn(tr("Profit"),        50, Qt::AlignCenter, true, "accnt_profit");
    _externalCol++;
  }

  _account->addColumn(tr("Main Segment"), 100, Qt::AlignCenter, true, "accnt_number");
  _externalCol++;

  if (_metrics->value("GLSubaccountSize").toInt() > 0)
  {
    _account->addColumn(tr("Sub."),          50, Qt::AlignCenter, true, "accnt_sub");
    _externalCol++;
  }

  _account->addColumn(tr("Description"),     -1, Qt::AlignLeft,   true, "accnt_descrip");
  _externalCol++;

  _account->addColumn(tr("Type"),            75, Qt::AlignLeft ,  true, "accnt_type");
  _externalCol++;

  _account->addColumn(tr("Sub. Type Code"),  75, Qt::AlignLeft,  false, "subaccnttype_code");
  _externalCol++;

  _account->addColumn(tr("Sub. Type"),      100, Qt::AlignLeft,  false, "subaccnttype_descrip");
  _externalCol++;

  _account->addColumn(tr("Active"),          75, Qt::AlignLeft,  false, "accnt_active");
  _externalCol++;

  if (_metrics->value("Application") != "PostBooks")
  {
    _account->addColumn(tr("External"), _ynColumn, Qt::AlignCenter, false, "company_external");
    _externalCol++;
  }
  else
    _externalCol = -1;

  sFillList();
}

void accountNumbers::sHandleButtons()
{
  // don't allow editing external accounts
  if (_externalCol >= 0 && _account->currentItem() &&
      _account->currentItem()->rawValue("company_external").toBool())
    _edit->setText(tr("View"));
  else
    _edit->setText(tr("Edit"));
}
