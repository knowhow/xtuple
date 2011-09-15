/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef FIXSERIAL_H
#define FIXSERIAL_H

#include "guiclient.h"
#include "xwidget.h"
#include <parameter.h>

#include "ui_fixSerial.h"

class fixSerial : public XWidget, public Ui::fixSerial
{
    Q_OBJECT

public:
    fixSerial(QWidget* parent = 0, const char * = 0, Qt::WindowFlags fl = Qt::Window);
    ~fixSerial();

public slots:
    virtual void sFillList();
    virtual void sFix();
    virtual void sFixAll();
    virtual void sHandleSerial();
    virtual void sPopulateMenu(QMenu *, QTreeWidgetItem *);

protected slots:
    virtual void languageChange();
    virtual bool fixOne(XTreeWidgetItem*);
};

#endif // FIXSERIAL_H
