/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspFrozenItemSites.h"

#include <QAction>
#include <QMenu>

#include "xtreewidget.h"

dspFrozenItemSites::dspFrozenItemSites(QWidget* parent, const char*, Qt::WFlags fl)
  : display(parent, "dspFrozenItemSites", fl)
{
  setupUi(optionsWidget());
  setWindowTitle(tr("Frozen Item Sites"));
  setListLabel(tr("Count Tags"));
  setReportName("FrozenItemSites");
  setMetaSQLOptions("frozenItemSites", "detail");

  list()->addColumn(tr("Site"),        _whsColumn,  Qt::AlignCenter,true, "warehous_code");
  list()->addColumn(tr("Item Number"), _itemColumn, Qt::AlignLeft,  true, "item_number");
  list()->addColumn(tr("Description"), -1,          Qt::AlignLeft,  true, "descrip");
  list()->addColumn(tr("Count Tag #"), _qtyColumn,  Qt::AlignRight, true, "cnttag");

  sFillList();
}

void dspFrozenItemSites::languageChange()
{
  display::languageChange();
  retranslateUi(this);
}

void dspFrozenItemSites::sPopulateMenu(QMenu *pMenu, QTreeWidgetItem *, int)
{
  QAction *menuItem;

  menuItem = pMenu->addAction(tr("Thaw"), this, SLOT(sThaw()));
  menuItem->setEnabled(_privileges->check("ThawInventory"));
}

void dspFrozenItemSites::sThaw()
{
  XSqlQuery qq;
  qq.prepare("SELECT thawItemsite(:itemsite_id) AS result;");
  qq.bindValue(":itemsite_id", list()->id());
  qq.exec();

  sFillList();
}

bool dspFrozenItemSites::setParams(ParameterList &params)
{
  _warehouse->appendValue(params);

  return true;
}

