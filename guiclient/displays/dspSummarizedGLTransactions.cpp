/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspSummarizedGLTransactions.h"

#include <QAction>
#include <QMenu>
#include <QMessageBox>
#include <QVariant>

#include "guiclient.h"
#include "glTransactionDetail.h"
#include "voucher.h"
#include "miscVoucher.h"
#include "invoice.h"
#include "purchaseOrder.h"

dspSummarizedGLTransactions::dspSummarizedGLTransactions(QWidget* parent, const char*, Qt::WFlags fl)
  : display(parent, "dspSummarizedGLTransactions", fl)
{
  setupUi(optionsWidget());
  setWindowTitle(tr("Summarized General Ledger Transactions"));
  setListLabel(tr("Transactions"));
  setReportName("SummarizedGLTransactions");
  setMetaSQLOptions("summarizedGLTransactions", "detail");
  setUseAltId(true);

  connect(_selectedSource, SIGNAL(toggled(bool)), _source, SLOT(setEnabled(bool)));

  list()->setRootIsDecorated(true);
  list()->addColumn(tr("Account #"),         150,               Qt::AlignCenter, true,  "account" );
  list()->addColumn(tr("Date"),              _dateColumn,       Qt::AlignCenter, true,  "gltrans_date" );
  list()->addColumn(tr("Description/Notes"), -1,                Qt::AlignLeft,   true,  "descrip_notes"   );
  list()->addColumn(tr("Src."),              _whsColumn,        Qt::AlignCenter, true,  "gltrans_source" );
  list()->addColumn(tr("Doc. Type"),         _docTypeColumn,    Qt::AlignCenter, true,  "gltrans_doctype" );
  list()->addColumn(tr("Doc. #"),            _orderColumn,      Qt::AlignCenter, true,  "gltrans_docnumber" );
  list()->addColumn(tr("Debit"),             _bigMoneyColumn,   Qt::AlignRight,  true,  "debit"  );
  list()->addColumn(tr("Credit"),            _bigMoneyColumn,   Qt::AlignRight,  true,  "credit"  );
  list()->addColumn( tr("Username"),         _userColumn,       Qt::AlignLeft,   true,  "gltrans_username" );
}

void dspSummarizedGLTransactions::languageChange()
{
  display::languageChange();
  retranslateUi(this);
}

void dspSummarizedGLTransactions::sPopulateMenu(QMenu * menuThis, QTreeWidgetItem*, int)
{
  if(list()->altId() == -1)
    return;

  menuThis->addAction(tr("View..."), this, SLOT(sViewTrans()));

  QTreeWidgetItem * item = list()->currentItem();
  if(0 == item)
    return;

  if(item->text(4) == "VO")
    menuThis->addAction(tr("View Voucher..."), this, SLOT(sViewDocument()));
  else if(item->text(4) == "IN")
    menuThis->addAction(tr("View Invoice..."), this, SLOT(sViewDocument()));
  else if(item->text(4) == "PO")
    menuThis->addAction(tr("View Purchase Order..."), this, SLOT(sViewDocument()));
}

bool dspSummarizedGLTransactions::setParams(ParameterList & params)
{
  if (_dates->allValid())
  {
    params.append("startDate", _dates->startDate());
    params.append("endDate", _dates->endDate());
  }

  if (_selectedSource->isChecked())
    params.append("source", _source->currentText());

  if (_unpostedTransactions->isChecked())
    params.append("unpostedTransactions");
  else if (_postedTransactions->isChecked())
    params.append("postedTransactions");

  params.append("showUsernames"); // report only?

  return true;
}

void dspSummarizedGLTransactions::sViewTrans()
{
  ParameterList params;

  params.append("gltrans_id", list()->altId());

  glTransactionDetail newdlg(this, "", true);
  newdlg.set(params);
  newdlg.exec();
}

void dspSummarizedGLTransactions::sViewDocument()
{
  QTreeWidgetItem * item = list()->currentItem();
  if(0 == item)
    return;

  ParameterList params;
  if(item->text(4) == "VO")
  {
    q.prepare("SELECT vohead_id, vohead_misc "
              "  FROM vohead"
              " WHERE (vohead_number=:vohead_number)");
    q.bindValue(":vohead_number", item->text(5));
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
  else if(item->text(4) == "IN")
  {
    q.prepare("SELECT invchead_id"
              "  FROM invchead"
              " WHERE (invchead_invcnumber=:invchead_invcnumber)");
    q.bindValue(":invchead_invcnumber", item->text(5));
    q.exec();
    if(!q.first())
      return;

    invoice::viewInvoice(q.value("invchead_id").toInt());
  }
  else if(item->text(4) == "PO")
  {
    q.prepare("SELECT pohead_id"
              "  FROM pohead"
              " WHERE (pohead_number=:pohead_number)");
    q.bindValue(":pohead_number", item->text(5));
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
