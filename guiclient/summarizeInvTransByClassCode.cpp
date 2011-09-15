/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "summarizeInvTransByClassCode.h"

#include <QVariant>
#include <QMessageBox>

summarizeInvTransByClassCode::summarizeInvTransByClassCode(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
  : XDialog(parent, name, modal, fl)
{
  setupUi(this);


  // signals and slots connections
  connect(_close, SIGNAL(clicked()), this, SLOT(reject()));
  connect(_summarize, SIGNAL(clicked()), this, SLOT(sSummarize()));

  _classCode->setType(ParameterGroup::ClassCode);
}

summarizeInvTransByClassCode::~summarizeInvTransByClassCode()
{
  // no need to delete child widgets, Qt does it all for us
}

void summarizeInvTransByClassCode::languageChange()
{
  retranslateUi(this);
}

void summarizeInvTransByClassCode::sSummarize()
{
  if ( QMessageBox::warning( this, tr("Summarize Inventory Transactions?"),
                             tr( "<p>You are about to summarize Inventory Transactions for the selected Site and Class Code(s). "
                                 "Summarizing Inventory Transactions will delete any detailed Transaction information.<br>"
                                 "Are you sure that you want to summarize the selected Transactions?</p>" ),
                             tr("&Yes"), tr("&No"), QString::null, 0, 1) == 1)
  {
    reject();
    return;
  }

  QString sql( "SELECT summarizeTransactions(itemsite_id, :startDate, :endDate) AS result "
               "FROM itemsite, item "
               "WHERE ( (itemsite_item_id=item_id)" );

  if (_warehouse->isSelected())
    sql += " AND (itemsite_warehous_id=:warehous_id)";

  if (_classCode->isSelected())
    sql += " AND (item_classcode_id=:classcode_id)";
  else if (_classCode->isPattern())
    sql += " AND (item_classcode_id IN (SELECT classcode_id FROM classcode WHERE (classcode_code ~ :classcode_pattern)))";

  sql += ");";

  q.prepare(sql);
  _warehouse->bindValue(q);
  _classCode->bindValue(q);
  _dates->bindValue(q);
  q.exec();

  QMessageBox::information( this, tr("Summary Complete"),
                            tr("The selected Inventory Transactions have been summarized.") );
}
