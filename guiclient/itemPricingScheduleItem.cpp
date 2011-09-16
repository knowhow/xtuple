/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "itemPricingScheduleItem.h"
#include "characteristicPrice.h"
#include "xdoublevalidator.h"

#include <metasql.h>
#include "mqlutil.h"

#include <QMessageBox>
#include <QSqlError>
#include <QValidator>
#include <QVariant>

itemPricingScheduleItem::itemPricingScheduleItem(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  _save = _buttonBox->button(QDialogButtonBox::Save);

  connect(_buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
  connect(_buttonBox, SIGNAL(accepted()), this, SLOT(sSave()));
  connect(_item, SIGNAL(valid(bool)), _save, SLOT(setEnabled(bool)));
  connect(_item, SIGNAL(newId(int)), this, SLOT(sUpdateCosts(int)));
  connect(_price, SIGNAL(idChanged(int)), _actCost, SLOT(setId(int)));
  connect(_price, SIGNAL(idChanged(int)), _stdCost, SLOT(setId(int)));
  connect(_price, SIGNAL(idChanged(int)), _listPrice, SLOT(setId(int)));
  connect(_price, SIGNAL(effectiveChanged(const QDate&)), _actCost, SLOT(setEffective(const QDate&)));
  connect(_price, SIGNAL(effectiveChanged(const QDate&)), _stdCost, SLOT(setEffective(const QDate&)));
  connect(_price, SIGNAL(effectiveChanged(const QDate&)), _listPrice, SLOT(setEffective(const QDate&)));
  connect(_price, SIGNAL(valueChanged()), this, SLOT(sUpdateMargins()));
  connect(_itemSelected, SIGNAL(toggled(bool)), this, SLOT(sTypeChanged(bool)));
  connect(_discountSelected, SIGNAL(toggled(bool)), this, SLOT(sTypeChanged(bool)));
  connect(_freightSelected, SIGNAL(toggled(bool)), this, SLOT(sTypeChanged(bool)));
  connect(_qtyUOM, SIGNAL(newID(int)), this, SLOT(sQtyUOMChanged()));
  connect(_priceUOM, SIGNAL(newID(int)), this, SLOT(sPriceUOMChanged()));
  connect(_new, SIGNAL(clicked()), this, SLOT(sNew()));
  connect(_edit, SIGNAL(clicked()), this, SLOT(sEdit()));
  connect(_delete, SIGNAL(clicked()), this, SLOT(sDelete()));
  connect(_dscbyItem, SIGNAL(toggled(bool)), this, SLOT(sCheckEnable()));
  connect(_dscbyprodcat, SIGNAL(toggled(bool)), this, SLOT(sCheckEnable()));
  connect(_dscitem, SIGNAL(valid(bool)), _save, SLOT(setEnabled(bool)));

  _ipsheadid = -1;
  _ipsitemid = -1;
  _ipsprodcatid = -1;
  _ipsfreightid = -1;
  _invuomid = -1;
  
  _charprice->addColumn(tr("Characteristic"), _itemColumn, Qt::AlignLeft, true,  "char_name" );
  _charprice->addColumn(tr("Value"),          -1,          Qt::AlignLeft, true,  "ipsitemchar_value" );
  _charprice->addColumn(tr("Price"),          _priceColumn,Qt::AlignRight, true, "ipsitemchar_price" );

  _qtyBreak->setValidator(omfgThis->qtyVal());
  _qtyBreakCat->setValidator(omfgThis->qtyVal());
  _qtyBreakFreight->setValidator(omfgThis->qtyVal());
  _discount->setValidator(new XDoubleValidator(-999, 999, decimalPlaces("percent"), this));
  _fixedAmtDiscount->setValidator(omfgThis->negMoneyVal());
  _pricingRatio->setPrecision(omfgThis->percentVal());
  _stdMargin->setPrecision(omfgThis->percentVal());
  _actMargin->setPrecision(omfgThis->percentVal());
  _item->setType(ItemLineEdit::cSold);
  _prodcat->setType(XComboBox::ProductCategories);
  _zoneFreight->setType(XComboBox::ShippingZones);
  _shipViaFreight->setType(XComboBox::ShipVias);
  _freightClass->setType(XComboBox::FreightClasses);
  
  _tab->setTabEnabled(_tab->indexOf(_configuredPrices),FALSE);
 
  q.exec("SELECT uom_name FROM uom WHERE (uom_item_weight);");
  if (q.first())
  {
    QString uom = q.value("uom_name").toString();
    QString title (tr("Price per "));
    title += uom;
    _perUOMFreight->setText(title);
    _qtyBreakFreightUOM->setText(uom);
  }

  _rejectedMsg = tr("The application has encountered an error and must "
                    "stop editing this Pricing Schedule.\n%1");
}

itemPricingScheduleItem::~itemPricingScheduleItem()
{
  // no need to delete child widgets, Qt does it all for us
}

void itemPricingScheduleItem::languageChange()
{
  retranslateUi(this);
}

enum SetResponse itemPricingScheduleItem::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("ipshead_id", &valid);
  if (valid)
    _ipsheadid = param.toInt();

  param = pParams.value("curr_id", &valid);
  if (valid)
  {
    _price->setId(param.toInt());
    _priceFreight->setId(param.toInt());
  }

  param = pParams.value("updated", &valid);
  if (valid)
  {
    _price->setEffective(param.toDate());
    _priceFreight->setEffective(param.toDate());
  }

  param = pParams.value("ipsitem_id", &valid);
  if (valid)
  {
    _ipsitemid = param.toInt();
    q.prepare("SELECT ipsitem_ipshead_id,"
              "       ipsitem_qtybreak, "
              "       ipsitem_discntprcnt, "
              "       ipsitem_fixedamtdiscount "
              "FROM ipsiteminfo "
              "WHERE (ipsitem_id = :ipsitem_id) ");
    q.bindValue(":ipsitem_id", _ipsitemid);
    q.exec();
    if (q.first())
    {
      _ipsheadid = q.value("ipsitem_ipshead_id").toInt();
      if(q.value("ipsitem_discntprcnt").toDouble() != 0.0 ||
         q.value("ipsitem_fixedamtdiscount").toDouble() != 0.0)
      {
        _discountSelected->setChecked(true);
        _dscbyItem->setChecked(true);
      }
      else
      {
        _itemSelected->setChecked(true);
      }
      populate();
    }
  }

  param = pParams.value("ipsprodcat_id", &valid);
  if (valid)
  {
    _ipsprodcatid = param.toInt();
    _discountSelected->setChecked(true);
    _dscbyprodcat->setChecked(true);
    populate();
  }

  param = pParams.value("ipsfreight_id", &valid);
  if (valid)
  {
    _ipsfreightid = param.toInt();
    _freightSelected->setChecked(true);
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

      _item->setReadOnly(TRUE);
      _prodcat->setEnabled(FALSE);
      _typeGroup->setEnabled(FALSE);
      _dscitem->setReadOnly(TRUE);
      _discountBy->setEnabled(FALSE);

      if(_ipsitemid != -1)
        _qtyBreak->setFocus();
      else if(_ipsprodcatid != -1)
        _qtyBreakCat->setFocus();
    }
    else if (param.toString() == "view")
    {
      _mode = cView;

      _item->setReadOnly(TRUE);
      _prodcat->setEnabled(FALSE);
      _qtyBreak->setEnabled(FALSE);
      _qtyBreakCat->setEnabled(FALSE);
      _qtyBreakFreight->setEnabled(FALSE);
      _price->setEnabled(FALSE);
      _discount->setEnabled(FALSE);
      _fixedAmtDiscount->setEnabled(FALSE);
      _priceFreight->setEnabled(FALSE);
      _typeGroup->setEnabled(FALSE);
      _typeFreightGroup->setEnabled(FALSE);
      _siteFreight->setEnabled(FALSE);
      _zoneFreightGroup->setEnabled(FALSE);
      _shipViaFreightGroup->setEnabled(FALSE);
      _freightClassGroup->setEnabled(FALSE);
      _dscitem->setReadOnly(TRUE);
      _discountBy->setEnabled(FALSE);
      _buttonBox->setStandardButtons(QDialogButtonBox::Close);
    }
  }

  return NoError;
}

