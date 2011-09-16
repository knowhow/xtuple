/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef CRMACCOUNTMERGEPICKTASKPAGE_H
#define CRMACCOUNTMERGEPICKTASKPAGE_H

#include <QWizardPage>

#include "ui_crmaccountMergePickTaskPage.h"

class CrmaccountMergePickTaskPage : public QWizardPage, public Ui::crmaccountMergePickTaskPage
{
  Q_OBJECT

  public:
    CrmaccountMergePickTaskPage(QWidget *parent = 0);

    Q_INVOKABLE virtual void initializePage();
    Q_INVOKABLE virtual bool isComplete() const;
    Q_INVOKABLE virtual int  nextId()     const;

  protected slots:
    virtual void sHandleButtons();
    virtual void sUpdateComboBoxes();
};

#endif
