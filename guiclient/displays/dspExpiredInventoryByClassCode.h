/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef DSPEXPIREDINVENTORYBYCLASSCODE_H
#define DSPEXPIREDINVENTORYBYCLASSCODE_H

#include "display.h"

#include "ui_dspExpiredInventoryByClassCode.h"

class dspExpiredInventoryByClassCode : public display, public Ui::dspExpiredInventoryByClassCode
{
    Q_OBJECT

public:
    dspExpiredInventoryByClassCode(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = Qt::Window);

    virtual bool setParams(ParameterList &);

public slots:
    virtual void sPopulateMenu(QMenu *, QTreeWidgetItem *, int);
    virtual void sTransfer();
    virtual void sAdjust();
    virtual void sReset();
    virtual void sMiscCount();
    virtual void sIssueCountTag();
    virtual void sHandleValue( bool pShowValue );
    virtual void sFillList();

protected slots:
    virtual void languageChange();

private:
    QButtonGroup* _costsGroupInt;

};

#endif // DSPEXPIREDINVENTORYBYCLASSCODE_H
