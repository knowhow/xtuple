/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef ENTERPORECEIPT_H
#define ENTERPORECEIPT_H

#include "guiclient.h"
#include "xwidget.h"
#include <parameter.h>
#include "ui_enterPoReceipt.h"


class enterPoReceipt : public XWidget, public Ui::enterPoReceipt
{
    Q_OBJECT

public:
    enterPoReceipt(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = Qt::Window);
    ~enterPoReceipt();

    static  void post(const QString, const int);

public slots:
    virtual void close();
    virtual enum SetResponse set(const ParameterList & pParams);
    virtual void sEnter();
    virtual void sFillList();
    virtual void sPost();
    virtual void sPrint();
    virtual void sReceiveAll();
    virtual void sSave();
    virtual void setParams(ParameterList &);
    virtual void sPrintItemLabel();
    virtual void sPopulateMenu( QMenu * pMenu, QTreeWidgetItem * selected );
    virtual void sBcFind();
    virtual void sCatchItemid( int pItemid );
    virtual void sCatchItemsiteid( int pItemsiteid );
    virtual void sCatchPoheadid( int pPoheadid );
    virtual void sCatchPoitemid( int pPoitemid );
    virtual void sCatchToheadid( int pToheadid );
    virtual void sCatchToitemid( int porderitemid );

protected slots:
    virtual void languageChange();

private:
    bool _captive;
    int _soheadid;

};

#endif // ENTERPORECEIPT_H
