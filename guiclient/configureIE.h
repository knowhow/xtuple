/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef CONFIGUREIE_H
#define CONFIGUREIE_H

#include "guiclient.h"
#include "xabstractconfigure.h"

#include "ui_configureIE.h"

class configureIE : public XAbstractConfigure, public Ui::configureIE
{
    Q_OBJECT

public:
    configureIE(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WFlags fl = 0);
    ~configureIE();

    static bool userHasPriv();

public slots:
    virtual void sDeleteAtlasMap();
    virtual void sDeleteMap();
    virtual void sEditAtlasMap();
    virtual void sEditMap();
    virtual void sFillList();
    virtual void sNewAtlasMap();
    virtual void sNewMap();
    virtual void sPopulate();
    virtual void sPopulateMenu(QMenu*, QTreeWidgetItem*);
    virtual bool sSave();

protected slots:
    virtual void languageChange();

signals:
    void saving();

};

#endif // CONFIGUREIE_H
