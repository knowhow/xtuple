/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef CRMACCOUNTMERGEPICKACCOUNTSPAGE_H
#define CRMACCOUNTMERGEPICKACCOUNTSPAGE_H

#include <QList>
#include <QWizardPage>

#include "ui_crmaccountMergePickAccountsPage.h"

class CrmaccountMergePickAccountsPagePrivate;

class CrmaccountMergePickAccountsPage : public QWizardPage, public Ui::crmaccountMergePickAccountsPage
{
  Q_OBJECT

  public:
    CrmaccountMergePickAccountsPage(QWidget *parent = 0);
    ~CrmaccountMergePickAccountsPage();

    Q_INVOKABLE virtual void cleanupPage();
    Q_INVOKABLE virtual void initializePage();
    Q_INVOKABLE virtual bool isComplete() const;
    Q_INVOKABLE virtual bool validatePage();

  protected slots:
    void sFillList();
    void sHandleButtons();

  protected:
    CrmaccountMergePickAccountsPagePrivate *_data;
};

#endif
