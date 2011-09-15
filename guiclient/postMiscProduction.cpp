/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "postMiscProduction.h"

#include <QMessageBox>
#include <QVariant>

#include "distributeInventory.h"
#include "storedProcErrorLookup.h"

postMiscProduction::postMiscProduction(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_post, SIGNAL(clicked()), this, SLOT(sPost()));

  _captive = FALSE;
  _itemsiteid = -1;
  _sense = 1;
  _qty = 0;

  _item->setType(ItemLineEdit::cManufactured);
  _qtyToPost->setValidator(omfgThis->qtyVal());
  
  _immediateTransfer->setEnabled(_privileges->check("CreateInterWarehouseTrans"));
    
  //If not multi-warehouse hide whs control
  if (!_metrics->boolean("MultiWhs")) 
  {
    _warehouseLit->hide();
    _warehouse->hide();
    _immediateTransfer->hide();
    _transferWarehouse->hide();
  }
  else
    _transferWarehouse->setEnabled(_immediateTransfer->isChecked());

  if (_preferences->boolean("XCheckBox/forgetful"))
    _backflush->setChecked(true);

  _nonPickItems->setEnabled(_backflush->isChecked() &&
			    _privileges->check("ChangeNonPickItems"));

  // TODO: unhide as part of implementation of 5847
  _nonPickItems->hide();
}

postMiscProduction::~postMiscProduction()
{
  // no need to delete child widgets, Qt does it all for us
}

void postMiscProduction::languageChange()
{
  retranslateUi(this);
}

enum SetResponse postMiscProduction::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  _captive = TRUE;

  QVariant param;
  bool     valid;

  param = pParams.value("itemsite_id", &valid);
  if (valid)
  {
    _item->setItemsiteid(param.toInt());
    _warehouse->setEnabled(FALSE);
    _item->setReadOnly(TRUE);

    _qtyToPost->setFocus();
  }

  return NoError;
}

void postMiscProduction::sPost()
{
  _qty = _qtyToPost->toDouble();
  if (_disassembly->isChecked())
    _qty = _qty * -1;
  
  if (_qty == 0)
  {
    QMessageBox::warning( this, tr("Invalid Quantity"),
                        tr( "The quantity may not be zero." ) );
    return;
  }
  
  if (_immediateTransfer->isChecked())
  {
    if (_warehouse->id() == _transferWarehouse->id())
    {
      QMessageBox::warning( this, tr("Cannot Post Immediate Transfer"),
                            tr( "Transaction canceled. Cannot post an immediate transfer for the newly posted production as the\n"
                                "transfer Site is the same as the production Site.  You must manually\n"
                                "transfer the production to the intended Site." ) );
      return;
    }
  }
  
  q.prepare( "SELECT itemsite_id "
             "FROM itemsite "
             "WHERE ( (itemsite_item_id=:item_id)"
             " AND (itemsite_warehous_id=:warehous_id) );" );
  q.bindValue(":item_id", _item->id());
  if (_qty > 0)
    q.bindValue(":warehous_id", _warehouse->id());
  else
    q.bindValue(":warehous_id", _transferWarehouse->id());
  q.exec();
  if (q.first())
  {
    _itemsiteid = q.value("itemsite_id").toInt();

    XSqlQuery rollback;
    rollback.prepare("ROLLBACK;");

    q.exec("BEGIN;");	// because of possible lot, serial, or location distribution cancelations
    
    if (_qty > 0)
    {
      if (post())
      {
        if (_immediateTransfer->isChecked())
        {
          if (transfer())
            q.exec("COMMIT;");
          else
            rollback.exec();
        }
        else
          q.exec("COMMIT;");
      }
      else
        rollback.exec();
    }
    else
    {
      _sense = -1;
      if (_immediateTransfer->isChecked())
      {
        if (transfer())
        {
          if (post())
            q.exec("COMMIT;");
          else
            rollback.exec();
        }
        else
          rollback.exec();
      }
      else
      {
        if (post())
          q.exec("COMMIT;");
        else
          rollback.exec();
      }
    }

    if (_captive)
      accept();
    else
    {
      _item->setId(-1);
      _qtyToPost->clear();
      _documentNum->clear();
      _comments->clear();
      _close->setText(tr("&Close"));

      _item->setFocus();
    }
  }
  else
    systemError(this, tr("A System Error occurred at %1::%2, Item Number %3.")
                      .arg(__FILE__)
                      .arg(__LINE__)
                      .arg(_item->itemNumber()) );
}

bool postMiscProduction::post()
{
  q.prepare( "SELECT postMiscProduction( :itemsite_id, :qty, :backflushMaterials,"
             "                           :docNumber, :comments ) AS result;" );
  q.bindValue(":itemsite_id", _itemsiteid);
  q.bindValue(":qty", _qty);
  q.bindValue(":backflushMaterials", QVariant(_backflush->isChecked()));
  q.bindValue(":docNumber", _documentNum->text().trimmed());
  q.bindValue(":comments", _comments->toPlainText());
  q.exec();
  if (q.first())
  {
    if (q.value("result").toInt() < 0)
    {
      systemError(this, storedProcErrorLookup("postProduction", q.value("result").toInt()),
                  __FILE__, __LINE__);
      return false;
    }
    else
    {
      if (distributeInventory::SeriesAdjust(q.value("result").toInt(), this) == XDialog::Rejected)
      {
        QMessageBox::information( this, tr("Post Misc. Production"), tr("Transaction Canceled") );
        return false;
      }
    }
  }
  return true;
}

bool postMiscProduction::transfer()
{
  q.prepare( "SELECT interWarehouseTransfer( :item_id, :from_warehous_id, :to_warehous_id,"
             "                               :qty, 'W', :documentNumber, 'Transfer from Misc. Production Posting' ) AS result;" );
  q.bindValue(":item_id", _item->id());
  q.bindValue(":from_warehous_id", _warehouse->id());
  q.bindValue(":to_warehous_id", _transferWarehouse->id());
  q.bindValue(":qty", _qty * _sense);
  q.bindValue(":documentNumber", _documentNum->text().trimmed());
  q.exec();
  if (q.first())
  {
    if (distributeInventory::SeriesAdjust(q.value("result").toInt(), this) == XDialog::Rejected)
    {
      QMessageBox::information( this, tr("Post Misc. Production"), tr("Transaction Canceled") );
      return false;
    }
  }
  else
  {
    systemError( this, tr("A System Error occurred at interWarehousTransfer::%1, Item Site ID #%2, Site ID #%3 to Site ID #%4.")
                       .arg(__LINE__)
                       .arg(_item->id())
                       .arg(_warehouse->id())
                       .arg(_transferWarehouse->id()));
    return false;
  }
  return true;
}



