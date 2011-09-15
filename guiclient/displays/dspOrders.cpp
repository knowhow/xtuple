/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspOrders.h"

#include <QAction>
#include <QMenu>
#include <QVariant>

#include "changePoitemQty.h"
#include "changeWoQty.h"
#include "printWoTraveler.h"
#include "reprioritizeWo.h"
#include "reschedulePoitem.h"
#include "rescheduleWo.h"

dspOrders::dspOrders(QWidget* parent, const char*, Qt::WFlags fl)
  : display(parent, "dspOrders", fl)
{
  setupUi(optionsWidget());
  setWindowTitle(tr("Item Orders"));
  setListLabel(tr("Item Orders"));
  setMetaSQLOptions("orders", "detail");
  setUseAltId(true);

  _item->setReadOnly(true);
  _warehouse->setEnabled(false);

  list()->addColumn(tr("Type"),         _docTypeColumn, Qt::AlignCenter, true,  "order_type" );
  list()->addColumn(tr("Order #"),      -1,             Qt::AlignLeft,   true,  "order_number"   );
  list()->addColumn(tr("Total"),        _qtyColumn,     Qt::AlignRight,  true,  "totalqty"  );
  list()->addColumn(tr("Received"),     _qtyColumn,     Qt::AlignRight,  true,  "relievedqty"  );
  list()->addColumn(tr("Balance"),      _qtyColumn,     Qt::AlignRight,  true,  "balanceqty"  );
  list()->addColumn(tr("Running Bal."), _qtyColumn,     Qt::AlignRight,  true,  "runningbalanceqty"  );
  list()->addColumn(tr("Required"),     _dateColumn,    Qt::AlignCenter, true,  "duedate" );

  if (!_metrics->boolean("MultiWhs"))
  {
    _warehouseLit->hide();
    _warehouse->hide();
  }
}

void dspOrders::languageChange()
{
  display::languageChange();
  retranslateUi(this);
}

enum SetResponse dspOrders::set(const ParameterList &pParams)
{
  XWidget::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("itemsite_id", &valid);
  if (valid)
    _item->setItemsiteid(param.toInt());

  _leadTime->setChecked(pParams.inList("byLeadTime"));

  param = pParams.value("byDate", &valid);
  if (valid)
  {
    _byDate->setChecked(true);
    _date->setDate(param.toDate());
  }

  param = pParams.value("byDays", &valid);
  if (valid)
  {
    _byDays->setChecked(true);
    _days->setValue(param.toInt());
  }

  _byRange->setChecked(pParams.inList("byRange"));

  param = pParams.value("startDate", &valid);
  if (valid)
    _startDate->setDate(param.toDate());

  param = pParams.value("endDate", &valid);
  if (valid)
    _endDate->setDate(param.toDate());

  if (pParams.inList("run"))
  {
    sFillList();
    return NoError_Run;
  }

  return NoError;
}

void dspOrders::sPopulateMenu(QMenu *pMenu, QTreeWidgetItem*, int)
{
  QAction *menuItem;

  if (list()->altId() == 1)
  {
    menuItem = pMenu->addAction(tr("Reschedule P/O Item..."), this, SLOT(sReschedulePoitem()));
    if (!_privileges->check("ReschedulePurchaseOrders"))
      menuItem->setEnabled(false);

    menuItem = pMenu->addAction(tr("Change P/O Item Quantity..."), this, SLOT(sChangePoitemQty()));
    if (!_privileges->check("ChangePurchaseOrderQty"))
      menuItem->setEnabled(false);
  }
  else if (list()->altId() == 2)
  {
    menuItem = pMenu->addAction(tr("Reprioritize W/O..."), this, SLOT(sReprioritizeWo()));
    if (!_privileges->check("ReprioritizeWorkOrders"))
      menuItem->setEnabled(false);

    menuItem = pMenu->addAction(tr("Reschedule W/O..."), this, SLOT(sRescheduleWO()));
    if (!_privileges->check("RescheduleWorkOrders"))
      menuItem->setEnabled(false);

    menuItem = pMenu->addAction(tr("Change W/O Quantity..."), this, SLOT(sChangeWOQty()));
    if (!_privileges->check("ChangeWorkOrderQty"))
      menuItem->setEnabled(false);

    menuItem = pMenu->addAction(tr("Print Traveler..."), this, SLOT(sPrintTraveler()));
    if (!_privileges->check("PrintWorkOrderPaperWork"))
      menuItem->setEnabled(false);
  }
}

void dspOrders::sReprioritizeWo()
{
  ParameterList params;
  params.append("wo_id", list()->id());

  reprioritizeWo newdlg(this, "", true);
  newdlg.set(params);
  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void dspOrders::sRescheduleWO()
{
  ParameterList params;
  params.append("wo_id", list()->id());

  rescheduleWo newdlg(this, "", true);
  newdlg.set(params);
  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void dspOrders::sChangeWOQty()
{
  ParameterList params;
  params.append("wo_id", list()->id());

  changeWoQty newdlg(this, "", true);
  newdlg.set(params);
  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void dspOrders::sPrintTraveler()
{
  ParameterList params;
  params.append("wo_id", list()->id());

  printWoTraveler newdlg(this, "", true);
  newdlg.set(params);
  newdlg.exec();
}

void dspOrders::sReschedulePoitem()
{
  ParameterList params;
  params.append("poitem_id", list()->id());

  reschedulePoitem newdlg(this, "", true);
  if(newdlg.set(params) != UndefinedError)
    if (newdlg.exec() != XDialog::Rejected)
      sFillList();
}

void dspOrders::sChangePoitemQty()
{
  ParameterList params;
  params.append("poitem_id", list()->id());

  changePoitemQty newdlg(this, "", true);
  newdlg.set(params);
  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

bool dspOrders::setParams(ParameterList & params)
{
  if ( !((_item->isValid()) &&
       ( (_leadTime->isChecked()) || (_byDays->isChecked()) ||
         ((_byDate->isChecked()) && (_date->isValid())) ||
         (_byRange->isChecked() && _startDate->isValid() && _endDate->isValid()) ) ) )
  {
    return false;
  }

  params.append("warehous_id", _warehouse->id());
  params.append("item_id",     _item->id());
  params.append("itemType",    _item->itemType());
  if (_leadTime->isChecked())
    params.append("useLeadTime");
  else if (_byDays->isChecked())
    params.append("days",      _days->value());
  else if (_byDate->isChecked())
    params.append("date",      _date->date());
  else if (_byRange->isChecked())
  {
    params.append("startDate", _startDate->date());
    params.append("endDate",   _endDate->date());
  }

  if (_metrics->value("Application") == "Standard")
    params.append("Standard");

  XSqlQuery xtmfg;
  xtmfg.exec("SELECT pkghead_name FROM pkghead WHERE pkghead_name='xtmfg'");
  if (xtmfg.first())
    params.append("Manufacturing");

  return true;
}
