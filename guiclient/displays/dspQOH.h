/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef DSPQOH_H
#define DSPQOH_H

#include "guiclient.h"
#include "display.h"

#include "ui_dspQOH.h"

class dspQOH : public display, public Ui::dspQOH
{
    Q_OBJECT

public:
    dspQOH(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = Qt::Window);

    virtual bool setParams(ParameterList &);
    virtual SetResponse set( const ParameterList & pParams );

public slots:
    virtual void sPopulateMenu(QMenu * pMenu, QTreeWidgetItem * pSelected, int);
    virtual void sViewDetail();
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
    QButtonGroup* _showGroupInt;
};

#endif // dspQOH_H
