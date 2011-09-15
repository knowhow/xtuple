/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "vendorPriceList.h"

#include <QVariant>
#include <QSqlError>

vendorPriceList::vendorPriceList(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);


  // signals and slots connections
  connect(_price, SIGNAL(clicked(QModelIndex)), this, SLOT(sQtyUpdate()));
  connect(_select, SIGNAL(clicked()), this, SLOT(sSelect()));
  connect(_close, SIGNAL(clicked()), this, SLOT(reject()));
  connect(_price, SIGNAL(itemSelected(int)), _select, SLOT(animateClick()));
  connect(_price, SIGNAL(valid(bool)), _select, SLOT(setEnabled(bool)));
  connect(_qty, SIGNAL(textChanged(const QString&)), this, SLOT(sQtyEntered()));

  QString base;
  q.exec("SELECT currConcat(baseCurrID()) AS base;");
  if (q.first())
    base = q.value("base").toString();
  else
    base = tr("Base");

  _price->addColumn(tr("Qty Break"),                   _qtyColumn, Qt::AlignRight,true, "itemsrcp_qtybreak");
  _price->addColumn(tr("Currency"),               _currencyColumn, Qt::AlignLeft, true, "currabbr");
  _price->addColumn(tr("Unit Price"),                          -1, Qt::AlignRight,true, "itemsrcp_price");
  _price->addColumn(tr("Unit Price\n(%1)").arg(base),_moneyColumn, Qt::AlignRight,true, "itemsrcp_price_base");

}

vendorPriceList::~vendorPriceList()
{
  // no need to delete child widgets, Qt does it all for us
}

void vendorPriceList::languageChange()
{
  retranslateUi(this);
}

enum SetResponse vendorPriceList::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("itemsrc_id", &valid);
  if (valid)
  {
    _itemsrcid = param.toInt();
  }

  param = pParams.value("qty", &valid);
  if (valid)
  {
    if(param.toDouble() <= 0)
      _qty->setDouble(1);
    else
    _qty->setDouble(param.toDouble());

  }

  sFillList();

  return NoError;
}

void vendorPriceList::sQtyUpdate()
{
  q.prepare( "SELECT itemsrcp_qtybreak "
             "FROM itemsrcp "
             "WHERE (itemsrcp_id=:itemsrcp_id) "
             "ORDER BY itemsrcp_qtybreak;" );

  q.bindValue(":itemsrcp_id", _price->id());
  q.exec();
  if (q.first())
    _qty->setDouble(q.value("itemsrcp_qtybreak").toDouble());
  else
  {
    _qty->setDouble(0);
  }
  sPopulatePricing();
}

void vendorPriceList::sQtyEntered()
{
  for (int i = 0; i < _price->topLevelItemCount(); i++)
  {
    _price->setCurrentItem(_price->topLevelItem(i));
    _price->scrollToItem(_price->topLevelItem(i));

    if ( _qty->toDouble() >= _price->currentItem()->rawValue("itemsrcp_qtybreak").toDouble() )
      break;
  }

  sPopulatePricing();
}

void vendorPriceList::sPopulatePricing()
{
   q.prepare( "SELECT currToCurr(itemsrcp_curr_id, :curr_id, itemsrcp_price, :effective) "
		      "AS new_itemsrcp_price "
               "FROM itemsrcp "
               "WHERE ( (itemsrcp_itemsrc_id=:itemsrc_id)"
               " AND (itemsrcp_qtybreak <= :qty) ) "
               "ORDER BY itemsrcp_qtybreak DESC "
               "LIMIT 1;" );
    q.bindValue(":itemsrc_id", _itemsrcid);
    q.bindValue(":qty", _qty->toDouble());
    q.bindValue(":curr_id", _unitPrice->id());
    q.bindValue(":effective", _unitPrice->effective());
    q.exec();
    if (q.first())
      _unitPrice->setLocalValue(q.value("new_itemsrcp_price").toDouble());
    else
	_unitPrice->clear();

  _extendedPrice->setLocalValue(_qty->toDouble() * _unitPrice->localValue());
}

void vendorPriceList::sSelect()
{
  if (  _qty->toDouble() > 0 )
     _selectedQty = _qty->toDouble();
     else
     _selectedQty = 1;

     accept();
}

void vendorPriceList::sFillList()
{
  XSqlQuery priceq;
  priceq.prepare("SELECT *,"
                 " currConcat(itemsrcp_curr_id) AS currabbr, "
                 " currToBase(itemsrcp_curr_id, itemsrcp_price,"
                 "            itemsrcp_updated) AS itemsrcp_price_base,"
                 " 'qty' AS itemsrcp_qtybreak_xtnumericrole,"
                 " 'purchprice' AS itemsrcp_price_xtnumericrole,"
                 " 'purchprice' AS itemsrcp_price_base_xtnumericrole "
                 "FROM itemsrcp "
                 "WHERE (itemsrcp_itemsrc_id=:itemsrc_id) "
                 "ORDER BY itemsrcp_qtybreak DESC;" );
  priceq.bindValue(":itemsrc_id", _itemsrcid);
  priceq.exec();
  _price->populate(priceq, TRUE);

   priceq.exec();
   if (priceq.first())
   _unitPrice->setId(priceq.value("itemsrcp_curr_id").toInt());
   _extendedPrice->setId(priceq.value("itemsrcp_curr_id").toInt());

  _price->clearSelection();
  for (int i = 0; i < _price->topLevelItemCount(); i++)
  {
    if ( _qty->toDouble() >= _price->topLevelItem(i)->text(0).toDouble() )
    {
    _price->setCurrentItem(_price->topLevelItem(i));
    _price->scrollToItem(_price->topLevelItem(i));
      break;
    }
  }
}

