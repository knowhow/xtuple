/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspInventoryLocator.h"

#include <QAction>
#include <QMenu>
#include <QMessageBox>
#include <QVariant>

#include "relocateInventory.h"
#include "reassignLotSerial.h"

dspInventoryLocator::dspInventoryLocator(QWidget* parent, const char*, Qt::WFlags fl)
    : display(parent, "dspInventoryLocator", fl)
{
  setupUi(optionsWidget());
  setWindowTitle(tr("Location/Lot/Serial # Detail"));
  setListLabel(tr("Locations"));
  setReportName("LocationLotSerialNumberDetail");
  setMetaSQLOptions("inventoryLocator", "detail");
  setUseAltId(true);

  list()->addColumn(tr("Site"),       _whsColumn, Qt::AlignCenter,true, "warehous_code");
  list()->addColumn(tr("Location"),          200, Qt::AlignLeft,  true, "locationname");
  list()->addColumn(tr("Netable"),  _orderColumn, Qt::AlignCenter,true, "netable");
  list()->addColumn(tr("Lot/Serial #"),       -1, Qt::AlignLeft,  true, "lotserial");
  list()->addColumn(tr("Expiration"),_dateColumn, Qt::AlignCenter,true, "expiration");
  list()->addColumn(tr("Warranty"),  _dateColumn, Qt::AlignCenter,true, "warranty");
  list()->addColumn(tr("Qty."),       _qtyColumn, Qt::AlignRight, true, "qoh");

  _item->setFocus();
}

void dspInventoryLocator::languageChange()
{
  display::languageChange();
  retranslateUi(this);
}

enum SetResponse dspInventoryLocator::set(const ParameterList &pParams)
{
  XWidget::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("item_id", &valid);
  if (valid)
    _item->setId(param.toInt());

  param = pParams.value("warehous_id", &valid);
  if (valid)
    _warehouse->setId(param.toInt());

  param = pParams.value("itemsite_id", &valid);
  if (valid)
    _item->setItemsiteid(param.toInt());

  return NoError;
}

bool dspInventoryLocator::setParams(ParameterList &params)
{
  if(!_item->isValid())
  {
    QMessageBox::warning( this, tr("Enter a Valid Item Number"),
                      tr("You must enter a valid Item Number for this report.") );
    return false;
  }

  _warehouse->appendValue(params);

  params.append("item_id", _item->id());
  params.append("yes",     tr("Yes"));
  params.append("no",      tr("No"));
  params.append("na",      tr("N/A"));

  //if (_showZeroLevel->isChecked())
  //  params.append("showZeroLevel");

  return true;
}

void dspInventoryLocator::sRelocateInventory()
{
  ParameterList params;
  params.append("itemloc_id", list()->id());

  relocateInventory newdlg(this, "", true);
  newdlg.set(params);

  if (newdlg.exec())
    sFillList();
}

void dspInventoryLocator::sReassignLotSerial()
{
  ParameterList params;
  params.append("itemloc_id", list()->id());

  reassignLotSerial newdlg(this, "", true);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void dspInventoryLocator::sPopulateMenu(QMenu *pMenu, QTreeWidgetItem *pSelected, int)
{
  QAction *menuItem;

  if (((XTreeWidgetItem *)pSelected)->altId() == -1)
  {
    menuItem = pMenu->addAction(tr("Relocate..."), this, SLOT(sRelocateInventory()));;
    if (!_privileges->check("RelocateInventory"))
      menuItem->setEnabled(false);

    menuItem = pMenu->addAction(tr("Reassign Lot/Serial #..."), this, SLOT(sReassignLotSerial()));;
    if (!_privileges->check("ReassignLotSerial"))
      menuItem->setEnabled(false);
  }
}

