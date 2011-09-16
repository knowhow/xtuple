/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspMRPDetail.h"

#include <QAction>
#include <QMenu>
#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include <datecluster.h>
#include <openreports.h>
#include <metasql.h>

#include "dspAllocations.h"
#include "dspOrders.h"
#include "purchaseOrder.h"
#include "purchaseRequest.h"
#include "workOrder.h"
#include "mqlutil.h"

dspMRPDetail::dspMRPDetail(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  connect(_itemsite, SIGNAL(itemSelected(int)), this, SLOT(sFillMRPDetail()));
  connect(_itemsite, SIGNAL(itemSelectionChanged()), this, SLOT(sFillMRPDetail()));
  connect(_mrp, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*,QTreeWidgetItem*,int)));
  connect(_plannerCode, SIGNAL(updated()), this, SLOT(sFillItemsites()));
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_warehouse, SIGNAL(updated()), this, SLOT(sFillItemsites()));

  _plannerCode->setType(ParameterGroup::PlannerCode);

  _itemsite->addColumn("Itemtype",        0,           Qt::AlignCenter,false,"item_type");
  _itemsite->addColumn(tr("Item Number"), _itemColumn, Qt::AlignLeft,  true, "item_number");
  _itemsite->addColumn(tr("Description"), -1,          Qt::AlignLeft,  true, "descrip");
  _itemsite->addColumn(tr("Site"),        _whsColumn,  Qt::AlignCenter,true, "warehous_code");

  _mrp->addColumn("", 120, Qt::AlignRight);

  sFillItemsites();
}

dspMRPDetail::~dspMRPDetail()
{
  // no need to delete child widgets, Qt does it all for us
}

void dspMRPDetail::languageChange()
{
  retranslateUi(this);
}

void dspMRPDetail::sPrint()
{
  if ( (_periods->isPeriodSelected()) && (_itemsite->id() != -1) )
  {
    XSqlQuery wsq ( QString( "SELECT mrpReport(%1, '%2') as worksetid;")
                    .arg(_itemsite->id())
                    .arg(_periods->periodString()) );
    if(wsq.first())
    {
      ParameterList params;

      _plannerCode->appendValue(params);
      _warehouse->appendValue(params);

      QList<XTreeWidgetItem*> selected = _periods->selectedItems();
      QList<QVariant> periodList;
      for (int i = 0; i < selected.size(); i++)
	periodList.append(((XTreeWidgetItem*)selected[i])->id());

      params.append("period_id_list", periodList);
      params.append("itemsite_id", _itemsite->id());
      params.append("workset_id", wsq.value("worksetid").toInt());

      orReport report("MRPDetail", params);
      if (report.isValid())
        report.print();
      else
      {
        report.reportError(this);
        return;
      }

      XSqlQuery dwsq( QString( "SELECT deleteMPSMRPWorkset(%1) as result")
                      .arg(wsq.value("worksetid").toInt()) );

    }
    else if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }
  else
    QMessageBox::critical(this, tr("Incomplete criteria"),
                          tr("<p>The criteria you specified is not complete. "
			     "Please make sure all fields are correctly filled "
			     "out before running the report." ) );
}

void dspMRPDetail::sPopulateMenu(QMenu *pMenu, QTreeWidgetItem *, int pColumn)
{
  QAction *menuItem;
  int      mrpIndex = 0;

  _column = pColumn;

  menuItem = pMenu->addAction(tr("View Allocations..."), this, SLOT(sViewAllocations()));
  while ((mrpIndex < _mrp->topLevelItemCount()) &&
	 (_mrp->topLevelItem(mrpIndex)->text(0) != tr("Allocations")))
    mrpIndex++;
  if (_mrp->topLevelItem(mrpIndex)->text(pColumn).toDouble() == 0.0)
    menuItem->setEnabled(false);

  menuItem = pMenu->addAction(tr("View Orders..."), this, SLOT(sViewOrders()));
  while ((mrpIndex < _mrp->topLevelItemCount()) &&
	 (_mrp->topLevelItem(mrpIndex)->text(0) != tr("Orders")))
    mrpIndex++;
  if (_mrp->topLevelItem(mrpIndex)->text(pColumn).toDouble() == 0.0)
    menuItem->setEnabled(false);

  pMenu->addSeparator();

  if (_itemsite->currentItem()->text(0) == "P")
  {
    menuItem = pMenu->addAction(tr("Issue P/R..."), this, SLOT(sIssuePR()));
    if (!_privileges->check("MaintainPurchaseRequests"))
      menuItem->setEnabled(false);

    menuItem = pMenu->addAction(tr("Issue P/O..."), this, SLOT(sIssuePO()));
    if (!_privileges->check("MaintainPurchaseOrders"))
      menuItem->setEnabled(false);
  }
  else if (_itemsite->currentItem()->text(0) == "M")
  {
    menuItem = pMenu->addAction(tr("Issue W/O..."), this, SLOT(sIssueWO()));
    if (!_privileges->check("MaintainWorkOrders"))
      menuItem->setEnabled(false);
  }
}

