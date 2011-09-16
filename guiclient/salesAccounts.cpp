/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "salesAccounts.h"

#include <QMessageBox>
#include <QSqlError>

#include <parameter.h>
#include <openreports.h>

#include "salesAccount.h"
#include "guiclient.h"

salesAccounts::salesAccounts(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_new, SIGNAL(clicked()), this, SLOT(sNew()));
  connect(_edit, SIGNAL(clicked()), this, SLOT(sEdit()));
  connect(_delete, SIGNAL(clicked()), this, SLOT(sDelete()));
  connect(_view, SIGNAL(clicked()), this, SLOT(sView()));

  _salesaccnt->addColumn(tr("Site"),            -1,          Qt::AlignCenter            , true, "warehouscode");
  _salesaccnt->addColumn(tr("Cust. Type"),      _itemColumn, Qt::AlignCenter            , true, "custtypecode");
  _salesaccnt->addColumn(tr("Prod. Cat."),      _itemColumn, Qt::AlignCenter            , true, "prodcatcode");
  _salesaccnt->addColumn(tr("Sales Accnt. #"),  _itemColumn, Qt::AlignCenter            , true, "salesaccount");
  _salesaccnt->addColumn(tr("Credit Accnt. #"), _itemColumn, Qt::AlignCenter            , true, "creditaccount");
  _salesaccnt->addColumn(tr("COS Accnt. #"),    _itemColumn, Qt::AlignCenter            , true, "cosaccount");
  _salesaccnt->addColumn(tr("Returns Accnt. #"), _itemColumn, Qt::AlignCenter           , true, "returnsaccount");
  _salesaccnt->addColumn(tr("Cost of Returns Accnt. #"),  _itemColumn, Qt::AlignCenter  , true, "coraccount" );
  _salesaccnt->addColumn(tr("Cost of Warranty Accnt. #"), _itemColumn, Qt::AlignCenter  , true, "cowaccount" );

  if (! _metrics->boolean("EnableReturnAuth"))
  {
    _salesaccnt->hideColumn(6);
    _salesaccnt->hideColumn(7);
    _salesaccnt->hideColumn(8);
  }

  if (_privileges->check("MaintainSalesAccount"))
  {
    connect(_salesaccnt, SIGNAL(valid(bool)), _edit, SLOT(setEnabled(bool)));
    connect(_salesaccnt, SIGNAL(valid(bool)), _delete, SLOT(setEnabled(bool)));
    connect(_salesaccnt, SIGNAL(itemSelected(int)), _edit, SLOT(animateClick()));
  }
  else
  {
    _new->setEnabled(FALSE);
    connect(_salesaccnt, SIGNAL(itemSelected(int)), _view, SLOT(animateClick()));
  }

  sFillList();
}

salesAccounts::~salesAccounts()
{
  // no need to delete child widgets, Qt does it all for us
}

void salesAccounts::languageChange()
{
  retranslateUi(this);
}

void salesAccounts::sPrint()
{
  orReport report("SalesAccountAssignmentsMasterList");
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void salesAccounts::sNew()
{
  ParameterList params;
  params.append("mode", "new");

  salesAccount newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void salesAccounts::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("salesaccnt_id", _salesaccnt->id());

  salesAccount newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void salesAccounts::sView()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("salesaccnt_id", _salesaccnt->id());

  salesAccount newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void salesAccounts::sDelete()
{
  q.prepare( "DELETE FROM salesaccnt "
             "WHERE (salesaccnt_id=:salesaccnt_id);" );
  q.bindValue(":salesaccnt_id", _salesaccnt->id());
  q.exec();
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  sFillList();
}

void salesAccounts::sFillList()
{
  XSqlQuery r;
  r.exec("SELECT salesaccnt_id,"
	 "       CASE WHEN (salesaccnt_warehous_id=-1) THEN TEXT('Any')"
	 "            ELSE (SELECT warehous_code FROM warehous WHERE (warehous_id=salesaccnt_warehous_id))"
	 "       END AS warehouscode,"
	 "       CASE WHEN ((salesaccnt_custtype_id=-1) AND (salesaccnt_custtype='.*')) THEN 'All'"
	 "            WHEN (salesaccnt_custtype_id=-1) THEN salesaccnt_custtype"
	 "            ELSE (SELECT custtype_code FROM custtype WHERE (custtype_id=salesaccnt_custtype_id))"
	 "       END AS custtypecode,"
	 "       CASE WHEN ((salesaccnt_prodcat_id=-1) AND (salesaccnt_prodcat='.*')) THEN 'All'"
	 "            WHEN (salesaccnt_prodcat_id=-1) THEN salesaccnt_prodcat"
	 "            ELSE (SELECT prodcat_code FROM prodcat WHERE (prodcat_id=salesaccnt_prodcat_id))"
	 "       END AS prodcatcode,"
         "       CASE WHEN (salesaccnt_sales_accnt_id IS NULL) THEN 'N/A' "
         "            WHEN (salesaccnt_sales_accnt_id = -1) THEN 'N/A' "
         "            ELSE formatGLAccount(salesaccnt_sales_accnt_id) END AS salesaccount,"
         "       CASE WHEN (salesaccnt_credit_accnt_id IS NULL) THEN 'N/A' "
         "            WHEN (salesaccnt_credit_accnt_id = -1) THEN 'N/A' "
         "            ELSE formatGLAccount(salesaccnt_credit_accnt_id) END AS creditaccount,"
         "       CASE WHEN (salesaccnt_cos_accnt_id IS NULL) THEN 'N/A' "
         "            WHEN (salesaccnt_cos_accnt_id = -1) THEN 'N/A' "
         "            ELSE formatGLAccount(salesaccnt_cos_accnt_id) END AS cosaccount,"
         "       CASE WHEN (salesaccnt_returns_accnt_id IS NULL) THEN 'N/A' "
         "            WHEN (salesaccnt_returns_accnt_id = -1) THEN 'N/A' "
         "            ELSE formatGLAccount(salesaccnt_returns_accnt_id) END AS returnsaccount,"
         "       CASE WHEN (salesaccnt_cor_accnt_id IS NULL) THEN 'N/A' "
         "            WHEN (salesaccnt_cor_accnt_id = -1) THEN 'N/A' "
         "            ELSE formatGLAccount(salesaccnt_cor_accnt_id) END AS coraccount,"
         "       CASE WHEN (salesaccnt_cow_accnt_id IS NULL) THEN 'N/A' "
         "            WHEN (salesaccnt_cow_accnt_id = -1) THEN 'N/A' "
         "            ELSE formatGLAccount(salesaccnt_cow_accnt_id) END AS cowaccount "
	 "FROM salesaccnt "
	 "ORDER BY warehouscode, custtypecode, prodcatcode;" );
  _salesaccnt->populate(r);
  if (r.lastError().type() != QSqlError::NoError)
  {
    systemError(this, r.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}
