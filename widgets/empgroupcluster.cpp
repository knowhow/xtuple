/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "empgroupcluster.h"

EmpGroupCluster::EmpGroupCluster(QWidget* pParent, const char* pName) :
    VirtualCluster(pParent, pName)
{
  addNumberWidget(new EmpGroupClusterLineEdit(this, pName));
  _number->setText(tr("Name"));
  _name->setText(tr("Description"));
}

int EmpGroupClusterLineEdit::idFromList(QWidget *pParent)
{
  return EmpGroupClusterLineEdit(pParent).listFactory()->exec();
}

EmpGroupClusterLineEdit::EmpGroupClusterLineEdit(QWidget* pParent, const char* pName) :
    VirtualClusterLineEdit(pParent, "empgrp", "empgrp_id", "empgrp_name", "empgrp_descrip", 0, 0, pName)
{
  setTitles(tr("Employee Group"), tr("Employee Groups"));
}

VirtualInfo *EmpGroupClusterLineEdit::infoFactory()
{
  return new EmpGroupInfo(this);
}

VirtualList *EmpGroupClusterLineEdit::listFactory()
{
    return new EmpGroupList(this);
}

VirtualSearch *EmpGroupClusterLineEdit::searchFactory()
{
    return new EmpGroupSearch(this);
}

EmpGroupInfo::EmpGroupInfo(QWidget *pParent, Qt::WindowFlags pFlags) : VirtualInfo(pParent, pFlags)
{
  _numberLit->setText(tr("Name:"));
  _nameLit->setText(tr("Description:"));
}

EmpGroupList::EmpGroupList(QWidget *pParent, Qt::WindowFlags pFlags) : VirtualList(pParent, pFlags)
{
  QTreeWidgetItem *hitem = _listTab->headerItem();
  hitem->setText(0, tr("Name"));
  hitem->setText(1, tr("Description"));
}

EmpGroupSearch::EmpGroupSearch(QWidget *pParent, Qt::WindowFlags pFlags) : VirtualSearch(pParent, pFlags)
{
  _searchNumber->setText(tr("Search through Name"));
  _searchName->setText(tr("Search through Description"));
}
