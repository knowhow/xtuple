/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef XDATEINPUTDIALOG_H
#define XDATEINPUTDIALOG_H

#include "guiclient.h"
#include <QDate>
#include "xdialog.h"
#include <parameter.h>

#include "ui_xdateinputdialog.h"

class XDateInputDialog : public XDialog, public Ui::XDateInputDialog
{
    Q_OBJECT

public:
    XDateInputDialog(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WFlags fl = 0);
    ~XDateInputDialog();
    QDate getDate();

public slots:
    virtual enum SetResponse set(const ParameterList & pParams );

protected slots:
    virtual void languageChange();

};

#endif // XDATEINPUTDIALOG_H
