/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "forms.h"

#include <QVariant>
#include <QMessageBox>
#include <parameter.h>
#include "form.h"
#include "guiclient.h"

forms::forms(QWidget* parent, const char* name, Qt::WFlags fl)
  : XWidget(parent, name, fl)
{
  setupUi(this);


  // signals and slots connections
  connect(_form, SIGNAL(valid(bool)), _delete, SLOT(setEnabled(bool)));
  connect(_form, SIGNAL(itemSelected(int)), _edit, SLOT(animateClick()));
  connect(_form, SIGNAL(valid(bool)), _edit, SLOT(setEnabled(bool)));
  connect(_new, SIGNAL(clicked()), this, SLOT(sNew()));
  connect(_edit, SIGNAL(clicked()), this, SLOT(sEdit()));
  connect(_delete, SIGNAL(clicked()), this, SLOT(sDelete()));
  connect(_close, SIGNAL(clicked()), this, SLOT(close()));

  _form->addColumn(tr("Name"),        _itemColumn, Qt::AlignLeft, true, "form_name" );
  _form->addColumn(tr("Description"), -1,          Qt::AlignLeft, true, "form_descrip" );

  sFillList();
}

forms::~forms()
{
  // no need to delete child widgets, Qt does it all for us
}

void forms::languageChange()
{
  retranslateUi(this);
}

void forms::sNew()
{
  ParameterList params;
  params.append("mode", "new");

  form newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void forms::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("form_id", _form->id());

  form newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void forms::sDelete()
{
  q.prepare( "DELETE FROM form "
             "WHERE (form_id=:form_id);" );
  q.bindValue(":form_id", _form->id());
  q.exec();

  sFillList();
}

void forms::sFillList()
{
  _form->populate( "SELECT form_id, form_name, form_descrip "
                   "FROM form "
                   "ORDER BY form_name;" );
}
