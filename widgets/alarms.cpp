/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include <QDesktopServices>
#include <QDialog>

#include <parameter.h>
#include <xsqlquery.h>

#include "alarmMaint.h"
#include "alarms.h"


// CAUTION: This will break if the order of this list does not match
//          the order of the enumerated values as defined.
const Alarms::AlarmMap Alarms::_alarmMap[] =
{
  AlarmMap( Uninitialized,     " "   ),
  AlarmMap( Address,           "ADDR"),
  AlarmMap( BBOMHead,          "BBH" ),
  AlarmMap( BBOMItem,          "BBI" ),
  AlarmMap( BOMHead,           "BMH" ),
  AlarmMap( BOMItem,           "BMI" ),
  AlarmMap( BOOHead,           "BOH" ),
  AlarmMap( BOOItem,           "BOI" ),
  AlarmMap( CRMAccount,        "CRMA"),
  AlarmMap( Contact,           "T"   ),
  AlarmMap( Customer,          "C"   ),
  AlarmMap( Employee,          "EMP" ),
  AlarmMap( Incident,          "INCDT"),
  AlarmMap( Item,              "I"   ),
  AlarmMap( ItemSite,          "IS"  ),
  AlarmMap( ItemSource,        "IR"  ),
  AlarmMap( Location,          "L"   ),
  AlarmMap( LotSerial,         "LS"   ),
  AlarmMap( Opportunity,       "OPP" ),
  AlarmMap( Project,           "J"   ),
  AlarmMap( PurchaseOrder,     "P"   ),
  AlarmMap( PurchaseOrderItem, "PI"  ),
  AlarmMap( ReturnAuth,        "RA"  ),
  AlarmMap( ReturnAuthItem,    "RI"  ),
  AlarmMap( Quote,             "Q"   ),
  AlarmMap( QuoteItem,         "QI"  ),
  AlarmMap( SalesOrder,        "S"   ),
  AlarmMap( SalesOrderItem,    "SI"  ),
  AlarmMap( TodoItem,          "TODO" ),
  AlarmMap( TransferOrder,     "TO"  ),
  AlarmMap( TransferOrderItem, "TI"  ),
  AlarmMap( Vendor,            "V"   ),
  AlarmMap( Warehouse,         "WH"  ),
  AlarmMap( WorkOrder,         "W"   ),
};

Alarms::Alarms(QWidget *pParent) :
  QWidget(pParent)
{
  setupUi(this);
  
  _source = Uninitialized;
  _sourceid = -1;
  _usrId1 = -1;
  _usrId2 = -1;
  _usrId3 = -1;
  _cntctId1 = -1;
  _cntctId2 = -1;
  _cntctId3 = -1;
  XSqlQuery tickle;
  if(_x_metrics)
  {
    tickle.exec( "SELECT CURRENT_TIME AS dbdate;" );
    if (tickle.first())
    {
      _dueDate = tickle.value("dbdate").toDate();
      _dueTime = tickle.value("dbdate").toTime();
    }
  }


  _alarms->addColumn(tr("Qualifier"),        _itemColumn,   Qt::AlignLeft, true, "f_offset" );
  _alarms->addColumn(tr("Due"),              -1,            Qt::AlignLeft, true, "alarm_time" );
  _alarms->addColumn(tr("Event Recipient"),  _itemColumn,   Qt::AlignLeft, true, "alarm_event_recipient" );
  _alarms->addColumn(tr("SysMsg Recipient"), _itemColumn,   Qt::AlignLeft, true, "alarm_sysmsg_recipient" );
  if (_x_metrics)
  {
    if (_x_metrics->boolean("EnableBatchManager"))
      _alarms->addColumn(tr("Email Recipient"),  _itemColumn,   Qt::AlignLeft, true, "alarm_email_recipient" );
  }

  connect(_new, SIGNAL(clicked()), this, SLOT(sNew()));
  connect(_edit, SIGNAL(clicked()), this, SLOT(sEdit()));
  connect(_view, SIGNAL(clicked()), this, SLOT(sView()));
  connect(_delete, SIGNAL(clicked()), this, SLOT(sDelete()));
}

void Alarms::setType(enum AlarmSources pSource)
{
  _source = pSource;
}

void Alarms::setId(int pSourceid)
{
  _sourceid = pSourceid;
  refresh();
}

void Alarms::setUsrId1(int pUsrId)
{
  _usrId1 = pUsrId;
}

void Alarms::setUsrId2(int pUsrId)
{
  _usrId2 = pUsrId;
}

void Alarms::setUsrId3(int pUsrId)
{
  _usrId3 = pUsrId;
}

void Alarms::setCntctId1(int pCntctId)
{
  _cntctId1 = pCntctId;
}

