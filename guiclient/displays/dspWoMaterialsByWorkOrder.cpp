/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspWoMaterialsByWorkOrder.h"

#include <QVariant>
#include <QMessageBox>
#include <QAction>
#include <QMenu>

#include "inputManager.h"
#include "woMaterialItem.h"

dspWoMaterialsByWorkOrder::dspWoMaterialsByWorkOrder(QWidget* parent, const char*, Qt::WFlags fl)
  : display(parent, "dspWoMaterialsByWorkOrder", fl)
{
  setupUi(optionsWidget());
  setWindowTitle(tr("Work Order Material Requirements By Work Order"));
  setListLabel(tr("Material Requirements"));
  setReportName("WOMaterialRequirementsByWorkOrder");
  setMetaSQLOptions("workOrderMaterial", "detail");

  _wo->setType(cWoExploded | cWoIssued | cWoReleased);
  _wo->setFocus();

  omfgThis->inputManager()->notify(cBCWorkOrder, this, _wo, SLOT(setId(int)));

  _manufacturing = false;
  if (_metrics->value("Application") == "Standard")
  {
    XSqlQuery xtmfg;
    xtmfg.exec("SELECT pkghead_name FROM pkghead WHERE pkghead_name='xtmfg'");
    if (xtmfg.first())
      _manufacturing = true;
  }

  list()->addColumn(tr("Component Item"),  _itemColumn,  Qt::AlignLeft,   true,  "item_number"   );
  if (_manufacturing)
    list()->addColumn(tr("Oper. #"),         _dateColumn,  Qt::AlignCenter, true,  "wooperseq" );
  list()->addColumn(tr("Iss. Meth.") ,     _orderColumn, Qt::AlignCenter, true,  "issuemethod" );
  list()->addColumn(tr("Iss. UOM") ,       _uomColumn,   Qt::AlignLeft,   true,  "uom_name"   );
  list()->addColumn(tr("Fxd. Qty."),       _qtyColumn,   Qt::AlignRight,  true,  "womatl_qtyfxd"  );
  list()->addColumn(tr("Qty. Per"),        _qtyColumn,   Qt::AlignRight,  true,  "womatl_qtyper"  );
  list()->addColumn(tr("Scrap %"),         _prcntColumn, Qt::AlignRight,  true,  "womatl_scrap"  );
  list()->addColumn(tr("Required"),        _qtyColumn,   Qt::AlignRight,  true,  "womatl_qtyreq"  );
  list()->addColumn(tr("Issued"),          _qtyColumn,   Qt::AlignRight,  true,  "womatl_qtyiss"  );
  list()->addColumn(tr("Scrapped"),        _qtyColumn,   Qt::AlignRight,  true,  "womatl_qtywipscrap"  );
  list()->addColumn(tr("Balance"),         _qtyColumn,   Qt::AlignRight,  true,  "balance"  );
  list()->addColumn(tr("Due Date"),        _dateColumn,  Qt::AlignCenter, true,  "womatl_duedate" );
}

void dspWoMaterialsByWorkOrder::languageChange()
{
  display::languageChange();
  retranslateUi(this);
}

enum SetResponse dspWoMaterialsByWorkOrder::set(const ParameterList &pParams)
{
  XWidget::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("wo_id", &valid);
  if (valid)
    _wo->setId(param.toInt());

  if (pParams.inList("run"))
  {
    sFillList();
    return NoError_Run;
  }

  return NoError;
}

bool dspWoMaterialsByWorkOrder::setParams(ParameterList & params)
{
  if(!_wo->isValid())
  {
    QMessageBox::warning(this, tr("Work Order Required"),
      tr("You must specify a Work Order Number."));
    return false;
  }

  params.append("wo_id", _wo->id());
  params.append("push", tr("Push"));
  params.append("pull", tr("Pull"));
  params.append("mixed", tr("Mixed"));
  params.append("error", tr("Error"));
  if (_manufacturing)
    params.append("Manufacturing");

  params.append("includeFormatted"); // report only?

  return true;
}

void dspWoMaterialsByWorkOrder::sPopulateMenu(QMenu *pMenu, QTreeWidgetItem *, int)
{
  QAction *menuItem;

  menuItem = pMenu->addAction(tr("View Requirement..."), this, SLOT(sView()));
}

void dspWoMaterialsByWorkOrder::sView()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("womatl_id", list()->id());
  
  woMaterialItem newdlg(this, "", true);
  newdlg.set(params);
  newdlg.exec();
}

