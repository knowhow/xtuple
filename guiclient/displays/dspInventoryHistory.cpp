/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspInventoryHistory.h"

#include <QAction>
#include <QMenu>
#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include "adjustmentTrans.h"
#include "countTag.h"
#include "expenseTrans.h"
#include "materialReceiptTrans.h"
#include "scrapTrans.h"
#include "transactionInformation.h"
#include "transferTrans.h"
#include "workOrder.h"
#include "parameterwidget.h"

dspInventoryHistory::dspInventoryHistory(QWidget* parent, const char* name, Qt::WFlags fl)
  : display(parent, name, fl)
{
  this->setWindowTitle(tr("Inventory History"));
  setReportName("InventoryHistory");
  setMetaSQLOptions("inventoryHistory", "detail");
  setUseAltId(true);
  setParameterWidgetVisible(true);

  QString qryType;
  if (_metrics->boolean("MultiWhs"))
    qryType = QString( "SELECT  1, '%1' UNION "
                       "SELECT  2, '%2' UNION "
                       "SELECT  4, '%3' UNION "
                       "SELECT  8, '%4' UNION "
                       "SELECT  16, '%5' UNION "
                       "SELECT  32, '%6'")
    .arg(tr("Receipts"))
    .arg(tr("Issues"))
    .arg(tr("Shipments"))
    .arg(tr("Adjustments and Counts"))
    .arg(tr("Transfers"))
    .arg(tr("Scraps"));
  else
    qryType = QString( "SELECT  1, '%1' UNION "
                       "SELECT  2, '%2' UNION "
                       "SELECT  4, '%3' UNION "
                       "SELECT  8, '%4' UNION "
                       "SELECT  32, '%5'")
        .arg(tr("Receipts"))
        .arg(tr("Issues"))
        .arg(tr("Shipments"))
        .arg(tr("Adjustments and Counts"))
        .arg(tr("Scraps"));

  parameterWidget()->append(tr("Start Date"), "startDate", ParameterWidget::Date, QDate::currentDate(), true);
  parameterWidget()->append(tr("End Date"),   "endDate",   ParameterWidget::Date, QDate::currentDate(), true);
  parameterWidget()->appendComboBox(tr("Class Code"), "classcode_id", XComboBox::ClassCodes);
  parameterWidget()->append(tr("Class Code Pattern"), "classcode_pattern", ParameterWidget::Text);
  parameterWidget()->append(tr("Item"), "item_id", ParameterWidget::Item);
  parameterWidget()->appendComboBox(tr("Item Group"), "itemgrp_id", XComboBox::ItemGroups);
  parameterWidget()->append(tr("Item Group Pattern"), "itemgrp_pattern", ParameterWidget::Text);
  parameterWidget()->append(tr("Order Number Pattern"), "orderNumber", ParameterWidget::Text);
  parameterWidget()->appendComboBox(tr("Planner Code"), "plancode_id", XComboBox::PlannerCodes);
  parameterWidget()->append(tr("Planner Code Pattern"), "plancode_pattern", ParameterWidget::Text);
  parameterWidget()->append(tr("Sales Order"), "cohead_id", ParameterWidget::SalesOrder);
  parameterWidget()->appendComboBox(tr("Transaction Type"), "transType", qryType);
  if (_metrics->boolean("MultiWhs"))
    parameterWidget()->append(tr("Transfer Order"), "tohead_id", ParameterWidget::TransferOrder);
  parameterWidget()->append(tr("Work Order"), "wo_id", ParameterWidget::WorkOrder);
  if (_metrics->boolean("MultiWhs"))
    parameterWidget()->append(tr("Site"), "warehous_id", ParameterWidget::Site);

  parameterWidget()->applyDefaultFilterSet();

  list()->setRootIsDecorated(true);
  list()->addColumn(tr("Transaction Time"),_timeDateColumn, Qt::AlignLeft,  true, "invhist_transdate");
  list()->addColumn(tr("Created Time"),    _timeDateColumn, Qt::AlignLeft,  false, "invhist_created");
  list()->addColumn(tr("Site"),                 _whsColumn, Qt::AlignCenter,true, "warehous_code");
  list()->addColumn(tr("Item Number"),                  -1, Qt::AlignLeft,  true, "item_number");
  list()->addColumn(tr("Type"),               _transColumn, Qt::AlignCenter,true, "invhist_transtype");
  list()->addColumn(tr("Order #"),             _itemColumn, Qt::AlignCenter,true, "orderlocation");
  list()->addColumn(tr("UOM"),                  _uomColumn, Qt::AlignCenter,true, "invhist_invuom");
  list()->addColumn(tr("Qty"),                  _qtyColumn, Qt::AlignRight, true, "transqty");
  list()->addColumn(tr("Value"),              _moneyColumn, Qt::AlignRight, true, "transvalue");
  list()->addColumn(tr("From Area"),          _orderColumn, Qt::AlignLeft,  true, "locfrom");
  list()->addColumn(tr("QOH Before"),           _qtyColumn, Qt::AlignRight, false, "qohbefore");
  list()->addColumn(tr("To Area"),            _orderColumn, Qt::AlignLeft,  true, "locto");
  list()->addColumn(tr("QOH After"),            _qtyColumn, Qt::AlignRight, false, "qohafter");
  list()->addColumn(tr("Cost Method"),          _qtyColumn, Qt::AlignLeft,  false, "costmethod");
  list()->addColumn(tr("Value Before"),         _qtyColumn, Qt::AlignRight, false, "invhist_value_before");
  list()->addColumn(tr("Value After"),          _qtyColumn, Qt::AlignRight, false, "invhist_value_after");
  list()->addColumn(tr("User"),               _orderColumn, Qt::AlignCenter,false, "invhist_user");
}

