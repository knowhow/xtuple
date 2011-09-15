/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef DUPLICATEACCOUNTNUMBERS_H
#define DUPLICATEACCOUNTNUMBERS_H

#include "guiclient.h"
#include "xwidget.h"

#include "ui_duplicateAccountNumbers.h"

class duplicateAccountNumbers : public XWidget, public Ui::duplicateAccountNumbers
{
    Q_OBJECT

public:
    duplicateAccountNumbers(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = Qt::Window);
    ~duplicateAccountNumbers();

public slots:
    virtual void sBuildList();
    virtual void sDuplicate();

protected slots:
    virtual void languageChange();

    virtual void sFillList();


};

#endif // DUPLICATEACCOUNTNUMBERS_H
