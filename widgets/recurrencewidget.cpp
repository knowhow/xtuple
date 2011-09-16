/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "recurrencewidget.h"

#include <QDebug>
#include <QMessageBox>
#include <QSqlError>
#include <QtScript>

#include "xsqlquery.h"

#include "storedProcErrorLookup.h"

#define DEBUG false

/**
  \class RecurrenceWidget
  
  \brief The RecurrenceWidget gives the %user a unified interface for telling
         the system how often certain %events occur.

  In general recurrences are stored in the recur table, but this is not
  required.
  Two basic values describe a recurrence:
  <UL>
  <LI>The period is the time unit of measure (hour, day, month).
  </LI>
  <LI>The frequency is the number of periods between recurrences.</LI>
  </UL>
  A recurrence with the period set to W (= week) and frequency of 3 will repeat
  once every three weeks.

  To add a new kind of recurring event or %item, you need to change data in the
  database, stored procedures, triggers, and application code.
  We'll use Invoice in the examples here, with 'I' as the internal code value,
  even though %invoices already use this mechanism (xTuple ERP 3.5.1 and later).

  Add a column to the parent table to track the recurrence parent/child
  relationship. The column name must follow this pattern:
    \c [tablename]_recurring_[tablename]_id ,
  e.g.
    \c invchead_recurring_invchead_id :
  \code
  ALTER TABLE invchead ADD COLUMN invchead_recurring_invchead_id INTEGER;
  COMMENT ON COLUMN invchead.invchead_recurring_invchead_id IS 'The first invchead record in the series if this is a recurring Invoice. If the invchead_recurring_invchead_id is the same as the invchead_id, this record is the first in the series.';
  \endcode

  Add a RecurrenceWidget to the .ui for the window that will maintain data of
  this type (e.g. invoice.ui).

  Initialize the widget in the window's constructor:
  \code
  _recur->setParent(-1, 'I');
  \endcode

  Update the widget again when the window gets an id for the object.
  This usually happens in either the
  %set(), %save(), or %sSave() method for new objects, depending on the class,
  and in %populate() when editing existing objects:
  \code
  // cNew case in invoice::set()
  _recur->setParent(_invcheadid, 'I');
  ...
  // invoice::populate()
  _recur->setParent(q.value("invchead_recurring_invchead_id").toInt(), "I");
  \endcode

  Ask the %user how to handle existing recurrences before saving the data
  and before starting a transaction.  getChangePolicy will return
  RecurrenceWidget::NoPolicy) if the %user chooses to cancel.
  Then when preparing to insert or update the
  main record, make sure to set the recurrence parentage.
  Finally, after the insert/update, save the recurrence and check for errors:

  \code
  bool %save()
  {
    // error checking
    RecurrenceWidget::ChangePolicy cp = _recur->getChangePolicy();
    if (cp == RecurrenceWidget::NoPolicy)
      return false;

    XSqlQuery beginq("BEGIN;");
    XSqlQuery qry;
    ...
    // set up qry to insert or update
    if (_recur->isRecurring())
      qry.bindValue(":invchead_recurring_invchead_id", _recur->parentId());
    ...
    // execute the insert/update
    ...
    QString errmsg;
    if (! _recur->save(true, cp, &errmsg))
    {
      rollbackq.exec();
      systemError(this, errmsg, __FILE__, __LINE__);
      return false;
    }
    ...
    // finish processing
    XSqlQuery commitq("COMMIT;");
    return true;
  }
  \endcode

  To maintain the recurrence relationships,
  there must be a function that copies an existing record
  based on its id and gives the copy a different timestamp or date.
  It must have a function signature like one of these:
  <OL>
    <LI>copy[tablename](INTEGER, TIMESTAMP WITH TIME ZONE)</LI>
    <LI>copy[tablename](INTEGER, TIMESTAMP WITHOUT TIME ZONE)</LI>
    <LI>copy[tablename](INTEGER, DATE)</LI>
  </OL>
  The copy function can take additional arguments as well, but they will be
  ignored by the recurrence maintenance functions. The data type of each
  argument must be listed in
  the recurtype table's recurtype_copyargs column (see below) so the appropriate
  casting can be done for the date or timestamp and an appropriate number
  of NULL arguments can be passed.
  The copy function must copy the
  \c [tablename]_recurring_[tablename]_id
  column.
  
  There can be a function to delete records of this type as well. If there is
  one, it must accept a single integer id of the record to delete.
  
  Add a row to the recurtype table to
  describe how the recurrence stored procedures interact with the
  events/items of this type ('I' == Invoice).
  \code
  INSERT INTO recurtype (recurtype_type, recurtype_table, recurtype_donecheck,
                         recurtype_schedcol, recurtype_limit,
                         recurtype_copyfunc, recurtype_copyargs, recurtype_delfunc
   ) VALUES ('I', 'invchead', 'invchead_posted',
             'invchead_invcdate', NULL,
             'copyinvoice', '{integer,date}', 'deleteinvoice');
  \endcode

  If there isn't a delete function, set the recurtype_delfunc to
  NULL.  Existing records will be deleted when necessary with an
  SQL DELETE statement.

  The DELETE trigger on the table should clean up the recurrence information:
  \code
  CREATE OR REPLACE FUNCTION _invcheadBeforeTrigger() RETURNS "trigger" AS $$
  DECLARE
    _recurid     INTEGER;
    _newparentid INTEGER;

  BEGIN
    IF (TG_OP = 'DELETE') THEN
      -- after other stuff not having to do with recurrence

      SELECT recur_id INTO _recurid
        FROM recur
       WHERE ((recur_parent_id=OLD.invchead_id)
          AND (recur_parent_type='I'));
      IF (_recurid IS NOT NULL) THEN
        SELECT invchead_id INTO _newparentid
          FROM invchead
         WHERE ((invchead_recurring_invchead_id=OLD.invchead_id)
            AND (invchead_id!=OLD.invchead_id))
         ORDER BY invchead_invcdate
         LIMIT 1;

        IF (_newparentid IS NULL) THEN
          DELETE FROM recur WHERE recur_id=_recurid;
        ELSE
          UPDATE recur SET recur_parent_id=_newparentid
           WHERE recur_id=_recurid;
          UPDATE invchead SET invchead_recurring_invchead_id=_newparentid
           WHERE invchead_recurring_invchead_id=OLD.invchead_id
             AND invchead_id!=OLD.invchead_id;
        END IF;
      END IF;

      RETURN OLD;
    END IF;

    RETURN NEW;
  END;
  $$ LANGUAGE 'plpgsql';
  \endcode

  \todo Simplify this so the developer of a new recurring item/event doesn't
        have to work so hard. There should be a way to (1) have the recurrence
        widget update the [tablename]_recurring_[tablename]_id column
        when saving the recurrence (this would save a couple of lines)
        and (2) there should be a way to simplify or eliminate the code
        in the DELETE triggers. Some of this stuff really belongs in an
        AFTER trigger instead of a before trigger.

  \see _invcheadBeforeTrigger
  \see copyInvoice
  \see createRecurringItems
  \see deleteInvoice
  \see deleteOpenRecurringItems
  \see invoice
  \see openRecurringItems
  \see recur
  \see recurtype
  \see splitRecurrence

 */

