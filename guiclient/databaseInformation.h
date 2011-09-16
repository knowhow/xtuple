/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef DATABASEINFORMATION_H
#define DATABASEINFORMATION_H

#include "guiclient.h"
#include "xabstractconfigure.h"

#include "ui_databaseInformation.h"

class databaseInformation : public XAbstractConfigure, public Ui::databaseInformation
{
  Q_OBJECT

  public:
    databaseInformation(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WFlags fl = 0);
    ~databaseInformation();

  protected slots:
    virtual void languageChange();

    virtual bool sSave();

  signals:
    void saving();
};

#endif // DATABASEINFORMATION_H
