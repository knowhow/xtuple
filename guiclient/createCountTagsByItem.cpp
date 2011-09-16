/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "createCountTagsByItem.h"

#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

createCountTagsByItem::createCountTagsByItem(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_create,    SIGNAL(clicked()),  this, SLOT(sCreate()));
  connect(_warehouse, SIGNAL(newID(int)), this, SLOT(sPopulateLocations()));

  _freeze->setEnabled(_privileges->check("FreezeInventory"));
  _item->setType(ItemLineEdit::cPurchased
                 | ItemLineEdit::cManufactured
                 | ItemLineEdit::cBreeder
                 | ItemLineEdit::cCoProduct
                 | ItemLineEdit::cByProduct
                 | ItemLineEdit::cCosting
                 | ItemLineEdit::cTooling
                 | ItemLineEdit::cOutsideProcess
                 | ItemLineEdit::cActive);

  _captive = FALSE;
  
  if (!_metrics->boolean("MultiWhs"))
  {
    _warehouseLit->hide();
    _warehouse->hide();
  }

  sPopulateLocations();
}

createCountTagsByItem::~createCountTagsByItem()
{
  // no need to delete child widgets, Qt does it all for us
}

void createCountTagsByItem::languageChange()
{
  retranslateUi(this);
}

enum SetResponse createCountTagsByItem::set(const ParameterList &pParams)
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
    _warehouse->setEnabled(FALSE);

    _priority->setFocus();
  }

  return NoError;
}

void createCountTagsByItem::sCreate()
{
  int invcnt_id = 0;
  q.prepare( "SELECT createCountTag(itemsite_id, :comments, :priority,"
             "                      :freeze, :location_id) AS invcnt_id "
             "FROM itemsite "
             "WHERE ( (itemsite_item_id=:item_id)"
             " AND (itemsite_warehous_id=:warehous_id) );" );
  q.bindValue(":comments", _comments->toPlainText());
  q.bindValue(":priority", QVariant(_priority->isChecked()));
  q.bindValue(":freeze",   QVariant(_freeze->isChecked()));
  q.bindValue(":item_id", _item->id());
  q.bindValue(":warehous_id", _warehouse->id());
  if(_byLocation->isChecked())
    q.bindValue(":location_id", _location->id());
  q.exec();
  if (q.first())
  {
    invcnt_id = q.value("invcnt_id").toInt();
    if (invcnt_id == 0)
      QMessageBox::information(this, tr("No Count Tags Created"),
                               tr("No Count Tags were created. Likely causes "
                                  "are the Item Site has Control Method: None "
                                  "or the Item is not a countable Type."));

  }
  if (q.lastError().type() != QSqlError::NoError)
    QMessageBox::critical(this, tr("Error Creating Count Tags"),
                          q.lastError().text());

  if (_captive)
    done(invcnt_id);
  else
  {
    _item->setId(-1);
    _comments->clear();
    _item->setFocus();
  }
}


void createCountTagsByItem::sPopulateLocations()
{
  q.prepare("SELECT itemsite_loccntrl"
            "  FROM itemsite"
            " WHERE ((itemsite_item_id=:item_id)"
            "   AND  (itemsite_warehous_id=:warehous_id));");
  q.bindValue(":item_id", _item->id());
  q.bindValue(":warehous_id", _warehouse->id());
  q.exec();
  if(q.first() && q.value("itemsite_loccntrl").toBool())
  {
    q.prepare( "SELECT location_id,"
               "       formatLocationName(location_id) AS locationname"
               "  FROM location, itemsite"
               " WHERE ((validLocation(location_id, itemsite_id))"
               "   AND  (itemsite_warehous_id=:warehous_id)"
               "   AND  (itemsite_item_id=:item_id)) "
               " ORDER BY locationname;");
    q.bindValue(":item_id", _item->id());
    q.bindValue(":warehous_id", _warehouse->id());
    q.exec();
    _location->populate(q);  
    _byLocation->setEnabled(true);
  }
  else
  {
    _location->clear();
    _byLocation->setChecked(false);
    _location->setEnabled(false);
    _byLocation->setEnabled(false);
  }
}

