/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef TERMSES_H
#define TERMSES_H

#include "guiclient.h"
#include "xwidget.h"
#include <parameter.h>
#include "ui_termses.h"

class termses : public XWidget, public Ui::termses
{
    Q_OBJECT

public:
    termses(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = Qt::Window);
    ~termses();

public slots:
    virtual void sFillList();
    virtual void sDelete();
    virtual void sNew();
    virtual void sEdit();
    virtual void sView();
    virtual void sPopulateMenu( QMenu * );
    virtual void sPrint();

protected slots:
    virtual void languageChange();

};

#endif // TERMSES_H
