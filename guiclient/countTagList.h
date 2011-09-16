/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef COUNTTAGLIST_H
#define COUNTTAGLIST_H

#include "guiclient.h"
#include "xdialog.h"
#include <parameter.h>

#include "ui_countTagList.h"

class countTagList : public XDialog, public Ui::countTagList
{
    Q_OBJECT

public:
    countTagList(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WFlags fl = 0);
    ~countTagList();

public slots:
    virtual enum SetResponse set(const ParameterList & pParams);
    virtual void sFillList();
    virtual void sSearch( const QString & pTarget );

protected slots:
    virtual void languageChange();

    virtual void sClose();
    virtual void sSelect();


private:
    int _cnttagid;
    int _type;

};

#endif // COUNTTAGLIST_H
