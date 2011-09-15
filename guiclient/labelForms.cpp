/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "labelForms.h"

#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include <parameter.h>

#include "labelForm.h"

labelForms::labelForms(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  connect(_new, SIGNAL(clicked()), this, SLOT(sNew()));
  connect(_edit, SIGNAL(clicked()), this, SLOT(sEdit()));
  connect(_view, SIGNAL(clicked()), this, SLOT(sView()));
  connect(_delete, SIGNAL(clicked()), this, SLOT(sDelete()));
  
  _labelforms->addColumn(tr("Form Name"),       -1, Qt::AlignLeft,  true, "labelform_name"  );
  _labelforms->addColumn(tr("#/Page"),_orderColumn, Qt::AlignCenter,true, "labelform_perpage"  );

  if (_privileges->check("MaintainShippingForms"))
  {
    connect(_labelforms, SIGNAL(valid(bool)), _edit, SLOT(setEnabled(bool)));
    connect(_labelforms, SIGNAL(valid(bool)), _delete, SLOT(setEnabled(bool)));
    connect(_labelforms, SIGNAL(itemSelected(int)), _edit, SLOT(animateClick()));
  }
  else
  {
    _new->setEnabled(FALSE);
    connect(_labelforms, SIGNAL(itemSelected(int)), _view, SLOT(animateClick()));
  }

  sFillList();
}

labelForms::~labelForms()
{
    // no need to delete child widgets, Qt does it all for us
}

void labelForms::languageChange()
{
    retranslateUi(this);
}

void labelForms::sNew()
{
  ParameterList params;
  params.append("mode", "new");

  labelForm newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void labelForms::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("labelform_id", _labelforms->id());

  labelForm newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void labelForms::sView()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("labelform_id", _labelforms->id());

  labelForm newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void labelForms::sDelete()
{
  q.prepare( "DELETE FROM labelform "
             "WHERE (labelform_id=:labelform_id);" );
  q.bindValue(":labelform_id", _labelforms->id());
  q.exec();

  sFillList();
}

void labelForms::sFillList()
{
  q.prepare("SELECT labelform_id, labelform_name, labelform_perpage "
            "FROM labelform "
            "ORDER BY labelform_name;");
  q.exec();
  _labelforms->populate(q);
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}
