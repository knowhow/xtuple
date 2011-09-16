/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "relocateInventory.h"

#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include "inputManager.h"
#include "storedProcErrorLookup.h"

relocateInventory::relocateInventory(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_warehouse, SIGNAL(newID(int)), this, SLOT(sFillList()));
  connect(_move, SIGNAL(clicked()), this, SLOT(sMove()));

  _captive = FALSE;

  _item->setType(ItemLineEdit::cLocationControlled);
  _qty->setValidator(omfgThis->transQtyVal());

  omfgThis->inputManager()->notify(cBCItem, this, _item, SLOT(setItemid(int)));
  omfgThis->inputManager()->notify(cBCItemSite, this, _item, SLOT(setItemsiteid(int)));

  _source->addColumn(tr("Location"),_itemColumn, Qt::AlignLeft, true, "location");
  _source->addColumn(tr("Lot/Serial #"),     -1, Qt::AlignLeft, true, "lotserial");
  _source->addColumn(tr("Qty."),     _qtyColumn, Qt::AlignRight,true, "itemloc_qty");

  _target->addColumn(tr("Location"),     -1, Qt::AlignLeft,  true, "locationname");
  _target->addColumn(tr("Qty."), _qtyColumn, Qt::AlignRight, true, "qty");

  if (!_metrics->boolean("MultiWhs"))
  {
    _warehouseLit->hide();
    _warehouse->hide();
  }

  _defaultToTarget->hide();

  _transDate->setEnabled(_privileges->check("AlterTransactionDates"));
  _transDate->setDate(omfgThis->dbDate());

}

relocateInventory::~relocateInventory()
{
  // no need to delete child widgets, Qt does it all for us
}

void relocateInventory::languageChange()
{
  retranslateUi(this);
}

enum SetResponse relocateInventory::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("itemsite_id", &valid);
  if (valid)
  {
    _captive = TRUE;

    _item->setItemsiteid(param.toInt());
    _item->setEnabled(FALSE);
    _warehouse->setEnabled(FALSE);
    _source->setFocus();
  }

  int locid;
  param = pParams.value("source_location_id", &valid);
  if (valid)
  {
    locid = param.toInt();
    for (int i = 0; i < _source->topLevelItemCount(); i++)
    {
      XTreeWidgetItem* cursor = (XTreeWidgetItem*)(_source->topLevelItem(i));
      if (cursor->altId() == locid)
      {
        _source->setCurrentItem(cursor);
        _source->scrollToItem(cursor);
        _source->setEnabled(false);
      }
    }
  }

  param = pParams.value("target_location_id", &valid);
  if (valid)
  {
    locid = param.toInt();
    for (int i = 0; i < _target->topLevelItemCount(); i++)
    {
      XTreeWidgetItem* cursor = (XTreeWidgetItem*)(_target->topLevelItem(i));
      if (cursor->id() == locid)
      {
        _target->setCurrentItem(cursor);
        _target->scrollToItem(cursor);
        _target->setEnabled(false);
      }
    }
  }

  param = pParams.value("itemloc_id", &valid);
  if (valid)
  {
    _captive = TRUE;

    XSqlQuery itemsiteid;
    itemsiteid.prepare( "SELECT itemloc_itemsite_id "
                        "FROM itemloc "
                        "WHERE (itemloc_id=:itemloc_id);" );
    itemsiteid.bindValue(":itemloc_id", param.toInt());
    itemsiteid.exec();
    if (itemsiteid.first())
    {
      _item->setItemsiteid(itemsiteid.value("itemloc_itemsite_id").toInt());
      _item->setEnabled(FALSE);
      _warehouse->setEnabled(FALSE);

      for (int i = 0; i < _source->topLevelItemCount(); i++)
      {
	XTreeWidgetItem* cursor = (XTreeWidgetItem*)(_source->topLevelItem(i));
        if (cursor->id() == param.toInt())
        {
          _source->setCurrentItem(cursor);
          _source->scrollToItem(cursor);
        }
      }

      _source->setEnabled(FALSE);
    }
  }

  param = pParams.value("qty", &valid);
  if (valid)
  {
    _qty->setDouble(param.toDouble());
    _qty->setEnabled(false);
  }

  return NoError;
}

