/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "createCycleCountTags.h"

#include <QSqlError>
#include <QVariant>

#include "storedProcErrorLookup.h"

createCycleCountTags::createCycleCountTags(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
    setupUi(this);

    _codeGroup = new QButtonGroup(this);
    _codeGroup->addButton(_plancode);
    _codeGroup->addButton(_classcode);

    connect(_create, SIGNAL(clicked()), this, SLOT(sCreate()));
    connect(_warehouse, SIGNAL(newID(int)), this, SLOT(sPopulateLocations()));
    connect(_codeGroup, SIGNAL(buttonClicked(int)), this, SLOT(sParameterTypeChanged()));

    _parameter->setType(ParameterGroup::ClassCode);

    _freeze->setEnabled(_privileges->check("CreateReceiptTrans"));
    
    //If not multi-warehouse hide whs control
    if (!_metrics->boolean("MultiWhs"))
    {
      _warehouseLit->hide();
      _warehouse->hide();
    }

    if (_preferences->boolean("XCheckBox/forgetful"))
      _priority->setChecked(true);

    sPopulateLocations();
}

createCycleCountTags::~createCycleCountTags()
{
    // no need to delete child widgets, Qt does it all for us
}

void createCycleCountTags::languageChange()
{
    retranslateUi(this);
}

enum SetResponse createCycleCountTags::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("maxTags", &valid);
  if (valid)
    _maxTags->setValue(param.toInt());

  _priority->setChecked(pParams.inList("priority"));
  _freeze->setChecked(pParams.inList("freeze"));

  param = pParams.value("warehous_id", &valid);
  if (valid)
    _warehouse->setId(param.toInt());

  param = pParams.value("classcode_id", &valid);
    _parameter->setId(param.toInt());

  param = pParams.value("classcode_pattern", &valid);
  if (valid)
    _parameter->setPattern(param.toString());

  param = pParams.value("comments", &valid);
  if (valid)
    _comments->setText(param.toString());

  if (pParams.inList("run"))
    sCreate();

  return NoError;
}

void createCycleCountTags::sCreate()
{
  QString fname;
  if ((_parameter->type() == ParameterGroup::ClassCode) && _parameter->isSelected())
  {
    q.prepare("SELECT createCycleCountsByWarehouseByClassCode(:warehous_id, :classcode_id, :maxTags, :comments, :priority, :freeze, :location_id, :ignore) AS result;");
    fname = "createCycleCountsByWarehouseByClassCode";
  }
  else if ((_parameter->type() == ParameterGroup::ClassCode) && _parameter->isPattern())
  {
    q.prepare("SELECT createCycleCountsByWarehouseByClassCode(:warehous_id, :classcode_pattern, :maxTags, :comments, :priority, :freeze, :location_id, :ignore) AS result;");
    fname = "createCycleCountsByWarehouseByClassCode";
  }
  else if ((_parameter->type() == ParameterGroup::PlannerCode) && _parameter->isSelected())
  {
    q.prepare("SELECT createCycleCountsByWarehouseByPlannerCode(:warehous_id, :plancode_id, :maxTags, :comments, :priority, :freeze, :location_id, :ignore) AS result;");
    fname = "createCycleCountsByWarehouseByPlannerCode";
  }
  else if ((_parameter->type() == ParameterGroup::PlannerCode) && _parameter->isPattern())
  {
    q.prepare("SELECT createCycleCountsByWarehouseByPlannerCode(:warehous_id, :plancode_pattern, :maxTags, :comments, :priority, :freeze, :location_id, :ignore) AS result;");
    fname = "createCycleCountsByWarehouseByPlannerCode";
  }
  else //if (_parameter->isAll())
  {
    q.prepare("SELECT createCycleCountsByWarehouse(:warehous_id, :maxTags, :comments, :priority, :freeze, :location_id, :ignore) AS result;");
    fname = "createCycleCountsByWarehouse";
  }

  _parameter->bindValue(q);
  q.bindValue(":warehous_id", _warehouse->id());
  q.bindValue(":maxTags", _maxTags->value());
  q.bindValue(":comments", _comments->toPlainText());
  q.bindValue(":priority", QVariant(_priority->isChecked()));
  q.bindValue(":freeze",   QVariant(_freeze->isChecked()));
  q.bindValue(":ignore",   QVariant(_ignoreZeroBalance->isChecked()));
  if(_byLocation->isChecked())
    q.bindValue(":location_id", _location->id());
  q.exec();
  if (q.first())
  {
    int result = q.value("result").toInt();
    if (result < 0)
    {
      systemError(this, storedProcErrorLookup(fname, result), __FILE__, __LINE__);
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

void createCycleCountTags::sPopulateLocations()
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
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void createCycleCountTags::sParameterTypeChanged()
{
  if(_plancode->isChecked())
    _parameter->setType(ParameterGroup::PlannerCode);
  else //if(_classcode->isChecked())
    _parameter->setType(ParameterGroup::ClassCode);
}
