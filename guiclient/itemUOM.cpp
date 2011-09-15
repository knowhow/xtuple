/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "itemUOM.h"

#include <QVariant>
#include <QMessageBox>
#include <QSqlError>
#include <parameter.h>
#include "storedProcErrorLookup.h"

/*
 *  Constructs a itemUOM as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
itemUOM::itemUOM(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  _itemid = _itemuomconvid = -1;
  _uomidFrom = -1;
  _ignoreSignals = false;

  // signals and slots connections
  connect(_save, SIGNAL(clicked()), this, SLOT(sSave()));
  connect(_close, SIGNAL(clicked()), this, SLOT(reject()));
  connect(_uomFrom, SIGNAL(currentIndexChanged(int)), this, SLOT(sCheck()));
  connect(_uomTo, SIGNAL(currentIndexChanged(int)), this, SLOT(sCheck()));
  connect(_add, SIGNAL(clicked()), this, SLOT(sAdd()));
  connect(_remove, SIGNAL(clicked()), this, SLOT(sRemove()));

  _fromValue->setValidator(omfgThis->ratioVal());
  _toValue->setValidator(omfgThis->ratioVal());

  _fromValue->setDouble(1);
  _toValue->setDouble(1);

  _uomFrom->setType(XComboBox::UOMs);
  _uomTo->setType(XComboBox::UOMs);
}

/*
 *  Destroys the object and frees any allocated resources
 */
itemUOM::~itemUOM()
{
  // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void itemUOM::languageChange()
{
  retranslateUi(this);
}

enum SetResponse itemUOM::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("item_id", &valid);
  if (valid)
  {
    _itemid = param.toInt();
    q.prepare("SELECT item_inv_uom_id"
              "  FROM item"
              " WHERE(item_id=:item_id);");
    q.bindValue(":item_id", _itemid);
    q.exec();
    if(q.first())
    {
      _uomidFrom = q.value("item_inv_uom_id").toInt();
      _ignoreSignals = true;
      _uomFrom->setId(_uomidFrom);
      _uomTo->setId(_uomidFrom);
      _ignoreSignals = false;
    }
  }

  param = pParams.value("inventoryUOM", &valid);
  if (valid)
  {
      _uomidFrom = param.toInt();
      _ignoreSignals = true;
      _uomFrom->setId(_uomidFrom);
      _uomTo->setId(_uomidFrom);
      _ignoreSignals = false;
  }

  param = pParams.value("itemuomconv_id", &valid);
  if (valid)
  {
    _itemuomconvid = param.toInt();
    _ignoreSignals = true;
    populate();
    _ignoreSignals = false;
  }

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
      _mode = cNew;
    else if (param.toString() == "edit")
    {
      _mode = cEdit;

      _save->setFocus();
      _uomTo->setEnabled(false);
      _uomFrom->setEnabled(false);
    }
    else if (param.toString() == "view")
    {
      _mode = cView;

      _toValue->setEnabled(false);
      _fromValue->setEnabled(false);
      _uomTo->setEnabled(false);
      _uomFrom->setEnabled(false);
      _add->setEnabled(false);
      _remove->setEnabled(false);
      _close->setText(tr("&Close"));
      _save->hide();

      _close->setFocus();
    }
  }

  return NoError;
}

void itemUOM::sSave()
{
  if(_fromValue->toDouble() <= 0.0)
  {
    QMessageBox::warning(this, tr("Invalid Ratio"),
      tr("<p>You must specify a ratio value greater than 0."));
    _fromValue->setFocus();
    return;
  }

  if(_toValue->toDouble() <= 0.0)
  {
    QMessageBox::warning(this, tr("Invalid Ratio"),
      tr("<p>You must specify a ratio value greater than 0."));
    _toValue->setFocus();
    return;
  }

  if(_selected->count() == 0)
  {
    QMessageBox::warning(this, tr("No Types Selected"),
      tr("<p>You must select at least one UOM Type for this conversion."));
    return;
  }

  q.prepare("UPDATE itemuomconv"
            "   SET itemuomconv_to_value=:tovalue,"
            "       itemuomconv_from_value=:fromvalue,"
            "       itemuomconv_fractional=:fractional "
            " WHERE(itemuomconv_id=:itemuomconv_id);");
  q.bindValue(":itemuomconv_id", _itemuomconvid);
  q.bindValue(":tovalue", _toValue->toDouble());
  q.bindValue(":fromvalue", _fromValue->toDouble());
  q.bindValue(":fractional", QVariant(_fractional->isChecked()));
  if(q.exec())
    accept();
}

