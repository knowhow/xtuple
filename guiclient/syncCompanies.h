/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef SYNCCOMPANIES_H
#define SYNCCOMPANIES_H

#include "guiclient.h"
#include <parameter.h>
#include "xwidget.h"

#include "ui_syncCompanies.h"

class syncCompanies : public XWidget, public Ui::syncCompanies
{
    Q_OBJECT

  public:
    syncCompanies(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = Qt::Window);
    ~syncCompanies();

    static bool userHasPriv(const int = cView);
    virtual void setVisible(bool);

  public slots:
    virtual void sHandleButtons();

  protected slots:
    virtual void languageChange();

    virtual void sFillList();
    virtual void sSync();
};

#endif // SYNCCOMPANIES_H
