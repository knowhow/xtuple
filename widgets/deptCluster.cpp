/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "deptcluster.h"

DeptCluster::DeptCluster(QWidget* pParent, const char* pName) :
    VirtualCluster(pParent, pName)
{
    addNumberWidget(new DeptClusterLineEdit(this, pName));
    _name->show();
}

DeptClusterLineEdit::DeptClusterLineEdit(QWidget* pParent, const char* pName) :
    VirtualClusterLineEdit(pParent, "dept", "dept_id", "dept_number", "dept_name", 0, 0, pName)
{
    setTitles(tr("Department"), tr("Departments"));
    setUiName("department");
    setEditPriv("MaintainDepartments");
    setViewPriv("ViewDepartments");
}
