/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef UPDATEOUTLEVELSBYCLASSCODE_H
#define UPDATEOUTLEVELSBYCLASSCODE_H

#include "guiclient.h"
#include "xdialog.h"
#include "ui_updateOUTLevelsByClassCode.h"

class updateOUTLevelsByClassCode : public XDialog, public Ui::updateOUTLevelsByClassCode
{
    Q_OBJECT

public:
    updateOUTLevelsByClassCode(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WFlags fl = 0);
    ~updateOUTLevelsByClassCode();

public slots:
    virtual void sUpdate();
    virtual void sSubmit();

protected slots:
    virtual void languageChange();

private:
    QButtonGroup* _daysGroupInt;
};

#endif // UPDATEOUTLEVELSBYCLASSCODE_H
