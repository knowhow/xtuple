/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef VIEWCHECKRUN_H
#define VIEWCHECKRUN_H

#include "guiclient.h"
#include "xwidget.h"
#include <parameter.h>
#include "ui_viewCheckRun.h"

class viewCheckRun : public XWidget, public Ui::viewCheckRun
{
    Q_OBJECT

public:
    viewCheckRun(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = Qt::Window);
    ~viewCheckRun();

public slots:
    virtual void sDelete();
    virtual void sEdit();
    virtual void sFillList();
    virtual void sFillList(int);
    virtual void sHandleItemSelection();
    virtual void sNewMiscCheck();
    virtual void sPost();
    virtual void sPostChecks();
    virtual void sPrepareCheckRun();
    virtual void sPrint();
    virtual void sPrintCheckRun();
    virtual void sPrintEditList();
    virtual void sReplace();
    virtual void sReplaceAll();
    virtual void sVoid();

protected slots:
    virtual void languageChange();
    virtual void sHandleVendorGroup();

};

#endif // VIEWCHECKRUN_H
