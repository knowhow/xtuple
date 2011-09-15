/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef SUMMARIZEINVTRANSBYCLASSCODE_H
#define SUMMARIZEINVTRANSBYCLASSCODE_H

#include "guiclient.h"
#include "xdialog.h"
#include "ui_summarizeInvTransByClassCode.h"

class summarizeInvTransByClassCode : public XDialog, public Ui::summarizeInvTransByClassCode
{
    Q_OBJECT

public:
    summarizeInvTransByClassCode(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WFlags fl = 0);
    ~summarizeInvTransByClassCode();

public slots:
    virtual void sSummarize();

protected slots:
    virtual void languageChange();

};

#endif // SUMMARIZEINVTRANSBYCLASSCODE_H
