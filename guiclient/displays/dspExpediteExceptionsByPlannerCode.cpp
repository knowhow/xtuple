/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspExpediteExceptionsByPlannerCode.h"

#include <QVariant>

#include "xtreewidget.h"

dspExpediteExceptionsByPlannerCode::dspExpediteExceptionsByPlannerCode(QWidget* parent, const char*, Qt::WFlags fl)
  : display(parent, "dspExpediteExceptionsByPlannerCode", fl)
{
  setupUi(optionsWidget());
  setWindowTitle(tr("Expedite Exceptions by Planner Code"));
  setListLabel(tr("Expedite Exceptions"));
  setReportName("ExpediteExceptionsByPlannerCode");
  setMetaSQLOptions("schedule", "expedite");
  setUseAltId(true);

  _plannerCode->setType(ParameterGroup::PlannerCode);

  list()->addColumn(tr("Order Type/#"),   _itemColumn, Qt::AlignCenter,true, "order_number");
  list()->addColumn(tr("To Site"),        _whsColumn,  Qt::AlignCenter,true, "to_warehous");
  list()->addColumn(tr("From Site"),      _whsColumn,  Qt::AlignCenter,true, "from_warehous");
  list()->addColumn(tr("Item Number"),    _itemColumn, Qt::AlignLeft,  true, "item_number");
  list()->addColumn(tr("Descriptions"),   -1,          Qt::AlignLeft,  true, "item_descrip");
  list()->addColumn(tr("Start/Due Date"), _itemColumn, Qt::AlignLeft,  true, "keydate");
  list()->addColumn(tr("Exception"),      120,         Qt::AlignLeft,  true, "exception");
}

void dspExpediteExceptionsByPlannerCode::languageChange()
{
  display::languageChange();
  retranslateUi(this);
}

bool dspExpediteExceptionsByPlannerCode::setParams(ParameterList & params)
{
  _plannerCode->appendValue(params);
  _warehouse->appendValue(params);

  params.append("releaseOrder", tr("Release Order"));
  params.append("startProduction", tr("Start Production"));
  params.append("expediteProduction", tr("Expedite Production"));
  params.append("days", _days->value());

  return true;
}

