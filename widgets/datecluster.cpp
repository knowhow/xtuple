/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include <QApplication>
#include <QCalendarWidget>
#include <QDateTime>
#include <QDesktopWidget>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QPoint>
#include <QRegExp>
#include <QSize>
#include <QSqlError>
#include <QVBoxLayout>
#include <QValidator>
#include <QDebug>

#include <xsqlquery.h>
#include <parameter.h>

#include "datecluster.h"
#include "dcalendarpopup.h"
#include "format.h"

#define DEBUG false

static bool determineIfStd()
{
  if (_x_metrics && _x_metrics->value("Application") == "Standard")
  {
    return true;
  }
  return false;
}

DCalendarPopup::DCalendarPopup(const QDate &date, QWidget *parent)
  : QWidget(parent, Qt::Popup)
{
  _cal = new QCalendarWidget(this);
  _cal->setVerticalHeaderFormat(QCalendarWidget::NoVerticalHeader);
  _cal->setSelectedDate(date);

  QVBoxLayout *vbox = new QVBoxLayout(this);
  vbox->setMargin(0);
  vbox->setSpacing(0);
  vbox->addWidget(_cal);

  connect(_cal, SIGNAL(activated(QDate)), this, SLOT(dateSelected(QDate)));
  connect(_cal, SIGNAL(clicked(QDate)),   this, SLOT(dateSelected(QDate)));
  connect(_cal, SIGNAL(activated(QDate)), this, SLOT(dateSelected(QDate)));

  // position the center of the popup near the center of its parent
  if (parent)
  {
    QSize parentsize = parent->sizeHint();
    QPoint parentcenter = parent->pos() + QPoint(parentsize.width() / 2,
						 parentsize.height() / 2);
    parentcenter = parent->mapToGlobal(parentcenter);
    QRect screen = QApplication::desktop()->availableGeometry(parentcenter);

    QSize mysize = sizeHint();
    QPoint mycenter = parentcenter;

    if (mycenter.x() + (mysize.width() / 2) > screen.right())
      mycenter.setX(screen.right() - (mysize.width() / 2));
    else if (mycenter.x() - (mysize.width() / 2) < screen.left())
      mycenter.setX(screen.left() + (mysize.width() / 2));

    if (mycenter.y() + (mysize.height() / 2) > screen.bottom())
      mycenter.setY(screen.bottom() - (mysize.height() / 2));
    else if (mycenter.y() - (mysize.height() / 2) < screen.top())
      mycenter.setY(screen.top() + (mysize.height() / 2));

    QPoint myorigin(mycenter.x() - mysize.width() / 2, mycenter.y() - mysize.height() / 2);

    move(myorigin);
  }

  _cal->setFocus();
}

void DCalendarPopup::dateSelected(const QDate &pDate)
{
  if (DEBUG)
    qDebug("DCalendarPopup::dateSelected(%s)", qPrintable(pDate.toString()));
  if (parent())
  {
    ((XDateEdit*)parent())->checkDate(pDate);
  }

  emit newDate(pDate);
  close();
}

///////////////////////////////////////////////////////////////////////////////

XDateEdit::XDateEdit(QWidget *parent, const char *name) :
  XLineEdit(parent, name)
{
  connect(this, SIGNAL(editingFinished()), this, SLOT(parseDate()));
  connect(this, SIGNAL(requestList()),     this, SLOT(showCalendar()));
  connect(this, SIGNAL(requestSearch()),   this, SLOT(showCalendar()));

  if (parent && ! parent->objectName().isEmpty())
    setObjectName(parent->objectName());
  else
    setObjectName(metaObject()->className());

  _allowNull   = FALSE;
  _default     = Empty;
  _parsed      = FALSE;
  _nullString  = QString::null;
  _valid       = FALSE;
  _siteId      = -1;
}

XDateEdit::~XDateEdit()
{
}

