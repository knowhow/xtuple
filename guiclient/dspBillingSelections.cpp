/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspBillingSelections.h"

#include <QAction>
#include <QMenu>
#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include <metasql.h>
#include "mqlutil.h"

#include <openreports.h>
#include "selectOrderForBilling.h"
#include "printInvoices.h"
#include "createInvoices.h"

dspBillingSelections::dspBillingSelections(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  connect(_close, SIGNAL(clicked()), this, SLOT(close()));
  connect(_cobill, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*,QTreeWidgetItem*)));
  connect(_post, SIGNAL(clicked()), this, SLOT(sPost()));
  connect(_edit, SIGNAL(clicked()), this, SLOT(sEdit()));
  connect(_cancel, SIGNAL(clicked()), this, SLOT(sCancel()));
  connect(_new, SIGNAL(clicked()), this, SLOT(sNew()));
  connect(_postAll, SIGNAL(clicked()), this, SLOT(sPostAll()));
  connect(_cobill, SIGNAL(valid(bool)), _edit, SLOT(setEnabled(bool)));
  connect(_cobill, SIGNAL(valid(bool)), _cancel, SLOT(setEnabled(bool)));
  connect(_cobill, SIGNAL(itemSelected(int)), _edit, SLOT(animateClick()));
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));

  _cobill->addColumn(tr("Order #"),       _orderColumn,  Qt::AlignLeft,   true,  "cohead_number"   );
  _cobill->addColumn(tr("Cust. #"),       _itemColumn,   Qt::AlignLeft,   true,  "cust_number"   );
  _cobill->addColumn(tr("Name"),          -1,            Qt::AlignLeft,   true,  "cust_name"   );
  _cobill->addColumn(tr("Subtotal"),      _moneyColumn,  Qt::AlignLeft,   false, "subtotal" );
  _cobill->addColumn(tr("Misc."),         _moneyColumn,  Qt::AlignLeft,   true, "cobmisc_misc" ); 
  _cobill->addColumn(tr("Freight"),       _moneyColumn,  Qt::AlignLeft,   true, "cobmisc_freight" );
  _cobill->addColumn(tr("Tax"),           _moneyColumn,  Qt::AlignLeft,   true, "cobmisc_tax" );
  _cobill->addColumn(tr("Total"),         _moneyColumn,  Qt::AlignLeft,   true, "total" );
  _cobill->addColumn(tr("Payment Rec'd"), _bigMoneyColumn,  Qt::AlignLeft,   true, "cobmisc_payment" );

  if (_privileges->check("PostARDocuments"))
    connect(_cobill, SIGNAL(valid(bool)), _post, SLOT(setEnabled(bool)));

  connect(omfgThis, SIGNAL(billingSelectionUpdated(int, int)), this, SLOT(sFillList()));

  sFillList();
}

dspBillingSelections::~dspBillingSelections()
{
    // no need to delete child widgets, Qt does it all for us
}

void dspBillingSelections::languageChange()
{
    retranslateUi(this);
}

void dspBillingSelections::sPopulateMenu(QMenu *pMenu, QTreeWidgetItem *)
{
  QAction *menuItem;

  pMenu->addAction("Edit...", this, SLOT(sEdit()));
  pMenu->addAction("Cancel...", this, SLOT(sCancel()));

  menuItem = pMenu->addAction("Create Invoice", this, SLOT(sPost()));
  menuItem->setEnabled(_privileges->check("PostARDocuments"));
}

void dspBillingSelections::sFillList()
{
  MetaSQLQuery mql = mqlLoad("billingSelections", "detail");
  ParameterList params;
  q = mql.toQuery(params);
  _cobill->populate(q);
}

void dspBillingSelections::sPostAll()
{
  createInvoices newdlg(this, "", TRUE);
  newdlg.exec();
}

void dspBillingSelections::sPost()
{
  int soheadid = -1;
  q.prepare("SELECT cobmisc_cohead_id AS sohead_id "
            "FROM cobmisc "
            "WHERE (cobmisc_id = :cobmisc_id)");
  q.bindValue(":cobmisc_id", _cobill->id());
  q.exec();
  if (q.first())
  {
    soheadid = q.value("sohead_id").toInt();
  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  q.prepare("SELECT createInvoice(:cobmisc_id) AS result;");
  q.bindValue(":cobmisc_id", _cobill->id());
  q.exec();
  if (q.first())
  {
    int result = q.value("result").toInt();

    if (result == -5)
      QMessageBox::critical( this, tr("Cannot Create one or more Invoices"),
                             tr( "The G/L Account Assignments for the selected Invoice are not configured correctly.\n"
                                 "Because of this, G/L Transactions cannot be created for this Invoices.\n"
                                 "You must contact your Systems Administrator to have this corrected before you may Create this Invoice." ) );
    else if (result < 0)
      systemError( this, tr("A System Error occurred at %1::%2, Error #%3.")
                         .arg(__FILE__)
                         .arg(__LINE__)
                         .arg(q.value("result").toInt()) );

    omfgThis->sInvoicesUpdated(result, TRUE);
    omfgThis->sSalesOrdersUpdated(soheadid);
    omfgThis->sBillingSelectionUpdated(soheadid, TRUE);

    sFillList();
  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void dspBillingSelections::sNew()
{
  ParameterList params;
  params.append("mode", "new");

  selectOrderForBilling *newdlg = new selectOrderForBilling();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspBillingSelections::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");

  q.prepare("SELECT cobmisc_cohead_id AS sohead_id "
            "FROM cobmisc "
            "WHERE (cobmisc_id = :cobmisc_id)");
  q.bindValue(":cobmisc_id", _cobill->id());
  q.exec();
  if (q.first())
  {
    int sohead_id = q.value("sohead_id").toInt();
    params.append("sohead_id", sohead_id);
  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  selectOrderForBilling *newdlg = new selectOrderForBilling();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}


void dspBillingSelections::sCancel()
{
  if ( QMessageBox::critical( this, tr("Cancel Billing"),
                              tr("Are you sure that you want to cancel billing for the selected order?"),
                              tr("&Yes"), tr("&No"), QString::null, 0, 1) == 0)
  {
    q.prepare( "SELECT cancelBillingSelection(cobmisc_id) AS result "
               "FROM cobmisc "
               "WHERE (cobmisc_id=:cobmisc_id);" );
    q.bindValue(":cobmisc_id", _cobill->id());
    q.exec();

    sFillList();
  }
}


void dspBillingSelections::sPrint()
{
  orReport report("BillingSelections");
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

