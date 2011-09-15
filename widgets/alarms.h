/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef alarms_h
#define alarms_h

#include <QWidget>
#include <QDateTime>

#include <xsqlquery.h>

#include "ui_alarms.h"

class XTUPLEWIDGETS_EXPORT Alarms : public QWidget, public Ui::alarms
{
  Q_OBJECT

  Q_ENUMS(AlarmSources)

  Q_PROPERTY(AlarmSources type READ type WRITE setType)

  friend class alarm;

  public:
    Alarms(QWidget *);

    // if you add to this then add to the _alarmMap[] below
    enum AlarmSources
    {
      Uninitialized,
      Address,          BBOMHead,           BBOMItem,
      BOMHead,          BOMItem,            BOOHead,
      BOOItem,          CRMAccount,         Contact, 
      Customer,         Employee,           Incident,   
      Item,             ItemSite,           ItemSource,
      Location,		LotSerial,          Opportunity,
      Project,		PurchaseOrder,      PurchaseOrderItem,
      ReturnAuth,       ReturnAuthItem,     Quote, 
      QuoteItem,        SalesOrder,         SalesOrderItem,
      TodoItem,
      TransferOrder,	TransferOrderItem,  Vendor,
      Warehouse,	WorkOrder
    };

    inline int  sourceid()          { return _sourceid; }
    inline enum AlarmSources type() { return _source;   }

    struct AlarmMap
    {
      enum AlarmSources source;
      const char *      ident;

      AlarmMap(enum AlarmSources s, const char * i)
      {
        source = s;
        ident = i;
      }
    };
    static const struct AlarmMap _alarmMap[]; // see Alarms.cpp for init

  public slots:
    void setType(enum AlarmSources);
    void setId(int);
    void setUsrId1(int);
    void setUsrId2(int);
    void setUsrId3(int);
    void setCntctId1(int);
    void setCntctId2(int);
    void setCntctId3(int);
    void setDate(QDate);
    void setReadOnly(bool);

    void sNew();
    void sEdit();
    void sView();
    void sDelete();
    
    void refresh();

  private:
    enum AlarmSources _source;
    int               _sourceid;
    int               _usrId1;
    int               _usrId2;
    int               _usrId3;
    int               _cntctId1;
    int               _cntctId2;
    int               _cntctId3;
    QDate             _dueDate;
    QTime             _dueTime;

};

#endif
