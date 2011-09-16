/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "openVouchers.h"

#include <QAction>
#include <QMenu>
#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include <metasql.h>
#include <mqlutil.h>
#include <parameter.h>
#include <openreports.h>

#include "errorReporter.h"
#include "failedPostList.h"
#include "getGLDistDate.h"
#include "miscVoucher.h"
#include "storedProcErrorLookup.h"
#include "voucher.h"

openVouchers::openVouchers(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  connect(_vendorgroup, SIGNAL(updated()), this, SLOT(sFillList()));
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_vohead, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*)), this, SLOT(sPopulateMenu(QMenu*)));
  connect(_edit, SIGNAL(clicked()), this, SLOT(sEdit()));
  connect(_view, SIGNAL(clicked()), this, SLOT(sView()));
  connect(_new, SIGNAL(clicked()), this, SLOT(sNew()));
  connect(_delete, SIGNAL(clicked()), this, SLOT(sDelete()));
  connect(_newMisc, SIGNAL(clicked()), this, SLOT(sNewMisc()));
  connect(_post, SIGNAL(clicked()), this, SLOT(sPost()));

  _vohead->addColumn(tr("Vchr. #"),        _orderColumn, Qt::AlignRight,  true,  "vohead_number"  );
  _vohead->addColumn(tr("P/O #"),          _orderColumn, Qt::AlignRight,  true,  "ponumber"  );
  _vohead->addColumn(tr("Vendor"),         -1,           Qt::AlignLeft,   true,  "vendor"   );
  _vohead->addColumn(tr("Vend. Type"),     _itemColumn,  Qt::AlignLeft,   true,  "vendtype_code"   );
  _vohead->addColumn(tr("Vendor Invc. #"), _itemColumn,  Qt::AlignRight,  true,  "vohead_invcnumber"  );
  _vohead->addColumn(tr("Dist. Date"),     _dateColumn,  Qt::AlignCenter, true,  "vohead_distdate" );
  _vohead->addColumn(tr("G/L Post Date"),  _dateColumn,  Qt::AlignCenter, true,  "postdate" );
  _vohead->addColumn(tr("Amount"),         _moneyColumn, Qt::AlignRight,  true,  "vohead_amount" );

  if (! _privileges->check("ChangeVOPostDate"))
    _vohead->hideColumn(6);

  if (_privileges->check("MaintainVouchers"))
  {
    connect(_vohead, SIGNAL(valid(bool)), _edit, SLOT(setEnabled(bool)));
    connect(_vohead, SIGNAL(valid(bool)), _delete, SLOT(setEnabled(bool)));
    connect(_vohead, SIGNAL(itemSelected(int)), _edit, SLOT(animateClick()));
  }
  else
  {
    _new->setEnabled(FALSE);
    _newMisc->setEnabled(FALSE);
    connect(_vohead, SIGNAL(itemSelected(int)), _view, SLOT(animateClick()));
  }

  if (_privileges->check("PostVouchers"))
    connect(_vohead, SIGNAL(valid(bool)), _post, SLOT(setEnabled(bool)));

  connect(omfgThis, SIGNAL(vouchersUpdated()), this, SLOT(sFillList()));

  sFillList();
}

openVouchers::~openVouchers()
{
  // no need to delete child widgets, Qt does it all for us
}

void openVouchers::languageChange()
{
  retranslateUi(this);
}

bool openVouchers::setParams(ParameterList &params)
{
  _vendorgroup->appendValue(params);
  params.append("misc", tr("Misc."));

  return true;
}

