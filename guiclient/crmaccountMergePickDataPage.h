/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef CRMACCOUNTMERGEPICKDATAPAGE_H
#define CRMACCOUNTMERGEPICKDATAPAGE_H

#include "ui_crmaccountMergePickDataPage.h"

class CrmaccountMergePickDataPagePrivate;

class CrmaccountMergePickDataPage : public QWizardPage, public Ui::crmaccountMergePickDataPage
{
  Q_OBJECT

  public:
    CrmaccountMergePickDataPage(QWidget *parent = 0);
    ~CrmaccountMergePickDataPage();

    virtual void cleanupPage();
    virtual void initializePage();
    virtual bool isComplete() const;
    virtual bool validatePage();

  protected slots:
    virtual void sDeselect();
    virtual bool sDelete();
    virtual void sEdit();
    virtual void sFillList();
    virtual void sHandleButtons();
    virtual void sPopulateMenu(QMenu *pMenu, XTreeWidgetItem *);
    virtual void sSelect();
    virtual void sView();

  protected:
    CrmaccountMergePickDataPagePrivate *_data;
};

#endif
