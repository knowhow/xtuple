/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */


#ifndef dcalendarpopup_h
#define dcalendarpopup_h

#include <QCalendarWidget>
#include <QDate>
#include <QWidget>

class DCalendarPopup : public QWidget
{
  Q_OBJECT

  public:
    DCalendarPopup(const QDate&, QWidget* = 0);

  public slots:
    void dateSelected(const QDate &);

  signals:
    void newDate(const QDate &);

  private:
    QCalendarWidget *_cal;
};

#endif