void itemUOM::populate()
{
  q.prepare("SELECT itemuomconv_item_id, item_inv_uom_id,"
            "       itemuomconv_from_uom_id, itemuomconv_to_uom_id,"
            "       itemuomconv_from_value, itemuomconv_to_value, itemuomconv_fractional,"
            "       (uomconv_id IS NOT NULL) AS global"
            "  FROM itemuomconv"
            "  JOIN item ON (itemuomconv_item_id=item_id)"
            "  LEFT OUTER JOIN uomconv"
            "    ON ((uomconv_from_uom_id=itemuomconv_from_uom_id AND uomconv_to_uom_id=itemuomconv_to_uom_id)"
            "     OR (uomconv_to_uom_id=itemuomconv_from_uom_id AND uomconv_from_uom_id=itemuomconv_to_uom_id))"
            " WHERE((itemuomconv_id=:itemuomconv_id));");
  q.bindValue(":itemuomconv_id", _itemuomconvid);
  q.exec();
  if(q.first())
  {
    _itemid = q.value("itemuomconv_item_id").toInt();
    _uomidFrom = q.value("item_inv_uom_id").toInt();
    _uomFrom->setId(q.value("itemuomconv_from_uom_id").toInt());
    _uomTo->setId(q.value("itemuomconv_to_uom_id").toInt());
    _fromValue->setDouble(q.value("itemuomconv_from_value").toDouble());
    _toValue->setDouble(q.value("itemuomconv_to_value").toDouble());
    _fractional->setChecked(q.value("itemuomconv_fractional").toBool());
    _toValue->setEnabled(!q.value("global").toBool());
    _fromValue->setEnabled(!q.value("global").toBool());

    sFillList();
  }
}

void itemUOM::sFillList()
{
  _available->clear();
  _selected->clear();
  q.prepare("SELECT uomtype_id, uomtype_name,"
            "       uomtype_descrip, (itemuom_id IS NOT NULL) AS selected"
            "  FROM uomtype"
            "  LEFT OUTER JOIN itemuom ON ((itemuom_uomtype_id=uomtype_id)"
            "                          AND (itemuom_itemuomconv_id=:itemuomconv_id))"
            " WHERE (uomtype_id NOT IN"
            "        (SELECT uomtype_id"
            "           FROM itemuomconv"
            "           JOIN itemuom ON (itemuom_itemuomconv_id=itemuomconv_id)"
            "           JOIN uomtype ON (itemuom_uomtype_id=uomtype_id)"
            "          WHERE((itemuomconv_item_id=:item_id)"
            "            AND (itemuomconv_id != :itemuomconv_id)"
            "            AND (uomtype_multiple=false))))"
            " ORDER BY uomtype_name;");
  q.bindValue(":item_id", _itemid);
  q.bindValue(":itemuomconv_id", _itemuomconvid);
  q.exec();
  while(q.next())
  {
    QListWidgetItem *item = new QListWidgetItem(q.value("uomtype_name").toString());
    item->setToolTip(q.value("uomtype_descrip").toString());
    item->setData(Qt::UserRole, q.value("uomtype_id").toInt());

    if(q.value("selected").toBool())
      _selected->addItem(item);
    else
      _available->addItem(item);
  }
  _add->setEnabled(true);
  _remove->setEnabled(true);
}

void itemUOM::sAdd()
{
  QList<QListWidgetItem*> items = _available->selectedItems();
  QListWidgetItem * item;
  for(int i = 0; i < items.size(); i++)
  {
    item = items.at(i);
    q.prepare("INSERT INTO itemuom"
              "      (itemuom_itemuomconv_id, itemuom_uomtype_id) "
              "VALUES(:itemuomconv_id, :uomtype_id);");
    q.bindValue(":itemuomconv_id", _itemuomconvid);
    q.bindValue(":uomtype_id", item->data(Qt::UserRole));
    q.exec();
  }
  sFillList();
}

