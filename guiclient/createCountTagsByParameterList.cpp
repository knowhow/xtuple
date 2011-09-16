/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "createCountTagsByParameterList.h"

#include <QVariant>
#include <QMessageBox>

/*
 *  Constructs a createCountTagsByParameterList as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
createCountTagsByParameterList::createCountTagsByParameterList(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);


  // signals and slots connections
  connect(_close, SIGNAL(clicked()), this, SLOT(reject()));
  connect(_create, SIGNAL(clicked()), this, SLOT(sCreate()));
  connect(_warehouse, SIGNAL(newID(int)), this, SLOT(sPopulateLocations()));
  connect(_byLocation, SIGNAL(toggled(bool)), _location, SLOT(setEnabled(bool)));

  _parameter->setType(ParameterGroup::ClassCode);

  _freeze->setEnabled(_privileges->check("FreezeInventory"));

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

/*
 *  Destroys the object and frees any allocated resources
 */
createCountTagsByParameterList::~createCountTagsByParameterList()
{
  // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
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
  QString sql;
//---------------Class Code--------------------------------------  
  if ((_parameter->type() == ParameterGroup::ClassCode) && _parameter->isSelected())
  {
    sql =  "SELECT createCountTag(itemsite_id, :comments, :priority, :freeze, :location_id) "
           "FROM ( SELECT itemsite_id";
    if(_byLocation->isChecked())
          sql += "      FROM itemsite, item, itemloc";
    else
          sql += "       FROM itemsite, item";
          sql +=  "       WHERE ( (itemsite_item_id=item_id)"
                "        AND (itemsite_warehous_id=:warehous_id)"
                "        AND (item_classcode_id=:classcode_id)";
    if(_byLocation->isChecked())
    {
       sql += "      AND (itemloc_location_id=:location_id)"
              "      AND (itemloc_itemsite_id = itemsite_id)"
              "      AND (validLocation(:location_id, itemsite_id))";
    }
    if(_ignoreZeroBalance->isChecked())
      sql += "      AND (itemsite_qtyonhand<>0.0)";
      sql += " )"
           "       ORDER BY item_number ) AS data;";
  }
//----------------Class Code Pattern-------------------------------  
  else if ((_parameter->type() == ParameterGroup::ClassCode) && _parameter->isPattern())
  {
    sql =  "SELECT createCountTag(itemsite_id, :comments, :priority, :freeze, :location_id) "
           "FROM ( SELECT itemsite_id";
    if(_byLocation->isChecked())
          sql += "      FROM itemsite, item, itemloc";
    else
          sql += "       FROM itemsite, item";
          sql += "       WHERE ( (itemsite_item_id=item_id)"
           "        AND (itemsite_warehous_id=:warehous_id)"
           "        AND (item_classcode_id IN (SELECT classcode_id FROM classcode WHERE (classcode_code ~ :classcode_pattern)))";
    if(_byLocation->isChecked())
    {
       sql += "      AND (itemloc_location_id=:location_id)"
              "      AND (itemloc_itemsite_id = itemsite_id)"
              "      AND (validLocation(:location_id, itemsite_id))";
    }
    if(_ignoreZeroBalance->isChecked())
      sql += "      AND (itemsite_qtyonhand<>0.0)";
      sql += " )"
           "       ORDER BY item_number ) AS data;";
  }
//-----------------Planner Code-------------------------------------
  else if ((_parameter->type() == ParameterGroup::PlannerCode) && _parameter->isSelected())
  {
    sql =  "SELECT createCountTag(itemsite_id, :comments, :priority, :freeze, :location_id) "
           "FROM ( SELECT itemsite_id";
    if(_byLocation->isChecked())
          sql += "      FROM itemsite, item, itemloc";
    else
          sql += "       FROM itemsite, item";
    sql +=       "       WHERE ( (itemsite_item_id=item_id)"
           "        AND (itemsite_warehous_id=:warehous_id)"           
           "        AND (itemsite_plancode_id=:plancode_id)";
    if(_byLocation->isChecked())
    {
       sql += "      AND (itemloc_location_id=:location_id)"
              "      AND (itemloc_itemsite_id = itemsite_id)"
              "      AND (validLocation(:location_id, itemsite_id))";
    }
    if(_ignoreZeroBalance->isChecked())
      sql += "      AND (itemsite_qtyonhand<>0.0)";
      sql += " )"
           "       ORDER BY item_number ) AS data;";
  }
//----------------Planner Code Pattern-------------------------------
  else if ((_parameter->type() == ParameterGroup::PlannerCode) && _parameter->isPattern())
  {
    sql =  "SELECT createCountTag(itemsite_id, :comments, :priority, :freeze, :location_id) "
           "FROM ( SELECT itemsite_id";
    if(_byLocation->isChecked())
          sql += "      FROM itemsite, item, itemloc";
    else
          sql += "       FROM itemsite, item";
          sql+=  "       WHERE ( (itemsite_item_id=item_id)"
           "        AND (itemsite_warehous_id=:warehous_id)"   
           "        AND (itemsite_plancode_id IN (SELECT plancode_id FROM plancode WHERE (plancode_code ~ :plancode_pattern)))";
    if(_byLocation->isChecked())
    {
       sql += "      AND (itemloc_location_id=:location_id)"
              "      AND (itemloc_itemsite_id = itemsite_id)"
              "      AND (validLocation(:location_id, itemsite_id))";
    }
    if(_ignoreZeroBalance->isChecked())
      sql += "      AND (itemsite_qtyonhand<>0.0)";
      sql += " )"
           "       ORDER BY item_number ) AS data;";
  }
  else
//----------------Warehouse ------------------------------------------
  {
    sql =  "SELECT createCountTag(itemsite_id, :comments, :priority, :freeze, :location_id) "
           "FROM ( SELECT itemsite_id";
    if(_byLocation->isChecked())
          sql += "      FROM itemsite, item, itemloc";
    else
          sql += "       FROM itemsite, item";
          sql+=  "       WHERE ( (itemsite_item_id=item_id)"
           "        AND (itemsite_warehous_id=:warehous_id))"; 
    if(_byLocation->isChecked())
    {
       sql += "      AND (itemloc_location_id=:location_id)"
              "      AND (itemloc_itemsite_id = itemsite_id)"
              "      AND (validLocation(:location_id, itemsite_id))";
    }
    if(_ignoreZeroBalance->isChecked())
      sql += "      AND (itemsite_qtyonhand<>0.0)";
    sql += "       ORDER BY item_number ) AS data;";
  }
  q.prepare(sql);
  q.bindValue(":comments", _comments->toPlainText());
  q.bindValue(":priority", QVariant(_priority->isChecked()));
  q.bindValue(":freeze",   QVariant(_freeze->isChecked()));
  q.bindValue(":warehous_id", _warehouse->id());
  if(_byLocation->isChecked())
    q.bindValue(":location_id", _location->id());
  _parameter->bindValue(q);
  if (!q.exec())
    systemError( this, tr("A System Error occurred at createCountTagsByParameterList::%1.")
                       .arg(__LINE__) );
  else if (!q.first())
    QMessageBox::information( this, tr("No Count Tags Created"),
      tr("No Item Sites matched the criteria you specified.") );

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

