/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "itemSubstitute.h"

#include <QVariant>
#include <QMessageBox>

#define cItemSub    0x01
#define cBOMItemSub 0x02

itemSubstitute::itemSubstitute(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
  : XDialog(parent, name, modal, fl)
{
  setupUi(this);


  // signals and slots connections
  connect(_substitute, SIGNAL(valid(bool)), _save, SLOT(setEnabled(bool)));
  connect(_close, SIGNAL(clicked()), this, SLOT(reject()));
  connect(_save, SIGNAL(clicked()), this, SLOT(sSave()));

  _uomRatio->setValidator(omfgThis->ratioVal());
  _uomRatio->setText("1.0");
}

itemSubstitute::~itemSubstitute()
{
  // no need to delete child widgets, Qt does it all for us
}

void itemSubstitute::languageChange()
{
  retranslateUi(this);
}

enum SetResponse itemSubstitute::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("bomitem_id", &valid);
  if (valid)
  {
    _type = cBOMItemSub;
    _bomitemid = param.toInt();
    q.prepare( "SELECT bomitem_item_id "
               "FROM bomitem "
               "WHERE (bomitem_id=:bomitem_id);" );
    q.bindValue(":bomitem_id", _bomitemid);
    q.exec();
    if (q.first())
    {
      _item->setId(q.value("bomitem_item_id").toInt());
      _item->setReadOnly(TRUE);
    }
  }

  param = pParams.value("bomitem_item_id", &valid);
  if (valid)
  {
    _type = cBOMItemSub;
    _item->setId(param.toInt());
    _item->setReadOnly(TRUE);
  }

  param = pParams.value("item_id", &valid);
  if (valid)
  {
    _type = cItemSub;
    _item->setId(param.toInt());
    _item->setReadOnly(TRUE);
  }

  param = pParams.value("itemsub_id", &valid);
  if (valid)
  {
    _type = cItemSub;
    _itemsubid = param.toInt();
    populate();
  }

  param = pParams.value("bomitemsub_id", &valid);
  if (valid)
  {
    _type = cBOMItemSub;
    _itemsubid = param.toInt();

    _item->setEnabled(FALSE);

    populate();
  }

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      _mode = cNew;
      _item->setFocus();
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;
      _save->setFocus();
    }
    else if (param.toString() == "view")
    {
      _mode = cView;

      _item->setReadOnly(TRUE);
      _substitute->setReadOnly(TRUE);
      _uomRatio->setEnabled(FALSE);
      _ranking->setEnabled(FALSE);
      _close->setText(tr("&Close"));
      _save->hide();

      _close->setFocus();
    }
  }

  return NoError;
}

