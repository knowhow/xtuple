/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef TODOLISTCALENDAR_H
#define TODOLISTCALENDAR_H

#include "guiclient.h"
#include "xwidget.h"
#include <parameter.h>

#include "ui_todoListCalendar.h"

class CalendarGraphicsItem;

class todoListCalendar : public XWidget, public Ui::todoListCalendar
{
  Q_OBJECT

  public:
    todoListCalendar(QWidget* parent = 0, const char * = 0, Qt::WindowFlags f = 0);

    virtual SetResponse	set(const ParameterList&);
    void setParams(ParameterList &);

  public slots:
    void languageChange();
    void sFillList();
    void sFillList(const QDate&);
    void sNew();
    void sEdit();
    void sView();
    void sDelete();
    void sEditCustomer();
    void sViewCustomer();

  protected:
    void resizeEvent(QResizeEvent*);
    void showEvent(QShowEvent*);

    QDate _lastDate;
    int   _myUsrId;
    CalendarGraphicsItem * calendar;

  protected slots:
    void handlePrivs();
    void sPopulateMenu(QMenu*);
};

#endif // TODOLIST_H
