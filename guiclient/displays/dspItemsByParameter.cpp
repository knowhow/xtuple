/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspItemsByParameter.h"

#include <QAction>
#include <QMenu>
#include <QVariant>

#include <parameter.h>

#include "bom.h"
#include "guiclient.h"
#include "item.h"

dspItemsByParameter::dspItemsByParameter(QWidget* parent, const char * name, Qt::WFlags flags)
  : display(parent, name, flags)
{
  setupUi(optionsWidget());
  setMetaSQLOptions("products", "items");
  setListLabel(tr("Items"));

  list()->addColumn( tr("Item Number"), _itemColumn, Qt::AlignLeft,  true, "item_number");
  list()->addColumn( tr("Description"), -1,          Qt::AlignLeft,  true, "descrip");
  list()->addColumn( tr("Class Code"),  _itemColumn, Qt::AlignLeft,  true, "classcode_code");
  list()->addColumn( tr("Product Cat."),_itemColumn, Qt::AlignLeft, false, "prodcat_code");
  list()->addColumn( tr("Type"),        _itemColumn, Qt::AlignCenter,true, "type");
  list()->addColumn( tr("UOM"),         _uomColumn,  Qt::AlignCenter,true, "uom_name");
  list()->setDragString("itemid=");

  connect(omfgThis, SIGNAL(itemsUpdated(int, bool)), this, SLOT(sFillList()));
}

void dspItemsByParameter::languageChange()
{
  display::languageChange();
  retranslateUi(this);
}

bool dspItemsByParameter::setParams(ParameterList &params)
{
  _parameter->appendValue(params);

  if(_showInactive->isChecked())
    params.append("showInactive");

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

  return true;
}

void dspItemsByParameter::sPopulateMenu(QMenu *pMenu, QTreeWidgetItem *selected, int)
{
  XTreeWidgetItem * xselected = static_cast<XTreeWidgetItem*>(selected);
  QAction *menuItem;

  menuItem = pMenu->addAction(tr("Edit Item Master..."), this, SLOT(sEdit()));;
  if (!_privileges->check("MaintainItemMasters"))
    menuItem->setEnabled(false);

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

void dspItemsByParameter::sEdit()
{
  item::editItem(list()->id());
}

void dspItemsByParameter::sEditBOM()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("item_id", list()->id());

  BOM *newdlg = new BOM();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspItemsByParameter::sViewBOM()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("item_id", list()->id());

  BOM *newdlg = new BOM();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