enum SetResponse dspInventoryHistory::set(const ParameterList &pParams)
{
  XWidget::set(pParams);

  parameterWidget()->setSavedFilters();

  QVariant param;
  bool     valid;

  param = pParams.value("classcode_id", &valid);
  if (valid)
    parameterWidget()->setDefault(tr("Class Code"), param);

  param = pParams.value("classcode_pattern", &valid);
  if (valid)
    parameterWidget()->setDefault(tr("Class Code Pattern"), param);

  param = pParams.value("plancode_id", &valid);
  if (valid)
    parameterWidget()->setDefault(tr("Planner Code"), param);

  param = pParams.value("plancode_pattern", &valid);
  if (valid)
    parameterWidget()->setDefault(tr("Planner Code Pattern"), param);

  param = pParams.value("itemgrp_id", &valid);
  if (valid)
    parameterWidget()->setDefault(tr("Item Group"), param);

  param = pParams.value("itemgrp_pattern", &valid);
  if (valid)
    parameterWidget()->setDefault(tr("Item Group Pattern"), param);

  param = pParams.value("item_id", &valid);
  if (valid)
    parameterWidget()->setDefault(tr("Item"), param);

  param = pParams.value("warehous_id", &valid);
  if (valid)
  {
    if (param.toInt() > 0)
      parameterWidget()->setDefault(tr("Site"), param);
  }

  param = pParams.value("itemsite_id", &valid);
  if (valid)
  {
    XSqlQuery qry;
    qry.prepare("SELECT itemsite_item_id, itemsite_warehous_id "
                "FROM itemsite "
                "WHERE itemsite_id=:itemsite_id;");
    qry.bindValue(":itemsite_id", param.toInt());
    qry.exec();
    if (qry.first())
    {
      parameterWidget()->setDefault(tr("Item"), qry.value("itemsite_item_id"));
      parameterWidget()->setDefault(tr("Site"), qry.value("itemsite_warehous_id"));
    }
  }

  param = pParams.value("startDate", &valid);
  if (valid)
    parameterWidget()->setDefault(tr("Start Date"), param);
  else
    parameterWidget()->setDefault(tr("Start Date"), omfgThis->startOfTime());

  param = pParams.value("endDate", &valid);
  if (valid)
    parameterWidget()->setDefault(tr("End Date"), param);
  else
    parameterWidget()->setDefault(tr("End Date"), omfgThis->endOfTime());

  param = pParams.value("transtype", &valid);
  if (valid)
  {
    QString transtype = param.toString();

    if (transtype == "R")
      parameterWidget()->setDefault(tr("Transaction Type"), 1);
    else if (transtype == "I")
      parameterWidget()->setDefault(tr("Transaction Type"), 2);
    else if (transtype == "S")
      parameterWidget()->setDefault(tr("Transaction Type"), 3);
    else if (transtype == "A")
      parameterWidget()->setDefault(tr("Transaction Type"), 4);
    else if (transtype == "T")
      parameterWidget()->setDefault(tr("Transaction Type"), 5);
    else if (transtype == "SC")
      parameterWidget()->setDefault(tr("Transaction Type"), 6);
  }

  parameterWidget()->applyDefaultFilterSet();

  if (pParams.inList("run"))
    sFillList();

  return NoError;
}

bool dspInventoryHistory::setParams(ParameterList & params)
{
  if (!display::setParams(params))
    return false;

  params.append("average", tr("Average"));
  params.append("standard", tr("Standard"));
  params.append("job", tr("Job"));
  params.append("none", tr("None"));
  params.append("unknown", tr("Unknown"));

  return true;
}

