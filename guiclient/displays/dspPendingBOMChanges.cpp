/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspPendingBOMChanges.h"

#include <QAction>
#include <QMenu>
#include <QMessageBox>

#include <parameter.h>

#include "bomItem.h"

dspPendingBOMChanges::dspPendingBOMChanges(QWidget* parent, const char*, Qt::WFlags fl)
  : display(parent, "dspPendingBOMChanges", fl)
{
  setupUi(optionsWidget());
  setWindowTitle(tr("Pending Bill of Materials Changes"));
  setListLabel(tr("Pending Bill of Materials Changes"));
  setReportName("PendingBOMChanges");
  setMetaSQLOptions("pendingBOMChanges", "detail");

  _item->setType(ItemLineEdit::cGeneralManufactured | ItemLineEdit::cGeneralPurchased |
                 ItemLineEdit::cPhantom | ItemLineEdit::cKit |
                 ItemLineEdit::cPlanning | ItemLineEdit::cTooling);

  _cutoff->setNullString(tr("Latest"));
  _cutoff->setNullDate(omfgThis->endOfTime().addDays(-1));
  _cutoff->setAllowNullDate(true);
  _cutoff->setNull();

  list()->addColumn(tr("Date"),        _dateColumn,  Qt::AlignCenter, true,  "actiondate" );
  list()->addColumn(tr("Action"),      _itemColumn,  Qt::AlignCenter, true,  "action" );
  list()->addColumn(tr("Seq #"),       40,           Qt::AlignCenter, true,  "bomitem_seqnumber"  );
  list()->addColumn(tr("Item Number"), _itemColumn,  Qt::AlignLeft,   true,  "item_number"   );
  list()->addColumn(tr("Description"), -1,           Qt::AlignCenter, true,  "description" );
  list()->addColumn(tr("UOM"),         _uomColumn,   Qt::AlignCenter, true,  "uom_name" );
  list()->addColumn(tr("Fxd. Qty."),   _qtyColumn,   Qt::AlignRight,  true,  "qtyfxd"  );
  list()->addColumn(tr("Qty. Per"),    _qtyColumn,   Qt::AlignRight,  true,  "qtyper"  );
  list()->addColumn(tr("Scrap %"),     _prcntColumn, Qt::AlignRight,  true,  "bomitem_scrap"  );
  
  connect(omfgThis, SIGNAL(bomsUpdated(int, bool)), SLOT(sFillList()));
  _revision->setMode(RevisionLineEdit::View);
  _revision->setType("BOM");

  //If not Revision Control, hide control
  _revision->setVisible(_metrics->boolean("RevControl"));
}

void dspPendingBOMChanges::languageChange()
{
  display::languageChange();
  retranslateUi(this);
}

void dspPendingBOMChanges::sPopulateMenu(QMenu *pMenu, QTreeWidgetItem *, int)
{
  QAction *menuItem;

  menuItem = pMenu->addAction(tr("Edit BOM Item..."), this, SLOT(sEdit()));;
  if (!_privileges->check("MaintainBOMs"))
    menuItem->setEnabled(false);

  menuItem = pMenu->addAction(tr("View BOM Item..."), this, SLOT(sView()));;
  if ( (!_privileges->check("MaintainBOMs")) && (!_privileges->check("ViewBOMs")) )
    menuItem->setEnabled(false);
}

void dspPendingBOMChanges::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("bomitem_id", list()->id());

  bomItem newdlg(this, "", true);
  newdlg.set(params);
  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void dspPendingBOMChanges::sView()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("bomitem_id", list()->id());

  bomItem newdlg(this, "", true);
  newdlg.set(params);
  newdlg.exec();
}

bool dspPendingBOMChanges::setParams(ParameterList &params)
{
  if(!_item->isValid())
  {
    QMessageBox::warning( this, tr("Item Required"),
      tr("You must specify a valid item."));
    return false;
  }
  params.append("effective", tr("Effective"));
  params.append("expires", tr("Expires"));

  params.append("item_id", _item->id());
  params.append("revision_id", _revision->id());
  params.append("cutOffDate", _cutoff->date());

  return true;
}

