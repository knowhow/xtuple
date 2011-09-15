/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspItemCostDetail.h"

#include <QSqlError>
#include <QMessageBox>
#include <QVariant>

dspItemCostDetail::dspItemCostDetail(QWidget* parent, const char*, Qt::WFlags fl)
    : display(parent, "dspItemCostDetail", fl)
{
  setupUi(optionsWidget());
  setWindowTitle(tr("Detailed Item Costs"));
  setListLabel(tr("Costing Detail"));
  setReportName("ItemCostDetail");
  setMetaSQLOptions("itemcostdetail", "detail");
  setUseAltId(true);

  QButtonGroup* _costTypeGroupInt = new QButtonGroup(this);
  _costTypeGroupInt->addButton(_standardCosts);
  _costTypeGroupInt->addButton(_actualCosts);

  connect(_item, SIGNAL(newId(int)), this, SLOT(sPopulate()));

  list()->addColumn(tr("#"),               _seqColumn,   Qt::AlignCenter,true, "seqnumber");
  list()->addColumn(tr("Item Number"),     _itemColumn,  Qt::AlignLeft,  true, "item_number");
  list()->addColumn(tr("Description"),     -1,           Qt::AlignLeft,  true, "itemdescrip");
  list()->addColumn(tr("UOM"),             _uomColumn,   Qt::AlignCenter,true, "uom_name");
  list()->addColumn(tr("Qty. Per"),        _qtyColumn,   Qt::AlignRight, true, "qtyper");
  list()->addColumn(tr("Scrap/Absorb. %"), _itemColumn,  Qt::AlignRight, true, "scrap" );
  list()->addColumn(tr("Unit Cost"),       _costColumn,  Qt::AlignRight, true, "cost");
  list()->addColumn(tr("Ext'd Cost"),      _moneyColumn, Qt::AlignRight, true, "extendedcost");

  connect(omfgThis, SIGNAL(bomsUpdated(int, bool)), SLOT(sFillList(int, bool)));
}

void dspItemCostDetail::languageChange()
{
  display::languageChange();
  retranslateUi(this);
}

enum SetResponse dspItemCostDetail::set(const ParameterList &pParams)
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

  param = pParams.value("costtype", &valid);
  if (valid)
  {
    for (int cursor = 0; cursor < _costType->count(); cursor++)
    {
      if (_costType->itemText(cursor) == param.toString())
      {
        _costType->setCurrentIndex(cursor);
        break;
      }
    }
  }

  param = pParams.value("itemcost_id", &valid);
  if (valid)
  {
    XSqlQuery qq;
    qq.prepare( "SELECT itemcost_item_id, itemcost_costelem_id "
               "FROM itemcost "
               "WHERE (itemcost_id=:itemcost_id)" );
    qq.bindValue(":itemcost_id", param.toInt());
    qq.exec();
    if (qq.first())
    {
      _item->setId(qq.value("itemcost_item_id").toInt());
      _item->setReadOnly(true);
      _costType->setId(qq.value("itemcost_costelem_id").toInt());
      _costType->setEnabled(false);
    }
    else if (qq.lastError().type() != QSqlError::NoError)
    {
      systemError(this, qq.lastError().databaseText(), __FILE__, __LINE__);
      return UndefinedError;
    }
  }

  if (pParams.inList("run"))
  {
    sFillList();
    return NoError_Run;
  }

  return NoError;
}

void dspItemCostDetail::sPopulate()
{
  XSqlQuery qq;
  qq.prepare( "SELECT costelem_id, costelem_type, costelem_type "
             "FROM costelem, itemcost "
             "WHERE ( (itemcost_costelem_id=costelem_id)"
             " AND (itemcost_lowlevel)"
             " AND (itemcost_item_id=:item_id) );" );
  qq.bindValue(":item_id", _item->id());
  qq.exec();
  _costType->populate(qq);
  if (qq.lastError().type() != QSqlError::NoError)
  {
    systemError(this, qq.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

bool dspItemCostDetail::setParams(ParameterList &params)
{
  if(!_item->isValid())
  {
    QMessageBox::warning( this, tr("Item Required"),
      tr("You must specify an Item Number."));
    return false;
  }

  params.append("item_id", _item->id());
  params.append("costelem_id", _costType->id());

  if (_standardCosts->isChecked())
    params.append("standardCost");
  else
    params.append("actualCost");

  if ( (_item->itemType() == "M") ||
       (_item->itemType() == "F") ||
       (_item->itemType() == "B") ||
       (_item->itemType() == "T") ||
       (_item->itemType() == "Y") ||
       (_item->itemType() == "R") ||
       (_item->itemType() == "O") ||
       (_item->itemType() == "P") )
    params.append("useBOM");
  else if (_item->itemType() == "C")
    params.append("useBBOM");

  if (_metrics->value("Application") != "PostBooks")
    params.append("includeRevisionControl");

  return true;
}

