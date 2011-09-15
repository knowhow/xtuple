/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspUnbalancedQOHByClassCode.h"

#include <QAction>
#include <QMenu>
#include <QSqlError>

#include "createCountTagsByItem.h"
#include "dspInventoryAvailability.h"
#include "itemSite.h"
#include "storedProcErrorLookup.h"

dspUnbalancedQOHByClassCode::dspUnbalancedQOHByClassCode(QWidget* parent, const char*, Qt::WFlags fl)
  : display(parent, "dspUnbalancedQOHByClassCode", fl)
{
  setupUi(optionsWidget());
  setWindowTitle(tr("Unbalanced QOH by Class Code"));
  setListLabel(tr("&Item Sites"));
  setMetaSQLOptions("unbalancedQOHByClassCode", "detail");
  setUseAltId(true);

  _classCode->setType(ParameterGroup::ClassCode);

  list()->addColumn(tr("Site"),        _whsColumn,   Qt::AlignCenter, true,  "warehous_code" );
  list()->addColumn(tr("Item Number"), _itemColumn,  Qt::AlignLeft,   true,  "item_number"   );
  list()->addColumn(tr("Description"), -1,           Qt::AlignLeft,   true,  "itemdescrip"   );
  list()->addColumn(tr("UOM"),         _uomColumn,   Qt::AlignCenter, true,  "uom_name" );
  list()->addColumn(tr("QOH"),         _qtyColumn,   Qt::AlignRight,  true,  "itemsite_qtyonhand"  );
  list()->addColumn(tr("QOH Detail."), _qtyColumn,   Qt::AlignRight,  true,  "detailedqoh"  );
  list()->addColumn(tr("NN QOH"),      _qtyColumn,   Qt::AlignRight,  true,  "itemsite_nnqoh"  );
  list()->addColumn(tr("NN Detail."),  _qtyColumn,   Qt::AlignRight,  true,  "detailednnqoh"  );
}

void dspUnbalancedQOHByClassCode::languageChange()
{
  display::languageChange();
  retranslateUi(this);
}

void dspUnbalancedQOHByClassCode::sBalance()
{
  q.prepare("SELECT balanceItemsite(:itemsite_id) AS result;");
  q.bindValue(":itemsite_id", list()->id());
  q.exec();
  if (q.first())
  {
    int result = q.value("result").toInt();
    if (result < 0)
    {
      systemError(this, storedProcErrorLookup("balanceItemsite", result),
                  __FILE__, __LINE__);
      return;
    }
  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
  sFillList();
}

void dspUnbalancedQOHByClassCode::sView()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("itemsite_id", list()->id());

  itemSite newdlg(this, "", true);
  newdlg.set(params);
  newdlg.exec();
}

void dspUnbalancedQOHByClassCode::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("itemsite_id", list()->id());

  itemSite newdlg(this, "", true);
  newdlg.set(params);
  newdlg.exec();
}

void dspUnbalancedQOHByClassCode::sInventoryAvailability()
{
  ParameterList params;
  params.append("itemsite_id", list()->id());
  params.append("byLeadTime");
  params.append("run");

  dspInventoryAvailability *newdlg = new dspInventoryAvailability();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspUnbalancedQOHByClassCode::sIssueCountTag()
{
  ParameterList params;
  params.append("itemsite_id", list()->id());
  
  createCountTagsByItem newdlg(this, "", true);
  newdlg.set(params);
  newdlg.exec();
}

void dspUnbalancedQOHByClassCode::sPopulateMenu(QMenu *pMenu, QTreeWidgetItem*, int)
{
  QAction *menuItem;

  menuItem = pMenu->addAction(tr("Balance Item Site..."), this, SLOT(sBalance()));

  pMenu->addSeparator();

  menuItem = pMenu->addAction(tr("View Item Site..."), this, SLOT(sView()));
  menuItem->setEnabled(_privileges->check("MaintainItemSites") ||
                       _privileges->check("ViewItemSites"));

  menuItem = pMenu->addAction(tr("Edit Item Site..."), this, SLOT(sEdit()));
  menuItem->setEnabled(_privileges->check("MaintainItemSites"));

  pMenu->addSeparator();

  menuItem = pMenu->addAction(tr("View Inventory Availability..."), this, SLOT(sInventoryAvailability()));
  menuItem->setEnabled(_privileges->check("ViewInventoryAvailability"));

  pMenu->addSeparator();

  menuItem = pMenu->addAction(tr("Issue Count Tag..."), this, SLOT(sIssueCountTag()));
  menuItem->setEnabled(_privileges->check("IssueCountTags"));
}

bool dspUnbalancedQOHByClassCode::setParams(ParameterList &params)
{
  if ((_classCode->isSelected()) || (_classCode->isPattern()))
    _classCode->appendValue(params);

  if (_warehouse->isSelected())
    params.append("warehous_id", _warehouse->id());

  params.append("regular", tr("Regular"));
  params.append("none", tr("None"));
  params.append("lot", tr("Lot #"));
  params.append("serial", tr("Serial #"));

  return true;
}
