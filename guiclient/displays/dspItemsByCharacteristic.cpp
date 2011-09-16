/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspItemsByCharacteristic.h"

#include <QAction>
#include <QMenu>
#include <QVariant>

#include <parameter.h>

#include "bom.h"
#include "item.h"
#include "guiclient.h"

dspItemsByCharacteristic::dspItemsByCharacteristic(QWidget* parent, const char*, Qt::WFlags fl)
    : display(parent, "dspItemsByCharacteristic", fl)
{
  setupUi(optionsWidget());
  setWindowTitle(tr("Items By Characteristic"));
  setListLabel(tr("Items"));
  setReportName("ItemsByCharacteristic");
  setMetaSQLOptions("products", "items");

  _char->populate( "SELECT char_id, char_name "
                   "FROM char "
                   "WHERE (char_items) "
                   "ORDER BY char_name;" );

  list()->addColumn(tr("Item Number"),    _itemColumn, Qt::AlignLeft,  true, "item_number");
  list()->addColumn(tr("Description"),    -1,          Qt::AlignLeft,  true, "descrip");
  list()->addColumn(tr("Characteristic"), _itemColumn, Qt::AlignCenter,true, "char_name");
  list()->addColumn(tr("Value"),          _itemColumn, Qt::AlignLeft,  true, "charass_value");
  list()->addColumn(tr("Type"),           _itemColumn, Qt::AlignCenter,true, "type");
  list()->addColumn(tr("UOM"),            _uomColumn,  Qt::AlignCenter,true, "uom_name");
  list()->setDragString("itemid=");

  connect(omfgThis, SIGNAL(itemsUpdated(int, bool)), this, SLOT(sFillList()));
}

void dspItemsByCharacteristic::languageChange()
{
  display::languageChange();
  retranslateUi(this);
}

bool dspItemsByCharacteristic::setParams(ParameterList &params)
{
  params.append("char_id",      _char->id());
  params.append("value",        _value->text().trimmed());
  params.append("purchased",    tr("Purchased"));
  params.append("manufactured", tr("Manufactured"));
  params.append("job",          tr("Job"));
  params.append("phantom",      tr("Phantom"));
  params.append("breeder",      tr("Breeder"));
  params.append("coProduct",    tr("Co-Product"));
  params.append("byProduct",    tr("By-Product"));
  params.append("reference",    tr("Reference"));
  params.append("costing",      tr("Costing"));
  params.append("tooling",      tr("Tooling"));
  params.append("outside",      tr("Outside Process"));
  params.append("kit",          tr("Kit"));
  params.append("planning",     tr("Planning"));
  params.append("error",        tr("Error"));
  params.append("byCharacteristic");

  if(_showInactive->isChecked())
    params.append("showInactive");

  return true;
}

void dspItemsByCharacteristic::sPopulateMenu(QMenu *pMenu, QTreeWidgetItem *selected, int)
{
  XTreeWidgetItem * xselected = static_cast<XTreeWidgetItem*>(selected);
  QAction *menuItem;

  menuItem = pMenu->addAction(tr("Edit Item Master..."), this, SLOT(sEdit()));;
  if (!_privileges->check("MaintainItemMasters"))
    menuItem->setEnabled(false);

  menuItem = pMenu->addAction(tr("View Item Master..."), this, SLOT(sView()));;

  if (xselected && (xselected->rawValue("type").toString() == "M"))
  {
    menuItem = pMenu->addAction(tr("Edit Bill of Material..."), this, SLOT(sEditBOM()));;
    if (!_privileges->check("MaintainBOMs"))
      menuItem->setEnabled(false);

    menuItem = pMenu->addAction(tr("View Bill of Material..."), this, SLOT(sViewBOM()));;
    if ( (!_privileges->check("MaintainBOMs")) && (!_privileges->check("ViewBOMs")) )
      menuItem->setEnabled(false);
  }
}

void dspItemsByCharacteristic::sEdit()
{
  item::editItem(list()->id());
}

void dspItemsByCharacteristic::sView()
{
  item::viewItem(list()->id());
}

void dspItemsByCharacteristic::sEditBOM()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("item_id", list()->id());

  BOM *newdlg = new BOM();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspItemsByCharacteristic::sViewBOM()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("item_id", list()->id());

  BOM *newdlg = new BOM();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

