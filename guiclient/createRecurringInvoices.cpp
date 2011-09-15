/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "createRecurringInvoices.h"

#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include "storedProcErrorLookup.h"
#include "submitAction.h"

createRecurringInvoices::createRecurringInvoices(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_update, SIGNAL(clicked()), this, SLOT(sUpdate()));
  connect(_submit, SIGNAL(clicked()), this, SLOT(sSubmit()));

  if (!_metrics->boolean("EnableBatchManager"))
    _submit->hide();
}

createRecurringInvoices::~createRecurringInvoices()
{
  // no need to delete child widgets, Qt does it all for us
}

void createRecurringInvoices::languageChange()
{
  retranslateUi(this);
}

void createRecurringInvoices::sUpdate()
{
  q.exec("SELECT createRecurringItems(NULL, 'I') AS result;");
  if (q.first())
  {
    int result = q.value("result").toInt();
    if (result < 0)
    {
      systemError(this,
                  storedProcErrorLookup("createRecurringInvoices", result),
                  __FILE__, __LINE__);
      return;
    }
  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  accept();
}

void createRecurringInvoices::sSubmit()
{
  ParameterList params;

  params.append("action_name", "CreateRecurringInvoices");

  submitAction newdlg(this, "", TRUE);
  newdlg.set(params);

  if(newdlg.exec() == XDialog::Accepted)
    accept();
}

