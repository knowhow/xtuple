/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "createCountTagsByParameterList.h"

#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include <metasql.h>
#include "mqlutil.h"

createCountTagsByParameterList::createCountTagsByParameterList(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_create,     SIGNAL(clicked()),     this, SLOT(sCreate()));
  connect(_warehouse,  SIGNAL(newID(int)),    this, SLOT(sPopulateLocations()));
  connect(_byLocation, SIGNAL(toggled(bool)), _location, SLOT(setEnabled(bool)));

  _parameter->setType(ParameterGroup::ClassCode);

  _freeze->setEnabled(_privileges->check("FreezeInventory"));

  if (!_metrics->boolean("MultiWhs"))
  {
    _warehouseLit->hide();
    _warehouse->hide();
  }

  if (_preferences->boolean("XCheckBox/forgetful"))
    _priority->setChecked(true);

  sPopulateLocations();
}

createCountTagsByParameterList::~createCountTagsByParameterList()
{
  // no need to delete child widgets, Qt does it all for us
}

void createCountTagsByParameterList::languageChange()
{
  retranslateUi(this);
}

enum SetResponse createCountTagsByParameterList::set(const ParameterList & pParams)
{
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("classcode", &valid);
  if (valid)
    _parameter->setType(ParameterGroup::ClassCode);

  param = pParams.value("plancode", &valid);
  if (valid)
    _parameter->setType(ParameterGroup::PlannerCode);

  switch (_parameter->type())
  {
    case ParameterGroup::ClassCode:
      setWindowTitle(tr("Create Count Tags by Class Code"));
      break;

    case ParameterGroup::PlannerCode:
      setWindowTitle(tr("Create Count Tags by Planner Code"));
      break;

    default:
      break;
  }

  return NoError;
}

void createCountTagsByParameterList::sCreate()
{
  ParameterList params;
  _parameter->appendValue(params);
  params.append("comments", _comments->toPlainText());
  params.append("priority", QVariant(_priority->isChecked()));
  params.append("freeze",   QVariant(_freeze->isChecked()));
  params.append("warehous_id", _warehouse->id());
  if(_byLocation->isChecked())
    params.append("location_id", _location->id());
  if(_ignoreZeroBalance->isChecked())
    params.append("ignoreZeroBalance");

  XSqlQuery createq;
  MetaSQLQuery mql = mqlLoad("countTags", "create");
  createq = mql.toQuery(params);
  int count = 0;
  while (createq.next())
  {
    if (createq.value(0).toInt() > 0)
      count++;
  }
  if (createq.lastError().type() != QSqlError::NoError)
  {
    systemError(this, createq.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  if (count <= 0)
    QMessageBox::information( this, tr("No Count Tags Created"),
                             tr("<p>No Count Tags were created. Either no "
                                "Item Sites matched the criteria, the matching "
                                "Item Sites all have Control Method: None, "
                                "or the Items are not countable Items."));

  accept();
}

void createCountTagsByParameterList::sPopulateLocations()
{
  q.prepare( "SELECT location_id, "
             "       CASE WHEN (LENGTH(location_descrip) > 0) THEN (formatLocationName(location_id) || '-' || location_descrip)"
             "            ELSE formatLocationName(location_id)"
             "       END AS locationname "
             "FROM location "
             "WHERE (location_warehous_id=:warehous_id) "
             "ORDER BY locationname;" );
  q.bindValue(":warehous_id", _warehouse->id());
  q.exec();
  _location->populate(q);
}

