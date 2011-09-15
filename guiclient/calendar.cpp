/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "calendar.h"

#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include "absoluteCalendarItem.h"
#include "relativeCalendarItem.h"

static const char *originTypes[] = { "D", "E", "W", "X", "M", "N", "L", "Y", "Z" };

calendar::calendar(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_buttonBox, SIGNAL(accepted()), this, SLOT(sSave()));
  connect(_edit, SIGNAL(clicked()), this, SLOT(sEdit()));
  connect(_delete, SIGNAL(clicked()), this, SLOT(sDelete()));
  connect(_new, SIGNAL(clicked()), this, SLOT(sNew()));
}

calendar::~calendar()
{
  // no need to delete child widgets, Qt does it all for us
}

void calendar::languageChange()
{
  retranslateUi(this);
}

enum SetResponse calendar::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("calhead_id", &valid);
  if (valid)
  {
    _calheadid = param.toInt();
    populate();
  }

  param = pParams.value("type", &valid);
  if (valid)
  {
    if (param.toString() == "absolute")
    {
      _type = 'A';
      _absolute->setChecked(TRUE);
      _relative->setEnabled(FALSE);
      _absolute->setEnabled(FALSE);
    }
    else if (param.toString() == "relative")
    {
      _type = 'R';
      _relative->setChecked(TRUE);
      _relative->setEnabled(FALSE);
      _absolute->setEnabled(FALSE);
    }
  }

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      _mode = cNew;
      _name->setFocus();

      q.exec("SELECT NEXTVAL('calhead_calhead_id_seq') AS _calhead_id;");
      if (q.first())
        _calheadid = q.value("_calhead_id").toInt();
      else if (q.lastError().type() != QSqlError::NoError)
      {
        systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
        return UndefinedError;
      }
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;

      _relative->setEnabled(FALSE);
      _absolute->setEnabled(FALSE);

      _descrip->setFocus();
    }
  }

  return NoError;
}

void calendar::sSave()
{
  if(_name->text().length() == 0)
  {
    QMessageBox::critical(this, tr("Calendar Name Required"),
      tr("You must enter a Calendar Name to continue.") );
    _name->setFocus();
    return;
  }

  q.prepare("SELECT calhead_id"
            "  FROM calhead"
            " WHERE((calhead_id != :calhead_id)"
            "   AND (calhead_name=:calhead_name))");
  q.bindValue(":calhead_id",       _calheadid);
  q.bindValue(":calhead_name",   _name->text());
  q.exec();
  if(q.first())
  {
    QMessageBox::critical(this, tr("Duplicate Calendar Name"),
      tr("A Calendar already exists for the Name specified.") );
    _name->setFocus();
    return;
  }

  if (_mode == cNew)
  {
    if (_type =='R')
      q.prepare( "INSERT INTO calhead "
                 "(calhead_id, calhead_name, calhead_descrip, calhead_type, calhead_origin) "
                 "VALUES "
                 "(:calhead_id, :calhead_name, :calhead_descrip, 'R', :calhead_origin);" );
    else
      q.prepare( "INSERT INTO calhead "
                 "(calhead_id, calhead_name, calhead_descrip, calhead_type) "
                 "VALUES "
                 "(:calhead_id, :calhead_name, :calhead_descrip, 'A');" );
  }
  else if (_mode == cEdit)
  {
    if (_type =='R')
      q.prepare( "UPDATE calhead "
                 "SET calhead_name=:calhead_name, calhead_descrip=:calhead_descrip, calhead_origin=:calhead_origin "
                 "WHERE (calhead_id=:calhead_id);" );
    else
      q.prepare( "UPDATE calhead "
                 "SET calhead_name=:calhead_name, calhead_descrip=:calhead_descrip "
                 "WHERE (calhead_id=:calhead_id);" );
  }

  q.bindValue(":calhead_id", _calheadid);
  q.bindValue(":calhead_name", _name->text());
  q.bindValue(":calhead_descrip", _descrip->text());
  q.bindValue(":calhead_origin", originTypes[_origin->currentIndex()]);
  q.exec();
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  done(_calheadid);
}

void calendar::sNew()
{
  ParameterList params;
  params.append("mode", "new");
  params.append("calhead_id", _calheadid);

  if (_mode == cNew)
    params.append("calendarName", _name->text());

  if (_type == 'A')
  {
    absoluteCalendarItem newdlg(this, "", TRUE);
    newdlg.set(params);

    if (newdlg.exec() != XDialog::Rejected)
      sFillList();
  }
  else if (_type == 'R')
  {
    relativeCalendarItem newdlg(this, "", TRUE);
    newdlg.set(params);

    if (newdlg.exec() != XDialog::Rejected)
      sFillList();
  }
}

void calendar::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("calitem_id", _calitem->id());

  if (_type == 'A')
  {
    absoluteCalendarItem newdlg(this, "", TRUE);
    newdlg.set(params);

    if (newdlg.exec() != XDialog::Rejected)
      sFillList();
  }
  else if (_type == 'R')
  {
    relativeCalendarItem newdlg(this, "", TRUE);
    newdlg.set(params);

    if (newdlg.exec() != XDialog::Rejected)
      sFillList();
  }
}

