/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "substituteList.h"

#include <QMessageBox>
#include <QVariant>

substituteList::substituteList(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
  : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  QButtonGroup * showByButtonGroup = new QButtonGroup(this);
  showByButtonGroup->addButton(_byLeadTime);
  showByButtonGroup->addButton(_byDays);
  showByButtonGroup->addButton(_byDate);

  // signals and slots connections
  connect(_byDays, SIGNAL(toggled(bool)), _days, SLOT(setEnabled(bool)));
  connect(_byDate, SIGNAL(toggled(bool)), _date, SLOT(setEnabled(bool)));
  connect(_subs, SIGNAL(valid(bool)), _select, SLOT(setEnabled(bool)));
  connect(_subs, SIGNAL(itemSelected(int)), _select, SLOT(animateClick()));
  connect(showByButtonGroup, SIGNAL(buttonClicked(int)), this, SLOT(sFillList()));
  connect(_close, SIGNAL(clicked()), this, SLOT(reject()));
  connect(_select, SIGNAL(clicked()), this, SLOT(sSelect()));
  connect(_item, SIGNAL(newId(int)), _warehouse, SLOT(findItemsites(int)));
  connect(_item, SIGNAL(warehouseIdChanged(int)), _warehouse, SLOT(setId(int)));

  _subs->addColumn(tr("Item Number"),  _itemColumn, Qt::AlignLeft,   true,  "item_number"  );
  _subs->addColumn(tr("Description"),  -1,          Qt::AlignLeft,   true,  "itemdescrip"  );
  _subs->addColumn(tr("QOH"),          _qtyColumn,  Qt::AlignRight,  true,  "qoh" );
  _subs->addColumn(tr("Norm. QOH"),    _qtyColumn,  Qt::AlignRight,  true,  "normqoh" );
  _subs->addColumn(tr("Availability"), _qtyColumn,  Qt::AlignRight,  true,  "available" );
  _subs->addColumn(tr("Norm. Avail."), _qtyColumn,  Qt::AlignRight,  true,  "normavailable" );

  //If not multi-warehouse hide whs control
  if (!_metrics->boolean("MultiWhs"))
  {
    _warehouseLit->hide();
    _warehouse->hide();
  }
}

substituteList::~substituteList()
{
  // no need to delete child widgets, Qt does it all for us
}

void substituteList::languageChange()
{
  retranslateUi(this);
}

enum SetResponse substituteList::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("womatl_id", &valid);
  if (valid)
  {
    q.prepare( "SELECT womatl_itemsite_id,"
               "       bomitem_id, COALESCE(bomitem_subtype, 'I') AS subtype,"
               "       COALESCE(wo_id, 0) AS wo_id "
               "FROM womatl LEFT OUTER JOIN bomitem ON (bomitem_id=womatl_bomitem_id) "
               "            LEFT OUTER JOIN wo ON ( (wo_ordtype='W') AND"
               "                                    (wo_ordid=womatl_wo_id) AND"
               "                                    (wo_itemsite_id=womatl_itemsite_id) ) "
               "WHERE (womatl_id=:womatl_id);" );
    q.bindValue(":womatl_id", param.toInt());
    q.exec();
    if (q.first())
    {
      if (q.value("wo_id").toInt() > 0)
      {
        QMessageBox::warning(this, tr("Child Work Order"),
            tr("A child Work Order exists for this Material \n"
               "Requirement.  You should delete this \n"
               "child Work Order before substituting.") );
      }
      _item->setItemsiteid(q.value("womatl_itemsite_id").toInt());
      _item->setReadOnly(TRUE);
      _warehouse->setEnabled(FALSE);
      
      _bomitemid = q.value("bomitem_id").toInt();
      _itemsiteid = q.value("womatl_itemsite_id").toInt();
      _source = q.value("subtype").toString();

      sFillList();
    }
  }

  _byLeadTime->setChecked(pParams.inList("byLeadTime"));

  param = pParams.value("byDays", &valid);
  if (valid)
  {
   _byDays->setChecked(TRUE);
   _days->setValue(param.toInt());
  }

  param = pParams.value("byDate", &valid);
  if (valid)
  {
   _byDate->setChecked(TRUE);
   _date->setDate(param.toDate());
  }

  if (pParams.inList("run"))
  {
    sFillList();
    return NoError_Run;
  }

  return NoError;
}

void substituteList::sSelect()
{
  if (_sub.findFirst("item_id", _subs->id()) != -1)
  {
    _uomratio = _sub.value("uomratio").toDouble();

    done(_subs->id());
  }
  else
    reject();
}