void relocateInventory::sMove()
{
  struct {
    bool        condition;
    QString     msg;
    QWidget     *widget;
  } error[] = {
    { ! _item->isValid(),
      tr("You must select an Item before posting this transaction."), _item },
    { _qty->text().length() == 0 || _qty->toDouble() <= 0,
      tr("<p>You must enter a positive Quantity before posting this Transaction."),
      _qty },
    { _source->id() < 0,
      tr("<p>You must select a Source Location before relocating Inventory."),
      _source },
    { _target->id() < 0,
      tr("<p>You must select a Target Location before relocating Inventory."),
      _target },
    { _source->id() > 0 && _target->id() > 0 &&
      ((XTreeWidgetItem *)_source->currentItem())->id() ==
        ((XTreeWidgetItem *)_target->currentItem())->id(),
      tr("<p>Please select different Locations for the Source and Target."),
      _target },
    { true, "", NULL }
  };

  int errIndex;
  for (errIndex = 0; ! error[errIndex].condition; errIndex++)
    ;
  if (! error[errIndex].msg.isEmpty())
  {
    QMessageBox::critical(this, tr("Cannot Post Transaction"),
                          error[errIndex].msg);
    error[errIndex].widget->setFocus();
    return;
  }

  XSqlQuery relocate;
  relocate.prepare( "SELECT relocateInventory(:source, :target, itemsite_id,"
                    ":qty, :comments, :date) AS result "
                    "FROM itemsite "
                    "WHERE ( (itemsite_item_id=:item_id)"
                    " AND (itemsite_warehous_id=:warehous_id));" );
  relocate.bindValue(":source", _source->id());
  relocate.bindValue(":target", _target->id());
  relocate.bindValue(":qty", _qty->toDouble());
  relocate.bindValue(":comments", _notes->toPlainText().trimmed());
  relocate.bindValue(":item_id", _item->id());
  relocate.bindValue(":warehous_id", _warehouse->id());
  relocate.bindValue(":date",           _transDate->date());
  relocate.exec();

  if (relocate.first())
  {
    int result = relocate.value("result").toInt();
    if (result < 0)
    {
      systemError(this, storedProcErrorLookup("relocateInventory", result),
                  __FILE__, __LINE__);
      return;
    }
  }
  else if (relocate.lastError().type() != QSqlError::NoError)
  {
    systemError(this, relocate.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
  
  sChangeDefaultLocation();

  if (_captive)
    accept();
  else
  {
    _close->setText(tr("&Close"));

    sFillList();
    _qty->clear();
    _notes->clear();
    _transDate->setDate(omfgThis->dbDate());
    _item->setFocus();
  }
}

void relocateInventory::sFillList()
{
  sShowHideDefaultToTarget();
  
  if (_item->isValid())
  {
    XSqlQuery query;
    query.prepare( "SELECT itemloc.*,"
                   "       CASE WHEN (itemloc_location_id=-1) THEN :undefined"
                   "            ELSE formatLocationName(itemloc_location_id)"
                   "       END AS location,"
                   "       formatlotserialnumber(itemloc_ls_id) AS lotserial,"
                   "       'qty' AS itemloc_qty_xtnumericrole,"
                   "       CASE WHEN (itemloc_location_id=itemsite_location_id) THEN 'altemphasis' "
                   "       END AS location_qtforegroundrole "
                   "FROM itemloc, itemsite "
                   "WHERE ( (itemloc_itemsite_id=itemsite_id)"
                   " AND (itemsite_item_id=:item_id)"
                   " AND (itemsite_warehous_id=:warehous_id) ) "
                   "ORDER BY location;" );
    query.bindValue(":undefined", tr("Undefined"));
    query.bindValue(":item_id", _item->id());
    query.bindValue(":warehous_id", _warehouse->id());
    query.exec();
    _source->populate(query, true);
    if (query.lastError().type() != QSqlError::NoError)
    {
      systemError(this, query.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }

    query.prepare( "SELECT location_id, locationname, qty,"
                   "       'qty' AS qty_xtnumericrole,"
                   "       CASE WHEN (qty=0) THEN '' "
                   "       END AS qty_qtdisplayrole,"
                   "       CASE WHEN (location_id=itemsite_location_id) THEN 'altemphasis' "
                   "       END AS locationname_qtforegroundrole "
                   "FROM ( "
                   "SELECT location_id, itemsite_location_id,"
                   "       formatLocationName(location_id) AS locationname,"
                   "       ( SELECT COALESCE(SUM(itemloc_qty), 0)"
                   "         FROM itemloc, itemsite"
                   "         WHERE ( (itemloc_location_id=location_id)"
                   "         AND (itemloc_itemsite_id=itemsite_id)"
                   "         AND (itemsite_item_id=:item_id)"
                   "         AND (itemsite_warehous_id=location_warehous_id))) AS qty "
                   "FROM location, itemsite "
                   "WHERE ( (itemsite_warehous_id=:warehous_id)"
                   " AND (location_warehous_id=:warehous_id)"
                   " AND (itemsite_item_id=:item_id)"
                   " AND  (validLocation(location_id, itemsite_id)) ) "
                   "ORDER BY locationname ) AS data" );
    query.bindValue(":warehous_id", _warehouse->id());
    query.bindValue(":item_id", _item->id());
    query.exec();
    _target->populate(query);
    if (query.lastError().type() != QSqlError::NoError)
    {
      systemError(this, query.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }
  else
  {
    _source->clear();
    _target->clear();
  }
}

void relocateInventory::sShowHideDefaultToTarget()
{
   if (!_metrics->boolean("SetDefaultLocations"))
     return;
    
   XSqlQuery query;
   query.prepare(" SELECT itemsite_id, itemsite_loccntrl, itemsite_location_id "
                 "  FROM itemsite "
                 "  WHERE (itemsite_item_id=:item_id) "
                 "  AND (itemsite_warehous_id=:warehous_id);");
   query.bindValue(":item_id", _item->id());
    query.bindValue(":warehous_id", _warehouse->id());
   query.exec();
   if(query.first())
   {
      if(query.value("itemsite_id").toInt() != -1
         && query.value("itemsite_loccntrl").toBool())
      {
         _defaultToTarget->show();
        //Allow default location update with correct privileges
        if (_privileges->check("MaintainItemSites"))
            _defaultToTarget->setEnabled(true);
        else
            _defaultToTarget->setEnabled(false);
            _defaultToTarget->setChecked(false);
      }
      else
      {
         _defaultToTarget->hide();
         _defaultToTarget->setChecked(false);
      }
   }
   else
   {
     _defaultToTarget->hide();
     _defaultToTarget->setChecked(false);
   }
}

void relocateInventory::sChangeDefaultLocation()
{
   if (_defaultToTarget->isChecked())
   {
   XSqlQuery query;
   query.prepare( " UPDATE itemsite"
                  " SET itemsite_location_id=:target_id"
                  " WHERE (itemsite_item_id=:item_id) ");
   query.bindValue(":target_id", _target->id());
   query.bindValue(":item_id", _item->id());
   query.exec();
   sFillList();
   if (query.lastError().type() != QSqlError::NoError)
      {
        systemError(this, query.lastError().databaseText(), __FILE__, __LINE__);
        return;
      }
   }

}