/* TODO: Add properties to set minimum/maximum periods?
         (e.g. it doesn't make sense for invoicing to recur hourly).

         The enum value Custom is intended for use by cases like
            Recur every Tuesday and Thursday
            Recur 1st of every month
            Recur every January and July

         It would be nice to have drag-n-drop using iCalendar (IETF RFC 5545)
         but this will have to be associated with the To-do or calendar item,
         not just the recurrence widget.
 */

RecurrenceWidget::RecurrenceWidget(QWidget *parent, const char *pName) :
  QWidget(parent)
{
  setupUi(this);

  if(pName)
    setObjectName(pName);

  setMinPeriod(Minutely);

  if (minPeriod() < Weekly)
    _period->setCode("W");

  _dates->setStartCaption(tr("From:"));
  _dates->setEndCaption(tr("Until:"));
  _dates->setStartNull(tr("Today"), QDate::currentDate(), true);
  setStartDateVisible(false);
  setEndTimeVisible(false);
  setStartTimeVisible(false);

  setMaxVisible(true);

  if (_x_preferences)
  {
    XSqlQuery eotq("SELECT endOfTime() AS eot;");
    if (eotq.first())
    {
      _eot = QDateTime(eotq.value("eot").toDate(), QTime(23, 59, 59, 999));
      _dates->setEndNull(tr("Forever"), _eot.date(), true);
    }
    else
      qWarning("RecurrenceWidget could not get endOfTime()");
  }

  _dates->setEndNull(tr("Forever"), _eot.date(), true);
  if (! _eot.isValid())
    _eot = QDateTime(QDate(2100,12,31), QTime(23, 59, 59, 999));

  clear();
}

