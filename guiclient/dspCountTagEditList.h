/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */


#ifndef DSPCOUNTTAGEDITLIST_H
#define DSPCOUNTTAGEDITLIST_H

#include "guiclient.h"
#include "xwidget.h"

#include "ui_dspCountTagEditList.h"

class dspCountTagEditList : public XWidget, public Ui::dspCountTagEditList
{
    Q_OBJECT

public:
    dspCountTagEditList(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = Qt::Window);
    ~dspCountTagEditList();

    virtual void setParams(ParameterList &params);

public slots:
    virtual void sPrint();
    virtual void sToggleList();
    virtual void sPopulateMenu( QMenu * pMenu, QTreeWidgetItem * pSelected );
    virtual void sEnterCountSlip();
    virtual void sCountSlipEditList();
    virtual void sViewInventoryHistory();
    virtual void sEdit();
    virtual void sEditTag();
    virtual void sEditSlip();
    virtual void sDelete();
    virtual void sDeleteTag();
    virtual void sDeleteSlip();
    virtual void sPost();
    virtual void sPostTag();
    virtual void sPostSlip();
    virtual void sSearch( const QString & pTarget );
    virtual void sFillList();
    virtual void sHandleAutoUpdate( bool pAutoUpdate );
    virtual void sHandleButtons(bool);
    virtual void sParameterTypeChanged();

protected slots:
    virtual void languageChange();

private:
    QButtonGroup* _highlightGroupInt;
    QButtonGroup* _codeGroup;

};

#endif // DSPCOUNTTAGEDITLIST_H
