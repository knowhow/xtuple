/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef COPYITEM_H
#define COPYITEM_H

#include "guiclient.h"
#include "xdialog.h"
#include <parameter.h>

#include "ui_copyItem.h"

class copyItem : public XDialog, public Ui::copyItem
{
    Q_OBJECT

public:
    copyItem(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WFlags fl = 0);
    ~copyItem();

    Q_INVOKABLE inline  bool    captive() const { return _captive; }
    Q_INVOKABLE virtual void    createItemSites(int pItemid);
    Q_INVOKABLE virtual bool    okToSave();

public slots:
    virtual enum SetResponse set(const ParameterList & pParams);
    virtual void clear();
    virtual void sHandleItemType(const QString & pItemType);
    virtual void sCopy();

protected slots:
    virtual void languageChange();

private:
    bool _captive;

};

#endif // COPYITEM_H