RecurrenceWidget::~RecurrenceWidget()
{
}

void RecurrenceWidget::languageChange()
{
}

/** \brief Set the RecurrenceWidget to its default state.
  */
void RecurrenceWidget::clear()
{
  set(false, 1, "W", QDateTime::currentDateTime(), _eot, 1);
  _id             = -1;

  _prevParentId   = -1;
  _prevParentType = "";
  _parentId       = -1;
  _parentType     = "";
}

/** \brief Convert a string representation of the period to a RecurrencePeriod.

     This is used to convert between the values stored in the recur table
     and the enumerated values used internally by the RecurrenceWidget.
     It also accepts human-readable values, such as "Minutes" and "Hours".

     \param p The recur_period or human-readable period name to convert.
              This is case-sensitive. Currently accepted values are
              m or Minutes, H or Hours, D or Days, W or Weeks, M or Months,
              Y or Years, C or Custom. If current translation includes
              the period names, the translations are also accepted.

     \return The RecurrencePeriod that matches the input or Never
             if no match was found.
  */
RecurrenceWidget::RecurrencePeriod RecurrenceWidget::stringToPeriod(QString p) const
{
  if (p == "m"      || p == "Minutes" || p == tr("Minutes"))  return Minutely;
  else if (p == "H" || p == "Hours"   || p == tr("Hours")  )  return Hourly;
  else if (p == "D" || p == "Days"    || p == tr("Days")   )  return Daily;
  else if (p == "W" || p == "Weeks"   || p == tr("Weeks")  )  return Weekly;
  else if (p == "M" || p == "Months"  || p == tr("Months") )  return Monthly;
  else if (p == "Y" || p == "Years"   || p == tr("Years")  )  return Yearly;
  else if (p == "C" || p == "Custom"  || p == tr("Custom") )  return Custom;
  else return Never;
}

/** \brief Return the date the recurrence is set to end.  */
QDate RecurrenceWidget::endDate() const
{
  return _dates->endDate();
}

/** \brief Return the date and time the recurrence is set to end. */
QDateTime RecurrenceWidget::endDateTime() const
{
  return QDateTime(_dates->endDate(), _endTime->time());
}

/** \brief Return the time of day the recurrence is set to end. */
QTime RecurrenceWidget::endTime() const
{
  return _endTime->time();
}

/** \brief Return whether the end date field is visible. */
bool RecurrenceWidget::endDateVisible() const
{
  return _dates->endVisible();
}

/** \brief Return whether the end time field is visible. */
bool RecurrenceWidget::endTimeVisible() const
{
  return _endTime->isVisible();
}

/** \brief Return the frequency (number of periods) the recurrence is set
           to run.
 */
int RecurrenceWidget::frequency() const
{
  return _frequency->value();
}

