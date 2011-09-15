/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "expensecluster.h"

ExpenseCluster::ExpenseCluster(QWidget *pParent, const char *pName) :
  VirtualCluster(pParent, pName)
{
  addNumberWidget(new ExpenseLineEdit(this, pName));
  _description->show();
  setLabel(tr("Expense Category:"));
}

ExpenseLineEdit::ExpenseLineEdit(QWidget *pParent, const char *pName) :
  VirtualClusterLineEdit(pParent, "expcat", "expcat_id", "expcat_code", "", "expcat_descrip", 0, pName, "expcat_active")
{
  setTitles(tr("Expense Category"), tr("Expense Categories"));
  setUiName("expenseCategory");
  setEditPriv("MaintainExpenseCategories");
  setNewPriv("MaintainExpenseCategories");
  setViewPriv("ViewExpenseCategories");
}
