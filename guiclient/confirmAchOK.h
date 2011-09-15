/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef CONFIRMACHOK_H
#define CONFIRMACHOK_H

#include "xdialog.h"
#include <QFile>
#include "ui_confirmAchOK.h"

class confirmAchOK : public XDialog, public Ui::confirmAchOK
{
    Q_OBJECT

public:
    confirmAchOK(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = 0);
    ~confirmAchOK();

    static bool askOK(QWidget *parent, QFile &);

protected slots:
    virtual void languageChange();

};

#endif // CONFIRMACHOK_H
