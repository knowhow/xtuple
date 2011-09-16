/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "bomList.h"

#include <QMessageBox>

#include <openreports.h>
#include <parameter.h>

#include "bom.h"
#include "copyBOM.h"

bomList::bomList(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  connect(_copy, SIGNAL(clicked()), this, SLOT(sCopy()));
  connect(_delete, SIGNAL(clicked()), this, SLOT(sDelete()));
  connect(_edit, SIGNAL(clicked()), this, SLOT(sEdit()));
  connect(_new, SIGNAL(clicked()), this, SLOT(sNew()));
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_searchFor, SIGNAL(textChanged(const QString&)), this, SLOT(sSearch(const QString&)));
  connect(_showInactive, SIGNAL(toggled(bool)), this, SLOT(sFillList()));
  connect(_showComponent, SIGNAL(toggled(bool)), this, SLOT(sFillList()));
  connect(_view, SIGNAL(clicked()), this, SLOT(sView()));

  _bom->addColumn(tr("Item Number"), _itemColumn, Qt::AlignLeft, true, "item_number");
  _bom->addColumn(tr("Description"), -1,          Qt::AlignLeft, true, "descrip");
  
  connect(omfgThis, SIGNAL(bomsUpdated(int, bool)), SLOT(sFillList(int, bool)));
  
  if (_privileges->check("MaintainBOMs"))
  {
    connect(_bom, SIGNAL(valid(bool)), _edit, SLOT(setEnabled(bool)));
    connect(_bom, SIGNAL(valid(bool)), _copy, SLOT(setEnabled(bool)));
    connect(_bom, SIGNAL(valid(bool)), this, SLOT(sHandleButtons()));

    connect(_bom, SIGNAL(itemSelected(int)), _edit, SLOT(animateClick()));
  }
  else
  {
    _new->setEnabled(FALSE);
    connect(_bom, SIGNAL(itemSelected(int)), _view, SLOT(animateClick()));
  }

  sFillList(-1, FALSE);

  _searchFor->setFocus();
}

bomList::~bomList()
{
  // no need to delete child widgets, Qt does it all for us
}

void bomList::languageChange()
{
  retranslateUi(this);
}

void bomList::sCopy()
{
  ParameterList params;
  params.append("item_id", _bom->id());

  copyBOM newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void bomList::sDelete()
{
  if (QMessageBox::critical( this, tr("Delete Bill of Materials"),
                             tr( "Are you sure that you want to delete the selected Bill of Materials?"),
                             tr("&Yes"), tr("&No"), QString::null, 0, 1) == 0)
  {
    q.prepare( "SELECT deletebom(:item_id);" );
    q.bindValue(":item_id", _bom->id());
    q.exec();

    omfgThis->sBOMsUpdated(-1, TRUE);
  }
}

void bomList::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("item_id", _bom->id());

  BOM *newdlg = new BOM();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void bomList::sFillList( int pItemid, bool pLocal )
{
  QString sql;

  sql = "SELECT DISTINCT item_id, "
        " CASE WHEN "
        "  COALESCE(bomitem_rev_id, -1)=-1 THEN "
        "   0 "
        " ELSE 1 "
        " END AS revcontrol, "
        " item_number, (item_descrip1 || ' ' || item_descrip2) AS descrip "
        "FROM item "
        "  LEFT OUTER JOIN bomitem ON (item_id=bomitem_parent_item_id) "
        "  LEFT OUTER JOIN bomhead ON (item_id=bomhead_item_id) "
        "WHERE (((bomitem_id IS NOT NULL) "
        "OR (bomhead_id IS NOT NULL)) ";

  if (!_showInactive->isChecked())
    sql += " AND (item_active)";

  if (_showComponent->isChecked())
    sql += " AND (bomitem_id is not null)";
  sql += ") "
         "ORDER BY item_number;";

  if ((pItemid != -1) && (pLocal))
    _bom->populate(sql, TRUE, pItemid);
  else
    _bom->populate(sql, TRUE);
}

void bomList::sNew()
{
  ParameterList params;
  params.append("mode", "new");

  BOM *newdlg = new BOM();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void bomList::sPrint()
{
  ParameterList params;
  params.append( "item_id", _bom->id() );

  orReport report("SingleLevelBOM", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void bomList::sSearch( const QString &pTarget )
{
  _bom->clearSelection();
  int i;
  for (i = 0; i < _bom->topLevelItemCount(); i++)
  {
   if (_bom->topLevelItem(i)->text(0).startsWith(pTarget, Qt::CaseInsensitive))
    break;
  }
    
  if (i < _bom->topLevelItemCount())
  {
    _bom->setCurrentItem(_bom->topLevelItem(i));
    _bom->scrollToItem(_bom->topLevelItem(i));
  }
}

void bomList::sView()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("item_id", _bom->id());

  BOM *newdlg = new BOM();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void bomList::sFillList()
{
  sFillList(-1, TRUE);
}

void bomList::sHandleButtons()
{
  if (_bom->altId() == 0)
    _delete->setEnabled(TRUE);
  else
    _delete->setEnabled(FALSE);
}
