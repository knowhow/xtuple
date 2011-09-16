/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "itemSources.h"

#include <QAction>
#include <QMenu>
#include <QMessageBox>
#include <QVariant>

#include <metasql.h>
#include <parameter.h>
#include <openreports.h>

#include "itemSource.h"

itemSources::itemSources(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  connect(_close,       SIGNAL(clicked()), this, SLOT(close()));
  connect(_copy,        SIGNAL(clicked()), this, SLOT(sCopy()));
  connect(_delete,      SIGNAL(clicked()), this, SLOT(sDelete()));
  connect(_edit,        SIGNAL(clicked()), this, SLOT(sEdit()));
  connect(_itemsrc, SIGNAL(populateMenu(QMenu*, XTreeWidgetItem*)), this, SLOT(sPopulateMenu(QMenu*)));
  connect(_itemsrc,   SIGNAL(valid(bool)),_view, SLOT(setEnabled(bool)));
  connect(_new,         SIGNAL(clicked()), this, SLOT(sNew()));
  connect(_print,       SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_searchFor, SIGNAL(textChanged(const QString&)), this, SLOT(sSearch(const QString&)));
  connect(_showInactive, SIGNAL(toggled(bool)), this, SLOT(sFillList()));
  connect(_view, SIGNAL(clicked()), this, SLOT(sView()));

  _itemsrc->addColumn(tr("Item Number"), _itemColumn, Qt::AlignLeft, true, "item_number" );
  _itemsrc->addColumn(tr("Description"), -1,          Qt::AlignLeft, true, "item_descrip" );
  _itemsrc->addColumn(tr("Vendor"),      _itemColumn, Qt::AlignLeft, true, "vend_name" );
  _itemsrc->addColumn(tr("Vendor Item"), _itemColumn, Qt::AlignLeft, true, "itemsrc_vend_item_number" );
  _itemsrc->addColumn(tr("Manufacturer"), _itemColumn, Qt::AlignLeft, true, "itemsrc_manuf_name" );
  _itemsrc->addColumn(tr("Manuf. Item#"), _itemColumn, Qt::AlignLeft, true, "itemsrc_manuf_item_number" );

  if (_privileges->check("MaintainItemSources"))
  {
    connect(_itemsrc, SIGNAL(valid(bool)), _edit, SLOT(setEnabled(bool)));
    connect(_itemsrc, SIGNAL(valid(bool)), _copy, SLOT(setEnabled(bool)));
    connect(_itemsrc, SIGNAL(valid(bool)), _delete, SLOT(setEnabled(bool)));
    connect(_itemsrc, SIGNAL(itemSelected(int)), _edit, SLOT(animateClick()));
  }
  else
  {
    _new->setEnabled(FALSE);
    connect(_itemsrc, SIGNAL(itemSelected(int)), _view, SLOT(animateClick()));
  }

  sFillList();
  _searchFor->setFocus();
}

itemSources::~itemSources()
{
  // no need to delete child widgets, Qt does it all for us
}

void itemSources::languageChange()
{
  retranslateUi(this);
}

