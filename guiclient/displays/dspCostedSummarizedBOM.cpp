/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspCostedSummarizedBOM.h"

#include <QVariant>
#include <QMessageBox>

dspCostedSummarizedBOM::dspCostedSummarizedBOM(QWidget* parent, const char*, Qt::WFlags fl)
    : display(parent, "dspCostedSummarizedBOM", fl)
{
  setupUi(optionsWidget());
  setWindowTitle(tr("Costed Summarized Bill of Materials"));
  setListLabel(tr("Costed Bill of Material"));
  setReportName("CostedSummarizedBOM");
  setMetaSQLOptions("costedBOM", "detail");

  QButtonGroup* _costsGroupInt = new QButtonGroup(this);
  _costsGroupInt->addButton(_useStandardCosts);
  _costsGroupInt->addButton(_useActualCosts);

  _item->setType(ItemLineEdit::cGeneralManufactured | ItemLineEdit::cGeneralPurchased |
                 ItemLineEdit::cPhantom | ItemLineEdit::cKit |
                 ItemLineEdit::cPlanning | ItemLineEdit::cTooling);

  list()->setRootIsDecorated(true);
  list()->addColumn(tr("Item Number"),  _itemColumn, Qt::AlignLeft,  true, "bomdata_item_number");
  list()->addColumn(tr("Description"),           -1, Qt::AlignLeft,  true, "bomdata_itemdescription");
  list()->addColumn(tr("UOM"),           _uomColumn, Qt::AlignCenter,true, "bomdata_uom_name");
  list()->addColumn(tr("Qty. Req."),     _qtyColumn, Qt::AlignRight, true, "bomdata_qtyreq");
  list()->addColumn(tr("Unit Cost"),    _costColumn, Qt::AlignRight, true, "unitcost");
  list()->addColumn(tr("Ext. Cost"),    _priceColumn, Qt::AlignRight, true, "extendedcost");
  list()->setIndentation(10);

  _expiredDaysLit->setEnabled(_showExpired->isChecked());
  _expiredDays->setEnabled(_showExpired->isChecked());
  _effectiveDaysLit->setEnabled(_showFuture->isChecked());
  _effectiveDays->setEnabled(_showFuture->isChecked());

  connect(omfgThis, SIGNAL(bomsUpdated(int, bool)), this, SLOT(sFillList(int, bool)));

  _revision->setMode(RevisionLineEdit::View);
  _revision->setType("BOM");

  _revision->setVisible(_metrics->boolean("RevControl"));
}

void dspCostedSummarizedBOM::languageChange()
{
  display::languageChange();
  retranslateUi(this);
}

enum SetResponse dspCostedSummarizedBOM::set(const ParameterList &pParams)
{
  XWidget::set(pParams);
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
    return NoError_Print;
  }

  return NoError;
}

bool dspCostedSummarizedBOM::setParams(ParameterList &params)
{
  if(!_item->isValid())
  {
    QMessageBox::warning(this, tr("Item Required"),
      tr("You must specify an Item Number."));
    return false;
  }

  params.append("item_id", _item->id());
  params.append("revision_id", _revision->id());

  if(_showExpired->isChecked())
    params.append("expiredDays", _expiredDays->value());
  else
    params.append("expiredDays", 0);

  if(_showFuture->isChecked())
    params.append("futureDays", _effectiveDays->value());
  else
    params.append("futureDays", 0);

  if (_useStandardCosts->isChecked())
    params.append("useStandardCosts");

  if (_useActualCosts->isChecked())
    params.append("useActualCosts");
  params.append("summarizedBOM");

  return true;
}

void dspCostedSummarizedBOM::sFillList()
{
  display::sFillList();
  list()->collapseAll();
}

void dspCostedSummarizedBOM::sFillList(int pid, bool)
{
  if (pid == _item->id())
    sFillList();
}
