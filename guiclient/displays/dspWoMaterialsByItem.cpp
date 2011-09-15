/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspWoMaterialsByItem.h"

#include <QVariant>
#include <QMessageBox>

#include "guiclient.h"
#include "inputManager.h"

dspWoMaterialsByItem::dspWoMaterialsByItem(QWidget* parent, const char*, Qt::WFlags fl)
  : display(parent, "dspWoMaterialsByItem", fl)
{
  setupUi(optionsWidget());
  setWindowTitle(tr("W/O Material Requirements by Component Item"));
  setListLabel(tr("Material Requirements"));
  setReportName("WOMaterialRequirementsByComponentItem");
  setMetaSQLOptions("workOrderMaterial", "detail");

  omfgThis->inputManager()->notify(cBCItem, this, _item, SLOT(setItemid(int)));
  omfgThis->inputManager()->notify(cBCItemSite, this, _item, SLOT(setItemsiteid(int)));

  _manufacturing = false;
  if (_metrics->value("Application") == "Standard")
  {
    XSqlQuery xtmfg;
    xtmfg.exec("SELECT pkghead_name FROM pkghead WHERE pkghead_name='xtmfg'");
    if (xtmfg.first())
      _manufacturing = true;
  }

  list()->addColumn(tr("W/O #"),         _orderColumn, Qt::AlignLeft,   true,  "wonumber"   );
  list()->addColumn(tr("Parent Item #"), _itemColumn,  Qt::AlignLeft,   true,  "item_number"   );
  if (_manufacturing)
    list()->addColumn(tr("Oper. #"),       _dateColumn,  Qt::AlignCenter, true,  "wooperseq" );
  list()->addColumn(tr("Iss. Meth."),    _dateColumn,  Qt::AlignCenter, true,  "issuemethod" );
  list()->addColumn(tr("Iss. UOM"),      _uomColumn,   Qt::AlignLeft,   true,  "uom_name"   );
  list()->addColumn(tr("Fxd. Qty."),     _qtyColumn,   Qt::AlignRight,  true,  "womatl_qtyfxd"  );
  list()->addColumn(tr("Qty. Per"),      _qtyColumn,   Qt::AlignRight,  true,  "womatl_qtyper"  );
  list()->addColumn(tr("Scrap %"),       _prcntColumn, Qt::AlignRight,  true,  "womatl_scrap"  );
  list()->addColumn(tr("Required"),      _qtyColumn,   Qt::AlignRight,  true,  "womatl_qtyreq"  );
  list()->addColumn(tr("Issued"),        _qtyColumn,   Qt::AlignRight,  true,  "womatl_qtyiss"  );
  list()->addColumn(tr("Scrapped"),      _qtyColumn,   Qt::AlignRight,  true,  "womatl_qtywipscrap"  );
  list()->addColumn(tr("Balance"),       _qtyColumn,   Qt::AlignRight,  true,  "balance"  );
  list()->addColumn(tr("Due Date"),      _dateColumn,  Qt::AlignCenter, true,  "womatl_duedate" );
}

void dspWoMaterialsByItem::languageChange()
{
  display::languageChange();
  retranslateUi(this);
}

bool dspWoMaterialsByItem::setParams(ParameterList & params)
{
  if(!_item->isValid())
  {
    QMessageBox::warning(this, tr("Item Required"),
      tr("You must specify an Item Number."));
    return false;
  }

  params.append("item_id", _item->id());
  _warehouse->appendValue(params);
  params.append("push", tr("Push"));
  params.append("pull", tr("Pull"));
  params.append("mixed", tr("Mixed"));
  params.append("error", tr("Error"));
  if (_manufacturing)
    params.append("Manufacturing");

  params.append("includeFormatted"); // report only?

  return true;
}

