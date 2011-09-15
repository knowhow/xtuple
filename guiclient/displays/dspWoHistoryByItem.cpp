/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspWoHistoryByItem.h"

#include <QAction>
#include <QMenu>
#include <QMessageBox>
#include <QVariant>

#include "guiclient.h"
#include "workOrder.h"

dspWoHistoryByItem::dspWoHistoryByItem(QWidget* parent, const char*, Qt::WFlags fl)
  : display(parent, "dspWoHistoryByItem", fl)
{
  setupUi(optionsWidget());
  setWindowTitle(tr("Work Order History by Item"));
  setListLabel(tr("Work Orders"));
  setReportName("WOHistoryByItem");
  setMetaSQLOptions("workOrderHistory", "detail");

  connect(_showCost, SIGNAL(toggled(bool)), this, SLOT(sHandleCosts(bool)));

  _item->setType(ItemLineEdit::cGeneralManufactured | ItemLineEdit::cGeneralPurchased);
  _item->setDefaultType(ItemLineEdit::cGeneralManufactured);
  _dates->setStartNull(tr("Earliest"), omfgThis->startOfTime(), true);
  _dates->setEndNull(tr("Latest"), omfgThis->endOfTime(), true);

  list()->addColumn(tr("W/O #"),      -1,            Qt::AlignLeft,   true,  "wonumber"   );
  list()->addColumn(tr("Status"),     _statusColumn, Qt::AlignCenter, true,  "wo_status" );
  list()->addColumn(tr("Site"),       _whsColumn,    Qt::AlignCenter, true,  "warehous_code" );
  list()->addColumn(tr("Ordered"),    _qtyColumn,    Qt::AlignRight,  true,  "wo_qtyord"  );
  list()->addColumn(tr("Received"),   _qtyColumn,    Qt::AlignRight,  true,  "wo_qtyrcv"  );
  list()->addColumn(tr("Start Date"), _dateColumn,   Qt::AlignCenter, true,  "wo_startdate" );
  list()->addColumn(tr("Due Date"),   _dateColumn,   Qt::AlignCenter, true,  "wo_duedate" );
  list()->addColumn(tr("Cost"),       _costColumn,   Qt::AlignRight,  true,  "wo_postedvalue" );

  sHandleCosts(_showCost->isChecked());
  
  connect(omfgThis, SIGNAL(workOrdersUpdated(int, bool)), this, SLOT(sFillList()));

  _item->setFocus();
}

void dspWoHistoryByItem::languageChange()
{
  display::languageChange();
  retranslateUi(this);
}

void dspWoHistoryByItem::sView()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("wo_id", list()->id());

  workOrder *newdlg = new workOrder();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspWoHistoryByItem::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("wo_id", list()->id());

  workOrder *newdlg = new workOrder();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspWoHistoryByItem::sPopulateMenu(QMenu *pMenu, QTreeWidgetItem *, int)
{
  QAction *menuItem;

  menuItem = pMenu->addAction(tr("Edit..."), this, SLOT(sEdit()));
  menuItem = pMenu->addAction(tr("View..."), this, SLOT(sView()));
}

void dspWoHistoryByItem::sHandleCosts(bool pShowCosts)
{
  if (pShowCosts)
    list()->showColumn(7);
  else
    list()->hideColumn(7);

  sFillList();
}

bool dspWoHistoryByItem::setParams(ParameterList & params)
{
  if(isVisible())
  {
    if (! _item->isValid())
    {
      QMessageBox::warning(this, tr("Invalid Item"),
                           tr("Enter a valid Item."));
      _item->setFocus();
      return false;
    }

    if (! _dates->startDate().isValid())
    {
      QMessageBox::warning(this, tr("Invalid Start Date"),
                           tr("Enter a valid Start Date."));
      _dates->setFocus();
      return false;
    }

    if (! _dates->endDate().isValid())
    {
      QMessageBox::warning(this, tr("Invalid End Date"),
                           tr("Enter a valid End Date."));
      _dates->setFocus();
      return false;
    }
  }

  params.append("woHistoryByItem"); // metasql only?
  params.append("item_id", _item->id());
  _dates->appendValue(params);
  _warehouse->appendValue(params);

  if (_showOnlyTopLevel->isChecked())
    params.append("showOnlyTopLevel");

  if(_showCost->isChecked())
    params.append("showCosts"); // report only?

  return true;
}

