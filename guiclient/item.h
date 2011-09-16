/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef ITEM_2_H
#define ITEM_2_H

#include "guiclient.h"
#include "xwidget.h"
#include <parameter.h>

#include "ui_item.h"

class item : public XWidget, public Ui::item
{
    Q_OBJECT

public:
    item(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = Qt::Window);
    ~item();

    virtual void populate();
    virtual void clear();
    static void newItem();
    static void editItem( int pId );
    static void viewItem( int pId );
    virtual bool checkSitePrivs( int itemsiteid );
    Q_INVOKABLE virtual int id() { return _itemid; }

public slots:
    virtual SetResponse set( const ParameterList & pParams );
    virtual void setId(int);
    virtual void sSave();
    virtual void sNew();
    virtual void sEdit();
    virtual void sDelete();
    virtual void sFillList();
    virtual void sPrint();
    virtual void sFormatItemNumber();
    virtual void sPopulateUOMs();
    virtual void sHandleItemtype();
    virtual void sNewAlias();
    virtual void sEditAlias();
    virtual void sDeleteAlias();
    virtual void sFillAliasList();
    virtual void sNewSubstitute();
    virtual void sEditSubstitute();
    virtual void sDeleteSubstitute();
    virtual void sFillSubstituteList();
    virtual void sNewTransformation();
    virtual void sDeleteTransformation();
    virtual void sFillTransformationList();
    virtual void sEditBOM();
    virtual void sWorkbench();
    virtual void sNewItemSite();
    virtual void sEditItemSite();
    virtual void sViewItemSite();
    virtual void sDeleteItemSite();
    virtual void sFillListItemSites();
    virtual void sNewItemtax();
    virtual void sEditItemtax();
    virtual void sDeleteItemtax();
    virtual void sFillListItemtax();
    virtual void sNewUOM();
    virtual void sEditUOM();
    virtual void sDeleteUOM();
    virtual void sFillUOMList();
    virtual void sPopulatePriceUOMs();
    virtual void sConfiguredToggled(bool p);
    virtual void closeEvent( QCloseEvent * pEvent );
    virtual void sNewClassCode();
    virtual void sHandleButtons();
    virtual void sFillSourceList();
    virtual void sNewSource();
    virtual void sEditSource();
    virtual void sViewSource();
    virtual void sCopySource();
    virtual void sDeleteSource();
    virtual void sMaintainItemCosts();

protected:
    virtual void keyPressEvent( QKeyEvent * e );

protected slots:
    virtual void languageChange();

signals:
    void newId(int);
    void saved(int);

private:
    int _mode;
    int _itemid;
    bool _disallowPlanningType;
    QString _originalItemType;
    bool _inTransaction;

    void saveCore();
};

#endif // ITEM_2_H