void substituteList::sFillList()
{
  _subs->clear();

  QString sql;

  if (_source == "I")
  {
    sql = "SELECT item_id, item_number, (item_descrip1 || ' ' || item_descrip2) AS itemdescrip,"
          "       (itemsite_qtyonhand) AS qoh,"
          "       (itemsite_qtyonhand * uomratio) AS normqoh,"
          "       (available) AS available,"
          "       (available * uomratio) AS normavailable,"
          "       uomratio,"
          "       'qty' AS qoh_xtnumericrole,"
          "       'qty' AS normqoh_xtnumericrole,"
          "       'qty' AS available_xtnumericrole,"
          "       'qty' AS normavailable_xtnumericrole "
          "FROM ( SELECT item_id, item_number, item_descrip1, item_descrip2,"
          "       itemsite_qtyonhand, itemsub_uomratio AS uomratio, itemsub_rank AS rank,";

    if (_byLeadTime->isChecked())
      sql += " qtyAvailable(itemsite_id, itemsite_leadtime) AS available ";
    else if (_byDays->isChecked())
      sql += " qtyAvailable(itemsite_id, :days) AS available ";
    else if (_byDate->isChecked())
      sql += " qtyAvailable(itemsite_id, (:date - CURRENT_DATE)) AS available ";

    sql += "FROM item, itemsite, itemsub "
           "WHERE ( (itemsite_item_id=item_id)"
           " AND (itemsite_item_id=itemsub_sub_item_id)"
           " AND (itemsite_warehous_id=:warehous_id)"
           " AND (itemsub_parent_item_id=:item_id) ) ) AS data "
           "ORDER BY rank";
  }
  else if (_source == "B")
  {
    sql = "SELECT item_id, item_number, (item_descrip1 || ' ' || item_descrip2) AS itemdescrip,"
          "       (itemsite_qtyonhand) AS qoh,"
          "       (itemsite_qtyonhand * uomratio) AS normqoh,"
          "       (available) AS available,"
          "       (available * uomratio) AS normavailable,"
          "       uomratio,"
          "       'qty' AS qoh_xtnumericrole,"
          "       'qty' AS normqoh_xtnumericrole,"
          "       'qty' AS available_xtnumericrole,"
          "       'qty' AS normavailable_xtnumericrole "
          "FROM ( SELECT item_id, item_number, item_descrip1, item_descrip2,"
          "       itemsite_qtyonhand, bomitemsub_uomratio AS uomratio, bomitemsub_rank AS rank,";

    if (_byLeadTime->isChecked())
      sql += " qtyAvailable(itemsite_id, itemsite_leadtime) AS available ";
    else if (_byDays->isChecked())
      sql += " qtyAvailable(itemsite_id, :days) AS available ";
    else if (_byDate->isChecked())
      sql += " qtyAvailable(itemsite_id, (:date - CURRENT_DATE)) AS available ";

    sql += "FROM item, itemsite, bomitemsub "
           "WHERE ( (itemsite_item_id=item_id)"
           " AND (itemsite_item_id=bomitemsub_item_id)"
           " AND (bomitemsub_bomitem_id=:bomitem_id)"
           " AND (itemsite_warehous_id=:warehous_id)"
           " AND (itemsite_id<>:itemsite_id) ) "
           "UNION "
           "SELECT item_id, item_number, item_descrip1, item_descrip2,"
           "       si.itemsite_qtyonhand, 1/bomitemsub_uomratio AS uomratio, 0 AS rank,";

    if (_byLeadTime->isChecked())
      sql += " qtyAvailable(si.itemsite_id, si.itemsite_leadtime) AS available ";
    else if (_byDays->isChecked())
      sql += " qtyAvailable(si.itemsite_id, :days) AS available ";
    else if (_byDate->isChecked())
      sql += " qtyAvailable(si.itemsite_id, (:date - CURRENT_DATE)) AS available ";

    sql += "FROM item, itemsite bi, bomitem, itemsite si, bomitemsub "
           "WHERE ( (bi.itemsite_item_id=item_id)"
           " AND (bi.itemsite_item_id=bomitem_item_id)"
           " AND (bomitem_id=:bomitem_id)"
           " AND (bi.itemsite_warehous_id=:warehous_id)"
           " AND (si.itemsite_id=:itemsite_id)"
           " AND (si.itemsite_item_id=bomitemsub_item_id)"
           " AND (bomitemsub_bomitem_id=:bomitem_id) ) "
           " ) AS data "
           "ORDER BY rank";
  }

  _sub.prepare(sql);
  _sub.bindValue(":warehous_id", _warehouse->id());
  _sub.bindValue(":item_id", _item->id());
  _sub.bindValue(":bomitem_id", _bomitemid);
  _sub.bindValue(":itemsite_id", _itemsiteid);

  if (_byDays->isChecked())
    _sub.bindValue(":days", _days->value());

  if (_byDate->isChecked())
    _sub.bindValue(":date", _date->date());

  _sub.exec();
  _subs->populate(_sub);
}
