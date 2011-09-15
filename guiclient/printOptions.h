/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef PRINTOPTIONS_H
#define PRINTOPTIONS_H

#include "guiclient.h"
#include "xdialog.h"
#include <parameter.h>

#include "ui_printOptions.h"

class printOptions : public XDialog, public Ui::printOptions
{
    Q_OBJECT

public:
    printOptions(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WFlags fl = 0);
    ~printOptions();

public slots:
    virtual enum SetResponse set(const ParameterList & pParams );
    virtual void sSave();
    virtual void sHandleCheckbox();

protected slots:
    virtual void languageChange();

private:
    QString _parentName;
};

#endif // PRINTOPTIONS_H
