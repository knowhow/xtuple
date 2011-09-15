/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "departments.h"

#include <QAction>
#include <QMenu>
#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include <openreports.h>
#include "department.h"

departments::departments(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  connect(_close, SIGNAL(clicked()), this, SLOT(sClose()));
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_new, SIGNAL(clicked()), this, SLOT(sNew()));
  connect(_edit, SIGNAL(clicked()), this, SLOT(sEdit()));
  connect(_view, SIGNAL(clicked()), this, SLOT(sView()));
  connect(_delete, SIGNAL(clicked()), this, SLOT(sDelete()));
  connect(_deptList, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*)));

  _deptList->addColumn(tr("Dept. Number"),_userColumn, Qt::AlignLeft, true, "dept_number");
  _deptList->addColumn(tr("Dept. Name"),           -1, Qt::AlignLeft, true, "dept_name");

  if (_privileges->check("MaintainDepartments"))
  {
    connect(_deptList, SIGNAL(valid(bool)),	_edit,	SLOT(setEnabled(bool)));
    connect(_deptList, SIGNAL(valid(bool)),	_delete,SLOT(setEnabled(bool)));
    connect(_deptList, SIGNAL(itemSelected(int)), _edit, SLOT(animateClick()));
  }
  else
  {
    _new->setEnabled(false);
    _edit->setEnabled(false);
    _delete->setEnabled(false);
    connect(_deptList, SIGNAL(itemSelected(int)), _view, SLOT(animateClick()));
  }
  connect(_deptList, SIGNAL(valid(bool)),	_view,	SLOT(setEnabled(bool)));

  sFillList();
}

departments::~departments()
{
    // no need to delete child widgets, Qt does it all for us
}

void departments::languageChange()
{
    retranslateUi(this);
}

void departments::sClose()
{
    close();
}

void departments::sPrint()
{
    orReport report("DepartmentsMasterList");
    if (report.isValid())
	report.print();
    else
	report.reportError(this);
}

void departments::sNew()
{
    ParameterList params;
    params.append("mode", "new");

    department newdlg(this, "", TRUE);
    newdlg.set(params);
    newdlg.exec();	//if (newdlg.exec() != XDialog::Rejected)
	sFillList();
}

void departments::sEdit()
{
    ParameterList params;
    params.append("mode", "edit");
    params.append("dept_id", _deptList->id());

    department newdlg(this, "", TRUE);
    newdlg.set(params);
    newdlg.exec();	//if (newdlg.exec() != XDialog::Rejected)
	sFillList();
}

void departments::sView()
{
    ParameterList params;
    params.append("mode", "view");
    params.append("dept_id", _deptList->id());

    department* newdlg = new department(this, "", TRUE);
    newdlg->set(params);
    newdlg->show();
}

void departments::sDelete()
{
    q.prepare("DELETE FROM dept WHERE dept_id = :dept_id;");
    q.bindValue(":dept_id", _deptList->id());
    q.exec();
    if (q.lastError().type() != QSqlError::NoError)
	systemError(this, tr("A System Error occurred at %1::%2\n\n%3")
			    .arg(__FILE__)
			    .arg(__LINE__)
			    .arg(q.lastError().databaseText()));
    sFillList();
}

void departments::sFillList()
{
    _deptList->populate("SELECT dept_id, dept_number, dept_name "
			"FROM dept "
			"ORDER BY dept_number;");
}

void departments::sPopulateMenu(QMenu *pMenu )
{
  QAction *menuItem;

  menuItem = pMenu->addAction(tr("Edit"), this, SLOT(sEdit()));
  menuItem->setEnabled(_privileges->check("MaintainDepartments"));

  menuItem = pMenu->addAction(tr("View"), this, SLOT(sView()));
  menuItem->setEnabled(_privileges->check("ViewDepartments") ||
                       _privileges->check("MaintainDepartments"));

  menuItem = pMenu->addAction(tr("Delete"), this, SLOT(sDelete()));
  menuItem->setEnabled(_privileges->check("MaintainDepartments"));
}