void itemSources::sPrint()
{
  orReport report("ItemSourceMasterList");
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void itemSources::sNew()
{
  ParameterList params;
  params.append("mode", "new");

  itemSource newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void itemSources::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("itemsrc_id", _itemsrc->id());

  itemSource newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void itemSources::sView()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("itemsrc_id", _itemsrc->id());

  itemSource newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void itemSources::sCopy()
{
  ParameterList params;
  params.append("mode", "copy");
  params.append("itemsrc_id", _itemsrc->id());

  itemSource newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void itemSources::sDelete()
{
  q.prepare("SELECT poitem_id, itemsrc_active "
            "FROM poitem, itemsrc "
            "WHERE ((poitem_itemsrc_id=:itemsrc_id) "
            "AND (itemsrc_id=:itemsrc_id)); ");
  q.bindValue(":itemsrc_id", _itemsrc->id());
  q.exec();
  if (q.first())
  {
    if (q.value("itemsrc_active").toBool())
    {
      if (QMessageBox::question(this, tr("Delete Item Source"),
                                tr("<p>This item source is used by existing "
                                   "purchase order records and may not be "
                                   "deleted. Would you like to deactivate it "
                                   "instead?"),
                                QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes)
      {
        q.prepare( "UPDATE itemsrc SET "
                   "  itemsrc_active=false "
                   "WHERE (itemsrc_id=:itemsrc_id);" );
        q.bindValue(":itemsrc_id", _itemsrc->id());
        q.exec();

        sFillList();
      }
    }
    else
      QMessageBox::critical(this, tr("Delete Item Source"),
                            tr("<p>This item source is used by existing "
                               "purchase order records and may not be deleted."));
    return;
  }
            
  q.prepare( "SELECT item_number "
             "FROM itemsrc, item "
             "WHERE ( (itemsrc_item_id=item_id)"
             " AND (itemsrc_id=:itemsrc_id) );" );
  q.bindValue(":itemsrc_id", _itemsrc->id());
  q.exec();
  if (q.first())
  {
    if (QMessageBox::question(this, tr("Delete Item Source"),
                              tr( "Are you sure that you want to delete the "
                                 "Item Source for %1?")
                                  .arg(q.value("item_number").toString()),
                                  QMessageBox::Yes | QMessageBox::No,
                                  QMessageBox::No) == QMessageBox::Yes)
    {
      q.prepare( "DELETE FROM itemsrc "
                 "WHERE (itemsrc_id=:itemsrc_id);"
                 "DELETE FROM itemsrcp "
                 "WHERE (itemsrcp_itemsrc_id=:itemsrc_id);" );
      q.bindValue(":itemsrc_id", _itemsrc->id());
      q.exec();

      sFillList();
    }
  }
}

void itemSources::sPopulateMenu(QMenu *menuThis)
{
  QAction *menuItem;

  menuItem = menuThis->addAction(tr("Edit Item Source..."), this, SLOT(sEdit()));
  menuItem->setEnabled(_privileges->check("MaintainItemSources"));

  menuItem = menuThis->addAction(tr("View Item Source..."), this, SLOT(sView()));
  menuItem->setEnabled(_privileges->check("MaintainItemSources") || _privileges->check("ViewItemSource"));

  menuItem = menuThis->addAction(tr("Delete Item Source..."), this, SLOT(sDelete()));
  menuItem->setEnabled(_privileges->check("MaintainItemSources"));
}

void itemSources::sFillList()
{
  QString sql( "SELECT itemsrc_id, item_number, (item_descrip1 || ' ' || item_descrip2) AS item_descrip,"
               "       vend_name, itemsrc_vend_item_number, itemsrc_manuf_name, "
               "       itemsrc_manuf_item_number "
               "FROM item, vend, itemsrc "
               "WHERE ( (itemsrc_item_id=item_id)"
               " AND (itemsrc_vend_id=vend_id)"
               "<? if exists(\"onlyShowActive\") ?>"
               " AND (itemsrc_active)"
               "<? endif ?>"
               ") ORDER BY item_number, vend_name;" );
               
  ParameterList params;
  if (!_showInactive->isChecked())
    params.append("onlyShowActive");

  MetaSQLQuery mql(sql);
  q = mql.toQuery(params);
  _itemsrc->populate(q);
}

void itemSources::sSearch( const QString &pTarget )
{
  _itemsrc->clearSelection();
  int i;
  for (i = 0; i < _itemsrc->topLevelItemCount(); i++)
  {
    if ( (_itemsrc->topLevelItem(i)->text(0).startsWith(pTarget, Qt::CaseInsensitive)) ||
         (_itemsrc->topLevelItem(i)->text(1).startsWith(pTarget, Qt::CaseInsensitive)) ||
         (_itemsrc->topLevelItem(i)->text(2).startsWith(pTarget, Qt::CaseInsensitive)) ||
         (_itemsrc->topLevelItem(i)->text(3).startsWith(pTarget, Qt::CaseInsensitive)) ||
         (_itemsrc->topLevelItem(i)->text(4).startsWith(pTarget, Qt::CaseInsensitive)) ||
         (_itemsrc->topLevelItem(i)->text(5).startsWith(pTarget, Qt::CaseInsensitive)) )
      break;
  }

  if (i < _itemsrc->topLevelItemCount())
  {
    _itemsrc->setCurrentItem(_itemsrc->topLevelItem(i));
    _itemsrc->scrollToItem(_itemsrc->topLevelItem(i));
  }
}

