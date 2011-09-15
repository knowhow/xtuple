/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "updateCycleCountFrequency.h"

#include <qvariant.h>

/*
 *  Constructs a updateCycleCountFrequency as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
updateCycleCountFrequency::updateCycleCountFrequency(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
    setupUi(this);


    // signals and slots connections
    connect(_close, SIGNAL(clicked()), this, SLOT(reject()));
    connect(_update, SIGNAL(clicked()), this, SLOT(sUpdate()));
    init();
}

/*
 *  Destroys the object and frees any allocated resources
 */
updateCycleCountFrequency::~updateCycleCountFrequency()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void updateCycleCountFrequency::languageChange()
{
    retranslateUi(this);
}


void updateCycleCountFrequency::init()
{
  _classCode->setType(ParameterGroup::ClassCode);

}

enum SetResponse updateCycleCountFrequency::set(ParameterList &pParams)
{
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("aFrequency", &valid);
  if (valid)
    _classAFrequency->setValue(param.toInt());
  else
    _classAFrequency->setValue(0);

  param = pParams.value("bFrequency", &valid);
  if (valid)
    _classBFrequency->setValue(param.toInt());
  else
    _classBFrequency->setValue(0);

  param = pParams.value("cFrequency", &valid);
  if (valid)
    _classCFrequency->setValue(param.toInt());
  else
    _classCFrequency->setValue(0);

  param = pParams.value("warehous_id", &valid);
  if (valid)
    _warehouse->setId(param.toInt());

  param = pParams.value("classcode_id", &valid);
  if (valid)
    _classCode->setId(param.toInt());

  param = pParams.value("classcode_code", &valid);
  if (valid)
    _classCode->setPattern(param.toString());

  if (pParams.inList("run"))
  {
    sUpdate();
    return NoError_Run;
  }

  return NoError;
}

void updateCycleCountFrequency::sUpdate()
{
  QString sql( "UPDATE itemsite "
               "SET itemsite_cyclecountfreq=:afreq "
               "FROM item "
               "WHERE ( (itemsite_item_id=item_id)"
               " AND (itemsite_abcclass='A')" );

  if (_warehouse->isSelected())
    sql += " AND (itemsite_warehous_id=:warehous_id)";

  if (_classCode->isSelected())
    sql += " AND (item_classcode_id=:classcode_id)";
  else if (_classCode->isPattern())
    sql += " AND (item_classcode_id IN (SELECT classcode_id FROM classcode WHERE (classcode_code ~ :classcode_pattern)))";

  sql += "); "
         "UPDATE itemsite "
         "SET itemsite_cyclecountfreq=:bfreq "
         "FROM item "
         "WHERE ( (itemsite_item_id=item_id)"
         " AND (itemsite_abcclass='B')" ;

  if (_warehouse->isSelected())
    sql += " AND (itemsite_warehous_id=:warehous_id)";

  if (_classCode->isSelected())
    sql += " AND (item_classcode_id=:classcode_id)";
  else if (_classCode->isPattern())
    sql += " AND (item_classcode_id IN (SELECT classcode_id FROM classcode WHERE (classcode_code ~ :classcode_pattern)))";

  sql += "); "
         "UPDATE itemsite "
         "SET itemsite_cyclecountfreq=:cfreq "
         "FROM item "
         "WHERE ( (itemsite_item_id=item_id)"
         " AND (itemsite_abcclass='C')" ;

  if (_warehouse->isSelected())
    sql += " AND (itemsite_warehous_id=:warehous_id)";

  if (_classCode->isSelected())
    sql += " AND (item_classcode_id=:classcode_id)";
  else if (_classCode->isPattern())
    sql += " AND (item_classcode_id IN (SELECT classcode_id FROM classcode WHERE (classcode_code ~ :classcode_pattern)))";

  sql += ");";

  q.prepare(sql);
  q.bindValue(":afreq", _classAFrequency->value());
  q.bindValue(":bfreq", _classBFrequency->value());
  q.bindValue(":cfreq", _classCFrequency->value());
  _warehouse->bindValue(q);
  _classCode->bindValue(q);
  q.exec();

  accept();
}
