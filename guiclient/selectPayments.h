/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef SELECTPAYMENTS_H
#define SELECTPAYMENTS_H

#include "xwidget.h"
#include "ui_selectPayments.h"

class selectPayments : public XWidget, public Ui::selectPayments
{
    Q_OBJECT

public:
    selectPayments(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = Qt::Window, bool pAutoFill = true);
    ~selectPayments();
    virtual bool setParams(ParameterList &);

public slots:
    virtual void sPrint();
    virtual void sSelectDue();
    virtual void sSelectDiscount();
    virtual void sClearAll();
    virtual void sSelect();
    virtual void sSelectLine();
    virtual void sClear();
    virtual void sApplyAllCredits();
    virtual void sFillList();
	virtual void sPopulateMenu(QMenu *pMenu,QTreeWidgetItem *selected);
	virtual void sOnHold();
	virtual void sOpen();

protected:
    bool _ignoreUpdates;

protected slots:
    virtual void languageChange();

};

#endif // SELECTPAYMENTS_H