void itemPricingScheduleItem::sCheckEnable()
{
  if (_dscbyItem->isChecked())
  {
    _prodcatLit->hide();
    _prodcat->hide();
    _itemGroup->show();
    if (_dscitem->isValid())
      _save->setEnabled(true);
    else
      _save->setEnabled(false);
  }
  if (_dscbyprodcat->isChecked())
  {
    _itemGroup->hide();
    _prodcatLit->show();
    _prodcat->show();
    _save->setEnabled(true);
  }
}

void itemPricingScheduleItem::sSave()
{
  sSave(TRUE);
}

void itemPricingScheduleItem::sSave( bool pClose)
{
  if(_itemSelected->isChecked())
  {
    q.prepare( "SELECT ipsitem_id "
               "  FROM ipsitem "
               " WHERE((ipsitem_ipshead_id=:ipshead_id)"
               "   AND (ipsitem_item_id=:item_id)"
               "   AND (ipsitem_qty_uom_id=:uom_id)"
               "   AND (ipsitem_qtybreak=:qtybreak)"
               "   AND (ipsitem_id <> :ipsitem_id));" );
    q.bindValue(":ipshead_id", _ipsheadid);
    q.bindValue(":item_id", _item->id());
    q.bindValue(":qtybreak", _qtyBreak->toDouble());
    q.bindValue(":uom_id", _qtyUOM->id());
    q.bindValue(":ipsitem_id", _ipsitemid);
    q.exec();
    if (q.first())
    {
      QMessageBox::critical( this, tr("Cannot Create Pricing Schedule Item"),
                             tr( "There is an existing Pricing Schedule Item for the selected Pricing Schedule, Item and Quantity Break defined.\n"
                                 "You may not create duplicate Pricing Schedule Items." ) );
      return;
    }
    else if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, _rejectedMsg.arg(q.lastError().databaseText()),
                __FILE__, __LINE__);
      done(-1);
    }
  }
  else if(_discountSelected->isChecked())
  {
    if(_dscbyItem->isChecked())
    {
      q.prepare( "SELECT * "
                 "FROM ipsitem JOIN ipshead ON (ipsitem_ipshead_id=ipshead_id) "
                 "WHERE ((ipshead_id = :ipshead_id)"
                 "   AND (ipsitem_item_id = :item_id)"
                 "   AND (ipsitem_qtybreak = :qtybreak)"
                 "   AND (ipsitem_id <> :ipsitem_id));" );

      q.bindValue(":ipshead_id", _ipsheadid);
      q.bindValue(":item_id", _dscitem->id());
      q.bindValue(":qtybreak", _qtyBreakCat->toDouble());
      q.bindValue(":ipsitem_id", _ipsitemid);
      q.exec();
      if (q.first())
      {
        QMessageBox::critical( this, tr("Cannot Create Pricing Schedule Item"),
                               tr( "There is an existing Pricing Schedule Item for the selected Pricing Schedule, Item and Quantity Break defined.\n"
                                   "You may not create duplicate Pricing Schedule Items." ) );
        return;
      }
      else if (q.lastError().type() != QSqlError::NoError)
      {
        systemError(this, _rejectedMsg.arg(q.lastError().databaseText()),
                  __FILE__, __LINE__);
        done(-1);
      }
    }
    else if(_dscbyprodcat->isChecked())
    {
      q.prepare( "SELECT ipsprodcat_id "
                 "FROM ipsprodcat "
                 "WHERE ( (ipsprodcat_ipshead_id=:ipshead_id)"
                 " AND (ipsprodcat_prodcat_id=:prodcat_id)"
                 " AND (ipsprodcat_qtybreak=:qtybreak)"
                 " AND (ipsprodcat_id <> :ipsprodcat_id) );" );
      q.bindValue(":ipshead_id", _ipsheadid);
      q.bindValue(":prodcat_id", _prodcat->id());
      q.bindValue(":qtybreak", _qtyBreakCat->toDouble());
      q.bindValue(":ipsprodcat_id", _ipsprodcatid);
      q.exec();
      if (q.first())
      {
        QMessageBox::critical( this, tr("Cannot Create Pricing Schedule Item"),
                               tr( "There is an existing Pricing Schedule Item for the selected Pricing Schedule, Product Category and Quantity Break defined.\n"
                                   "You may not create duplicate Pricing Schedule Items." ) );
        return;
      }
      else if (q.lastError().type() != QSqlError::NoError)
      {
        systemError(this, _rejectedMsg.arg(q.lastError().databaseText()),
                  __FILE__, __LINE__);
        done(-1);
      }
    }
  }
  else if(_freightSelected->isChecked())
  {
    MetaSQLQuery mql = mqlLoad("pricingFreight", "detail");

    ParameterList params;
    params.append("checkDup", true);
    params.append("ipshead_id", _ipsheadid);
    if (_siteFreight->isSelected())
      params.append("warehous_id", _siteFreight->id());
    if (_selectedZoneFreight->isChecked())
      params.append("shipzone_id", _zoneFreight->id());
    if (_selectedFreightClass->isChecked())
      params.append("freightclass_id", _freightClass->id());
    if (_selectedShipViaFreight->isChecked())
      params.append("shipvia", _shipViaFreight->currentText());
    params.append("qtybreak", _qtyBreakFreight->toDouble());
    params.append("ipsfreight_id", _ipsfreightid);
    q = mql.toQuery(params);
    if (q.first())
    {
      QMessageBox::critical( this, tr("Cannot Create Pricing Schedule Item"),
                             tr( "There is an existing Pricing Schedule Item for the selected Pricing Schedule, Freight Criteria and Quantity Break defined.\n"
                                 "You may not create duplicate Pricing Schedule Items." ) );
      return;
    }
    else if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, _rejectedMsg.arg(q.lastError().databaseText()),
                __FILE__, __LINE__);
      done(-1);
    }
  }

  if (_mode == cNew)
  {
    if(_itemSelected->isChecked())
    {
      q.exec("SELECT NEXTVAL('ipsitem_ipsitem_id_seq') AS ipsitem_id;");
      if (q.first())
        _ipsitemid = q.value("ipsitem_id").toInt();
      else if (q.lastError().type() != QSqlError::NoError)
      {
        systemError(this, _rejectedMsg.arg(q.lastError().databaseText()),
                  __FILE__, __LINE__);
        done(-1);
      }

      q.prepare( "INSERT INTO ipsitem "
                 "( ipsitem_id, ipsitem_ipshead_id, ipsitem_item_id,"
                 "  ipsitem_qty_uom_id, ipsitem_qtybreak, "
                 "  ipsitem_price_uom_id, ipsitem_price, "
                 "  ipsitem_discntprcnt, ipsitem_fixedamtdiscount ) "
                 "VALUES "
                 "( :ipsitem_id, :ipshead_id, :ipsitem_item_id, "
                 "  :qty_uom_id, :ipsitem_qtybreak, "
                 "  :price_uom_id, :ipsitem_price, "
                 "  :ipsitem_discntprcnt, :ipsitem_fixedamtdiscount );" );
    }
    else if(_discountSelected->isChecked())
    {
      if(_dscbyItem->isChecked())
      {
        q.exec("SELECT NEXTVAL ('ipsitem_ipsitem_id_seq') AS ipsitem_id;");
        if (q.first())
          _ipsitemid = q.value("ipsitem_id").toInt();
        else if (q.lastError().type() != QSqlError::NoError)
        {
          systemError(this, _rejectedMsg.arg(q.lastError().databaseText()),
                    __FILE__, __LINE__);
          done(-1);
        }

        q.prepare("INSERT INTO ipsitem "
                  "( ipsitem_id, ipsitem_ipshead_id, "
                  "  ipsitem_item_id, ipsitem_qtybreak, "
                  "  ipsitem_price, ipsitem_qty_uom_id, "
                  "  ipsitem_price_uom_id, ipsitem_discntprcnt, "
                  "  ipsitem_fixedamtdiscount ) "
                  "VALUES "
                  "( :ipsitem_id, :ipshead_id, "
                  "  :ipsitem_item_id, :ipsitem_qtybreak, "
                  "  :ipsitem_price, :ipsitem_qty_uom_id, "
                  "  :ipsitem_price_uom_id, :ipsitem_discntprcnt, "
                  "  :ipsitem_fixedamtdiscount); ");

        XSqlQuery qry;
        qry.prepare("SELECT item_inv_uom_id, item_price_uom_id "
                    "FROM item "
                    "WHERE (item_id=:item_id);");
        qry.bindValue(":item_id", _dscitem->id());
        qry.exec();
        if (qry.first())
        {
          q.bindValue(":ipsitem_qty_uom_id", qry.value("item_inv_uom_id"));
          q.bindValue(":ipsitem_price_uom_id", qry.value("item_price_uom_id"));
        }
        else if (qry.lastError().type() != QSqlError::NoError)
        {
          systemError(this, _rejectedMsg.arg(qry.lastError().databaseText()),
                    __FILE__, __LINE__);
          done(-1);
        }
      }
      else if(_dscbyprodcat->isChecked())
      {
        q.exec("SELECT NEXTVAL('ipsprodcat_ipsprodcat_id_seq') AS ipsprodcat_id;");
        if (q.first())
          _ipsprodcatid = q.value("ipsprodcat_id").toInt();
        else if (q.lastError().type() != QSqlError::NoError)
        {
          systemError(this, _rejectedMsg.arg(q.lastError().databaseText()),
                    __FILE__, __LINE__);
          done(-1);
        }

        q.prepare( "INSERT INTO ipsprodcat "
                   "( ipsprodcat_id, ipsprodcat_ipshead_id, ipsprodcat_prodcat_id, ipsprodcat_qtybreak,"
                   "  ipsprodcat_discntprcnt, ipsprodcat_fixedamtdiscount )"
                   "VALUES "
                   "( :ipsprodcat_id, :ipshead_id, :ipsprodcat_prodcat_id, :ipsprodcat_qtybreak,"
                   "  :ipsprodcat_discntprcnt, :ipsprodcat_fixedamtdiscount );" );
      }
    }
    else if(_freightSelected->isChecked())
    {
      q.exec("SELECT NEXTVAL('ipsfreight_ipsfreight_id_seq') AS ipsfreight_id;");
      if (q.first())
        _ipsfreightid = q.value("ipsfreight_id").toInt();
      else if (q.lastError().type() != QSqlError::NoError)
      {
        systemError(this, _rejectedMsg.arg(q.lastError().databaseText()),
                  __FILE__, __LINE__);
        done(-1);
      }

      q.prepare( "INSERT INTO ipsfreight "
                 "( ipsfreight_id, ipsfreight_ipshead_id, ipsfreight_qtybreak, ipsfreight_price,"
                 "  ipsfreight_type, ipsfreight_warehous_id, ipsfreight_shipzone_id,"
                 "  ipsfreight_freightclass_id, ipsfreight_shipvia ) "
                 "VALUES "
                 "( :ipsfreight_id, :ipshead_id, :ipsfreight_qtybreak, :ipsfreight_price,"
                 "  :ipsfreight_type, :ipsfreight_warehous_id, :ipsfreight_shipzone_id,"
                 "  :ipsfreight_freightclass_id, :ipsfreight_shipvia ) " );
    }
  }
  else if (_mode == cEdit)
  {
    if(_itemSelected->isChecked())
      q.prepare( "UPDATE ipsitem "
                 "   SET ipsitem_qty_uom_id=:qty_uom_id,"
                 "       ipsitem_qtybreak=:ipsitem_qtybreak,"
                 "       ipsitem_price_uom_id=:price_uom_id,"
                 "       ipsitem_price=:ipsitem_price "
                 "WHERE (ipsitem_id=:ipsitem_id);" );
    else if(_discountSelected->isChecked())
    {
      if(_dscbyprodcat->isChecked())
      {
        q.prepare( "UPDATE ipsprodcat "
                   "   SET ipsprodcat_qtybreak=:ipsprodcat_qtybreak,"
                   "       ipsprodcat_discntprcnt=:ipsprodcat_discntprcnt,"
                   "       ipsprodcat_fixedamtdiscount=:ipsprodcat_fixedamtdiscount "
                   "WHERE (ipsprodcat_id=:ipsprodcat_id);" );
      }
      else if(_dscbyItem->isChecked())
      {
        q.prepare("UPDATE ipsiteminfo "
                  "SET ipsitem_qtybreak = :ipsitem_qtybreak, "
                  "    ipsitem_discntprcnt = :ipsitem_discntprcnt, "
                  "    ipsitem_fixedamtdiscount = :ipsitem_fixedamtdiscount "
                  "WHERE (ipsitem_id= :ipsitem_id);");
      }
    }
    else if(_freightSelected->isChecked())
      q.prepare( "UPDATE ipsfreight "
                 "   SET ipsfreight_qtybreak=:ipsfreight_qtybreak,"
                 "       ipsfreight_type=:ipsfreight_type,"
                 "       ipsfreight_price=:ipsfreight_price,"
                 "       ipsfreight_warehous_id=:ipsfreight_warehous_id,"
                 "       ipsfreight_shipzone_id=:ipsfreight_shipzone_id,"
                 "       ipsfreight_freightclass_id=:ipsfreight_freightclass_id,"
                 "       ipsfreight_shipvia=:ipsfreight_shipvia "
                 "WHERE (ipsfreight_id=:ipsfreight_id);" );
  }

  q.bindValue(":ipsitem_id", _ipsitemid);
  q.bindValue(":ipsprodcat_id", _ipsprodcatid);
  q.bindValue(":ipsfreight_id", _ipsfreightid);
  q.bindValue(":ipshead_id", _ipsheadid);

  if(_itemSelected->isChecked())
  {
    q.bindValue(":ipsitem_item_id", _item->id());
    q.bindValue(":ipsitem_qtybreak", _qtyBreak->toDouble());
    q.bindValue(":ipsitem_discntprcnt", 0.00);
    q.bindValue(":ipsitem_fixedamtdiscount", 0.00);
    q.bindValue(":ipsitem_price", _price->localValue());
  }
  else if((_discountSelected->isChecked()) && (_dscbyItem->isChecked()))
  {
    q.bindValue(":ipsitem_item_id", _dscitem->id());
    q.bindValue(":ipsitem_qtybreak", _qtyBreakCat->toDouble());
    q.bindValue(":ipsitem_discntprcnt", (_discount->toDouble() / 100.0));
    q.bindValue(":ipsitem_fixedamtdiscount", (_fixedAmtDiscount->toDouble()));
    q.bindValue(":ipsitem_price", 0.00);
  }

  q.bindValue(":ipsprodcat_prodcat_id", _prodcat->id());
  q.bindValue(":ipsprodcat_qtybreak", _qtyBreakCat->toDouble());
  q.bindValue(":ipsfreight_qtybreak", _qtyBreakFreight->toDouble());
  q.bindValue(":ipsprodcat_discntprcnt", (_discount->toDouble() / 100.0));
  q.bindValue(":ipsprodcat_fixedamtdiscount", (_fixedAmtDiscount->toDouble()));
  q.bindValue(":ipsfreight_price", _priceFreight->localValue());
  q.bindValue(":qty_uom_id", _qtyUOM->id());
  q.bindValue(":price_uom_id", _priceUOM->id());

  if (_flatRateFreight->isChecked())
    q.bindValue(":ipsfreight_type", "F");
  else
    q.bindValue(":ipsfreight_type", "P");
  if (_siteFreight->isSelected())
    q.bindValue(":ipsfreight_warehous_id", _siteFreight->id());
  if (_selectedZoneFreight->isChecked())
    q.bindValue(":ipsfreight_shipzone_id", _zoneFreight->id());
  if (_selectedFreightClass->isChecked())
    q.bindValue(":ipsfreight_freightclass_id", _freightClass->id());
  if (_selectedShipViaFreight->isChecked())
    q.bindValue(":ipsfreight_shipvia", _shipViaFreight->currentText());
  q.exec();
  if (q.lastError().type() != QSqlError::NoError)
  {
	systemError(this, _rejectedMsg.arg(q.lastError().databaseText()),
                  __FILE__, __LINE__);
        done(-1);
  }

  if (pClose)
  {
    if(_itemSelected->isChecked())
      done(_ipsitemid);
    if(_discountSelected->isChecked())
    {
      if(_dscbyItem->isChecked())
        done(_ipsitemid);
      else if(_dscbyprodcat->isChecked())
        done(_ipsprodcatid);
    }
    if(_freightSelected->isChecked())
      done(_ipsfreightid);
  }
  else
  {
    _mode = cEdit;

    _item->setReadOnly(TRUE);
    _prodcat->setEnabled(FALSE);
    _typeGroup->setEnabled(FALSE);
  }

}

