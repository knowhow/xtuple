/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "selectBillingQty.h"

#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include "storedProcErrorLookup.h"

selectBillingQty::selectBillingQty(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_item,	SIGNAL(newId(int)),	this, SLOT(sHandleItem()));
  connect(_save, SIGNAL(clicked()), this, SLOT(sSave()));
  connect(_toBill, SIGNAL(textChanged(const QString&)), this, SLOT(sHandleBillingQty()));

  _taxType->setEnabled(_privileges->check("OverrideTax"));
 
  _taxzoneid = -1;

  _toBill->setValidator(omfgThis->qtyVal());
  _ordered->setPrecision(omfgThis->qtyVal());
  _shipped->setPrecision(omfgThis->qtyVal());
  _balance->setPrecision(omfgThis->qtyVal());
  _uninvoiced->setPrecision(omfgThis->qtyVal());
}

selectBillingQty::~selectBillingQty()
{
  // no need to delete child widgets, Qt does it all for us
}

void selectBillingQty::languageChange()
{
  retranslateUi(this);
}

enum SetResponse selectBillingQty::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("taxzone_id", &valid);
  if (valid)
    _taxzoneid = param.toInt();
    
  param = pParams.value("soitem_id", &valid);
  if (valid)
  {
    _soitemid = param.toInt();

    _item->setReadOnly(TRUE);

    XSqlQuery soitem;
    soitem.prepare( "SELECT itemsite_item_id, cust_partialship,"
                    "       cohead_number, coitem_linenumber,"
                    "       uom_name,"
                    "       coitem_qtyord,"
                    "       coitem_qtyshipped,"
                    "       (coitem_qtyord - coitem_qtyshipped) AS qtybalance,"
                    "       COALESCE(coitem_taxtype_id, -1) AS taxtype_id "
                    "FROM coitem, itemsite, cohead, cust, uom "
                    "WHERE ( (coitem_itemsite_id=itemsite_id)"
                    " AND (coitem_cohead_id=cohead_id)"
                    " AND (coitem_status <> 'X')"
                    " AND (coitem_qty_uom_id=uom_id)"
                    " AND (cohead_cust_id=cust_id)"
                    " AND (coitem_id=:soitem_id) )" );
    soitem.bindValue(":soitem_id", _soitemid);
    soitem.exec();
    if (soitem.first())
    {
      _cachedBalanceDue = soitem.value("qtybalance").toDouble();

      _item->setId(soitem.value("itemsite_item_id").toInt());
      _salesOrderNumber->setText(soitem.value("cohead_number").toString());
      _lineNumber->setText(soitem.value("coitem_linenumber").toString());
      _qtyUOM->setText(soitem.value("uom_name").toString());
      _ordered->setDouble(soitem.value("coitem_qtyord").toDouble());
      _shipped->setDouble(soitem.value("coitem_qtyshipped").toDouble());
      _balance->setDouble(soitem.value("qtybalance").toDouble());               

      _cachedPartialShip = soitem.value("cust_partialship").toBool();
      _closeLine->setChecked(!_cachedPartialShip);
      _closeLine->setEnabled(_cachedPartialShip);

      double uninvoiced;
      XSqlQuery coship;
      coship.prepare( "SELECT SUM(coship_qty) AS uninvoiced "
                      "FROM cosmisc, coship "
                      "WHERE ( (coship_cosmisc_id=cosmisc_id)"
                      " AND (NOT coship_invoiced)"
                      " AND (cosmisc_shipped)"
                      " AND (coship_coitem_id=:soitem_id) );" );
      coship.bindValue(":soitem_id", _soitemid);
      coship.exec();
      if (coship.first())
      {
        uninvoiced = coship.value("uninvoiced").toDouble();
        _uninvoiced->setDouble(coship.value("uninvoiced").toDouble());
      }
      else
      {
        uninvoiced = 0.0;
        _uninvoiced->setDouble(0.0);
      }

      // take uninvoiced into account
      _cachedBalanceDue += uninvoiced;

      XSqlQuery cobill;
      cobill.prepare( "SELECT cobill_qty, cobill_toclose,"
		              "       cobill_taxtype_id " 
                      "FROM cobill, cobmisc "
                      "WHERE ((cobill_cobmisc_id=cobmisc_id) "
		              "      AND   (NOT cobmisc_posted)"
                      "      AND   (cobill_coitem_id=:soitem_id));" );
      cobill.bindValue(":soitem_id", _soitemid);
      cobill.exec();
      if (cobill.first())
      {
	_toBill->setDouble(cobill.value("cobill_qty").toDouble());

        if (soitem.value("cust_partialship").toBool())
	  _closeLine->setChecked(cobill.value("cobill_toclose").toBool());

	// overwrite automatically-found values if user previously set them
	_taxType->setId(cobill.value("cobill_taxtype_id").toInt());
   }
   else if (cobill.lastError().type() != QSqlError::NoError)
   {
	systemError(this, cobill.lastError().databaseText(), __FILE__, __LINE__);
	return UndefinedError;
   }
   else
   {
	_toBill->setDouble(uninvoiced);

   if (soitem.value("cust_partialship").toBool())
	  _closeLine->setChecked((uninvoiced == _cachedBalanceDue));
   }

      _toBill->setSelection(0, _toBill->text().length());
  }
  else if (soitem.lastError().type() != QSqlError::NoError)
  {
      systemError(this, soitem.lastError().databaseText(), __FILE__, __LINE__);
      return UndefinedError;
  }
 }

  return NoError;
}

void selectBillingQty::sHandleBillingQty()
{
  if (_cachedPartialShip)
    _closeLine->setChecked((_toBill->toDouble() == _cachedBalanceDue));
}

void selectBillingQty::sSave()
{
  XSqlQuery select;

  select.prepare("SELECT selectForBilling(:soitem_id, :qty, :close, "
		         "                        :taxtypeid ) AS result;");

  select.bindValue(":soitem_id", _soitemid);
  select.bindValue(":qty",	 _toBill->toDouble());
  select.bindValue(":close",	 QVariant(_closeLine->isChecked()));
  if(_taxType->isValid())
    select.bindValue(":taxtypeid", _taxType->id());
 
  select.exec();
  
  if(select.first())
  {
    int result = select.value("result").toInt();
    if (result < 0)
    {
      systemError(this, storedProcErrorLookup("selectForBilling", result),
		  __FILE__, __LINE__);
      return;
    }
  }
  else if (select.lastError().type() != QSqlError::NoError)
  {
    systemError(this, select.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  omfgThis->sBillingSelectionUpdated(-1, _soitemid);

  accept();
}

void selectBillingQty::sHandleItem()
{
  XSqlQuery itemq;
  itemq.prepare("SELECT getItemTaxType(:item_id, :taxzone) AS result;");
  itemq.bindValue(":item_id", _item->id());
  if (_taxzoneid != -1)
    itemq.bindValue(":taxzone", _taxzoneid);    
  itemq.exec();
  if (itemq.first())
  {
    int result = itemq.value("result").toInt();
    if (result < 0)
    {
      systemError(this, storedProcErrorLookup("getItemTaxType", result),
		  __FILE__, __LINE__);
      return;
    }
    _taxType->setId(result);
  }
  else if (itemq.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
  else
    _taxType->id(-1);
}
