/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspInventoryHistoryBase.h"

#include <QAction>
#include <QMenu>
#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include "adjustmentTrans.h"
#include "countTag.h"
#include "expenseTrans.h"
#include "materialReceiptTrans.h"
#include "ordercluster.h"
#include "scrapTrans.h"
#include "transactionInformation.h"
#include "transferTrans.h"
#include "workOrder.h"

// TODO: handle RA?
// TODO: is there a way to handle RA and TO entirely in scripts?
dspInventoryHistoryBase::dspInventoryHistoryBase(QWidget* parent, const char* name, Qt::WFlags fl)
  : display(parent, name, fl)
{
  setupUi(optionsWidget());
  setListLabel(tr("Inventory History"));
  setReportName("InventoryHistory");
  setMetaSQLOptions("inventoryHistory", "detail");
  setUseAltId(true);

  connect(_orderList,   SIGNAL(clicked()),          this, SLOT(sOrderList()));
  connect(_orderNumber, SIGNAL(requestList()),      this, SLOT(sOrderList()));
  connect(_orderType,   SIGNAL(valid(bool)),  _orderList, SLOT(setEnabled(bool)));
  connect(_orderType,   SIGNAL(newID(int)), _orderNumber, SLOT(clear()));

#ifndef Q_WS_MAC
  _orderList->setMaximumWidth(25);
#endif

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

  _transType->append(cTransAll,       tr("All Transactions")       );
  _transType->append(cTransReceipts,  tr("Receipts")               );
  _transType->append(cTransIssues,    tr("Issues")                 );
  _transType->append(cTransShipments, tr("Shipments")              );
  _transType->append(cTransAdjCounts, tr("Adjustments and Counts") );
  
  _orderType->setAllowNull(true);
  _orderType->setNullStr(tr("All Orders"));
  _orderType->append(1, tr("Sales Orders"),      "SO" );
  _orderType->append(2, tr("Purchase Orders"),   "PO" );
  _orderType->append(3, tr("Work Orders"),       "WO");
  
  if (_metrics->boolean("MultiWhs"))
  {
    _transType->append(cTransTransfers, tr("Transfers") );
    _orderType->append(4, tr("Transfer Orders"), "TO" );
  }
  
  _transType->append(cTransScraps,    tr("Scraps")                 );
  _transType->setCurrentIndex(0);
  _orderType->setId(-1);

  _dates->setStartNull(tr("Earliest"), omfgThis->startOfTime(), true);                                                     
  _dates->setEndNull(tr("Latest"), omfgThis->endOfTime(), true);
}

void dspInventoryHistoryBase::languageChange()
{
  display::languageChange();
  retranslateUi(this);
}

enum SetResponse dspInventoryHistoryBase::set(const ParameterList &pParams)
{
  XWidget::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("classcode_id", &valid);
  if (valid)
  {
    _parameter->setType(ParameterGroup::ClassCode);
    _parameter->setId(param.toInt());
  }

  param = pParams.value("classcode_pattern", &valid);
  if (valid)
  {
    _parameter->setType(ParameterGroup::ClassCode);
    _parameter->setPattern(param.toString());
  }

  param = pParams.value("classcode", &valid);
  if (valid)
    _parameter->setType(ParameterGroup::ClassCode);

  param = pParams.value("plancode_id", &valid);
  if (valid)
  {
    _parameter->setType(ParameterGroup::PlannerCode);
    _parameter->setId(param.toInt());
  }

  param = pParams.value("plancode_pattern", &valid);
  if (valid)
  {
    _parameter->setType(ParameterGroup::PlannerCode);
    _parameter->setPattern(param.toString());
  }

  param = pParams.value("plancode", &valid);
  if (valid)
    _parameter->setType(ParameterGroup::PlannerCode);

  param = pParams.value("itemgrp_id", &valid);
  if (valid)
  {
    _parameter->setType(ParameterGroup::ItemGroup);
    _parameter->setId(param.toInt());
  }

  param = pParams.value("itemgrp_pattern", &valid);
  if (valid)
  {
    _parameter->setType(ParameterGroup::ItemGroup);
    _parameter->setPattern(param.toString());
  }

  param = pParams.value("itemgrp", &valid);
  if (valid)
    _parameter->setType(ParameterGroup::ItemGroup);

  param = pParams.value("item_id", &valid);
  if (valid)
    _item->setId(param.toInt());

  param = pParams.value("warehous_id", &valid);
  if (valid)
    _warehouse->setId(param.toInt());

  param = pParams.value("itemsite_id", &valid);
  if (valid)
  {
    _item->setItemsiteid(param.toInt());
    _dates->setFocus();
  }

  param = pParams.value("startDate", &valid);
  if (valid)
    _dates->setStartDate(param.toDate());

  param = pParams.value("endDate", &valid);
  if (valid)
    _dates->setEndDate(param.toDate());

