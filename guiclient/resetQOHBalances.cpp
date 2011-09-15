/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "resetQOHBalances.h"

#include <QMessageBox>
#include <QSqlError>

#include "storedProcErrorLookup.h"

resetQOHBalances::resetQOHBalances(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_reset, SIGNAL(clicked()), this, SLOT(sReset()));

  _classCode->setType(ParameterGroup::ClassCode);
  _transDate->setEnabled(_privileges->check("AlterTransactionDates"));
  _transDate->setDate(omfgThis->dbDate());
  _username->setText(omfgThis->username());
}

resetQOHBalances::~resetQOHBalances()
{
  // no need to delete child widgets, Qt does it all for us
}

void resetQOHBalances::languageChange()
{
  retranslateUi(this);
}

void resetQOHBalances::sReset()
{
  if (QMessageBox::question(this, tr("Reset Selected QOH Balances?"),
                            tr( "<p>You are about to issue Adjustment "
                                "Transactions for all Item Sites within "
                                "the selected Site and Class Code(s) "
                                "that are not Lot/Serial or Location  "
                                "Controlled to adjust their QOH values to "
                                "0.<br>Are you sure that you want to do this?"),
                            QMessageBox::Yes,
                            QMessageBox::No | QMessageBox::Default) == QMessageBox::No)
    return;

  QString sql( "SELECT resetQOHBalance(itemsite_id, :date) AS result "
               "FROM itemsite, item "
               "WHERE ((itemsite_qtyonhand < 0)"
               " AND (itemsite_item_id=item_id)" );

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
  q.bindValue(":date", _transDate->date());
  q.exec();
  if (q.first())
  {
    int result = q.value("result").toInt();
    if (result < 0)
    {
      systemError(this, storedProcErrorLookup("resetQOHBalance", result),
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