void itemUOM::sRemove()
{
  QList<QListWidgetItem*> items = _selected->selectedItems();
  QListWidgetItem * item;
  for(int i = 0; i < items.size(); i++)
  {
    item = items.at(i);
    q.prepare("SELECT deleteItemuom(itemuom_id) AS result"
              "  FROM itemuom"
              " WHERE((itemuom_itemuomconv_id=:itemuomconv_id)"
              "   AND (itemuom_uomtype_id=:uomtype_id));");
    q.bindValue(":itemuomconv_id", _itemuomconvid);
    q.bindValue(":uomtype_id", item->data(Qt::UserRole));
    q.exec();
    // TODO: add in some addtional error checking
  }
  sFillList();
}

void itemUOM::sCheck()
{
  if(cNew != _mode || _ignoreSignals)
    return;

  _ignoreSignals = true;

  _uomFrom->setEnabled(false);
  _uomTo->setEnabled(false);

  q.prepare("SELECT itemuomconv_id"
            "  FROM itemuomconv"
            " WHERE((itemuomconv_item_id=:item_id)"
            "   AND (((itemuomconv_from_uom_id=:from_uom_id) AND (itemuomconv_to_uom_id=:to_uom_id))"
            "     OR ((itemuomconv_from_uom_id=:to_uom_id) AND (itemuomconv_to_uom_id=:from_uom_id))));");
  q.bindValue(":item_id", _itemid);
  q.bindValue(":from_uom_id", _uomFrom->id());
  q.bindValue(":to_uom_id", _uomTo->id());
  q.exec();
  if(q.first())
  {
    _itemuomconvid = q.value("itemuomconv_id").toInt();
    _mode = cEdit;
    populate();
    _ignoreSignals = false;
    return;
  }

  q.prepare("SELECT uomconv_from_uom_id, uomconv_from_value,"
            "       uomconv_to_uom_id, uomconv_to_value,"
            "       uomconv_fractional"
            "  FROM uomconv"
            " WHERE(((uomconv_from_uom_id=:from_uom_id) AND (uomconv_to_uom_id=:to_uom_id))"
            "    OR ((uomconv_from_uom_id=:to_uom_id) AND (uomconv_to_uom_id=:from_uom_id)));");
  q.bindValue(":from_uom_id", _uomFrom->id());
  q.bindValue(":to_uom_id", _uomTo->id());
  q.exec();
  if(q.first())
  {
    _uomFrom->setId(q.value("uomconv_from_uom_id").toInt());
    _uomTo->setId(q.value("uomconv_to_uom_id").toInt());
    _fromValue->setDouble(q.value("uomconv_from_value").toDouble());
    _toValue->setDouble(q.value("uomconv_to_value").toDouble());
    _fractional->setChecked(q.value("uomconv_fractional").toBool());
    _fromValue->setEnabled(false);
    _toValue->setEnabled(false);
  }
  else
  {
    _fromValue->setEnabled(true);
    _toValue->setEnabled(true);
  }

  q.exec("SELECT nextval('itemuomconv_itemuomconv_id_seq') AS result;");
  q.first();
  _itemuomconvid = q.value("result").toInt();

  q.prepare("INSERT INTO itemuomconv"
            "      (itemuomconv_id, itemuomconv_item_id, itemuomconv_from_uom_id, itemuomconv_to_uom_id,"
            "       itemuomconv_from_value, itemuomconv_to_value, itemuomconv_fractional) "
            "VALUES(:id, :item_id, :from_uom_id, :to_uom_id, :fromvalue, :tovalue, :fractional);");
  q.bindValue(":id", _itemuomconvid);
  q.bindValue(":item_id", _itemid);
  q.bindValue(":from_uom_id", _uomFrom->id());
  q.bindValue(":to_uom_id", _uomTo->id());
  q.bindValue(":fromvalue", _fromValue->toDouble());
  q.bindValue(":tovalue", _toValue->toDouble());
  q.bindValue(":fractional", QVariant(_fractional->isChecked()));
  q.exec();

  sFillList();

  _ignoreSignals = false;
}

void itemUOM::reject()
{
  if(cNew == _mode)
  {
    q.prepare("DELETE FROM itemuom WHERE itemuom_itemuomconv_id=:itemuomconv_id;"
              "DELETE FROM itemuomconv WHERE itemuomconv_id=:itemuomconv_id;");
    q.bindValue(":itemuomconv_id", _itemuomconvid);
    q.exec();
  }

  XDialog::reject();
}
