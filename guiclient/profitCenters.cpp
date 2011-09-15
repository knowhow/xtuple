/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "profitCenters.h"

#include <QAction>
#include <QMenu>
#include <QMessageBox>
#include <QSqlError>

#include <parameter.h>

#include "profitCenter.h"
#include "storedProcErrorLookup.h"

profitCenters::profitCenters(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  connect(_delete, SIGNAL(clicked()), this, SLOT(sDelete()));
  connect(_edit,   SIGNAL(clicked()), this, SLOT(sEdit()));
  connect(_new,    SIGNAL(clicked()), this, SLOT(sNew()));
  connect(_prftcntr, SIGNAL(populateMenu(QMenu *, QTreeWidgetItem *, int)), this, SLOT(sPopulateMenu(QMenu*)));
  connect(_view,   SIGNAL(clicked()), this, SLOT(sView()));

  if (_privileges->check("MaintainChartOfAccounts"))
  {
    connect(_prftcntr, SIGNAL(valid(bool)), _edit, SLOT(setEnabled(bool)));
    connect(_prftcntr, SIGNAL(valid(bool)), _delete, SLOT(setEnabled(bool)));
    connect(_prftcntr, SIGNAL(itemSelected(int)), _edit, SLOT(animateClick()));
  }
  else
  {
    _new->setEnabled(FALSE);
    connect(_prftcntr, SIGNAL(itemSelected(int)), _view, SLOT(animateClick()));
  }

  _prftcntr->addColumn(tr("Number"),_itemColumn, Qt::AlignCenter,true, "prftcntr_number");
  _prftcntr->addColumn(tr("Description"),    -1, Qt::AlignLeft,  true, "prftcntr_descrip");

  sFillList();
}

profitCenters::~profitCenters()
{
  // no need to delete child widgets, Qt does it all for us
}

void profitCenters::languageChange()
{
  retranslateUi(this);
}

void profitCenters::sNew()
{
  ParameterList params;
  params.append("mode", "new");

  profitCenter *newdlg = new profitCenter(this, "", TRUE);
  newdlg->set(params);
  if (newdlg->exec() != XDialog::Rejected)
    sFillList();
}

void profitCenters::sEdit()
{
  ParameterList params;
  params.append("prftcntr_id", _prftcntr->id());
  params.append("mode", "edit");

  profitCenter *newdlg = new profitCenter(this, "", TRUE);
  newdlg->set(params);
  if (newdlg->exec() != XDialog::Rejected)
    sFillList();
}

void profitCenters::sView()
{
  ParameterList params;
  params.append("prftcntr_id", _prftcntr->id());
  params.append("mode", "view");

  profitCenter *newdlg = new profitCenter(this, "", TRUE);
  newdlg->set(params);
  newdlg->exec();
}

void profitCenters::sDelete()
{
  q.prepare( "SELECT deleteProfitCenter(:id) AS result;" );
  q.bindValue(":id", _prftcntr->id());
  q.exec();
  if (q.first())
  {
    int result = q.value("result").toInt();
    if (result < 0)
    {
      systemError(this, storedProcErrorLookup("deleteProfitCenter", result),
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

void profitCenters::sPopulateMenu(QMenu *pMenu)
{
  QAction *menuItem;

  pMenu->addAction("View...", this, SLOT(sView()));

  menuItem = pMenu->addAction("Edit...", this, SLOT(sEdit()));
  menuItem->setEnabled(_privileges->check("MaintainChartOfAccounts"));

  menuItem = pMenu->addAction("Delete...", this, SLOT(sDelete()));
  menuItem->setEnabled(_privileges->check("MaintainChartOfAccounts"));
}

void profitCenters::sFillList()
{
  q.prepare( "SELECT prftcntr_id,"
             "       prftcntr_number, prftcntr_descrip "
             "FROM prftcntr "
                 "ORDER BY prftcntr_number;" );
  q.exec();
  _prftcntr->populate(q);
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}
