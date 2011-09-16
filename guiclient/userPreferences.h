/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef USERPREFERENCES_H
#define USERPREFERENCES_H

#include "guiclient.h"
#include "xdialog.h"
#include "ui_userPreferences.h"

class userPreferences : public XDialog, public Ui::userPreferences
{
    Q_OBJECT

public:
    userPreferences(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WFlags fl = 0);
    ~userPreferences();

    bool save();

    virtual void setBackgroundImage( int pImageid );

public slots:
    virtual void sBackgroundList();
    virtual void sPopulate();
    virtual void sApply();
    virtual void sSave(bool close = true);
    virtual void sClose();

    virtual void sFillList();
    virtual void sNew();
    virtual void sEdit();
    virtual void sDelete();

    virtual void sAllWarehousesToggled( int pEvnttypeid );
    virtual void sWarehouseToggled( QTreeWidgetItem * selected );
    virtual void sFillWarehouseList();
    virtual void sTranslations();
    virtual void sDictionaries();

protected slots:
    virtual void languageChange();

private:
    int _backgroundImageid;
    Preferences * _pref;
    Preferences * _altPref;
    bool _dirty;
};

#endif // USERPREFERENCES_H
