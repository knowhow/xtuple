/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "creditMemoItem.h"

#include <QMessageBox>
#include <QSqlError>
#include <QValidator>
#include <QVariant>

#include "priceList.h"
#include "taxDetail.h"
#include "xdoublevalidator.h"

creditMemoItem::creditMemoItem(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

#ifndef Q_WS_MAC
  _listPrices->setMaximumWidth(25);
#endif

  connect(_discountFromSale, SIGNAL(lostFocus()), this, SLOT(sCalculateFromDiscount()));
  connect(_extendedPrice, SIGNAL(valueChanged()), this, SLOT(sCalculateTax()));
  connect(_item,	  SIGNAL(newId(int)),     this, SLOT(sPopulateItemInfo()));
  connect(_warehouse,	  SIGNAL(newID(int)),     this, SLOT(sPopulateItemsiteInfo()));
  connect(_listPrices,	  SIGNAL(clicked()),      this, SLOT(sListPrices()));
  connect(_netUnitPrice,  SIGNAL(valueChanged()), this, SLOT(sCalculateDiscountPrcnt()));
  connect(_netUnitPrice,  SIGNAL(valueChanged()), this, SLOT(sCalculateExtendedPrice()));
  connect(_netUnitPrice,  SIGNAL(idChanged(int)), this, SLOT(sPriceGroup()));
  connect(_qtyToCredit,	  SIGNAL(textChanged(const QString&)), this, SLOT(sCalculateExtendedPrice()));
  connect(_save,	  SIGNAL(clicked()),      this, SLOT(sSave()));
  connect(_taxLit, SIGNAL(leftClickedURL(const QString&)), this, SLOT(sTaxDetail()));
  connect(_taxType,	  SIGNAL(newID(int)),	  this, SLOT(sCalculateTax()));
  connect(_qtyUOM, SIGNAL(newID(int)), this, SLOT(sQtyUOMChanged()));
  connect(_pricingUOM, SIGNAL(newID(int)), this, SLOT(sPriceUOMChanged()));

  _mode = cNew;
  _cmitemid = -1;
  _cmheadid = -1;
  _custid = -1;
  _invoiceNumber = -1;
  _priceRatio = 1.0;
  _qtyShippedCache = 0.0;
  _shiptoid = -1;
  _taxzoneid	= -1;
  _qtyinvuomratio = 1.0;
  _priceinvuomratio = 1.0;
  _invuomid = -1;
  _saved = false;

  _qtyToCredit->setValidator(omfgThis->qtyVal());
  _qtyReturned->setValidator(omfgThis->qtyVal());
  _qtyShipped->setPrecision(omfgThis->qtyVal());
  _discountFromList->setPrecision(omfgThis->percentVal());
  _discountFromSale->setValidator(new XDoubleValidator(-9999, 100, 2, this));

  _taxType->setEnabled(_privileges->check("OverrideTax"));
  
  //If not multi-warehouse hide whs control
  if (!_metrics->boolean("MultiWhs"))
  {
    _warehouseLit->hide();
    _warehouse->hide();
  }
  
  adjustSize();
}

creditMemoItem::~creditMemoItem()
{
    // no need to delete child widgets, Qt does it all for us
}

void creditMemoItem::languageChange()
{
    retranslateUi(this);
}

