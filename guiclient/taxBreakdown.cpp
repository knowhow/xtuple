/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "taxBreakdown.h"

#include <QSqlError>
#include <QVariant>

#include <metasql.h>
#include "mqlutil.h"

#include "taxCache.h"
#include "taxDetail.h"

taxBreakdown::taxBreakdown(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
  : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  _currency->setLabel(_currencyLit);
  _taxcurrency->setLabel(_taxcurrencyLit);

  connect(_adjTaxLit,    SIGNAL(leftClickedURL(QString)),this, SLOT(sAdjTaxDetail()));
  connect(_freightTaxLit,SIGNAL(leftClickedURL(QString)),this, SLOT(sFreightTaxDetail()));
  connect(_lineTaxLit,   SIGNAL(leftClickedURL(QString)),this, SLOT(sLineTaxDetail()));
  connect(_totalTaxLit,  SIGNAL(leftClickedURL(QString)),this, SLOT(sTotalTaxDetail()));

  _orderid	= -1;
  _ordertype	= "";
  _sense        = 1;
}

taxBreakdown::~taxBreakdown()
{
  // no need to delete child widgets, Qt does it all for us
}

void taxBreakdown::languageChange()
{
  retranslateUi(this);
}

SetResponse taxBreakdown::set(const ParameterList& pParams)
{
  XDialog::set(pParams);
  QVariant param;
  bool	   valid;

  param = pParams.value("order_id", &valid);
  if (valid)
    _orderid = param.toInt();

  param = pParams.value("order_type", &valid);
  if (valid)
    _ordertype = param.toString();

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if(param.toString() == "view")
      _mode = cView;
    else
      _mode = cEdit;
  }
  
   param = pParams.value("sense", &valid);
   if (valid)
    _sense = param.toInt();

  sPopulate();

  return NoError;
}

void taxBreakdown::sAdjTaxDetail()
{
  taxDetail newdlg(this, "", true);
  ParameterList params;

  params.append("curr_id", _taxcurrency->id());
  params.append("date",    _adjTax->effective());
  if (cView == _mode)
    params.append("readOnly");

  q.exec("SELECT getadjustmenttaxtypeid() as taxtype;");
	if(q.first())
	 params.append("taxtype_id", q.value("taxtype").toInt());  
   
  params.append("order_type", _ordertype);
  params.append("order_id", _orderid);
  params.append("display_type", "A");
  params.append("adjustment");
  params.append("sense", _sense);
  if (newdlg.set(params) == NoError)  
  {
	  newdlg.exec();
	  sPopulate();
  }
}

void taxBreakdown::sFreightTaxDetail()
{
  taxDetail newdlg(this, "", true);
  ParameterList params;

   params.append("curr_id", _taxcurrency->id());
   params.append("date",    _freight->effective());
   params.append("subtotal",CurrDisplay::convert(_freight->id(),
						_taxcurrency->id(),
						_freight->localValue(),
						_freight->effective()));
   params.append("sense", _sense);

  if (_ordertype == "S" || _ordertype == "Q" || _ordertype == "RA")
  {
   params.append("taxzone_id",  _taxzone->id());

   q.exec("SELECT getfreighttaxtypeid() as taxtype;");
	 if(q.first())
		params.append("taxtype_id", q.value("taxtype").toInt());  
  
   params.append("date",    _freight->effective());
  
   params.append("readOnly");
   if (newdlg.set(params) == NoError) 
	   newdlg.exec();
     
 }
 else if (_ordertype == "I" || _ordertype == "B" || _ordertype == "CM" || _ordertype == "TO")
 {
   q.exec("SELECT getfreighttaxtypeid() as taxtype;");
	 if(q.first())
	   params.append("taxtype_id", q.value("taxtype").toInt());  
   params.append("order_type", _ordertype);
   params.append("order_id", _orderid);
   params.append("display_type", "F");
   params.append("readOnly");
   if (newdlg.set(params) == NoError) 
	   newdlg.exec();  
 }
}

void taxBreakdown::sLineTaxDetail()
{
  taxDetail newdlg(this, "", true);
  ParameterList params;

  params.append("order_type", _ordertype);
  params.append("order_id", _orderid);
  params.append("display_type", "L");
  params.append("curr_id", _taxcurrency->id());
  params.append("date",    _line->effective());
  params.append("subtotal",CurrDisplay::convert(_line->id(),
						_taxcurrency->id(),
						_line->localValue(),
						_line->effective()));
  params.append("readOnly");
  params.append("sense", _sense);

  if (newdlg.set(params) == NoError)
    newdlg.exec();
}

