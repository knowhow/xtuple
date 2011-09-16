/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "invoiceList.h"

#include <QVariant>

invoiceList::invoiceList(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
  : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  // signals and slots connections
  connect(_close, SIGNAL(clicked()), this, SLOT(sClose()));
  connect(_select, SIGNAL(clicked()), this, SLOT(sSelect()));
  connect(_invoice, SIGNAL(itemSelected(int)), this, SLOT(sSelect()));
  connect(_cust, SIGNAL(newId(int)), this, SLOT(sFillList()));
  connect(_dates, SIGNAL(updated()), this, SLOT(sFillList()));

  QDate today(omfgThis->dbDate());
  _dates->setEndDate(today);
  _dates->setStartDate(today.addMonths(-1));

  _invoice->addColumn(tr("Invoice #"),    _orderColumn, Qt::AlignRight, true, "invchead_invcnumber" );
  _invoice->addColumn(tr("Invoice Date"), _dateColumn,  Qt::AlignCenter, true, "invchead_invcdate" );
  _invoice->addColumn(tr("S/O #"),        _orderColumn, Qt::AlignRight, true, "invchead_ordernumber"  );
  _invoice->addColumn(tr("Ship Date"),    _dateColumn,  Qt::AlignCenter, true, "invchead_shipdate" );
  _invoice->addColumn(tr("Cust. P/O #"),  -1,           Qt::AlignLeft, true, "invchead_ponumber"   );
}

invoiceList::~invoiceList()
{
  // no need to delete child widgets, Qt does it all for us
}

void invoiceList::languageChange()
{
  retranslateUi(this);
}

enum SetResponse invoiceList::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("invoiceNumber", &valid);
  if (valid)
    _invoiceNumber = param.toString();
  else
    _invoiceNumber = "";

  param = pParams.value("cust_id", &valid);
  if (valid)
    _cust->setId(param.toInt());
  else if (! _invoiceNumber.isEmpty())
  {
    q.prepare( "SELECT invchead_id, invchead_cust_id "
               "FROM invchead "
               "WHERE (invchead_invcnumber=:invoiceNumber) ;" );
    q.bindValue(":invoiceNumber", _invoiceNumber);
    q.exec();
    if (q.first())
    {
      _invoiceid = q.value("invchead_id").toInt();
      _cust->setId(q.value("invchead_cust_id").toInt());
    }
  }

  return NoError;
}

void invoiceList::sClose()
{
  done(_invoice->id());
}

void invoiceList::sSelect()
{
  done(_invoice->id());
}

void invoiceList::sFillList()
{
  q.prepare( "SELECT DISTINCT invchead_id, invchead_invcnumber, invchead_invcdate,"
             "                invchead_ordernumber, invchead_shipdate,"
             "                COALESCE(invchead_ponumber, '') AS invchead_ponumber "
             "FROM invchead "
             "WHERE ( (invchead_posted)"
             "  AND   (invchead_invcdate BETWEEN :startDate AND :endDate)"
             " AND (invchead_cust_id=:cust_id) ) "
             "ORDER BY invchead_invcnumber DESC;" );
  q.bindValue(":cust_id", _cust->id());
  _dates->bindValue(q);
  q.exec();
  _invoice->populate(q, _invoiceid);
}

