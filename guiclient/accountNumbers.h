/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef ACCOUNTNUMBERS_H
#define ACCOUNTNUMBERS_H

#include "guiclient.h"
#include <parameter.h>
#include "xwidget.h"

#include "ui_accountNumbers.h"

class accountNumbers : public XWidget, public Ui::accountNumbers
{
    Q_OBJECT

  public:
    accountNumbers(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = Qt::Window);
    ~accountNumbers();

  public slots:
    virtual bool setParams(ParameterList &);
    virtual void sBuildList();
    virtual void sHandleButtons();

  protected slots:
    virtual void languageChange();

    virtual void sDelete();
    virtual void sNew();
    virtual void sEdit();
    virtual void sPrint();
    virtual void sFillList();

  private:
    int _externalCol;

};

#endif // ACCOUNTNUMBERS_H