void XDateEdit::parseDate()
{
  QString dateString = text().trimmed();
  bool    isNumeric;

  if (DEBUG)
    qDebug("%s::parseDate() with dateString %s, _currentDate %s, _allowNull %d",
           qPrintable(parent() ? parent()->objectName() : objectName()),
           qPrintable(dateString),
           qPrintable(_currentDate.toString()), _allowNull);

#ifdef GUIClient_h
  QDate today = ofmgThis->dbDate();
#else
  QDate today = QDate::currentDate();
#endif

  if (_parsed)
  {
    if (DEBUG)
      qDebug("%s::parseDate() looks like we've already parsed this string",
             qPrintable(parent() ? parent()->objectName() : objectName()));
    return;
  }

  _valid = false;

  if (dateString == _nullString || dateString.isEmpty())
    setNull();

  else if (dateString == "0")                           // today
    checkDate(today);

  else if (dateString.contains(QRegExp("^[+-][0-9]+"))) // offset from today
  {
    int offset = dateString.toInt(&isNumeric);
    if (isNumeric)
      checkDate(today.addDays(offset));
  }

  else if (dateString[0] == '#')                        // julian day
  {
    int offset = dateString.right(dateString.length() - 1).toInt(&isNumeric);
    if (isNumeric)
      checkDate(QDate(today.year(), 1, 1).addDays(offset - 1));
  }

  else if (dateString.contains(QRegExp("^[0-9][0-9]?$"))) // date in month
  {
    int offset = dateString.toInt(&isNumeric, 10);
    if (isNumeric)
    {
      if (offset > today.daysInMonth())
        offset = today.daysInMonth();
 
      checkDate(QDate(today.year(), today.month(), 1).addDays(offset - 1));
    }
  }

  else                                                  // interpret with locale
  {
    QString dateFormatStr = QLocale().dateFormat(QLocale::ShortFormat);
    if (DEBUG)
      qDebug("%s::parseDate() trying to parse with %s",
             qPrintable(parent() ? parent()->objectName() : objectName()),
             qPrintable(dateFormatStr));

    QDate tmp = QDate::fromString(dateString, dateFormatStr);
    bool twodigitformat = !(dateFormatStr.indexOf(QRegExp("y{4}")) >= 0);
    if (tmp.isValid())
    {
      if (twodigitformat && tmp.year() < 1950) // Qt docs say 2-digit years are 1900-based so
      {
        qDebug("%s::parseDate() found valid 2-digit year %d",
               qPrintable(parent() ? parent()->objectName() : objectName()),
               tmp.year());
        tmp = tmp.addYears(100); // add backwards-compat with pre-3.0 DLineEdit
        qDebug("%s::parseDate() altered year to %d",
               qPrintable(parent() ? parent()->objectName() : objectName()),
               tmp.year());
      }
    }
    else if (twodigitformat)
    {
      // try 4 digits, ignoring the possibility of '-literals in the format str
      dateFormatStr.replace(QRegExp("y{2}"), "yyyy");
      if (DEBUG)
        qDebug("%s::parseDate() rewriting 2-digit year format string to %s",
               qPrintable(parent() ? parent()->objectName() : objectName()),
               qPrintable(dateFormatStr));
      tmp = QDate::fromString(dateString, dateFormatStr);

      if (tmp.isValid())
      {
        if (tmp.year() < 10)
          tmp = tmp.addYears(today.year() - today.year() % 100);
        if (DEBUG)
          qDebug("%s::parseDate() after changing to 4-digit year, year = %d",
                 qPrintable(parent() ? parent()->objectName() : objectName()),
                 tmp.year());
      }
      else if (DEBUG)
        qDebug("%s::parseDate() after changing to 4-digit year, date still isn't valid",
               qPrintable(parent() ? parent()->objectName() : objectName()));

    }
    else
    {
      // try 2 digits, ignoring the possibility of '-literals in the format str
      dateFormatStr.replace(QRegExp("y{4}"), "yy");
      if (DEBUG)
        qDebug("%s::parseDate() rewriting 4-digit year format string to %s",
               qPrintable(parent() ? parent()->objectName() : objectName()),
               qPrintable(dateFormatStr));
      tmp = QDate::fromString(dateString, dateFormatStr);
      if (tmp.isValid() && tmp.year() < 1950) // Qt docs say 2-digit years are 1900-based so
      {
        qDebug("%s::parseDate() found valid 2-digit year %d",
               qPrintable(parent() ? parent()->objectName() : objectName()),
               tmp.year());
        tmp = tmp.addYears(100); // add backwards-compat with pre-3.0 DLineEdit
        qDebug("%s::parseDate() altered year to %d",
               qPrintable(parent() ? parent()->objectName() : objectName()),
               tmp.year());
      }
    }
    if(!tmp.isValid())
    {
      // still no match -- we will decompose the format and input and
      // build a date based on that information
      QRegExp rx("(\\d+)");
      QRegExp rx2("(m+|y+|d+)");
      rx2.setCaseSensitivity(Qt::CaseInsensitive);
      QStringList numberList;
      QStringList formatList;
      int pos = 0;
      while ((pos = rx.indexIn(dateString, pos)) != -1)
      {
        numberList << rx.cap(1);
        pos += rx.matchedLength();
      }
      pos = 0;
      while((pos = rx2.indexIn(dateFormatStr, pos)) != -1)
      {
        formatList << rx2.cap(1);
        pos += rx2.matchedLength();
      }

      if (DEBUG)
        qDebug("%s::parseDate() aligning numberList %s with formatList %s",
               qPrintable(parent() ? parent()->objectName() : objectName()),
               qPrintable(numberList.join(":")), qPrintable(formatList.join(":")));

      // if we don't have exactly 3 and the numberList is not 2 or 3 then don't bother
      if(formatList.size() == 3 && (numberList.size() == 2 || numberList.size() == 3))
      {
        int year = today.year();
        int day = -1;
        int month = -1;

        pos = 0;
        for (int i = 0; i < formatList.size(); ++i)
        {
          QChar ch = formatList.at(i).toLower().at(0);
          if(ch == 'y' && numberList.size() == 3)
          {
            year = numberList.at(pos).toInt();
            pos++;
          }
          else if(ch == 'm')
          {
            month = numberList.at(pos).toInt();
            pos++;
          }
          else if(ch == 'd')
          {
            day = numberList.at(pos).toInt();
            pos++;
          }
        }

        // if single digit year, move it to the current century
        if (year < 10)
          year += today.year() - today.year() % 100;

        if(day > 0 && month > 0 && year > 0)
          tmp = QDate(year, month, day);
      }
      else if(formatList.size() == 3 && numberList.size() == 1)
      {
        QString ns = numberList.at(0);
        bool isNumber = false;
        (void)ns.toInt(&isNumber);
        if(isNumber && (ns.length() == 6 || ns.length() == 8))
        {
          int year = today.year();
          int day = -1;
          int month = -1;

          pos = 0;
          for (int i = 0; i < formatList.size(); ++i)
          {
            QChar ch = formatList.at(i).toLower().at(0);
            if(ch == 'y')
            {
              if(ns.length() == 8)
              {
                year = ns.mid(pos, 4).toInt();
                pos+=4;
              }
              else
              {
                year = ns.mid(pos, 2).toInt(&isNumber);
                pos+=2;
                if(isNumber)
                {
                  if(year < 50)
                    year += 2000;
                  else
                    year += 1900;
                }
              }
            }
            else if(ch == 'm')
            {
              month = ns.mid(pos, 2).toInt();
              pos+=2;
            }
            else if(ch == 'd')
            {
              day = ns.mid(pos, 2).toInt();
              pos+=2;
            }
          }

          if(day > 0 && month > 0 && year > 0)
            tmp = QDate(year, month, day);
        }
      }
    }

    checkDate(QDate(tmp.year(), tmp.month(), tmp.day()));
  }

  if (!_valid)
    setText("");

  _parsed = true;
}

