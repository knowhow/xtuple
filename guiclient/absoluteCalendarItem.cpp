/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "absoluteCalendarItem.h"

#include <QVariant>
#include <QMessageBox>

absoluteCalendarItem::absoluteCalendarItem(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
  : XDialog(parent, name, modal, fl)
{
  setupUi(this);


  // signals and slots connections
  connect(_buttonBox, SIGNAL(accepted()), this, SLOT(sSave()));
  connect(_buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
}

absoluteCalendarItem::~absoluteCalendarItem()
{
  // no need to delete child widgets, Qt does it all for us
}

void absoluteCalendarItem::languageChange()
{
  retranslateUi(this);
}

enum SetResponse absoluteCalendarItem::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("calhead_id", &valid);
  if (valid)
    _calheadid = param.toInt();
  else
    _calheadid = -1;

  param = pParams.value("calendarName", &valid);
  if (valid)
    _calendarName->setText(param.toString());
  else if (_calheadid != -1)
  {
    q.prepare( "SELECT calhead_name "
               "FROM calhead "
               "WHERE (calhead_id=:calhead_id);" );
    q.bindValue(":calhead_id", _calheadid);
    q.exec();
    if (q.first())
      _calendarName->setText(q.value("calhead_name").toString());
  }

  param = pParams.value("calitem_id", &valid);
  if (valid)
  {
    _calitemid = param.toInt();
    populate();
  }

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      _mode = cNew;

      _name->setFocus();
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;
    }
  }

  return NoError;
}

void absoluteCalendarItem::sSave()
{
  if (_mode == cNew)
  {
    q.exec("SELECT NEXTVAL('xcalitem_xcalitem_id_seq') AS _calitem_id");
    if (q.first())
      _calitemid = q.value("_calitem_id").toInt();
//  ToDo

    q.prepare( "INSERT INTO acalitem "
               "( acalitem_id, acalitem_calhead_id, acalitem_name,"
               "  acalitem_periodstart, acalitem_periodlength )"
               "VALUES "
               "( :acalitem_id, :acalitem_calhead_id, :acalitem_name,"
               "  :acalitem_periodstart, :acalitem_periodlength )" );
  }
  else
    q.prepare( "UPDATE acalitem "
               "SET acalitem_name=:acalitem_name,"
               "    acalitem_periodstart=:acalitem_periodstart, acalitem_periodlength=:acalitem_periodlength "
               "WHERE (acalitem_id=:acalitem_id);" );

  q.bindValue(":acalitem_id", _calitemid);
  q.bindValue(":acalitem_calhead_id", _calheadid);
  q.bindValue(":acalitem_name", _name->text());
  q.bindValue(":acalitem_periodstart", _startDate->date());
  q.bindValue(":acalitem_periodlength", _periodLength->value());
  q.exec();

  done (_calitemid);
}

void absoluteCalendarItem::populate()
{
  q.prepare( "SELECT calhead_id, calhead_name "
             "FROM calhead, acalitem "
             "WHERE ( (acalitem_calhead_id=calhead_id)"
             " AND (acalitem_id=:acalitem_id));" );
  q.bindValue(":acalitem_id", _calitemid);
  q.exec();
  if (q.first())
  {
    _calheadid = q.value("calhead_id").toInt();
    _calendarName->setText(q.value("calhead_name").toString());
  }

  q.prepare( "SELECT acalitem_name,"
             "       acalitem_periodstart, acalitem_periodlength "
             "FROM acalitem "
             "WHERE (acalitem_id=:acalitem_id);" );
  q.bindValue(":acalitem_id", _calitemid);
  q.exec();
  if (q.first())
  {
    _name->setText(q.value("acalitem_name").toString());
    _startDate->setDate(q.value("acalitem_periodstart").toDate());
    _periodLength->setValue(q.value("acalitem_periodlength").toInt());
  }
//  ToDo
}
