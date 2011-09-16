/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspTimePhasedOpenAPItems.h"

#include <QAction>
#include <QMenu>
#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include <metasql.h>
#include "mqlutil.h"

#include <datecluster.h>

#include "dspAPOpenItemsByVendor.h"
#include "submitReport.h"

dspTimePhasedOpenAPItems::dspTimePhasedOpenAPItems(QWidget* parent, const char*, Qt::WFlags fl)
  : display(parent, "dspTimePhasedOpenAPItems", fl)
{
  setupUi(optionsWidget());
  setWindowTitle(tr("Payables Aging"));
  setReportName("APAging");

  connect(_custom, SIGNAL(toggled(bool)), this, SLOT(sToggleCustom()));

  _asOf->setDate(omfgThis->dbDate(), true);
  sToggleCustom();

  if(_preferences->value("APAgingDefaultDate") == "doc")
    _useDocDate->setChecked(true);
  else
    _useDistDate->setChecked(true);
}

dspTimePhasedOpenAPItems::~dspTimePhasedOpenAPItems()
{
  // no need to delete child widgets, Qt does it all for us
  QString str("dist");
  if(_useDocDate->isChecked())
    str = "doc";
  _preferences->set("APAgingDefaultDate", str);
}

bool dspTimePhasedOpenAPItems::setParams(ParameterList &params)
{
  if ((_custom->isChecked() && ! _periods->isPeriodSelected()) ||
      (!_custom->isChecked() && ! _asOf->isValid()))
  {
    QMessageBox::critical(this, tr("Incomplete criteria"),
                          tr("<p>The criteria you specified are not complete. "
                             "Please make sure all fields are correctly filled "
                             "out before running the report." ) );
    return false;
  }

  _vendorGroup->appendValue(params);

  if(_custom->isChecked())
  {
    QList<XTreeWidgetItem*> selected = _periods->selectedItems();
    QList<QVariant> periodList;
    for (int i = 0; i < selected.size(); i++)
      periodList.append(((XTreeWidgetItem*)selected[i])->id());
    params.append("period_id_list", periodList);

    params.append("report_name", "TimePhasedOpenAPItems");
  }
  else
  {
    params.append("relDate", _asOf->date());
    params.append("report_name", "APAging");
  }

  // have both in case we add a third option
  params.append("useDocDate",  QVariant(_useDocDate->isChecked()));
  params.append("useDistDate", QVariant(_useDistDate->isChecked()));

  return true;
}