RecurrenceWidget::RecurrenceChangePolicy RecurrenceWidget::getChangePolicy()
{
  if (DEBUG)
    qDebug("%s::getChangePolicy() entered with id %d type %s",
           qPrintable(objectName()), _parentId, qPrintable(_parentType));

  if (! modified())
    return IgnoreFuture;

  RecurrenceChangePolicy ans;

  XSqlQuery futureq;
  futureq.prepare("SELECT openRecurringItems(:parent_id, :parent_type,"
                  "                          NULL) AS open;");
  futureq.bindValue(":parent_id",   _parentId);
  futureq.bindValue(":parent_type", _parentType);
  futureq.exec();
  if (futureq.first())
  {
    int open = futureq.value("open").toInt();
    if (open < 0)
    {
      QMessageBox::critical(this, tr("Processing Error"),
                            storedProcErrorLookup("openRecurringItems", open));
      return NoPolicy;
    }
    else if (open == 0)
      return ChangeFuture;
  }
  else if (futureq.lastError().type() != QSqlError::NoError)
  {
    QMessageBox::critical(this, tr("Database Error"),
                          futureq.lastError().text());
    return NoPolicy;
  }

  switch (QMessageBox::question(this, tr("Change Open Events?"),
                                tr("<p>This event is part of a recurring "
                                   "series. Do you want to change all open "
                                   "events in this series?"),
                                QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel,
                                QMessageBox::Yes))
  {
    case QMessageBox::Yes:      ans = ChangeFuture; break;
    case QMessageBox::No:       ans = IgnoreFuture; break;
    case QMessageBox::Cancel:
    default:                    ans = NoPolicy;     break;
  };

  return ans;
}

bool RecurrenceWidget::isRecurring() const
{
  return _recurring->isChecked();
}

int RecurrenceWidget::max() const
{
  return _max->value();
}

bool RecurrenceWidget::maxVisible() const
{
  return _max->isVisible();
}

RecurrenceWidget::RecurrencePeriod RecurrenceWidget::minPeriod() const
{
  return (RecurrencePeriod)_period->id(0);
}

bool RecurrenceWidget::modified() const
{
  if (DEBUG)
    qDebug() << "recurring:"     << isRecurring()  << _prevRecurring     << "\n"
             << "period:"        << period()       << _prevPeriod        << "\n"
             << "frequency:"     << frequency()    << _prevFrequency     << "\n"
             << "startDateTime:" << startDateTime()<< _prevStartDateTime << "\n"
             << "endDateTime:"   << endDateTime()  << _prevEndDateTime   << "\n"
             << "max:"           << max()          << _prevMax           << "\n"
             << "_parentId:"     << _parentId      << _prevParentId      << "\n"
             << "_parentType:"   << _parentType    << _prevParentType
             ;

  bool returnVal = (isRecurring()   != _prevRecurring    ||
                    period()        != _prevPeriod       ||
                    frequency()     != _prevFrequency    ||
                    startDateTime() != _prevStartDateTime||
                    endDateTime()   != _prevEndDateTime  ||
                    max()           != _prevMax          ||
                    _parentId       != _prevParentId     ||
                    _parentType     != _prevParentType);

  return returnVal;
}

int RecurrenceWidget::parentId() const
{
  return _parentId;
}

QString RecurrenceWidget::parentType() const
{
  return _parentType;
}

RecurrenceWidget::RecurrencePeriod RecurrenceWidget::period() const
{
  return (RecurrencePeriod)(_period->id());
}

QString RecurrenceWidget::periodCode() const
{
  return _period->code();
}

