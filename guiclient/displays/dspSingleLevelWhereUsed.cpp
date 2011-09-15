/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspSingleLevelWhereUsed.h"

#include <QAction>
#include <QMenu>
#include <QMessageBox>
#include <QVariant>

#include "bom.h"
#include "dspInventoryHistory.h"
#include "item.h"

dspSingleLevelWhereUsed::dspSingleLevelWhereUsed(QWidget* parent, const char*, Qt::WFlags fl)
    : display(parent, "dspSingleLevelWhereUsed", fl)
{
  setupUi(optionsWidget());
  setWindowTitle(tr("Single Level Where Used"));
  setListLabel(tr("Bill of Materials Items"));
  setReportName("SingleLevelWhereUsed");
  setMetaSQLOptions("whereUsed", "detail");

  if (_metrics->boolean("AllowInactiveBomItems"))
    _item->setType(ItemLineEdit::cGeneralComponents);
  else
    _item->setType(ItemLineEdit::cGeneralComponents | ItemLineEdit::cActive);

  _effective->setNullString(tr("Today"));
  _effective->setNullDate(QDate::currentDate());
  _effective->setAllowNullDate(true);
  _effective->setNull();

  list()->addColumn(tr("Seq #"),       40,           Qt::AlignCenter, true, "bomitem_seqnumber");
  list()->addColumn(tr("Parent Item"), _itemColumn,  Qt::AlignLeft,  true, "item_number");
  list()->addColumn(tr("Description"), -1,           Qt::AlignLeft,  true, "descrip");
  list()->addColumn(tr("UOM"),         _uomColumn,   Qt::AlignLeft,  true, "uom_name");
  list()->addColumn(tr("Fxd. Qty."),   _qtyColumn,   Qt::AlignRight, true, "qtyfxd");
  list()->addColumn(tr("Qty. Per"),    _qtyColumn,   Qt::AlignRight, true, "qtyper");
  list()->addColumn(tr("Scrap %"),     _prcntColumn, Qt::AlignRight, true, "bomitem_scrap");
  list()->addColumn(tr("Effective"),   _dateColumn,  Qt::AlignCenter,true, "bomitem_effective");
  list()->addColumn(tr("Expires"),     _dateColumn,  Qt::AlignCenter,true, "bomitem_expires");
  
  connect(omfgThis, SIGNAL(bomsUpdated(int, bool)), SLOT(sFillList()));

  _item->setFocus();
}

void dspSingleLevelWhereUsed::languageChange()
{
  display::languageChange();
  retranslateUi(this);
}

enum SetResponse dspSingleLevelWhereUsed::set(const ParameterList &pParams)
{
  XWidget::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("item_id", &valid);
  if (valid)
    _item->setId(param.toInt());

  param = pParams.value("effective", &valid);
  if (valid)
    _effective->setDate(param.toDate());

  if (pParams.inList("run"))
  {
    sFillList();
    return NoError_Run;
  }

  return NoError;
}


void dspSingleLevelWhereUsed::sPopulateMenu(QMenu *menu, QTreeWidgetItem *, int)
{
  QAction *menuItem;

  menuItem = menu->addAction(tr("Edit Bill of Materials..."), this, SLOT(sEditBOM()));
  if (!_privileges->check("MaintainBOMs"))
    menuItem->setEnabled(false);

  menuItem = menu->addAction(tr("Edit Item Master..."), this, SLOT(sEditItem()));
  if (!_privileges->check("MaintainItemMasters"))
    menuItem->setEnabled(false);

  menuItem = menu->addAction(tr("View Item Inventory History..."), this, SLOT(sViewInventoryHistory()));
  if (!_privileges->check("ViewInventoryHistory"))
    menuItem->setEnabled(false);
}

void dspSingleLevelWhereUsed::sEditBOM()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("item_id", list()->id());

  BOM *newdlg = new BOM();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspSingleLevelWhereUsed::sEditItem()
{
  item::editItem(list()->id());
}

void dspSingleLevelWhereUsed::sViewInventoryHistory()
{
  ParameterList params;

  XSqlQuery qq;
  qq.prepare( "SELECT item_id "
             "FROM item JOIN bomitem ON (bomitem_parent_item_id=item_id) "
             "WHERE (bomitem_parent_item_id = :bomitem_parent_item_id) " );
  qq.bindValue(":bomitem_parent_item_id", list()->id());
  qq.exec();
  if (qq.first())
  {
    int _itemId = qq.value("item_id").toInt();
    params.append("item_id", _itemId);
  }

  params.append("warehous_id", -1);
  params.append("run");

  dspInventoryHistory *newdlg = new dspInventoryHistory();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

bool dspSingleLevelWhereUsed::setParams(ParameterList &params)
{
  if (!_item->isValid())
  {
    QMessageBox::warning( this, tr("Enter a Valid Item Number"),
                          tr("You must enter a valid Item Number.") );
    _item->setFocus();
    return false;
  }

  params.append("item_id", _item->id());

  if (_effective->isNull())
    params.append("notEffective");
  else
    params.append("effective", _effective->date());

  params.append("Always", tr("Always"));
  params.append("Never", tr("Never"));
  params.append("Push", tr("Push"));
  params.append("Pull", tr("Pull"));
  params.append("Mixed", tr("Mixed"));
  params.append("Special", tr("Special"));

  return true;
}
