/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "checkFormats.h"

#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include <parameter.h>

#include "checkFormat.h"
#include "guiclient.h"
#include "storedProcErrorLookup.h"

checkFormats::checkFormats(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  connect(_new, SIGNAL(clicked()), this, SLOT(sNew()));
  connect(_edit, SIGNAL(clicked()), this, SLOT(sEdit()));
  connect(_delete, SIGNAL(clicked()), this, SLOT(sDelete()));

  _form->addColumn(tr("Name"), _itemColumn, Qt::AlignLeft, true, "form_name");
  _form->addColumn(tr("Description"),   -1, Qt::AlignLeft, true, "form_descrip");

  sFillList();
}

checkFormats::~checkFormats()
{
  // no need to delete child widgets, Qt does it all for us
}

void checkFormats::languageChange()
{
  retranslateUi(this);
}

void checkFormats::sNew()
{
  ParameterList params;
  params.append("mode", "new");

  checkFormat newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void checkFormats::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("form_id", _form->id());

  checkFormat newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void checkFormats::sDelete()
{
  q.prepare("SELECT deleteForm(:form_id) AS result;");
  q.bindValue(":form_id", _form->id());
  q.exec();
  if (q.first())
  {
    int result = q.value("result").toInt();
    if (result < 0)
    {
      systemError(this, storedProcErrorLookup("deleteForm", result),
                  __FILE__, __LINE__);
      return;
    }
  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  sFillList();
}

void checkFormats::sFillList()
{
  _form->populate( "SELECT form_id, form_name, form_descrip "
                   "FROM form "
                   "WHERE form_key='Chck' "
                   "ORDER BY form_name;" );
}
