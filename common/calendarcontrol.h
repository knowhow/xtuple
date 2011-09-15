/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef __CALENDARCONTROL_H__
#define __CALENDARCONTROL_H__

#include <QObject>
#include <QDate>

class CalendarControl : public QObject
{
  Q_OBJECT

  public:
    CalendarControl(QObject * parent = 0);
    ~CalendarControl();

    virtual QString contents(const QDate &) = 0;
    virtual void setSelectedDay(const QDate & day);

  signals:
    void selectedDayChanged(const QDate &);
};

#endif
