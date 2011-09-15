/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "itemListPrice.h"

#include <QVariant>
#include <QMessageBox>
#include <QValidator>

itemListPrice::itemListPrice(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_close, SIGNAL(clicked()), this, SLOT(reject()));
  connect(_save, SIGNAL(clicked()), this, SLOT(sSave()));
  connect(_item, SIGNAL(valid(bool)), _save, SLOT(setEnabled(bool)));
  connect(_item, SIGNAL(newId(int)), this, SLOT(sPopulate()));
  connect(_listPrice, SIGNAL(lostFocus()), this, SLOT(sUpdateMargins()));

  _item->setType(ItemLineEdit::cSold);
  _listPrice->setValidator(omfgThis->priceVal());
  _extPrice->setPrecision(omfgThis->priceVal());
  _stdCost->setPrecision(omfgThis->costVal());
  _actCost->setPrecision(omfgThis->costVal());
  _extStdCost->setPrecision(omfgThis->costVal());
  _extActCost->setPrecision(omfgThis->costVal());
  _stdMargin->setPrecision(omfgThis->percentVal());
  _actMargin->setPrecision(omfgThis->percentVal());
  _pricingRatio->setPrecision(omfgThis->percentVal());

  if (!_privileges->check("MaintainListPrices"))
  {
    _listPrice->setEnabled(FALSE);
    _close->setText(tr("&Close"));
    _save->hide();
  }
}

itemListPrice::~itemListPrice()
{
  // no need to delete child widgets, Qt does it all for us
}

void itemListPrice::languageChange()
{
  retranslateUi(this);
}

enum SetResponse itemListPrice::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("item_id", &valid);
  if (valid)
  {
    _item->setId(param.toInt());
    _item->setReadOnly(TRUE);
  }

  return NoError;
}

void itemListPrice::sSave()
{
  q.prepare( "UPDATE item "
             "SET item_listprice=:item_listprice "
             "WHERE (item_id=:item_id);" );
  q.bindValue(":item_listprice", _listPrice->toDouble());
  q.bindValue(":item_id", _item->id());
  q.exec();

  accept();
}

void itemListPrice::sPopulate()
{
  q.prepare( "SELECT uom_name, iteminvpricerat(item_id) AS invpriceratio,"
             "       item_listprice,"
             "       (item_listprice / iteminvpricerat(item_id)) AS extprice,"
             "       stdCost(item_id) AS standardcost,"
             "       actCost(item_id) AS actualcost,"
             "       (stdCost(item_id) * iteminvpricerat(item_id)) AS extstandardcost,"
             "       (actCost(item_id) * iteminvpricerat(item_id)) AS extactualcost "
             "FROM item JOIN uom ON (item_price_uom_id=uom_id)"
             "WHERE (item_id=:item_id);" );
  q.bindValue(":item_id", _item->id());
  q.exec();
  if (q.first())
  {
    _cachedRatio = q.value("invpriceratio").toDouble();
    _cachedStdCost = q.value("standardcost").toDouble();
    _cachedActCost = q.value("actualcost").toDouble();

    _listPrice->setDouble(q.value("item_listprice").toDouble());
    _priceUOM->setText(q.value("uom_name").toString());
    _pricingRatio->setDouble(q.value("invpriceratio").toDouble());
    _extPrice->setDouble(q.value("extprice").toDouble());

    _stdCost->setDouble(q.value("standardcost").toDouble());
    _actCost->setDouble(q.value("actualcost").toDouble());
    _extStdCost->setDouble(q.value("extstandardcost").toDouble());
    _extActCost->setDouble(q.value("extactualcost").toDouble());
  }

  sUpdateMargins();
}

void itemListPrice::sUpdateMargins()
{
  if (_item->isValid())
  {
    double price = _listPrice->toDouble() / _cachedRatio;

    _extPrice->setDouble(price);

    QString stylesheet;

    if (_cachedStdCost > 0.0)
    {
      _stdMargin->setDouble((price - _cachedStdCost) / price * 100);

      if (_cachedStdCost > price)
        stylesheet = QString("* { color: %1; }").arg(namedColor("error").name());
    }
    else
      _stdMargin->setText("N/A");

    _stdMargin->setStyleSheet(stylesheet);

    stylesheet = "";
    if (_cachedActCost > 0.0)
    {
      _actMargin->setDouble((price - _cachedActCost) / price * 100);

      if (_cachedActCost > price)
        stylesheet = QString("* { color: %1; }").arg(namedColor("error").name());
    }
    else
      _actMargin->setText("N/A");

    _actMargin->setStyleSheet(stylesheet);
  }
}
