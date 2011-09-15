/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "priceList.h"

#include <QSqlError>
#include <QVariant>

#include "mqlutil.h"

priceList::priceList(QWidget* parent, const char * name, Qt::WindowFlags fl)
    : XDialog(parent, name, fl)
{
  setupUi(this);

  connect(_item,   SIGNAL(newId(int)),        this,    SLOT(sNewItem()));
  connect(_close,  SIGNAL(clicked()),         this,    SLOT(reject()));
  connect(_price,  SIGNAL(itemSelected(int)), _select, SLOT(animateClick()));
  connect(_price,  SIGNAL(itemSelected(int)), this,    SLOT(sSelect()));
  connect(_price,  SIGNAL(valid(bool)),       _select, SLOT(setEnabled(bool)));
  connect(_select, SIGNAL(clicked()),         this,    SLOT(sSelect()));

  _price->addColumn(tr("Schedule"),                  -1, Qt::AlignLeft,  true, "schedulename");
  _price->addColumn(tr("Source"),           _itemColumn, Qt::AlignLeft,  true, "type");
  _price->addColumn(tr("Qty. Break"),        _qtyColumn, Qt::AlignRight, true, "qty_break");
  _price->addColumn(tr("Qty. UOM"),          _qtyColumn, Qt::AlignRight, true, "qty_uom");
  _price->addColumn(tr("Price"),           _priceColumn, Qt::AlignRight, true, "base_price");
  _price->addColumn(tr("Price UOM"),       _priceColumn, Qt::AlignRight, true, "price_uom");
  _price->addColumn(tr("Discount %"),      _prcntColumn, Qt::AlignRight, true, "discountpercent");
  _price->addColumn(tr("Fixed Discount"),  _priceColumn, Qt::AlignRight, true, "discountfixed");
  _price->addColumn(tr("Currency"),     _currencyColumn, Qt::AlignLeft,  true, "currency");
  _price->addColumn(tr("Price (in Base)"), _priceColumn, Qt::AlignRight, true, "price");
  // column title reset in priceList::set

  if (omfgThis->singleCurrency())
  {
    _price->hideColumn(_price->column("price"));
    _price->hideColumn(_price->column("currency"));
  }

  _shiptoid = -1;

  _qty->setValidator(omfgThis->qtyVal());
  _listPrice->setPrecision(omfgThis->priceVal());
}

priceList::~priceList()
{
  // no need to delete child widgets, Qt does it all for us
}

void priceList::languageChange()
{
  retranslateUi(this);
}

enum SetResponse priceList::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("cust_id", &valid);
  if (valid)
  {
    _cust->setId(param.toInt());
    _cust->setReadOnly(true);
  }

  param = pParams.value("shipto_id", &valid);
  if (valid)
    _shiptoid = param.toInt();

  param = pParams.value("item_id", &valid);
  if (valid)
  {
    _item->setId(param.toInt());
    _item->setReadOnly(true);
  }

  param = pParams.value("curr_id", &valid);
  if (valid)
  {
    _curr_id = param.toInt();
    if (! omfgThis->singleCurrency())
    {
      q.prepare("SELECT currConcat(:curr_id) AS currConcat;");
      q.bindValue(":curr_id", _curr_id);
      q.exec();
      if (q.first())
        _price->headerItem()->setText(_price->column("price"),
                                      tr("Price\n(in %1)")
                                        .arg(q.value("currConcat").toString()));
    }
  }

  param = pParams.value("effective", &valid);
  _effective = (valid) ? param.toDate() : QDate::currentDate();

  param = pParams.value("asof", &valid);
  _asOf = (valid) ? param.toDate() : QDate::currentDate();

  param = pParams.value("qty", &valid);
  if (valid)
  {
    _qty->setDouble(param.toDouble());
    _qty->setEnabled(false);
  }

  sFillList();

  return NoError;
}

void priceList::sNewItem()
{
  _listPrice->clear();
  if (_item->isValid())
  {
    XSqlQuery itemq;
    itemq.prepare("SELECT item_listprice"
                  "  FROM item"
                  " WHERE (item_id=:id);");
    itemq.bindValue(":id", _item->id());
    itemq.exec();
    if (itemq.first())
      _listPrice->setDouble(itemq.value("item_listprice").toDouble());
    else if (itemq.lastError().type() != QSqlError::NoError)
    {
      systemError(this, itemq.lastError().text(), __FILE__, __LINE__);
      return;
    }
  }
}

void priceList::sSelect()
{
  switch (_price->id())
  {
    case 1:
    case 2:
    case 3:
    case 4:
    case 6:
      q.prepare( "SELECT currToCurr(ipshead_curr_id, :curr_id, ipsitem_price, "
                 "                  :effective) AS price "
                 "FROM ipsitem JOIN ipshead ON (ipsitem_ipshead_id = ipshead_id) "
                 "WHERE (ipsitem_id=:ipsitem_id);" );
      q.bindValue(":ipsitem_id", _price->altId());

      break;

    case 11:
    case 12:
    case 13:
    case 14:
    case 16:
      q.prepare( "SELECT currToLocal(:curr_id,"
                 "       noneg(item_listprice - (item_listprice * ipsprodcat_discntprcnt) - ipsprodcat_fixedamtdiscount),"
                 "       :effective) AS price "
                 "  FROM ipsprodcat JOIN item ON (ipsprodcat_prodcat_id=item_prodcat_id AND item_id=:item_id) "
                 " WHERE (ipsprodcat_id=:ipsprodcat_id);" );
      q.bindValue(":item_id", _item->id());
      q.bindValue(":ipsprodcat_id", _price->altId());

      break;

    case 5:
      q.prepare( "SELECT currToLocal(:curr_id, "
                 "        item_listprice - (item_listprice * cust_discntprcnt),"
                 "        :effective) AS price "
                 "FROM cust, item "
                 "WHERE ( (cust_id=:cust_id)"
                 " AND (item_id=:item_id) );" );
      q.bindValue(":cust_id", _cust->id());
      q.bindValue(":item_id", _item->id());

      break;

    default:
      q.prepare( "SELECT 0 AS price;" );
  }

  q.bindValue(":curr_id", _curr_id);
  q.bindValue(":effective", _effective);
  q.exec();
  if (q.first())
    _selectedPrice = q.value("price").toDouble();
  else
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  accept();
}

void priceList::sFillList()
{
  bool    ok = false;
  QString errString;
  MetaSQLQuery pricelistm = MQLUtil::mqlLoad("pricelist", "detail",
                                          errString, &ok);
  if (! ok)
  {
    systemError(this, errString, __FILE__, __LINE__);
    return;
  }

  ParameterList pricelistp;
  pricelistp.append("na",               tr("N/A"));
  pricelistp.append("customer",         tr("Customer"));
  pricelistp.append("shipTo",           tr("Cust. Ship-To"));
  pricelistp.append("shipToPattern",    tr("Cust. Ship-To Pattern"));
  pricelistp.append("custType",         tr("Cust. Type"));
  pricelistp.append("custTypePattern",  tr("Cust. Type Pattern"));
  pricelistp.append("sale",             tr("Sale"));
  pricelistp.append("listPrice",        tr("List Price"));
  pricelistp.append("item_id",          _item->id());
  pricelistp.append("cust_id",          _cust->id());
  pricelistp.append("shipto_id",        _shiptoid);
  pricelistp.append("curr_id",          _curr_id);
  pricelistp.append("effective",        _effective);
  pricelistp.append("asof",             _asOf);

  XSqlQuery pricelistq = pricelistm.toQuery(pricelistp);
  _price->populate(pricelistq, true);
}