void itemPricingScheduleItem::populate()
{
  if(_itemSelected->isChecked())
  {
    q.prepare( "SELECT ipsitem_ipshead_id,"
               "       ipsitem_item_id,"
               "       ipsitem_qty_uom_id,"
               "       ipsitem_qtybreak,"
               "       ipsitem_price_uom_id,"
               "       ipsitem_price "
               "FROM ipsitem "
               "WHERE (ipsitem_id=:ipsitem_id);" );
    q.bindValue(":ipsitem_id", _ipsitemid);
    q.exec();
    if (q.first())
    {
      _ipsheadid=q.value("ipsitem_ipshead_id").toInt();
      _item->setId(q.value("ipsitem_item_id").toInt());
      _qtyBreak->setDouble(q.value("ipsitem_qtybreak").toDouble());
      _price->setLocalValue(q.value("ipsitem_price").toDouble());
      _qtyUOM->setId(q.value("ipsitem_qty_uom_id").toInt());
      _priceUOM->setId(q.value("ipsitem_price_uom_id").toInt());

      sUpdateMargins();
    }
    else if (q.lastError().type() != QSqlError::NoError)
    {
	systemError(this, _rejectedMsg.arg(q.lastError().databaseText()),
                  __FILE__, __LINE__);
        done(-1);
    }
  }
  else if(_discountSelected->isChecked())
  {
    if(_dscbyprodcat->isChecked())
    {
      q.prepare( "SELECT ipsprodcat_ipshead_id,"
                 "       ipsprodcat_prodcat_id,"
                 "       ipsprodcat_qtybreak AS qty_brk,"
                 "       (ipsprodcat_discntprcnt * 100) AS discntprcnt,"
                 "       ipsprodcat_fixedamtdiscount AS fxd_amnt "
                 "FROM ipsprodcat "
                 "WHERE (ipsprodcat_id=:ipsprodcat_id);" );
      q.bindValue(":ipsprodcat_id", _ipsprodcatid);
    }
    else if(_dscbyItem->isChecked())
    {
      q.prepare( "SELECT ipsitem_ipshead_id,"
                 "       ipsitem_item_id,"
                 "       ipsitem_qtybreak AS qty_brk,"
                 "       (ipsitem_discntprcnt * 100) AS discntprcnt,"
                 "       ipsitem_fixedamtdiscount AS fxd_amnt "
                 "FROM ipsiteminfo "
                 "WHERE (ipsitem_id=:ipsitem_id);" );
      q.bindValue(":ipsitem_id", _ipsitemid);
    }
    q.exec();
    if (q.first())
    {
      if(_dscbyprodcat->isChecked())
      {
        _ipsheadid=q.value("ipsprodcat_ipshead_id").toInt();
        _prodcat->setId(q.value("ipsprodcat_prodcat_id").toInt());
      }
      else if(_dscbyItem->isChecked())
      {
        _ipsheadid=q.value("ipsitem_ipshead_id").toInt();
        _dscitem->setId(q.value("ipsitem_item_id").toInt());
      }

      _qtyBreakCat->setDouble(q.value("qty_brk").toDouble());
      _discount->setDouble(q.value("discntprcnt").toDouble());
      _fixedAmtDiscount->setDouble(q.value("fxd_amnt").toDouble());
    }
    else if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, _rejectedMsg.arg(q.lastError().databaseText()),
                   __FILE__, __LINE__);
      done(-1);
    }
  }
  else if(_freightSelected->isChecked())
  {
    MetaSQLQuery mql = mqlLoad("pricingFreight", "detail");

    ParameterList params;
    params.append("ipsfreight_id", _ipsfreightid);
    q = mql.toQuery(params);
    if (q.first())
    {
      _ipsheadid=q.value("ipsfreight_ipshead_id").toInt();
      _qtyBreakFreight->setDouble(q.value("ipsfreight_qtybreak").toDouble());
      _priceFreight->setLocalValue(q.value("ipsfreight_price").toDouble());
      if (q.value("ipsfreight_type").toString() == "F")
      {
        _flatRateFreight->setChecked(true);
        _qtyBreakFreight->setEnabled(false);
      }
      else
        _perUOMFreight->setChecked(true);
      if (q.value("ipsfreight_warehous_id").toInt() > 0)
        _siteFreight->setId(q.value("ipsfreight_warehous_id").toInt());
      else
        _siteFreight->setAll();
      if (q.value("ipsfreight_shipzone_id").toInt() > 0)
      {
        _selectedZoneFreight->setChecked(true);
        _zoneFreight->setId(q.value("ipsfreight_shipzone_id").toInt());
      }
      else
        _allZonesFreight->setChecked(true);
      if (q.value("ipsfreight_freightclass_id").toInt() > 0)
      {
        _selectedFreightClass->setChecked(true);
        _freightClass->setId(q.value("ipsfreight_freightclass_id").toInt());
      }
      else
        _allFreightClasses->setChecked(true);
      //  Handle the free-form Ship Via
      _shipViaFreight->setCurrentIndex(-1);
      QString shipvia = q.value("ipsfreight_shipvia").toString();
      if (shipvia.trimmed().length() != 0)
      {
        _selectedShipViaFreight->setChecked(true);
        for (int counter = 0; counter < _shipViaFreight->count(); counter++)
          if (_shipViaFreight->itemText(counter) == shipvia)
            _shipViaFreight->setCurrentIndex(counter);

        if (_shipViaFreight->id() == -1)
        {
          _shipViaFreight->addItem(shipvia);
          _shipViaFreight->setCurrentIndex(_shipViaFreight->count() - 1);
        }
      }
      else
        _allShipViasFreight->setChecked(true);
    }
    else if (q.lastError().type() != QSqlError::NoError)
    {
       systemError(this, _rejectedMsg.arg(q.lastError().databaseText()),
                  __FILE__, __LINE__);
        done(-1);
    }
  }
  sFillList();
}

