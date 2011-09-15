/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "aropencluster.h"

#define DEBUG false

#define EXTRACLAUSE " (aropen_open) "	                        \
          " AND aropen_amount - aropen_paid - "	                \
          "(SELECT COALESCE(SUM(checkhead_amount),0) "	        \
          " FROM checkhead,checkitem "	                        \
          " WHERE ((checkhead_id=checkitem_checkhead_id) "	\
          " AND (NOT checkhead_posted) "	                \
          " AND (NOT checkhead_void) "	                        \
          " AND (checkitem_aropen_id=aropen_id))) > 0 "

AropenCluster::AropenCluster(QWidget *pParent, const char *pName) :
  VirtualCluster(pParent, pName)
{
  addNumberWidget(new AropenLineEdit(this, pName));
}

AropenLineEdit::DocTypes AropenCluster::allowedDocTypes()  const
{
  return (qobject_cast<AropenLineEdit*>(_number))->allowedDocTypes();
}

void AropenLineEdit::setExtraClause(const QString &clause)
{
  _userExtraClause = QString(clause);
}

void AropenCluster::setExtraClause(const QString &clause)
{
  (qobject_cast<AropenLineEdit*>(_number))->setExtraClause(clause);
}

AropenLineEdit::DocType  AropenCluster::type()             const
{
  return (qobject_cast<AropenLineEdit*>(_number))->type();
}

QString  AropenCluster::typeString()       const
{
  return (qobject_cast<AropenLineEdit*>(_number))->typeString();
}

void AropenCluster::setCustId(int pcustid)
{
  return (qobject_cast<AropenLineEdit*>(_number))->setCustId(pcustid);
}

void AropenCluster::setAllowedDocTypes(const AropenLineEdit::DocTypes ptypes)
{
  return (qobject_cast<AropenLineEdit*>(_number))->setAllowedDocTypes(ptypes);
}

AropenLineEdit::AropenLineEdit(QWidget *pParent, const char *pName) :
  VirtualClusterLineEdit(pParent, "aropen", "aropen_id", "aropen_docnumber", 
          "formatmoney(aropen_amount-aropen_paid- "
          
          //Subtract amount on existing checks
          "(SELECT COALESCE(SUM(checkhead_amount),0) "
          " FROM checkhead,checkitem "
          " WHERE ((checkhead_id=checkitem_checkhead_id) "
          " AND (NOT checkhead_posted) "
          " AND (NOT checkhead_void) "
          " AND (checkitem_aropen_id=aropen_id))) "
          
          ")"
          
          , QString("CASE WHEN (aropen_doctype='I') THEN '%1' "
                    "     WHEN (aropen_doctype='D') THEN '%2' "
                    "     WHEN (aropen_doctype='C') THEN '%3' "
                    "     WHEN (aropen_doctype='R') THEN '%4' "
                    "END ")
          .arg(tr("Invoice"))
          .arg(tr("Debit Memo"))
          .arg(tr("Credit Memo"))
          .arg(tr("Customer Deposit")).toAscii(),
          EXTRACLAUSE, pName)
{
  setAllowedDocTypes(AnyType);
  _custId       = -1;
  _standardExtraClause = EXTRACLAUSE;

  if (DEBUG)
    qDebug("%s::AropenLineEdit() _standardExtraClause = %s",
           qPrintable(objectName()), qPrintable(_standardExtraClause));

}

void AropenLineEdit::setCustId(int pCust)
{
  _custId = pCust;
  _custClause = QString("aropen_cust_id=%1").arg(_custId);
  (void)buildExtraClause();
}

AropenLineEdit::DocTypes AropenLineEdit::allowedDocTypes()  const
{
  return _allowedTypes;
}

void AropenLineEdit::setAllowedDocTypes(DocTypes ptypes)
{
  QStringList typelist;

  _allowedTypes = ptypes;

  if (ptypes == AnyType)
    _typeClause = "";
  else
  {
    if (_allowedTypes & CreditMemo)     typelist << "'C'";
    if (_allowedTypes & DebitMemo)      typelist << "'D'";
    if (_allowedTypes & Invoice)        typelist << "'I'";
    if (_allowedTypes & CashDeposit)    typelist << "'R'";

    _typeClause = "(aropen_doctype IN (" + typelist.join(", ") + "))" ;
  }

  switch (ptypes) 
  {
    case CreditMemo:  setTitles(tr("Credit Memo"),  tr("Credit Memos"));  break;
    case DebitMemo:   setTitles(tr("Debit Memo"),   tr("Debit Memos"));   break;
    case Invoice:     setTitles(tr("Invoice"),      tr("Invoices"));      break;
    case CashDeposit: setTitles(tr("Cash Deposit"), tr("Cash Deposits")); break;
    default:          setTitles(tr("A/R Open Item"),tr("A/R Open Items"));break;
  }

  (void)buildExtraClause();
}

AropenLineEdit::DocType AropenLineEdit::type() const
{
  if ("C" == _description) return CreditMemo;
  if ("D" == _description) return DebitMemo;
  if ("I" == _description) return Invoice;
  if ("R" == _description) return CashDeposit;

  return AnyType;
}

QString AropenLineEdit::typeString() const
{
  if ("C" == _description) return tr("Credit Memo");
  if ("D" == _description) return tr("Debit Memo");
  if ("I" == _description) return tr("Invoice");
  if ("R" == _description) return tr("Cash Deposit");

  return QString("A/R Open Item");
}

QString AropenLineEdit::buildExtraClause()
{
  QStringList clauses;
  if (! _custClause.isEmpty())          clauses << _custClause;
  if (! _typeClause.isEmpty())          clauses << _typeClause;
  if (! _standardExtraClause.isEmpty()) clauses << _standardExtraClause;
  if (! _userExtraClause.isEmpty())     clauses << _userExtraClause;

  VirtualClusterLineEdit::setExtraClause(clauses.join(" AND "));

  if (DEBUG)
    qDebug("%s::buildExtraClause() returning %s",
           qPrintable(objectName()), qPrintable(_extraClause));

  return _extraClause;
}
