/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "warehouses.h"

#include <QAction>
#include <QMenu>
#include <QMessageBox>
#include <QVariant>

#include <metasql.h>

#include <openreports.h>
#include "itemSites.h"
#include "warehouse.h"

warehouses::warehouses(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_new, SIGNAL(clicked()), this, SLOT(sNew()));
  connect(_edit, SIGNAL(clicked()), this, SLOT(sEdit()));
  connect(_warehouse, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*)));
  connect(_view, SIGNAL(clicked()), this, SLOT(sView()));
  connect(_showInactive, SIGNAL(toggled(bool)), this, SLOT(sFillList()));
  
  _warehouse->addColumn(tr("Site"),        _whsColumn, Qt::AlignCenter, true,  "warehous_code" );
  _warehouse->addColumn(tr("Active"),      _ynColumn,  Qt::AlignCenter, true,  "warehous_active" );
  _warehouse->addColumn(tr("Type"),        _whsColumn, Qt::AlignCenter, true,  "sitetype_name" );
  _warehouse->addColumn(tr("Description"), 130,        Qt::AlignLeft,   true,  "warehous_descrip"   );
  _warehouse->addColumn(tr("Address"),     -1,         Qt::AlignLeft,   true,  "addr_line1"   );

  if (_privileges->check("MaintainWarehouses"))
  {
    connect(_warehouse, SIGNAL(valid(bool)), _edit, SLOT(setEnabled(bool)));
    connect(_warehouse, SIGNAL(itemSelected(int)), _edit, SLOT(animateClick()));
  }
  else
  {
    _new->setEnabled(FALSE);
    connect(_warehouse, SIGNAL(itemSelected(int)), _view, SLOT(animateClick()));
  }

  connect(omfgThis, SIGNAL(warehousesUpdated()),  SLOT(sFillList())  );

  sFillList();
}

warehouses::~warehouses()
{
  // no need to delete child widgets, Qt does it all for us
}

void warehouses::languageChange()
{
  retranslateUi(this);
}

void warehouses::sNew()
{
  ParameterList params;
  params.append("mode", "new");

  warehouse newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void warehouses::sView()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("warehous_id", _warehouse->id());

  warehouse newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void warehouses::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("warehous_id", _warehouse->id());

  warehouse newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void warehouses::setParams(ParameterList & params)
{
  if (_showInactive->isChecked())
    params.append("showInactive");
}

void warehouses::sFillList()
{
  QString whss = "SELECT warehous_id, warehous_code,"
                 "       warehous_active, sitetype_name,"
                 "       warehous_descrip, addr_line1 "
                 "FROM site() LEFT OUTER JOIN addr ON (addr_id=warehous_addr_id) "
                 "  LEFT OUTER JOIN sitetype ON (sitetype_id=warehous_sitetype_id) "
                 "WHERE ((TRUE) "
                 "<? if not exists(\"showInactive\") ?>"
                 "  AND (warehous_active) "
                 "<? endif ?>"
                 ") ORDER BY warehous_code;" ;
  ParameterList whsp;
  setParams(whsp);
  MetaSQLQuery whsm(whss);
  XSqlQuery whsq = whsm.toQuery(whsp);

  _warehouse->populate(whsq);
}

void warehouses::sListItemSites()
{
  ParameterList params;
  params.append("run");
  params.append("warehous_id", _warehouse->id());

  itemSites *newdlg = new itemSites();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void warehouses::sPopulateMenu( QMenu * pMenu )
{
  QAction *menuItem;

  menuItem = pMenu->addAction(tr("Edit..."), this, SLOT(sEdit()));
  menuItem->setEnabled(_privileges->check("MaintainWarehouses"));

  pMenu->addAction(tr("View..."), this, SLOT(sView()));
  pMenu->addSeparator();

  menuItem = pMenu->addAction(tr("List Item Sites..."), this, SLOT(sListItemSites()));
  menuItem->setEnabled(_privileges->check("ViewItemSites"));
}

void warehouses::sPrint()
{
  ParameterList params;
  setParams(params);

  orReport report("WarehouseMasterList", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}
