/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspSubstituteAvailabilityByItem.h"

#include <QAction>
#include <QAction>
#include <QAction>
#include <QAction>
#include <QMenu>
#include <QVariant>
#include <QMessageBox>

#include "dspAllocations.h"
#include "dspOrders.h"

dspSubstituteAvailabilityByItem::dspSubstituteAvailabilityByItem(QWidget* parent, const char*, Qt::WFlags fl)
  : display(parent, "dspSubstituteAvailabilityByItem", fl)
{
  setupUi(optionsWidget());
  setWindowTitle(tr("Substitute Availability by Root Item"));
  setListLabel(tr("Substitute Availability"));
  setReportName("SubstituteAvailabilityByRootItem");
  setMetaSQLOptions("substituteAvailability", "detail");
  setUseAltId(true);

  _showByGroupInt = new QButtonGroup(this);
  _showByGroupInt->addButton(_leadTime);
  _showByGroupInt->addButton(_byDays);
  _showByGroupInt->addButton(_byDate);

  list()->addColumn(tr("Site"),          _whsColumn,  Qt::AlignCenter, true,  "warehous_code" );
  list()->addColumn(tr("Item Number"),   _itemColumn, Qt::AlignLeft,   true,  "item_number"   );
  list()->addColumn(tr("Description"),   -1,          Qt::AlignLeft,   true,  "itemdescrip"   );
  list()->addColumn(tr("LT"),            _whsColumn,  Qt::AlignCenter, true,  "leadtime" );
  list()->addColumn(tr("QOH"),           _qtyColumn,  Qt::AlignRight,  true,  "qtyonhand"  );
  list()->addColumn(tr("Allocated"),     _qtyColumn,  Qt::AlignRight,  true,  "allocated"  );
  list()->addColumn(tr("On Order"),      _qtyColumn,  Qt::AlignRight,  true,  "ordered"  );
  list()->addColumn(tr("Reorder Lvl."),  _qtyColumn,  Qt::AlignRight,  true,  "reorderlevel"  );
  list()->addColumn(tr("Available"),     _qtyColumn,  Qt::AlignRight,  true,  "available"  );
}

void dspSubstituteAvailabilityByItem::languageChange()
{
  display::languageChange();
  retranslateUi(this);
}

enum SetResponse dspSubstituteAvailabilityByItem::set(const ParameterList &pParams)
{
  XWidget::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("item_id", &valid);
  if (valid)
  {
    _item->setId(param.toInt());
    _item->setReadOnly(true);
  }

  param = pParams.value("warehous_id", &valid);
  if (valid)
  {
    _warehouse->setId(param.toInt());
    _warehouse->setEnabled(false);
  }

  param = pParams.value("itemsite_id", &valid);
  if (valid)
  {
    _item->setItemsiteid(param.toInt());
    _item->setReadOnly(true);
    _warehouse->setEnabled(false);
  }

  _leadTime->setChecked(pParams.inList("byLeadTime"));

  param = pParams.value("byDays", &valid);
  if (valid)
  {
   _byDays->setChecked(true);
   _days->setValue(param.toInt());
  }

  param = pParams.value("byDate", &valid);
  if (valid)
  {
   _byDate->setChecked(true);
   _date->setDate(param.toDate());
  }

  if (pParams.inList("run"))
  {
    sFillList();
    return NoError_Run;
  }

  return NoError;
}

void dspSubstituteAvailabilityByItem::sViewAllocations()
{
  ParameterList params;
  params.append("itemsite_id", list()->id());
  params.append("run");

  if (_leadTime->isChecked())
    params.append("byLeadTime", true);
  else if (_byDays->isChecked())
    params.append("byDays", _days->value() );
  else if (_byDate->isChecked())
    params.append("byDate", _date->date());

  dspAllocations *newdlg = new dspAllocations();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspSubstituteAvailabilityByItem::sViewOrders()
{
  ParameterList params;
  params.append("itemsite_id", list()->id());
  params.append("run");

  if (_leadTime->isChecked())
    params.append("byLeadTime", true);
  else if (_byDays->isChecked())
    params.append("byDays", _days->value() );
  else if (_byDate->isChecked())
    params.append("byDate", _date->date());

  dspOrders *newdlg = new dspOrders();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspSubstituteAvailabilityByItem::sPopulateMenu(QMenu *menu, QTreeWidgetItem*, int)
{
  menu->addAction(tr("View Allocations..."), this, SLOT(sViewAllocations()));
  menu->addAction(tr("View Orders..."), this, SLOT(sViewOrders()));
}

bool dspSubstituteAvailabilityByItem::setParams(ParameterList &params)
{
  if(!_item->isValid())
  {
    QMessageBox::warning(this, tr("Item Required"),
      tr("You must specify an Item Number"));
    return false;
  }

  if (_byDate->isChecked() && !_date->isValid())
  {
    QMessageBox::warning(this, tr("Date Required"),
      tr("You must specify a valid date."));
    return false;
  }

  if (_normalize->isChecked())
    params.append("normalize");

  if (_leadTime->isChecked())
  {
    params.append("leadTime");
    params.append("byLeadTime"); // report
  }
  else if (_byDays->isChecked())
  {
    params.append("byDays", _days->value());
    params.append("days", _days->value()); // MetaSQL
  }
  else if (_byDate->isChecked())
  {
    params.append("byDate", _date->date());
    params.append("date", _date->date()); // MetaSQL
  }

  params.append("item_id",  _item->id());

  _warehouse->appendValue(params);

  return true;
}

