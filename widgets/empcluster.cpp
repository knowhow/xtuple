/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "empcluster.h"

EmpCluster::EmpCluster(QWidget* pParent, const char* pName) :
    VirtualCluster(pParent, pName)
{
  addNumberWidget(new EmpClusterLineEdit(this, pName));
}

int EmpClusterLineEdit::idFromList(QWidget *pParent)
{
  return EmpClusterLineEdit(pParent).listFactory()->exec();
}

EmpClusterLineEdit::EmpClusterLineEdit(QWidget* pParent, const char* pName) :
    VirtualClusterLineEdit(pParent, "emp", "emp_id", "emp_code", "emp_number", 0, 0, pName, "emp_active")
{
  setTitles(tr("Employee"), tr("Employees"));
  setUiName("employee");
  setEditPriv("MaintainEmployees");
  setNewPriv("MaintainEmployees");
  setViewPriv("ViewEmployees");
}

VirtualInfo *EmpClusterLineEdit::infoFactory()
{
  return new EmpInfo(this);
}

VirtualList *EmpClusterLineEdit::listFactory()
{
    return new EmpList(this);
}

VirtualSearch *EmpClusterLineEdit::searchFactory()
{
    return new EmpSearch(this);
}

EmpInfo::EmpInfo(QWidget *pParent, Qt::WindowFlags pFlags) : VirtualInfo(pParent, pFlags)
{
  _numberLit->setText(tr("Code:"));
  _nameLit->setText(tr("Number:"));
}

EmpList::EmpList(QWidget *pParent, Qt::WindowFlags pFlags) : VirtualList(pParent, pFlags)
{
  QTreeWidgetItem *hitem = _listTab->headerItem();
  hitem->setText(0, tr("Code"));
  hitem->setText(1, tr("Number"));
}

EmpSearch::EmpSearch(QWidget *pParent, Qt::WindowFlags pFlags) : VirtualSearch(pParent, pFlags)
{
  _searchNumber->setText(tr("Search through Codes"));
  _searchName->setText(tr("Search through Numbers"));
}
