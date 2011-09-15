/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "salesHistoryInformation.h"

#include <QVariant>
#include <QValidator>

salesHistoryInformation::salesHistoryInformation(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  // signals and slots connections
  connect(_close, SIGNAL(clicked()), this, SLOT(reject()));
  connect(_save, SIGNAL(clicked()), this, SLOT(sSave()));
  connect(_item, SIGNAL(newId(int)), _warehouse, SLOT(findItemsites(int)));
  connect(_item, SIGNAL(warehouseIdChanged(int)), _warehouse, SLOT(setId(int)));

  _orderNumber->setValidator(omfgThis->orderVal());
  _invoiceNumber->setValidator(omfgThis->orderVal());
  _shipped->setValidator(omfgThis->qtyVal());
  _unitPrice->setValidator(omfgThis->priceVal());
  _unitCost->setValidator(omfgThis->priceVal());
  _commission->setValidator(omfgThis->negMoneyVal());

  _extendedPrice->setPrecision(omfgThis->moneyVal());
  _extendedCost->setPrecision(omfgThis->moneyVal());

  _salesrep->setType(XComboBox::SalesReps);

  //If not multi-warehouse hide whs control
  if (!_metrics->boolean("MultiWhs"))
  {
    _warehouseLit->hide();
    _warehouse->hide();
  }
}

salesHistoryInformation::~salesHistoryInformation()
{
  // no need to delete child widgets, Qt does it all for us
}

void salesHistoryInformation::languageChange()
{
  retranslateUi(this);
}

enum SetResponse salesHistoryInformation::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("sohist_id", &valid);
  if (valid)
  {
    _sohistid = param.toInt();
    populate();
  }

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "edit")
    {
      _mode = cEdit;
      _orderNumber->setFocus();
    }
    else if (param.toString() == "view")
    {
      _mode = cView;

      _orderNumber->setEnabled(FALSE);
      _invoiceNumber->setEnabled(FALSE);
      _orderDate->setEnabled(FALSE);
      _invoiceDate->setEnabled(FALSE);
      _billtoName->setEnabled(FALSE);
      _billtoAddress1->setEnabled(FALSE);
      _billtoAddress2->setEnabled(FALSE);
      _billtoAddress3->setEnabled(FALSE);
      _billtoCity->setEnabled(FALSE);
      _billtoState->setEnabled(FALSE);
      _billtoZip->setEnabled(FALSE);
      _shiptoName->setEnabled(FALSE);
      _shiptoAddress1->setEnabled(FALSE);
      _shiptoAddress2->setEnabled(FALSE);
      _shiptoAddress3->setEnabled(FALSE);
      _shiptoCity->setEnabled(FALSE);
      _shiptoState->setEnabled(FALSE);
      _shiptoZip->setEnabled(FALSE);
      _shipped->setEnabled(FALSE);
      _unitPrice->setEnabled(FALSE);
      _unitCost->setEnabled(FALSE);
      _item->setReadOnly(TRUE);
      _warehouse->setEnabled(FALSE);
      _salesrep->setEnabled(FALSE);
      _commission->setEnabled(FALSE);
      _commissionPaid->setEnabled(FALSE);
      _close->setText(tr("&Close"));
      _save->hide();

      _close->setFocus();
    }
  }

  return NoError;
}

