/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "updateLateCustCreditStatus.h"

#include <QVariant>
#include <QMessageBox>

#include "submitAction.h"

/*
 *  Constructs a updateLateCustCreditStatus as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
updateLateCustCreditStatus::updateLateCustCreditStatus(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_update, SIGNAL(clicked()), this, SLOT(sUpdate()));
  connect(_submit, SIGNAL(clicked()), this, SLOT(sSubmit()));

  if (!_metrics->boolean("EnableBatchManager"))
    _submit->hide();
}

/*
 *  Destroys the object and frees any allocated resources
 */
updateLateCustCreditStatus::~updateLateCustCreditStatus()
{
  // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void updateLateCustCreditStatus::languageChange()
{
  retranslateUi(this);
}

void updateLateCustCreditStatus::sUpdate()
{
  q.exec("UPDATE custinfo"
         "   SET cust_creditstatus = 'W'"
         " WHERE(((SELECT count(aropen_id)"
         "           FROM aropen"
         "          WHERE aropen_cust_id = cust_id"
         "            AND aropen_open"
         "            AND aropen_doctype IN ('I', 'D')"
         "            AND aropen_duedate < CURRENT_DATE"
         "                   - COALESCE(cust_gracedays,"
         "                       COALESCE((SELECT CAST(metric_value AS INTEGER)"
         "                          FROM metric"
         "                         WHERE(metric_name='DefaultAutoCreditWarnGraceDays')),30))) > 0)"
         "   AND (cust_autoupdatestatus)"
         "   AND (cust_creditstatus = 'G'));");

  accept();
}

void updateLateCustCreditStatus::sSubmit()
{
  ParameterList params;

  params.append("action_name", "UpdateLateCustCreditStatus");

  submitAction newdlg(this, "", TRUE);
  newdlg.set(params);

  if(newdlg.exec() == XDialog::Accepted)
    accept();
}

