/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "subAccntTypes.h"

#include <QVariant>
#include <QMessageBox>
#include "subAccntType.h"
#include <openreports.h>

subAccntTypes::subAccntTypes(QWidget* parent, const char* name, Qt::WFlags fl)
  : XWidget(parent, name, fl)
{
  setupUi(this);


  // signals and slots connections
  connect(_subaccnttypes, SIGNAL(valid(bool)), _delete, SLOT(setEnabled(bool)));
  connect(_subaccnttypes, SIGNAL(valid(bool)), _edit, SLOT(setEnabled(bool)));
  connect(_subaccnttypes, SIGNAL(itemSelected(int)), _edit, SLOT(animateClick()));
  connect(_close, SIGNAL(clicked()), this, SLOT(close()));
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_new, SIGNAL(clicked()), this, SLOT(sNew()));
  connect(_edit, SIGNAL(clicked()), this, SLOT(sEdit()));
  connect(_delete, SIGNAL(clicked()), this, SLOT(sDelete()));

  _subaccnttypes->addColumn(tr("Code"),        70, Qt::AlignLeft,   true,  "subaccnttype_code" );
  _subaccnttypes->addColumn(tr("Type"),        50, Qt::AlignLeft,   true,  "type" );
  _subaccnttypes->addColumn(tr("Description"), -1, Qt::AlignLeft,   true,  "subaccnttype_descrip" );
  
  sFillList();
}

subAccntTypes::~subAccntTypes()
{
  // no need to delete child widgets, Qt does it all for us
}

void subAccntTypes::languageChange()
{
  retranslateUi(this);
}

void subAccntTypes::sNew()
{
  ParameterList params;
  params.append("mode", "new");

  subAccntType newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void subAccntTypes::sPrint()
{
  orReport report("SubAccountTypeMasterList");
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void subAccntTypes::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("subaccnttype_id", _subaccnttypes->id());

  subAccntType newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void subAccntTypes::sDelete()
{
  q.prepare("SELECT deleteSubAccountType(:subaccnttype_id) AS result;");
  q.bindValue(":subaccnttype_id", _subaccnttypes->id());
  q.exec();
  if(q.first())
  {
    switch(q.value("result").toInt())
    {
    case -1:
      QMessageBox::critical( this, tr("Cannot Delete G/L Subaccount Type"),
          tr( "The selected G/L Subaccount Type cannot be deleted as it is currently used in one or more G/L Accounts.\n"
              "You must reassign these G/L Accounts before you may delete the selected G/L Subaccount Type." ) );
        break;
    default:
      sFillList();
    }
  }
}

void subAccntTypes::sFillList()
{
  q.prepare(
      "SELECT subaccnttype_id, subaccnttype_code,"
      "       CASE WHEN(subaccnttype_accnt_type='A') THEN :asset"
      "            WHEN(subaccnttype_accnt_type='L') THEN :liability"
      "            WHEN(subaccnttype_accnt_type='E') THEN :expense"
      "            WHEN(subaccnttype_accnt_type='R') THEN :revenue"
      "            WHEN(subaccnttype_accnt_type='Q') THEN :equity"
      "            ELSE :error"
      "       END AS type, subaccnttype_descrip "
      "FROM subaccnttype "
      "ORDER BY subaccnttype_code; " );
  q.bindValue(":asset", tr("Asset"));
  q.bindValue(":liability", tr("Liability"));
  q.bindValue(":expense", tr("Expense"));
  q.bindValue(":revenue", tr("Revenue"));
  q.bindValue(":equity", tr("Equity"));
  q.bindValue(":error", tr("ERROR"));
  q.exec();
  _subaccnttypes->populate(q);
}

