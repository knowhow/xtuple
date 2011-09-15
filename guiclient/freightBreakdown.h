/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef FREIGHTBREAKDOWN_H
#define FREIGHTBREAKDOWN_H

#include "guiclient.h"
#include "xdialog.h"
#include <parameter.h>
#include "ui_freightBreakdown.h"

class freightBreakdown : public XDialog, public Ui::freightBreakdown
{
  Q_OBJECT

  public:
    freightBreakdown(QWidget* = 0, const char* = 0, bool = 0, Qt::WFlags = 0);
    ~freightBreakdown();

  public slots:
    void	     languageChange();

    virtual SetResponse set(const ParameterList&);
    virtual void	sSave();

  private:
    int         _mode;
    int         _orderid;
    QString     _ordertype;
    bool        _calcfreight;
};
#endif // FREIGHTBREAKDOWN_H
