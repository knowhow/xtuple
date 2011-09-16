/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its postJournals.
 */

#include "postJournals.h"
#include "dspJournals.h"

#include <metasql.h>
#include <openreports.h>
#include "mqlutil.h"

#include <QAction>
#include <QMessageBox>
#include <QSqlError>

postJournals::postJournals(QWidget* parent, const char* name, Qt::WFlags fl)
  : XWidget(parent, name, fl)
{
  setupUi(this);

  _journalDates->setStartNull(tr("Earliest"), omfgThis->startOfTime(), true);
  _journalDates->setEndNull(tr("Latest"), omfgThis->endOfTime(), true);
  _distDate->setDate(omfgThis->dbDate());

  connect(_post, SIGNAL(clicked()), this, SLOT(sPost()));
  connect(_query, SIGNAL(clicked()), this, SLOT(sFillList()));
  connect(_selectAll, SIGNAL(clicked()), _sources, SLOT(selectAll()));
  connect(_preview, SIGNAL(toggled(bool)), this, SLOT(sHandlePreview()));
  connect(_sources, SIGNAL(valid(bool)), this, SLOT(sHandleSelection()));
  connect(_sources, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*)), this, SLOT(sPopulateMenu(QMenu*,QTreeWidgetItem*)));

  _sources->addColumn(tr("Source"), _docTypeColumn, Qt::AlignLeft, true, "sltrans_source");
  _sources->addColumn(tr("Description"), -1, Qt::AlignLeft, true, "description");
  sHandlePreview();

  sFillList();
}

postJournals::~postJournals()
{
}

void postJournals::languageChange()
{
  retranslateUi(this);
}

void postJournals::sPost()
{
  QStringList sources;
  QList<int> journalnumbers;
  XSqlQuery qry;
  MetaSQLQuery mql = mqlLoad("postJournals", "post");
  QList<XTreeWidgetItem*> selected = _sources->selectedItems();
  for (int i = 0; i < selected.size(); i++)
    sources << selected.at(i)->rawValue("sltrans_source").toString();

  ParameterList params;
  _journalDates->appendValue(params);
  params.append("distDate", _distDate->date());
  params.append("source_list", sources);

  XSqlQuery jrnls;
  jrnls = mql.toQuery(params);
  if (jrnls.lastError().type() != QSqlError::NoError)
  {
    systemError(this, jrnls.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  while (jrnls.next())
    journalnumbers << jrnls.value("result").toInt();

  if (_print->isChecked() && journalnumbers.size())
    sPrint(journalnumbers);

  sFillList();
}

void postJournals::sFillList()
{
  MetaSQLQuery mql = mqlLoad("postJournals", "detail");
  ParameterList params;
  _journalDates->appendValue(params);
  params.append("AP", tr("Accounts Payable"));
  params.append("AR", tr("Accounts Receivable"));
  params.append("GL", tr("General Ledger"));
  params.append("IM", tr("Inventory Management"));
  params.append("PD", tr("Products"));
  params.append("PO", tr("Purchase Order"));
  params.append("SO", tr("Sales Order"));
  params.append("SR", tr("Shipping and Receiving"));
  params.append("WO", tr("Work Order"));
  params.append("Other", tr("Other"));
  if (_preview->isChecked())
    params.append("preview");

  XSqlQuery qry;
  qry = mql.toQuery(params);
  _sources->populate(qry, true);
  _sources->expandAll();
  if (qry.lastError().type() != QSqlError::NoError)
  {
    systemError(this, qry.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void postJournals::sHandlePreview()
{
  _sources->clear();
  _sources->setColumnCount(2);
  if (_preview->isChecked())
  {
    _sources->addColumn(tr("Debit"), _bigMoneyColumn, Qt::AlignRight, true, "debit");
    _sources->addColumn(tr("Credit"),_bigMoneyColumn, Qt::AlignRight, true, "credit");
    _sources->setSelectionMode(QAbstractItemView::SingleSelection);
    _selectAll->setEnabled(false);
  }
  else
  {
    _sources->setSelectionMode(QAbstractItemView::ExtendedSelection);
    _selectAll->setEnabled(true);
  }

  _sources->addColumn(tr("Entries"), _qtyColumn, Qt::AlignRight, true, "journals");
}

void postJournals::sHandleSelection()
{
  _post->setEnabled(_sources->id() == 0);
}

void postJournals::sPopulateMenu(QMenu *pMenu, QTreeWidgetItem *)
{
  QAction *menuItem;
  if (_sources->id() == 0)
  {
    menuItem = pMenu->addAction(tr("Post"), this, SLOT(sPost()));
    pMenu->addSeparator();
  }

  menuItem = pMenu->addAction(tr("View Journal..."), this, SLOT(sViewTransactions()));
  menuItem->setEnabled(_privileges->check("ViewJournals"));
}

void postJournals::sViewTransactions()
{
  ParameterList params;
  if (_sources->id())
    params.append("accnt_id", _sources->id());
  _journalDates->appendValue(params);
  params.append("source", _sources->rawValue("sltrans_source"));
  params.append("posted", false);
  params.append("run");

  dspJournals *newdlg = new dspJournals();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void postJournals::sPrint(QList<int> journalnumbers)
{
  if (!journalnumbers.size())
    return;

  QPrinter printer(QPrinter::HighResolution);
  bool setupPrinter = true;
  bool userCanceled = false;

  if (orReport::beginMultiPrint(&printer, userCanceled) == false)
  {
    if(!userCanceled)
      systemError(this, tr("<p>Could not initialize printing system for "
                           "multiple reports."));
    return;
  }

  for (int i = 0; i < journalnumbers.size(); i++)
  {
    ParameterList params;
    params.append("startDate", _distDate->date());
    params.append("endDate", _distDate->date());
    params.append("startJrnlnum", journalnumbers.at(i));
    params.append("endJrnlnum", journalnumbers.at(i));
    params.append("gltrans", true);
    params.append("title",tr("General Ledger Series"));
    params.append("table", "gltrans");

    orReport report("GLSeries", params);
    if (report.isValid() && report.print(&printer, setupPrinter))
      setupPrinter = false;
    else
    {
      report.reportError(this);
      break;
    }
  }

  orReport::endMultiPrint(&printer);
}