bool RecurrenceWidget::save(bool externaltxn, RecurrenceChangePolicy cp, QString *message)
{
  if (! message)
    message = new QString();

  if (DEBUG)
    qDebug("%s::save(%d, %d, %p) entered with id %d type %s",
           qPrintable(objectName()), externaltxn, cp, message, _parentId,
           qPrintable(_parentType));

  if (! modified())
    return true;

  if (_parentId < 0 || _parentType.isEmpty())
  {
    *message = tr("Could not save Recurrence information. The "
                  "parent object/event has not been set.");
    if (! externaltxn)
      QMessageBox::warning(this, tr("Missing Data"), *message);
    else
      qWarning("%s", qPrintable(*message));
    return false;
  }

  if (! externaltxn && cp == NoPolicy)
  {
    cp = getChangePolicy();
    if (cp == NoPolicy)
      return false;
  }
  else if (externaltxn && cp == NoPolicy)
  {
    *message = tr("You must choose how open events are to be handled");
    qWarning("%s", qPrintable(*message));
    return false;
  }

  XSqlQuery rollbackq;
  if (! externaltxn)
  {
    XSqlQuery beginq("BEGIN;");
    rollbackq.prepare("ROLLBACK;");
  }

  XSqlQuery recurq;
  if (isRecurring())
  {
    if (_id > 0)
    {
      if (cp == ChangeFuture)
      {
        XSqlQuery futureq;
        futureq.prepare("SELECT splitRecurrence(:parent_id, :parent_type,"
                        "                       :splitdate) AS newrecurid;");
        futureq.bindValue(":parent_id",   _parentId);
        futureq.bindValue(":parent_type", _parentType);
        futureq.bindValue(":splitdate",   startDate());
        futureq.exec();
        if (futureq.first())
        {
          int result = futureq.value("newrecurid").toInt();
          if (result > 0)
          {
            _id = result;
            futureq.prepare("SELECT recur_parent_id"
                            "  FROM recur"
                            " WHERE recur_id=:recur_id;");
            futureq.bindValue(":recur_id", _id);
            futureq.exec();
            if (futureq.first())
              _parentId = futureq.value("recur_parent_id").toInt();
          }
          else if (result < 0)
          {
            *message = storedProcErrorLookup("splitRecurrence", result);
            if (! externaltxn)
            {
              rollbackq.exec();
              QMessageBox::warning(this, tr("Processing Error"), *message);
            }
            else
              qWarning("%s", qPrintable(*message));
            return false;
          }
        }
        // one check for potentially 2 queries
        if (futureq.lastError().type() != QSqlError::NoError)
        {
          *message = futureq.lastError().text();
          if (! externaltxn)
          {
            rollbackq.exec();
            QMessageBox::warning(this, tr("Database Error"), *message);
          }
          else
            qWarning("%s", qPrintable(*message));
          return false;
        }
      }

      recurq.prepare("UPDATE recur SET"
                     "  recur_parent_id=:recur_parent_id,"
                     "  recur_parent_type=UPPER(:recur_parent_type),"
                     "  recur_period=:recur_period,"
                     "  recur_freq=:recur_freq,"
                     "  recur_start=:recur_start,"
                     "  recur_end=:recur_end,"
                     "  recur_max=:recur_max"
                     " WHERE (recur_id=:recurid)"
                     " RETURNING recur_id;");
      recurq.bindValue(":recurid", _id);
    }
    else
    {
      recurq.prepare("INSERT INTO recur ("
                     "  recur_parent_id,  recur_parent_type,"
                     "  recur_period,     recur_freq,"
                     "  recur_start,      recur_end,"
                     "  recur_max"
                     ") VALUES ("
                     "  :recur_parent_id, UPPER(:recur_parent_type),"
                     "  :recur_period,    :recur_freq,"
                     "  :recur_start,     :recur_end,"
                     "  :recur_max"
                     ") RETURNING recur_id;");
    }

    recurq.bindValue(":recur_parent_id",   _parentId);
    recurq.bindValue(":recur_parent_type", _parentType);
    recurq.bindValue(":recur_period",      periodCode());
    recurq.bindValue(":recur_freq",        frequency());
    recurq.bindValue(":recur_start",       startDateTime());
    if (endDate() < _eot.date())
      recurq.bindValue(":recur_end",       endDateTime());
    recurq.bindValue(":recur_max",         max());
    recurq.exec();
    if (recurq.first())
    {
      _id = recurq.value("recur_id").toInt();
      _prevParentId      = _parentId;
      _prevParentType    = _parentType;
      _prevEndDateTime   = endDateTime();
      _prevFrequency     = frequency();
      _prevMax           = max();
      _prevPeriod        = period();
      _prevRecurring     = isRecurring();
      _prevStartDateTime = startDateTime();
    }
  }
  else // ! isRecurring()
  {
    recurq.prepare("DELETE FROM recur"
                   " WHERE ((recur_parent_id=:recur_parent_id)"
                   "    AND (UPPER(recur_parent_type)=UPPER(:recur_parent_type)));");
    recurq.bindValue(":recur_parent_id",   _parentId);
    recurq.bindValue(":recur_parent_type", _parentType);
    recurq.exec();
  }

  if (recurq.lastError().type() != QSqlError::NoError)
  {
    *message = recurq.lastError().text();
    if (! externaltxn)
    {
      rollbackq.exec();
      QMessageBox::warning(this, tr("Database Error"), *message);
    }
    else
      qWarning("%s", qPrintable(*message));
    return false;
  }

  if (cp == ChangeFuture)
  {
    int procresult = -1;
    QString procname = "deleteOpenRecurringItems";
    XSqlQuery cfq;
    cfq.prepare("SELECT deleteOpenRecurringItems(:parentId, :parentType,"
                "                                :splitdate, FALSE) AS result;");
    cfq.bindValue(":parentId",   _parentId);
    cfq.bindValue(":parentType", _parentType);
    cfq.bindValue(":splitdate",  startDate());
    cfq.exec();
    if (cfq.first())
    {
      procresult = cfq.value("result").toInt();
      if (procresult >= 0)
      {
        QString procname = "createOpenRecurringItems";
        cfq.prepare("SELECT createRecurringItems(:parentId, :parentType)"
                    "       AS result;");
        cfq.bindValue(":parentId",   _parentId);
        cfq.bindValue(":parentType", _parentType);
        cfq.exec();
        if (cfq.first())
          procresult = cfq.value("result").toInt();
      }
    }

    // error handling for either 1 or 2 queries so not elseif
    // check cfq.lastError() first to avoid misreporting db errs as -1
    if (cfq.lastError().type() != QSqlError::NoError)
    {
      *message = cfq.lastError().text();
      if (! externaltxn)
      {
        rollbackq.exec();
        QMessageBox::critical(this, tr("Database Error"), *message);
      }
      else
        qWarning("%s", qPrintable(*message));
      return false;
    }
    else if (procresult < 0)
    {
      *message = storedProcErrorLookup(procname, procresult);
      if (! externaltxn)
      {
        rollbackq.exec();
        QMessageBox::critical(this, tr("Processing Error"), *message);
      }
      else
        qWarning("%s", qPrintable(*message));
      return false;
    }
  }

  if (! externaltxn)
    XSqlQuery commitq("COMMIT;");
  return true;
}

