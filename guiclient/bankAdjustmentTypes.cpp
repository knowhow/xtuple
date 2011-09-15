/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "bankAdjustmentTypes.h"

#include <QMenu>
#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include <parameter.h>
#include <openreports.h>

#include "bankAdjustmentType.h"
#include "storedProcErrorLookup.h"

bankAdjustmentTypes::bankAdjustmentTypes(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  connect(_new, SIGNAL(clicked()), this, SLOT(sNew()));
  connect(_edit, SIGNAL(clicked()), this, SLOT(sEdit()));
  connect(_view, SIGNAL(clicked()), this, SLOT(sView()));
  connect(_delete, SIGNAL(clicked()), this, SLOT(sDelete()));
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_bankadjtype, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*)));

  if (_privileges->check("MaintainAdjustmentTypes"))
  {
    connect(_bankadjtype, SIGNAL(valid(bool)), _edit, SLOT(setEnabled(bool)));
    connect(_bankadjtype, SIGNAL(valid(bool)), _delete, SLOT(setEnabled(bool)));
    connect(_bankadjtype, SIGNAL(itemSelected(int)), _edit, SLOT(animateClick()));
  }
  else
  {
    _new->setEnabled(FALSE);
    connect(_bankadjtype, SIGNAL(itemSelected(int)), _view, SLOT(animateClick()));
  }

  _bankadjtype->addColumn(tr("Name"), _itemColumn, Qt::AlignLeft, true, "bankadjtype_name");
  _bankadjtype->addColumn(tr("Description"),   -1, Qt::AlignLeft, true, "bankadjtype_descrip");
    
  sFillList();
}

bankAdjustmentTypes::~bankAdjustmentTypes()
{
  // no need to delete child widgets, Qt does it all for us
}

void bankAdjustmentTypes::languageChange()
{
  retranslateUi(this);
}

void bankAdjustmentTypes::sNew()
{
  ParameterList params;
  params.append("mode", "new");

  bankAdjustmentType newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void bankAdjustmentTypes::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("bankadjtype_id", _bankadjtype->id());

  bankAdjustmentType newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void bankAdjustmentTypes::sView()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("bankadjtype_id", _bankadjtype->id());

  bankAdjustmentType newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void bankAdjustmentTypes::sDelete()
{
  q.prepare( "SELECT deleteBankAdjustmentType(:bankadjtype_id) AS result;" );
  q.bindValue(":bankadjtype_id", _bankadjtype->id());
  q.exec();
  if (q.first())
  {
    int result = q.value("result").toInt();
    if (result < 0)
    {
      systemError(this,
		  storedProcErrorLookup("deleteBankAdjustmentType", result),
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

void bankAdjustmentTypes::sFillList()
{
  _bankadjtype->populate( "SELECT bankadjtype_id, bankadjtype_name, bankadjtype_descrip "
                      "FROM bankadjtype "
                      "ORDER BY bankadjtype_name;" );
}

void bankAdjustmentTypes::sPopulateMenu( QMenu * )
{
}

void bankAdjustmentTypes::sPrint()
{
  orReport report("AdjustmentTypes");
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