void calendar::sDelete()
{
  if (_type == 'A')
    q.prepare( "DELETE FROM acalitem "
               "WHERE (acalitem_id=:xcalitem_id);" );
  else if (_type == 'R')
    q.prepare( "DELETE FROM rcalitem "
               "WHERE (rcalitem_id=:xcalitem_id);" );

  q.bindValue(":xcalitem_id", _calitem->id());
  q.exec();
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  sFillList();
}

void calendar::sFillList()
{
  _calitem->setColumnCount(0);
  _calitem->addColumn(tr("Name"), _itemColumn, Qt::AlignLeft, true, "name");
  if (_type == 'A')
  {
    _calitem->addColumn(tr("Start Date"), _dateColumn, Qt::AlignCenter, true, "acalitem_periodstart");
    _calitem->addColumn(tr("Period"),     _qtyColumn,  Qt::AlignRight,  true, "periodlength");
    q.prepare( "SELECT acalitem_id, acalitem_name AS name, acalitem_periodstart,"
               "       (TEXT(acalitem_periodlength) || TEXT(:days)) AS periodlength "
               "FROM acalitem "
               "WHERE (acalitem_calhead_id=:calhead_id) "
               "ORDER BY acalitem_periodstart;" );
    q.bindValue(":days", tr("Day(s)"));
  }
  else if (_type == 'R')
  {
    _calitem->addColumn(tr("Offset"),        _itemColumn, Qt::AlignRight, true, "offsetdays");
    _calitem->addColumn(tr("Period Length"), -1,          Qt::AlignLeft,  true, "periodlength");
    q.prepare( "SELECT rcalitem_id, rcalitem_name AS name,"
               "       CASE WHEN (rcalitem_periodtype='D') THEN (TEXT(rcalitem_periodcount) || :days)"
               "            WHEN (rcalitem_periodtype='B') THEN (TEXT(rcalitem_periodcount) || :businessDays)"
               "            WHEN (rcalitem_periodtype='W') THEN (TEXT(rcalitem_periodcount) || :weeks)"
               "            WHEN (rcalitem_periodtype='M') THEN (TEXT(rcalitem_periodcount) || :months)"
               "            WHEN (rcalitem_periodtype='Q') THEN (TEXT(rcalitem_periodcount) || :quarters)"
               "            WHEN (rcalitem_periodtype='Y') THEN (TEXT(rcalitem_periodcount) || :years)"
               "            ELSE :userDefined"
               "       END AS periodlength,"
               "       CASE WHEN (rcalitem_offsettype='D') THEN (rcalitem_offsetcount)"
               "            WHEN (rcalitem_offsettype='B') THEN (rcalitem_offsetcount)"
               "            WHEN (rcalitem_offsettype='W') THEN (rcalitem_offsetcount * 7)"
               "            WHEN (rcalitem_offsettype='M') THEN (rcalitem_offsetcount * 30)"
               "            WHEN (rcalitem_offsettype='Q') THEN (rcalitem_offsetcount * 120)"
               "            WHEN (rcalitem_offsettype='Y') THEN (rcalitem_offsetcount * 365)"
               "            ELSE (rcalitem_offsetcount)"
               "       END AS offsetdays,"
               "       CASE WHEN (rcalitem_offsettype='D') THEN (TEXT(rcalitem_offsetcount) || :days)"
               "            WHEN (rcalitem_offsettype='B') THEN (TEXT(rcalitem_offsetcount) || :businessDays)"
               "            WHEN (rcalitem_offsettype='W') THEN (TEXT(rcalitem_offsetcount) || :weeks)"
               "            WHEN (rcalitem_offsettype='M') THEN (TEXT(rcalitem_offsetcount) || :months)"
               "            WHEN (rcalitem_offsettype='Q') THEN (TEXT(rcalitem_offsetcount) || :quarters)"
               "            WHEN (rcalitem_offsettype='Y') THEN (TEXT(rcalitem_offsetcount) || :years)"
               "            ELSE :userDefined"
               "       END AS offsetdays_qtdisplayrole "
               "FROM rcalitem "
               "WHERE (rcalitem_calhead_id=:calhead_id) "
               "ORDER BY offsetdays;" );

    q.bindValue(":days", tr("Day(s)"));
    q.bindValue(":businessDays", tr("Business Day(s)"));
    q.bindValue(":weeks", tr("Week(s)"));
    q.bindValue(":months", tr("Month(s)"));
    q.bindValue(":quarters", tr("Quarter(s)"));
    q.bindValue(":years", tr("Year(s)"));
    q.bindValue(":userDefined", tr("User-Defined"));
  }
  
  q.bindValue(":calhead_id", _calheadid);
  q.exec();
  _calitem->populate(q);
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void calendar::populate()
{
  q.prepare( "SELECT calhead_name, calhead_descrip, calhead_type, calhead_origin "
             "FROM calhead "
             "WHERE (calhead_id=:calhead_id);");
  q.bindValue(":calhead_id", _calheadid);
  q.exec();
  if (q.first())
  {
    _name->setText(q.value("calhead_name"));
    _descrip->setText(q.value("calhead_descrip"));

    _type = q.value("calhead_type").toString()[0].toAscii();

    if (_type == 'A')
    {
      _absolute->setChecked(TRUE);
    }
    else if (_type == 'R')
    {
      _relative->setChecked(TRUE);

      for (int counter = 0; counter < _origin->count(); counter++)
        if (q.value("calhead_origin").toString() == originTypes[counter])
        {
          _origin->setCurrentIndex(counter);
          break;
        }
    }

    sFillList();
  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}
