/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "customerFormAssignments.h"

#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include <parameter.h>
#include <openreports.h>
#include "customerFormAssignment.h"

customerFormAssignments::customerFormAssignments(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  connect(_new, SIGNAL(clicked()), this, SLOT(sNew()));
  connect(_edit, SIGNAL(clicked()), this, SLOT(sEdit()));
  connect(_delete, SIGNAL(clicked()), this, SLOT(sDelete()));
  connect(_view, SIGNAL(clicked()), this, SLOT(sView()));

  _custform->addColumn(tr("Customer Type"), -1, Qt::AlignCenter, true, "custtypecode");
  _custform->addColumn(tr("Invoice"),      100, Qt::AlignCenter, true, "invoice");
  _custform->addColumn(tr("Credit Memo"),  100, Qt::AlignCenter, true, "creditmemo");
  _custform->addColumn(tr("Statement"),    100, Qt::AlignCenter, true, "statement");
  _custform->addColumn(tr("Quote"),        100, Qt::AlignCenter, true, "quote");
  _custform->addColumn(tr("Packing List"), 100, Qt::AlignCenter, true, "packinglist");
  _custform->addColumn(tr("S/O Pick List"),100, Qt::AlignCenter, true, "sopicklist");

  if (_privileges->check("MaintainSalesAccount"))
  {
    connect(_custform, SIGNAL(valid(bool)), _edit, SLOT(setEnabled(bool)));
    connect(_custform, SIGNAL(valid(bool)), _delete, SLOT(setEnabled(bool)));
    connect(_custform, SIGNAL(itemSelected(int)), _edit, SLOT(animateClick()));
  }
  else
  {
    _new->setEnabled(FALSE);
    connect(_custform, SIGNAL(itemSelected(int)), _view, SLOT(animateClick()));
  }

  sFillList();
}

customerFormAssignments::~customerFormAssignments()
{
  // no need to delete child widgets, Qt does it all for us
}

void customerFormAssignments::languageChange()
{
  retranslateUi(this);
}

void customerFormAssignments::sNew()
{
  ParameterList params;
  params.append("mode", "new");
  
  customerFormAssignment newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void customerFormAssignments::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("custform_id", _custform->id());
  
  customerFormAssignment newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void customerFormAssignments::sView()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("custform_id", _custform->id());
  
  customerFormAssignment newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void customerFormAssignments::sDelete()
{
  q.prepare( "DELETE FROM custform "
             "WHERE (custform_id=:custform_id);" );
  q.bindValue(":custform_id", _custform->id());
  q.exec();
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
  sFillList();
}

void customerFormAssignments::sFillList()
{
  q.prepare( "SELECT custform_id,"
             "       CASE WHEN custform_custtype_id=-1 THEN custform_custtype"
             "            ELSE (SELECT custtype_code FROM custtype WHERE (custtype_id=custform_custtype_id))"
             "       END AS custtypecode,"
             "       COALESCE(custform_invoice_report_name, :default) AS invoice,"
             "       COALESCE(custform_creditmemo_report_name, :default) AS creditmemo,"
             "       COALESCE(custform_statement_report_name, :default) AS statement,"
             "       COALESCE(custform_quote_report_name, :default) AS quote,"
             "       COALESCE(custform_packinglist_report_name, :default) AS packinglist,"
             "       COALESCE(custform_sopicklist_report_name, :default) AS sopicklist "
             "FROM custform "
             "ORDER BY custtypecode;" );
  q.bindValue(":default", tr("Default"));
  q.exec();
  _custform->populate(q);
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}
