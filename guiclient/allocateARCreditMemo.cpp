/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "allocateARCreditMemo.h"

#include <QSqlError>
#include <QVariant>
#include <QMessageBox>
#include "metasql.h"
#include "mqlutil.h"

allocateARCreditMemo::allocateARCreditMemo(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);
  
  _aropen->setRootIsDecorated(true);
  _aropen->addColumn(tr("Doc. Type"),                   -1, Qt::AlignLeft,   true,  "doctype");
  _aropen->addColumn(tr("Doc. #"),            _orderColumn, Qt::AlignLeft,   true,  "docnumber");
  _aropen->addColumn(tr("Doc. Date"),          _dateColumn, Qt::AlignCenter, true,  "docdate");
  _aropen->addColumn(tr("Due Date"),           _dateColumn, Qt::AlignCenter, true,  "duedate");
  _aropen->addColumn(tr("Amount"),            _moneyColumn, Qt::AlignRight,  true,  "amount");
  _aropen->addColumn(tr("Paid"),              _moneyColumn, Qt::AlignRight,  true,  "paid");
  _aropen->addColumn(tr("Balance"),           _moneyColumn, Qt::AlignRight,  true,  "balance");
  _aropen->addColumn(tr("This Alloc."),       _moneyColumn, Qt::AlignRight,  true,  "allocated");
  _aropen->addColumn(tr("Total Alloc."),      _moneyColumn, Qt::AlignRight,  true,  "totalallocated");

  connect(omfgThis, SIGNAL(creditMemosUpdated()), this, SLOT(sPopulate()));

  if (omfgThis->singleCurrency())
  {
    _aropen->hideColumn("currAbbr");
    _aropen->hideColumn("balance");
  }

  connect(_aropen,      SIGNAL(valid(bool)),    this,           SLOT(sHandleButton()));
  connect(_allocate,	SIGNAL(clicked()),	this,           SLOT(sAllocate()));
  connect(_cancel,	SIGNAL(clicked()),	this,           SLOT(sCancel()));

  _coheadid = -1;
  _invcheadid = -1;
  
}

allocateARCreditMemo::~allocateARCreditMemo()
{
    // no need to delete child widgets, Qt does it all for us
}

void allocateARCreditMemo::languageChange()
{
    retranslateUi(this);
}

enum SetResponse allocateARCreditMemo::set(const ParameterList & pParams )
{
  XDialog::set(pParams);
  QVariant param;
  bool     valid;
  
   param = pParams.value("effective", &valid);
   if (valid)
   {
      _total->setEffective(param.toDate());
      _balance->setEffective(param.toDate());
   }

   param = pParams.value("curr_id", &valid);
   if (valid)
   {
      _total->setId(param.toInt());
      _balance->setId(param.toInt());
   }

   param = pParams.value("total", &valid);
   if (valid)
     _total->setLocalValue(param.toDouble());
    
   param = pParams.value("balance", &valid);
   if (valid)
     _balance->setLocalValue(param.toDouble());

   param = pParams.value("cust_id", &valid);
   if (valid)
    _custid = param.toInt();

   param = pParams.value("cohead_id", &valid);
   if (valid)
    _coheadid = param.toInt();

   param = pParams.value("invchead_id", &valid);
   if (valid)
    _invcheadid = param.toInt();

   param = pParams.value("doctype", &valid);
   if (valid)
    _doctype = param.toString();

   sPopulate();

  return NoError;
}

void allocateARCreditMemo::sCancel()
{
    accept();
}

