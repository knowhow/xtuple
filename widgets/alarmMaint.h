/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef ALARMMAINT_H
#define ALARMMAINT_H

#include <parameter.h>
#include <xsqlquery.h>

#include "alarms.h"
#include "widgets.h"
#include "ui_alarmMaint.h"

class alarmMaint : public QDialog, public Ui::alarmMaint
{
    Q_OBJECT

public:
    alarmMaint(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WFlags fl = 0);
    ~alarmMaint();

    QPushButton* _userLookup;
    QPushButton* _contactLookup;

public slots:
    virtual void set( const ParameterList & pParams );
    virtual void sSave();
    virtual void sGetUser(int pUsrId);
    virtual void sGetContact(int pCntctId);
    virtual void sPopulate();
    virtual void sUserLookup(int pId);
    virtual void sContactLookup(int pId);

protected slots:
    virtual void languageChange();

private:
    enum Alarms::AlarmSources _source;
    int _mode;
    int _alarmid;
    int _sourceid;

};

#endif // ALARMMAINT_H