void XDateEdit::setDataWidgetMap(XDataWidgetMapper* m)
{
  m->addMapping(this, _fieldName, "date", "currentDefault");
}

void XDateEdit::setNull()
{
  if (DEBUG)
    qDebug("%s::setNull() with _currentDate %s, _allowNull %d",
           qPrintable(parent() ? parent()->objectName() : objectName()),
           qPrintable(_currentDate.toString()),
           _allowNull);
  if (_allowNull)
  {
    _valid  = TRUE;
    _parsed = TRUE;
    setText(_nullString);
    _currentDate = _nullDate;
    emit newDate(_currentDate);
  }
  else
  {
    _valid = FALSE;
    _parsed = TRUE;
    setText("");
    _currentDate = QDate();
  }
}

void XDateEdit::setDate(const QDate &pDate, bool pAnnounce)
{
  if (DEBUG)
    qDebug("%s::setDate(%s, %d) with _currentDate %s, _allowNull %d",
           qPrintable(parent() ? parent()->objectName() : objectName()),
           qPrintable(pDate.toString()), pAnnounce,
           qPrintable(_currentDate.toString()), _allowNull);

  if (pDate.isNull())
    setNull();
  else
  {
    if(!pAnnounce)
    {
      if(determineIfStd() && (_siteId != -1))
        return checkDate(pDate);
    }
    else
    {
      pAnnounce = (pDate != _currentDate);
    }
    _currentDate = pDate;
    _valid = _currentDate.isValid();
    _parsed = _valid;

    if (DEBUG)
      qDebug("%s::setDate() setting text",
             qPrintable(parent() ? parent()->objectName() : objectName()));
    if ((_allowNull) && (_currentDate == _nullDate))
      setText(_nullString);
    else
      setText(formatDate(pDate));
    if (DEBUG)
      qDebug("%s::setDate() done setting text",
             qPrintable(parent() ? parent()->objectName() : objectName()));
  }

  if (pAnnounce)
  {
    if (DEBUG)
      qDebug("%s::setDate() emitting newDate(%s)",
             qPrintable(parent() ? parent()->objectName() : objectName()),
             qPrintable(_currentDate.toString()));
    emit newDate(_currentDate);
  }
}

