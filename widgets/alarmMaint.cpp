/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include <QFileDialog>
#include <QUrl>
#include <QVariant>
#include <QMessageBox>

#include "alarmMaint.h"
#include "shortcuts.h"

const char *_alarmQualifiers[] = { "MB", "HB", "DB", "MA", "HA", "DA" };

alarmMaint::alarmMaint(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
  : QDialog(parent, fl)
{
  setupUi(this);

  setObjectName(name ? name : "alarmMaint");
  setModal(modal);

  _userLookup = _buttonBox->addButton(tr("&User..."), QDialogButtonBox::ActionRole);
  _contactLookup = _buttonBox->addButton(tr("&Contact..."), QDialogButtonBox::ActionRole);

  // signals and slots connections
  connect(_buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
  connect(_buttonBox, SIGNAL(accepted()), this, SLOT(sSave()));
  connect(_userLookup, SIGNAL(clicked()), _usernameCluster, SLOT(sList()));
  connect(_contactLookup, SIGNAL(clicked()), _contactCluster, SLOT(sList()));
  connect(_contactCluster, SIGNAL(newId(int)), this, SLOT(sContactLookup(int)));
  connect(_usernameCluster, SIGNAL(newId(int)), this, SLOT(sUserLookup(int)));

  _mode = cNew;
  _source = Alarms::Uninitialized;
  _sourceid = -1;
  _alarmid = -1;
  XSqlQuery tickle;
  tickle.exec( "SELECT CURRENT_TIME AS dbdate;" );
  if (tickle.first())
  {
    _alarmDate->setDate(tickle.value("dbdate").toDate());
    _alarmTime->setTime(tickle.value("dbdate").toTime());
  }
  
  _eventAlarm->setChecked(_x_preferences && _x_preferences->boolean("AlarmEventDefault"));
  if (_x_metrics)
  {
    if (_x_metrics->boolean("EnableBatchManager"))
      _emailAlarm->setChecked(_x_preferences && _x_preferences->boolean("AlarmEmailDefault"));
    else
    {
      _emailAlarm->hide();
      _emailRecipient->hide();
    }
  }
  _sysmsgAlarm->setChecked(_x_preferences && _x_preferences->boolean("AlarmSysmsgDefault"));

  _contactCluster->hide();
  _usernameCluster->hide();
  adjustSize();

  shortcuts::setStandardKeys(this);
}

alarmMaint::~alarmMaint()
{
  // no need to delete child widgets, Qt does it all for us
}

void alarmMaint::languageChange()
{
  retranslateUi(this);
}

void alarmMaint::set( const ParameterList & pParams )
{
  QVariant param;
  bool        valid;
  
  param = pParams.value("source", &valid);
  if (valid)
  {
    _source = (enum Alarms::AlarmSources)param.toInt();
    if ( (QString(Alarms::_alarmMap[_source].ident) == QString("TODO")) || (QString(Alarms::_alarmMap[_source].ident) == QString("J")) )
      _alarmDate->setEnabled(false);
  }
    
  param = pParams.value("source_id", &valid);
  if(valid)
    _sourceid = param.toInt();

  param = pParams.value("due_date", &valid);
  if(valid)
    _alarmDate->setDate(param.toDate());

  param = pParams.value("usrId1", &valid);
  if(valid)
    sGetUser(param.toInt());

  param = pParams.value("usrId2", &valid);
  if(valid)
    sGetUser(param.toInt());

  param = pParams.value("usrId3", &valid);
  if(valid)
    sGetUser(param.toInt());

  param = pParams.value("cntctId1", &valid);
  if(valid)
    sGetContact(param.toInt());

  param = pParams.value("cntctId2", &valid);
  if(valid)
    sGetContact(param.toInt());

  param = pParams.value("cntctId3", &valid);
  if(valid)
    sGetContact(param.toInt());

  param = pParams.value("mode", &valid);
  if(valid)
  {
    if(param.toString() == "new")
    {
      _mode = cNew;
    }
    else if(param.toString() == "edit")
      _mode = cEdit;
    else if(param.toString() == "view")
    {
      _mode = cView;
      _buttonBox->setStandardButtons(QDialogButtonBox::Cancel);
    }
  }

  param = pParams.value("alarm_id", &valid);
  if (valid)
  {
    _alarmid = param.toInt();
    sPopulate();
  }

}

void alarmMaint::sSave()
{
  XSqlQuery q;

  q.prepare("SELECT saveAlarm(:alarm_id, :alarm_number,"
            "                 :alarm_date, :alarm_time, :alarm_time_offset, :alarm_time_qualifier,"
            "                 :alarm_event, :alarm_event_recipient,"
            "                 :alarm_email, :alarm_email_recipient,"
            "                 :alarm_sysmsg, :alarm_sysmsg_recipient,"
            "                 :alarm_source, :alarm_source_id, 'CHANGEALL') AS result; ");

  q.bindValue(":alarm_event", QVariant(_eventAlarm->isChecked()));
  q.bindValue(":alarm_email", QVariant(_emailAlarm->isChecked()));
  q.bindValue(":alarm_sysmsg", QVariant(_sysmsgAlarm->isChecked()));
  q.bindValue(":alarm_event_recipient", _eventRecipient->text());
  q.bindValue(":alarm_email_recipient", _emailRecipient->text());
  q.bindValue(":alarm_sysmsg_recipient", _sysmsgRecipient->text());
  q.bindValue(":alarm_date", _alarmDate->date());
  q.bindValue(":alarm_time", _alarmTime->time());
  q.bindValue(":alarm_time_offset", _alarmOffset->value());
  q.bindValue(":alarm_time_qualifier", _alarmQualifiers[_alarmQualifier->currentIndex()]);
  q.bindValue(":alarm_source", Alarms::_alarmMap[_source].ident);
  q.bindValue(":alarm_source_id", _sourceid);
  q.bindValue(":alarm_id", _alarmid);
  q.exec();

  accept();
}

void alarmMaint::sUserLookup(int pId)
{
  sGetUser(pId);
}

void alarmMaint::sContactLookup(int /*pId*/)
{
  QString recipient;
  recipient = _emailRecipient->text();
  if (recipient.length() == 0)
    recipient = _contactCluster->emailAddress();
  else if (recipient != _contactCluster->emailAddress())
  {
    recipient += ",";
    recipient += _contactCluster->emailAddress();
  }
  _emailRecipient->setText(recipient);
}

void alarmMaint::sGetUser(int pUsrId)
{
  XSqlQuery q;
  q.prepare( "SELECT usr.* "
             "FROM usr "
             "WHERE (usr_id=:usr_id);" );
  q.bindValue(":usr_id", pUsrId);
  q.exec();
  if (q.first())
  {
    bool recipientFound = false;
    QString recipient;
    recipient = _eventRecipient->text();
    for (int pcounter = 0; pcounter < 10; pcounter++)
    {
      if (recipient.section(",", pcounter, pcounter) == q.value("usr_username").toString())
        recipientFound = true;
    }
      
    if (recipient.length() == 0)
      recipient = q.value("usr_username").toString();
    else if (!recipientFound)
    {
      recipient += ",";
      recipient += q.value("usr_username").toString();
    }
    _eventRecipient->setText(recipient);
    _sysmsgRecipient->setText(recipient);

    recipientFound = false;
    recipient = _emailRecipient->text();
    for (int pcounter = 0; pcounter < 10; pcounter++)
    {
      if (recipient.section(",", pcounter, pcounter) == q.value("usr_email").toString())
        recipientFound = true;
    }
      
    if (recipient.length() == 0)
      recipient = q.value("usr_email").toString();
    else if (!recipientFound)
    {
      recipient += ",";
      recipient += q.value("usr_email").toString();
    }
    _emailRecipient->setText(recipient);
  }
}

void alarmMaint::sGetContact(int pCntctId)
{
  XSqlQuery q;
  q.prepare( "SELECT cntct.* "
             "FROM cntct "
             "WHERE (cntct_id=:cntct_id);" );
  q.bindValue(":cntct_id", pCntctId);
  q.exec();
  if (q.first())
  {
    QString recipient;
    recipient = _emailRecipient->text();
    if (recipient.length() == 0)
      recipient = q.value("cntct_email").toString();
    else if (recipient != q.value("cntct_email").toString())
    {
      recipient += ",";
      recipient += q.value("cntct_email").toString();
    }
    _emailRecipient->setText(recipient);
  }
}

void alarmMaint::sPopulate()
{
  XSqlQuery q;
  
  q.prepare( "SELECT * "
             "FROM alarm "
             "WHERE (alarm_id=:alarm_id);" );
  q.bindValue(":alarm_id", _alarmid);
  q.exec();
  if (q.first())
  {
    _alarmOffset->setValue(q.value("alarm_time_offset").toInt());
    for (int pcounter = 0; pcounter < _alarmQualifier->count(); pcounter++)
    {
      if (QString(q.value("alarm_time_qualifier").toString()) == _alarmQualifiers[pcounter])
        _alarmQualifier->setCurrentIndex(pcounter);
    }
    _alarmDate->setDate(q.value("alarm_time").toDate());
    _alarmTime->setTime(q.value("alarm_time").toTime());
    _eventAlarm->setChecked(q.value("alarm_event").toBool());
    _emailAlarm->setChecked(q.value("alarm_email").toBool());
    _sysmsgAlarm->setChecked(q.value("alarm_sysmsg").toBool());
    _eventRecipient->setText(q.value("alarm_event_recipient").toString());
    _emailRecipient->setText(q.value("alarm_email_recipient").toString());
    _sysmsgRecipient->setText(q.value("alarm_sysmsg_recipient").toString());
    _source = (enum Alarms::AlarmSources)q.value("alarm_source").toInt();
    _sourceid = q.value("alarm_source_id").toInt();
  }
}
