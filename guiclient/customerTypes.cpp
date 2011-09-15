/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "customerTypes.h"

#include <QMenu>
#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include <parameter.h>
#include <openreports.h>

#include "customerType.h"
#include "guiclient.h"
#include "storedProcErrorLookup.h"

customerTypes::customerTypes(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_new, SIGNAL(clicked()), this, SLOT(sNew()));
  connect(_edit, SIGNAL(clicked()), this, SLOT(sEdit()));
  connect(_delete, SIGNAL(clicked()), this, SLOT(sDelete()));
  connect(_custtype, SIGNAL(populateMenu(QMenu *, QTreeWidgetItem *, int)), this, SLOT(sPopulateMenu(QMenu*)));

  if (_privileges->check("MaintainCustomerTypes"))
  {
    connect(_custtype, SIGNAL(valid(bool)), _edit, SLOT(setEnabled(bool)));
    connect(_custtype, SIGNAL(valid(bool)), _delete, SLOT(setEnabled(bool)));
    connect(_custtype, SIGNAL(itemSelected(int)), _edit, SLOT(animateClick()));
  }
  else
  {
    _new->setEnabled(FALSE);
  }

  _custtype->addColumn(tr("Code"),        70, Qt::AlignLeft, true, "custtype_code");
  _custtype->addColumn(tr("Description"), -1, Qt::AlignLeft, true, "custtype_descrip");
  
  sFillList();
}

customerTypes::~customerTypes()
{
  // no need to delete child widgets, Qt does it all for us
}

void customerTypes::languageChange()
{
  retranslateUi(this);
}

void customerTypes::sNew()
{
  ParameterList params;
  params.append("mode", "new");

  customerType newdlg(this, "", TRUE);
  newdlg.set(params);
  
  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void customerTypes::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("custtype_id", _custtype->id());

  customerType newdlg(this, "", TRUE);
  newdlg.set(params);
  
  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void customerTypes::sView()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("custtype_id", _custtype->id());

  customerType newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void customerTypes::sFillList()
{
  _custtype->populate( "SELECT custtype_id, custtype_code, custtype_descrip "
                       "FROM custtype "
                       "ORDER BY custtype_code;" );
}

void customerTypes::sDelete()
{
  q.prepare("SELECT deleteCustomerType(:custtype_id) AS result;");
  q.bindValue(":custtype_id", _custtype->id());
  q.exec();
  if (q.first())
  {
    int result = q.value("result").toInt();
    if (result < 0)
    {
      systemError(this, storedProcErrorLookup("deleteCustomerType", result),
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

void customerTypes::sPopulateMenu(QMenu *)
{
}

void customerTypes::sPrint()
{
  orReport report("CustomerTypesMasterList");
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}
