/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef TAXBREAKDOWN_H
#define TAXBREAKDOWN_H

#include "guiclient.h"
#include "xdialog.h"
#include <parameter.h>
#include "taxCache.h"
#include "ui_taxBreakdown.h"

class taxBreakdown : public XDialog, public Ui::taxBreakdown
{
  Q_OBJECT

  public:
    taxBreakdown(QWidget* = 0, const char* = 0, bool = 0, Qt::WFlags = 0);
    ~taxBreakdown();

  public slots:
    void	     languageChange();

    virtual SetResponse set(const ParameterList&);
    virtual void	sAdjTaxDetail();
    virtual void	sFreightTaxDetail();
    virtual void	sLineTaxDetail();
    virtual void	sTotalTaxDetail();
    virtual void	sPopulate();

  private:
    int         _mode;
    int		_orderid;
    int         _sense;
    QString	_ordertype;
};
#endif // TAXBREAKDOWN_H