void taxBreakdown::sTotalTaxDetail()
{
  taxDetail newdlg(this, "", true);
  ParameterList params;
  params.append("order_type", _ordertype);
  params.append("order_id", _orderid);
  params.append("display_type", "T");
  params.append("curr_id", _taxcurrency->id());
  params.append("date",    _total->effective());
  params.append("subtotal",CurrDisplay::convert(_pretax->id(),
						_taxcurrency->id(),
						_pretax->localValue(),
						_pretax->effective()));
  params.append("readOnly");
  params.append("sense", _sense);

  if (newdlg.set(params) == NoError)
    newdlg.exec();
}

void taxBreakdown::sPopulate()
{
  ParameterList params;
  if (_ordertype == "I")
  {
    _currencyLit->setText(tr("Invoice Currency:"));
    _header->setText(tr("Tax Breakdown for Invoice:"));
    _totalLit->setText(tr("Invoice Total:"));

    params.append("invchead_id", _orderid);
  }
  else if (_ordertype == "S")
  {
    _currencyLit->setText(tr("Sales Order Currency:"));
    _header->setText(tr("Tax Breakdown for Sales Order:"));
    _totalLit->setText(tr("Sales Order Total:"));

    _adjTaxLit->setVisible(false);
    _adjTax->setVisible(false);

    params.append("cohead_id", _orderid);
  }
  else if (_ordertype == "Q")
  {
    _currencyLit->setText(tr("Quote Currency:"));
    _header->setText(tr("Tax Breakdown for Quote:"));
    _totalLit->setText(tr("Quote Total:"));

    _adjTaxLit->setVisible(false);
    _adjTax->setVisible(false);

    params.append("quhead_id", _orderid);
  }
  else if (_ordertype == "RA")
  {
    _currencyLit->setText(tr("Return Authorization Currency:"));
    _header->setText(tr("Tax Breakdown for Return:"));
    _totalLit->setText(tr("Return Total:"));

    _adjTaxLit->setVisible(false);
    _adjTax->setVisible(false);

    params.append("rahead_id", _orderid);
  }
  else if (_ordertype == "B")
  {
    _currencyLit->setText(tr("Billing Currency:"));
    _header->setText(tr("Tax Breakdown for Billing Order:"));
    _totalLit->setText(tr("Billing Total:"));

    params.append("cobmisc_id", _orderid);
  }
  else if (_ordertype == "CM")
  {
    _currencyLit->setText(tr("Credit Memo Currency:"));
    _header->setText(tr("Tax Breakdown for Credit Memo:"));
    _totalLit->setText(tr("Credit Memo Total:"));

    params.append("cmhead_id", _orderid);
  }
  else if (_ordertype == "TO")
  {
    _currencyLit->setText(tr("Transfer Order Currency:"));
    _header->setText(tr("Tax Breakdown for Transfer Order:"));
    _totalLit->setText(tr("Transfer Order Total:"));
    _adjTaxLit->setVisible(false);
    _adjTax->setVisible(false);
    _freightTaxLit->setVisible(false);
    _freightTax->setVisible(false);

    params.append("tohead_id", _orderid);
  }
  else if (_ordertype == "PO")
  {
    _currencyLit->setText(tr("Purchase Order Currency:"));
    _header->setText(tr("Tax Breakdown for Purchase Order:"));
    _totalLit->setText(tr("Purchase Order Total:"));

    _adjTaxLit->setVisible(false);
    _adjTax->setVisible(false);
	  _freightTaxLit->setVisible(false);
    _freightTax->setVisible(false);
    params.append("pohead_id", _orderid);
  }
   else if (_ordertype == "VO")
  {
    _currencyLit->setText(tr("Voucher Currency:"));
    _header->setText(tr("Tax Breakdown for Voucher:"));
    _totalLit->setText(tr("Voucher Total:"));

	_freightTaxLit->setVisible(false);
    _freightTax->setVisible(false);

    params.append("vohead_id", _orderid);
  }



  MetaSQLQuery mql = mqlLoad("taxBreakdown", "detail");
  q = mql.toQuery(params);
  if (q.first())
  {
    // do dates and currencies first because of signal/slot cascades
    _total->setEffective(q.value("date").toDate());
    _currency->setId(q.value("curr_id").toInt());
    _taxcurrency->setId(q.value("tax_curr_id").toInt());

    // now the rest
    _document->setText(q.value("number").toString());
    _taxzone->setId(q.value("taxzone_id").toInt());
    _line->setLocalValue(q.value("line").toDouble());
    _lineTax->setLocalValue(q.value("total_tax").toDouble());
    _freight->setLocalValue(q.value("freight").toDouble());
    _freightTax->setLocalValue(q.value("freighttaxamt").toDouble());
    _adjTax->setLocalValue(q.value("adjtaxamt").toDouble());
    _pretax->setLocalValue(_line->localValue() + _freight->localValue());
    _totalTax->setLocalValue(_lineTax->localValue() + _freightTax->localValue() + _adjTax->localValue());
    _total->setLocalValue(_pretax->localValue() + _totalTax->localValue());
  }
  else if (q.lastError().type() != QSqlError::NoError)
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
}