enum SetResponse creditMemoItem::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  QVariant param;
  bool     valid;
  bool     vrestrict = FALSE;

  param = pParams.value("cmhead_id", &valid);
  if (valid)
  {
    _cmheadid = param.toInt();
    q.prepare("SELECT cmhead_cust_id, cmhead_shipto_id, "
			  "       cmhead_number, COALESCE(cmhead_invcnumber, '-1') AS cmhead_invcnumber, "
			  "       cmhead_docdate, cmhead_curr_id, "
			  "       cmhead_taxzone_id, cmhead_rsncode_id "
              "FROM cmhead "
              "WHERE (cmhead_id=:cmhead_id);");
    q.bindValue(":cmhead_id", _cmheadid);
    q.exec();
    if (q.first())
    {
      _custid = q.value("cmhead_cust_id").toInt();
      _shiptoid = q.value("cmhead_shipto_id").toInt();
      _orderNumber->setText(q.value("cmhead_number").toString());
	  _invoiceNumber = q.value("cmhead_invcnumber").toInt();
	  if ( (_invoiceNumber != -1) && (_metrics->boolean("RestrictCreditMemos")) )
        vrestrict = TRUE;
      _taxzoneid = q.value("cmhead_taxzone_id").toInt();
      _tax->setId(q.value("cmhead_curr_id").toInt());
      _tax->setEffective(q.value("cmhead_docdate").toDate());
      _netUnitPrice->setId(q.value("cmhead_curr_id").toInt());
      _netUnitPrice->setEffective(q.value("cmhead_docdate").toDate());
      _rsnCode->setId(q.value("cmhead_rsncode_id").toInt());
    }
    else if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return UndefinedError;
    }
  }

  param = pParams.value("cmitem_id", &valid);
  if (valid)
  {
    _cmitemid = param.toInt();
    populate();
  }

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      _mode = cNew;

      q.prepare( "SELECT (COALESCE(MAX(cmitem_linenumber), 0) + 1) AS n_linenumber "
                 "FROM cmitem "
                 "WHERE (cmitem_cmhead_id=:cmhead_id);" );
      q.bindValue(":cmhead_id", _cmheadid);
      q.exec();
      if (q.first())
        _lineNumber->setText(q.value("n_linenumber").toString());
      else if (q.lastError().type() == QSqlError::NoError)
      {
	      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
	      return UndefinedError;
      }

      connect(_discountFromSale, SIGNAL(lostFocus()), this, SLOT(sCalculateFromDiscount()));
      connect(_item, SIGNAL(valid(bool)), _listPrices, SLOT(setEnabled(bool)));

      _item->setFocus();
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;

      _item->setReadOnly(TRUE);
      _warehouse->setEnabled(FALSE);
      _qtyReturned->setFocus();

      connect(_discountFromSale, SIGNAL(lostFocus()), this, SLOT(sCalculateFromDiscount()));
      connect(_item, SIGNAL(valid(bool)), _listPrices, SLOT(setEnabled(bool)));

      _save->setFocus();
    }
    else if (param.toString() == "view")
    {
      _mode = cView;

      _item->setReadOnly(TRUE);
      _warehouse->setEnabled(FALSE);
      _qtyReturned->setEnabled(FALSE);
      _qtyToCredit->setEnabled(FALSE);
      _netUnitPrice->setEnabled(FALSE);
      _discountFromSale->setEnabled(FALSE);
      _comments->setReadOnly(TRUE);
      _taxType->setEnabled(FALSE);
      _rsnCode->setEnabled(FALSE);

      _save->hide();
      _close->setText(tr("&Close"));
      _close->setFocus();
    }
  }

  if (vrestrict)
    _item->setQuery( QString( "SELECT DISTINCT item_id, item_number,"
                              "                (item_descrip1 || ' ' || item_descrip2) AS itemdescrip,"
                              "                item_upccode,"
                              "                item_descrip1, item_descrip2,"
                              "                item_active, item_config, item_type, uom_name, item_upccode "
                              "FROM invchead, invcitem, item, uom "
                              "WHERE ( (invcitem_invchead_id=invchead_id)"
                              " AND (invcitem_item_id=item_id)"
                              " AND (item_inv_uom_id=uom_id)"
                              " AND (invchead_invcnumber='%1') ) "
                              "ORDER BY item_number" )
                     .arg(_invoiceNumber) );
  else
    _item->setQuery( QString( "SELECT DISTINCT item_id, item_number,"
                              "                (item_descrip1 || ' ' || item_descrip2) AS itemdescrip,"
                              "                item_upccode,"
                              "                item_descrip1, item_descrip2,"
                              "                item_active, uom_name, item_type, item_config "
                              "FROM item, itemsite, uom "
                              "WHERE ( (itemsite_item_id=item_id)"
                              " AND (item_inv_uom_id=uom_id)"
                              " AND (itemsite_active)"
                              " AND (item_active)"
                              " AND (customerCanPurchase(item_id, %1, %2)) ) "
                              "ORDER BY item_number" )
                     .arg(_custid).arg(_shiptoid) );

  return NoError;
}

