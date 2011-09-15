/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspQOHByLocation.h"

#include <QAction>
#include <QMenu>
#include <QSqlError>
#include <QVariant>

#include "inputManager.h"
#include "relocateInventory.h"

dspQOHByLocation::dspQOHByLocation(QWidget* parent, const char*, Qt::WFlags fl)
    : display(parent, "dspQOHByLocation", fl)
{
  setupUi(optionsWidget());
  setWindowTitle(tr("Quantities on Hand by Location"));
  setListLabel(tr("Location"));
  setReportName("QOHByLocation");
  setMetaSQLOptions("qoh", "detail");

  connect(_warehouse, SIGNAL(updated()), this, SLOT(sPopulateLocations()));

  omfgThis->inputManager()->notify(cBCLocation, this, this, SLOT(set(int)));

  _location->setAllowNull(true);
  _asOf->setDate(omfgThis->dbDate(), true);

  list()->setRootIsDecorated(true);
  list()->addColumn(tr("Site"),         _itemColumn, Qt::AlignLeft,   true,  "warehous_code"   );
  list()->addColumn(tr("Item Number"),  _itemColumn, Qt::AlignLeft,   true,  "item_number"   );
  list()->addColumn(tr("Description"),  -1,          Qt::AlignLeft,   true,  "f_descrip"   );
  list()->addColumn(tr("Lot/Serial #"), 150,         Qt::AlignLeft,   true,  "f_lotserial"   );
  list()->addColumn(tr("UOM"),          _uomColumn,  Qt::AlignCenter, true,  "uom_name" );
  list()->addColumn(tr("QOH"),          _qtyColumn,  Qt::AlignRight,  true,  "qoh"  );
  list()->addColumn(tr("Reserved"),     _qtyColumn,  Qt::AlignRight,  false, "reservedqty"  );
  
  if(_metrics->boolean("EnableSOReservationsByLocation"))
    list()->showColumn(6);

  _asofGroup->hide();; // Issue #11793 - Not ready for this yet.

  sPopulateLocations();
}

void dspQOHByLocation::languageChange()
{
  display::languageChange();
  retranslateUi(this);
}

enum SetResponse dspQOHByLocation::set(const ParameterList &pParams)
{
  XWidget::set(pParams);
  QVariant param;
  bool     valid;
  
  param = pParams.value("location_id", &valid);
  if (valid)
  {
    XSqlQuery qq;
    qq.prepare( "SELECT location_warehous_id "
                "FROM location "
                "WHERE (location_id=:location_id);" );
    qq.bindValue(":location_id", param.toInt());
    qq.exec();
    if (qq.first())
    {
      _warehouse->setId(qq.value("location_warehous_id").toInt());
      _location->setId(param.toInt());
    }
  }
  
  if (pParams.inList("run"))
  {
    sFillList();
    return NoError_Run;
  }

  return NoError;
}

void dspQOHByLocation::sPopulateLocations()
{
  if (_warehouse->isAll())
    _location->populate( "SELECT location_id,"
                         "       CASE WHEN (LENGTH(location_descrip) > 0) THEN (warehous_code || '-' || formatLocationName(location_id) || '-' || location_descrip)"
                         "            ELSE (warehous_code || '-' || formatLocationName(location_id))"
                         "       END AS locationname "
                         "FROM location, warehous "
                         "WHERE (location_warehous_id=warehous_id) "
                         "ORDER BY locationname;" );
  else
  {
    XSqlQuery qq;
    qq.prepare( "SELECT location_id, "
                "       CASE WHEN (LENGTH(location_descrip) > 0) THEN (formatLocationName(location_id) || '-' || location_descrip)"
                "            ELSE formatLocationName(location_id)"
                "       END AS locationname "
                "FROM location "
                "WHERE (location_warehous_id=:warehous_id) "
                "ORDER BY locationname;" );
    qq.bindValue(":warehous_id", _warehouse->id());
    qq.exec();
    _location->populate(qq);
  }
}

void dspQOHByLocation::sRelocate()
{
  ParameterList params;
  params.append("itemloc_id", list()->id());

  relocateInventory newdlg(this, "", true);
  newdlg.set(params);

  if (newdlg.exec())
    sFillList();
}

void dspQOHByLocation::sPopulateMenu(QMenu *menu, QTreeWidgetItem*, int)
{
  QAction *menuItem;

  if (list()->id() != -1)
  {
    menuItem = menu->addAction(tr("Relocate..."), this, SLOT(sRelocate()));
    menuItem->setEnabled(_privileges->check("RelocateInventory"));
  }
}

void dspQOHByLocation::sFillList()
{
  list()->clear();

  if (_location->id() != -1)
  {
    XSqlQuery qq;
    qq.prepare( "SELECT formatBoolYN(location_netable) AS netable,"
                "       formatBoolYN(location_restrict) AS restricted "
                "FROM location, warehous "
                "WHERE ( (location_warehous_id=warehous_id)"
                " AND (location_id=:location_id));" );
    qq.bindValue(":location_id", _location->id());
    qq.exec();
    if (qq.first())
    {
      _netable->setText(qq.value("netable").toString());
      _restricted->setText(qq.value("restricted").toString());
    }

    display::sFillList();
  }
  else
  {
    _netable->clear();
    _restricted->clear();
    list()->clear();
  }
}

bool dspQOHByLocation::setParams(ParameterList &params)
{
  params.append("byLocation");
  params.append("asOf", _asOf->date());

  params.append("na", tr("N/A"));

  _warehouse->appendValue(params);

  if (_location->id() != -1)
    params.append("location_id", _location->id());

  if (_metrics->boolean("EnableSOReservationsByLocation"))
    params.append("EnableSOReservationsByLocation");

  return true;
}
