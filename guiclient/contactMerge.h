/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef CONTACTMERGE_H
#define CONTACTMERGE_H

#include "guiclient.h"
#include "xwidget.h"
#include <parameter.h>

#include "ui_contactMerge.h"

class contactMerge : public XWidget, public Ui::contactMerge
{
    Q_OBJECT

public:
    contactMerge(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = Qt::Window);
    ~contactMerge();

protected slots:
    virtual void languageChange();

    virtual void sAdd();
    virtual void sCntctDelete();
    virtual void sCntctDoubleClicked();
    virtual void sCntctEdit();
    virtual void sCntctView();
    virtual void sSrcCntctEdit();
    virtual void sSrcCntctView();
    virtual void sFillList();
    virtual void sHandleMode();
    virtual void sHandleProcess();
    virtual void sPopulateCntctMenu(QMenu *);
    virtual void sPopulateSrcMenu(QMenu *, QTreeWidgetItem *, int);
    virtual void sPopulateSources();
    virtual void sPopulateTarget();
    virtual void sProcess();
    virtual void sPurge();
    virtual bool purgeConfirm();
    virtual void sRestore();
    virtual void sSelect(bool);
    virtual void sSelectCol();
    virtual void sSetTarget();
    virtual void sDeselect(int);
    virtual void sDeselectCntct();
    virtual void sDeselectSource();

private:
    int _selectCol;

};

#endif // CONTACTMERGE_H
