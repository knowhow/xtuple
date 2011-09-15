/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "countTagList.h"

#include <QSqlError>
#include <QVariant>

countTagList::countTagList(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_close, SIGNAL(clicked()), this, SLOT(sClose()));
  connect(_select, SIGNAL(clicked()), this, SLOT(sSelect()));
  connect(_searchFor, SIGNAL(textChanged(const QString&)), this, SLOT(sSearch(const QString&)));
  connect(_warehouse, SIGNAL(updated()), this, SLOT(sFillList()));

  _cnttag->addColumn(tr("Tag #"),       _orderColumn, Qt::AlignLeft,  true, "invcnt_tagnumber");
  _cnttag->addColumn(tr("Item Number"), _itemColumn,  Qt::AlignLeft,  true, "item_number");
  _cnttag->addColumn(tr("Description"), -1,           Qt::AlignLeft,  true, "item_descrip1");
  _cnttag->addColumn(tr("Site"),        _whsColumn,   Qt::AlignCenter,true, "warehous_code");
}

countTagList::~countTagList()
{
  // no need to delete child widgets, Qt does it all for us
}

void countTagList::languageChange()
{
  retranslateUi(this);
}

enum SetResponse countTagList::set(ParameterList &pParams)
{
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("cnttag_id", &valid);
  if (valid)
    _cnttagid = param.toInt();

  param = pParams.value("tagType", &valid);
  if (valid)
    _type = param.toInt();

  sFillList();

  return NoError;
}

void countTagList::sFillList()
{
  QString sql( "SELECT invcnt_id, invcnt_tagnumber, item_number, item_descrip1, warehous_code "
               "FROM invcnt, itemsite, item, warehous "
               "WHERE ((invcnt_itemsite_id=itemsite_id)"
               " AND (itemsite_item_id=item_id)"
               " AND (itemsite_warehous_id=warehous_id)" );

  if (_warehouse->isSelected())
    sql += " AND (warehous_id=:warehous_id)";

  if (_type & cPostedCounts)
    sql += " AND (invcnt_posted)";
  else if (_type & cUnpostedCounts)
    sql += " AND (NOT invcnt_posted)";

  sql += ") "
         "ORDER BY invcnt_tagnumber";

  q.prepare(sql);
  _warehouse->bindValue(q);
  q.exec();

  _cnttag->populate(q, _cnttagid);
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void countTagList::sClose()
{
  done(_cnttagid);
}

void countTagList::sSelect()
{
  done(_cnttag->id());
}

void countTagList::sSearch(const QString &pTarget)
{
  _cnttag->clearSelection();
  int i;
  for (i = 0; i < _cnttag->topLevelItemCount(); i++)
  {
    if (_cnttag->topLevelItem(i)->text(1).contains(pTarget, Qt::CaseInsensitive))
    break;
  }

  if (i < _cnttag->topLevelItemCount())
  {
    _cnttag->setCurrentItem(_cnttag->topLevelItem(i));
    _cnttag->scrollToItem(_cnttag->topLevelItem(i));
  }
}
