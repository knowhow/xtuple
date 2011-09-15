/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef PRINTLABELSBYTO_H
#define PRINTLABELSBYTO_H

#include <QPushButton>
#include "xdialog.h"
#include "ui_printLabelsByTo.h"

class printLabelsByTo : public XDialog, public Ui::printLabelsByTo
{
    Q_OBJECT

public:
    printLabelsByTo(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WFlags fl = 0);
    ~printLabelsByTo();

public slots:
    virtual void sPrint();
    virtual void sSetToMin( int pValue );

protected slots:
    virtual void languageChange();

private:
    QPushButton* _print;

};

#endif // PRINTLABELSBYTO_H
