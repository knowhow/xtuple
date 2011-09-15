/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef calendarTools_h
#define calendarTools_h

#include <QDateTime>

#include <parameter.h>

#include "widgets.h"
#include "xcombobox.h"
#include "xtreewidget.h"

class QString;

class XTUPLEWIDGETS_EXPORT Numeric
{
  public:
    Numeric()
    {
      value = 0.0;
    }
  
    inline double operator+=(double pValue) { value += pValue; return value; }
    inline double toDouble()                { return value;                  }

    double value;
};

class XTUPLEWIDGETS_EXPORT DatePair
{
  public:
    DatePair()
    {
    }

    DatePair(QDate pStartDate, QDate pEndDate)
    {
      startDate = pStartDate;
      endDate = pEndDate;
    }

    QDate startDate;
    QDate endDate;
};


class XTUPLEWIDGETS_EXPORT CalendarComboBox : public XComboBox
{
  Q_OBJECT

  public:
    CalendarComboBox(QWidget *, const char * = 0);

    Q_INVOKABLE void load(ParameterList &);

  signals:
    void newCalendarId(int);
    void select(ParameterList &);
};


class PeriodListViewItem;

class XTUPLEWIDGETS_EXPORT PeriodsListView : public XTreeWidget
{
  Q_OBJECT

  public:
    PeriodsListView(QWidget *, const char * = 0);

    Q_INVOKABLE QList<QVariant>    periodList();
    Q_INVOKABLE QString            periodString();
    Q_INVOKABLE PeriodListViewItem *getSelected(int);
    Q_INVOKABLE void               getSelected(ParameterList &);
    Q_INVOKABLE bool               isPeriodSelected();

  public slots:	
    void populate(int);
    void load(ParameterList &);

  private:
    int _calheadid;
};


void setupPeriodListViewItem(QScriptEngine *engine);

class XTUPLEWIDGETS_EXPORT PeriodListViewItem : public XTreeWidgetItem
{
  Q_OBJECT

  public:
    PeriodListViewItem( PeriodsListView *, XTreeWidgetItem *, int,
                        QDate, QDate,
                        QString, QString );

    Q_INVOKABLE inline QDate startDate() { return _startDate; }
    Q_INVOKABLE inline QDate endDate()   { return _endDate;   }

  private:
    QDate _startDate;
    QDate _endDate;
};
Q_DECLARE_METATYPE(PeriodListViewItem*)

#endif
