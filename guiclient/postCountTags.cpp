/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "postCountTags.h"

#include <QVariant>
#include <QMessageBox>

postCountTags::postCountTags(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
  : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  _codeGroup = new QButtonGroup(this);
  _codeGroup->addButton(_plancode);
  _codeGroup->addButton(_classcode);

  // signals and slots connections
  connect(_close, SIGNAL(clicked()), this, SLOT(reject()));
  connect(_post, SIGNAL(clicked()), this, SLOT(sPost()));
  connect(_codeGroup, SIGNAL(buttonClicked(int)), this, SLOT(sParameterTypeChanged()));

  _parameter->setType(ParameterGroup::ClassCode);
  
  if(!_privileges->check("ThawInventory"))
  {
    _thaw->setChecked(FALSE);
    _thaw->setEnabled(FALSE);
  }
}

postCountTags::~postCountTags()
{
  // no need to delete child widgets, Qt does it all for us
}

void postCountTags::languageChange()
{
  retranslateUi(this);
}

void postCountTags::sPost()
{
  QString sql( "SELECT MIN(postCountTag(invcnt_id, :thaw)) AS result "
               "FROM invcnt, itemsite, item "
               "WHERE ( (invcnt_itemsite_id=itemsite_id)"
               " AND (invcnt_qoh_after IS NOT NULL)"
               " AND (NOT invcnt_posted)"
               " AND (itemsite_item_id=item_id)" );

  if (_warehouse->isSelected())
    sql += " AND (itemsite_warehous_id=:warehous_id)";
  
    if ((_parameter->type() == ParameterGroup::ClassCode) && _parameter->isSelected())
      sql += " AND (item_classcode_id=:classcode_id)";
    else if ((_parameter->type() == ParameterGroup::ClassCode) && _parameter->isPattern())
      sql += " AND (item_classcode_id IN (SELECT classcode_id FROM classcode WHERE (classcode_code ~ :classcode_pattern)))";
    else if ((_parameter->type() == ParameterGroup::PlannerCode) && _parameter->isSelected())
      sql += " AND (itemsite_plancode_id=:plancode_id)";
    else if ((_parameter->type() == ParameterGroup::PlannerCode) && _parameter->isPattern())
      sql += " AND (itemsite_plancode_id IN (SELECT plancode_id FROM plancode WHERE (plancode_code ~ :plancode_pattern)))";
  
  sql += ");";
  
  q.prepare(sql);
  q.bindValue(":thaw", QVariant(_thaw->isChecked()));
  _warehouse->bindValue(q);
  _parameter->bindValue(q);
  q.exec();

  if (q.first())
  {
    if (q.value("result").toInt() < 0)
      QMessageBox::information( this, tr("One or More Posts Failed"),
        tr("One or more of your Count Tags could not be posted. Review\n"
           "the Count Tag Edit list to ensure your Count Tags and\n"
           "Count Slips have been entered properly. Or contact your\n"
           "Systems Administrator for more information. The function\n"
           "postCountTag returned the error #%1." )
           .arg(q.value("result").toInt()) );
  }
  else
    systemError( this, tr("A System Error occurred at postCountTags::%1.")
                       .arg(__LINE__) );

  accept();
}

void postCountTags::sParameterTypeChanged()
{
  if(_plancode->isChecked())
    _parameter->setType(ParameterGroup::PlannerCode);
  else
    _parameter->setType(ParameterGroup::ClassCode);

}
