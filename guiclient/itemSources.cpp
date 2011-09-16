/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
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

#include "itemSource.h"
#include "buyCard.h"
#include "dspPoItemsByVendor.h"
#include "guiclient.h"
#include "parameterwidget.h"

itemSources::itemSources(QWidget* parent, const char*, Qt::WFlags fl)
  : display(parent, "itemSources", fl)
{
  setWindowTitle(tr("Item Sources"));
  setReportName("ItemSources");
  setMetaSQLOptions("itemSources", "detail");
  setUseAltId(true);
  setParameterWidgetVisible(true);
  setNewVisible(true);
  setQueryOnStartEnabled(true);

  parameterWidget()->append(tr("Item"), "item_id", ParameterWidget::Item);
  parameterWidget()->append(tr("Vendor"), "vend_id", ParameterWidget::Vendor);

  list()->addColumn(tr("Vendor"),             -1,          Qt::AlignLeft,   true,  "vend_name"   );
  list()->addColumn(tr("Item Number"),        _itemColumn, Qt::AlignLeft,   true,  "item_number"   );
  list()->addColumn(tr("Description"),        -1,          Qt::AlignLeft,   true,  "item_descrip"   );
  list()->addColumn(tr("UOM"),                _uomColumn,  Qt::AlignCenter, true,  "uom_name" );
  list()->addColumn(tr("Vendor Item Number"), _itemColumn, Qt::AlignLeft,   true,  "itemsrc_vend_item_number"   );
  list()->addColumn(tr("Vendor UOM"),         _uomColumn,  Qt::AlignLeft,   true,  "itemsrc_vend_uom"   );
  list()->addColumn(tr("UOM Ratio"),          _qtyColumn,  Qt::AlignRight,  true,  "itemsrc_invvendoruomratio"  );
  list()->addColumn(tr("Manufacturer"),       _itemColumn, Qt::AlignLeft,   false, "itemsrc_manuf_name" );
  list()->addColumn(tr("Manuf. Item#"),       _itemColumn, Qt::AlignLeft,   false, "itemsrc_manuf_item_number" );

  if (_privileges->check("MaintainItemSources"))
    connect(list(), SIGNAL(itemSelected(int)), this, SLOT(sEdit()));
  else
  {
    newAction()->setEnabled(false);
    connect(list(), SIGNAL(itemSelected(int)), this, SLOT(sView()));
  }
}

void itemSources::sPopulateMenu(QMenu *menuThis, QTreeWidgetItem*, int)
{
  QAction *menuItem;

  menuItem = menuThis->addAction(tr("Edit..."), this, SLOT(sEdit()));
  menuItem->setEnabled(_privileges->check("MaintainItemSources"));

  menuItem = menuThis->addAction(tr("View..."), this, SLOT(sView()));
  menuItem->setEnabled(_privileges->check("MaintainItemSources") || _privileges->check("ViewItemSource"));

  menuItem = menuThis->addAction(tr("Copy..."), this, SLOT(sCopy()));
  menuItem->setEnabled(_privileges->check("MaintainItemSources"));

  menuItem = menuThis->addAction(tr("Delete..."), this, SLOT(sDelete()));
  menuItem->setEnabled(_privileges->check("MaintainItemSources"));

  menuThis->addSeparator();

  menuThis->addAction("View Buy Card...",  this, SLOT(sBuyCard()));
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
  params.append("itemsrc_id", list()->id());

  itemSource newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void itemSources::sView()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("itemsrc_id", list()->id());

  itemSource newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void itemSources::sCopy()
{
  ParameterList params;
  params.append("mode", "copy");
  params.append("itemsrc_id", list()->id());

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
  q.bindValue(":itemsrc_id", list()->id());
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
        q.bindValue(":itemsrc_id", list()->id());
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
  q.bindValue(":itemsrc_id", list()->id());
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
      q.bindValue(":itemsrc_id", list()->id());
      q.exec();

      sFillList();
    }
  }
}

void itemSources::sBuyCard()
{
  ParameterList params;
  params.append("itemsrc_id", list()->id());

  buyCard *newdlg = new buyCard();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