void itemPricingScheduleItem::sUpdateCosts(int pItemid)
{
  XSqlQuery uom;
  uom.prepare("SELECT uom_id, uom_name"
              "  FROM item"
              "  JOIN uom ON (item_inv_uom_id=uom_id)"
              " WHERE(item_id=:item_id)"
              " UNION "
              "SELECT uom_id, uom_name"
              "  FROM item"
              "  JOIN itemuomconv ON (itemuomconv_item_id=item_id)"
              "  JOIN uom ON (itemuomconv_to_uom_id=uom_id)"
              " WHERE((itemuomconv_from_uom_id=item_inv_uom_id)"
              "   AND (item_id=:item_id))"
              " UNION "
              "SELECT uom_id, uom_name"
              "  FROM item"
              "  JOIN itemuomconv ON (itemuomconv_item_id=item_id)"
              "  JOIN uom ON (itemuomconv_from_uom_id=uom_id)"
              " WHERE((itemuomconv_to_uom_id=item_inv_uom_id)"
              "   AND (item_id=:item_id))"
              " ORDER BY uom_name;");
  uom.bindValue(":item_id", _item->id());
  uom.exec();
  if (q.lastError().type() != QSqlError::NoError)
  {
	systemError(this, _rejectedMsg.arg(q.lastError().databaseText()),
                  __FILE__, __LINE__);
        done(-1);
  }
  _qtyUOM->populate(uom);
  _priceUOM->populate(uom);

  XSqlQuery cost;
  cost.prepare( "SELECT item_inv_uom_id, item_price_uom_id,"
                "       iteminvpricerat(item_id) AS ratio,"
                "       item_listprice, "
                "       (stdcost(item_id) * iteminvpricerat(item_id)) AS standard,"
                "       (actcost(item_id, :curr_id) * iteminvpricerat(item_id)) AS actual "
                "  FROM item"
                " WHERE (item_id=:item_id);" );
  cost.bindValue(":item_id", pItemid);
  cost.bindValue(":curr_id", _actCost->id());
  cost.exec();
  if (cost.first())
  {
    _invuomid = cost.value("item_inv_uom_id").toInt();
    _listPrice->setBaseValue(cost.value("item_listprice").toDouble());
    _pricingRatio->setDouble(cost.value("ratio").toDouble());

    _stdCost->setBaseValue(cost.value("standard").toDouble());
    _actCost->setLocalValue(cost.value("actual").toDouble());

    _qtyUOM->setId(cost.value("item_inv_uom_id").toInt());
    _priceUOM->setId(cost.value("item_price_uom_id").toInt());
  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
	systemError(this, _rejectedMsg.arg(q.lastError().databaseText()),
                  __FILE__, __LINE__);
        done(-1);
  }
  
  if (_item->isConfigured())
    _tab->setTabEnabled(_tab->indexOf(_configuredPrices),TRUE);
  else
    _tab->setTabEnabled(_tab->indexOf(_configuredPrices),FALSE);
}

