/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "employees.h"

#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include <parameter.h>
#include <openreports.h>
#include <metasql.h>

#include "employee.h"
#include "errorReporter.h"
#include "guiclient.h"
#include "storedProcErrorLookup.h"

employees::employees(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  connect(_delete, SIGNAL(clicked()), this, SLOT(sDelete()));
  connect(_edit, SIGNAL(clicked()), this, SLOT(sEdit()));
  connect(_new, SIGNAL(clicked()), this, SLOT(sNew()));
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_view, SIGNAL(clicked()), this, SLOT(sView()));
  connect(_warehouse, SIGNAL(updated()), this, SLOT(sFillList()));

  _emp->addColumn(tr("Site"),   _whsColumn,  Qt::AlignLeft, true, "warehous_code");
  _emp->addColumn(tr("Code"),   _itemColumn, Qt::AlignLeft, true, "emp_code");
  _emp->addColumn(tr("Number"), -1,          Qt::AlignLeft, true, "emp_number");
  _emp->addColumn(tr("First"),  _itemColumn, Qt::AlignLeft, true, "cntct_first_name");
  _emp->addColumn(tr("Last"),   _itemColumn, Qt::AlignLeft, true, "cntct_last_name");
  

  if (_privileges->check("MaintainEmployees"))
  {
    connect(_emp, SIGNAL(valid(bool)), _edit, SLOT(setEnabled(bool)));
    connect(_emp, SIGNAL(valid(bool)), _delete, SLOT(setEnabled(bool)));
    connect(_emp, SIGNAL(itemSelected(int)), _edit, SLOT(animateClick()));
  }
  else
  {
    connect(_emp, SIGNAL(itemSelected(int)), _view, SLOT(animateClick()));
    _new->setEnabled(FALSE);
  }

   sFillList();
}

employees::~employees()
{
  // no need to delete child widgets, Qt does it all for us
}

void employees::languageChange()
{
  retranslateUi(this);
}

void employees::sDelete()
{
  if (QMessageBox::question(this, tr("Delete?"),
                            tr("<p>Are you sure you want to delete the "
                               "selected employee?"),
                            QMessageBox::Yes,
                            QMessageBox::No | QMessageBox::Default) == QMessageBox::No)
    return;

  XSqlQuery delq;
  delq.prepare("DELETE FROM emp WHERE (emp_id=:id);");
  delq.bindValue(":id", _emp->id());
  delq.exec();
  if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Deleting"),
                           delq, __FILE__, __LINE__))
    return;
  sFillList();
}

void employees::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("emp_id", _emp->id());

  employee newdlg(this);
  newdlg.set(params);
  
  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void employees::sView()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("emp_id", _emp->id());

  employee newdlg(this);
  newdlg.set(params);
  newdlg.exec();
}

void employees::sFillList()
{
  
  QString sql("SELECT emp_id, warehous_code, emp_code, emp_number, "
              "       cntct_first_name, cntct_last_name "
              "FROM emp "
              "  LEFT OUTER JOIN cntct ON (emp_cntct_id=cntct_id) "
              "  LEFT OUTER JOIN whsinfo ON (emp_warehous_id=warehous_id) "
	     "<? if exists(\"warehouse_id\") ?>"
             "WHERE (warehous_id=<? value(\"warehouse_id\") ?>)"
             "<? endif ?>"
              "ORDER BY emp_code;" );
              
  MetaSQLQuery mql(sql);
  ParameterList params;
  if (!_warehouse->isAll())
    params.append("warehouse_id", _warehouse->id());
  XSqlQuery r = mql.toQuery(params);
  _emp->populate(r);
  if (r.lastError().type() != QSqlError::NoError)
  {
    systemError(this, r.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void employees::sNew()
{
  ParameterList params;
  params.append("mode", "new");

  employee newdlg(this);
  newdlg.set(params);
  
  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void employees::sPrint()
{
  ParameterList params;
  if (!_warehouse->isAll())
    params.append("warehouse_id", _warehouse->id());

  orReport report("EmployeeList", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}
