/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspWoHistoryByNumber.h"

#include <QAction>
#include <QMenu>
#include <QMessageBox>
#include <QVariant>

#include "workOrder.h"

dspWoHistoryByNumber::dspWoHistoryByNumber(QWidget* parent, const char*, Qt::WFlags fl)
  : display(parent, "dspWoHistoryByNumber", fl)
{
  setupUi(optionsWidget());
  setWindowTitle(tr("Work Order History by W/O Number"));
  setListLabel(tr("Work Orders"));
  setReportName("WOHistoryByNumber");
  setMetaSQLOptions("workOrderHistory", "detail");

  connect(_showCost, SIGNAL(toggled(bool)), this, SLOT(sHandleCosts(bool)));

  list()->addColumn(tr("W/O #"),       _orderColumn,  Qt::AlignLeft,   true,  "wo_number"   );
  list()->addColumn(tr("Sub. #"),      _uomColumn,    Qt::AlignLeft,   true,  "wo_subnumber"   );
  list()->addColumn(tr("Item #"),      _itemColumn,   Qt::AlignLeft,   true,  "item_number"   );
  list()->addColumn(tr("Description"), -1,            Qt::AlignLeft,   true,  "itemdescrip"   );
  list()->addColumn(tr("Status"),      _statusColumn, Qt::AlignCenter, true,  "wo_status" );
  list()->addColumn(tr("Site"),        _whsColumn,    Qt::AlignCenter, true,  "warehous_code" );
  list()->addColumn(tr("Ordered"),     _qtyColumn,    Qt::AlignRight,  true,  "wo_qtyord"  );
  list()->addColumn(tr("Received"),    _qtyColumn,    Qt::AlignRight,  true,  "wo_qtyrcv"  );
  list()->addColumn(tr("Start Date"),  _dateColumn,   Qt::AlignCenter, true,  "wo_startdate"  );
  list()->addColumn(tr("Due Date"),    _dateColumn,   Qt::AlignCenter, true,  "wo_duedate"  );
  list()->addColumn(tr("Cost"),        _costColumn,   Qt::AlignRight,  true,  "wo_postedvalue" );
  list()->addColumn(tr("WIP"),         _costColumn,   Qt::AlignRight,  false, "wo_wipvalue" );
  list()->addColumn(tr("Project"),     _orderColumn,  Qt::AlignLeft,   false, "project" );
  list()->addColumn(tr("Priority"),    _statusColumn, Qt::AlignCenter, false, "wo_priority" );
  list()->addColumn(tr("BOM Rev"),     _orderColumn,  Qt::AlignLeft,   false, "bom_rev_number" );
  list()->addColumn(tr("BOO Rev"),     _orderColumn,  Qt::AlignLeft,   false, "boo_rev_number" );

  sHandleCosts(_showCost->isChecked());

  connect(omfgThis, SIGNAL(workOrdersUpdated(int, bool)), SLOT(sFillList()));
}

void dspWoHistoryByNumber::languageChange()
{
  display::languageChange();
  retranslateUi(this);
}

enum SetResponse dspWoHistoryByNumber::set(const ParameterList &pParams)
{
  XWidget::set(pParams);
  QVariant param;
  bool     valid;
  
  param = pParams.value("wo_number", &valid);
  if (valid)
  {
    _woNumber->setText(param.toString());
    sFillList();
  }

  return NoError;
}

void dspWoHistoryByNumber::sView()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("wo_id", list()->id());

  workOrder *newdlg = new workOrder();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspWoHistoryByNumber::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("wo_id", list()->id());

  workOrder *newdlg = new workOrder();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspWoHistoryByNumber::sPopulateMenu(QMenu *pMenu, QTreeWidgetItem *, int)
{
  QAction *menuItem;

  menuItem = pMenu->addAction(tr("Edit..."), this, SLOT(sEdit()));
  menuItem = pMenu->addAction(tr("View..."), this, SLOT(sView()));
}

void dspWoHistoryByNumber::sHandleCosts(bool pShowCosts)
{
  if (pShowCosts)
    list()->showColumn(list()->column("wo_postedvalue"));
  else
    list()->hideColumn(list()->column("wo_postedvalue"));
}

bool dspWoHistoryByNumber::setParams(ParameterList &params)
{
  if(_woNumber->text().isEmpty())
  {
    QMessageBox::warning( this, tr("Invalid Work Order Number"),                                                   
                      tr( "You must enter a work order number for this report." ) );                               
    _woNumber->setFocus();
    return false;
  }

  params.append("wo_number", _woNumber->text()); // metasql only?
  params.append("woNumber", _woNumber->text()); // report only?

  if (_metrics->boolean("RevControl"))
    params.append("revControl"); // metasql only?

  if (_showOnlyTopLevel->isChecked())
    params.append("showOnlyTopLevel");

  if(_showCost->isChecked())
    params.append("showCosts"); // report only?

  return true;
}

