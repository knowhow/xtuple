/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include <xsqlquery.h>

#include "invoicelineedit.h"

InvoiceLineEdit::InvoiceLineEdit(QWidget *pParent, const char *name) :
  XLineEdit(pParent, name)
{
  _custid = -1;
  _coheadid = -1;
  _invoiceNumber = "";
  _id = -1;

  connect(this, SIGNAL(lostFocus()), this, SLOT(sParse()));
}

void InvoiceLineEdit::sParse()
{
  setInvoiceNumber(text());
}

void InvoiceLineEdit::setInvoiceNumber(QString pInvoiceNumber)
{
  XSqlQuery query;
  query.prepare( "SELECT cohead_id, invchead_cust_id, invchead_id "
                 "FROM invchead LEFT OUTER JOIN cohead ON (invchead_ordernumber=cohead_number) "
                 "WHERE (invchead_invcnumber=:invoiceNumber);" );
  query.bindValue(":invoiceNumber", pInvoiceNumber);
  query.exec();
  if (query.first())
  {
    _custid = query.value("invchead_cust_id").toInt();
    _coheadid = query.value("cohead_id").toInt();
    _invoiceNumber = pInvoiceNumber;
    _id = query.value("invchead_id").toInt();
    _valid = TRUE;

    setText(_invoiceNumber);
  }
  else
  {
    _custid = -1;
    _coheadid = -1;
    _invoiceNumber = "";
    _id = -1;
    _valid = FALSE;

    setText("");
  }

  emit newCustid(_custid);
  emit newCoheadid(_coheadid);
  emit newInvoiceNumber(_invoiceNumber);
  emit newId(_id);
  emit valid(_valid);
}

void InvoiceLineEdit::setId(const int pId)
{
  XSqlQuery query;
  query.prepare( "SELECT cohead_id, invchead_cust_id, invchead_invcnumber "
                 "FROM invchead LEFT OUTER JOIN cohead ON (invchead_ordernumber=cohead_number) "
                 "WHERE (invchead_id=:id);" );
  query.bindValue(":id", pId);
  query.exec();
  if (query.first())
  {
    _custid = query.value("invchead_cust_id").toInt();
    _coheadid = query.value("cohead_id").toInt();
    _invoiceNumber = query.value("invchead_invcnumber").toString();
    _id = pId;
    _valid = TRUE;

    setText(_invoiceNumber);
  }
  else
  {
    _custid = -1;
    _coheadid = -1;
    _invoiceNumber = "";
    _id = -1;
    _valid = FALSE;

    setText("");
  }

  emit newCustid(_custid);
  emit newCoheadid(_coheadid);
  emit newInvoiceNumber(_invoiceNumber);
  emit newId(_id);
  emit valid(_valid);
}
