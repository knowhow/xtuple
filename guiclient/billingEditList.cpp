/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "billingEditList.h"

#include <QAction>
#include <QMenu>
#include <QMessageBox>
#include <QSqlError>

#include <openreports.h>

#include "printInvoices.h"
#include "selectBillingQty.h"
#include "selectOrderForBilling.h"
#include "storedProcErrorLookup.h"

billingEditList::billingEditList(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  connect(_cobill, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*,QTreeWidgetItem*)));
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));

  _cobill->setRootIsDecorated(TRUE);
  _cobill->addColumn(tr("Document #"), (_itemColumn+ _cobill->indentation()),  Qt::AlignLeft, true, "documentnumber");
  _cobill->addColumn(tr("Order #"),    _orderColumn, Qt::AlignLeft,  true, "ordernumber");
  _cobill->addColumn(tr("Cust./Item #"),_itemColumn, Qt::AlignLeft,  true, "shortname");
  _cobill->addColumn(tr("Name/Description"),     -1, Qt::AlignLeft,  true, "longname");
  _cobill->addColumn(tr("UOM"),          _uomColumn, Qt::AlignCenter,true, "iteminvuom");
  _cobill->addColumn(tr("Qty. to Bill"), _qtyColumn, Qt::AlignRight, true, "qtytobill");
  _cobill->addColumn(tr("Price"),       _costColumn, Qt::AlignRight, true, "price");
  _cobill->addColumn(tr("Ext. Price"), _moneyColumn, Qt::AlignRight, true, "extprice");
  _cobill->addColumn(tr("Currency"),_currencyColumn, Qt::AlignLeft,  true, "currabbr");

  connect(omfgThis, SIGNAL(billingSelectionUpdated(int, int)), this, SLOT(sFillList()));

  sFillList();
}

billingEditList::~billingEditList()
{
  // no need to delete child widgets, Qt does it all for us
}

void billingEditList::languageChange()
{
  retranslateUi(this);
}

void billingEditList::sEditBillingOrd()
{
  if (!checkSitePrivs(_cobill->id()))
    return;
  
  ParameterList params;
  params.append("mode", "edit");
  params.append("sohead_id", _cobill->id());

  selectOrderForBilling *newdlg = new selectOrderForBilling();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void billingEditList::sCancelBillingOrd()
{
  if (!checkSitePrivs(_cobill->id()))
    return;
  
  if ( QMessageBox::critical( this, tr("Cancel Billing"),
                              tr("Are you sure that you want to cancel billing for the selected order?"),
                              tr("&Yes"), tr("&No"), QString::null, 0, 1) == 0)
  {
    q.prepare( "SELECT cancelBillingSelection(cobmisc_id) AS result "
               "FROM cobmisc "
               "WHERE (cobmisc_cohead_id=:sohead_id);" );
    q.bindValue(":sohead_id", _cobill->id());
    q.exec();
    if (q.first())
    {
      int result = q.value("result").toInt();
      if (result < 0)
      {
        systemError(this, storedProcErrorLookup("cancelBillingSelection", result), __FILE__, __LINE__);
        return;
      }
    }
    else if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }

    sFillList();
  }
}

void billingEditList::sEditBillingQty()
{ 
  if (!checkSitePrivs(_cobill->id()))
    return;
  
  ParameterList params;
  params.append("soitem_id", _cobill->altId());

  selectBillingQty newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void billingEditList::sCancelBillingQty()
{
}

void billingEditList::sPopulateMenu(QMenu *pMenu, QTreeWidgetItem *pSelected)
{
  _orderid = _cobill->id();

  if (((XTreeWidgetItem *)pSelected)->altId() == -2)
  {
    _itemtype = 1;
    pMenu->addAction("Edit Billing...", this, SLOT(sEditBillingOrd()));
    pMenu->addAction("Cancel Billing...", this, SLOT(sCancelBillingOrd()));
  }
  else if (((XTreeWidgetItem *)pSelected)->altId() != -1)
  {
    _itemtype = 2;
    pMenu->addAction("Edit Billing...", this, SLOT(sEditBillingQty()), 0);
    pMenu->addAction("Cancel Billing...", this, SLOT(sCancelBillingQty()));
  }
}

void billingEditList::sFillList()
{
  /* indent: order
                line item
                  credit account
                line item
                  credit account
                debit account for entire order
  */
  q.exec("SELECT orderid, itemid, seq, documentnumber, ordernumber,"
         "       shortname, longname, iteminvuom, qtytobill, price, extprice,"
         "       currConcat(curr_id) AS currabbr,"
         "       'qty'        AS qtytobill_xtnumericrole,"
         "       'salesprice' AS price_xtnumericrole,"
         "       'extprice'   AS extprice_xtnumericrole,"
         "       CASE WHEN seq = 3 THEN 1"
         "            ELSE seq END AS xtindentrole,"
         "       CASE WHEN account = 'Not Assigned' THEN 'error'"
         "       END AS qtforegroundrole "
         "FROM ("
         "  SELECT orderid, itemid, 0 AS seq,"      // order info
         "        documentnumber, ordernumber, cust_number AS shortname,"
         "        billtoname AS longname, '' AS iteminvuom, NULL AS qtytobill,"
         "        NULL AS price, NULL AS extprice, account, NULL AS curr_id,"
         "        ordernumber AS sortord, linenumber "
         "  FROM billingEditList "
         "  WHERE (itemid = -2) "
         "  UNION "       // line item info
         "  SELECT orderid, itemid, 1,"
         "        documentnumber, '', item,"
         "        itemdescrip, iteminvuom, qtytobill, price, extprice, account, curr_id,"
         "        ordernumber, linenumber "
         "  FROM billingEditList "
         "  WHERE (itemid >= 0) "
         "  UNION "       // credits
         "  SELECT orderid, itemid, 2,"
         "         documentnumber, '', sence,"
         "         account, '', NULL, NULL, extprice, account, curr_id,"
         "        ordernumber, linenumber "
         "  FROM billingEditList "
         "  WHERE (itemid >= 0) "
         "  UNION "       // debits
         "  SELECT orderid, itemid, 3,"
         "         '', '', sence,"
         "         account, '', NULL, NULL, extprice, account, curr_id,"
         "        ordernumber, 9999999999 "
         "  FROM billingEditList "
         "  WHERE (itemid = -2) "
         ") AS sub "
         "ORDER BY sortord, linenumber, seq;");
  _cobill->populate(q, true);
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void billingEditList::sPrint()
{
  orReport report("BillingEditList");
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

bool billingEditList::checkSitePrivs(int ordid)
{
  if (_preferences->boolean("selectedSites"))
  {
    XSqlQuery check;
    check.prepare("SELECT checkSOSitePrivs(:coheadid) AS result;");
    check.bindValue(":coheadid", ordid);
    check.exec();
    if (check.first())
    {
      if (!check.value("result").toBool())
      {
        QMessageBox::critical(this, tr("Access Denied"),
                              tr("You may not view or edit this Billing as it references "
                                 "a Site for which you have not been granted privileges.")) ;
        return false;
      }
    }
  }
  return true;
}
