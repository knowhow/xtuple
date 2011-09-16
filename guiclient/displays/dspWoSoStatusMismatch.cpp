/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspWoSoStatusMismatch.h"

#include <QAction>
#include <QMenu>
#include <QVariant>

#include "closeWo.h"
#include "dspWoMaterialsByWorkOrder.h"

dspWoSoStatusMismatch::dspWoSoStatusMismatch(QWidget* parent, const char*, Qt::WFlags fl)
  : display(parent, "dspWoSoStatusMismatch", fl)
{
  setupUi(optionsWidget());
  setWindowTitle(tr("Open Work Orders with Closed Parent Sales Orders"));
  setListLabel(tr("Work Orders"));
  setReportName("OpenWorkOrdersWithClosedParentSalesOrders");
  setMetaSQLOptions("workOrderSoStatus", "detail");

  list()->addColumn(tr("W/O #"),       _orderColumn,  Qt::AlignLeft   , true, "wonumber" );
  list()->addColumn(tr("Status"),      _statusColumn, Qt::AlignCenter , true, "wo_status");
  list()->addColumn(tr("Item Number"), -1,            Qt::AlignLeft   , true, "item_number");
  list()->addColumn(tr("UOM"),         _uomColumn,    Qt::AlignLeft   , true, "uom_name");
  list()->addColumn(tr("Site"),        _whsColumn,    Qt::AlignCenter , true, "warehous_code");
  list()->addColumn(tr("S/O #"),       _orderColumn,  Qt::AlignLeft   , true, "cohead_number");
  list()->addColumn(tr("Ordered"),     _qtyColumn,    Qt::AlignRight  , true, "wo_qtyord");
  list()->addColumn(tr("Received"),    _qtyColumn,    Qt::AlignRight  , true, "wo_qtyrcv");
  list()->addColumn(tr("Start Date"),  _dateColumn,   Qt::AlignCenter , true, "wo_startdate");
  list()->addColumn(tr("Due Date"),    _dateColumn,   Qt::AlignCenter , true, "wo_duedate");

  sFillList();
}

void dspWoSoStatusMismatch::languageChange()
{
  display::languageChange();
  retranslateUi(this);
}

void dspWoSoStatusMismatch::sCloseWo()
{
  ParameterList params;
  params.append("wo_id", list()->id());

  closeWo newdlg(this, "", true);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void dspWoSoStatusMismatch::sPopulateMenu(QMenu *pMenu, QTreeWidgetItem *pSelected, int)
{
  if ((pSelected->text(1) == "E") || (pSelected->text(1) == "I"))
    pMenu->addAction(tr("View W/O Material Requirements..."), this, SLOT(sViewWomatlreq()));

  pMenu->addAction(tr("Close W/O..."), this, SLOT(sCloseWo()));
}

void dspWoSoStatusMismatch::sViewWomatlreq()
{
  ParameterList params;
  params.append("wo_id", list()->id());
  params.append("run");

  dspWoMaterialsByWorkOrder *newdlg = new dspWoMaterialsByWorkOrder();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

bool dspWoSoStatusMismatch::setParams(ParameterList & params)
{
  params.append("woSoStatusMismatch");
  _warehouse->appendValue(params);

  return true;
}

