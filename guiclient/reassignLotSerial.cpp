/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "reassignLotSerial.h"

#include "math.h"

#include <QMessageBox>
#include <QSqlError>
#include <QValidator>
#include <QVariant>

#include "storedProcErrorLookup.h"

reassignLotSerial::reassignLotSerial(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
    setupUi(this);

    connect(_warehouse, SIGNAL(newID(int)), this, SLOT(sFillList()));
    connect(_reassign, SIGNAL(clicked()), this, SLOT(sReassign()));

    _captive = FALSE;

    _item->setType(ItemLineEdit::cLotSerialControlled);
    _qty->setValidator(omfgThis->qtyPerVal());

    _source->addColumn(tr("Location"),     _itemColumn, Qt::AlignLeft  , true, "locationname"   );
    _source->addColumn(tr("Lot/Serial #"), -1,          Qt::AlignLeft  , true, "ls_number" );
    _source->addColumn(tr("Expires"),      _dateColumn, Qt::AlignCenter, true, "itemloc_expiration" );
    _source->addColumn(tr("Warranty"),     _dateColumn, Qt::AlignCenter, true, "itemloc_warrpurc");
    _source->addColumn(tr("Qty."),         _qtyColumn,  Qt::AlignRight , true, "itemloc_qty" );
    
    //If not multi-warehouse hide whs control
    if (!_metrics->boolean("MultiWhs"))
    {
      _warehouseLit->hide();
      _warehouse->hide();
    }
}

reassignLotSerial::~reassignLotSerial()
{
    // no need to delete child widgets, Qt does it all for us
}

void reassignLotSerial::languageChange()
{
    retranslateUi(this);
}

enum SetResponse reassignLotSerial::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("itemsite_id", &valid);
  if (valid)
  {
    _captive = TRUE;

    _item->setItemsiteid(param.toInt());
    _item->setReadOnly(TRUE);
  }

  param = pParams.value("itemloc_id", &valid);
  if (valid)
  {
    q.prepare( "SELECT itemloc_itemsite_id "
               "FROM itemloc "
               "WHERE (itemloc_id=:itemloc_id);" );
    q.bindValue(":itemloc_id", param.toInt());
    q.exec();
    if (q.first())
    {
      _captive = TRUE;

      _item->setItemsiteid(q.value("itemloc_itemsite_id").toInt());
      _item->setReadOnly(TRUE);
      _warehouse->setEnabled(FALSE);

      for (int i = 0; i < _source->topLevelItemCount(); i++)
      {
        if (((XTreeWidget*)(_source->topLevelItem(i)))->id() == param.toInt())
          _source->setCurrentItem(_source->topLevelItem(i));
      }
    }
    else if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return UndefinedError;
    }
  }

  return NoError;
}

void reassignLotSerial::sReassign()
{
  if (_expirationDate->isEnabled())
  {
    if (!_expirationDate->isValid() || _expirationDate->isNull())
    {
      QMessageBox::critical( this, tr("Enter a valid date"),
                             tr("You must enter a valid expiration date before you can continue.") );
      _expirationDate->setFocus();
      return;
    }
  }
	
  if (_source->currentItem() == 0)
  {
    QMessageBox::critical( this, tr("Select Source Location"),
                           tr("You must select a Source Location before reassigning its Lot/Serial #.") );
    _source->setFocus();
    return;
  }

  if (_qty->toDouble() == 0)
  {
    QMessageBox::critical( this, tr("Enter Quantity to Reassign"),
                           tr("You must enter a quantity to reassign.") );
    _qty->setFocus();
    return;
  }

  if (_lotNumber->text().length() == 0)
  {
    QMessageBox::critical( this, tr("Enter New Lot Number to Reassign"),
                           tr("You must enter a New Lot Number to reassign.") );
    _qty->setFocus();
    return;
  }

  QDoubleValidator* qtyVal = (QDoubleValidator*)(_qty->validator());
  q.prepare("SELECT reassignLotSerial(:source, CAST (:qty AS NUMERIC(100,:decimals)), "
	    "                         :lotNumber, :expirationDate, :warrantyDate) AS result;");
  q.bindValue(":source", _source->id());
  q.bindValue(":qty", _qty->toDouble());
  q.bindValue(":decimals", qtyVal->decimals());
  q.bindValue(":lotNumber", _lotNumber->text());

  if (_expirationDate->isEnabled())
    q.bindValue(":expirationDate", _expirationDate->date());
  else
    q.bindValue(":expirationDate", omfgThis->endOfTime());

  if (_warrantyDate->isEnabled())
    q.bindValue(":warrantyDate", _warrantyDate->date());

  q.exec();
  if (q.first())
  {
    int result = q.value("result").toInt();
    if (result < 0)
    {
      systemError(this, storedProcErrorLookup("reassignLotSerial", result),
		  __FILE__, __LINE__);
      return;
    }
  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  if (_captive)
    accept();
  else
  {
    _close->setText(tr("&Close"));

    sFillList();

    if (_qty->isEnabled())
      _qty->clear();

    _qty->setFocus();
    _lotNumber->clear();
    _expirationDate->setNull();
    _warrantyDate->setNull();
  }
}

void reassignLotSerial::sFillList()
{
  if (_item->isValid())
  {
    q.prepare( "SELECT itemsite_id, itemsite_perishable, itemsite_controlmethod, itemsite_warrpurc "
               "FROM itemsite "
               "WHERE ( (itemsite_item_id=:item_id)"
               " AND (itemsite_warehous_id=:warehous_id) );" );
    q.bindValue(":item_id", _item->id());
    q.bindValue(":warehous_id", _warehouse->id());
    q.exec();
    if (q.first())
    {
      if (q.value("itemsite_controlmethod").toString() == "S")
      {
        _qty->setEnabled(FALSE);
        _qty->setDouble(1.0);
      }
      else if (q.value("itemsite_controlmethod").toString() == "L")
        _qty->setEnabled(TRUE);
      else
      {
        _source->clear();
        return;
      }

      int itemsiteid = q.value("itemsite_id").toInt();
      _expirationDate->setEnabled(q.value("itemsite_perishable").toBool());
      _warrantyDate->setEnabled(q.value("itemsite_warrpurc").toBool());

      q.prepare( "SELECT itemloc_id, formatLocationName(itemloc_location_id) AS locationname, ls_number,"
                 "       itemloc_expiration, itemloc_warrpurc, itemloc_qty, "
                 "       'qty' AS itemloc_qty_xtnumericrole "
                 "FROM itemloc "
                 "  LEFT OUTER JOIN ls ON (itemloc_ls_id=ls_id), itemsite "
                 "WHERE ( (itemloc_itemsite_id=itemsite_id)"
                 " AND (itemsite_id=:itemsite_id) ) "
                 "ORDER BY locationname;" );
      q.bindValue(":never", tr("Never"));
      q.bindValue(":itemsite_id", itemsiteid);
      q.exec();
      _source->populate(q);
    }
    else if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }
  else
    _source->clear();
}
