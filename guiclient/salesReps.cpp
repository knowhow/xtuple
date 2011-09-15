/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "salesReps.h"

#include <QAction>
#include <QMenu>
#include <QMessageBox>
#include <QSqlError>

#include <parameter.h>
#include <openreports.h>

#include "guiclient.h"
#include "salesRep.h"
#include "storedProcErrorLookup.h"

salesReps::salesReps(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_new, SIGNAL(clicked()), this, SLOT(sNew()));
  connect(_edit, SIGNAL(clicked()), this, SLOT(sEdit()));
  connect(_delete, SIGNAL(clicked()), this, SLOT(sDelete()));
  connect(_salesrep, SIGNAL(populateMenu(QMenu *, QTreeWidgetItem *, int)), this, SLOT(sPopulateMenu(QMenu*)));
  connect(_view, SIGNAL(clicked()), this, SLOT(sView()));

  if (_privileges->check("MaintainSalesReps"))
  {
    connect(_salesrep, SIGNAL(valid(bool)), _edit, SLOT(setEnabled(bool)));
    connect(_salesrep, SIGNAL(valid(bool)), _delete, SLOT(setEnabled(bool)));
    connect(_salesrep, SIGNAL(itemSelected(int)), _edit, SLOT(animateClick()));
  }
  else
  {
    _new->setEnabled(FALSE);
    connect(_salesrep, SIGNAL(itemSelected(int)), _view, SLOT(animateClick()));
  }

  _salesrep->addColumn(tr("Number"), 70,  Qt::AlignLeft,  true, "salesrep_number");
  _salesrep->addColumn(tr("Name"),   -1,  Qt::AlignLeft,  true, "salesrep_name");
  _salesrep->addColumn(tr("Active"), 50,  Qt::AlignCenter,true, "active");

  sFillList();
}

salesReps::~salesReps()
{
  // no need to delete child widgets, Qt does it all for us
}

void salesReps::languageChange()
{
  retranslateUi(this);
}

void salesReps::sDelete()
{
  q.prepare("SELECT deleteSalesRep(:salesrep_id) AS result;" );
  q.bindValue(":salesrep_id", _salesrep->id());
  q.exec();
  if (q.first())
  {
    int result = q.value("result").toInt();
    if (result < 0)
    {
      systemError(this, storedProcErrorLookup("deleteSalesRep", result),
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

void salesReps::sNew()
{
  ParameterList params;
  params.append("mode", "new");

  salesRep newdlg(this, "", TRUE);
  newdlg.set(params);
  
  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void salesReps::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("salesrep_id", _salesrep->id());

  salesRep newdlg(this, "", TRUE);
  newdlg.set(params);
  
  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void salesReps::sView()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("salesrep_id", _salesrep->id());

  salesRep newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void salesReps::sFillList()
{
  q.exec("SELECT *, formatBoolYN(salesrep_active) AS active "
         "FROM salesrep "
         "ORDER BY salesrep_number;" );
  _salesrep->populate(q);
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void salesReps::sPopulateMenu( QMenu * menu )
{
  QAction *menuItem;

  menuItem = menu->addAction(tr("Edit Sales Rep..."), this, SLOT(sEdit()));
  if (!_privileges->check("MaintainSalesReps"))
    menuItem->setEnabled(FALSE);

  menuItem = menu->addAction(tr("View Sales Rep..."), this, SLOT(sView()));
  if ((!_privileges->check("MaintainSalesReps")) && (!_privileges->check("ViewSalesReps")))
    menuItem->setEnabled(FALSE);

  menuItem = menu->addAction(tr("Delete Sales Rep..."), this, SLOT(sDelete()));
  if (!_privileges->check("MaintainSalesReps"))
    menuItem->setEnabled(FALSE);
}

void salesReps::sPrint()
{
  orReport report("SalesRepsMasterList");
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}
