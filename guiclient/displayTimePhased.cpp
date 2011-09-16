/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "displayTimePhased.h"
#include "ui_displayTimePhased.h"

#include <QSqlError>
#include <QMessageBox>

#include <parameter.h>


class displayTimePhasedPrivate : public Ui::displayTimePhased
{
public:
  displayTimePhasedPrivate(::displayTimePhased * parent) : _parent(parent)
  {
    setupUi(_parent->display::optionsWidget());
    _baseColumns = -1;
  }

  int _baseColumns;

private:
  ::displayTimePhased * _parent;
};

displayTimePhased::displayTimePhased(QWidget* parent, const char* name, Qt::WindowFlags flags)
  : display(parent, name, flags)
{
  _data = new displayTimePhasedPrivate(this);

  connect(_data->_calendar, SIGNAL(newCalendarId(int)), _data->_periods, SLOT(populate(int)));
  connect(_data->_calendar, SIGNAL(select(ParameterList&)), _data->_periods, SLOT(load(ParameterList&)));

  _column = 0;
}

displayTimePhased::~displayTimePhased()
{
  delete _data;
  _data = 0;
}

void displayTimePhased::languageChange()
{
  display::languageChange();
  _data->retranslateUi(this);
}

QWidget * displayTimePhased::optionsWidget()
{
  return _data->_optionsFrame2;
}

bool displayTimePhased::setParams(ParameterList &params)
{
  if (_data->_calendar->id() == -1)
  {
    QMessageBox::critical(this, tr("Calendar Required"),
                          tr("You must select a Calendar"));
    _data->_calendar->setFocus();
    return false;
  }

  if (_data->_periods->selectedItems().length() == 0)
  {
    QMessageBox::critical(this, tr("Periods Required"),
                          tr("You must select at least one Calendar Period"));
    _data->_calendar->setFocus();
    return false;
  }

  params.append("period_id_list", _data->_periods->periodList());
  params.append("calendar_id", _data->_calendar->id());

  return setParamsTP(params);
}

void displayTimePhased::setBaseColumns(int columns)
{
  _data->_baseColumns = columns;
}

void displayTimePhased::sFillList()
{
  ParameterList params;
  if(!setParams(params))
    return;

  if(_data->_baseColumns == -1)
    _data->_baseColumns = list()->columnCount();

  list()->clear();
  list()->setColumnCount(_data->_baseColumns);

  _columnDates.clear();
  _column = 0;

  QList<XTreeWidgetItem*> selected = _data->_periods->selectedItems();
  for (int i = 0; i < selected.size(); i++)
  {
    PeriodListViewItem *cursor = (PeriodListViewItem*)selected[i];
    QString bucketname = QString("bucket_%1").arg(cursor->id());
    list()->addColumn(formatDate(cursor->startDate()), _qtyColumn, Qt::AlignRight, true, bucketname);
    _columnDates.append(DatePair(cursor->startDate(), cursor->endDate()));
  }

  display::sFillList();
}

