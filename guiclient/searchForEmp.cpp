/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "searchForEmp.h"

#include <QAction>
#include <QMenu>
#include <QSqlError>
#include <QVariant>

#include <metasql.h>
#include <parameter.h>
#include <openreports.h>

#include "employee.h"

#define DEBUG   false

// TODO: XDialog should have a default implementation that returns FALSE
bool searchForEmp::userHasPriv(const int pMode)
{
  if (DEBUG)
    qDebug("searchForEmp::userHasPriv(%d)", pMode);
  bool retval = _privileges->check("ViewEmployees") ||
                _privileges->check("MaintainEmployees");
  if (DEBUG)
    qDebug("searchForEmp::userHasPriv(%d) returning %d", pMode, retval);
  return retval;
}

// TODO: this code really belongs in XDialog
void searchForEmp::setVisible(bool visible)
{
  if (DEBUG)
    qDebug("searchForEmp::setVisible(%d) called",
           visible);
  if (! visible)
    XWidget::setVisible(false);

  else if (! userHasPriv())
  {
    systemError(this,
		tr("You do not have sufficient privilege to view this window"),
		__FILE__, __LINE__);
    close();
  }
  else
    XWidget::setVisible(true);
}

searchForEmp::searchForEmp(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  connect(_edit,         SIGNAL(clicked()),     this, SLOT(sEdit()));
  connect(_emp, SIGNAL(populateMenu(QMenu *, QTreeWidgetItem *, int)), this, SLOT(sPopulateMenu(QMenu *, QTreeWidgetItem *)));
  connect(_new,          SIGNAL(clicked()),     this, SLOT(sNew()));
  connect(_print,        SIGNAL(clicked()),     this, SLOT(sPrint()));
  connect(_searchCode,   SIGNAL(toggled(bool)), this, SLOT(sFillList()));
  connect(_searchDept,   SIGNAL(toggled(bool)), this, SLOT(sFillList()));
  connect(_searchMgr,    SIGNAL(toggled(bool)), this, SLOT(sFillList()));
  connect(_searchNumber, SIGNAL(toggled(bool)), this, SLOT(sFillList()));
  connect(_searchName, SIGNAL(toggled(bool)), this, SLOT(sFillList()));
  connect(_searchShift,  SIGNAL(toggled(bool)), this, SLOT(sFillList()));
  connect(_search,       SIGNAL(lostFocus()),   this, SLOT(sFillList()));
  connect(_showInactive, SIGNAL(clicked()),     this, SLOT(sFillList()));
  connect(_view,         SIGNAL(clicked()),     this, SLOT(sView()));
  connect(_warehouse,    SIGNAL(updated()),     this, SLOT(sFillList()));

  _emp->addColumn(tr("Site"),   _whsColumn,  Qt::AlignLeft, true, "warehous_code");
  _emp->addColumn(tr("Code"),   _itemColumn, Qt::AlignLeft, true, "emp_code");
  _emp->addColumn(tr("Number"),          -1, Qt::AlignLeft, true, "emp_number");
  _emp->addColumn(tr("First"),  _itemColumn, Qt::AlignLeft, true, "cntct_first_name");
  _emp->addColumn(tr("Last"),   _itemColumn, Qt::AlignLeft, true, "cntct_last_name");
  _emp->addColumn(tr("Manager"),_itemColumn, Qt::AlignLeft, true, "mgr_code");
  _emp->addColumn(tr("Dept."),  _itemColumn, Qt::AlignLeft, true, "dept_number");
  _emp->addColumn(tr("Shift"),  _itemColumn, Qt::AlignLeft, true, "shift_number");

  if (_privileges->check("MaintainEmployees"))
  {
    _new->setEnabled(true);
    connect(_emp, SIGNAL(valid(bool)),       _edit, SLOT(setEnabled(bool)));
    connect(_emp, SIGNAL(itemSelected(int)), _edit, SLOT(animateClick()));
  }
  else
    connect(_emp, SIGNAL(itemSelected(int)), _view, SLOT(animateClick()));

  connect(omfgThis, SIGNAL(itemsUpdated(int, bool)), SLOT(sFillList()));

  if (_preferences->boolean("XCheckBox/forgetful"))
  {
    _searchCode->setChecked(true);
    _searchDept->setChecked(true);
    _searchMgr->setChecked(true);
    _searchNumber->setChecked(true);
    _searchShift->setChecked(true);
  }

  _search->setFocus();
}

searchForEmp::~searchForEmp()
{
  // no need to delete child widgets, Qt does it all for us
}

void searchForEmp::languageChange()
{
  retranslateUi(this);
}