  param = pParams.value("transtype", &valid);
  if (valid)
  {
    QString transtype = param.toString();

    if (transtype == "R")
      _transType->setCurrentIndex(1);
    else if (transtype == "I")
      _transType->setCurrentIndex(2);
    else if (transtype == "S")
      _transType->setCurrentIndex(3);
    else if (transtype == "A")
      _transType->setCurrentIndex(4);
    else if (transtype == "T")
      _transType->setCurrentIndex(5);
    else if (transtype == "SC")
      _transType->setCurrentIndex(6);
  }

  param = pParams.value("ordertype", &valid);
  if (valid)
    _orderType->setCode(param.toString());

  if (pParams.inList("run"))
    sFillList();

  if(_parameter->isVisibleTo(this))
  {
    switch (_parameter->type())
    {
      case ParameterGroup::ClassCode:
        setWindowTitle(tr("Inventory History by Class Code"));
        break;
  
      case ParameterGroup::PlannerCode:
        setWindowTitle(tr("Inventory History by Planner Code"));
        break;
  
      case ParameterGroup::ItemGroup:
        setWindowTitle(tr("Inventory History by Item Group"));
        break;
  
      default:
        break;
    }
  }

  return NoError;
}

bool dspInventoryHistoryBase::setParams(ParameterList & params)
{
  if(_itemGroup->isVisibleTo(this) && !_item->isValid())
  {
    QMessageBox::critical( this, tr("Item Number Required"),
      tr("You must specify an Item Number."));
    return false;
  }

  if (_orderNumber->isVisibleTo(this) && _orderNumber->text().trimmed().length() == 0)
  {
    QMessageBox::critical( this, tr("Enter Order Search Pattern"),
                           tr("You must enter a Order # pattern to search for." ) );
    _orderNumber->setFocus();
    return false;
  }

  if (!_dates->startDate().isValid())
  {
    QMessageBox::critical( this, tr("Enter Start Date"),
                           tr("Please enter a valid Start Date.") );
    _dates->setFocus();
    return false;
  }

  if (!_dates->endDate().isValid())
  {
    QMessageBox::critical( this, tr("Enter End Date"),
                           tr("Please enter a valid End Date.") );
    _dates->setFocus();
    return false;
  }

  _warehouse->appendValue(params);
  _dates->appendValue(params);
  params.append("transType", _transType->id());

  if(_itemGroup->isVisibleTo(this))
  {
    params.append("item_id", _item->id());
    params.append("includeFormatted"); // ??? originally from dspInventoryHistoryByItem::sFillList()
  }

  if(_orderNumber->isVisibleTo(this))
    params.append("orderNumber", _orderNumber->text());

  if (_orderType->isVisibleTo(this) && _orderType->isValid())
    params.append("orderType", _orderType->code());

  if(_parameter->isVisibleTo(this))
  {
    _parameter->appendValue(params);

    if (_parameter->type() == ParameterGroup::ItemGroup)
      params.append("itemgrp");
    else if(_parameter->type() == ParameterGroup::PlannerCode)
      params.append("plancode");
    else
      params.append("classcode");
  }

  params.append("average", tr("Average"));
  params.append("standard", tr("Standard"));
  params.append("job", tr("Job"));
  params.append("none", tr("None"));
  params.append("unknown", tr("Unknown"));

  return true;
}

void dspInventoryHistoryBase::sViewTransInfo()
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

void dspInventoryHistoryBase::sEditTransInfo()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("invhist_id", list()->id());

  transactionInformation newdlg(this, "", true);
  newdlg.set(params);
  newdlg.exec();
}

void dspInventoryHistoryBase::sViewWOInfo()
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

void dspInventoryHistoryBase::sPopulateMenu(QMenu *pMenu, QTreeWidgetItem *pItem, int)
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

void dspInventoryHistoryBase::sOrderList()
{
  // TODO: simplify when the order line edit can handle wo.
  OrderLineEdit tmpOrderLE(this, "tmpOrderLE");
  ParameterList params;
  if (_orderType->code() == "PO" ||
      _orderType->code() == "SO" ||
      _orderType->code() == "TO")
  {
    tmpOrderLE.setAllowedType(_orderType->code());
    tmpOrderLE.setAllowedStatuses(OrderLineEdit::Open);
    tmpOrderLE.sList();
    if (tmpOrderLE.isValid())
      _orderNumber->setText(tmpOrderLE.text());

  }
  else if (_orderType->code() == "WO")
  {
    params.append("woType", cWoOpen);

    woList *newdlg = new woList(_orderNumber);
    newdlg->set(params);

    int id = newdlg->exec();
    if(id != QDialog::Rejected)
    {
      XSqlQuery woq;
      woq.prepare("SELECT formatWoNumber(:id) AS result;");
      woq.bindValue(":id", id);
      woq.exec();
      if (woq.first())
        _orderNumber->setText(woq.value("result").toString());
      else if (woq.lastError().type() != QSqlError::NoError)
      {
        QMessageBox::critical(this, tr("Database Error"), woq.lastError().text());
        return;
      }
    }
  }
}
