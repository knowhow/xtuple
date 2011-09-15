/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef XABSTRACTCONFIGURE_H
#define XABSTRACTCONFIGURE_H

#include <QObject>

#include "xwidget.h"

class XAbstractConfigure : public XWidget
{
  Q_OBJECT

  public slots:
    virtual bool sSave() = 0;

  protected:
    XAbstractConfigure(QWidget *parent = 0, Qt::WindowFlags f = 0);
    virtual ~XAbstractConfigure();

  protected slots:
    virtual void languageChange() = 0;
};

#endif
