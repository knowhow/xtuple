/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "creditMemoEditList.h"

#include <QAction>
#include <QMenu>
#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include <openreports.h>
#include <metasql.h>

#include "selectOrderForBilling.h"
#include "selectBillingQty.h"
#include "printInvoices.h"
#include "creditMemo.h"
#include "creditMemoItem.h"
#include "mqlutil.h"

creditMemoEditList::creditMemoEditList(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  connect(_cmhead, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*,QTreeWidgetItem*)));
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));

  _cmhead->addColumn(tr("Document #"), (_itemColumn+_cmhead->indentation()), Qt::AlignLeft,  true, "docnumber");
  _cmhead->addColumn(tr("Order #"),     _orderColumn, Qt::AlignLeft,  true, "ordernumber");
  _cmhead->addColumn(tr("Cust./Item #"), _itemColumn, Qt::AlignLeft,  true, "custitemnumber");
  _cmhead->addColumn(tr("Name/Description"),      -1, Qt::AlignLeft,  true, "namedescrip");
  _cmhead->addColumn(tr("UOM"),           _uomColumn, Qt::AlignCenter,true, "uom_name");
  _cmhead->addColumn(tr("Qty. to Bill"),  _qtyColumn, Qt::AlignRight, true, "qtytobill");
  _cmhead->addColumn(tr("Price"),        _costColumn, Qt::AlignRight, true, "price");
  _cmhead->addColumn(tr("Ext. Price"),  _moneyColumn, Qt::AlignRight, true, "extprice");
  _cmhead->addColumn(tr("Currency"), _currencyColumn, Qt::AlignLeft,  true, "currabbr");

  connect(omfgThis, SIGNAL(creditMemosUpdated()), this, SLOT(sFillList()));

  sFillList();
}

creditMemoEditList::~creditMemoEditList()
{
  // no need to delete child widgets, Qt does it all for us
}

void creditMemoEditList::languageChange()
{
  retranslateUi(this);
}

void creditMemoEditList::sEditCreditMemo()
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

void creditMemoEditList::sEditCreditMemoItem()
{
  if (!checkSitePrivs(_cmhead->id()))
    return;
  
  ParameterList params;
  params.append("mode", "edit");
  params.append("cmitem_id", _cmhead->altId());

  creditMemoItem newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void creditMemoEditList::sPopulateMenu(QMenu *pMenu, QTreeWidgetItem *pSelected)
{
  _orderid = _cmhead->id();

  pMenu->addAction(tr("Edit Credit Memo..."), this, SLOT(sEditCreditMemo()));

  if (((XTreeWidgetItem *)pSelected)->altId() != -1)
    pMenu->addAction(tr("Edit Credit Memo Item..."), this, SLOT(sEditCreditMemoItem()));
}

void creditMemoEditList::sFillList()
{
  ParameterList params;
  params.append("notassigned", tr("Not Assigned"));
  params.append("taxes", tr("Taxes"));
  params.append("debit", tr("Debit"));
  params.append("credit", tr("Credit"));

  MetaSQLQuery mql = mqlLoad("creditMemo", "editlist");
  q = mql.toQuery(params);
  _cmhead->populate(q, true);
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void creditMemoEditList::sPrint()
{
  orReport report("CreditMemoEditList");
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

bool creditMemoEditList::checkSitePrivs(int ordid)
{
  if (_preferences->boolean("selectedSites"))
  {
    XSqlQuery check;
    check.prepare("SELECT checkCreditMemoSitePrivs(:cmheadid) AS result;");
    check.bindValue(":cmheadid", ordid);
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
