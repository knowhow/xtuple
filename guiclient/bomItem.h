/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef BOMITEM_H
#define BOMITEM_H

#include "guiclient.h"
#include "xdialog.h"
#include <parameter.h>

#include "ui_bomItem.h"

class bomItem : public XDialog, public Ui::bomItem
{
    Q_OBJECT

public:
    bomItem(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WFlags fl = 0);
    ~bomItem();

public slots:
    virtual SetResponse set(const ParameterList & pParams );
    virtual void sSave();
    virtual void sSaveClick();
    virtual void sClose();
    virtual void sItemTypeChanged( const QString & type );
    virtual void populate();
    virtual void sNewSubstitute();
    virtual void sEditSubstitute();
    virtual void sDeleteSubstitute();
    virtual void sFillSubstituteList();
    virtual void sItemIdChanged();
    virtual void sCharIdChanged();

signals:
   void saved(int);

protected slots:
    virtual void languageChange();

private:
    int _mode;
    int _bomitemid;
    int _sourceBomitemid;
    int _itemid;
    int _revisionid;
    bool _saved;

};

#endif // BOMITEM_H
