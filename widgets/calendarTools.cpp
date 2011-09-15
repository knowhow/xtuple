/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include <xsqlquery.h>

#include <QtScript>

#include "calendarTools.h"

CalendarComboBox::CalendarComboBox(QWidget *pParent, const char *pName) :
  XComboBox(pParent, pName)
{
  setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  setAllowNull(TRUE);

  if(_x_metrics)
  {
    XSqlQuery q;
    q.exec( "SELECT calhead_id, calhead_name, calhead_name "
            "FROM calhead "
            "ORDER BY calhead_name;" );
    populate(q);
  }

  connect(this, SIGNAL(newID(int)), this, SIGNAL(newCalendarId(int)));
}

void CalendarComboBox::load(ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("calitem_id(0)", &valid);
  if (valid)
  {
    XSqlQuery q;
    q.prepare( "SELECT acalitem_calhead_id AS calheadid "
               "FROM acalitem "
               "WHERE (acalitem_id=:calitem_id) "
               "UNION SELECT rcalitem_calhead_id AS calheadid "
               "FROM rcalitem "
               "WHERE (rcalitem_id=:calitem_id);" );
    q.bindValue(":calitem_id", param.toInt());
    q.exec();
    if (q.first())
      setId(q.value("calheadid").toInt());

    emit select(pParams);
  }
}


PeriodsListView::PeriodsListView(QWidget *pParent, const char *pName) :
  XTreeWidget(pParent)
{
  setObjectName(pName);
  _calheadid = -1;

  setSelectionMode(QAbstractItemView::ExtendedSelection);
    
  // If we're in a Designer, don't populate
  if (_x_metrics)
  {
    QObject *ancestor = this;
    bool designMode;
    for ( ; ancestor; ancestor = ancestor->parent())
    {
      designMode = ancestor->inherits("xTupleDesigner");
      if (designMode)
        return;
    }
  
    addColumn(tr("Name"),    _itemColumn, Qt::AlignLeft,   true, "periodname");
    addColumn(tr("Selected Periods"), -1, Qt::AlignCenter, true, "periodrange");
  }
}

void PeriodsListView::populate(int pCalheadid)
{
  XSqlQuery caltype( QString( "SELECT calhead_type "
                              "FROM calhead "
                              "WHERE (calhead_id=%1);" )
                     .arg(pCalheadid) );
  if (caltype.first())
  {
    QString sql;

    _calheadid = pCalheadid;
    clear();

    if (caltype.value("calhead_type").toString() == "A")
      sql = QString( "SELECT acalitem_id, periodstart, periodend, acalitem_name AS periodname,"
                     "       (formatDate(periodstart) || ' - ' || formatDate(periodend)) AS periodrange "
                     "FROM ( SELECT acalitem_id, acalitem_name,"
                     "              findPeriodStart(acalitem_id) AS periodstart,"
                     "              findPeriodEnd(acalitem_id) AS periodend "
                     "       FROM acalitem "
                     "       WHERE (acalitem_calhead_id=%1) ) AS data "
                     "ORDER BY periodstart;" )
              .arg(pCalheadid);

    else if (caltype.value("calhead_type").toString() == "R")
      sql = QString( "SELECT rcalitem_id, periodstart, periodend, rcalitem_name AS periodname,"
                     "       (formatDate(periodstart) || ' - ' || formatDate(periodend)) AS periodrange "
                     "FROM ( SELECT rcalitem_id, rcalitem_name,"
                     "              findPeriodStart(rcalitem_id) AS periodstart,"
                     "              findPeriodEnd(rcalitem_id) AS periodend "
                     "       FROM rcalitem "
                     "       WHERE (rcalitem_calhead_id=%1) ) AS data "
                     "ORDER BY periodstart;" )
              .arg(pCalheadid);

    XSqlQuery query(sql);
    XTreeWidgetItem *last = 0;
    QAbstractItemView::SelectionMode tmp = selectionMode();
    setSelectionMode(QAbstractItemView::MultiSelection);
    while (query.next())
    {
      last = new PeriodListViewItem(this, last, query.value(0).toInt(),
                                    query.value(1).toDate(), query.value(2).toDate(),
                                    query.value(3).toString(), query.value(4).toString() );
      setCurrentItem(last);
    }
    setSelectionMode(tmp);
  }
  else
    _calheadid = -1;
}

void PeriodsListView::getSelected(ParameterList &pParams)
{
  QList<XTreeWidgetItem *>list = selectedItems();
  int           counter = 0;

  for (int i = 0; i < list.size(); i++)
    pParams.append(QString("calitem_id(%1)").arg(counter++), ((XTreeWidgetItem*)list[i])->id());
}

bool PeriodsListView::isPeriodSelected()
{
  QList<XTreeWidgetItem *>list = selectedItems();

  return (list.size() > 0);
}

QString PeriodsListView::periodString()
{
  QString     returnString;
  QList<XTreeWidgetItem *>list = selectedItems();

  for (int i = 0; i < list.size(); i++)
  {
    if (returnString.length())
      returnString += ",";

    returnString += QString("%1").arg(((XTreeWidgetItem*)list[i])->id());
  }

  return returnString;
}

QList<QVariant> PeriodsListView::periodList()
{
  QList<QVariant>     returnList;
  QList<XTreeWidgetItem *>list = selectedItems();

  for (int i = 0; i < list.size(); i++)
    returnList.append(((XTreeWidgetItem*)(list[i]))->id());

  return returnList;
}

PeriodListViewItem *PeriodsListView::getSelected(int pIndex)
{
  QList<XTreeWidgetItem *>list = selectedItems();

  int i;
  for (i = 0; i < list.size(); i++)
    if (--pIndex == 0)
      break;

  if (pIndex == 0)
    return (PeriodListViewItem *)list[i];
  else
    return NULL;
}

void PeriodsListView::load(ParameterList &pParams)
{
  clearSelection();
  QAbstractItemView::SelectionMode oldSelMode = selectionMode();
  setSelectionMode(QAbstractItemView::MultiSelection);

  if (topLevelItemCount() > 0)
  {
    QVariant param;
    bool     valid = TRUE;
    for (unsigned int counter = 0; valid; counter++)
    {
      param = pParams.value(QString("calitem_id(%1)").arg(counter), &valid);
      if (valid)
      {
        for (int i = 0; i < topLevelItemCount(); i++)
          if (((XTreeWidgetItem*)topLevelItem(i))->id() == param.toInt())
            setCurrentItem(topLevelItem(i));
      }
    }
  }

  setSelectionMode(oldSelMode);
}

QScriptValue PeriodListViewItemtoScriptValue(QScriptEngine *engine, PeriodListViewItem* const &item)
{
  return engine->newQObject(item);
}

void PeriodListViewItemfromScriptValue(const QScriptValue &obj, PeriodListViewItem* &item)
{
  item = qobject_cast<PeriodListViewItem*>(obj.toQObject());
}

void setupPeriodListViewItem(QScriptEngine *engine)
{
  qScriptRegisterMetaType(engine, PeriodListViewItemtoScriptValue, PeriodListViewItemfromScriptValue);
}

PeriodListViewItem::PeriodListViewItem( PeriodsListView *parent, XTreeWidgetItem *itm, int pId,
                        QDate pStartDate, QDate pEndDate,
                        QString s0, QString s1 ) :
XTreeWidgetItem(parent, itm, pId, QVariant(s0), QVariant(s1))
{
  _startDate = pStartDate;
  _endDate = pEndDate;
}
