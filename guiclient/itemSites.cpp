/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "itemSites.h"

#include <QKeyEvent>
#include <QAction>
#include <QMenu>
#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include "itemSite.h"
#include "storedProcErrorLookup.h"

itemSites::itemSites(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  connect(_new, SIGNAL(clicked()), this, SLOT(sNew()));
  connect(_edit, SIGNAL(clicked()), this, SLOT(sEdit()));
  connect(_itemSite, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*)));
  connect(_showInactive, SIGNAL(toggled(bool)), this, SLOT(sFillList()));
  connect(_searchFor, SIGNAL(textChanged(const QString&)), this, SLOT(sSearch(const QString&)));
  connect(_view, SIGNAL(clicked()), this, SLOT(sView()));
  connect(_warehouse, SIGNAL(updated()), this, SLOT(sFillList()));
  connect(_copy, SIGNAL(clicked()), this, SLOT(sCopy()));
  connect(_delete, SIGNAL(clicked()), this, SLOT(sDelete()));

  _itemSite->addColumn(tr("Item Number"),   _itemColumn, Qt::AlignLeft,  true, "item_number"   );
  _itemSite->addColumn(tr("Active"),        _dateColumn, Qt::AlignCenter,true, "itemsite_active" );
  _itemSite->addColumn(tr("Description"),   -1,          Qt::AlignLeft,  true, "item_descrip"   );
  _itemSite->addColumn(tr("Site"),          _whsColumn,  Qt::AlignCenter,true, "warehous_code" );
  _itemSite->addColumn(tr("Cntrl. Method"), _itemColumn, Qt::AlignCenter,true, "itemsite_controlmethod" );
  _itemSite->setDragString("itemsiteid=");

  _searchFor->setAcceptDrops(FALSE);
  
  connect(omfgThis, SIGNAL(itemsitesUpdated()), SLOT(sFillList()));

  if (_privileges->check("MaintainItemSites"))
  {
    connect(_itemSite, SIGNAL(valid(bool)), _edit, SLOT(setEnabled(bool)));
    connect(_itemSite, SIGNAL(valid(bool)), _copy, SLOT(setEnabled(bool)));
    connect(_itemSite, SIGNAL(itemSelected(int)), _edit, SLOT(animateClick()));
  }
  else
  {
    _new->setEnabled(FALSE);
    connect(_itemSite, SIGNAL(itemSelected(int)), _view, SLOT(animateClick()));
  }

  if (_privileges->check("DeleteItemSites"))
    connect(_itemSite, SIGNAL(valid(bool)), _delete, SLOT(setEnabled(bool)));

  _copy->setVisible(_metrics->boolean("MultiWhs"));

  sFillList();

  _searchFor->setFocus();
}

itemSites::~itemSites()
{
    // no need to delete child widgets, Qt does it all for us
}

void itemSites::languageChange()
{
    retranslateUi(this);
}

void itemSites::sNew()
{
  ParameterList params;
  params.append("mode", "new");

  itemSite newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void itemSites::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("itemsite_id", _itemSite->id());

  itemSite newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void itemSites::sView()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("itemsite_id", _itemSite->id());

  itemSite newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void itemSites::sCopy()
{
  q.prepare("SELECT copyItemSite(:olditemsiteid, NULL) AS result;");
  q.bindValue(":olditemsiteid", _itemSite->id());
  q.exec();
  if (q.first())
  {
    int result = q.value("result").toInt();
    if (result < 0)
    {
      systemError(this, storedProcErrorLookup("copyItemSite", result), __FILE__, __LINE__);
      return;
    }
    ParameterList params;
    params.append("mode", "edit");
    params.append("itemsite_id", result);

    itemSite newdlg(this, "", TRUE);
    newdlg.set(params);
    if (newdlg.exec() != XDialog::Accepted)
    {
      q.prepare("SELECT deleteItemSite(:itemsite_id) AS result;");
      q.bindValue(":itemsite_id", result);
      q.exec();
      if (q.first())
      {
	int result = q.value("result").toInt();
	if (result < 0)
	{
	  systemError(this, storedProcErrorLookup("deleteItemSite", result), __FILE__, __LINE__);
	  return;
	}
      }
      else if (q.lastError().type() != QSqlError::NoError)
      {
	systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
	return;
      }
    }
  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void itemSites::sDelete()
{
  q.prepare("SELECT deleteItemSite(:itemsite_id) AS result;");
  q.bindValue(":itemsite_id", _itemSite->id());
  q.exec();
  if (q.first())
  {
    int result = q.value("result").toInt();
    if (result < 0)
    {
      systemError(this, storedProcErrorLookup("deleteItemSite", result), __FILE__, __LINE__);
      return;
    }
    sFillList();
  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void itemSites::sPopulateMenu(QMenu *pMenu)
{
  QAction *menuItem;

  menuItem = pMenu->addAction(tr("View Item Site"), this, SLOT(sView()));

  menuItem = pMenu->addAction(tr("Edit Item Site"), this, SLOT(sEdit()));
  menuItem->setEnabled(_privileges->check("MaintainItemSites"));
}

void itemSites::sFillList()
{
  QString sql( "SELECT itemsite_id, item_number, itemsite_active,"
               "       (item_descrip1 || ' ' || item_descrip2) AS item_descrip, warehous_code,"
               "       CASE WHEN itemsite_controlmethod='R' THEN :regular"
               "            WHEN itemsite_controlmethod='N' THEN :none"
               "            WHEN itemsite_controlmethod='L' THEN :lotNumber"
               "            WHEN itemsite_controlmethod='S' THEN :serialNumber"
               "       END AS itemsite_controlmethod "
               "FROM itemsite, item, warehous "
               "WHERE ( (itemsite_item_id=item_id)"
               " AND (itemsite_warehous_id=warehous_id)" );

  if (_warehouse->isSelected())
    sql += " AND (warehous_id=:warehous_id)";

  if (!_showInactive->isChecked())
    sql += " AND (itemsite_active)";

  sql += ") "
         "ORDER BY item_number, warehous_code;";

  q.prepare(sql);
  _warehouse->bindValue(q);
  q.bindValue(":regular", tr("Regular"));
  q.bindValue(":none", tr("None"));
  q.bindValue(":lotNumber", tr("Lot #"));
  q.bindValue(":serialNumber", tr("Serial #"));
  q.exec();
  _itemSite->populate(q);
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void itemSites::sSearch(const QString &pTarget)
{
  QList<XTreeWidgetItem*> items = _itemSite->findItems(pTarget, Qt::MatchStartsWith, 0);

  if (items.count() > 0)
  {
    _itemSite->clearSelection();
    _itemSite->setItemSelected(items.at(0), true);
    _itemSite->scrollToItem(items.at(0));
  }
}

void itemSites::keyPressEvent( QKeyEvent * e )
{
#ifdef Q_WS_MAC
  if(e->key() == Qt::Key_N && (e->modifiers() & Qt::ControlModifier))
  {
    _new->animateClick();
    e->accept();
  }
  else if(e->key() == Qt::Key_E && (e->modifiers() & Qt::ControlModifier))
  {
    _edit->animateClick();
    e->accept();
  }
  if(e->isAccepted())
    return;
#endif
  e->ignore();
}