void salesHistoryInformation::sSave()
{
  q.prepare( "UPDATE cohist "
             "SET cohist_ordernumber=:cohist_ordernumber, cohist_invcnumber=:cohist_invcnumber,"
             "    cohist_orderdate=:cohist_orderdate, cohist_invcdate=:cohist_invcdate,"
             "    cohist_billtoname=:cohist_billtoname, cohist_billtoaddress1=:cohist_billtoaddress1,"
             "    cohist_billtoaddress2=cohist_billtoaddress2, cohist_billtoaddress3=:cohist_billtoaddress3,"
             "    cohist_billtocity=:cohist_billtocity, cohist_billtostate=:cohist_billtostate,"
             "    cohist_billtozip=:cohist_billtozip,"
             "    cohist_shiptoname=:cohist_shiptoname, cohist_shiptoaddress1=:cohist_shiptoaddress1,"
             "    cohist_shiptoaddress2=:cohist_shiptoaddress2, cohist_shiptoaddress3=:cohist_shiptoaddress3,"
             "    cohist_shiptocity=:cohist_shiptocity, cohist_shiptostate=:cohist_shiptostate,"
             "    cohist_shiptozip=:cohist_shiptozip,"
             "    cohist_itemsite_id=itemsite_id,"
             "    cohist_qtyshipped=:cohist_qtyshipped,"
             "    cohist_unitprice=:cohist_unitprice, cohist_unitcost=:cohist_unitcost,"
             "    cohist_salesrep_id=:cohist_salesrep_id, cohist_commission=:cohist_commission,"
             "    cohist_commissionpaid=:cohist_commissionpaid "
             "FROM itemsite "
             "WHERE ( (itemsite_item_id=:item_id)"
             " AND (itemsite_warehous_id=:warehous_id)"
             " AND (cohist_id=:cohist_id) );" );

  q.bindValue(":cohist_id", _sohistid);
  q.bindValue(":cohist_ordernumber", _orderNumber->text().toInt());
  q.bindValue(":cohist_invcnumber", _invoiceNumber->text().toInt());
  q.bindValue(":cohist_orderdate", _orderDate->date());
  q.bindValue(":cohist_invcdate", _invoiceDate->date());
  q.bindValue(":cohist_billtoname", _billtoName->text().trimmed());
  q.bindValue(":cohist_billtoaddress1", _billtoAddress1->text().trimmed());
  q.bindValue(":cohist_billtoaddress2", _billtoAddress2->text().trimmed());
  q.bindValue(":cohist_billtoaddress3", _billtoAddress3->text().trimmed());
  q.bindValue(":cohist_billtocity", _billtoCity->text().trimmed());
  q.bindValue(":cohist_billtostate", _billtoState->text().trimmed());
  q.bindValue(":cohist_billtozip", _billtoZip->text().trimmed());
  q.bindValue(":cohist_shiptoname", _shiptoName->text().trimmed());
  q.bindValue(":cohist_shiptoaddress1", _shiptoAddress1->text().trimmed());
  q.bindValue(":cohist_shiptoaddress2", _shiptoAddress2->text().trimmed());
  q.bindValue(":cohist_shiptoaddress3", _shiptoAddress3->text().trimmed());
  q.bindValue(":cohist_shiptocity", _shiptoCity->text().trimmed());
  q.bindValue(":cohist_shiptostate", _shiptoState->text().trimmed());
  q.bindValue(":cohist_shiptozip", _shiptoZip->text().trimmed());
  q.bindValue(":cohist_qtyshipped", _shipped->toDouble());
  q.bindValue(":cohist_unitprice", _unitPrice->toDouble());
  q.bindValue(":cohist_unitcost", _unitCost->toDouble());
  q.bindValue(":item_id", _item->id());
  q.bindValue(":warehous_id", _warehouse->id());
  q.bindValue(":cohist_salesrep_id", _salesrep->id());
  q.bindValue(":cohist_commission", _commission->toDouble());
  q.bindValue(":cohist_commissionpaid", QVariant(_commissionPaid->isChecked()));
  q.exec();

  done(_sohistid);
}

void salesHistoryInformation::populate()
{
  q.prepare( "SELECT cohist_ordernumber, cohist_invcnumber,"
             "       cohist_orderdate, cohist_invcdate,"
             "       cohist_billtoname, cohist_billtoaddress1,"
             "       cohist_billtoaddress2, cohist_billtoaddress3,"
             "       cohist_billtocity, cohist_billtostate, cohist_billtozip,"
             "       cohist_shiptoname, cohist_shiptoaddress1,"
             "       cohist_shiptoaddress2, cohist_shiptoaddress3,"
             "       cohist_shiptocity, cohist_shiptostate, cohist_shiptozip,"
             "       cohist_itemsite_id, cohist_salesrep_id, cohist_commissionpaid,"
             "       cohist_qtyshipped, cohist_unitprice, cohist_unitcost,"
             "       (cohist_qtyshipped * cohist_unitprice) AS extprice,"
             "       (cohist_qtyshipped * cohist_unitcost) AS extcost,"
             "       cohist_commission "
             "FROM cohist "
             "WHERE (cohist_id=:sohist_id);" );
  q.bindValue(":sohist_id", _sohistid);
  q.exec();
  if (q.first())
  {
    _orderNumber->setText(q.value("cohist_ordernumber"));
    _invoiceNumber->setText(q.value("cohist_invcnumber"));
    _orderDate->setDate(q.value("cohist_orderdate").toDate());
    _invoiceDate->setDate(q.value("cohist_invcdate").toDate());
    _billtoName->setText(q.value("cohist_billtoname"));
    _billtoAddress1->setText(q.value("cohist_billtoaddress1"));
    _billtoAddress2->setText(q.value("cohist_billtoaddress2"));
    _billtoAddress3->setText(q.value("cohist_billtoaddress3"));
    _billtoCity->setText(q.value("cohist_billtocity"));
    _billtoState->setText(q.value("cohist_billtostate"));
    _billtoZip->setText(q.value("cohist_billtozip"));
    _shiptoName->setText(q.value("cohist_shiptoname"));
    _shiptoAddress1->setText(q.value("cohist_shiptoaddress1"));
    _shiptoAddress2->setText(q.value("cohist_shiptoaddress2"));
    _shiptoAddress3->setText(q.value("cohist_shiptoaddress3"));
    _shiptoCity->setText(q.value("cohist_shiptocity"));
    _shiptoState->setText(q.value("cohist_shiptostate"));
    _shiptoZip->setText(q.value("cohist_shiptozip"));

    _item->setItemsiteid(q.value("cohist_itemsite_id").toInt());
    _shipped->setDouble(q.value("cohist_qtyshipped").toDouble());
    _unitPrice->setDouble(q.value("cohist_unitprice").toDouble());
    _unitCost->setDouble(q.value("cohist_unitcost").toDouble());
    _extendedPrice->setDouble(q.value("extprice").toDouble());
    _extendedCost->setDouble(q.value("extcost").toDouble());
    _salesrep->setId(q.value("cohist_salesrep_id").toInt());
    _commission->setDouble(q.value("cohist_commission").toDouble());
    _commissionPaid->setChecked(q.value("cohist_commissionpaid").toBool());
  }
}

