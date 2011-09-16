/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef CRMACCOUNTMERGEPICKPURGEPAGE_H
#define CRMACCOUNTMERGEPICKPURGEPAGE_H

#include "ui_crmaccountMergePurgePage.h"

class CrmaccountMergePurgePagePrivate;

class CrmaccountMergePurgePage : public QWizardPage, public Ui::crmaccountMergePurgePage
{
  Q_OBJECT
  public:
    CrmaccountMergePurgePage(QWidget *parent = 0);
    ~CrmaccountMergePurgePage();

    virtual void initializePage();

  protected slots:
    virtual void sDelete();
    virtual void sDeleteAll();
    virtual void sFillList();
    virtual void sHandleButtons();

  protected:
    CrmaccountMergePurgePagePrivate *_data;
};

#endif
