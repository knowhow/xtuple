/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "costCategories.h"

#include <QAction>
#include <QMenu>
#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include <openreports.h>
#include "costCategory.h"
#include "itemSites.h"

costCategories::costCategories(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  connect(_edit, SIGNAL(clicked()), this, SLOT(sEdit()));
  connect(_view, SIGNAL(clicked()), this, SLOT(sView()));
  connect(_delete, SIGNAL(clicked()), this, SLOT(sDelete()));
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_new, SIGNAL(clicked()), this, SLOT(sNew()));
  connect(_copy, SIGNAL(clicked()), this, SLOT(sCopy()));
  connect(_costcat, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*)));
  
  _costcat->addColumn(tr("Category"), _itemColumn, Qt::AlignLeft, true, "costcat_code");
  _costcat->addColumn(tr("Description"),       -1, Qt::AlignLeft, true, "costcat_descrip");

  if (_privileges->check("MaintainCostCategories"))
  {
    connect(_costcat, SIGNAL(valid(bool)), _edit, SLOT(setEnabled(bool)));
    connect(_costcat, SIGNAL(valid(bool)), _copy, SLOT(setEnabled(bool)));
    connect(_costcat, SIGNAL(valid(bool)), _delete, SLOT(setEnabled(bool)));
    connect(_costcat, SIGNAL(itemSelected(int)), _edit, SLOT(animateClick()));
  }
  else
  {
    _new->setEnabled(FALSE);
    connect(_costcat, SIGNAL(itemSelected(int)), _view, SLOT(animateClick()));
  }

  sFillList();
}

costCategories::~costCategories()
{
  // no need to delete child widgets, Qt does it all for us
}

void costCategories::languageChange()
{
  retranslateUi(this);
}

void costCategories::sPrint()
{
  orReport report("CostCategoriesMasterList");
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void costCategories::sNew()
{
  ParameterList params;
  params.append("mode", "new");

  costCategory newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void costCategories::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("costcat_id", _costcat->id());

  costCategory newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void costCategories::sView()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("costcat_id", _costcat->id());

  costCategory newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void costCategories::sCopy()
{
  ParameterList params;
  params.append("mode", "copy");
  params.append("costcat_id", _costcat->id());

  costCategory newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void costCategories::sDelete()
{
  if ( QMessageBox::question(this, tr("Delete Cost Category"),
                             tr("<p>Are you sure that you want to delete the selected Cost Category?"),
                             QMessageBox::Yes,
                             QMessageBox::No | QMessageBox::Default) == QMessageBox::Yes)
  {
    q.prepare( "SELECT itemsite_id "
               "FROM itemsite "
               "WHERE (itemsite_costcat_id=:costcat_id) "
               "LIMIT 1;" );
    q.bindValue(":costcat_id", _costcat->id());
    q.exec();
    if (q.first())
      QMessageBox::information( this, tr("Cost Category in Use"),
                                tr("<p>The selected Cost Category cannot be "
                                   "deleted as it still contains Items. You "
                                   "must reassign these Items before deleting "
                                   "this Cost Category.") );
    else if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
    else
    {
      q.prepare( "DELETE FROM costcat "
                 "WHERE (costcat_id=:costcat_id);" );
      q.bindValue(":costcat_id", _costcat->id());
      q.exec();
      sFillList();
    }
  }
}

void costCategories::sPopulateMenu(QMenu *menu)
{
  QAction *menuItem;

  menuItem = menu->addAction(tr("Edit Inventory Cost Cateogry..."), this, SLOT(sEdit()));
  menuItem->setEnabled(_privileges->check("MaintainCostCategories"));

  menuItem = menu->addAction(tr("View Inventory Cost Category..."), this, SLOT(sView()));
  menuItem->setEnabled(_privileges->check("MaintainCostCategories") ||
                       _privileges->check("ViewCostCategories"));

  menuItem = menu->addAction(tr("Delete Inventory Cost Category..."), this, SLOT(sDelete()));
  menuItem->setEnabled(_privileges->check("MaintainCostCategories"));

  menu->addSeparator();

  menuItem = menu->addAction(tr("List Items in this Inventory Cost Category..."), this, SLOT(sListItemSites()));
  menuItem->setEnabled(_privileges->check("ViewItemSites"));
}

void costCategories::sListItemSites()
{
  ParameterList params;
  params.append("run");
  params.append("costcat_id", _costcat->id());

  itemSites *newdlg = new itemSites();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void costCategories::sFillList()
{
  _costcat->populate( "SELECT costcat_id, costcat_code, costcat_descrip "
                      "FROM costcat "
                      "ORDER BY costcat_code" );
}
