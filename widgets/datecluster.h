/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */


#ifndef datecluster_h
#define datecluster_h

#include <QDateTime>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>

#include "xdatawidgetmapper.h"
#include "xlineedit.h"

class ParameterList;
class QFocusEvent;
class XSqlQuery;

class XDateEdit : public XLineEdit
{
  Q_OBJECT
  Q_ENUMS   (Defaults)
  Q_PROPERTY(QDate    date            READ date        WRITE setDate)
  Q_PROPERTY(Defaults defaultDate     READ defaultDate WRITE setDefaultDate)
  Q_PROPERTY(QDate    currentDefault  READ currentDefault)
  Q_PROPERTY(QString  fieldName       READ fieldName   WRITE setFieldName)

  public:
    XDateEdit(QWidget *parent = 0, const char * = 0);
    virtual ~XDateEdit();
    
    enum Defaults     { Empty, Current, None };

    virtual bool      isNull();
    virtual bool      isValid();
    virtual int       calendarSiteId()                          { return _siteId; }
    virtual Defaults  defaultDate()                             { return _default;}
    virtual QDate     currentDefault();
    virtual QDate     date();
    virtual QString   fieldName()   const                       { return _fieldName;        }
    virtual void      clear();
    inline  void      setAllowNullDate(bool pAllowNull)         { _allowNull  = pAllowNull;  }
    inline  void      setNullString(const QString &pNullString) { _nullString = pNullString;}
    inline  void      setNullDate(const QDate &pNullDate)       { _nullDate   = pNullDate;    }
    inline  void      setCalendarSiteId(int siteId)             { _siteId = siteId; }

  public slots:
    virtual void setDataWidgetMap(XDataWidgetMapper* m);
    virtual void setFieldName(QString p) { _fieldName = p; }

    void setNull();
    void setDate(const QDate &, bool = false);
    void setDefaultDate(Defaults p)                             { _default = p; }
    void parseDate();
    void showCalendar();
    void checkDate(const QDate &);

  signals:
    void newDate(const QDate &);

  private:
    bool fixMonthEnd(int *, int, int);
    
    bool          _allowNull;
    enum Defaults _default;
    QDate         _currentDate;
    QDate         _nullDate;
    QString       _fieldName;
    QString       _nullString;
    int           _siteId;
};

class XTUPLEWIDGETS_EXPORT DLineEdit : public QWidget
{
  Q_OBJECT
  Q_ENUMS(XDateEdit::Defaults)
  Q_PROPERTY(QDate                 date            READ date        WRITE setDate)
  Q_PROPERTY(XDateEdit::Defaults   defaultDate     READ defaultDate WRITE setDefaultDate)
  Q_PROPERTY(QString               fieldName       READ fieldName   WRITE setFieldName)

  public:
    DLineEdit(QWidget *parent = 0, const char * = 0);
    
    Q_INVOKABLE inline void     setAllowNullDate(bool p)       { _lineedit.setAllowNullDate(p); }
    Q_INVOKABLE inline void     setNullString(const QString &p){ _lineedit.setNullString(p); }
    Q_INVOKABLE inline void     setNullDate(const QDate &p)    { _lineedit.setNullDate(p); }
    Q_INVOKABLE inline void     setCalendarSiteId(int siteId)  { _lineedit.setCalendarSiteId(siteId); }

    Q_INVOKABLE void     setDate(const QVariant &p, bool b = false);

    Q_INVOKABLE virtual bool    isNull()          { return _lineedit.isNull(); }
    Q_INVOKABLE virtual bool    isValid()         { return _lineedit.isValid(); }
    Q_INVOKABLE virtual int     calendarSiteId()  { return _lineedit.calendarSiteId(); }
    virtual XDateEdit::Defaults defaultDate()     { return _lineedit.defaultDate(); }
    virtual QDate               date()            { return _lineedit.date(); }
    virtual QString             fieldName() const { return _lineedit.fieldName(); }
    Q_INVOKABLE virtual void    clear()           { _lineedit.clear(); }
      
  public slots:
    virtual void setDataWidgetMap(XDataWidgetMapper* m)             { _lineedit.setDataWidgetMap(m); }
    virtual void setDefaultDate(XDateEdit::Defaults p)              { _lineedit.setDefaultDate(p); }
    virtual void setEnabled(const bool);
    virtual void setFieldName(QString p)                            { _lineedit.setFieldName(p); }

    void setDate(const QDate &p, bool b = false)                    { _lineedit.setDate(p, b); }
    void setNull()                                                  { _lineedit.setNull(); }
    void showCalendar()                                             { _lineedit.showCalendar(); }

    QString toISOString();

  signals:
    void newDate(const QDate &);

  private:
    QPushButton _calbutton;
    XDateEdit   _lineedit;
};


class XTUPLEWIDGETS_EXPORT DateCluster : public QWidget
{
  Q_OBJECT
  Q_PROPERTY(QDate startDate READ startDate WRITE setStartDate)
  Q_PROPERTY(QDate endDate   READ endDate   WRITE setEndDate)

  public:
    DateCluster(QWidget *, const char * = 0);

    Q_INVOKABLE void setStartNull(const QString &, const QDate &, bool);
    Q_INVOKABLE void setEndNull(const QString &, const QDate &, bool);

    Q_INVOKABLE void appendValue(ParameterList &);
    void bindValue(XSqlQuery &);
    
    inline QDate startDate() { return _startDate->date(); }
    inline QDate endDate()   { return _endDate->date();   }

    inline void  setStartDate(const QDate &pDate) { _startDate->setDate(pDate); }
    inline void  setEndDate(const QDate &pDate)   { _endDate->setDate(pDate);   }

    Q_INVOKABLE void  setStartDate(const QVariant &pDate);
    Q_INVOKABLE void  setEndDate(const QVariant &pDate);

    Q_INVOKABLE inline bool  allValid() { return ((_startDate->isValid()) && (_endDate->isValid())); }

    Q_INVOKABLE void setStartCaption(const QString &);
    Q_INVOKABLE void setEndCaption(const QString &);

    Q_INVOKABLE bool startVisible() const { return _startDate->isVisible(); }
    Q_INVOKABLE bool endVisible() const   { return _endDate->isVisible(); }

  public slots:
    void setStartVisible(bool p);
    void setEndVisible(bool p);

  signals:
    void updated();

  protected:
    DLineEdit *_startDate;
    DLineEdit *_endDate;

  private:
    QLabel    *_startDateLit;
    QLabel    *_endDateLit;
    QString   _fieldNameStart;
    QString   _fieldNameEnd;
};

#endif

