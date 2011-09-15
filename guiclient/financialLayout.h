/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef FINANCIALLAYOUT_H
#define FINANCIALLAYOUT_H

#include "guiclient.h"
#include "xdialog.h"
#include <QStack>
#include <parameter.h>

#include "ui_financialLayout.h"

class financialLayout : public XDialog, public Ui::financialLayout
{
    Q_OBJECT

public:
    financialLayout(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WFlags fl = 0);
    ~financialLayout();

    virtual void insertFlGroup( int pFlgrpid, QString pFlgrpname, XTreeWidgetItem * pParent, int pId, int pType );
    virtual void insertFlGroupAdHoc( int pFlgrpid, QString pFlgrpname, XTreeWidgetItem * pParent, int pId, int pType );

public slots:
    virtual SetResponse set( const ParameterList & pParams );
    virtual void sCheck();
    virtual void populate();
    virtual void sFillList();
    virtual void sHandleButtons();
    virtual void sHandleSelection();
    virtual void sAddTopLevelGroup();
    virtual void sAddGroup();
    virtual void sAddAccount();
    virtual void sAddSpecial();
    virtual void sEdit();
    virtual void sDelete();
    virtual void sMoveUp();
    virtual void sMoveDown();
    virtual void sSetType();
    virtual void reject();
    virtual void sHandleButtonsCol();
    virtual void sAddCol();
    virtual void sEditCol();
    virtual void sDeleteCol();
    virtual void sUncheckAltGrandTotal();

protected slots:
    virtual void languageChange();

    virtual void sSave();


private:
    XTreeWidgetItem * _last;
    QStack<XTreeWidgetItem*> _lastStack;
    int _flheadid;
    int _mode;
    int _cachedType;

};

#endif // FINANCIALLAYOUT_H
