/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef IDLESHUTDOWN_H
#define IDLESHUTDOWN_H

#include "guiclient.h"
#include "xdialog.h"
#include <parameter.h>

#include "ui_idleShutdown.h"

class idleShutdown : public XDialog, public Ui::idleShutdown
{
    Q_OBJECT

public:
    idleShutdown(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WFlags fl = 0);
    ~idleShutdown();

public slots:
    virtual enum SetResponse set( ParameterList & pParams );
    virtual void timerEvent( QTimerEvent * );

protected slots:
    virtual void languageChange();

private:
    int _secsRemaining;
    QString _countMessage;

};

#endif // IDLESHUTDOWN_H
