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

#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include <mqlutil.h>

#include "storedProcErrorLookup.h"

postCountTags::postCountTags(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
  : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  _codeGroup = new QButtonGroup(this);
  _codeGroup->addButton(_plancode);
  _codeGroup->addButton(_classcode);

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

bool postCountTags::setParams(ParameterList &params)
{
  params.append("thaw", QVariant(_thaw->isChecked()));
  _warehouse->appendValue(params);
  _parameter->appendValue(params);

  return true;
}

void postCountTags::sPost()
{
  ParameterList postp;
  if (! setParams(postp))
    return;

  bool    valid = false;
  QString errmsg;
  MetaSQLQuery postm = MQLUtil::mqlLoad("postCountTags", "post",
                                        errmsg, &valid);
  if (! valid)
  {
    QMessageBox::critical(this, tr("Query Error"), errmsg);
    return;
  }

  XSqlQuery postq = postm.toQuery(postp);
  if (postq.first())
  {
    int result = postq.value("result").toInt();
    if (result < 0)
    {
      QMessageBox::critical(this, tr("One or More Posts Failed"),
                            storedProcErrorLookup("postCountTag", result));
      return;
    }
  }
  else if (postq.lastError().type() != QSqlError::NoError)
  {
    QMessageBox::critical(this, tr("Database Error"), postq.lastError().text());
    return;
  }

  accept();
}

void postCountTags::sParameterTypeChanged()
{
  if(_plancode->isChecked())
    _parameter->setType(ParameterGroup::PlannerCode);
  else
    _parameter->setType(ParameterGroup::ClassCode);
}

