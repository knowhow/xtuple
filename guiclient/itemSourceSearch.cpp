/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "itemSourceSearch.h"

#include <QVariant>
#include <QMessageBox>
#include <parameter.h>
#include <openreports.h>

/*
 *  Constructs a itemSourceSearch as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
itemSourceSearch::itemSourceSearch(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  _vendid = -1;
  _itemid = -1;

  // signals and slots connections
  connect(_searchNumber, SIGNAL(toggled(bool)), this, SLOT(sFillList()));
  connect(_searchDescrip1, SIGNAL(toggled(bool)), this, SLOT(sFillList()));
  connect(_searchDescrip2, SIGNAL(toggled(bool)), this, SLOT(sFillList()));
  connect(_searchVendNumber, SIGNAL(toggled(bool)), this, SLOT(sFillList()));
  connect(_searchVendDescrip, SIGNAL(toggled(bool)), this, SLOT(sFillList()));
  connect(_search, SIGNAL(textChanged(QString)), this, SLOT(sFillList()));

  _itemsrc->addColumn(tr("Item Number"),    _itemColumn, Qt::AlignLeft, true, "item_number" );
  _itemsrc->addColumn(tr("Description"),    -1,          Qt::AlignLeft, true, "item_descrip" );
  _itemsrc->addColumn(tr("Vendor"),         _itemColumn, Qt::AlignLeft, true, "vend_name" );
  _itemsrc->addColumn(tr("Vendor Item"),    _itemColumn, Qt::AlignLeft, true, "itemsrc_vend_item_number" );
  _itemsrc->addColumn(tr("Vendor Descrip"), _itemColumn, Qt::AlignLeft, true, "itemsrc_vend_item_descrip" );
  _itemsrc->addColumn(tr("Manufacturer"),   _itemColumn, Qt::AlignLeft, true, "itemsrc_manuf_name" );
  _itemsrc->addColumn(tr("Manuf. Item#"),   _itemColumn, Qt::AlignLeft, true, "itemsrc_manuf_item_number" );
  _itemsrc->addColumn(tr("Manuf. Descrip."),   _itemColumn, Qt::AlignLeft, false, "itemsrc_manuf_item_descrip" );

}

/*
 *  Destroys the object and frees any allocated resources
 */