void Alarms::setCntctId2(int pCntctId)
{
  _cntctId2 = pCntctId;
}

void Alarms::setCntctId3(int pCntctId)
{
  _cntctId3 = pCntctId;
}

void Alarms::setDate(QDate pDate)
{
  _dueDate = pDate;

  XSqlQuery q;
  
  q.prepare("SELECT saveAlarm(alarm_id, alarm_number,"
            "                 :alarm_date, LOCALTIME, alarm_time_offset, alarm_time_qualifier,"
            "                 alarm_event, alarm_event_recipient,"
            "                 alarm_email, alarm_email_recipient,"
            "                 alarm_sysmsg, alarm_sysmsg_recipient,"
            "                 alarm_source, alarm_source_id, 'CHANGEALL') AS result "
            "FROM alarm "
            "WHERE ( (alarm_source=:alarm_source)"
            "  AND   (alarm_source_id=:alarm_source_id) ); ");

  q.bindValue(":alarm_date", _dueDate);
  q.bindValue(":alarm_source", Alarms::_alarmMap[_source].ident);
  q.bindValue(":alarm_source_id", _sourceid);
  q.exec();
  
  refresh();
}

void Alarms::setReadOnly(bool pReadOnly)
{
  _new->setEnabled(!pReadOnly);
  _edit->setEnabled(!pReadOnly);
  _delete->setEnabled(!pReadOnly);
  
}

void Alarms::sNew()
{
  ParameterList params;
  params.append("mode", "new");
  params.append("source", _source);
  params.append("source_id", _sourceid);
  params.append("due_date", _dueDate);
  params.append("usrId1", _usrId1);
  params.append("usrId2", _usrId2);
  params.append("usrId3", _usrId3);
  params.append("cntctId1", _cntctId1);
  params.append("cntctId2", _cntctId2);
  params.append("cntctId3", _cntctId3);

  alarmMaint newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != QDialog::Rejected)
    refresh();
}

void Alarms::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("alarm_id", _alarms->id());

  alarmMaint newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != QDialog::Rejected)
    refresh();
}

void Alarms::sView()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("alarm_id", _alarms->id());

  alarmMaint newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void Alarms::sDelete()
{
  XSqlQuery q;
  q.prepare( "DELETE FROM alarm "
             "WHERE (alarm_id=:alarm_id);" );
  q.bindValue(":alarm_id", _alarms->id());
  q.exec();

  refresh();
}

void Alarms::refresh()
{
  if(-1 == _sourceid)
  {
    _alarms->clear();
    return;
  }

  XSqlQuery query;
  
  //Populate alarms
  QString sql( "SELECT alarm_id,"
               "       CASE WHEN (alarm_time_offset > 0) THEN"
               "                 CAST(alarm_time_offset AS TEXT) || ' ' || "
               "                 CASE WHEN (alarm_time_qualifier = 'MB') THEN :minutesbefore"
               "                      WHEN (alarm_time_qualifier = 'HB') THEN :hoursbefore"
               "                      WHEN (alarm_time_qualifier = 'DB') THEN :daysbefore"
               "                      WHEN (alarm_time_qualifier = 'MA') THEN :minutesafter"
               "                      WHEN (alarm_time_qualifier = 'HA') THEN :hoursafter"
               "                      WHEN (alarm_time_qualifier = 'DA') THEN :daysafter"
               "                 END"
               "            ELSE '' "
               "       END AS f_offset, alarm_time,"
               "       CASE WHEN (alarm_event) THEN alarm_event_recipient END AS alarm_event_recipient,"
               "       CASE WHEN (alarm_email) THEN alarm_email_recipient END AS alarm_email_recipient,"
               "       CASE WHEN (alarm_sysmsg) THEN alarm_sysmsg_recipient END AS alarm_sysmsg_recipient "
               "FROM alarm "
               "WHERE ( (alarm_source=:source) "
               "  AND   (alarm_source_id=:sourceid) ) "
               "ORDER BY alarm_time; ");
  query.prepare(sql);
  query.bindValue(":email", tr("Email"));
  query.bindValue(":event", tr("Event"));
  query.bindValue(":sysmsg", tr("System Message"));
  query.bindValue(":other", tr("Other"));
  query.bindValue(":minutesbefore", tr("minutes before"));
  query.bindValue(":hoursbefore", tr("hours before"));
  query.bindValue(":daysbefore", tr("days before"));
  query.bindValue(":minutesafter", tr("mintues after"));
  query.bindValue(":hoursafter", tr("hours after"));
  query.bindValue(":daysafter", tr("days after"));
  query.bindValue(":source", _alarmMap[_source].ident);
  query.bindValue(":sourceid", _sourceid);
  query.exec();
  _alarms->populate(query);
}


