/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspItemCostSummary.h"

#include <QAction>
#include <QMenu>
#include <QMessageBox>
#include <QVariant>

#include <openreports.h>

#include <metasql.h>
#include "mqlutil.h"

#include "dspItemCostDetail.h"
#include "itemCost.h"

dspItemCostSummary::dspItemCostSummary(QWidget* parent, const char*, Qt::WFlags fl)
    : display(parent, "dspItemCostSummary", fl)
{
  setupUi(optionsWidget());
  setWindowTitle(tr("Item Costs Summary"));
  setListLabel(tr("Costing &Elements"));
  setReportName("ItemCostSummary");
  setMetaSQLOptions("itemCost", "detail");
  setUseAltId(true);

  list()->addColumn(tr("Element"),   -1,           Qt::AlignLeft,   true,  "costelem_type"   );
  list()->addColumn(tr("Lower"),     _costColumn,  Qt::AlignCenter, true,  "lowlevel" );
  list()->addColumn(tr("Std. Cost"), _costColumn,  Qt::AlignRight,  true,  "itemcost_stdcost"  );
  list()->addColumn(tr("Posted"),    _dateColumn,  Qt::AlignCenter, true,  "itemcost_posted" );
  list()->addColumn(tr("Act. Cost"), _costColumn,  Qt::AlignRight,  true,  "itemcost_actcost"  );
  list()->addColumn(tr("Updated"),   _dateColumn,  Qt::AlignCenter, true,  "itemcost_updated" );
}

void dspItemCostSummary::languageChange()
{
  display::languageChange();
  retranslateUi(this);
}

enum SetResponse dspItemCostSummary::set(const ParameterList &pParams)
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

  if (pParams.inList("run"))
  {
    sFillList();
    return NoError_Run;
  }

  return NoError;
}

void dspItemCostSummary::sPopulateMenu(QMenu *pMenu, QTreeWidgetItem *pSelected, int)
{
  if (pSelected->text(1) == "Yes")
  {
    pMenu->addAction(tr("View Costing Detail..."), this, SLOT(sViewDetail()));
    pMenu->addSeparator();
  }
}

void dspItemCostSummary::sViewDetail()
{
  ParameterList params;
  params.append("itemcost_id", list()->id());
  params.append("run");

  dspItemCostDetail *newdlg = new dspItemCostDetail();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

bool dspItemCostSummary::setParams(ParameterList &params)
{
  if(!_item->isValid())
  {
    QMessageBox::warning(this, tr("Item Required"),
      tr("You must specify an Item Number."));
    return false;
  }

  params.append("bySummrey");

  params.append("item_id", _item->id());
  params.append("never", tr("Never")); 
  return true;
}
