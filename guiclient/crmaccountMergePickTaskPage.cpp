/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include <metasql.h>
#include <mqlutil.h>

#include "crmaccountMerge.h"
#include "crmaccountMergePickTaskPage.h"
#include "errorReporter.h"

CrmaccountMergePickTaskPage::CrmaccountMergePickTaskPage(QWidget *parent)
  : QWizardPage(parent)
{
  setupUi(this);

  connect(_continue,         SIGNAL(toggled(bool)), this, SLOT(sHandleButtons()));
  connect(_existingMerge,      SIGNAL(valid(bool)), this, SLOT(sHandleButtons()));
  connect(_revert,           SIGNAL(toggled(bool)), this, SLOT(sHandleButtons()));
  connect(_startPurge,       SIGNAL(toggled(bool)), this, SLOT(sHandleButtons()));
  connect(omfgThis,SIGNAL(crmAccountsUpdated(int)), this, SLOT(sUpdateComboBoxes()));

  registerField("_completedMerge", _completedMerge, "text", "currentIndexChanged(QString)");
  registerField("_existingMerge",  _existingMerge,  "text", "currentIndexChanged(QString)");
}

void CrmaccountMergePickTaskPage::initializePage()
{
  sUpdateComboBoxes();
}

bool CrmaccountMergePickTaskPage::isComplete() const
{
  if (_continue->isChecked() || _revert->isChecked())
    return _existingMerge->isValid();

  return true;
}

int CrmaccountMergePickTaskPage::nextId() const
{
  if (_continue->isChecked())    return crmaccountMerge::Page_PickData;
  else if (_revert->isChecked()) return crmaccountMerge::Page_Result;
  else if (_startPurge->isChecked())     return crmaccountMerge::Page_Purge;

  return crmaccountMerge::Page_PickAccounts;
}

void CrmaccountMergePickTaskPage::sHandleButtons()
{
  _continue->setEnabled(_existingMerge->count());
  _revert->setEnabled(_completedMerge->count());
  _startPurge->setEnabled(_completedMerge->count());

  _completedMerge->setEnabled(_revert->isEnabled());
  _existingMerge->setEnabled(_continue->isEnabled());

  emit completeChanged();
}

void CrmaccountMergePickTaskPage::sUpdateComboBoxes()
{
  XSqlQuery contq("SELECT crmacct_id, crmacct_number"
                  "  FROM crmacct"
                  " WHERE crmacct_id IN (SELECT crmacctsel_dest_crmacct_id"
                  "                        FROM crmacctsel)"
                  " ORDER BY crmacct_number;");
  _existingMerge->populate(contq);
  ErrorReporter::error(QtCriticalMsg, this, tr("Looking for Merges"),
                       contq, __FILE__, __LINE__);

  XSqlQuery undoq("SELECT crmacct_id, crmacct_number"
                  "  FROM crmacct"
                  " WHERE crmacct_id IN (SELECT mrgundo_base_id"
                  "                        FROM mrgundo"
                  "                      WHERE mrgundo_base_schema='public'"
                  "                        AND mrgundo_base_table='crmacct')"
                  " ORDER BY crmacct_number;");
  _completedMerge->populate(undoq);
  ErrorReporter::error(QtCriticalMsg, this, tr("Looking for Merges"),
                       undoq, __FILE__, __LINE__);

  sHandleButtons();
}
