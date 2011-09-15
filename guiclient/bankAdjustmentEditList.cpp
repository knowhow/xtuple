/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "bankAdjustmentEditList.h"

#include <QAction>
#include <QMenu>
#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include <parameter.h>
#include <openreports.h>

#include "guiclient.h"
#include "bankAdjustment.h"
#include "storedProcErrorLookup.h"

bankAdjustmentEditList::bankAdjustmentEditList(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_new, SIGNAL(clicked()), this, SLOT(sNew()));
  connect(_edit, SIGNAL(clicked()), this, SLOT(sEdit()));
  connect(_view, SIGNAL(clicked()), this, SLOT(sView()));
  connect(_delete, SIGNAL(clicked()), this, SLOT(sDelete()));
  connect(_bankaccnt, SIGNAL(newID(int)), this, SLOT(sFillList()));
  connect(_adjustments, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*)));

 _bankaccnt->populate("SELECT bankaccnt_id,"
		     "       (bankaccnt_name || '-' || bankaccnt_descrip),"
		     "       bankaccnt_name "
		     "FROM bankaccnt "
		     "ORDER BY bankaccnt_name;");
  
  _adjustments->addColumn(tr("Bank Account"),_itemColumn, Qt::AlignLeft,  true, "f_bank");
  _adjustments->addColumn(tr("Adj. Type"),  _orderColumn, Qt::AlignLeft,  true, "bankadjtype_name");
  _adjustments->addColumn(tr("Dist. Date"),  _dateColumn, Qt::AlignCenter,true, "bankadj_date");
  _adjustments->addColumn(tr("Doc. Number"),          -1, Qt::AlignRight, true, "bankadj_docnumber");
  _adjustments->addColumn(tr("Amount"),  _bigMoneyColumn, Qt::AlignRight, true, "bankadj_amount");
  _adjustments->addColumn(tr("Currency"),_currencyColumn, Qt::AlignLeft,  true, "currabbr");

  if (omfgThis->singleCurrency())
      _adjustments->hideColumn("currabbr");
  
  if (_privileges->check("MaintainBankAdjustments"))
  {
    connect(_adjustments, SIGNAL(valid(bool)), _edit, SLOT(setEnabled(bool)));
    connect(_adjustments, SIGNAL(valid(bool)), _delete, SLOT(setEnabled(bool)));
    connect(_adjustments, SIGNAL(itemSelected(int)), _edit, SLOT(animateClick()));
  }
  else
  {
    _new->setEnabled(FALSE);
    connect(_adjustments, SIGNAL(itemSelected(int)), _view, SLOT(animateClick()));
  }
  
  connect(omfgThis, SIGNAL(bankAdjustmentsUpdated(int, bool)), this, SLOT(sFillList()));
  
  sFillList();
}

bankAdjustmentEditList::~bankAdjustmentEditList()
{
  // no need to delete child widgets, Qt does it all for us
}

void bankAdjustmentEditList::languageChange()
{
  retranslateUi(this);
}

void bankAdjustmentEditList::sPrint()
{
  ParameterList params;

  params.append("bankaccnt_id", _bankaccnt->id());

  orReport report("BankAdjustmentEditList", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void bankAdjustmentEditList::sNew()
{
  ParameterList params;
  params.append("mode", "new");

  params.append("bankaccnt_id", _bankaccnt->id());
  
  bankAdjustment *newdlg = new bankAdjustment();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void bankAdjustmentEditList::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("bankadj_id", _adjustments->id());
  
  bankAdjustment *newdlg = new bankAdjustment();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void bankAdjustmentEditList::sView()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("bankadj_id", _adjustments->id());
  
  bankAdjustment *newdlg = new bankAdjustment();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void bankAdjustmentEditList::sDelete()
{
  q.prepare( "DELETE FROM bankadj "
             "WHERE ( (bankadj_id=:bankadj_id)"
             " AND (NOT bankadj_posted) ); ");
  q.bindValue(":bankadj_id", _adjustments->id());
  q.exec();
  sFillList();
}

void bankAdjustmentEditList::sFillList()
{
  q.prepare( "SELECT *,"
	     "       (bankaccnt_name || '-' || bankaccnt_descrip) AS f_bank,"
             "       'curr' AS bankadj_amount_xtnumericrole, "
	     "       currConcat(bankadj_curr_id) AS currabbr "
             "FROM bankadj LEFT OUTER JOIN bankaccnt ON (bankadj_bankaccnt_id=bankaccnt_id)"
             "                       LEFT OUTER JOIN bankadjtype ON (bankadj_bankadjtype_id=bankadjtype_id) "
             "WHERE ( (NOT bankadj_posted)"
             " AND (bankadj_bankaccnt_id=:bankaccnt_id) ); ");
  q.bindValue(":bankaccnt_id", _bankaccnt->id());
  q.exec();
  _adjustments->populate(q);
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void bankAdjustmentEditList::sPopulateMenu( QMenu * pMenu )
{
  QAction *menuItem;
  
  menuItem = pMenu->addAction(tr("Edit..."), this, SLOT(sEdit()));
  menuItem->setEnabled(_privileges->check("MaintainBankAdjustments"));
  
  menuItem = pMenu->addAction(tr("View..."), this, SLOT(sView()));
  menuItem->setEnabled(_privileges->check("ViewBankAdjustments"));
  
  menuItem = pMenu->addAction(tr("Delete..."), this, SLOT(sDelete()));
  menuItem->setEnabled(_privileges->check("MaintainBankAdjustments"));
  
  pMenu->addSeparator();
  
  menuItem = pMenu->addAction(tr("Post..."), this, SLOT(sPost()));
  menuItem->setEnabled(_privileges->check("PostBankAdjustments"));
}


void bankAdjustmentEditList::sPost()
{
  q.prepare("SELECT postBankAdjustment(:bankadjid) AS result;");
  q.bindValue(":bankadjid", _adjustments->id());
  q.exec();
  if (q.first())
  {
    int result = q.value("result").toInt();
    if (result < 0)
    {
      systemError(this, storedProcErrorLookup("postBankAdjustment", result),
		  __FILE__, __LINE__);
      return;
    }
    sFillList();
  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}