void creditMemoItem::sSave()
{
  if (_qtyToCredit->toDouble() == 0.0)
  {
    QMessageBox::warning(this, tr("Invalid Credit Quantity"),
                         tr("<p>You have not selected a quantity of the "
			    "selected item to credit. If you wish to return a "
			    "quantity to stock but not issue a Credit Memo "
			    "then you should enter a Return to Stock "
			    "transaction from the I/M module.  Otherwise, you "
			    "must enter a quantity to credit." ));
    _qtyToCredit->setFocus();
    return;
  }

  if (_mode == cNew)
  {
    q.exec("SELECT NEXTVAL('cmitem_cmitem_id_seq') AS _cmitem_id");
    if (q.first())
      _cmitemid  = q.value("_cmitem_id").toInt();
    else if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }

    q.prepare( "INSERT INTO cmitem "
               "( cmitem_id, cmitem_cmhead_id, cmitem_linenumber, cmitem_itemsite_id,"
               "  cmitem_qtyreturned, cmitem_qtycredit, cmitem_updateinv,"
               "  cmitem_qty_uom_id, cmitem_qty_invuomratio,"
               "  cmitem_price_uom_id, cmitem_price_invuomratio,"
               "  cmitem_unitprice, cmitem_taxtype_id,"
               "  cmitem_comments, cmitem_rsncode_id ) "
               "SELECT :cmitem_id, :cmhead_id, :cmitem_linenumber, itemsite_id,"
               "       :cmitem_qtyreturned, :cmitem_qtycredit, :cmitem_updateinv,"
               "       :qty_uom_id, :qty_invuomratio,"
               "       :price_uom_id, :price_invuomratio,"
               "       :cmitem_unitprice, :cmitem_taxtype_id,"
	             "       :cmitem_comments, :cmitem_rsncode_id "
               "FROM itemsite "
               "WHERE ( (itemsite_item_id=:item_id)"
               " AND (itemsite_warehous_id=:warehous_id) );" );
  }
  else
    q.prepare( "UPDATE cmitem "
               "SET cmitem_qtyreturned=:cmitem_qtyreturned,"
               "    cmitem_qtycredit=:cmitem_qtycredit,"
               "    cmitem_updateinv=:cmitem_updateinv,"
               "    cmitem_qty_uom_id=:qty_uom_id,"
               "    cmitem_qty_invuomratio=:qty_invuomratio,"
               "    cmitem_price_uom_id=:price_uom_id,"
               "    cmitem_price_invuomratio=:price_invuomratio,"
               "    cmitem_unitprice=:cmitem_unitprice,"
               "    cmitem_taxtype_id=:cmitem_taxtype_id,"
               "    cmitem_comments=:cmitem_comments,"
               "    cmitem_rsncode_id=:cmitem_rsncode_id "
               "WHERE (cmitem_id=:cmitem_id);" );

  q.bindValue(":cmitem_id", _cmitemid);
  q.bindValue(":cmhead_id", _cmheadid);
  q.bindValue(":cmitem_linenumber", _lineNumber->text().toInt());
  q.bindValue(":cmitem_qtyreturned", _qtyReturned->toDouble());
  q.bindValue(":cmitem_qtycredit", _qtyToCredit->toDouble());
  q.bindValue(":cmitem_updateinv", QVariant(_updateInv->isChecked()));
  q.bindValue(":qty_uom_id", _qtyUOM->id());
  q.bindValue(":qty_invuomratio", _qtyinvuomratio);
  q.bindValue(":price_uom_id", _pricingUOM->id());
  q.bindValue(":price_invuomratio", _priceinvuomratio);
  q.bindValue(":cmitem_unitprice", _netUnitPrice->localValue());
  if (_taxType->isValid())
    q.bindValue(":cmitem_taxtype_id",	_taxType->id());
  q.bindValue(":cmitem_comments", _comments->toPlainText());
  q.bindValue(":cmitem_rsncode_id", _rsnCode->id());
  q.bindValue(":item_id", _item->id());
  q.bindValue(":warehous_id", _warehouse->id());
  q.exec();
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  done(_cmitemid);
}

