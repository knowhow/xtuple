/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef REVERSEGLSERIES_H
#define REVERSEGLSERIES_H

#include "guiclient.h"
#include "xdialog.h"
#include <parameter.h>
#include "ui_reverseGLSeries.h"

class reverseGLSeries : public XDialog, public Ui::reverseGLSeries
{
    Q_OBJECT

public:
    reverseGLSeries(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WFlags fl = 0);
    ~reverseGLSeries();

public slots:
    virtual SetResponse set( const ParameterList & pParams );
    virtual void sPost();

protected:
    int _glseries;

protected slots:
    virtual void languageChange();

};

#endif // REVERSEGLSERIES_H
