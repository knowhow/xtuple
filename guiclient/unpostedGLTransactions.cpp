/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "unpostedGLTransactions.h"

#include <QAction>
#include <QMenu>
#include <QSqlError>
#include <QVariant>

#include <openreports.h>

#include "voucher.h"
#include "miscVoucher.h"
#include "invoice.h"
#include "purchaseOrder.h"
#include "glTransactionDetail.h"

unpostedGLTransactions::unpostedGLTransactions(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_gltrans, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*)), this, SLOT(sPopulateMenu(QMenu*, QTreeWidgetItem*)));
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));

  _gltrans->addColumn(tr("Date"),      _dateColumn,    Qt::AlignCenter, true,  "gltrans_date" );
  _gltrans->addColumn(tr("Source"),    _orderColumn,   Qt::AlignCenter, true,  "gltrans_source" );
  _gltrans->addColumn(tr("Doc. Type"), _docTypeColumn, Qt::AlignCenter, true,  "gltrans_doctype" );
  _gltrans->addColumn(tr("Doc. #"),    _orderColumn,   Qt::AlignCenter, true,  "docnumber" );
  _gltrans->addColumn(tr("Reference"), -1,             Qt::AlignLeft,   true,  "reference"   );
  _gltrans->addColumn(tr("Account"),   _itemColumn,    Qt::AlignLeft,   true,  "account"   );
  _gltrans->addColumn(tr("Debit"),     _moneyColumn,   Qt::AlignRight,  true,  "debit"  );
  _gltrans->addColumn(tr("Credit"),    _moneyColumn,   Qt::AlignRight,  true,  "credit"  );
  _gltrans->addColumn(tr("Posted"),    _ynColumn,      Qt::AlignCenter, true,  "gltrans_posted" );

  _periodid = -1;
}

unpostedGLTransactions::~unpostedGLTransactions()
{
    // no need to delete child widgets, Qt does it all for us
}

void unpostedGLTransactions::languageChange()
{
    retranslateUi(this);
}

enum SetResponse unpostedGLTransactions::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("period_id", &valid);
  if (valid)
  {
    _periodid = param.toInt();
    q.prepare( "SELECT period_name, period_start, period_end "
               "FROM period "
               "WHERE (period_id=:period_id);" );
    q.bindValue(":period_id", _periodid);
    q.exec();
    if (q.first())
    {
      _period->setText(q.value("period_name").toString());
      _dates->setStartDate(q.value("period_start").toDate());
      _dates->setEndDate(q.value("period_end").toDate());
    }
  }

  sFillList();

  return NoError;
}

void unpostedGLTransactions::sPopulateMenu(QMenu * menuThis, QTreeWidgetItem* pItem)
{
  menuThis->addAction(tr("View..."), this, SLOT(sViewTrans()));

  XTreeWidgetItem * item = (XTreeWidgetItem*)pItem;
  if(0 == item)
    return;

  if(item->text(2) == "VO")
    menuThis->addAction(tr("View Voucher..."), this, SLOT(sViewDocument()));
  else if(item->text(2) == "IN")
    menuThis->addAction(tr("View Invoice..."), this, SLOT(sViewDocument()));
  else if(item->text(2) == "PO")
    menuThis->addAction(tr("View Purchase Order..."), this, SLOT(sViewDocument()));
}

void unpostedGLTransactions::sPrint()
{
  ParameterList params;
  params.append("period_id", _periodid);

  orReport report("UnpostedGLTransactions", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void unpostedGLTransactions::sFillList()
{
  QString sql( "SELECT gltrans_id, formatDate(gltrans_date), gltrans_source, gltrans_doctype,"
               "       CASE WHEN(gltrans_docnumber='Misc.' AND invhist_docnumber IS NOT NULL) THEN"
               "              (gltrans_docnumber || ' - ' || invhist_docnumber)"
               "            ELSE gltrans_docnumber"
               "       END AS docnumber,"
               "       firstLine(gltrans_notes) AS reference,"
               "       (formatGLAccount(accnt_id) || ' - ' || accnt_descrip) AS account,"
               "       CASE WHEN (gltrans_amount < 0) THEN ABS(gltrans_amount)"
               "            ELSE 0"
               "       END AS debit,"
               "       CASE WHEN (gltrans_amount > 0) THEN (gltrans_amount)"
               "            ELSE 0"
               "       END AS credit,"
               "       gltrans_posted, gltrans_username,"
               "       'curr' AS debit_xtnumericrole,"
               "       'curr' AS credit_xtnumericrole,"
               "       CASE WHEN (gltrans_amount < 0) THEN '' END AS credit_qtdisplayrole,"
               "       CASE WHEN (gltrans_amount >= 0) THEN '' END AS debit_qtdisplayrole "
               "FROM period, gltrans JOIN accnt ON (gltrans_accnt_id=accnt_id) "
               "     LEFT OUTER JOIN invhist ON (gltrans_misc_id=invhist_id AND"
	       "                                 gltrans_docnumber='Misc.') "
               "WHERE ((NOT gltrans_posted) "
               "  AND  (NOT gltrans_deleted) "
	       "  AND  (gltrans_date BETWEEN period_start AND period_end)"
	       "  AND  (period_id=:period_id)) "
	       "ORDER BY gltrans_created, gltrans_sequence, gltrans_amount;");

  q.prepare(sql);
  q.bindValue(":period_id", _periodid);
  q.exec();
  _gltrans->populate(q);
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void unpostedGLTransactions::sViewTrans()
{
  ParameterList params;

  params.append("gltrans_id", _gltrans->id());

  glTransactionDetail newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void unpostedGLTransactions::sViewDocument()
{
  QTreeWidgetItem * item = (XTreeWidgetItem*)_gltrans->currentItem();
  if(0 == item)
    return;

  ParameterList params;
  if(item->text(2) == "VO")
  {
    q.prepare("SELECT vohead_id, vohead_misc"
              "  FROM vohead"
              " WHERE (vohead_number=:vohead_number)");
    q.bindValue(":vohead_number", item->text(3));
    q.exec();
    if(!q.first())
      return;

    params.append("vohead_id", q.value("vohead_id").toInt());
    params.append("mode", "view");

    if(q.value("vohead_misc").toBool())
    {
      miscVoucher *newdlg = new miscVoucher();
      newdlg->set(params);
      omfgThis->handleNewWindow(newdlg);
    }
    else
    {
      voucher *newdlg = new voucher();
      newdlg->set(params);
      omfgThis->handleNewWindow(newdlg);
    }
  }
  else if(item->text(2) == "IN")
  {
    q.prepare("SELECT invchead_id"
              "  FROM invchead"
              " WHERE (invchead_invcnumber=:invchead_invcnumber)");
    q.bindValue(":invchead_invcnumber", item->text(3));
    q.exec();
    if(!q.first())
      return;

    invoice::viewInvoice(q.value("invchead_id").toInt());
  }
  else if(item->text(2) == "PO")
  {
    q.prepare("SELECT pohead_id"
              "  FROM pohead"
              " WHERE (pohead_number=:pohead_number)");
    q.bindValue(":pohead_number", item->text(3));
    q.exec();
    if(!q.first())
      return;

    params.append("pohead_id", q.value("pohead_id").toInt());
    params.append("mode", "view");

    purchaseOrder *newdlg = new purchaseOrder();
    newdlg->set(params);
    omfgThis->handleNewWindow(newdlg);
  }
}
