/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef CRMACCOUNTMERGEPICKRESULTPAGE_H
#define CRMACCOUNTMERGEPICKRESULTPAGE_H

#include <QWizardPage>

#include "ui_crmaccountMergeResultPage.h"

class CrmaccountMergeResultPagePrivate;

class CrmaccountMergeResultPage : public QWizardPage, public Ui::crmaccountMergeResultPage
{
  Q_OBJECT
  public:
    CrmaccountMergeResultPage(QWidget *parent = 0);
    ~CrmaccountMergeResultPage();

    virtual void initializePage();
    virtual int  nextId() const;
    virtual bool validatePage();

  protected:
    CrmaccountMergeResultPagePrivate *_data;
};

#endif