void dspInventoryHistory::sViewTransInfo()
{
  QString transType(((XTreeWidgetItem *)list()->currentItem())->text(list()->column("invhist_transtype")));

  ParameterList params;
  params.append("mode", "view");
  params.append("invhist_id", list()->id());

  if (transType == "AD")
  {
    adjustmentTrans *newdlg = new adjustmentTrans();
    newdlg->set(params);
    omfgThis->handleNewWindow(newdlg);
  }
  else if (transType == "TW")
  {
    transferTrans *newdlg = new transferTrans();
    newdlg->set(params);
    omfgThis->handleNewWindow(newdlg);
  }
  else if (transType == "SI")
  {
    scrapTrans *newdlg = new scrapTrans();
    newdlg->set(params);
    omfgThis->handleNewWindow(newdlg);
  }
  else if (transType == "EX")
  {
    expenseTrans *newdlg = new expenseTrans();
    newdlg->set(params);
    omfgThis->handleNewWindow(newdlg);
  }
  else if (transType == "RX")
  {
    materialReceiptTrans *newdlg = new materialReceiptTrans();
    newdlg->set(params);
    omfgThis->handleNewWindow(newdlg);
  }
  else if (transType == "CC")
  {
    countTag newdlg(this, "", true);
    newdlg.set(params);
    newdlg.exec();
  }
  else
  {
    transactionInformation newdlg(this, "", true);
    newdlg.set(params);
    newdlg.exec();
  }
}

void dspInventoryHistory::sEditTransInfo()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("invhist_id", list()->id());

  transactionInformation newdlg(this, "", true);
  newdlg.set(params);
  newdlg.exec();
}

void dspInventoryHistory::sViewWOInfo()
{
  QString orderNumber = list()->currentItem()->text(list()->column("orderlocation"));
  int sep1            = orderNumber.indexOf('-');
  int sep2            = orderNumber.indexOf('-', (sep1 + 1));
  int mainNumber      = orderNumber.mid((sep1 + 1), ((sep2 - sep1) - 1)).toInt();
  int subNumber       = orderNumber.right((orderNumber.length() - sep2) - 1).toInt();

  XSqlQuery qq;
  qq.prepare( "SELECT wo_id "
             "FROM wo "
             "WHERE ( (wo_number=:wo_number)"
             " AND (wo_subnumber=:wo_subnumber) );" );
  qq.bindValue(":wo_number", mainNumber);
  qq.bindValue(":wo_subnumber", subNumber);
  qq.exec();
  if (qq.first())
  {
    ParameterList params;
    params.append("mode", "view");
    params.append("wo_id", qq.value("wo_id"));

    workOrder *newdlg = new workOrder();
    newdlg->set(params);
    omfgThis->handleNewWindow(newdlg);
  }
}

void dspInventoryHistory::sPopulateMenu(QMenu *pMenu, QTreeWidgetItem *pItem, int)
{
  QAction *menuItem;

  menuItem = pMenu->addAction(tr("View Transaction Information..."), this, SLOT(sViewTransInfo()));
  menuItem = pMenu->addAction(tr("Edit Transaction Information..."), this, SLOT(sEditTransInfo()));

  if ( (pItem->text(list()->column("warehous_code")).length()) &&
       ( (pItem->text(list()->column("invhist_transtype")) == "RM") || (pItem->text(list()->column("invhist_transtype")) == "IM") ) )
  {
    QString orderNumber = list()->currentItem()->text(list()->column("orderlocation"));
    int sep1            = orderNumber.indexOf('-');
    int sep2            = orderNumber.indexOf('-', (sep1 + 1));
    int mainNumber      = orderNumber.mid((sep1 + 1), ((sep2 - sep1) - 1)).toInt();
    int subNumber       = orderNumber.right((orderNumber.length() - sep2) - 1).toInt();

    if ( (mainNumber) && (subNumber) )
    {
      q.prepare( "SELECT wo_id "
                 "FROM wo "
                 "WHERE ( (wo_number=:wo_number)"
                 " AND (wo_subnumber=:wo_subnumber) );" );
      q.bindValue(":wo_number", mainNumber);
      q.bindValue(":wo_subnumber", subNumber);
      q.exec();
      if (q.first())
      {
        menuItem = pMenu->addAction(tr("View Work Order Information..."), this, SLOT(sViewWOInfo()));
        menuItem->setEnabled(_privileges->check("MaintainWorkOrders") || _privileges->check("ViewWorkOrders"));
      }
    }
  }
}

