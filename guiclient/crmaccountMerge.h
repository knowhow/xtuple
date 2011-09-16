/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef CRMACCOUNTMERGE_H
#define CRMACCOUNTMERGE_H

#include "guiclient.h"
#include "xwidget.h"
#include <parameter.h>

class crmaccount;
class crmaccountMergePrivate;

#include "ui_crmaccountMerge.h"

class crmaccountMerge : public QWizard, public Ui::crmaccountMerge
{
    Q_OBJECT

  public:
    crmaccountMerge(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = Qt::Window);
    ~crmaccountMerge();

    enum { Page_PickTask, Page_PickAccounts, Page_PickData,
           Page_Result,   Page_Purge };

  protected slots:
    virtual void languageChange();

  protected:
    crmaccountMergePrivate *_data;

};

#endif // CRMACCOUNTMERGE_H
