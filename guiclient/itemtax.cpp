/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "itemtax.h"

#include <QMessageBox>
#include <QVariant>

/*
 *  Constructs a itemtax as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
itemtax::itemtax(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  // signals and slots connections
  connect(_save, SIGNAL(clicked()), this, SLOT(sSave()));

  _itemtaxid = -1;
  _itemid = -1;
}

/*
 *  Destroys the object and frees any allocated resources
 */
itemtax::~itemtax()
{
  // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void itemtax::languageChange()
{
  retranslateUi(this);
}

enum SetResponse itemtax::set(const ParameterList & pParams)
{
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("item_id", &valid);
  if (valid)
    _itemid = param.toInt();

  param = pParams.value("itemtax_id", &valid);
  if (valid)
  {
    _itemtaxid = param.toInt();
    q.prepare("SELECT itemtax_item_id,"
              "       COALESCE(itemtax_taxzone_id,-1) AS taxzone_id,"
              "       itemtax_taxtype_id"
              "  FROM itemtax"
              " WHERE (itemtax_id=:itemtax_id);");
    q.bindValue(":itemtax_id", _itemtaxid);
    q.exec();
    if(q.first())
    {
      _itemid = q.value("itemtax_item_id").toInt();
      _taxzone->setId(q.value("taxzone_id").toInt());
      _taxtype->setId(q.value("itemtax_taxtype_id").toInt());
    }
    // TODO: catch any possible errors
  }

  param = pParams.value("mode", &valid);
  if(valid)
  {
    if(param.toString() == "new")
      _mode = cNew;
    else if(param.toString() == "edit")
      _mode = cEdit;
    else if(param.toString() == "view")
    {
      _mode = cView;
      _save->hide();
      _taxzone->setEnabled(false);
      _taxtype->setEnabled(false);
    }
  }

  return NoError;
}

void itemtax::sSave()
{
  q.prepare("SELECT itemtax_id"
            "  FROM itemtax"
            " WHERE ((itemtax_taxzone_id=:taxzone_id)"
            "   AND  (itemtax_item_id=:item_id)"
            "   AND  (itemtax_id != :itemtax_id))");
  q.bindValue(":item_id", _itemid);
  q.bindValue(":itemtax_id", _itemtaxid);
  if(_taxzone->isValid())
    q.bindValue(":taxzone_id", _taxzone->id());
  q.exec();
  if(q.first())
  {
    QMessageBox::warning(this, tr("Tax Zone Already Exists"),
                      tr("The Tax Zone you have choosen already exists for this item."));
    return;
  }

  if(cNew == _mode)
    q.prepare("INSERT INTO itemtax"
              "      (itemtax_item_id, itemtax_taxzone_id, itemtax_taxtype_id) "
              "VALUES(:item_id, :taxzone_id, :taxtype_id)");
  else if(cEdit == _mode)
    q.prepare("UPDATE itemtax"
              "   SET itemtax_taxzone_id=:taxzone_id,"
              "       itemtax_taxtype_id=:taxtype_id"
              " WHERE (itemtax_id=:itemtax_id);");

  q.bindValue(":item_id", _itemid);
  q.bindValue(":itemtax_id", _itemtaxid);
  if(_taxzone->isValid())
    q.bindValue(":taxzone_id", _taxzone->id());
  q.bindValue(":taxtype_id", _taxtype->id());
  q.exec();

  accept();
}

