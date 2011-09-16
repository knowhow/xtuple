/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspUndefinedManufacturedItems.h"

#include <QAction>
#include <QMenu>
#include <QVariant>

#include "guiclient.h"
#include "bom.h"
#include "item.h"

dspUndefinedManufacturedItems::dspUndefinedManufacturedItems(QWidget* parent, const char*, Qt::WFlags fl)
  : display(parent, "dspUndefinedManufacturedItems", fl)
{
  setupUi(optionsWidget());
  setWindowTitle(tr("Undefined Manufactured Items"));
  setListLabel(tr("Undefined Manufactured Items"));
  setMetaSQLOptions("undefinedManufacturedItems", "detail");
  setUseAltId(true);

  list()->addColumn(tr("Item Number"), _itemColumn,  Qt::AlignLeft,   true,  "item_number"   );
  list()->addColumn(tr("Description"), -1,           Qt::AlignLeft,   true,  "itemdescrip"   );
  list()->addColumn(tr("Type"),        _uomColumn,   Qt::AlignCenter, true,  "item_type" );
  list()->addColumn(tr("Active"),      _orderColumn, Qt::AlignCenter, true,  "item_active" );
  list()->addColumn(tr("Exception"),   _itemColumn,  Qt::AlignCenter, true,  "exception" );

  connect(omfgThis, SIGNAL(itemsUpdated(int, bool)), this, SLOT(sFillList(int, bool)));
  connect(omfgThis, SIGNAL(bomsUpdated(int, bool)), this, SLOT(sFillList(int, bool)));
  connect(omfgThis, SIGNAL(boosUpdated(int, bool)), this, SLOT(sFillList(int, bool)));
  
  if (_preferences->boolean("XCheckBox/forgetful"))
  {
    _bom->setChecked(true);
  }

  if (!_privileges->check("ViewBOMs"))
  {
    _bom->setChecked(false);
    _bom->setEnabled(false);
  }

}

void dspUndefinedManufacturedItems::languageChange()
{
  display::languageChange();
  retranslateUi(this);
}

void dspUndefinedManufacturedItems::sPopulateMenu(QMenu *pMenu, QTreeWidgetItem*, int)
{
  QAction *menuItem;

  menuItem = pMenu->addAction(tr("Edit Item..."), this, SLOT(sEditItem()));
  menuItem->setEnabled(_privileges->check("MaintainItemMasters"));

  if (((XTreeWidgetItem *)list()->currentItem())->altId() == 2)
  {
    menuItem = pMenu->addAction(tr("Create BOM..."), this, SLOT(sCreateBOM()));
    menuItem->setEnabled(_privileges->check("MaintainBOMs"));
  }
}

void dspUndefinedManufacturedItems::sCreateBOM()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("item_id", list()->id());

  BOM *newdlg = new BOM();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspUndefinedManufacturedItems::sEditItem()
{
  item::editItem(list()->id());
}

bool dspUndefinedManufacturedItems::setParams(ParameterList &params)
{
  if (!_bom->isChecked())
  {
    list()->clear();
    return false;
  }

  params.append("noBom", tr("No BOM"));
  if (!_showInactive->isChecked())
    params.append("notshowInactive");

  return true;
}