void XDateEdit::checkDate(const QDate &pDate)
{
  QDate nextWorkDate = pDate;

  if(determineIfStd() && (_siteId != -1))
  {
    XSqlQuery workday;

    workday.prepare("SELECT calculatenextworkingdate(:whsid, :date, :desired) AS result;");
    workday.bindValue(":whsid", _siteId);
    workday.bindValue(":date", pDate);
    workday.bindValue(":desired", 0);
    workday.exec();
    if (workday.first())
      nextWorkDate = workday.value("result").toDate();
    else if (workday.lastError().type() != QSqlError::NoError)
    {
      QMessageBox::warning(this, tr("No work week calendar found"),
                            tr("<p>The selected site has no work week defined. "
                               "Please go to Schedule Setup and define "
                               "the working days for this site."));
      return;
    }
  }

  if (nextWorkDate == pDate)
    setDate(pDate, TRUE);
  else
  {
    if (QMessageBox::question(this, tr("Non-Working Day Entered"),
                             tr("<p>The selected Date is not a Working "
                                "Day for the site selected. Do you want to "
                                "automatically select a next working day?"),
                             QMessageBox::Yes | QMessageBox::Default,
                             QMessageBox::No  | QMessageBox::Escape) == QMessageBox::Yes)
      setDate(nextWorkDate, TRUE);
    else
      clear();
  }
}

void XDateEdit::clear()
{
  if (DEBUG)
    qDebug("%s::clear()",
          qPrintable(parent() ? parent()->objectName() : objectName()));
  setDate(_nullDate, true);
}