void RecurrenceWidget::set(bool recurring, int frequency, QString period, QDate startDate, QDate endDate, int max)
{
  if (DEBUG)
    qDebug() << objectName() << "::set(" << recurring << ", "
             << frequency    << ", "     << period    << ", "
             << startDate    << ", "     << endDate   << ", "
             << max          << ") entered";
  // run from the beginning of the start date to the end of the end date
  QDateTime startDateTime(startDate);
  QDateTime endDateTime(endDate.addDays(1));
  endDateTime = endDateTime.addMSecs(-1);

  set(recurring, frequency, period, startDateTime, endDateTime, max);
}

void RecurrenceWidget::set(bool recurring, int frequency, QString period, QDateTime start, QDateTime end, int max)
{
  if (DEBUG)
    qDebug() << objectName() << "::set(" << recurring << ", "
             << frequency    << ", "     << period    << ", "
             << start        << ", "     << end       << ", "
             << max          << ") entered";
  setRecurring(recurring);
  setPeriod(period);
  setFrequency(frequency);
  setStartDateTime(start);
  setEndDateTime(end);
  setMax(max);

  _prevEndDateTime   = end.isValid() ? end : _eot ;
  _prevFrequency     = frequency;
  _prevPeriod        = stringToPeriod(period);
  _prevRecurring     = recurring;
  _prevStartDateTime = start;
  _prevMax           = max;
}