void itemPricingScheduleItem::sUpdateMargins()
{
  if (_item->isValid())
  {
    double price = _price->baseValue();
    QString stylesheet;

    if (_stdCost->baseValue() > 0.0)
    {
      _stdMargin->setDouble((price - _stdCost->baseValue()) / price * 100);

      if (_stdCost->baseValue() > price)
        stylesheet = QString("* { color: %1; }").arg(namedColor("error").name());
    }
    else
      _stdMargin->setText("N/A");

    _stdMargin->setStyleSheet(stylesheet);

    if (_actCost->baseValue() > 0.0)
    {
      _actMargin->setDouble((price - _actCost->baseValue()) / price * 100);

      if (_actCost->baseValue() > price)
        stylesheet = QString("* { color: %1; }").arg(namedColor("error").name());
    }
    else
      _actMargin->setText("N/A");

    _actMargin->setStyleSheet(stylesheet);
  }
}

void itemPricingScheduleItem::sTypeChanged(bool pChecked)
{
  if(!pChecked)
    return;
  
  if(_itemSelected->isChecked())
  {
    _widgetStack->setCurrentIndex(0);
    _save->setEnabled(_item->isValid());
    _dscbyItem->setChecked(false);
    _dscbyprodcat->setChecked(false);
  }
  else if(_discountSelected->isChecked())
  {
    _widgetStack->setCurrentIndex(1);
    _dscbyItem->setChecked(true);
    _dscbyprodcat->setChecked(false);
    _save->setEnabled(false);
    _save->setEnabled(_dscitem->isValid());
    connect(_dscbyItem, SIGNAL(toggled(bool)), this, SLOT(sCheckEnable()));
    connect(_dscbyprodcat, SIGNAL(toggled(bool)), this, SLOT(sCheckEnable()));
  }
  else if(_freightSelected->isChecked())
  {
    _widgetStack->setCurrentIndex(2);
    _save->setEnabled(true);
    _dscbyItem->setChecked(false);
    _dscbyprodcat->setChecked(false);
  }
}

