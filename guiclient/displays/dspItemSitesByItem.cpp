/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspItemSitesByItem.h"

#include <QAction>
#include <QMenu>
#include <QVariant>
#include <QMessageBox>

#include "inputManager.h"
#include "itemSite.h"
#include "dspInventoryAvailabilityByItem.h"
#include "createCountTagsByItem.h"
#include "dspInventoryLocator.h"
#include "guiclient.h"

dspItemSitesByItem::dspItemSitesByItem(QWidget* parent, const char*, Qt::WFlags fl)
    : display(parent, "dspItemSitesByItem", fl)
{
  setupUi(optionsWidget());
  setWindowTitle(tr("Item Sites by Item"));
  setListLabel(tr("Item Sites"));
  setReportName("ItemSitesByItem");
  setMetaSQLOptions("itemSites", "detail");
  setUseAltId(true);

  omfgThis->inputManager()->notify(cBCItem, this, _item, SLOT(setId(int)));

  list()->addColumn(tr("Site"),          _whsColumn, Qt::AlignCenter, true, "warehous_code");
  list()->addColumn(tr("QOH"),                   -1, Qt::AlignRight, true, "itemsite_qtyonhand");
  list()->addColumn(tr("Loc. Cntrl."), _orderColumn, Qt::AlignCenter,true, "itemsite_loccntrl");
  list()->addColumn(tr("Cntrl. Meth."), _itemColumn, Qt::AlignCenter,true, "controlmethod");
  list()->addColumn(tr("Sold Ranking"), _itemColumn, Qt::AlignCenter,true, "soldranking");
  list()->addColumn(tr("Last Cnt'd"),   _dateColumn, Qt::AlignCenter,true, "itemsite_datelastcount");
  list()->addColumn(tr("Last Used"),    _dateColumn, Qt::AlignCenter,true, "itemsite_datelastused");
  list()->setDragString("itemsiteid=");
  
  connect(omfgThis, SIGNAL(itemsitesUpdated()), SLOT(sFillList()));

  _item->setFocus();
}

void dspItemSitesByItem::languageChange()
{
  display::languageChange();
  retranslateUi(this);
}

void dspItemSitesByItem::sPopulateMenu(QMenu *menu, QTreeWidgetItem *pSelected, int)
{
  QAction *menuItem;

  menuItem = menu->addAction(tr("View Item Site..."), this, SLOT(sViewItemsite()));
  menuItem->setEnabled(_privileges->check("MaintainItemSites") || _privileges->check("ViewItemSites"));

  menuItem = menu->addAction(tr("Edit Item Site..."), this, SLOT(sEditItemsite()));
  menuItem->setEnabled(_privileges->check("MaintainItemSites"));

  menu->addSeparator();

  menuItem = menu->addAction(tr("View Inventory Availability..."), this, SLOT(sViewInventoryAvailability()));
  menuItem->setEnabled(_privileges->check("ViewInventoryAvailability"));

  if (((XTreeWidgetItem *)pSelected)->altId() == 1)
  {
    menuItem = menu->addAction(tr("View Location/Lot/Serial # Detail..."), this, SLOT(sViewLocationLotSerialDetail()));
    menuItem->setEnabled(_privileges->check("ViewQOH"));
  }

  menu->addSeparator();

  menuItem = menu->addAction(tr("Issue Count Tag..."), this, SLOT(sIssueCountTag()));
  if (!_privileges->check("IssueCountTags"))
    menuItem->setEnabled(false);
}

void dspItemSitesByItem::sViewItemsite()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("itemsite_id", list()->id());

  itemSite newdlg(this, "", true);
  newdlg.set(params);
  newdlg.exec();
}

void dspItemSitesByItem::sEditItemsite()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("itemsite_id", list()->id());

  itemSite newdlg(this, "", true);
  newdlg.set(params);
  newdlg.exec();
}

void dspItemSitesByItem::sViewInventoryAvailability()
{
  ParameterList params;
  params.append("itemsite_id", list()->id());
  params.append("byLeadTime");
  params.append("run");

  dspInventoryAvailabilityByItem *newdlg = new dspInventoryAvailabilityByItem();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspItemSitesByItem::sViewLocationLotSerialDetail()
{
  ParameterList params;
  params.append("itemsite_id", list()->id());
  params.append("run");

  dspInventoryLocator *newdlg = new dspInventoryLocator();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspItemSitesByItem::sIssueCountTag()
{
  ParameterList params;
  params.append("itemsite_id", list()->id());
  
  createCountTagsByItem newdlg(this, "", true);
  newdlg.set(params);
  newdlg.exec();
}

bool dspItemSitesByItem::setParams(ParameterList &params)
{
  params.append("byItem");  

  params.append("regular", tr("Regular"));
  params.append("none", tr("None"));
  params.append("lot", tr("Lot #"));
  params.append("serial", tr("Serial #"));
  params.append("na", tr("N/A"));
  params.append("never", tr("Never"));
  
  if (_showInactive->isChecked())
    params.append("showInactive");  
  
  if (_item->isValid())
    params.append("item_id", _item->id());
  else
  {
    _item->setFocus();
    QMessageBox::warning( this, tr("Item Required"),
      tr("You must specify a valid Item Number."));
    return false;
  }

//  Ack - HackHackHack
  params.append("placeHolder", -1); // for report definition

  return true;
}