void allocateARCreditMemo::sPopulate()
{
  if (_coheadid == -1)
  {
    // get the cohead_id associated with the invoice
    q.prepare("SELECT cohead_id "
              "  FROM invchead JOIN cohead ON (cohead_number=invchead_ordernumber)"
              " WHERE (invchead_id=:invchead_id);");
    q.bindValue(":invchead_id", _invcheadid);
    q.exec();
    if (q.first())
      _coheadid = q.value("cohead_id").toInt();
    if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }
  // Get the list of Unallocated CM's with amount
  q.prepare("SELECT aropen_id, "
            "       docnumber, doctype, docdate, duedate,"
            "       amount, paid, balance, allocated, totalallocated, "
            "       CASE WHEN (doctype='C') THEN :creditmemo"
            "            WHEN (doctype='R') THEN :cashdeposit"
            "            WHEN (doctype='S') THEN :salesorder"
            "            WHEN (doctype='I') THEN :invoice"
            "       END AS doctype_qtdisplayrole, "
            "       indent AS xtindentrole "
            "  FROM ( "
            "SELECT aropen_id, 0 AS indent, "
            "       aropen_docnumber AS docnumber, aropen_doctype AS doctype, "
            "       aropen_docdate AS docdate, aropen_duedate AS duedate,"
            "       currToCurr(aropen_curr_id, :curr_id, aropen_amount, :effective) AS amount,"
            "       currToCurr(aropen_curr_id, :curr_id, aropen_paid, :effective) AS paid,"
            "       currToCurr(aropen_curr_id, :curr_id, noNeg(aropen_amount - aropen_paid), :effective) AS balance,"
            "       COALESCE(SUM(CASE WHEN ((aropenalloc_doctype='S' AND aropenalloc_doc_id=:cohead_id) OR "
            "                               (aropenalloc_doctype='I' AND aropenalloc_doc_id=:invchead_id)) THEN "
            "                         currToCurr(aropenalloc_curr_id, :curr_id, aropenalloc_amount, :effective)"
            "                         ELSE 0.00 END), 0.0) AS allocated,"
            "       COALESCE(SUM(currToCurr(aropenalloc_curr_id, :curr_id, aropenalloc_amount, :effective)), 0.0) AS totalallocated "
            "  FROM aropen LEFT OUTER JOIN aropenalloc ON (aropenalloc_aropen_id=aropen_id)"
            " WHERE ( (aropen_cust_id=:cust_id)"
            "   AND   (aropen_doctype IN ('C', 'R'))"
            "   AND   (aropen_open) )"
            " GROUP BY aropen_id, aropen_docnumber, aropen_doctype, aropen_docdate, aropen_duedate,"
            "          aropen_curr_id, aropen_amount, aropen_paid "
            "UNION "
            "SELECT aropen_id, 1 AS indent, "
            "       cohead_number AS docnumber, 'S' AS doctype, "
            "       cohead_orderdate AS docdate, NULL AS duedate,"
            "       NULL AS amount,"
            "       NULL AS paid,"
            "       NULL AS balance,"
            "       NULL AS allocated,"
            "       COALESCE(currToCurr(aropenalloc_curr_id, :curr_id, aropenalloc_amount, :effective), 0.0) AS totalallocated "
            "  FROM aropen JOIN aropenalloc ON (aropenalloc_aropen_id=aropen_id AND aropenalloc_doctype='S')"
            "              JOIN cohead ON (cohead_id=aropenalloc_doc_id) "
            " WHERE ( (aropen_cust_id=:cust_id)"
            "   AND   (aropen_doctype IN ('C', 'R'))"
            "   AND   (aropen_open) ) "
            "UNION "
            "SELECT aropen_id, 1 AS indent, "
            "       invchead_invcnumber AS docnumber, 'I' AS doctype, "
            "       invchead_invcdate AS docdate, NULL AS duedate,"
            "       NULL AS amount,"
            "       NULL AS paid,"
            "       NULL AS balance,"
            "       NULL AS allocated,"
            "       COALESCE(currToCurr(aropenalloc_curr_id, :curr_id, aropenalloc_amount, :effective), 0.0) AS totalallocated "
            "  FROM aropen JOIN aropenalloc ON (aropenalloc_aropen_id=aropen_id AND aropenalloc_doctype='I')"
            "              JOIN invchead ON (invchead_id=aropenalloc_doc_id) "
            " WHERE ( (aropen_cust_id=:cust_id)"
            "   AND   (aropen_doctype IN ('C', 'R'))"
            "   AND   (aropen_open) )"
            " ) AS data "
            " ORDER BY aropen_id, indent, duedate;");
  q.bindValue(":cohead_id", _coheadid);
  q.bindValue(":invchead_id", _invcheadid);
  q.bindValue(":cust_id", _custid);
  q.bindValue(":curr_id",   _total->id());
  q.bindValue(":effective", _total->effective());
  q.bindValue(":creditmemo", tr("Credit Memo"));
  q.bindValue(":cashdeposit", tr("Customer Deposit"));
  q.bindValue(":salesorder", tr("Sales Order"));
  q.bindValue(":invoice", tr("Invoice"));
  q.exec();
  if (q.lastError().type() != QSqlError::NoError)
  {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
  _aropen->populate(q, false);
}

void allocateARCreditMemo::sAllocate()
{
  if (_allocateMode && _balance->localValue() > 0.0)
  {
    double amount = _aropen->rawValue("balance").toDouble() - _aropen->rawValue("totalallocated").toDouble();
    if (amount > _balance->localValue()) // make sure we don't apply more to a credit memo than we have left.
      amount = _balance->localValue();
    XSqlQuery allocCM;
    allocCM.prepare("INSERT INTO aropenalloc"
                    "      (aropenalloc_aropen_id, aropenalloc_doctype, aropenalloc_doc_id, "
                    "       aropenalloc_amount, aropenalloc_curr_id)"
                    "VALUES(:aropen_id, :doctype, :doc_id, :amount, :curr_id);");
    allocCM.bindValue(":doctype", _doctype);
    if (_doctype == "S")
      allocCM.bindValue(":doc_id", _coheadid);
    else
      allocCM.bindValue(":doc_id", _invcheadid);
    allocCM.bindValue(":aropen_id", _aropen->id());
    allocCM.bindValue(":amount", amount);
    allocCM.bindValue(":curr_id", _total->id());
    allocCM.exec();
    if (!allocCM.lastError().type() == QSqlError::NoError)
      systemError(this, allocCM.lastError().databaseText(), __FILE__, __LINE__);
    double newbalance = _balance->localValue() - amount;
    _balance->setLocalValue(newbalance);
  }
  else
  {
    double amount = _aropen->rawValue("allocated").toDouble();
    XSqlQuery allocCM;
    allocCM.prepare("DELETE FROM aropenalloc "
                    "WHERE ((aropenalloc_aropen_id=:aropen_id)"
                    "  AND  ((aropenalloc_doctype='S' AND aropenalloc_doc_id=:cohead_id) OR"
                    "        (aropenalloc_doctype='I' AND aropenalloc_doc_id=:invchead_id)));");
    allocCM.bindValue(":cohead_id", _coheadid);
    allocCM.bindValue(":invchead_id", _invcheadid);
    allocCM.bindValue(":aropen_id", _aropen->id());
    allocCM.exec();
    if (!allocCM.lastError().type() == QSqlError::NoError)
      systemError(this, allocCM.lastError().databaseText(), __FILE__, __LINE__);
    double newbalance = _balance->localValue() + amount;
    _balance->setLocalValue(newbalance);
  }
  sPopulate();
}

void allocateARCreditMemo::sHandleButton()
{
  if (_aropen->rawValue("allocated").toDouble() == 0.0 && _balance->localValue() &&
      (_aropen->rawValue("balance").toDouble() - _aropen->rawValue("totalallocated").toDouble()) > 0.0)
  {
    // Allocate mode
    _allocateMode = true;
    _allocate->setText("Allocate");
    _allocate->setEnabled(true);
  }
  else if (_aropen->rawValue("allocated").toDouble() > 0.0)
  {
    // Deallocate mode
    _allocateMode = false;
    _allocate->setText("Clear");
    _allocate->setEnabled(true);
  }
  else
    _allocate->setEnabled(false);
}