void itemSubstitute::sSave()
{
  if (_type == cItemSub)
  {
    if (_item->id() == _substitute->id())
    {
      QMessageBox::critical( this, tr("Cannot Create Substitute"),
                             tr("You cannot define an Item to be a substitute for itself.") );
      _substitute->setFocus();
      return;
    }

    q.prepare( "SELECT itemsub_id "
               "  FROM itemsub "
               " WHERE((itemsub_parent_item_id=:parentItem_id)"
               "   AND (itemsub_sub_item_id=:item_id)"
               "   AND (itemsub_id != :itemsub_id) );" );
    q.bindValue(":parentItem_id", _item->id());
    q.bindValue(":item_id", _substitute->id());
    q.bindValue(":itemsub_id", _itemsubid);
    q.exec();
    if (q.first())
    {
      QMessageBox::critical( this, tr("Cannot Create Duplicate Substitute"),
                             tr( "This substitution has already been defined for the selected Item.\n"
                                 "You may edit the existing Substitution but you may not create a duplicate." ) );
      _substitute->setFocus();
      return;
    }

    if (_mode == cNew)
    {
      q.exec("SELECT NEXTVAL('itemsub_itemsub_id_seq') AS _itemsub_id");
      if (q.first())
        _itemsubid = q.value("_itemsub_id").toInt();
//  ToDo

      q.prepare( "INSERT INTO itemsub "
                 "( itemsub_id, itemsub_parent_item_id, itemsub_sub_item_id,"
                 "  itemsub_uomratio, itemsub_rank ) "
                 "VALUES "
                 "( :itemsub_id, :itemsub_parent_item_id, :itemsub_sub_item_id,"
                 "  :itemsub_uomratio, :itemsub_rank );" );
    }
    else if (_mode == cEdit)
      q.prepare( "UPDATE itemsub "
                 "   SET itemsub_uomratio=:itemsub_uomratio,"
                 "       itemsub_rank=:itemsub_rank,"
                 "       itemsub_sub_item_id=:itemsub_sub_item_id"
                 " WHERE(itemsub_id=:itemsub_id);" );

    q.bindValue(":itemsub_id", _itemsubid);
    q.bindValue(":itemsub_parent_item_id", _item->id());
    q.bindValue(":itemsub_sub_item_id", _substitute->id());
    q.bindValue(":itemsub_uomratio", _uomRatio->toDouble());
    q.bindValue(":itemsub_rank", _ranking->value());
    q.exec();
  }
  else if (_type == cBOMItemSub)
  {
    if (_mode == cNew)
    {
      q.prepare( "SELECT bomitemsub_id "
                 "FROM bomitemsub "
                 "WHERE ( (bomitemsub_bomitem_id=:bomitem_id)"
                 " AND (bomitemsub_item_id=:item_id) );" );
      q.bindValue(":bomitem_id", _bomitemid);
      q.bindValue(":item_id", _substitute->id());
      q.exec();
      if (q.first())
      {
        QMessageBox::critical( this, tr("Cannot Create Duplicate Substitute"),
                               tr( "This substitution has already been defined for the selected BOM Item.\n"
                                   "You may edit the existing Substitution but you may not create a duplicate." ) );
        _substitute->setFocus();
        return;
      }
    }

    if (_mode == cNew)
    {
      q.exec("SELECT NEXTVAL('bomitemsub_bomitemsub_id_seq') AS bomitemsub_id");
      if (q.first())
        _itemsubid = q.value("bomitemsub_id").toInt();
//  ToDo

      q.prepare( "INSERT INTO bomitemsub "
                 "( bomitemsub_id, bomitemsub_bomitem_id, bomitemsub_item_id,"
                 "  bomitemsub_uomratio, bomitemsub_rank ) "
                 "VALUES "
                 "( :bomitemsub_id, :bomitemsub_bomitem_id, :bomitemsub_item_id,"
                 "  :bomitemsub_uomratio, :bomitemsub_rank );" );
    }
    else if (_mode == cEdit)
      q.prepare( "UPDATE bomitemsub "
                 "SET bomitemsub_uomratio=:bomitemsub_uomratio, bomitemsub_rank=:bomitemsub_rank "
                 "WHERE (bomitemsub_id=:bomitemsub_id);" );

    q.bindValue(":bomitemsub_id", _itemsubid);
    q.bindValue(":bomitemsub_bomitem_id", _bomitemid);
    q.bindValue(":bomitemsub_item_id", _substitute->id());
    q.bindValue(":bomitemsub_uomratio", _uomRatio->toDouble());
    q.bindValue(":bomitemsub_rank", _ranking->value());
    q.exec();
  }

  done(_itemsubid);
}

void itemSubstitute::populate()
{
  if (_type == cItemSub)
  {
    q.prepare( "SELECT itemsub_parent_item_id, itemsub_sub_item_id,"
               "       itemsub_uomratio, itemsub_rank "
               "FROM itemsub "
               "WHERE (itemsub_id=:itemsub_id);" );
    q.bindValue(":itemsub_id", _itemsubid);
    q.exec();
    if (q.first())
    {
      _item->setId(q.value("itemsub_parent_item_id").toInt());
      _substitute->setId(q.value("itemsub_sub_item_id").toInt());
      _ranking->setValue(q.value("itemsub_rank").toInt());
      _uomRatio->setDouble(q.value("itemsub_uomratio").toDouble());
    }
  }
  else if (_type == cBOMItemSub)
  {
    q.prepare( "SELECT bomitemsub_bomitem_id, item_id,  bomitemsub_item_id,"
               "       bomitemsub_uomratio, bomitemsub_rank "
               "FROM bomitemsub, bomitem, item "
               "WHERE ( (bomitemsub_bomitem_id=bomitem_id)"
               " AND (bomitem_item_id=item_id)"
               " AND (bomitemsub_id=:bomitemsub_id) );" );
    q.bindValue(":bomitemsub_id", _itemsubid);
    q.exec();
    if (q.first())
    {
      _item->setId(q.value("item_id").toInt());
      _substitute->setId(q.value("bomitemsub_item_id").toInt());
      _ranking->setValue(q.value("bomitemsub_rank").toInt());
      _uomRatio->setDouble(q.value("bomitemsub_uomratio").toDouble());
    }
  }
}

