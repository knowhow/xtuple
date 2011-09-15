/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspBOMBase.h"

#include <QVariant>
#include <QMessageBox>

#include "item.h"

dspBOMBase::dspBOMBase(QWidget* parent, const char* name, Qt::WFlags fl)
    : display(parent, name, fl)
{
  setupUi(optionsWidget());
  setListLabel(tr("Bill Of Materials"));
  setMetaSQLOptions("bom", "detail");

  connect(_item, SIGNAL(valid(bool)), _revision, SLOT(setEnabled(bool)));
  connect(omfgThis, SIGNAL(bomsUpdated(int, bool)), this, SLOT(sFillList()));

  _item->setType(ItemLineEdit::cGeneralManufactured | ItemLineEdit::cGeneralPurchased |
                 ItemLineEdit::cPhantom | ItemLineEdit::cKit |
                 ItemLineEdit::cPlanning | ItemLineEdit::cTooling);

  _expiredDaysLit->setEnabled(_showExpired->isChecked());
  _expiredDays->setEnabled(_showExpired->isChecked());
  _effectiveDaysLit->setEnabled(_showFuture->isChecked());
  _effectiveDays->setEnabled(_showFuture->isChecked());

  _item->setFocus();
  _revision->setEnabled(false);
  _revision->setMode(RevisionLineEdit::View);
  _revision->setType("BOM");

  _revision->setVisible(_metrics->boolean("RevControl"));
}

void dspBOMBase::languageChange()
{
  display::languageChange();
  retranslateUi(this);
}

enum SetResponse dspBOMBase::set(const ParameterList &pParams)
{
  display::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("item_id", &valid);
  if (valid)
  {
    _item->setId(param.toInt());
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

bool dspBOMBase::setParams(ParameterList &params)
{
  if(!_item->isValid())
  {
    QMessageBox::warning(this, tr("Invalid Item"),
      tr("You must specify a valid item.") );
    return false;
  }

  params.append("item_id", _item->id());
  params.append("revision_id", _revision->id());

  if (_showExpired->isChecked())
    params.append("expiredDays", _expiredDays->value());
  else
    params.append("expiredDays", 0);

  if(_showFuture->isChecked())
  {
    params.append("futureDays", _effectiveDays->value());
    params.append("effectiveDays", _effectiveDays->value()); // used by dspSingleBOM
  }
  else
    params.append("futureDays", 0);

  params.append("always", tr("Always"));
  params.append("never",  tr("Never"));

  return true;
}

void dspBOMBase::sPopulateMenu(QMenu *pMenu, QTreeWidgetItem *, int)
{
  QAction *menuItem;

  menuItem = pMenu->addAction(tr("Edit..."), this, SLOT(sEdit()));;
  if (!_privileges->check("MaintainItemMasters"))
    menuItem->setEnabled(false);

  menuItem = pMenu->addAction(tr("View..."), this, SLOT(sView()));;
}

void dspBOMBase::sEdit()
{
  item::editItem(list()->id());
}

void dspBOMBase::sView()
{
  item::viewItem(list()->id());
}