itemSourceSearch::~itemSourceSearch()
{
  // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void itemSourceSearch::languageChange()
{
  retranslateUi(this);
}

enum SetResponse itemSourceSearch::set(const ParameterList & pParams)
{
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("vend_id", &valid);
  if(valid)
    _vendid = param.toInt();

  param = pParams.value("search", &valid);
  if(valid)
    _search->setText(param.toString());

  sFillList();

  return NoError;
}

void itemSourceSearch::sFillList()
{
  _itemsrc->clear();
  
  bool first = true;

  QString sql( "SELECT itemsrc_id AS id,"
               "       1 AS altid,"
               "       item_number,"
               "       (item_descrip1 || ' ' || item_descrip2) AS item_descrip,"
               "       vend_name,"
               "       itemsrc_vend_item_number,"
               "       itemsrc_vend_item_descrip, "
               "       itemsrc_manuf_name, "
               "       itemsrc_manuf_item_number, "
               "       itemsrc_manuf_item_descrip "
               "  FROM item, vend, itemsrc "
               " WHERE((itemsrc_item_id=item_id)"
               "   AND (itemsrc_vend_id=vend_id)"
               "   AND (vend_id=:vend_id)"
               "   AND (" );
  if(_searchNumber->isChecked())
  {
    if(!first)
      sql += " OR ";
    sql +=     "        (item_number ~* :searchString)";
    first = false;
  }
  if(_searchVendNumber->isChecked())
  {
    if(!first)
      sql += " OR ";
    sql +=     "        (itemsrc_vend_item_number ~* :searchString)";
    first = false;
  }
  if(_searchDescrip1->isChecked())
  {
    if(!first)
      sql += " OR ";
    sql +=     "        (item_descrip1 ~* :searchString)";
    first = false;
  }
  if(_searchDescrip2->isChecked())
  {
    if(!first)
      sql += " OR ";
    sql +=     "        (item_descrip2 ~* :searchString)";
    first = false;
  }
  if(_searchVendDescrip->isChecked())
  {
    if(!first)
      sql += " OR ";
    sql +=     "        (itemsrc_vend_item_descrip ~* :searchString)";
    first = false;
  }
  
  if(_searchManufName->isChecked())
  {
    if(!first)
      sql += " OR ";
    sql +=     "        (itemsrc_manuf_name ~* :searchString)";
    first = false;
  }
  
  if(_searchManufNumber->isChecked())
  {
    if(!first)
      sql += " OR ";
    sql +=     "        (itemsrc_manuf_item_number ~* :searchString)";
    first = false;
  }
  
  sql +=       "       )"
               "  )"
               " UNION "
               "SELECT DISTINCT poitem_expcat_id AS id,"
               "        2 AS altid,"
               "        :non AS item_number,"
               "       (expcat_code || ' ' || expcat_descrip) AS item_descrip,"
               "       vend_name,"
               "       poitem_vend_item_number,"
               "       poitem_vend_item_descrip, "
               "       poitem_manuf_name, "
               "       poitem_manuf_item_number, "
               "       poitem_manuf_item_descrip "
               "  FROM vend, pohead, poitem"
               "       LEFT OUTER JOIN expcat ON (poitem_expcat_id=expcat_id)"
               " WHERE((pohead_vend_id=vend_id)"
               "   AND (COALESCE(poitem_vend_item_number, '')!='')"
               "   AND (poitem_pohead_id=pohead_id)"
               "   AND (poitem_itemsite_id IS NULL) "
               "   AND (vend_id=:vend_id)"
               "   AND ( ";

  first = true;
  if(_searchVendNumber->isChecked())
  {
    if(!first)
      sql += " OR ";
    sql +=     "        (poitem_vend_item_number ~* :searchString)";
    first = false;
  }
  if(_searchDescrip1->isChecked())
  {
    if(!first)
      sql += " OR ";
    sql +=     "    (expcat_code ~* :searchString)"
               "    OR  (expcat_descrip ~* :searchString)";
    first = false;
  }
  if(_searchVendDescrip->isChecked())
  {
    if(!first)
      sql += " OR ";
    sql +=     "        (poitem_vend_item_descrip ~* :searchString)";
    first = false;
  }
  
  if(_searchManufName->isChecked())
  {
    if(!first)
      sql += " OR ";
    sql +=     "        (poitem_manuf_name ~* :searchString)";
    first = false;
  }
  
  if(_searchManufNumber->isChecked())
  {
    if(!first)
      sql += " OR ";
    sql +=     "        (poitem_manuf_item_number ~* :searchString)";
    first = false;
  }


  sql +=       "    ) )"
               " ORDER BY item_number, vend_name;";
  q.prepare(sql);
  q.bindValue(":searchString", _search->text());
  q.bindValue(":vend_id", _vendid);
  q.bindValue(":item_id", _itemid);
  q.bindValue(":non", tr("Non-Inventory"));
  q.exec();
  _itemsrc->populate(q, TRUE);
}

int itemSourceSearch::itemsrcId()
{
  XTreeWidgetItem * item = (XTreeWidgetItem*)_itemsrc->currentItem();
  if(item)
  {
    if(item->altId() == 1)
      return item->id();
  }
  return -1;
}

int itemSourceSearch::expcatId()
{
  XTreeWidgetItem * item = (XTreeWidgetItem*)_itemsrc->currentItem();
  if(item)
  {
    if(item->altId() == 2)
      return item->id();
  }
  return -1;
}

QString itemSourceSearch::vendItemNumber()
{
  XTreeWidgetItem * item = (XTreeWidgetItem*)_itemsrc->currentItem();
  if(item)
    return item->text(3);
  return QString();
}

QString itemSourceSearch::vendItemDescrip()
{
  XTreeWidgetItem * item = (XTreeWidgetItem*)_itemsrc->currentItem();
  if(item)
    return item->text(4);
  return QString();
}

QString itemSourceSearch::manufName()
{
  XTreeWidgetItem * item = (XTreeWidgetItem*)_itemsrc->currentItem();
  if(item)
    return item->text(5);
  return QString();
}

QString itemSourceSearch::manufItemNumber()
{
  XTreeWidgetItem * item = (XTreeWidgetItem*)_itemsrc->currentItem();
  if(item)
    return item->text(6);
  return QString();
}

QString itemSourceSearch::manufItemDescrip()
{
  XTreeWidgetItem * item = (XTreeWidgetItem*)_itemsrc->currentItem();
  if(item)
    return item->text(7);
  return QString();
}

