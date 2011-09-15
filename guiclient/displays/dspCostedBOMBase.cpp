/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspCostedBOMBase.h"

#include <QAction>
#include <QAction>
#include <QAction>
#include <QAction>
#include <QMenu>
#include <QMessageBox>
#include <QVariant>
#include <QSqlError>

#include "dspItemCostSummary.h"
#include "maintainItemCosts.h"

dspCostedBOMBase::dspCostedBOMBase(QWidget* parent, const char* name, Qt::WFlags fl)
    : display(parent, name, fl)
{
  setupUi(optionsWidget());
  setListLabel(tr("Costed Bill of Materials"));
  setMetaSQLOptions("costedBOM", "detail");
  setUseAltId(true);

  QButtonGroup* _costsGroupInt = new QButtonGroup(this);
  _costsGroupInt->addButton(_useStandardCosts);
  _costsGroupInt->addButton(_useActualCosts);

  _item->setType(ItemLineEdit::cGeneralManufactured | ItemLineEdit::cGeneralPurchased |
                 ItemLineEdit::cPhantom | ItemLineEdit::cKit |
                 ItemLineEdit::cPlanning |ItemLineEdit::cTooling);

  list()->addColumn(tr("Seq #"),      _itemColumn, Qt::AlignLeft,  true, "bomdata_bomwork_seqnumber");
  list()->addColumn(tr("Item Number"),_itemColumn, Qt::AlignLeft,  true, "bomdata_item_number");
  list()->addColumn(tr("Description"),         -1, Qt::AlignLeft,  true, "bomdata_itemdescription");
  list()->addColumn(tr("UOM"),         _uomColumn, Qt::AlignCenter,true, "bomdata_uom_name");
  list()->addColumn(tr("Batch Sz."),   _qtyColumn, Qt::AlignRight, true, "bomdata_batchsize");
  list()->addColumn(tr("Fxd. Qty."),   _qtyColumn, Qt::AlignRight, true, "bomdata_qtyfxd");
  list()->addColumn(tr("Qty. Per"),    _qtyColumn, Qt::AlignRight, true, "bomdata_qtyper");
  list()->addColumn(tr("Scrap %"),   _prcntColumn, Qt::AlignRight, true, "bomdata_scrap");
  list()->addColumn(tr("Effective"),  _dateColumn, Qt::AlignCenter,true, "bomdata_effective");
  list()->addColumn(tr("Expires"),    _dateColumn, Qt::AlignCenter,true, "bomdata_expires");
  list()->addColumn(tr("Unit Cost"),  _costColumn, Qt::AlignRight, true, "unitcost");
  list()->addColumn(tr("Ext. Cost"),  _priceColumn,Qt::AlignRight, true, "extendedcost");

  connect(omfgThis, SIGNAL(bomsUpdated(int, bool)), this, SLOT(sFillList(int, bool)));

  _revision->setMode(RevisionLineEdit::View);
  _revision->setType("BOM");

  _revision->setVisible(_metrics->boolean("RevControl"));
}

void dspCostedBOMBase::languageChange()
{
  display::languageChange();
  retranslateUi(this);
}

enum SetResponse dspCostedBOMBase::set(const ParameterList &pParams)
{
  XWidget::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("item_id", &valid);
  {
    _item->setId(param.toInt());
    _item->setReadOnly(true);
    param = pParams.value("revision_id", &valid);
    if (valid)
      _revision->setId(param.toInt());
  }

  if (pParams.inList("run"))
  {
    sFillList();
    return NoError_Run;
  }

  return NoError;
}

bool dspCostedBOMBase::setParams(ParameterList &params)
{
  if (! _item->isValid())
  {
    QMessageBox::critical(this, tr("Need Item Number"),
                          tr("You must select an Item to see its BOM."));
    _item->setFocus();
    return false;
  }

  params.append("item_id",     _item->id());
  params.append("revision_id", _revision->id());

  if(_useStandardCosts->isChecked())
    params.append("useStandardCosts");

  if(_useActualCosts->isChecked())
    params.append("useActualCosts");

  params.append("always", tr("Always"));
  params.append("never",  tr("Never"));

  return true;
}

void dspCostedBOMBase::sPopulateMenu(QMenu *pMenu, QTreeWidgetItem *pSelected, int)
{
  if (((XTreeWidgetItem *)pSelected)->id() != -1)
    pMenu->addAction(tr("Maintain Item Costs..."), this, SLOT(sMaintainItemCosts()));

  if (((XTreeWidgetItem *)pSelected)->id() != -1)
    pMenu->addAction(tr("View Item Costing..."), this, SLOT(sViewItemCosting()));
}

void dspCostedBOMBase::sMaintainItemCosts()
{
  ParameterList params;
  params.append("item_id", list()->altId());

  maintainItemCosts *newdlg = new maintainItemCosts();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspCostedBOMBase::sViewItemCosting()
{
  ParameterList params;
  params.append( "item_id", list()->altId() );
  params.append( "run",     true              );

  dspItemCostSummary *newdlg = new dspItemCostSummary();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspCostedBOMBase::sFillList()
{
  display::sFillList();
}

void dspCostedBOMBase::sFillList(int pItemid, bool)
{
  if (pItemid != _item->id())
    return;
  sFillList();
}