void itemPricingScheduleItem::sQtyUOMChanged()
{
  if(_qtyUOM->id() != _invuomid)
  {
    _priceUOM->setId(_qtyUOM->id());
    _priceUOM->setEnabled(false);
  }
  else
    _priceUOM->setEnabled(true);
  sPriceUOMChanged();
}

void itemPricingScheduleItem::sPriceUOMChanged()
{
  if(_priceUOM->id() == -1 || _qtyUOM->id() == -1)
    return;

  XSqlQuery cost;
  cost.prepare( "SELECT "
                "       itemuomtouomratio(item_id, :qtyuomid, :priceuomid) AS ratio,"
                "       ((item_listprice / iteminvpricerat(item_id)) * itemuomtouomratio(item_id, :priceuomid, item_inv_uom_id)) AS listprice, "
                "       (stdcost(item_id) * itemuomtouomratio(item_id, :priceuomid, item_inv_uom_id)) AS standard,"
                "       (actcost(item_id, :curr_id) * itemuomtouomratio(item_id, :priceuomid, item_inv_uom_id)) AS actual "
                "  FROM item"
                " WHERE (item_id=:item_id);" );
  cost.bindValue(":item_id", _item->id());
  cost.bindValue(":curr_id", _actCost->id());
  cost.bindValue(":qtyuomid", _qtyUOM->id());
  cost.bindValue(":priceuomid", _priceUOM->id());
  cost.exec();
  if (cost.first())
  {
    _listPrice->setBaseValue(cost.value("listprice").toDouble());
    _pricingRatio->setDouble(cost.value("ratio").toDouble());

    _stdCost->setBaseValue(cost.value("standard").toDouble());
    _actCost->setLocalValue(cost.value("actual").toDouble());

    sUpdateMargins();
  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, _rejectedMsg.arg(q.lastError().databaseText()),
                  __FILE__, __LINE__);
    done(-1);
  }
}

