/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "budgets.h"

#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include <parameter.h>
#include <openreports.h>

#include "copyBudget.h"
#include "guiclient.h"
#include "maintainBudget.h"
#include "storedProcErrorLookup.h"

budgets::budgets(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  connect(_copy,   SIGNAL(clicked()), this, SLOT(sCopy()));
  connect(_delete, SIGNAL(clicked()), this, SLOT(sDelete()));
  connect(_edit,   SIGNAL(clicked()), this, SLOT(sEdit()));
  connect(_new,    SIGNAL(clicked()), this, SLOT(sNew()));
  connect(_print,  SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_view,   SIGNAL(clicked()), this, SLOT(sView()));

  _budget->addColumn(tr("Start Date"),_dateColumn, Qt::AlignLeft, true, "startdate");
  _budget->addColumn(tr("End Date"),  _dateColumn, Qt::AlignLeft, true, "enddate");
  _budget->addColumn(tr("Code"),      _itemColumn, Qt::AlignLeft, true, "budghead_name");
  _budget->addColumn(tr("Description"),        -1, Qt::AlignLeft, true, "budghead_descrip");

  if (_privileges->check("MaintainBudgets"))
  {
    connect(_budget, SIGNAL(valid(bool)), _edit, SLOT(setEnabled(bool)));
    connect(_budget, SIGNAL(valid(bool)), _delete, SLOT(setEnabled(bool)));
    connect(_budget, SIGNAL(valid(bool)), _copy, SLOT(setEnabled(bool)));
    connect(_budget, SIGNAL(itemSelected(int)), _edit, SLOT(animateClick()));
  }
  else
  {
    connect(_budget, SIGNAL(itemSelected(int)), _view, SLOT(animateClick()));
    _new->setEnabled(FALSE);
  }

  connect(omfgThis, SIGNAL(budgetsUpdated(int, bool)), this, SLOT(sFillList()));

   sFillList();
}

budgets::~budgets()
{
  // no need to delete child widgets, Qt does it all for us
}

void budgets::languageChange()
{
  retranslateUi(this);
}

void budgets::sDelete()
{
  q.prepare( "SELECT deleteBudget(:budghead_id) AS result;");
  q.bindValue(":budghead_id", _budget->id());
  q.exec();
  if (q.first())
  {
    int result = q.value("result").toInt();
    if (result < 0)
    {
      systemError(this, storedProcErrorLookup("deleteBudget", result),
                  __FILE__, __LINE__);
      return;
    }
  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  sFillList();
}

void budgets::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("budghead_id", _budget->id());

  maintainBudget *newdlg = new maintainBudget();
  newdlg->set(params);
  
  omfgThis->handleNewWindow(newdlg);
}

void budgets::sView()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("budghead_id", _budget->id());

  maintainBudget *newdlg = new maintainBudget();
  newdlg->set(params);

  omfgThis->handleNewWindow(newdlg);
}

void budgets::sFillList()
{
  q.prepare("SELECT budghead_id, "
            "       MIN(period_start) AS startdate,"
	    "       MAX(period_end) AS enddate,"
            "       budghead_name, budghead_descrip "
	    "  FROM budghead LEFT OUTER JOIN budgitem "
            "         JOIN period ON (budgitem_period_id=period_id) "
            "       ON (budgitem_budghead_id=budghead_id) "
            " GROUP BY budghead_id, budghead_name, budghead_descrip "
	    " ORDER BY startdate DESC, budghead_name;" );
  q.exec();
  _budget->populate(q);
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void budgets::sNew()
{
  ParameterList params;
  params.append("mode", "new");

  maintainBudget *newdlg = new maintainBudget();
  newdlg->set(params);
  
  omfgThis->handleNewWindow(newdlg);
}

void budgets::sPrint()
{
  orReport report("BudgetsMasterList");
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void budgets::sCopy()
{
  ParameterList params;
  params.append("budghead_id", _budget->id());

  copyBudget newdlg(this, "", true);
  newdlg.set(params);

  if(newdlg.exec() == XDialog::Accepted)
    sFillList();
}
