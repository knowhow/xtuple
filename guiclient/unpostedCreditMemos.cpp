/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "unpostedCreditMemos.h"

#include <QAction>
#include <QMenu>
#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include <parameter.h>
#include <openreports.h>

#include "creditMemo.h"
#include "distributeInventory.h"
#include "failedPostList.h"
#include "getGLDistDate.h"
#include "printCreditMemo.h"
#include "storedProcErrorLookup.h"

unpostedCreditMemos::unpostedCreditMemos(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
    setupUi(this);

    connect(_cmhead, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*)), this, SLOT(sPopulateMenu(QMenu*)));
    connect(_new, SIGNAL(clicked()), this, SLOT(sNew()));
    connect(_edit, SIGNAL(clicked()), this, SLOT(sEdit()));
    connect(_view, SIGNAL(clicked()), this, SLOT(sView()));
    connect(_delete, SIGNAL(clicked()), this, SLOT(sDelete()));
    connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
    connect(_post, SIGNAL(clicked()), this, SLOT(sPost()));

    _cmhead->addColumn(tr("C/M #"),         _orderColumn, Qt::AlignLeft,   true,  "cmhead_number"   );
    _cmhead->addColumn(tr("Prnt'd"),        _orderColumn, Qt::AlignCenter, true,  "printed" );
    _cmhead->addColumn(tr("Customer"),      -1,           Qt::AlignLeft,   true,  "cmhead_billtoname"   );
    _cmhead->addColumn(tr("Memo Date"),     _dateColumn,  Qt::AlignCenter, true,  "cmhead_docdate" );
    _cmhead->addColumn(tr("Hold"),          _whsColumn,   Qt::AlignCenter, true,  "cmhead_hold" );
    _cmhead->addColumn(tr("G/L Dist Date"), _dateColumn,  Qt::AlignCenter, true,  "distdate" );

    if (! _privileges->check("ChangeSOMemoPostDate"))
      _cmhead->hideColumn(5);

    if (_privileges->check("MaintainCreditMemos"))
    {
      connect(_cmhead, SIGNAL(valid(bool)), _edit, SLOT(setEnabled(bool)));
      connect(_cmhead, SIGNAL(valid(bool)), _delete, SLOT(setEnabled(bool)));
      connect(_cmhead, SIGNAL(itemSelected(int)), _edit, SLOT(animateClick()));
    }
    else
    {
      _new->setEnabled(FALSE);
      connect(_cmhead, SIGNAL(itemSelected(int)), _view, SLOT(animateClick()));
    }
    
    if (_privileges->check("PrintCreditMemos"))
      connect(_cmhead, SIGNAL(valid(bool)), _print, SLOT(setEnabled(bool)));

    if (_privileges->check("PostARDocuments"))
      connect(_cmhead, SIGNAL(valid(bool)), _post, SLOT(setEnabled(bool)));

    connect(omfgThis, SIGNAL(creditMemosUpdated()), this, SLOT(sFillList()));

    sFillList();
}

unpostedCreditMemos::~unpostedCreditMemos()
{
    // no need to delete child widgets, Qt does it all for us
}

void unpostedCreditMemos::languageChange()
{
    retranslateUi(this);
}

void unpostedCreditMemos::sPopulateMenu(QMenu *pMenu)
{
  QAction *menuItem;

  menuItem = pMenu->addAction(tr("Edit..."), this, SLOT(sEdit()));
  menuItem->setEnabled(_privileges->check("MaintainCreditMemos"));

  menuItem = pMenu->addAction(tr("View..."), this, SLOT(sView()));
  menuItem->setEnabled(_privileges->check("MaintainCreditMemos") ||
                       _privileges->check("ViewCreditMemos"));

  pMenu->addSeparator();

  menuItem = pMenu->addAction(tr("Post..."), this, SLOT(sPost()));
  menuItem->setEnabled(_privileges->check("PostARDocuments"));
}