QDate XDateEdit::currentDefault()
{
  XSqlQuery query;
  
  if (_default==Empty)
    return _nullDate;
  else if (_default==Current)
  {
    query.exec("SELECT current_date AS result;");
    if (query.first())
      return query.value("result").toDate();
  }
  return date();
}

QDate XDateEdit::date()
{
  if (DEBUG)
    qDebug("%s::date()",
            qPrintable(parent() ? parent()->objectName() : objectName()));

  if (!_parsed)
    parseDate();

  if (!_valid)
    return _nullDate;

  return _currentDate;
}

bool XDateEdit::isNull()
{
  if (DEBUG)
    qDebug("%s::isNull()",
            qPrintable(parent() ? parent()->objectName() : objectName()));

  if (!_parsed)
    parseDate();

  return date().isNull();
}

bool XDateEdit::isValid()
{
  if (DEBUG)
    qDebug("%s::isValid()",
            qPrintable(parent() ? parent()->objectName() : objectName()));
  if (!_parsed)
    parseDate();

  return _valid;
}

void XDateEdit::showCalendar()
{
  if (DEBUG)
    qDebug("%s::showCalendar()",
            qPrintable(parent() ? parent()->objectName() : objectName()));
  QDate d = date();
  if(d.isNull() || d == _nullDate)
    d = QDate::currentDate();
  DCalendarPopup *cal = new DCalendarPopup(d, this);
  connect(cal, SIGNAL(newDate(const QDate &)), this, SIGNAL(newDate(const QDate &)));
  cal->show();
}

///////////////////////////////////////////////////////////////////////////////

DLineEdit::DLineEdit(QWidget *parent, const char *name) :
  QWidget(parent)
{
  setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
  setFocusPolicy(Qt::StrongFocus);
  setMaximumWidth(200);
  setObjectName(name);

  QHBoxLayout *hbox = new QHBoxLayout(this);
  hbox->addWidget(&_lineedit);
  hbox->addWidget(&_calbutton);
  hbox->setSpacing(1);
  hbox->setMargin(0);

  QPixmap pixmap(":/widgets/images/calendar_24.png"); 
  _calbutton.setIconSize(QSize(24,24));
  _calbutton.setIcon(QIcon(pixmap));
  _calbutton.setFlat(true);
  _calbutton.setMaximumSize(pixmap.size());
  _calbutton.setFocusPolicy(Qt::NoFocus);

  connect(&_calbutton,            SIGNAL(clicked()), &_lineedit, SLOT(showCalendar()));
  connect(&_lineedit,     SIGNAL(editingFinished()), &_lineedit, SLOT(parseDate()));
  connect(&_lineedit,SIGNAL(newDate(const QDate &)), this,       SIGNAL(newDate(const QDate &)));

  setFocusProxy(&_lineedit);
}

/*!
  Sets the date on the cluster to \a p where \a b flags whether to emit the newDate signal.
  Specifically designed to work in a scripted environment where java script dates are passed
  as a QVariant.
*/
void DLineEdit::setDate(const QVariant &p, bool b)
{
  _lineedit.setDate(p.toDate(), b);
}

void DLineEdit::setEnabled(const bool p)
{
  QWidget::setEnabled(p);
  _lineedit.setEnabled(p);
  _calbutton.setEnabled(p);
}

QString DLineEdit::toISOString()
{
  QDate mydate = _lineedit.date();
  return mydate.toString(Qt::ISODate);
}

