/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "freightBreakdown.h"

#include <QSqlError>
#include <QVariant>

#include <metasql.h>

freightBreakdown::freightBreakdown(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
  : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_close,         SIGNAL(clicked()),              this, SLOT(sSave()));

  _freight->addColumn(tr("Schedule"),           _itemColumn,     Qt::AlignLeft,   true, "freightdata_schedule");
  _freight->addColumn(tr("From"),               _itemColumn,     Qt::AlignLeft,   true, "freightdata_from");
  _freight->addColumn(tr("To"),                 _itemColumn,     Qt::AlignLeft,   true, "freightdata_to");
  _freight->addColumn(tr("Ship Via"),           _itemColumn,     Qt::AlignLeft,   true, "freightdata_shipvia");
  _freight->addColumn(tr("Freight Class"),      _itemColumn,     Qt::AlignLeft,   true, "freightdata_freightclass");
  _freight->addColumn(tr("Total Weight"),       _qtyColumn,      Qt::AlignRight,  true, "freightdata_weight");
  _freight->addColumn(tr("UOM"),                _uomColumn,      Qt::AlignCenter, true, "freightdata_uom");
  _freight->addColumn(tr("Price"),              _moneyColumn,    Qt::AlignRight,  true, "freightdata_price");
  _freight->addColumn(tr("Type"),               _itemColumn,     Qt::AlignLeft,   true, "freightdata_type");
  _freight->addColumn(tr("Total"),              _moneyColumn,    Qt::AlignRight,  true, "freightdata_total");
  _freight->addColumn(tr("Currency"),           _currencyColumn, Qt::AlignCenter, true, "freightdata_currency");
}

freightBreakdown::~freightBreakdown()
{
  // no need to delete child widgets, Qt does it all for us
}

void freightBreakdown::languageChange()
{
  retranslateUi(this);
}

SetResponse freightBreakdown::set(const ParameterList& pParams)
{
  XDialog::set(pParams);
  QVariant param;
  bool	   valid;

  ParameterList params;
  param = pParams.value("order_id", &valid);
  if (valid)
  {
    _orderid = param.toInt();
    params.append("order_id", _orderid);
  }

  param = pParams.value("cust_id", &valid);
  if (valid)
    params.append("cust_id", param.toInt());

  param = pParams.value("shipto_id", &valid);
  if (valid)
    params.append("shipto_id", param.toInt());

  param = pParams.value("orderdate", &valid);
  if (valid)
    params.append("orderdate", param.toDate());

  param = pParams.value("shipvia", &valid);
  if (valid)
    params.append("shipvia", param.toString());

  param = pParams.value("curr_id", &valid);
  if (valid)
    params.append("curr_id", param.toInt());

  param = pParams.value("document_number", &valid);
  if (valid)
    _document->setText(param.toString());

  param = pParams.value("order_type", &valid);
  if (valid)
  {
    _ordertype = param.toString();
    params.append("order_type", _ordertype);
    if (_ordertype == "SO")
      _header->setText(tr("Freight Breakdown for Sales Order:"));
    else if (_ordertype == "QU")
      _header->setText(tr("Freight Breakdown for Quote:"));
    else
      _header->setText(tr("Freight Breakdown for Return Auth.:"));
  }

  param = pParams.value("calcfreight", &valid);
  if (valid)
  {
    _calcfreight = param.toBool();
    if (_calcfreight == true)
      _calculated->setChecked(true);
    else
      _manual->setChecked(true);
  }

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if(param.toString() == "view")
      _mode = cView;
    else
      _mode = cEdit;
  }

  QString sql =	"SELECT *,"
                "       'weight' AS freightdata_weight_xtnumericrole,"
                "       'salesprice' AS freightdata_price_xtnumericrole,"
                "       'curr' AS freightdata_total_xtnumericrole,"
                "       0 AS freightdata_total_xttotalrole "
                "FROM freightDetail(<? value(\"order_type\") ?>,"
                "                   <? value(\"order_id\") ?>,"
                "                   <? value(\"cust_id\") ?>,"
                "                   <? value(\"shipto_id\") ?>,"
                "                   <? value(\"orderdate\") ?>,"
                "                   <? value(\"shipvia\") ?>,"
                "                   <? value(\"curr_id\") ?>);";

  MetaSQLQuery mql(sql);
  q = mql.toQuery(params);
  _freight->populate(q);
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return UndefinedError;
  }

  return NoError;
}

void freightBreakdown::sSave()
{
  XSqlQuery ord;

  if (_ordertype == "SO")
  {
    ord.prepare("UPDATE cohead "
                " SET cohead_calcfreight=:calc "
                "WHERE (cohead_id=:ordid); ");
  }
  else if (_ordertype == "QU")
  {
    ord.prepare("UPDATE quhead "
                " SET quhead_calcfreight=:calc "
                "WHERE (quhead_id=:ordid); ");
  }
  else
  {
    accept();
    return;
  }

  ord.bindValue(":ordid", _orderid);
  ord.bindValue(":calc", QVariant(_calculated->isChecked()));
  ord.exec();
  accept();
}