void creditMemoItem::sPopulateItemInfo()
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
  _qtyUOM->populate(uom);
  _pricingUOM->populate(uom);

  XSqlQuery item;
  item.prepare( "SELECT item_inv_uom_id, item_price_uom_id,"
                "       iteminvpricerat(item_id) AS iteminvpricerat,"
                "       item_listprice, "
                "       stdCost(item_id) AS f_cost,"
		"       getItemTaxType(item_id, :taxzone) AS taxtype_id "
                "  FROM item"
                " WHERE (item_id=:item_id);" );
  item.bindValue(":item_id", _item->id());
  item.bindValue(":taxzone", _taxzoneid);
  item.exec();
  if (item.first())
  {
    _invuomid = item.value("item_inv_uom_id").toInt();
    _priceRatio = item.value("iteminvpricerat").toDouble();
    _qtyUOM->setId(item.value("item_inv_uom_id").toInt());
    _pricingUOM->setId(item.value("item_price_uom_id").toInt());
    _priceinvuomratio = item.value("iteminvpricerat").toDouble();
    _qtyinvuomratio = 1.0;
    _ratio=item.value("iteminvpricerat").toDouble();
    // {_listPrice,_unitCost}->setBaseValue() because they're stored in base
    _listPrice->setBaseValue(item.value("item_listprice").toDouble());
    _unitCost->setBaseValue(item.value("f_cost").toDouble());
    _taxType->setId(item.value("taxtype_id").toInt());
  }
  else if (item.lastError().type() != QSqlError::NoError)
  {
    systemError(this, item.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  if (_invoiceNumber != -1)
  {
    XSqlQuery cmitem;
    cmitem.prepare( "SELECT invcitem_warehous_id,"
                    "       invcitem_qty_uom_id, invcitem_qty_invuomratio,"
                    "       invcitem_price_uom_id, invcitem_price_invuomratio,"
                    "       invcitem_billed * invcitem_qty_invuomratio AS f_billed,"
                    "       currToCurr(invchead_curr_id, :curr_id, "
		    "                  invcitem_price / invcitem_price_invuomratio, invchead_invcdate) AS invcitem_price_local "
                    "FROM invchead, invcitem "
                    "WHERE ( (invcitem_invchead_id=invchead_id)"
                    " AND (invchead_invcnumber=text(:invoiceNumber))"
                    " AND (invcitem_item_id=:item_id) ) "
                    "LIMIT 1;" );
    cmitem.bindValue(":invoiceNumber", _invoiceNumber);
    cmitem.bindValue(":item_id", _item->id());
    cmitem.bindValue(":curr_id", _netUnitPrice->id());
    cmitem.exec();
    if (cmitem.first())
    {
      _qtyUOM->setId(cmitem.value("invcitem_qty_uom_id").toInt());
      _pricingUOM->setId(cmitem.value("invcitem_price_uom_id").toInt());
      _priceinvuomratio = cmitem.value("invcitem_price_invuomratio").toDouble();
      _ratio=_priceinvuomratio;
      _salePrice->setLocalValue(cmitem.value("invcitem_price_local").toDouble() * _priceinvuomratio);

      if (_mode == cNew)
        _netUnitPrice->setLocalValue(cmitem.value("invcitem_price_local").toDouble() * _priceinvuomratio);

      _warehouse->setId(cmitem.value("invcitem_warehous_id").toInt());
      _qtyShippedCache = cmitem.value("f_billed").toDouble();
      _qtyShipped->setDouble(cmitem.value("f_billed").toDouble() / _qtyinvuomratio);
    }
    else if (cmitem.lastError().type() != QSqlError::NoError)
    {
      systemError(this, cmitem.lastError().databaseText(), __FILE__, __LINE__);
      _salePrice->clear();
      return;
    }
  }
}

void creditMemoItem::sPopulateItemsiteInfo()
{
  XSqlQuery itemsite;
  itemsite.prepare( "SELECT itemsite_controlmethod"
                "  FROM itemsite"
                " WHERE ( (itemsite_item_id=:item_id)"
                "   AND   (itemsite_warehous_id=:warehous_id) );" );
  itemsite.bindValue(":item_id", _item->id());
  itemsite.bindValue(":warehous_id", _warehouse->id());
  itemsite.exec();
  if (itemsite.first())
  {
    if (itemsite.value("itemsite_controlmethod").toString() == "N")
    {
      _qtyReturned->setDouble(0.0);
      _qtyReturned->setEnabled(false);
      _updateInv->setChecked(false);
      _updateInv->setEnabled(false);
    }
    else
    {
      _qtyReturned->clear();
      _qtyReturned->setEnabled(true);
      _updateInv->setChecked(true);
      _updateInv->setEnabled(true);
    }
  }
  else if (itemsite.lastError().type() != QSqlError::NoError)
  {
    systemError(this, itemsite.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void creditMemoItem::populate()
{
  XSqlQuery cmitem;
  cmitem.prepare("SELECT cmitem.*,  "
                 "       cmhead_taxzone_id, cmhead_curr_id, "
                 "      (SELECT SUM(taxhist_tax * -1) "
				 "       FROM cmitemtax WHERE (cmitem_id=taxhist_parent_id)) AS tax "
                 "FROM cmhead, cmitem "
                 "WHERE ( (cmitem_cmhead_id=cmhead_id)"
                 "  AND   (cmitem_id=:cmitem_id) );" );
  cmitem.bindValue(":cmitem_id", _cmitemid);
  cmitem.exec();
  if (cmitem.first())
  { 
    _cmheadid = cmitem.value("cmitem_cmhead_id").toInt();
    _taxzoneid = cmitem.value("cmhead_taxzone_id").toInt();
    _rsnCode->setId(cmitem.value("cmitem_rsncode_id").toInt());
    _item->setItemsiteid(cmitem.value("cmitem_itemsite_id").toInt());
    _lineNumber->setText(cmitem.value("cmitem_linenumber").toString());
    _netUnitPrice->setLocalValue(cmitem.value("cmitem_unitprice").toDouble());
    _qtyToCredit->setDouble(cmitem.value("cmitem_qtycredit").toDouble());
    _qtyReturned->setDouble(cmitem.value("cmitem_qtyreturned").toDouble());
    if (cmitem.value("cmitem_raitem_id").toInt() > 0)
    {
      _updateInv->setChecked(false);
      _updateInv->setEnabled(false);
    }
    else
    {
      _updateInv->setChecked(cmitem.value("cmitem_updateinv").toBool());
      _updateInv->setEnabled(true);
    }
    _qtyUOM->setId(cmitem.value("cmitem_qty_uom_id").toInt());
    _ratio=cmitem.value("cmitem_qty_invuomratio").toDouble();
    _pricingUOM->setId(cmitem.value("cmitem_price_uom_id").toInt());
    _priceinvuomratio = cmitem.value("cmitem_price_invuomratio").toDouble();
    _comments->setText(cmitem.value("cmitem_comments").toString());
    _taxType->setId(cmitem.value("cmitem_taxtype_id").toInt());
    _tax->setId(cmitem.value("cmhead_curr_id").toInt());
    _tax->setLocalValue(cmitem.value("tax").toDouble());
    sCalculateDiscountPrcnt();
    _listPrices->setEnabled(true);
    _saved=true;
  }
  else if (cmitem.lastError().type() != QSqlError::NoError)
  {
    systemError(this, cmitem.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void creditMemoItem::sCalculateExtendedPrice()
{
  _extendedPrice->setLocalValue(((_qtyToCredit->toDouble() * _qtyinvuomratio) / _priceinvuomratio) * _netUnitPrice->localValue());
  sCalculateTax();
}

void creditMemoItem::sCalculateDiscountPrcnt()
{
  double unitPrice = _netUnitPrice->localValue();

  if (unitPrice == 0.0)
  {
    _discountFromList->setText("N/A");
    _discountFromSale->setText("N/A");
  }
  else
  {
    if (_listPrice->isZero())
      _discountFromList->setText("N/A");
    else
      _discountFromList->setDouble((1 - (unitPrice / _listPrice->localValue())) * 100);

    if (_salePrice->isZero())
      _discountFromSale->setText("N/A");
    else
      _discountFromSale->setDouble((1 - (unitPrice / _salePrice->localValue())) * 100);
  }
}

void creditMemoItem::sCalculateFromDiscount()
{
  double discount = _discountFromSale->toDouble() / 100.0;

  if (_salePrice->isZero())
    _discountFromSale->setText(tr("N/A"));
  else
    _netUnitPrice->setLocalValue(_salePrice->localValue() - (_salePrice->localValue() * discount));
}

void creditMemoItem::sPriceGroup()
{
  if (! omfgThis->singleCurrency())
    _priceGroup->setTitle(tr("In %1:").arg(_netUnitPrice->currAbbr()));
}

void creditMemoItem::sListPrices()
{
  q.prepare( "SELECT currToCurr(ipshead_curr_id, :curr_id, ipsprice_price, :effective) AS price"
             "       FROM ipsass, ipshead, ipsprice "
             "       WHERE ( (ipsass_ipshead_id=ipshead_id)"
             "        AND (ipsprice_ipshead_id=ipshead_id)"
             "        AND (ipsprice_item_id=:item_id)"
             "        AND (ipsass_cust_id=:cust_id)"
             "        AND (COALESCE(LENGTH(ipsass_shipto_pattern), 0) = 0)"
             "        AND (CURRENT_DATE BETWEEN ipshead_effective AND (ipshead_expires - 1) ) )"

             "       UNION SELECT ipsprice_price AS price"
             "       FROM ipsass, ipshead, ipsprice "
             "       WHERE ( (ipsass_ipshead_id=ipshead_id)"
             "        AND (ipsprice_ipshead_id=ipshead_id)"
             "        AND (ipsprice_item_id=:item_id)"
             "        AND (ipsass_shipto_id=:shipto_id)"
             "        AND (ipsass_shipto_id != -1)"
             "        AND (CURRENT_DATE BETWEEN ipshead_effective AND (ipshead_expires - 1)) )"
             
             "       UNION SELECT ipsprice_price AS price"
             "       FROM ipsass, ipshead, ipsprice, cust "
             "       WHERE ( (ipsass_ipshead_id=ipshead_id)"
             "        AND (ipsprice_ipshead_id=ipshead_id)"
             "        AND (ipsprice_item_id=:item_id)"
             "        AND (ipsass_custtype_id=cust_custtype_id)"
             "        AND (cust_id=:cust_id)"
             "        AND (CURRENT_DATE BETWEEN ipshead_effective AND (ipshead_expires - 1)) )"
             
             "       UNION SELECT ipsprice_price AS price"
             "       FROM ipsass, ipshead, ipsprice, custtype, cust "
             "       WHERE ( (ipsass_ipshead_id=ipshead_id)"
             "        AND (ipsprice_ipshead_id=ipshead_id)"
             "        AND (ipsprice_item_id=:item_id)"
             "        AND (coalesce(length(ipsass_custtype_pattern), 0) > 0)"
             "        AND (custtype_code ~ ipsass_custtype_pattern)"
             "        AND (cust_custtype_id=custtype_id)"
             "        AND (cust_id=:cust_id)"
             "        AND (CURRENT_DATE BETWEEN ipshead_effective AND (ipshead_expires - 1)))"
             
             "       UNION SELECT ipsprice_price AS price"
             "       FROM ipsass, ipshead, ipsprice, shipto "
             "       WHERE ( (ipsass_ipshead_id=ipshead_id)"
             "        AND (ipsprice_ipshead_id=ipshead_id)"
             "        AND (ipsprice_item_id=:item_id)"
             "        AND (shipto_id=:shipto_id)"
             "        AND (COALESCE(LENGTH(ipsass_shipto_pattern), 0) > 0)"
             "        AND (shipto_num ~ ipsass_shipto_pattern)"
             "        AND (ipsass_cust_id=:cust_id)"
             "        AND (CURRENT_DATE BETWEEN ipshead_effective AND (ipshead_expires - 1)) )"

             "       UNION SELECT ipsprice_price AS price"
             "       FROM sale, ipshead, ipsprice "
             "       WHERE ((sale_ipshead_id=ipshead_id)"
             "        AND (ipsprice_ipshead_id=ipshead_id)"
             "        AND (ipsprice_item_id=:item_id)"
             "        AND (CURRENT_DATE BETWEEN sale_startdate AND (sale_enddate - 1)) ) "

             "       UNION SELECT (item_listprice - (item_listprice * cust_discntprcnt)) AS price "
             "       FROM item, cust "
             "       WHERE ( (item_sold)"
             "        AND (NOT item_exclusive)"
             "        AND (item_id=:item_id)"
             "        AND (cust_id=:cust_id) );");
  q.bindValue(":item_id", _item->id());
  q.bindValue(":cust_id", _custid);
  q.bindValue(":shipto_id", _shiptoid);
  q.bindValue(":curr_id", _netUnitPrice->id());
  q.bindValue(":effective", _netUnitPrice->effective());
  q.exec();
  if (q.size() == 1)
  {
	q.first();
	_netUnitPrice->setLocalValue(q.value("price").toDouble() * (_priceinvuomratio / _priceRatio));
  }
  else
  {
    ParameterList params;
    params.append("cust_id", _custid);
    params.append("shipto_id", _shiptoid);
    params.append("item_id", _item->id());
    // don't params.append("qty", ...) as we don't know how many were purchased
    params.append("curr_id", _netUnitPrice->id());
    params.append("effective", _netUnitPrice->effective());

    priceList newdlg(this);
    newdlg.set(params);
    if (newdlg.exec() == XDialog::Accepted)
    {
      _netUnitPrice->setLocalValue(newdlg._selectedPrice * (_priceinvuomratio / _priceRatio));
      sCalculateDiscountPrcnt();
    }
  }
}

void creditMemoItem::sCalculateTax()
{
  _saved = false;
  XSqlQuery calcq;
  calcq.prepare( "SELECT calculateTax(cmhead_taxzone_id,:taxtype_id,cmhead_docdate,cmhead_curr_id,ROUND(:ext,2)) AS tax "
                 "FROM cmhead "
                 "WHERE (cmhead_id=:cmhead_id); "); 
  calcq.bindValue(":cmhead_id", _cmheadid);
  calcq.bindValue(":taxtype_id", _taxType->id());
  calcq.bindValue(":ext", _extendedPrice->localValue());
  calcq.exec();
  if (calcq.first())
    _tax->setLocalValue(calcq.value("tax").toDouble());
  else if (calcq.lastError().type() != QSqlError::NoError)
  {
    systemError(this, calcq.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void creditMemoItem::sTaxDetail()
{
  taxDetail newdlg(this, "", true);
  ParameterList params;
  params.append("taxzone_id", _taxzoneid);
  params.append("taxtype_id", _taxType->id());
  params.append("date", _netUnitPrice->effective());
  params.append("subtotal", _extendedPrice->localValue());
  params.append("curr_id",  _tax->id());
  params.append("sense", -1);
  
  if(cView == _mode)
    params.append("readOnly");
  
  if(_saved == true)
  {
    params.append("order_id", _cmitemid);
    params.append("order_type", "CI");
  }

  newdlg.set(params);
  
  if (newdlg.set(params) == NoError && newdlg.exec())
  {
    if (_taxType->id() != newdlg.taxtype())
      _taxType->setId(newdlg.taxtype());
  }
}

void creditMemoItem::sQtyUOMChanged()
{
  if(_qtyUOM->id() == _invuomid)
    _qtyinvuomratio = 1.0;
  else
  {
    XSqlQuery invuom;
    invuom.prepare("SELECT itemuomtouomratio(item_id, :uom_id, item_inv_uom_id) AS ratio"
                   "  FROM item"
                   " WHERE(item_id=:item_id);");
    invuom.bindValue(":item_id", _item->id());
    invuom.bindValue(":uom_id", _qtyUOM->id());
    invuom.exec();
    if(invuom.first())
      _qtyinvuomratio = invuom.value("ratio").toDouble();
    else
      systemError(this, invuom.lastError().databaseText(), __FILE__, __LINE__);
  }

  if(_qtyUOM->id() != _invuomid)
  {
    _pricingUOM->setId(_qtyUOM->id());
    _pricingUOM->setEnabled(false);
  }
  else
    _pricingUOM->setEnabled(true);
  sCalculateExtendedPrice();
}

void creditMemoItem::sPriceUOMChanged()
{
  if(_pricingUOM->id() == -1 || _qtyUOM->id() == -1)
    return;

  if(_pricingUOM->id() == _invuomid)
    _priceinvuomratio = 1.0;
  else
  {
    XSqlQuery invuom;
    invuom.prepare("SELECT itemuomtouomratio(item_id, :uom_id, item_inv_uom_id) AS ratio"
                   "  FROM item"
                   " WHERE(item_id=:item_id);");
    invuom.bindValue(":item_id", _item->id());
    invuom.bindValue(":uom_id", _pricingUOM->id());
    invuom.exec();
    if(invuom.first())
      _priceinvuomratio = invuom.value("ratio").toDouble();
    else
      systemError(this, invuom.lastError().databaseText(), __FILE__, __LINE__);
  }
  _ratio=_priceinvuomratio;

  updatePriceInfo();
}

void creditMemoItem::updatePriceInfo()
{
  XSqlQuery item;
  item.prepare("SELECT item_listprice"
               "  FROM item"
               " WHERE(item_id=:item_id);");
  item.bindValue(":item_id", _item->id());
  item.exec();
  item.first();
  _listPrice->setBaseValue(item.value("item_listprice").toDouble() * (_priceinvuomratio / _priceRatio));
}