void dspMRPDetail::sViewAllocations()
{
  ParameterList params;
  params.append("itemsite_id", _itemsite->id());
  params.append("byRange");
  params.append("startDate", _periods->getSelected(_column)->startDate());
  params.append("endDate", _periods->getSelected(_column)->endDate());
  params.append("run");

  dspAllocations *newdlg = new dspAllocations();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspMRPDetail::sViewOrders()
{
  ParameterList params;
  params.append("itemsite_id", _itemsite->id());
  params.append("byRange");
  params.append("startDate", _periods->getSelected(_column)->startDate());
  params.append("endDate", _periods->getSelected(_column)->endDate());
  params.append("run");

  dspOrders *newdlg = new dspOrders();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspMRPDetail::sIssuePR()
{
  ParameterList params;
  params.append("mode", "new");
  params.append("itemsite_id", _itemsite->id());

  purchaseRequest newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillMRPDetail();
}

void dspMRPDetail::sIssuePO()
{
  ParameterList params;
  params.append("mode", "new");
  params.append("itemsite_id", _itemsite->id());

  purchaseOrder *newdlg = new purchaseOrder();
  if(newdlg->set(params) == NoError)
    omfgThis->handleNewWindow(newdlg);
}

void dspMRPDetail::sIssueWO()
{
  ParameterList params;
  params.append("mode", "new");
  params.append("itemsite_id", _itemsite->id());

  workOrder *newdlg = new workOrder();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspMRPDetail::sFillItemsites()
{
  ParameterList params;

  if (! setParams(params))
    return;

  MetaSQLQuery mql = mqlLoad("mrpDetail", "item");

  q = mql.toQuery(params);
  _itemsite->populate(q, true);
}

void dspMRPDetail::sFillMRPDetail()
{
  _mrp->clear();

  _mrp->setColumnCount(1);

  QList<XTreeWidgetItem*> selected = _periods->selectedItems();
  for (int i = 0; i < selected.size(); i++)
  {
    XTreeWidgetItem *cursor = (XTreeWidgetItem*)selected[i];
    _mrp->addColumn(formatDate(((PeriodListViewItem *)cursor)->startDate()), _qtyColumn, Qt::AlignRight);
  }

  XTreeWidgetItem *qoh;
  XTreeWidgetItem *allocations;
  XTreeWidgetItem *orders;
  XTreeWidgetItem *availability;
  XTreeWidgetItem *firmedAllocations;
  XTreeWidgetItem *firmedOrders;
  XTreeWidgetItem *firmedAvailability;
  double        runningAvailability = 0.0;
  double	runningFirmed = 0.0;
  int           counter = 0;

  for (int i = 0; i < selected.size(); i++)
  {
    XTreeWidgetItem *cursor = (XTreeWidgetItem*)selected[i];
    counter++;
    ParameterList params;
    params.append("cursorId", cursor->id());
    params.append("counter", counter);
    params.append("itemsite_id", _itemsite->id());

    MetaSQLQuery mql = mqlLoad("mrpDetail", "detail");
    q = mql.toQuery(params);
    if (q.first())
    {
      if (counter == 1)
      {
        runningFirmed = q.value("firmedorders1").toDouble();
        runningAvailability = (q.value("itemsite_qtyonhand").toDouble() - q.value("allocations1").toDouble() + q.value("orders1").toDouble());

        qoh                = new XTreeWidgetItem(_mrp, 0, QVariant(tr("Projected QOH")), q.value("f_qoh"));
        allocations        = new XTreeWidgetItem(_mrp, qoh, 0, QVariant(tr("Allocations")), formatQty(q.value("allocations1").toDouble()));
        orders             = new XTreeWidgetItem(_mrp, allocations,  0, QVariant(tr("Orders")), formatQty(q.value("orders1").toDouble()));
        availability       = new XTreeWidgetItem(_mrp, orders, 0, QVariant(tr("Availability")), formatQty(runningAvailability));
        firmedAllocations  = new XTreeWidgetItem(_mrp, availability, 0, QVariant(tr("Firmed Allocations")), formatQty(q.value("firmedallocations1").toDouble()));
        firmedOrders       = new XTreeWidgetItem(_mrp, firmedAllocations, 0, QVariant(tr("Firmed Orders")), formatQty(runningFirmed));
        firmedAvailability = new XTreeWidgetItem(_mrp, firmedOrders, 0, QVariant(tr("Firmed Availability")), formatQty( runningAvailability -
                                                                                                            q.value("firmedallocations1").toDouble() +
                                                                                                            runningFirmed ) );
      }
      else
      {
        qoh->setText(counter, formatQty(runningAvailability));
        allocations->setText(counter, formatQty(q.value(QString("allocations%1").arg(counter)).toDouble()));
        orders->setText(counter, formatQty(q.value(QString("orders%1").arg(counter)).toDouble()));

        runningAvailability =  ( runningAvailability - q.value(QString("allocations%1").arg(counter)).toDouble() +
                                                       q.value(QString("orders%1").arg(counter)).toDouble() );
        availability->setText(counter, formatQty(runningAvailability));

        firmedAllocations->setText(counter, formatQty(q.value(QString("firmedallocations%1").arg(counter)).toDouble()));

        runningFirmed += q.value(QString("firmedorders%1").arg(counter)).toDouble();
        firmedOrders->setText(counter, formatQty(runningFirmed));
        firmedAvailability->setText(counter, formatQty( runningAvailability -
                                                              q.value(QString("firmedallocations%1").arg(counter)).toDouble() +
                                                              runningFirmed ) );
      }
    }
  }
}

bool dspMRPDetail::setParams(ParameterList &params)
{

  _plannerCode->appendValue(params);

  if (_warehouse->isSelected())
    params.append("warehous_id", _warehouse->id());

  return true;
}
