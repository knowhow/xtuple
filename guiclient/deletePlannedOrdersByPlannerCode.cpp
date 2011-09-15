/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "deletePlannedOrdersByPlannerCode.h"

#include <QVariant>
#include <QMessageBox>

deletePlannedOrdersByPlannerCode::deletePlannedOrdersByPlannerCode(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
  : XDialog(parent, name, modal, fl)
{
  setupUi(this);


  // signals and slots connections
  connect(_close, SIGNAL(clicked()), this, SLOT(reject()));
  connect(_delete, SIGNAL(clicked()), this, SLOT(sDelete()));

  _captive = FALSE;

  _plannerCode->setType(ParameterGroup::PlannerCode);
}

deletePlannedOrdersByPlannerCode::~deletePlannedOrdersByPlannerCode()
{
  // no need to delete child widgets, Qt does it all for us
}

void deletePlannedOrdersByPlannerCode::languageChange()
{
  retranslateUi(this);
}

void deletePlannedOrdersByPlannerCode::sDelete()
{
  if (!_cutoffDate->isValid())
  {
    QMessageBox::critical( this, tr("Enter Cut Off Date"),
                           tr( "You must enter a cut off date for the Planned Orders\n"
                               "to be deleted." ) );
    _cutoffDate->setFocus();
    return;
  }

  if (!(_both->isChecked() || _mps->isChecked() || _mrp->isChecked()))
  {
    QMessageBox::critical( this, tr("Select Planning System"),
                           tr("You must select which Planning System(s) to delete from.") );
    return;
  }

  QString sql = "SELECT deletePlannedOrder(planord_id, :deleteChildren) "
                "FROM planord, itemsite "
                "WHERE ( (planord_itemsite_id=itemsite_id)"
                " AND (planord_startdate<=:cutOffDate)";

  if (!_deleteFirmed->isChecked())
    sql += " AND (NOT planord_firm)";

  if (_plannerCode->isSelected())
    sql += " AND (itemsite_plancode_id=:plancode_id)";
  else if (_plannerCode->isPattern())
    sql + " AND (itemsite_plancode_id IN (SELECT plancode_id FROM plancode WHERE (plancode_code ~ :plancode_pattern)))";

  if (_warehouse->isSelected())
    sql += " AND (itemsite_warehous_id=:warehous_id)";

  if (_mps->isChecked())
    sql += " AND (planord_mps)";
  else if (_mrp->isChecked())
    sql += " AND (NOT planord_mps)";
  
  sql += ");";

  q.prepare(sql);
  q.bindValue(":deleteChildren", QVariant(_deleteChildren->isChecked()));
  q.bindValue(":cutOffDate", _cutoffDate->date());
  _warehouse->bindValue(q);
  _plannerCode->bindValue(q);
  q.exec();

  accept();
}