void dspTimePhasedOpenAPItems::sViewOpenItems()
{
  ParameterList params;
  params.append("vend_id", list()->id());
  if (_custom->isChecked())
  {
    QDate checkDate = _columnDates[_column - 2].startDate;
    if (checkDate.isValid())
    {
      params.append("startDate", _columnDates[_column - 2].startDate);
      params.append("endDate", _columnDates[_column - 2].endDate);
    }
  }
  else
  {
    QDate asOfDate;
    asOfDate = _asOf->date();
    if (_column == 3)
    {
      params.append("startDate", asOfDate );
    }
    else if (_column == 4)
    {
      params.append("startDate", asOfDate.addDays(-30) );
      params.append("endDate", asOfDate);
    }
    else if (_column == 5)
    {
      params.append("startDate",asOfDate.addDays(-60) );
      params.append("endDate", asOfDate.addDays(-31));
    }
    else if (_column == 6)
    {
      params.append("startDate",asOfDate.addDays(-90) );
      params.append("endDate", asOfDate.addDays(-61));
    }
    else if (_column == 7)
      params.append("endDate",asOfDate.addDays(-91) );
  }
  params.append("run");
  params.append("asofDate",_asOf->date());

  dspAPOpenItemsByVendor *newdlg = new dspAPOpenItemsByVendor();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspTimePhasedOpenAPItems::sPopulateMenu(QMenu *menuThis, QTreeWidgetItem *, int pColumn)
{
  _column = pColumn;

  if ((_column > 1) && (list()->id() > 0))
    menuThis->addAction(tr("View Open Items..."), this, SLOT(sViewOpenItems()));
}

void dspTimePhasedOpenAPItems::sFillList()
{
  if (_custom->isChecked())
    sFillCustom();
  else
    sFillStd();
}

void dspTimePhasedOpenAPItems::sFillCustom()
{
  if (!_periods->isPeriodSelected())
  {
    if (isVisible())
      QMessageBox::warning( this, tr("Select Calendar Periods"),
                            tr("Please select one or more Calendar Periods") );
    return;
  }

  _columnDates.clear();
  list()->setColumnCount(2);

  QString sql("SELECT vend_id, vend_number, vend_name");
  QStringList linetotal;

  int columns = 1;
  QList<XTreeWidgetItem*> selected = _periods->selectedItems();
  for (int i = 0; i < selected.size(); i++)
  {
    PeriodListViewItem *cursor = (PeriodListViewItem*)selected[i];
    QString bucketname = QString("bucket%1").arg(columns++);
    sql += QString(", openAPItemsValue(vend_id, %2) AS %1,"
                   " 'curr' AS %3_xtnumericrole, 0 AS %4_xttotalrole")
          .arg(bucketname)
          .arg(cursor->id())
          .arg(bucketname)
          .arg(bucketname);

    list()->addColumn(formatDate(cursor->startDate()), _bigMoneyColumn, Qt::AlignRight, true, bucketname);
    _columnDates.append(DatePair(cursor->startDate(), cursor->endDate()));
    linetotal << QString("openAPItemsValue(vend_id, %1)").arg(cursor->id());
  }

  list()->addColumn(tr("Total"), _bigMoneyColumn, Qt::AlignRight, true, "linetotal");
  _columnDates.append(DatePair());
  sql += ", " + linetotal.join("+") + " AS linetotal,"
         " 'curr' AS linetotal_xtnumericrole,"
         " 0 AS linetotal_xttotalrole,"
         " (" + linetotal.join("+") + ") = 0.0 AS xthiddenrole "
         "FROM vend "
         "<? if exists(\"vend_id\") ?>"
         "WHERE (vend_id=<? value (\"vend_id\") ?>)"
         "<? elseif exists(\"vendtype_id\") ?>"
         "WHERE (vend_vendtype_id=<? value (\"vendtype_id\") ?>)"
         "<? elseif exists(\"vendtype_code\") ?>"
         "WHERE (vend_vendtype_id IN (SELECT vendtype_id FROM vendtype WHERE (vendtype_code ~ <? value (\"vendtype_pattern\") ?>))) "
         "<? endif ?>"
         "ORDER BY vend_number;";

  MetaSQLQuery mql(sql);
  ParameterList params;
  if (! setParams(params))
    return;

  q = mql.toQuery(params);
  list()->populate(q);
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void dspTimePhasedOpenAPItems::sFillStd()
{
  MetaSQLQuery mql = mqlLoad("apAging", "detail");
  ParameterList params;
  if (! setParams(params))
    return;
  q = mql.toQuery(params);
  list()->populate(q);
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void dspTimePhasedOpenAPItems::sToggleCustom()
{
  list()->clear();
  if (_custom->isChecked())
  {
    setReportName("TimePhasedOpenAPItems");
    _calendarLit->setHidden(FALSE);
    _calendar->setHidden(FALSE);
    _periods->setHidden(FALSE);
    _asOf->setDate(omfgThis->dbDate(), true);
    _asOf->setEnabled(FALSE);
    _useGroup->setHidden(TRUE);

    list()->setColumnCount(0);
    list()->addColumn(tr("Vend. #"), _orderColumn, Qt::AlignLeft, true, "vend_number");
    list()->addColumn(tr("Vendor"),  180,          Qt::AlignLeft, true, "vend_name");
  }
  else
  {
    setReportName("APAging");
    _calendarLit->setHidden(TRUE);
    _calendar->setHidden(TRUE);
    _periods->setHidden(TRUE);
    _asOf->setEnabled(TRUE);
    _useGroup->setHidden(FALSE);

    list()->addColumn(tr("Total Open"), _bigMoneyColumn, Qt::AlignRight, true, "total_val");
    list()->addColumn(tr("0+ Days"),    _bigMoneyColumn, Qt::AlignRight, true, "cur_val");
    list()->addColumn(tr("0-30 Days"),  _bigMoneyColumn, Qt::AlignRight, true, "thirty_val");
    list()->addColumn(tr("31-60 Days"), _bigMoneyColumn, Qt::AlignRight, true, "sixty_val");
    list()->addColumn(tr("61-90 Days"), _bigMoneyColumn, Qt::AlignRight, true, "ninety_val");
    list()->addColumn(tr("90+ Days"),   _bigMoneyColumn, Qt::AlignRight, true, "plus_val");
    list()->setColumnCount(0);
    list()->addColumn(tr("Vend. #"), _orderColumn, Qt::AlignLeft, true, "apaging_vend_number");
    list()->addColumn(tr("Vendor"),  -1,          Qt::AlignLeft, true, "apaging_vend_name");
    list()->addColumn(tr("Total Open"), _bigMoneyColumn, Qt::AlignRight, true, "apaging_total_val_sum");
    list()->addColumn(tr("0+ Days"),    _bigMoneyColumn, Qt::AlignRight, true, "apaging_cur_val_sum");
    list()->addColumn(tr("0-30 Days"),  _bigMoneyColumn, Qt::AlignRight, true, "apaging_thirty_val_sum");
    list()->addColumn(tr("31-60 Days"), _bigMoneyColumn, Qt::AlignRight, true, "apaging_sixty_val_sum");
    list()->addColumn(tr("61-90 Days"), _bigMoneyColumn, Qt::AlignRight, true, "apaging_ninety_val_sum");
    list()->addColumn(tr("90+ Days"),   _bigMoneyColumn, Qt::AlignRight, true, "apaging_plus_val_sum");
  }
}
