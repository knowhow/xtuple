/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef GLSERIES_H
#define GLSERIES_H

#include <QPushButton>

#include "guiclient.h"
#include "xdialog.h"
#include <parameter.h>

#include "ui_glSeries.h"

class glSeries : public XDialog, public Ui::glSeries
{
    Q_OBJECT

public:
    glSeries(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WFlags fl = 0);
    ~glSeries();

public slots:
    virtual void reject();
    virtual enum SetResponse set(const ParameterList & pParams );
    virtual void sNew();
    virtual void sEdit();
    virtual void sDelete();
    virtual void sPost();
    virtual void sFillList();
    virtual void sSave();

protected slots:
    virtual void languageChange();

private:
    int _mode;
    int _glsequence;
    int _journal;
    bool update();
    bool _submit;
    QPushButton* _post;

};

#endif // GLSERIES_H