void unpostedCreditMemos::sNew()
{
  ParameterList params;
  params.append("mode", "new");

  creditMemo *newdlg = new creditMemo();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void unpostedCreditMemos::sEdit()
{
  if (!checkSitePrivs(_cmhead->id()))
    return;
	
  ParameterList params;
  params.append("mode", "edit");
  params.append("cmhead_id", _cmhead->id());

  creditMemo *newdlg = new creditMemo();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void unpostedCreditMemos::sView()
{
  if (!checkSitePrivs(_cmhead->id()))
    return;
	
  ParameterList params;
  params.append("mode", "view");
  params.append("cmhead_id", _cmhead->id());

  creditMemo *newdlg = new creditMemo();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void unpostedCreditMemos::sPrint()
{
  QList<XTreeWidgetItem*> selected = _cmhead->selectedItems();
  QList<XTreeWidgetItem*> triedToClosed;

  for (int i = 0; i < selected.size(); i++)
  {
    if (checkSitePrivs(((XTreeWidgetItem*)(selected[i]))->id()))
    {
      ParameterList params;
      params.append("cmhead_id", ((XTreeWidgetItem*)(selected[i]))->id());
      params.append("persistentPrint");

      printCreditMemo newdlg(this, "", TRUE);
      newdlg.set(params);

      if (!newdlg.isSetup())
      {
        newdlg.exec();
        newdlg.setSetup(TRUE);
      }
    }
  }
  omfgThis->sCreditMemosUpdated();
}

void unpostedCreditMemos::sPost()
{
  bool changeDate = false;
  QDate newDate = QDate::currentDate();

  if (_privileges->check("ChangeSOMemoPostDate"))
  {
    getGLDistDate newdlg(this, "", TRUE);
    newdlg.sSetDefaultLit(tr("Credit Memo Date"));
    if (newdlg.exec() == XDialog::Accepted)
    {
      newDate = newdlg.date();
      changeDate = (newDate.isValid());
    }
    else
      return;
  }

  XSqlQuery setDate;
  setDate.prepare("UPDATE cmhead SET cmhead_gldistdate=:distdate "
		  "WHERE cmhead_id=:cmhead_id;");

  QList<XTreeWidgetItem*> selected = _cmhead->selectedItems();
  QList<XTreeWidgetItem*> triedToClosed;

  for (int i = 0; i < selected.size(); i++)
  {
    if (checkSitePrivs(((XTreeWidgetItem*)(selected[i]))->id()))
    {
      int id = ((XTreeWidgetItem*)(selected[i]))->id();

      if (changeDate)
      {
        setDate.bindValue(":distdate",  newDate);
        setDate.bindValue(":cmhead_id", id);
        setDate.exec();
        if (setDate.lastError().type() != QSqlError::NoError)
        {
	      systemError(this, setDate.lastError().databaseText(), __FILE__, __LINE__);
        }
      }
    }
  }

  XSqlQuery rollback;
  rollback.prepare("ROLLBACK;");
    
  XSqlQuery postq;
  postq.prepare("SELECT postCreditMemo(:cmhead_id, 0) AS result;");

  bool tryagain = false;
  do {
    for (int i = 0; i < selected.size(); i++)
    {
      if (checkSitePrivs(((XTreeWidgetItem*)(selected[i]))->id()))
      {
        int id = ((XTreeWidgetItem*)(selected[i]))->id();
 
        XSqlQuery tx;
        tx.exec("BEGIN;");	// because of possible lot, serial, or location distribution cancelations
      
        postq.bindValue(":cmhead_id", id);
        postq.exec();
        if (postq.first())
        {
          int result = postq.value("result").toInt();
          if (result < 0)
          {
            rollback.exec();
            systemError( this, storedProcErrorLookup("postCreditMemo", result),
                  __FILE__, __LINE__);
            return;
          }
          else
          {
            if (distributeInventory::SeriesAdjust(result, this) == XDialog::Rejected)
            {
              rollback.exec();
              QMessageBox::information( this, tr("Post Credit Memo"), tr("Transaction Canceled") );
              return;
            }

            q.exec("COMMIT;");
          }
        }
        // contains() string is hard-coded in stored procedure
        else if (postq.lastError().databaseText().contains("post to closed period"))
        {
          rollback.exec();
          if (changeDate)
          {
            triedToClosed = selected;
            break;
          }
          else
            triedToClosed.append(selected[i]);
        }
        else if (postq.lastError().type() != QSqlError::NoError)
        {
          rollback.exec();
          systemError(this, tr("A System Error occurred posting Credit Memo#%1.\n%2")
                  .arg(selected[i]->text(0))
                  .arg(postq.lastError().databaseText()),
                __FILE__, __LINE__);
        }
      }

      if (triedToClosed.size() > 0)
      {
        failedPostList newdlg(this, "", true);
        newdlg.sSetList(triedToClosed, _cmhead->headerItem(), _cmhead->header());
        tryagain = (newdlg.exec() == XDialog::Accepted);
        selected = triedToClosed;
        triedToClosed.clear();
      }
    }
  } while (tryagain);

  omfgThis->sCreditMemosUpdated();
}

void unpostedCreditMemos::sDelete()
{
  if (QMessageBox::question(this, tr("Delete Selected Credit Memos?"),
                            tr("<p>Are you sure that you want to delete the "
			       "selected Credit Memos?"),
                            QMessageBox::Yes, QMessageBox::No | QMessageBox::Default) == QMessageBox::Yes)
  {
    XSqlQuery delq;
    delq.prepare("SELECT deleteCreditMemo(:cmhead_id) AS result;");

    QList<XTreeWidgetItem*> selected = _cmhead->selectedItems();
    for (int i = 0; i < selected.size(); i++)
    {
      if (checkSitePrivs(((XTreeWidgetItem*)(selected[i]))->id()))
      {
        delq.bindValue(":cmhead_id", ((XTreeWidgetItem*)(selected[i]))->id());
        delq.exec();
        if (delq.first())
        {
	      if (! delq.value("result").toBool())
	        systemError(this, tr("Could not delete Credit Memo."),
		            __FILE__, __LINE__);
        }
        else if (delq.lastError().type() != QSqlError::NoError)
	      systemError(this,
		          tr("Error deleting Credit Memo %1\n").arg(selected[i]->text(0)) +
		          delq.lastError().databaseText(), __FILE__, __LINE__);
      }
    }

    omfgThis->sCreditMemosUpdated();
  }
}

void unpostedCreditMemos::sFillList()
{
  _cmhead->clear();
  _cmhead->populate( "SELECT cmhead_id, cmhead_number,"
                     "       COALESCE(cmhead_printed, FALSE) AS printed,"
                     "       cmhead_billtoname, cmhead_docdate, cmhead_hold,"
                     "       COALESCE(cmhead_gldistdate, cmhead_docdate) AS distdate "
                     "FROM cmhead "
                     "WHERE ( (NOT cmhead_posted) "
                     "  AND   ( ((SELECT COUNT(*)"
                     "            FROM cmitem, itemsite, site()"
                     "            WHERE ( (cmitem_cmhead_id=cmhead_id)"
                     "              AND   (itemsite_id=cmitem_itemsite_id)"
                     "              AND   (warehous_id=itemsite_warehous_id) )) > 0) OR"
                     "          ((SELECT COUNT(*)"
                     "            FROM cmitem"
                     "            WHERE (cmitem_cmhead_id=cmhead_id)) = 0) ) ) "
                     "ORDER BY cmhead_number;" );
}

bool unpostedCreditMemos::checkSitePrivs(int orderid)
{
  if (_preferences->boolean("selectedSites"))
  {
    XSqlQuery check;
    check.prepare("SELECT checkCreditMemoSitePrivs(:cmheadid) AS result;");
    check.bindValue(":cmheadid", orderid);
    check.exec();
    if (check.first())
    {
    if (!check.value("result").toBool())
      {
        QMessageBox::critical(this, tr("Access Denied"),
                                       tr("You may not view or edit this Credit Memo as it references "
                                       "a Site for which you have not been granted privileges.")) ;
        return false;
      }
    }
  }
  return true;
}
