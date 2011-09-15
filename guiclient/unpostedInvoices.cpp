/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "unpostedInvoices.h"

#include <QAction>
#include <QMenu>
#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include <metasql.h>
#include  <openreports.h>

#include "failedPostList.h"
#include "getGLDistDate.h"
#include "invoice.h"
#include "mqlutil.h"
#include "printInvoice.h"
#include "storedProcErrorLookup.h"
#include "distributeInventory.h"

unpostedInvoices::unpostedInvoices(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  connect(_new, SIGNAL(clicked()), this, SLOT(sNew()));
  connect(_edit, SIGNAL(clicked()), this, SLOT(sEdit()));
  connect(_delete, SIGNAL(clicked()), this, SLOT(sDelete()));
  connect(_invchead, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*)), this, SLOT(sPopulateMenu(QMenu*)));
  connect(_view, SIGNAL(clicked()), this, SLOT(sView()));
  connect(_post, SIGNAL(clicked()), this, SLOT(sPost()));
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_query, SIGNAL(clicked()), this, SLOT(sFillList()));

  _invchead->addColumn(tr("Invoice #"),  _orderColumn, Qt::AlignLeft,   true,  "invchead_invcnumber" );
  _invchead->addColumn(tr("Prnt'd"),     _orderColumn, Qt::AlignCenter, true,  "invchead_printed" );
  _invchead->addColumn(tr("S/O #"),      _orderColumn, Qt::AlignLeft,   true,  "invchead_ordernumber" );
  _invchead->addColumn(tr("Customer"),   -1,           Qt::AlignLeft,   true,  "cust_name" );
  _invchead->addColumn(tr("Ship-to"),   100,          Qt::AlignLeft,   false,  "invchead_shipto_name" );
  _invchead->addColumn(tr("Invc. Date"), _dateColumn,  Qt::AlignCenter, true,  "invchead_invcdate" );
  _invchead->addColumn(tr("Ship Date"),  _dateColumn,  Qt::AlignCenter, true,  "invchead_shipdate" );
  _invchead->addColumn(tr("G/L Dist Date"),_dateColumn,Qt::AlignCenter, true,  "gldistdate" );
  _invchead->addColumn(tr("Recurring"),  _ynColumn,    Qt::AlignCenter, false, "isRecurring" );
  _invchead->addColumn(tr("Ship Date"),  _dateColumn,  Qt::AlignCenter, false, "invchead_shipdate" );
  _invchead->addColumn(tr("P/O #"),      _orderColumn, Qt::AlignCenter, false, "invchead_ponumber" );
  _invchead->addColumn(tr("Total Amount"), _bigMoneyColumn, Qt::AlignRight, true, "extprice" );
  _invchead->setSelectionMode(QAbstractItemView::ExtendedSelection);

  if (! _privileges->check("ChangeARInvcDistDate"))
    _invchead->hideColumn(7);

  if (_privileges->check("MaintainMiscInvoices"))
  {
    _new->setEnabled(TRUE);
    connect(_invchead, SIGNAL(valid(bool)), _edit, SLOT(setEnabled(bool)));
    connect(_invchead, SIGNAL(valid(bool)), _delete, SLOT(setEnabled(bool)));
    connect(_invchead, SIGNAL(itemSelected(int)), _edit, SLOT(animateClick()));
  }
  else
    connect(_invchead, SIGNAL(itemSelected(int)), _view, SLOT(animateClick()));

  if (_privileges->check("MaintainMiscInvoices") || _privileges->check("ViewMiscInvoices"))
    connect(_invchead, SIGNAL(valid(bool)), _view, SLOT(setEnabled(bool)));

  if (_privileges->check("PrintInvoices"))
    connect(_invchead, SIGNAL(valid(bool)), _print, SLOT(setEnabled(bool)));
  
  if (_privileges->check("PostMiscInvoices"))
  {
    connect(_invchead, SIGNAL(valid(bool)), _post, SLOT(setEnabled(bool)));
    connect(_invchead, SIGNAL(valid(bool)), _printJournal, SLOT(setEnabled(bool)));
  }

  connect(omfgThis, SIGNAL(invoicesUpdated(int, bool)), this, SLOT(sFillList()));

  if (_preferences->boolean("XCheckBox/forgetful"))
    _printJournal->setChecked(true);

  sFillList();
}

