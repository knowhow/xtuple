/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspInvalidBillsOfMaterials.h"

#include <QAction>
#include <QMenu>
#include <QSqlError>
#include <QVariant>

#include <parameter.h>

#include <metasql.h>
#include "mqlutil.h"

#include "item.h"
#include "itemSite.h"

dspInvalidBillsOfMaterials::dspInvalidBillsOfMaterials(QWidget* parent, const char*, Qt::WFlags fl)
  : display(parent, "dspInvalidBillsOfMaterials", fl)
{
  setupUi(optionsWidget());
  setWindowTitle(tr("Bills of Materials without Component Item Sites"));
  setListLabel(tr("Invalid Bill of Material Items"));
  setMetaSQLOptions("invalidBillsofMaterials", "detail");
  setUseAltId(true);

  connect(_update, SIGNAL(toggled(bool)), this, SLOT(sHandleUpdate()));

  list()->addColumn("componentItemid",                 0, Qt::AlignCenter,true, "pitem_id");
  list()->addColumn("componentSiteId",                 0, Qt::AlignCenter,true, "citem_id");
  list()->addColumn("warehousId",                      0, Qt::AlignCenter,true, "warehous_id");
  list()->addColumn(tr("Site"),               _whsColumn, Qt::AlignCenter,true, "warehous_code");
  list()->addColumn(tr("Parent Item #"),     _itemColumn, Qt::AlignLeft,  true, "parentitem");
  list()->addColumn(tr("Component Item #"),  _itemColumn, Qt::AlignLeft,  true, "componentitem");
  list()->addColumn(tr("Component Item Description"), -1, Qt::AlignLeft,  true, "descrip");

  if (_preferences->boolean("XCheckBox/forgetful"))
    _update->setChecked(true);
}

void dspInvalidBillsOfMaterials::languageChange()
{
  display::languageChange();
  retranslateUi(this);
}

void dspInvalidBillsOfMaterials::sEditItem()
{
  item::editItem(list()->altId());
}

void dspInvalidBillsOfMaterials::sCreateItemSite()
{
  ParameterList params;
  params.append("mode", "new");
  params.append("item_id",     list()->currentItem()->rawValue("citem_id").toInt());
  params.append("warehous_id", list()->currentItem()->rawValue("warehous_id").toInt());
  
  itemSite newdlg(this, "", true);
  newdlg.set(params);
  newdlg.exec();
  
  sFillList();
}

void dspInvalidBillsOfMaterials::sEditItemSite()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("itemsite_id", list()->id());
  
  itemSite newdlg(this, "", true);
  newdlg.set(params);
  newdlg.exec();
}

void dspInvalidBillsOfMaterials::sHandleUpdate()
{
  if (_update->isChecked())
  {
    connect(omfgThis, SIGNAL(itemsUpdated(int, bool)), this, SLOT(sFillList()));
    connect(omfgThis, SIGNAL(itemsitesUpdated()), this, SLOT(sFillList()));
  }
  else
  {
    disconnect(omfgThis, SIGNAL(itemsUpdated(int, bool)), this, SLOT(sFillList()));
    disconnect(omfgThis, SIGNAL(itemsitesUpdated()), this, SLOT(sFillList()));
  }
}

bool dspInvalidBillsOfMaterials::setParams(ParameterList &params)
{

  if (_warehouse->isSelected())
    params.append("warehous_id", _warehouse->id());

  return true;
}

void dspInvalidBillsOfMaterials::sPopulateMenu(QMenu *pMenu, QTreeWidgetItem *, int)
{
  QAction *menuItem;

  menuItem = pMenu->addAction(tr("Edit Parent Item..."), this, SLOT(sEditItem()));
  menuItem->setEnabled(_privileges->check("MaintainItemMasters"));

  menuItem = pMenu->addAction(tr("Edit Parent Item Site..."), this, SLOT(sEditItemSite()));
  menuItem->setEnabled(_privileges->check("MaintainItemSites"));

  menuItem = pMenu->addAction(tr("Create Component Item Site..."), this, SLOT(sCreateItemSite()));
  menuItem->setEnabled(_privileges->check("MaintainItemSites"));
}

