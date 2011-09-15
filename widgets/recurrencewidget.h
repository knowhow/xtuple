/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef RECURRENCEWIDGET_H
#define RECURRENCEWIDGET_H

#include "widgets.h"
#include "ui_recurrencewidget.h"

class QScriptEngine;

void setupRecurrenceWidget(QScriptEngine *engine);

class RecurrenceWidget : public QWidget, public Ui::RecurrenceWidget
{
  Q_OBJECT

  Q_ENUMS(RecurrencePeriod)
  Q_ENUMS(RecurrenceChangePolicy)

  Q_PROPERTY(bool maxVisible       READ maxVisible       WRITE setMaxVisible DESIGNABLE false)
  Q_PROPERTY(bool endTimeVisible   READ endTimeVisible   WRITE setEndTimeVisible)
  Q_PROPERTY(bool endDateVisible   READ endDateVisible   WRITE setEndDateVisible)
  Q_PROPERTY(RecurrencePeriod minPeriod READ minPeriod   WRITE setMinPeriod)
  Q_PROPERTY(bool startDateVisible READ startDateVisible WRITE setStartDateVisible)
  Q_PROPERTY(bool startTimeVisible READ startTimeVisible WRITE setStartTimeVisible)

  public:
    // Never must = XComboBox::id() when ! XComboBox::isValid()
    enum RecurrencePeriod
    { Never = -1, Minutely, Hourly, Daily, Weekly, Monthly, Yearly, Custom };

    enum RecurrenceChangePolicy
    { NoPolicy = -1, IgnoreFuture, ChangeFuture };
                          
    RecurrenceWidget(QWidget* parent = 0, const char* name = 0);
    ~RecurrenceWidget();

    Q_INVOKABLE virtual QDate            endDate()          const;
    Q_INVOKABLE virtual QDateTime        endDateTime()      const;
    Q_INVOKABLE virtual QTime            endTime()          const;
    Q_INVOKABLE virtual bool             endDateVisible()   const;
    Q_INVOKABLE virtual bool             endTimeVisible()   const;
    Q_INVOKABLE virtual int              frequency()        const;
    Q_INVOKABLE virtual RecurrenceChangePolicy getChangePolicy();
    Q_INVOKABLE virtual bool             isRecurring()      const;
    Q_INVOKABLE virtual int              max()              const;
    /*useprop*/ virtual bool             maxVisible()       const;
    Q_INVOKABLE virtual RecurrencePeriod minPeriod()        const;
    Q_INVOKABLE virtual bool             modified()         const;
    Q_INVOKABLE virtual int              parentId()         const;
    Q_INVOKABLE virtual QString          parentType()       const;
    Q_INVOKABLE virtual RecurrencePeriod period()           const;
    Q_INVOKABLE virtual QString          periodCode()       const;
    Q_INVOKABLE virtual QDate            startDate()        const;
    Q_INVOKABLE virtual QDateTime        startDateTime()    const;
    /*useprop*/ virtual bool             startDateVisible() const;
    Q_INVOKABLE virtual QTime            startTime()        const;
    Q_INVOKABLE virtual bool             startTimeVisible() const;
    Q_INVOKABLE virtual RecurrencePeriod stringToPeriod(QString p) const;

  public slots:
    virtual void clear();
    virtual bool save(bool intxn, RecurrenceChangePolicy cp, QString *msg = 0);
    virtual void set(bool recurring = false, int frequency = 1, QString period = QString("W"), QDate startDate = QDate::currentDate(), QDate endDate = QDate(), int max = 1);
    virtual void set(bool recurring, int frequency, QString period, QDateTime startDateTime, QDateTime endDateTime, int max);
    virtual void setEndDate(QDate p);
    virtual void setEndDateTime(QDateTime p);
    virtual void setEndDateVisible(bool p);
    virtual void setEndTime(QTime p);
    virtual void setEndTimeVisible(bool p);
    virtual void setFrequency(int p);
    virtual void setMax(int p);
    virtual void setMaxVisible(bool p);
    virtual void setMinPeriod(RecurrencePeriod minPeriod);
    virtual bool setParent(int pid, QString ptype);
    virtual void setPeriod(RecurrencePeriod p);
    virtual void setPeriod(QString p);
    virtual void setRecurring(bool p);
    virtual void setStartDate(QDate p);
    virtual void setStartDateTime(QDateTime p);
    virtual void setStartDateVisible(bool p);
    virtual void setStartTime(QTime p);
    virtual void setStartTimeVisible(bool p);

  protected slots:
    virtual void languageChange();

  protected:
   QDateTime        _eot;
   int              _id;
   int              _parentId;
   QString          _parentType;
   QDateTime        _prevEndDateTime;
   int              _prevFrequency;
   int              _prevMax;
   int              _prevParentId;
   QString          _prevParentType;
   RecurrencePeriod _prevPeriod;
   bool             _prevRecurring;
   QDateTime        _prevStartDateTime;

  private:

};

Q_DECLARE_METATYPE(RecurrenceWidget*)

#endif // RECURRENCEWIDGET_H
