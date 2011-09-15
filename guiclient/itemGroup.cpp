/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "itemGroup.h"

#include <QMessageBox>
#include <QVariant>

#include "itemcluster.h"

itemGroup::itemGroup(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  connect(_new, SIGNAL(clicked()), this, SLOT(sNew()));
  connect(_delete, SIGNAL(clicked()), this, SLOT(sDelete()));
  connect(_save, SIGNAL(clicked()), this, SLOT(sSave()));
  connect(_close, SIGNAL(clicked()), this, SLOT(sClose()));
  connect(_name, SIGNAL(lostFocus()), this, SLOT(sCheck()));

  _itemgrpitem->addColumn(tr("Name"),        _itemColumn,  Qt::AlignLeft, true, "item_number" );
  _itemgrpitem->addColumn(tr("Description"), -1,           Qt::AlignLeft, true, "item_descrip" );
}

itemGroup::~itemGroup()
{
    // no need to delete child widgets, Qt does it all for us
}

void itemGroup::languageChange()
{
    retranslateUi(this);
}

enum SetResponse itemGroup::set(const ParameterList &pParams)
{
  XWidget::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("itemgrp_id", &valid);
  if (valid)
  {
    _itemgrpid = param.toInt();
    populate();
  }

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      _mode = cNew;

      q.exec("SELECT NEXTVAL('itemgrp_itemgrp_id_seq') AS itemgrp_id;");
      if (q.first())
        _itemgrpid = q.value("itemgrp_id").toInt();
//  ToDo

      connect(_itemgrpitem, SIGNAL(valid(bool)), _delete, SLOT(setEnabled(bool)));

      _name->setFocus();
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;

      connect(_itemgrpitem, SIGNAL(valid(bool)), _delete, SLOT(setEnabled(bool)));

      _name->setFocus();
    }
    else if (param.toString() == "view")
    {
      _mode = cView;
      _name->setEnabled(FALSE);
      _descrip->setEnabled(FALSE);
      _new->setEnabled(FALSE);
      _close->setText(tr("&Close"));
      _save->hide();
      _close->setFocus();
    }
  }

  return NoError;
}

void itemGroup::sCheck()
{
  _name->setText(_name->text().trimmed());
  if ((_mode == cNew) && (_name->text().length()))
  {
    q.prepare( "SELECT itemgrp_id "
               "FROM itemgrp "
               "WHERE (UPPER(itemgrp_name)=UPPER(:itemgrp_name));" );
    q.bindValue(":itemgrp_name", _name->text());
    q.exec();
    if (q.first())
    {
      _itemgrpid = q.value("itemgrp_id").toInt();
      _mode = cEdit;
      populate();

      _name->setEnabled(FALSE);
    }
  }
}

void itemGroup::sClose()
{
  if (_mode == cNew)
  {
    q.prepare( "DELETE FROM itemgrpitem "
               "WHERE (itemgrpitem_itemgrp_id=:itemgrp_id);"

                "DELETE FROM itemgrp "
                "WHERE (itemgrp_id=:itemgrp_id);" );
    q.bindValue(":itemgrp_id", _itemgrpid);
    q.exec();
  }

  close();
}

void itemGroup::sSave()
{
  if (_name->text().trimmed().length() == 0)
  {
    QMessageBox::warning( this, tr("Cannot Save Item Group"),
                          tr("You must enter a Name for this Item Group before you may save it.") );
    _name->setFocus();
    return;
  }

  if (_mode == cNew)
    q.prepare( "INSERT INTO itemgrp "
               "(itemgrp_id, itemgrp_name, itemgrp_descrip) "
               "VALUES "
               "(:itemgrp_id, :itemgrp_name, :itemgrp_descrip);" );
  else
    q.prepare( "UPDATE itemgrp "
               "SET itemgrp_name=:itemgrp_name, itemgrp_descrip=:itemgrp_descrip "
               "WHERE (itemgrp_id=:itemgrp_id);" );

  q.bindValue(":itemgrp_id", _itemgrpid);
  q.bindValue(":itemgrp_name", _name->text());
  q.bindValue(":itemgrp_descrip", _descrip->text());
  q.exec();

  omfgThis->sItemGroupsUpdated(_itemgrpid, TRUE);

  close();
}

void itemGroup::sDelete()
{
  q.prepare( "DELETE FROM itemgrpitem "
             "WHERE (itemgrpitem_id=:itemgrpitem_id);" );
  q.bindValue(":itemgrpitem_id", _itemgrpitem->id());
  q.exec();

  sFillList();
}

void itemGroup::sNew()
{
  ParameterList params;
  itemList* newdlg = new itemList(this);
  newdlg->set(params);

  int itemid = newdlg->exec();
  if (itemid != -1)
  {
    q.prepare( "SELECT itemgrpitem_id "
               "FROM itemgrpitem "
               "WHERE ( (itemgrpitem_itemgrp_id=:itemgrp_id)"
               " AND (itemgrpitem_item_id=:item_id) );" );
    q.bindValue(":itemgrp_id", _itemgrpid);
    q.bindValue(":item_id", itemid);
    q.exec();
    if (q.first())
    {
      QMessageBox::warning( this, tr("Cannot Add Item to Item Group"),
                            tr("The selected Item is already a member of this Item Group") );
      return;
    }

    q.prepare( "INSERT INTO itemgrpitem "
               "(itemgrpitem_itemgrp_id, itemgrpitem_item_id) "
               "VALUES "
               "(:itemgrpitem_itemgrp_id, :itemgrpitem_item_id);" );
    q.bindValue(":itemgrpitem_itemgrp_id", _itemgrpid);
    q.bindValue(":itemgrpitem_item_id", itemid);
    q.exec();

    sFillList();
  }
}

void itemGroup::sFillList()
{
  q.prepare( "SELECT itemgrpitem_id, item_number, (item_descrip1 || ' ' || item_descrip2) AS item_descrip "
             "FROM itemgrpitem, item "
             "WHERE ( (itemgrpitem_item_id=item_id) "
             " AND (itemgrpitem_itemgrp_id=:itemgrp_id) ) "
             "ORDER BY item_number;" );
  q.bindValue(":itemgrp_id", _itemgrpid);
  q.exec();
  _itemgrpitem->populate(q);
}

void itemGroup::populate()
{
  q.prepare( "SELECT itemgrp_name, itemgrp_descrip "
             "FROM itemgrp "
             "WHERE (itemgrp_id=:itemgrp_id);" );
  q.bindValue(":itemgrp_id", _itemgrpid);
  q.exec();
  if (q.first())
  {
    _name->setText(q.value("itemgrp_name").toString());
    _descrip->setText(q.value("itemgrp_descrip").toString());

    sFillList();
  }
}