DateCluster::DateCluster(QWidget *pParent, const char *pName) : QWidget(pParent)
{
  if(pName)
    setObjectName(pName);

  QSizePolicy tsizePolicy(static_cast<QSizePolicy::Policy>(0), static_cast<QSizePolicy::Policy>(0));
  tsizePolicy.setHorizontalStretch(0);
  tsizePolicy.setVerticalStretch(0);
  tsizePolicy.setHeightForWidth(sizePolicy().hasHeightForWidth());
  setSizePolicy(tsizePolicy);

  QHBoxLayout *mainLayout = new QHBoxLayout(this);
  mainLayout->setSpacing(5);
  mainLayout->setMargin(0);
  mainLayout->setObjectName(QString::fromUtf8("mainLayout"));

  QVBoxLayout *literalLayout = new QVBoxLayout();
  literalLayout->setSpacing(5);
  literalLayout->setMargin(0);
  literalLayout->setObjectName(QString::fromUtf8("literalLayout"));

  _startDateLit = new QLabel(tr("Start Date:"), this);
  _startDateLit->setObjectName("_startDateLit");
  _startDateLit->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
  literalLayout->addWidget(_startDateLit);

  _endDateLit = new QLabel(tr("End Date:"), this);
  _endDateLit->setObjectName("_endDateLit");
  _endDateLit->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
  literalLayout->addWidget(_endDateLit);

  mainLayout->addLayout(literalLayout);

  QVBoxLayout *dataLayout = new QVBoxLayout();
  dataLayout->setSpacing(5);
  dataLayout->setMargin(0);
  dataLayout->setObjectName(QString::fromUtf8("dataLayout"));

  _startDate = new DLineEdit(this);
  _startDate->setObjectName("_startDate");
  dataLayout->addWidget(_startDate);

  _endDate = new DLineEdit(this);
  _endDate->setObjectName("_endDate");
  dataLayout->addWidget(_endDate);

  mainLayout->addLayout(dataLayout);

  _startDateLit->setBuddy(_startDate);
  _endDateLit->setBuddy(_endDate);

  connect(_startDate, SIGNAL(newDate(const QDate &)), this, SIGNAL(updated()));
  connect(_endDate,   SIGNAL(newDate(const QDate &)), this, SIGNAL(updated()));

  //setTabOrder(_startDate, _endDate);
  //setTabOrder(_endDate, _startDate);
  setFocusProxy(_startDate);
}

/*!
  Sets the start date on the cluster to \a pDate.  Specifically designed to work
  in a scripted environment where java script dates are passed as a QVariant.
*/
void DateCluster::setStartDate(const QVariant &pDate)
{
  _startDate->setDate(pDate.toDate());
}

/*!
  Sets the end date on the cluster to \a pDate.  Specifically designed to work
  in a scripted environment where java script dates are passed as a QVariant.
*/
void DateCluster::setEndDate(const QVariant &pDate)
{
  _endDate->setDate(pDate.toDate());
}

void DateCluster::setStartNull(const QString &pString, const QDate &pDate, bool pSetNull)
{
  _startDate->setAllowNullDate(TRUE);
  _startDate->setNullString(pString);
  _startDate->setNullDate(pDate);

  if (pSetNull)
    _startDate->setNull();
}

void DateCluster::setEndNull(const QString &pString, const QDate &pDate, bool pSetNull)
{
  _endDate->setAllowNullDate(TRUE);
  _endDate->setNullString(pString);
  _endDate->setNullDate(pDate);

  if (pSetNull)
    _endDate->setNull();
}

void DateCluster::setStartCaption(const QString &pString)
{
  _startDateLit->setText(pString);
}

void DateCluster::setEndCaption(const QString &pString)
{
  _endDateLit->setText(pString);
}

void DateCluster::appendValue(ParameterList &pParams)
{
  pParams.append("startDate", _startDate->date());
  pParams.append("endDate", _endDate->date());
}

void DateCluster::bindValue(XSqlQuery &pQuery)
{
  pQuery.bindValue(":startDate", _startDate->date());
  pQuery.bindValue(":endDate", _endDate->date());
}

void DateCluster::setStartVisible(bool p)
{
  _startDate->setVisible(p);
  _startDateLit->setVisible(p);
}

void DateCluster::setEndVisible(bool p)
{
  _endDate->setVisible(p);
  _endDateLit->setVisible(p);
}
