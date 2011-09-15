/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspCountTagsBase.h"

#include <QAction>
#include <QMenu>
#include <QVariant>

#include <parameter.h>

#include "countTag.h"

dspCountTagsBase::dspCountTagsBase(QWidget* parent, const char* name, Qt::WFlags fl)
  : display(parent, name, fl)
{
  setupUi(optionsWidget());
  setListLabel(tr("Count Tags"));
  setMetaSQLOptions("countTags", "detail");

  _dates->setStartNull(tr("Earliest"), omfgThis->startOfTime(), true);
  _dates->setEndNull(tr("Latest"), omfgThis->endOfTime(), true);

  list()->addColumn(tr("Tag #"),               -1, Qt::AlignLeft,  true, "invcnt_tagnumber");
  list()->addColumn(tr("Site"),        _whsColumn, Qt::AlignCenter,true, "warehous_code");
  list()->addColumn(tr("Item"),       _itemColumn, Qt::AlignLeft,  true, "item_number");
  list()->addColumn(tr("Created"),    _dateColumn, Qt::AlignCenter,true, "invcnt_tagdate");
  list()->addColumn(tr("Created By"), _dateColumn, Qt::AlignCenter,true, "creator");
  list()->addColumn(tr("Entered"),    _dateColumn, Qt::AlignCenter,true, "invcnt_cntdate");
  list()->addColumn(tr("Entered By"), _dateColumn, Qt::AlignCenter,true, "counter");
  list()->addColumn(tr("Posted"),     _dateColumn, Qt::AlignCenter,true, "invcnt_postdate");
  list()->addColumn(tr("Posted By"),  _dateColumn, Qt::AlignCenter,true, "poster");
  list()->addColumn(tr("QOH Before"),  _qtyColumn, Qt::AlignRight, true, "qohbefore");
  list()->addColumn(tr("Qty. Counted"),_qtyColumn, Qt::AlignRight, true, "invcnt_qoh_after");
  list()->addColumn(tr("Variance"),    _qtyColumn, Qt::AlignRight, true, "variance");
  list()->addColumn(tr("%"),         _prcntColumn, Qt::AlignRight, true, "percent");

  if (_preferences->boolean("XCheckBox/forgetful"))
    _showUnposted->setChecked(true);
  
  sFillList();
}

void dspCountTagsBase::languageChange()
{
  display::languageChange();
  retranslateUi(this);
}

bool dspCountTagsBase::setParams(ParameterList &params)
{
  if(_item->isVisibleTo(this))
  {
    if (! _item->isValid())
    {
      _item->setFocus();
      return false;
    }
    params.append("item_id", _item->id());
  }
  if (! _dates->allValid())
  {
    _dates->setFocus();
    return false;
  }

  if(_parameter->isVisibleTo(this))
    _parameter->appendValue(params);
  _warehouse->appendValue(params);
  _dates->appendValue(params);

  if (_showUnposted->isChecked())
    params.append("showUnposted");

  return true;
}

void dspCountTagsBase::sPopulateMenu(QMenu *pMenu, QTreeWidgetItem*, int)
{
  pMenu->addAction(tr("View Count Tag..."), this, SLOT(sView()));
}

void dspCountTagsBase::sView()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("cnttag_id", list()->id());

  countTag newdlg(this, "", true);
  newdlg.set(params);
  newdlg.exec();
}