void itemPricingScheduleItem::sNew()
{
  if (_mode == cNew)
    sSave(FALSE);
  ParameterList params;
  params.append("mode", "new");
  params.append("ipsitem_id", _ipsitemid);
  params.append("curr_id", _price->id());
  params.append("item_id", _item->id());

  characteristicPrice newdlg(this, "", TRUE);
  newdlg.set(params);

  int result;
  if ((result = newdlg.exec()) != XDialog::Rejected)
  {
    if (result == -1)
      done(-1);
     else
      sFillList();
  }
}

void itemPricingScheduleItem::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("ipsitem_id", _ipsitemid);
  params.append("ipsitemchar_id", _charprice->id());
  params.append("curr_id", _price->id());
  params.append("item_id", _item->id());

  characteristicPrice newdlg(this, "", TRUE);
  newdlg.set(params);

  int result;
  if ((result = newdlg.exec()) != XDialog::Rejected)
  {
    if (result == -1)
      done(-1);
     else
      sFillList();
  }
}

void itemPricingScheduleItem::sDelete()
{
  q.prepare("DELETE FROM ipsitemchar "
            "WHERE (ipsitemchar_id=:ipsitemchar_id);");
  q.bindValue(":ipsitemchar_id", _charprice->id());
  q.exec();
  if (q.lastError().type() != QSqlError::NoError)
  {
	systemError(this, _rejectedMsg.arg(q.lastError().databaseText()),
                  __FILE__, __LINE__);
        done(-1);
  }
  sFillList();
}

void itemPricingScheduleItem::sFillList()
{
  q.prepare("SELECT ipsitemchar_id, char_name, ipsitemchar_value, ipsitemchar_price, "
            "  'salesprice' AS ipsitemchar_price_xtnumericrole "
            "FROM ipsitemchar, char "
            "WHERE ((ipsitemchar_char_id=char_id) "
            "AND (ipsitemchar_ipsitem_id=:ipsitem_id)); ");
  q.bindValue(":ipsitem_id", _ipsitemid);
  q.exec();
  if (q.lastError().type() != QSqlError::NoError)
  {
	systemError(this, _rejectedMsg.arg(q.lastError().databaseText()),
                  __FILE__, __LINE__);
        done(-1);
  }
  _charprice->populate(q);
}

