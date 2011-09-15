/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef ENTERPORETURN_H
#define ENTERPORETURN_H

#include "guiclient.h"
#include "xwidget.h"
#include <parameter.h>

#include "ui_enterPoReturn.h"
#include "xerrormessage.h"

class enterPoReturn : public XWidget, public Ui::enterPoReturn
{
    Q_OBJECT

  public:
    enterPoReturn(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = Qt::Window);
    ~enterPoReturn();

  public slots:
    virtual enum SetResponse set(const ParameterList & pParams );
    virtual void closeEvent( QCloseEvent * pEvent );
    virtual void sEnter();
    virtual void sFillList();
    virtual void sPost();

  protected slots:
    virtual void languageChange();

  private:
    bool _captive;
    XErrorMessage *_dropshipWarn;
};

#endif // ENTERPORETURN_H
