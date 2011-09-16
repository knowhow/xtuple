/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef CONTACTS_H
#define CONTACTS_H

#include "guiclient.h"
#include "display.h"
#include <parameter.h>

#include "ui_contacts.h"

class contacts : public display, public Ui::contacts
{
    Q_OBJECT

public:
    contacts(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = Qt::Window);

    virtual void setCrmacctid(int crmacctId);
    virtual int crmacctId();

    QAction * attachAction();
    QAction * detachAction();

public slots:
    virtual enum SetResponse set(const ParameterList&);
    virtual void sPopulateMenu(QMenu *, QTreeWidgetItem *, int);
    virtual void sNew();
    virtual void sEdit();
    virtual void sView();
    virtual void sDelete();
    virtual void sAttach();
    virtual void sDetach();
    virtual void sOpen();
    virtual bool setParams(ParameterList &);
   
signals:
    void cntctDetached(int);

private:
    int _crmacctid;

    QAction * _attachAct;
    QAction * _detachAct;

};

#endif // CONTACTS_H
