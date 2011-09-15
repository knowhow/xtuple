/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "systemMessage.h"

#include <QVariant>
#include <QMessageBox>

#define cAcknowledge 0x80

systemMessage::systemMessage(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
  : XDialog(parent, name, modal, fl)
{
  setupUi(this);


  // signals and slots connections
  connect(_save, SIGNAL(clicked()), this, SLOT(sSave()));
  connect(_close, SIGNAL(clicked()), this, SLOT(sClose()));

  _scheduledDate->setNullString(tr("ASAP"));
  _scheduledDate->setNullDate(omfgThis->startOfTime());
  _scheduledDate->setAllowNullDate(TRUE);

  _expiresDate->setNullString(tr("Never"));
  _expiresDate->setNullDate(omfgThis->endOfTime());
  _expiresDate->setAllowNullDate(TRUE);
  _expiresDate->setNull();
}

systemMessage::~systemMessage()
{
  // no need to delete child widgets, Qt does it all for us
}

void systemMessage::languageChange()
{
  retranslateUi(this);
}

enum SetResponse systemMessage::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("msguser_id", &valid);
  if (valid)
  {
    _msguserid = param.toInt();
    populate();
  }

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      _mode = cNew;
      _scheduledDate->setFocus();
    }
    else if (param.toString() == "acknowledge")
    {
      _mode = cAcknowledge;

      _scheduledDate->setEnabled(FALSE);
      _scheduledTime->setEnabled(FALSE);
      _expiresDate->setEnabled(FALSE);
      _expiresTime->setEnabled(FALSE);
      _message->setReadOnly(TRUE);
      _close->setText("&Acknowlege");
      _save->setText("&Snooze");

      _close->setFocus();
    }
  }

  return NoError;
}

void systemMessage::sClose()
{
  if (_mode == cNew)
    reject();

  else if (_mode == cAcknowledge)
  {
    q.prepare( "SELECT acknowledgeMessage(msguser_msg_id) "
               "FROM msguser "
               "WHERE (msguser_id=:msguser_id);" );
    q.bindValue(":msguser_id", _msguserid);
    q.exec();

    accept();
  }
}

void systemMessage::sSave()
{
  if (_mode == cNew)
  {
    if (!_scheduledDate->isValid())
    {
      QMessageBox::warning( this, tr("Enter Valid Schedule Date"),
                            tr("You must enter a valid Schedule Date before saving this Message.") );
      _scheduledDate->setFocus();
    }
    else
    {
      q.prepare("SELECT postMessage(:scheduled, :expires, :message) AS msgid;");

      QDateTime scheduled;
      QDateTime expires;
      scheduled.setDate(_scheduledDate->date());
      scheduled.setTime(_scheduledTime->time());
      expires.setDate(_expiresDate->date());
      expires.setTime(_expiresTime->time());

      q.bindValue(":scheduled", scheduled);
      q.bindValue(":expires", expires);
      q.bindValue(":message", (_message->toPlainText()));
      q.exec();
      if (q.first())
        done(q.value("msgid").toInt());
      else
        reject();
    }
  }
  else if (_mode == cAcknowledge)
  {
    q.prepare( "SELECT snoozeMessage(msguser_msg_id) "
               "FROM msguser "
               "WHERE (msguser_id=:msguser_id);" );
    q.bindValue(":msguser_id", _msguserid);
    q.exec();

    accept();
  }
}

void systemMessage::populate()
{
  QString sql = QString( "SELECT DATE(msg_posted) AS postdate,"
                         "       msg_posted::TIME AS posttime,"
                         "       DATE(msg_scheduled) AS scheduleddate,"
                         "       msg_scheduled::TIME AS scheduledtime,"
                         "       DATE(msg_expires) AS expiresdate,"
                         "       msg_expires::TIME AS expirestime,"
                         "       msg_username, msg_text "
                         "FROM msg, msguser "
                         "WHERE ( (msguser_msg_id=msg_id)"
                         " AND (msguser_id=%1) );" )
                .arg(_msguserid);
  q.exec(sql);
  if (q.first())
  {
    _postedDate->setDate(q.value("postdate").toDate());
    _postedTime->setTime(q.value("posttime").toTime());
    _scheduledDate->setDate(q.value("scheduleddate").toDate());
    _scheduledTime->setTime(q.value("scheduledtime").toTime());
    _expiresDate->setDate(q.value("expiresdate").toDate());

    if (q.value("expiresdate") != tr("Never"))
      _expiresTime->setTime(q.value("expirestime").toTime());

    _user->setText(q.value("msg_username").toString());
    _message->setText(q.value("msg_text").toString());
  }
}

