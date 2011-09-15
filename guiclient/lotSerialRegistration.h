/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef LOTSERIALREGISTRATION_H
#define LOTSERIALREGISTRATION_H

#include "guiclient.h"
#include "xdialog.h"
#include "xdatawidgetmapper.h"
#include <QStringList>
#include <QSqlTableModel>
#include <parameter.h>

#include "ui_lotSerialRegistration.h"

class lotSerialRegistration : public XDialog, public Ui::lotSerialRegistration
{
    Q_OBJECT

public:
    lotSerialRegistration(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WFlags fl = 0);
    ~lotSerialRegistration();

public slots:
    virtual SetResponse set( const ParameterList & pParams );
    virtual void closeEvent( QCloseEvent * pEvent );
    virtual void populate();
    virtual void sSave();
    virtual void sNewCharass();
    virtual void sEditCharass();
    virtual void sDeleteCharass();
    virtual void sFillList();
    virtual void sDateUpdated();
    virtual void sSetSoCustId();
    virtual void sSetSoId();

protected slots:
    virtual void languageChange();

private:
    int               _mode;
    int               _lsregid;
    
};

#endif // LOTSERIALREGISTRATION_H
