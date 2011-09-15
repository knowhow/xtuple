/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "createPlannedOrdersByPlannerCode.h"

#include <QVariant>
#include <QMessageBox>
#include <QSqlError>
#include "submitAction.h"
#include <metasql.h>
#include "mqlutil.h"

createPlannedOrdersByPlannerCode::createPlannedOrdersByPlannerCode(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
  : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  // signals and slots connections
  connect(_buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
  connect(_buttonBox, SIGNAL(accepted()), this, SLOT(sCreate()));

  _plannerCode->setType(ParameterGroup::PlannerCode);
}

createPlannedOrdersByPlannerCode::~createPlannedOrdersByPlannerCode()
{
  // no need to delete child widgets, Qt does it all for us
}

void createPlannedOrdersByPlannerCode::languageChange()
{
  retranslateUi(this);
}

void createPlannedOrdersByPlannerCode::sCreate()
{
  ParameterList params;
  if (! setParams(params))
    return;

  sCreate(params);
}

void createPlannedOrdersByPlannerCode::sCreate(ParameterList params)
{
  QProgressDialog progress;
  progress.setWindowModality(Qt::ApplicationModal);

  MetaSQLQuery mql = mqlLoad("schedule", "load");
  q = mql.toQuery(params);
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  int count=0;
  progress.setMaximum(q.size());
  XSqlQuery create;
  while (q.next())
  {
    progress.setLabelText(tr("Site: %1\n"
                             "Item: %2 - %3")
                          .arg(q.value("warehous_code").toString())
                          .arg(q.value("item_number").toString())
                          .arg(q.value("item_descrip1").toString()));

    ParameterList rparams = params;
    rparams.append("itemsite_id", q.value("itemsite_id"));
    MetaSQLQuery mql2 = mqlLoad("schedule", "create");
    create = mql2.toQuery(rparams);
    if (create.lastError().type() != QSqlError::NoError)
    {
      systemError(this, create.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }

    if (progress.wasCanceled())
      break;

    count++;
    progress.setValue(count);
  }

  accept();
}


void createPlannedOrdersByPlannerCode::sSubmit()
{
  ParameterList params;
  if (! setParams(params))
    return;

  submitAction newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() == XDialog::Accepted)
    accept();
}

bool createPlannedOrdersByPlannerCode::setParams(ParameterList &pParams)
{
  if (!_cutOffDate->isValid())
  {
    QMessageBox::warning( this, tr("Enter Cut Off Date"),
                          tr( "You must enter a valid Cut Off Date before\n"
                              "creating Planned Orders." ));
    _cutOffDate->setFocus();
    return false;
  }

  pParams.append("action_name", "RunMRP");

  pParams.append("MPS", QVariant(false));
  pParams.append("planningType", "M");
  _plannerCode->appendValue(pParams);
  _warehouse->appendValue(pParams); 
  
  pParams.append("cutOffDate", _cutOffDate->date());
  pParams.append("cutoff_offset", QDate::currentDate().daysTo(_cutOffDate->date()));
  pParams.append("deleteFirmed", QVariant(_deleteFirmed->isChecked()));

  return true;
}

