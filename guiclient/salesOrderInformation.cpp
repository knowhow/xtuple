/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "salesOrderInformation.h"

#include <QSqlError>
#include <QVariant>

salesOrderInformation::salesOrderInformation(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  _soitemid = 0;
}

salesOrderInformation::~salesOrderInformation()
{
  // no need to delete child widgets, Qt does it all for us
}

void salesOrderInformation::languageChange()
{
  retranslateUi(this);
}

enum SetResponse salesOrderInformation::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("soitem_id", &valid);
  if (valid)
  {
    _soitemid = param.toInt();
    q.prepare( "SELECT coitem_cohead_id "
               "FROM coitem "
               "WHERE (coitem_id=:soitem_id);" );
    q.bindValue(":soitem_id", _soitemid);
    q.exec();
    if (q.first())
    {
      _soheadid = q.value("coitem_cohead_id").toInt();
      populate();
    }
    else if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return UndefinedError;
    }
  }

  param = pParams.value("sohead_id", &valid);
  if (valid)
  {
    _soheadid = param.toInt();
    populate();
  }

  return NoError;
}

void salesOrderInformation::populate()
{
  q.prepare( "SELECT cohead_number, warehous_code,"
             "       formatDate(cohead_orderdate) AS f_orderdate,"
             "       formatDate(MIN(coitem_scheddate)) AS f_shipdate,"
             "       formatDate(cohead_packdate) AS f_packdate,"
             "       CASE WHEN (cohead_holdtype='N') THEN :none"
             "            WHEN (cohead_holdtype='C') THEN :credit"
             "            WHEN (cohead_holdtype='S') THEN :ship"
             "            WHEN (cohead_holdtype='P') THEN :pack"
             "            WHEN (cohead_holdtype='R') THEN :return"
             "            ELSE :other"
             "       END AS f_holdtype,"
             "       cohead_shipvia, cohead_billtoname,"
             "       cohead_billtoaddress1, cohead_billtoaddress2, cohead_billtoaddress3,"
             "       cohead_billtocity, cohead_billtostate, cohead_billtozipcode,"
             "       cohead_shiptoname,"
             "       cohead_shiptoaddress1, cohead_shiptoaddress2, cohead_shiptoaddress3,"
             "       cohead_shiptocity, cohead_shiptostate, cohead_shiptozipcode "
             "FROM cohead, coitem, itemsite LEFT OUTER JOIN "
	     "     warehous ON (itemsite_warehous_id = warehous_id) "
             "WHERE ( (coitem_cohead_id=cohead_id)"
	     " AND (coitem_itemsite_id=itemsite_id) "
             " AND (coitem_status <> 'X')"
             " AND (coitem_id=:soitem_id) "
             " AND (cohead_id=:sohead_id) ) "
             "GROUP BY cohead_number, warehous_code, cohead_orderdate, cohead_packdate,"
             "         cohead_holdtype, cohead_shipvia, cohead_billtoname,"
             "         cohead_billtoaddress1, cohead_billtoaddress2, cohead_billtoaddress3,"
             "         cohead_billtocity, cohead_billtostate, cohead_billtozipcode,"
             "         cohead_shiptoname,"
             "         cohead_shiptoaddress1, cohead_shiptoaddress2, cohead_shiptoaddress3,"
             "         cohead_shiptocity, cohead_shiptostate, cohead_shiptozipcode;" );
  q.bindValue(":none",   tr("None"));
  q.bindValue(":credit", tr("Credit"));
  q.bindValue(":ship",   tr("Ship"));
  q.bindValue(":pack",   tr("Pack"));
  q.bindValue(":return", tr("Return"));
  q.bindValue(":other",  tr("Other"));
  q.bindValue(":sohead_id", _soheadid);
  q.bindValue(":soitem_id", _soitemid);
  q.exec();
  if (q.first())
  {
    _orderNumber->setText(q.value("cohead_number").toString());
    _warehouse->setText(q.value("warehous_code").toString());
    _orderDate->setText(q.value("f_orderdate").toString());
    _shipDate->setText(q.value("f_shipdate").toString());
    _packDate->setText(q.value("f_packdate").toString());
    _shipVia->setText(q.value("cohead_shipvia").toString());
    _holdType->setText(q.value("f_holdtype").toString());

    _billtoName->setText(q.value("cohead_billtoname").toString());
    _billtoAddress1->setText(q.value("cohead_billtoaddress1").toString());
    _billtoAddress2->setText(q.value("cohead_billtoaddress2").toString());
    _billtoAddress3->setText(q.value("cohead_billtoaddress3").toString());
    _billtoCity->setText(q.value("cohead_billtocity").toString());
    _billtoState->setText(q.value("cohead_billtostate").toString());
    _billtoZipCode->setText(q.value("cohead_billtozipcode").toString());

    _shiptoName->setText(q.value("cohead_shiptoname").toString());
    _shiptoAddress1->setText(q.value("cohead_shiptoaddress1").toString());
    _shiptoAddress2->setText(q.value("cohead_shiptoaddress2").toString());
    _shiptoAddress3->setText(q.value("cohead_shiptoaddress3").toString());
    _shiptoCity->setText(q.value("cohead_shiptocity").toString());
    _shiptoState->setText(q.value("cohead_shiptostate").toString());
    _shiptoZipCode->setText(q.value("cohead_shiptozipcode").toString());
  }
}