void RecurrenceWidget::setEndDate(QDate p)
{
  _dates->setEndDate(p.isValid() ? p : _eot.date());
  _endTime->setTime(QTime(23, 59, 59, 999));
}

void RecurrenceWidget::setEndDateTime(QDateTime p)
{
  if (DEBUG)
    qDebug("setEndDateTime(%s) entered, valid = %d",
           qPrintable(p.toString()), p.isValid());
  if (p.isValid())
  {
    _dates->setEndDate(p.date());
    _endTime->setTime(p.time());
  }
  else
    setEndDateTime(_eot);
}

void RecurrenceWidget::setEndDateVisible(bool p)
{
  _dates->setEndVisible(p);
}

void RecurrenceWidget::setEndTime(QTime p)
{
  _endTime->setTime(p);
}

void RecurrenceWidget::setEndTimeVisible(bool p)
{
  _endTime->setVisible(p);
}

void RecurrenceWidget::setFrequency(int p)
{
  _frequency->setValue(p);
}

void RecurrenceWidget::setMax(int p)
{
  _max->setValue(p);
}

void RecurrenceWidget::setMaxVisible(bool p)
{
  _max->setVisible(p);
  _maxLit->setVisible(p);
}

void RecurrenceWidget::setMinPeriod(RecurrencePeriod min)
{
  _period->clear();

  // fall through all cases in the switch
  switch (min)
  {
    default:
    // case Never: _period->append(Never,        tr("Never"),   "");
    case Minutely: _period->append(Minutely,     tr("Minutes"), "m");
    case Hourly:   _period->append(Hourly,       tr("Hours"),   "H");
    case Daily:    _period->append(Daily,        tr("Days"),    "D");
    case Weekly:   _period->append(Weekly,       tr("Weeks"),   "W");
    case Monthly:  _period->append(Monthly,      tr("Months"),  "M");
    case Yearly:   _period->append(Yearly,       tr("Years"),   "Y");
    //case Custom: _period->append(Custom,       tr("Custom"),  "C");
  }
}

bool RecurrenceWidget::setParent(int pid, QString ptype)
{
  _parentId       = pid;
  _parentType     = ptype;

  XSqlQuery recurq;
  recurq.prepare("SELECT *"
                 "  FROM recur"
                 " WHERE ((recur_parent_id=:parentid)"
                 "   AND  (recur_parent_type=UPPER(:parenttype)));");
  recurq.bindValue(":parentid",   pid);
  recurq.bindValue(":parenttype", ptype);
  recurq.exec();
  if (recurq.first())
  {
    set(true,
        recurq.value("recur_freq").toInt(),
        recurq.value("recur_period").toString(),
        recurq.value("recur_start").toDateTime(),
        recurq.value("recur_end").toDateTime(),
        recurq.value("recur_max").toInt());
    _id             = recurq.value("recur_id").toInt();
    _prevParentId   = _parentId;
    _prevParentType = _parentType;
    return true;
  }
  else if (recurq.lastError().type() != QSqlError::NoError)
    QMessageBox::warning(this, tr("Database Error"),
                         recurq.lastError().text());
  else
  {
    clear();
    _parentId       = pid;
    _parentType     = ptype;
  }

  return false;
}

void RecurrenceWidget::setPeriod(RecurrencePeriod p)
{
  _period->setId((int)p);
}

void RecurrenceWidget::setPeriod(QString p)
{
  _period->setCode(p);
}

void RecurrenceWidget::setRecurring(bool p)
{
  _recurring->setChecked(p);
}

