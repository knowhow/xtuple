/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "unappliedARCreditMemos.h"

#include <QVariant>
//#include <QStatusBar>
#include <QMenu>
#include <parameter.h>
#include <openreports.h>
#include "applyARCreditMemo.h"
#include "arOpenItem.h"
/*
 *  Constructs a unappliedARCreditMemos as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
unappliedARCreditMemos::unappliedARCreditMemos(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

//  (void)statusBar();

  // signals and slots connections
  connect(_aropen, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*)));
  connect(_close, SIGNAL(clicked()), this, SLOT(close()));
  connect(_aropen, SIGNAL(valid(bool)), _view, SLOT(setEnabled(bool)));
  connect(_new, SIGNAL(clicked()), this, SLOT(sNew()));
  connect(_view, SIGNAL(clicked()), this, SLOT(sView()));
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_apply, SIGNAL(clicked()), this, SLOT(sApply()));

  _new->setEnabled(_privileges->check("MaintainARMemos"));

  connect(_aropen, SIGNAL(itemSelected(int)), _view, SLOT(animateClick()));

  _aropen->addColumn( tr("Doc. #"),   _itemColumn,     Qt::AlignCenter, true,  "aropen_docnumber" );
  _aropen->addColumn( tr("Customer"), -1,              Qt::AlignLeft,   true,  "customer"   );
  _aropen->addColumn( tr("Amount"),   _moneyColumn,    Qt::AlignRight,  true,  "aropen_amount"  );
  _aropen->addColumn( tr("Applied"),  _moneyColumn,    Qt::AlignRight,  true,  "applied"  );
  _aropen->addColumn( tr("Balance"),  _moneyColumn,    Qt::AlignRight,  true,  "balance"  );
  _aropen->addColumn( tr("Currency"), _currencyColumn, Qt::AlignLeft,   true,  "currAbbr" );

  if (omfgThis->singleCurrency())
    _aropen->hideColumn(5);

  if (_privileges->check("ApplyARMemos"))
    connect(_aropen, SIGNAL(valid(bool)), _apply, SLOT(setEnabled(bool)));

  sFillList();
}

/*
 *  Destroys the object and frees any allocated resources
 */
unappliedARCreditMemos::~unappliedARCreditMemos()
{
  // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void unappliedARCreditMemos::languageChange()
{
  retranslateUi(this);
}

void unappliedARCreditMemos::sPrint()
{
  ParameterList params;

  orReport report("UnappliedARCreditMemos", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}


void unappliedARCreditMemos::sNew()
{
  ParameterList params;
  params.append("mode", "new");
  params.append("docType", "creditMemo");

  arOpenItem newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void unappliedARCreditMemos::sView()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("aropen_id", _aropen->id());

  arOpenItem newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void unappliedARCreditMemos::sPopulateMenu( QMenu * )
{
}

void unappliedARCreditMemos::sFillList()
{
  q.prepare( "SELECT aropen_id, aropen_docnumber,"
             "       (cust_number || '-' || cust_name) AS customer,"
             "       aropen_amount,"
             "       (aropen_paid + COALESCE(prepared,0.0) + COALESCE(cashapplied,0.0)) AS applied,"
             "       (aropen_amount - aropen_paid - COALESCE(prepared,0.0) - COALESCE(cashapplied,0.0)) AS balance, "
             "       currConcat(aropen_curr_id) AS currAbbr,"
             "       'curr' AS aropen_amount_xtnumericrole,"
             "       'curr' AS applied_xtnumericrole,"
             "       'curr' AS balance_xtnumericrole "
             "FROM aropen "
             "       LEFT OUTER JOIN (SELECT aropen_id AS prepared_aropen_id,"
             "                               SUM(checkitem_amount + checkitem_discount) AS prepared"
             "                          FROM checkhead JOIN checkitem ON (checkitem_checkhead_id=checkhead_id)"
             "                                     JOIN aropen ON (checkitem_aropen_id=aropen_id)"
             "                         WHERE ((NOT checkhead_posted)"
             "                           AND  (NOT checkhead_void))"
             "                         GROUP BY aropen_id ) AS sub1"
             "         ON (prepared_aropen_id=aropen_id)"
             "       LEFT OUTER JOIN (SELECT aropen_id AS cash_aropen_id,"
             "                               SUM(cashrcptitem_amount + cashrcptitem_discount) * -1.0 AS cashapplied"
             "                          FROM cashrcpt JOIN cashrcptitem ON (cashrcptitem_cashrcpt_id=cashrcpt_id)"
             "                                     JOIN aropen ON (cashrcptitem_aropen_id=aropen_id)"
             "                         WHERE (NOT cashrcpt_posted)"
             "                         GROUP BY aropen_id ) AS sub2"
             "         ON (cash_aropen_id=aropen_id)"
             ", cust "
             "WHERE ( (aropen_doctype IN ('C', 'R'))"
             " AND (aropen_open)"
             " AND ((aropen_amount - aropen_paid - COALESCE(prepared,0.0) - COALESCE(cashapplied,0.0)) > 0.0)"
             " AND (aropen_cust_id=cust_id) ) "
             "ORDER BY aropen_docnumber;" );
  q.exec();
  _aropen->populate(q);
}

void unappliedARCreditMemos::sApply()
{
  ParameterList params;
  params.append("aropen_id", _aropen->id());

  applyARCreditMemo newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