unpostedInvoices::~unpostedInvoices()
{
    // no need to delete child widgets, Qt does it all for us
}

void unpostedInvoices::languageChange()
{
    retranslateUi(this);
}

void unpostedInvoices::sNew()
{
  invoice::newInvoice(-1);
}

void unpostedInvoices::sEdit()
{
  QList<XTreeWidgetItem*> selected = _invchead->selectedItems();
  for (int i = 0; i < selected.size(); i++)
      if (checkSitePrivs(((XTreeWidgetItem*)(selected[i]))->id()))
        invoice::editInvoice(((XTreeWidgetItem*)(selected[i]))->id());
      
}

void unpostedInvoices::sView()
{
  QList<XTreeWidgetItem*> selected = _invchead->selectedItems();
  for (int i = 0; i < selected.size(); i++)
      if (checkSitePrivs(((XTreeWidgetItem*)(selected[i]))->id()))
        invoice::viewInvoice(((XTreeWidgetItem*)(selected[i]))->id());
}

void unpostedInvoices::sDelete()
{
  if ( QMessageBox::warning( this, tr("Delete Selected Invoices"),
                             tr("<p>Are you sure that you want to delete the "
			        "selected Invoices?"),
                             tr("Delete"), tr("Cancel"), QString::null, 1, 1 ) == 0)
  {
    q.prepare("SELECT deleteInvoice(:invchead_id) AS result;");

    QList<XTreeWidgetItem*> selected = _invchead->selectedItems();
    for (int i = 0; i < selected.size(); i++)
    {
      if (checkSitePrivs(((XTreeWidgetItem*)(selected[i]))->id()))
	  {
        q.bindValue(":invchead_id", ((XTreeWidgetItem*)(selected[i]))->id());
        q.exec();
        if (q.first())
        {
	      int result = q.value("result").toInt();
	      if (result < 0)
	      {
	        systemError(this, storedProcErrorLookup("deleteInvoice", result),
		            __FILE__, __LINE__);
	      }
        }
        else if (q.lastError().type() != QSqlError::NoError)
	      systemError(this,
		          tr("Error deleting Invoice %1\n").arg(selected[i]->text(0)) +
		          q.lastError().databaseText(), __FILE__, __LINE__);
      }
    }

    omfgThis->sInvoicesUpdated(-1, TRUE);
    omfgThis->sBillingSelectionUpdated(-1, -1);
  }
}

void unpostedInvoices::sPrint()
{
  QList<XTreeWidgetItem*> selected = _invchead->selectedItems();
  printInvoice newdlg(this, "", TRUE);

  for (int i = 0; i < selected.size(); i++)
  {
    if (checkSitePrivs(((XTreeWidgetItem*)(selected[i]))->id()))
    {
      ParameterList params;
      params.append("invchead_id", ((XTreeWidgetItem*)(selected[i]))->id());
      params.append("persistentPrint");

      newdlg.set(params);

      if (!newdlg.isSetup())
      {
        if(newdlg.exec() == QDialog::Rejected)
          break;
        newdlg.setSetup(TRUE);
      }
    }
  }

  omfgThis->sInvoicesUpdated(-1, TRUE);
}