void RecurrenceWidget::setStartDate(QDate p)
{
  _dates->setStartDate(p.isValid() ? p : QDate::currentDate());
  _startTime->setTime(QTime(0, 0));
}

void RecurrenceWidget::setStartDateTime(QDateTime p)
{
  if (p.isValid())
  {
    _dates->setStartDate(p.date());
    _startTime->setTime(p.time());
  }
  else
    setStartDate(QDate::currentDate());
}

void RecurrenceWidget::setStartDateVisible(bool p)
{
  _dates->setStartVisible(p);
}

void RecurrenceWidget::setStartTime(QTime p)
{
  _startTime->setTime(p);
}

void RecurrenceWidget::setStartTimeVisible(bool p)
{
  _startTime->setVisible(p);
}

QDate RecurrenceWidget::startDate() const
{
  return _dates->startDate();
}

QDateTime RecurrenceWidget::startDateTime() const
{
  return QDateTime(_dates->startDate(), _startTime->time());
}

bool RecurrenceWidget::startDateVisible() const
{
  return _dates->startVisible();
}

QTime RecurrenceWidget::startTime() const
{
  return _startTime->time();
}

bool RecurrenceWidget::startTimeVisible() const
{
  return _startTime->isVisible();
}

// scripting exposure /////////////////////////////////////////////////////////

QScriptValue RecurrenceWidgettoScriptValue(QScriptEngine *engine, RecurrenceWidget* const &item)
{
  return engine->newQObject(item);
}

void RecurrenceWidgetfromScriptValue(const QScriptValue &obj, RecurrenceWidget* &item)
{
  item = qobject_cast<RecurrenceWidget*>(obj.toQObject());
}

QScriptValue constructRecurrenceWidget(QScriptContext *context,
                                       QScriptEngine  *engine)
{
  QWidget *parent = (qscriptvalue_cast<QWidget*>(context->argument(0)));
  const char *objname = "_recurrenceWidget";
  if (context->argumentCount() > 1)
    objname = context->argument(1).toString().toAscii().data();
  return engine->toScriptValue(new RecurrenceWidget(parent, objname));
}

void setupRecurrenceWidget(QScriptEngine *engine)
{
  QScriptValue::PropertyFlags stdflags = QScriptValue::ReadOnly |
                                         QScriptValue::Undeletable;

  qScriptRegisterMetaType(engine, RecurrenceWidgettoScriptValue,
                          RecurrenceWidgetfromScriptValue);

  QScriptValue constructor = engine->newFunction(constructRecurrenceWidget);
  engine->globalObject().setProperty("RecurrenceWidget", constructor, stdflags);

  constructor.setProperty("Never",   QScriptValue(engine, RecurrenceWidget::Never), stdflags);
  constructor.setProperty("Minutely",QScriptValue(engine, RecurrenceWidget::Minutely), stdflags);
  constructor.setProperty("Hourly",  QScriptValue(engine, RecurrenceWidget::Hourly), stdflags);
  constructor.setProperty("Daily",   QScriptValue(engine, RecurrenceWidget::Daily), stdflags);
  constructor.setProperty("Weekly",  QScriptValue(engine, RecurrenceWidget::Weekly), stdflags);
  constructor.setProperty("Monthly", QScriptValue(engine, RecurrenceWidget::Monthly), stdflags);
  constructor.setProperty("Yearly",  QScriptValue(engine, RecurrenceWidget::Yearly), stdflags);
  constructor.setProperty("Custom",  QScriptValue(engine, RecurrenceWidget::Custom), stdflags);

  constructor.setProperty("NoPolicy",     QScriptValue(engine, RecurrenceWidget::NoPolicy), stdflags);
  constructor.setProperty("IgnoreFuture", QScriptValue(engine, RecurrenceWidget::IgnoreFuture), stdflags);
  constructor.setProperty("ChangeFuture", QScriptValue(engine, RecurrenceWidget::ChangeFuture), stdflags);
}
