/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef TAXCODERATE_H
#define TAXCODERATE_H

#include "guiclient.h"
#include "xdialog.h"
#include <parameter.h>

#include "ui_taxCodeRate.h"

class taxCodeRate : public XDialog, public Ui::taxCodeRate
{
  Q_OBJECT

  public:
    taxCodeRate(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WFlags fl = 0);
    ~taxCodeRate();

  public slots:
    virtual enum SetResponse set( const ParameterList & pParams );
    virtual void sSave();
    virtual void sPopulate();
    
  protected slots:
    virtual void languageChange();

  private:
    int _taxrateid;
    int _taxId;
    int _mode;
};

#endif // TAXCODERATE_H
