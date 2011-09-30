/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef METASQLS_H
#define METASQLS_H

#include "guiclient.h"
#include "xwidget.h"

#include <mqledit.h>
#include <parameter.h>

#include "ui_metasqls.h"

class metasqls : public XWidget, public Ui::metasqls
{
    Q_OBJECT

public:
    metasqls(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = Qt::Window);
    ~metasqls();

    static bool userHasPriv(const int priv = cView);

public slots:
    virtual bool setParams(ParameterList &params);
    virtual void sDelete();
    virtual void sEdit();
    virtual void sFillList();
    virtual void sHandleButtons();
    virtual void sNew();
    virtual void sPopulateMenu(QMenu *pMenu);
    virtual void sPrint();
    virtual void sPrintMetaSQL();

protected slots:
    virtual void languageChange();

};

#endif // METASQLS_H