void openVouchers::sPrint()
{
  ParameterList params;
  if (! setParams(params))
    return;

  orReport report("VoucheringEditList", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void openVouchers::sNew()
{
  ParameterList params;
  params.append("mode", "new");

  voucher *newdlg = new voucher();
  omfgThis->handleNewWindow(newdlg);
  newdlg->set(params);
}

void openVouchers::sNewMisc()
{
  ParameterList params;
  params.append("mode", "new");
  if (_vendorgroup->isSelectedVend())
    params.append("vend_id", _vendorgroup->vendId());

  miscVoucher *newdlg = new miscVoucher();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void openVouchers::sEdit()
{
  if (!checkSitePrivs(_vohead->id()))
    return;
    
  ParameterList params;
  params.append("mode", "edit");
  params.append("vohead_id", _vohead->id());

  if (_vohead->altId() == -1)
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

void openVouchers::sView()
{
  if (!checkSitePrivs(_vohead->id()))
    return;
    
  ParameterList params;
  params.append("mode", "view");
  params.append("vohead_id", _vohead->id());

  if (_vohead->altId() == -1)
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

void openVouchers::sDelete()
{
  if (QMessageBox::question(this, tr("Delete Selected Vouchers"),
			    tr("<p>Are you sure that you want to delete the "
			       "selected Vouchers?"),
			    QMessageBox::Yes, QMessageBox::No | QMessageBox::Default) == QMessageBox::Yes)
  {
    XSqlQuery delq;
    delq.prepare("DELETE FROM vohead WHERE vohead_id=:vohead_id;");

    QList<XTreeWidgetItem*>selected = _vohead->selectedItems();
    for (int i = 0; i < selected.size(); i++)
    {
      if (checkSitePrivs(((XTreeWidgetItem*)(selected[i]))->id()))
      {
       int id = ((XTreeWidgetItem*)(selected[i]))->id();
       delq.bindValue(":vohead_id", id);
       delq.exec();
       ErrorReporter::error(QtCriticalMsg, this, tr("Deleting Voucher"),
                            delq, __FILE__, __LINE__);
      }
    }

    omfgThis->sVouchersUpdated();
  }
}

void openVouchers::sPost()
{
  bool changeDate = false;
  QDate newDate = QDate::currentDate();

  if (_privileges->check("ChangeVOPostDate"))
  {
    getGLDistDate newdlg(this, "", TRUE);
    newdlg.sSetDefaultLit(tr("Distribution Date"));
    if (newdlg.exec() == XDialog::Accepted)
    {
      newDate = newdlg.date();
      changeDate = (newDate.isValid());
    }
    else
      return;
  }

  XSqlQuery setDate;
  setDate.prepare("UPDATE vohead SET vohead_gldistdate=:distdate "
		  "WHERE vohead_id=:vohead_id;");

  QList<XTreeWidgetItem*>selected = _vohead->selectedItems();
  QList<XTreeWidgetItem*>triedToClosed;

  for (int i = 0; i < selected.size(); i++)
  {
    if (checkSitePrivs(((XTreeWidgetItem*)(selected[i]))->id()))
    {
      int id = ((XTreeWidgetItem*)(selected[i]))->id();

      if (changeDate)
      {
        setDate.bindValue(":distdate",  newDate);
        setDate.bindValue(":vohead_id", id);
        setDate.exec();
        ErrorReporter::error(QtCriticalMsg, this, tr("Changing Dist. Date"),
                             setDate, __FILE__, __LINE__);
      }
    }
  }
  
  XSqlQuery post;
  post.prepare("SELECT fetchJournalNumber('AP-VO') AS result;");
  post.exec();
  int journalNumber = 0;
  if(post.first())
    journalNumber = post.value("result").toInt();
  else if (ErrorReporter::error(QtCriticalMsg, this, tr("Getting Journal Number"),
                                post, __FILE__, __LINE__))
    return;

  post.prepare("SELECT postVoucher(:vohead_id, :journalNumber, FALSE) AS result;");

  bool tryagain = false;
  do {
    for (int i = 0; i < selected.size(); i++)
    {
      if (checkSitePrivs(((XTreeWidgetItem*)(selected[i]))->id()))
      {
        int id = ((XTreeWidgetItem*)(selected[i]))->id();

        post.bindValue(":vohead_id", id);
        post.bindValue(":journalNumber", journalNumber);
        post.exec();
        if (post.first())
        {
          int result = post.value("result").toInt();
          if (result < 0)
            ErrorReporter::error(QtCriticalMsg, this, tr("Posting Voucher"),
                                 storedProcErrorLookup("postVoucher", result),
                                 __FILE__, __LINE__);
        }
      // contains() string is hard-coded in stored procedure
        else if (post.lastError().databaseText().contains("post to closed period"))
        {
          if (changeDate)
          {
            triedToClosed = selected;
            break;
          }
          else
            triedToClosed.append(selected[i]);
        }
        else
          ErrorReporter::error(QtCriticalMsg, this, tr("Posting Voucher"),
                               post, __FILE__, __LINE__);
      }
    }

    if (triedToClosed.size() > 0)
    {
      failedPostList newdlg(this, "", true);
      newdlg.sSetList(triedToClosed, _vohead->headerItem(), _vohead->header());
      tryagain = (newdlg.exec() == XDialog::Accepted);
      selected = triedToClosed;
      triedToClosed.clear();
    }
  } while (tryagain);

  if (_printJournal->isChecked())
  {
    ParameterList params;
    params.append("journalNumber", journalNumber);

    orReport report("PayablesJournal", params);
    if (report.isValid())
      report.print();
    else
      report.reportError(this);
  }

  omfgThis->sVouchersUpdated();
}

void openVouchers::sPopulateMenu(QMenu *pMenu)
{
  QAction *menuItem;

  menuItem = pMenu->addAction(tr("Edit Voucher..."), this, SLOT(sEdit()));
  menuItem->setEnabled(_privileges->check("MaintainVouchers"));

  menuItem = pMenu->addAction(tr("View Voucher..."), this, SLOT(sView()));

  menuItem = pMenu->addAction(tr("Delete Voucher..."), this, SLOT(sDelete()));
  menuItem->setEnabled(_privileges->check("MaintainVouchers"));

  pMenu->addSeparator();

  menuItem = pMenu->addAction(tr("Post Voucher..."), this, SLOT(sPost()));
  menuItem->setEnabled(_privileges->check("PostVouchers"));
}

void openVouchers::sFillList()
{
  ParameterList params;
  if (! setParams(params))
    return;

  bool ok = true;
  QString errorString;
  MetaSQLQuery mql = MQLUtil::mqlLoad("openVouchers", "populate", errorString, &ok);
  if(!ok)
  {
    ErrorReporter::error(QtCriticalMsg, this, tr("Getting Open Vouchers"),
                         errorString, __FILE__, __LINE__);
    return;
  }
	
  XSqlQuery r = mql.toQuery(params);
  _vohead->clear();
  _vohead->populate(r, TRUE);
  if (ErrorReporter::error(QtCriticalMsg, this, tr("Getting Open Vouchers"),
                           r, __FILE__, __LINE__))
    return;
}

bool openVouchers::checkSitePrivs(int orderid)
{
  if (_preferences->boolean("selectedSites"))
  {
    XSqlQuery check;
    check.prepare("SELECT checkVoucherSitePrivs(:voheadid) AS result;");
    check.bindValue(":voheadid", orderid);
    check.exec();
    if (check.first())
    {
      if (!check.value("result").toBool())
      {
        QMessageBox::critical(this, tr("Access Denied"),
                              tr("<p>You may not view or edit this Voucher as "
                                 "it references a Site for which you have not "
                                 "been granted privileges.")) ;
        return false;
      }
    }
    else if (ErrorReporter::error(QtCriticalMsg, this, tr("Checking Privileges"),
                                  check, __FILE__, __LINE__))
      return false;
  }
  return true;
}