void searchForEmp::sPrint()
{
  ParameterList params;
  if (! setParams(params))
    return;

  orReport report("EmployeeList", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void searchForEmp::sPopulateMenu(QMenu *pMenu, QTreeWidgetItem *pSelected)
{
  QAction *menuItem;

  menuItem = pMenu->addAction(tr("View..."), this, SLOT(sView()));
  menuItem = pMenu->addAction(tr("Edit..."), this, SLOT(sEdit()));
  menuItem->setEnabled(_privileges->check("MaintainEmployees"));

  if (((XTreeWidgetItem *)pSelected)->altId() > 0)
  {
    pMenu->addSeparator();
    menuItem = pMenu->addAction(tr("View Manager..."), this, SLOT(sViewMgr()));
    menuItem = pMenu->addAction(tr("Edit Manager..."), this, SLOT(sEditMgr()));
    menuItem->setEnabled(_privileges->check("MaintainEmployees"));
  }
}

void searchForEmp::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("emp_id", _emp->id());

  employee newdlg(this);
  newdlg.set(params);
  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void searchForEmp::sEditMgr()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("emp_id", _emp->altId());

  employee newdlg(this);
  newdlg.set(params);
  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void searchForEmp::sNew()
{
  ParameterList params;
  params.append("mode", "new");

  employee newdlg(this);
  newdlg.set(params);
  
  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void searchForEmp::sView()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("emp_id", _emp->id());

  employee newdlg(this);
  newdlg.set(params);
  newdlg.exec();
}

void searchForEmp::sViewMgr()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("emp_id", _emp->altId());

  employee newdlg(this);
  newdlg.set(params);
  newdlg.exec();
}

bool searchForEmp::setParams(ParameterList &pParams)
{
  pParams.append("searchString", _search->text().toUpper());

  if (_searchCode->isChecked())    pParams.append("searchCode");
  if (_searchDept->isChecked())    pParams.append("searchDept");
  if (_searchMgr->isChecked())     pParams.append("searchMgr");
  if (_searchNumber->isChecked())  pParams.append("searchNumber");
  if (_searchName->isChecked())    pParams.append("searchName");
  if (_searchShift->isChecked())   pParams.append("searchShift");
  if (!_showInactive->isChecked()) pParams.append("activeOnly");
  if (!_warehouse->isAll())        pParams.append("warehouse_id", _warehouse->id());

  return true;
}

void searchForEmp::sFillList()
{
  QString sql("SELECT e.emp_id, m.emp_id, warehous_code, e.emp_code, e.emp_number,"
              "       cntct_first_name, cntct_last_name, "
              "       m.emp_code AS mgr_code, dept_number, shift_number "
              "FROM emp e "
              "  LEFT OUTER JOIN cntct ON (emp_cntct_id=cntct_id) "
              "  LEFT OUTER JOIN whsinfo ON (emp_warehous_id=warehous_id) "
              "  LEFT OUTER JOIN emp m ON (e.emp_mgr_emp_id=m.emp_id) "
              "  LEFT OUTER JOIN shift ON (e.emp_shift_id=shift_id) "
              "  LEFT OUTER JOIN dept  ON (e.emp_dept_id=dept_id) "
              "WHERE ((LENGTH(TRIM(<? value(\"searchString\") ?>)) = 0)"
              "<? if exists(\"searchCode\") ?>"
              "   OR (e.emp_code ~* <? value(\"searchString\") ?>)"
              "<? endif ?>"
              "<? if exists(\"searchDept\") ?>"
              "   OR (dept_number ~* <? value(\"searchString\") ?>)"
              "   OR (dept_name   ~* <? value(\"searchString\") ?>)"
              "<? endif ?>"
              "<? if exists(\"searchMgr\") ?>"
              "   OR (m.emp_code   ~* <? value(\"searchString\") ?>)"
              "   OR (m.emp_number ~* <? value(\"searchString\") ?>)"
              "<? endif ?>"
              "<? if exists(\"searchNumber\") ?>"
              "   OR (e.emp_number ~* <? value(\"searchString\") ?>)"
              "<? endif ?>"
              "<? if exists(\"searchShift\") ?>"
              "   OR (shift_number ~* <? value(\"searchString\") ?>)"
              "   OR (shift_name   ~* <? value(\"searchString\") ?>)"
              "<? endif ?>"
              "<? if exists(\"searchName\") ?>"
              "   OR (cntct_first_name ~* <? value(\"searchString\") ?>)"
              "   OR (cntct_last_name   ~* <? value(\"searchString\") ?>)"
              "<? endif ?>"
              "      )"  // end searchString
              "<? if exists(\"activeOnly\") ?>"
              "   AND e.emp_active "
              "<? endif ?>"
	     "<? if exists(\"warehouse_id\") ?>"
             "    AND (warehous_id=<? value(\"warehouse_id\") ?>)"
             "<? endif ?>"
              "ORDER BY emp_code");

  MetaSQLQuery mql(sql);
  ParameterList params;
  if (! setParams(params))
    return;
  q = mql.toQuery(params);
  _emp->populate(q, true);
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}
