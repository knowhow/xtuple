/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "items.h"

#include <QAction>
#include <QMenu>
#include <QMessageBox>
#include <QSqlError>
#include <QVariant>
#include <metasql.h>

#include "characteristic.h"
#include "copyItem.h"
#include "item.h"
#include "storedProcErrorLookup.h"
#include "parameterwidget.h"

items::items(QWidget* parent, const char*, Qt::WFlags fl)
  : display(parent, "items", fl)
{
  setWindowTitle(tr("Items"));
  setReportName("Items");
  setMetaSQLOptions("items", "detail");
  setNewVisible(true);
  setSearchVisible(true);
  setQueryOnStartEnabled(true);
  setParameterWidgetVisible(true);

  QString qryType = QString( "SELECT  1, '%1' UNION "
                             "SELECT  2, '%2' UNION "
                             "SELECT  3, '%3'")
      .arg(tr("Buy Items"))
      .arg(tr("Make Items"))
      .arg(tr("Sold Items"));

  parameterWidget()->appendComboBox(tr("Class Code"), "classcode_id", XComboBox::ClassCodes);
  parameterWidget()->append(tr("Class Code Pattern"), "classcode_pattern", ParameterWidget::Text);
  parameterWidget()->appendComboBox(tr("Freight Class"), "freightclass_id", XComboBox::FreightClasses);
  parameterWidget()->append(tr("Freight Class Pattern"), "freightclass_pattern", ParameterWidget::Text);
  parameterWidget()->append(tr("Item Number Pattern"), "item_number_pattern", ParameterWidget::Text);
  parameterWidget()->append(tr("Item Description"), "item_descrip_pattern", ParameterWidget::Text);
  parameterWidget()->appendComboBox(tr("Item Group"), "itemgrp_id", XComboBox::ItemGroups);
  parameterWidget()->appendComboBox(tr("Item Types"), "item_types", qryType);
  parameterWidget()->appendComboBox(tr("Product Category"), "prodcat_id", XComboBox::ProductCategories);
  parameterWidget()->append(tr("Product Category Pattern"), "prodcat_pattern", ParameterWidget::Text);
  parameterWidget()->append(tr("Show Inactive"), "showInactive", ParameterWidget::Exists);

  list()->addColumn(tr("Item Number"), _itemColumn, Qt::AlignLeft   , true, "item_number" );
  list()->addColumn(tr("Active"),      _ynColumn,   Qt::AlignCenter , true, "item_active" );
  list()->addColumn(tr("Description"), -1,          Qt::AlignLeft   , true, "item_descrip" );
  list()->addColumn(tr("Class Code"),  _dateColumn, Qt::AlignLeft , true, "classcode_code");
  list()->addColumn(tr("Type"),        _itemColumn, Qt::AlignLeft , true, "item_type");
  list()->addColumn(tr("UOM"),         _uomColumn,  Qt::AlignLeft , true, "uom_name");
  list()->addColumn(tr("Product Category"),  _itemColumn, Qt::AlignLeft , false, "prodcat_code");
  list()->addColumn(tr("Freight Class"),  _itemColumn, Qt::AlignLeft , false, "freightclass_code");
  
  setupCharacteristics(characteristic::Items);
  parameterWidget()->applyDefaultFilterSet();

  if (_privileges->check("MaintainItemMasters"))
    connect(list(), SIGNAL(itemSelected(int)), this, SLOT(sEdit()));
  else
  {
    newAction()->setEnabled(FALSE);
    connect(list(), SIGNAL(itemSelected(int)), this, SLOT(sView()));
  }

  connect(omfgThis, SIGNAL(itemsUpdated(int, bool)), this, SLOT(sFillList()));
}


void items::sPopulateMenu(QMenu * pMenu, QTreeWidgetItem *, int)
{
  QAction *menuItem;

  menuItem = pMenu->addAction(tr("Edit..."), this, SLOT(sEdit()));
  menuItem->setEnabled(_privileges->check("MaintainItemMasters"));

  menuItem = pMenu->addAction(tr("View..."), this, SLOT(sView()));

  QAction *tmpaction = pMenu->addAction(tr("Delete..."));
  connect(tmpaction, SIGNAL(triggered()), this, SLOT(sDelete()));
  tmpaction->setEnabled(_privileges->check("DeleteItemMasters"));
  tmpaction->setObjectName("items.popup.delete");

  pMenu->addSeparator();

  menuItem = pMenu->addAction(tr("Copy..."), this, SLOT(sCopy()));
  menuItem->setEnabled(_privileges->check("MaintainItemMasters"));
}

void items::sNew()
{
  item::newItem();
}

void items::sEdit()
{
  item::editItem(list()->id());
}

void items::sView()
{
  item::viewItem(list()->id());
}

void items::sDelete()
{
  if (QMessageBox::information( this, tr("Delete Item"),
                                tr( "Are you sure that you want to delete the Item?"),
                                tr("&Delete"), tr("&Cancel"), 0, 0, 1 ) == 0  )
  {
    XSqlQuery qry;
    qry.prepare("SELECT deleteItem(:item_id) AS returnVal;");
    qry.bindValue(":item_id", list()->id());
    qry.exec();
    if (qry.first())
    {
      int returnVal = qry.value("returnVal").toInt();
      if (returnVal < 0)
      {
        systemError(this, storedProcErrorLookup("deleteItem", returnVal), __FILE__, __LINE__);
        return;
      }
      sFillList();
    }
    else if (qry.lastError().type() != QSqlError::NoError)
    {
      systemError(this, qry.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }   
}

bool items::setParams(ParameterList &params)
{
  display::setParams(params);

  bool valid;
  QVariant param;

  param = params.value("item_types", &valid);
  if (valid)
  {
    int types = param.toInt();
    if (types == 1)
      params.append("showPurchased");
    else if (types == 2)
      params.append("showManufactured");
    else if (types == 3)
      params.append("showSold");
  }

  if (_preferences->boolean("ListNumericItemNumbersFirst"))
    params.append("ListNumericItemNumbersFirst");

  params.append("purchased", tr("Purchased"));
  params.append("manufactured", tr("Manufactured"));
  params.append("phantom", tr("Phantom"));
  params.append("breeder", tr("Breeder"));
  params.append("coProduct", tr("Co-Product"));
  params.append("byProduct", tr("By-Product"));
  params.append("reference", tr("Reference"));
  params.append("costing", tr("Costing"));
  params.append("tooling", tr("Tooling"));
  params.append("outside", tr("Outside Process"));
  params.append("planning", tr("Planning"));
  params.append("assortment", tr("Assortment"));
  params.append("kit", tr("Kit"));
  params.append("error", tr("Error"));

  return true;
}

void items::sCopy()
{
  ParameterList params;
  params.append("item_id", list()->id());

  copyItem newdlg(this, "", true);
  newdlg.set(params);
  newdlg.exec();
}