void unpostedInvoices::sPost()
{
  bool changeDate = false;
  QDate newDate = QDate::currentDate();

  if (_privileges->check("ChangeARInvcDistDate"))
  {
    getGLDistDate newdlg(this, "", TRUE);
    newdlg.sSetDefaultLit(tr("Invoice Date"));
    if (newdlg.exec() == XDialog::Accepted)
    {
      newDate = newdlg.date();
      changeDate = (newDate.isValid());
    }
    else
      return;
  }

  int journal = -1;
  q.exec("SELECT fetchJournalNumber('AR-IN') AS result;");
  if (q.first())
  {
    journal = q.value("result").toInt();
    if (journal < 0)
    {
      systemError(this, storedProcErrorLookup("fetchJournalNumber", journal), __FILE__, __LINE__);
      return;
    }
  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  XSqlQuery xrate;
  xrate.prepare("SELECT curr_rate "
		"FROM curr_rate, invchead "
		"WHERE ((curr_id=invchead_curr_id)"
		"  AND  (invchead_id=:invchead_id)"
		"  AND  (invchead_invcdate BETWEEN curr_effective AND curr_expires));");
  // if SUM becomes dependent on curr_id then move XRATE before it in the loop
  XSqlQuery sum;
  sum.prepare("SELECT invoicetotal(:invchead_id) AS subtotal;");

  XSqlQuery rollback;
  rollback.prepare("ROLLBACK;");

  XSqlQuery post;
  post.prepare("SELECT postInvoice(:invchead_id, :journal) AS result;");

  XSqlQuery setDate;
  setDate.prepare("UPDATE invchead SET invchead_gldistdate=:distdate "
		  "WHERE invchead_id=:invchead_id;");

  QList<XTreeWidgetItem*> selected = _invchead->selectedItems();
  QList<XTreeWidgetItem*> triedToClosed;

  for (int i = 0; i < selected.size(); i++)
  {
    if (checkSitePrivs(((XTreeWidgetItem*)(selected[i]))->id()))
    {
      int id = ((XTreeWidgetItem*)(selected[i]))->id();

      if (changeDate)
      {
        setDate.bindValue(":distdate",    newDate);
        setDate.bindValue(":invchead_id", id);
        setDate.exec();
        if (setDate.lastError().type() != QSqlError::NoError)
        {
	      systemError(this, setDate.lastError().databaseText(), __FILE__, __LINE__);
        }
      }
    }
  }

  bool tryagain = false;
  do {
    for (int i = 0; i < selected.size(); i++)
    {
      if (checkSitePrivs(((XTreeWidgetItem*)(selected[i]))->id()))
      {
        int id = ((XTreeWidgetItem*)(selected[i]))->id();

        sum.bindValue(":invchead_id", id);
        if (sum.exec() && sum.first() && sum.value("subtotal").toDouble() == 0)
        {
	      if (QMessageBox::question(this, tr("Invoice Has Value 0"),
		      		  tr("Invoice #%1 has a total value of 0.\n"
			     	     "Would you like to post it anyway?")
				    .arg(selected[i]->text(0)),
				  QMessageBox::Yes,
				  QMessageBox::No | QMessageBox::Default)
	      == QMessageBox::No)
	        continue;
        }
        else if (sum.lastError().type() != QSqlError::NoError)
        {
	      systemError(this, sum.lastError().databaseText(), __FILE__, __LINE__);
	      continue;
        }
        else if (sum.value("subtotal").toDouble() != 0)
        {
	      xrate.bindValue(":invchead_id", id);
	      xrate.exec();
	      if (xrate.lastError().type() != QSqlError::NoError)
	      {
	        systemError(this, tr("System Error posting Invoice #%1\n%2")
			            .arg(selected[i]->text(0))
			            .arg(xrate.lastError().databaseText()),
		                __FILE__, __LINE__);
	        continue;
	      }
	      else if (!xrate.first() || xrate.value("curr_rate").isNull())
	      {
	        systemError(this, tr("Could not post Invoice #%1 because of a missing exchange rate.")
						.arg(selected[i]->text(0)));
	        continue;
	      }
        }

        q.exec("BEGIN;");	// because of possible lot, serial, or location distribution cancelations
        post.bindValue(":invchead_id", id);
        post.bindValue(":journal",     journal);
        post.exec();
        if (post.first())
        {
	      int result = post.value("result").toInt();
	      if (result < 0)
              {
                rollback.exec();
                systemError(this, storedProcErrorLookup("postInvoice", result),
		            __FILE__, __LINE__);
              }
              else if (distributeInventory::SeriesAdjust(result, this) == XDialog::Rejected)
              {
                rollback.exec();
                QMessageBox::information( this, tr("Post Invoices"), tr("Transaction Canceled") );
                return;
              }

              q.exec("COMMIT;");
        }
        // contains() string is hard-coded in stored procedure
        else if (post.lastError().databaseText().contains("post to closed period"))
        {
            if (changeDate)
              triedToClosed = selected;
            else
              triedToClosed.append(selected[i]);
      }
      else if (post.lastError().type() != QSqlError::NoError)
      {
        rollback.exec();
        systemError(this, tr("A System Error occurred posting Invoice #%1.\n%2")
                    .arg(selected[i]->text(0))
                        .arg(post.lastError().databaseText()),
                        __FILE__, __LINE__);
      }
    }

    if (triedToClosed.size() > 0)
    {
      failedPostList newdlg(this, "", true);
      newdlg.sSetList(triedToClosed, _invchead->headerItem(), _invchead->header());
      tryagain = (newdlg.exec() == XDialog::Accepted);
      selected = triedToClosed;
      triedToClosed.clear();
      }
    }
  } while (tryagain);

  if (_printJournal->isChecked())
  {
    ParameterList params;
    params.append("journalNumber", journal);

    orReport report("SalesJournal", params);
    if (report.isValid())
      report.print();
    else
      report.reportError(this);
  }

  omfgThis->sInvoicesUpdated(-1, TRUE);
}

void unpostedInvoices::sPopulateMenu(QMenu *pMenu)
{
  QAction *menuItem;

  menuItem = pMenu->addAction(tr("Edit..."), this, SLOT(sEdit()));
  if (!_privileges->check("MaintainMiscInvoices"))
    menuItem->setEnabled(false);

  menuItem = pMenu->addAction(tr("View..."), this, SLOT(sView()));
  if ((!_privileges->check("MaintainMiscInvoices")) && (!_privileges->check("ViewMiscInvoices")))
    menuItem->setEnabled(false);

  menuItem = pMenu->addAction(tr("Delete..."), this, SLOT(sDelete()));
  if (!_privileges->check("MaintainMiscInvoices"))
    menuItem->setEnabled(false);

  pMenu->addSeparator();

  menuItem = pMenu->addAction(tr("Print..."), this, SLOT(sPrint()));
  if (!_privileges->check("PrintInvoices"))
    menuItem->setEnabled(false);

  menuItem = pMenu->addAction(tr("Post..."), this, SLOT(sPost()));
  if (!_privileges->check("PostMiscInvoices"))
    menuItem->setEnabled(false);
}

void unpostedInvoices::sFillList()
{
  MetaSQLQuery mql = mqlLoad("invoices", "detail");
  ParameterList params;
  params.append("unpostedOnly");
  q = mql.toQuery(params);
  _invchead->populate(q);
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

bool unpostedInvoices::checkSitePrivs(int invcid)
{
  if (_preferences->boolean("selectedSites"))
  {
    XSqlQuery check;
    check.prepare("SELECT checkInvoiceSitePrivs(:invcheadid) AS result;");
    check.bindValue(":invcheadid", invcid);
    check.exec();
    if (check.first())
    {
    if (!check.value("result").toBool())
      {
        QMessageBox::critical(this, tr("Access Denied"),
                                       tr("You may not view or edit this Invoice as it references "
                                       "a Site for which you have not been granted privileges.")) ;
        return false;
      }
    }
  }
  return true;
}
