/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include <metasql.h>
#include <parameter.h>
#include <xsqlquery.h>

#include "widgets.h"
#include "warehouseCluster.h"

WComboBox::WComboBox(QWidget *parent, const char *name) : XComboBox(parent, name)
{
  setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
  setMinimumWidth(80);

  setType(AllActive);
}

void WComboBox::setAllowNull(bool pAllowNull)
{
  _allowNull = pAllowNull;
  setType(_type);
}

WComboBox::WComboBoxTypes WComboBox::type()
{
  return _type;
}

void WComboBox::setType(WComboBoxTypes pType)
{
  _type = pType;
  
  // If we're in Designer, don't populate
  QObject *ancestor = this;
  bool designMode;
  for ( ; ancestor; ancestor = ancestor->parent())
  {
    designMode = ancestor->inherits("xTupleDesigner");
    if (designMode)
      return;
  } 
  
  int warehousid = ((_x_preferences) ? _x_preferences->value("PreferredWarehouse").toInt() : -1);

  QString whss("SELECT warehous_id, warehous_code, warehous_code "
             "FROM site() "
             "WHERE (true"
             "<? if exists(\"active\") ?>  AND (warehous_active)  <? endif ?>"
             "<? if exists(\"shipping\") ?>AND (warehous_shipping)<? endif ?>"
             "<? if exists(\"transit\") ?>"
             "  AND (warehous_transit) "
             "<? elseif exists(\"nottransit\") ?>"
             "  AND (NOT warehous_transit)"
             "<? endif ?>"
             ") "
             "ORDER BY warehous_code;" );

  ParameterList whsp;

  switch (_type)
  {
    case AllActive:
      whsp.append("active");
      break;

    case Sold:
    case NonTransit:
      whsp.append("nottransit");
      whsp.append("active");
      break;

    case Shipping:
      whsp.append("shipping");
      whsp.append("active");
      break;

    case Supply:
      clear();
      return; // yes, the previous version just had "clear(); return;"
      break;

    case Transit:
      whsp.append("transit");
      whsp.append("active");
      break;

    case All:
    default:
      break;

  }

  MetaSQLQuery whsm(whss);
  XSqlQuery whsq = whsm.toQuery(whsp);
  populate(whsq, warehousid);
}

void WComboBox::findItemsites(int pItemID)
{
  if (pItemID != -1)
  {
    QString iss("SELECT warehous_id, warehous_code, warehous_code "
               "FROM site(), itemsite "
               "WHERE ((itemsite_warehous_id=warehous_id)"
               "  AND  (itemsite_item_id=<? value(\"itemid\") ?>) "
               "<? if exists(\"active\") ?>  AND (warehous_active)  <? endif ?>"
               "<? if exists(\"shipping\") ?>AND (warehous_shipping)<? endif ?>"
               "<? if exists(\"transit\") ?>"
               "  AND (warehous_transit) "
               "<? elseif exists(\"nottransit\") ?>"
               "  AND (NOT warehous_transit)"
               "<? endif ?>"
               "<? if exists(\"active\") ?>    AND (itemsite_active)  <? endif ?>"
               "<? if exists(\"soldIS\") ?>    AND (itemsite_sold)    <? endif ?>"
               "<? if exists(\"supplyIS\") ?>  AND ( (itemsite_wosupply) OR (itemsite_posupply) OR (itemsite_supply_itemsite_id IS NOT NULL) )  <? endif ?>"
               "<? if exists(\"inventory\") ?> AND (itemsite_controlmethod<>'N')  <? endif ?>"
               ") "
               "ORDER BY warehous_code;" );
    ParameterList isp;
    isp.append("itemid", pItemID);

    switch (_type)
    {
      case AllActive:
        isp.append("active");
        break;

      case AllActiveInventory:
        isp.append("active");
        isp.append("inventory");
        break;

      case NonTransit:
        isp.append("nottransit");
        isp.append("active");
        break;

      case Sold:
        isp.append("active");
        isp.append("nottransit");
        isp.append("soldIS");
        break;

      /* TODO: ? previous version of this function didn't have Shipping case
      case Shipping:
        isp.append("shipping");
        isp.append("active");
        break;
      */

      case Supply:
        isp.append("active");
        isp.append("supplyIS");
        break;

      case Transit:
        isp.append("transit");
        isp.append("active");
        break;

      case All:
      default:
        break;
    }

    MetaSQLQuery ism(iss);
    XSqlQuery isq = ism.toQuery(isp);
    populate(isq, ((_x_preferences) ?
                    _x_preferences->value("PreferredWarehouse").toInt() : -1));

    if (currentIndex() == -1)
      setCurrentIndex(0);

  }
  else
    clear();
}

void WComboBox::setId(int pId)
{
  XComboBox::setId(pId);
}
