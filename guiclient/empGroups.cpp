/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "empGroups.h"

#include <QSqlError>

#include <parameter.h>

#include "empGroup.h"
#include "guiclient.h"
#include "storedProcErrorLookup.h"

empGroups::empGroups(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  connect(_new, SIGNAL(clicked()), this, SLOT(sNew()));
  connect(_edit, SIGNAL(clicked()), this, SLOT(sEdit()));
  connect(_delete, SIGNAL(clicked()), this, SLOT(sDelete()));
  connect(_view, SIGNAL(clicked()), this, SLOT(sView()));
  
  _empgrp->addColumn(tr("Name"), _itemColumn, Qt::AlignLeft, true, "empgrp_name");
  _empgrp->addColumn(tr("Description"),   -1, Qt::AlignLeft, true, "empgrp_descrip");
  
  if (_privileges->check("MaintainEmployeeGroups"))
  {
    connect(_empgrp, SIGNAL(valid(bool)), _edit, SLOT(setEnabled(bool)));
    connect(_empgrp, SIGNAL(valid(bool)), _delete, SLOT(setEnabled(bool)));
    connect(_empgrp, SIGNAL(itemSelected(int)), _edit, SLOT(animateClick()));
  }
  else
  {
    _new->setEnabled(FALSE);
    connect(_empgrp, SIGNAL(itemSelected(int)), _view, SLOT(animateClick()));
  }

  sFillList();
}

empGroups::~empGroups()
{
  // no need to delete child widgets, Qt does it all for us
}

void empGroups::languageChange()
{
  retranslateUi(this);
}

void empGroups::sDelete()
{
  q.prepare( "SELECT deleteEmpgrp(:grpid) AS result;");
  q.bindValue(":grpid", _empgrp->id());
  q.exec();
  if (q.first())
  {
    int result = q.value("result").toInt();
    if (result < 0)
    {
      systemError(this, storedProcErrorLookup("deleteEmpgrp", result),
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

void empGroups::sNew()
{
  ParameterList params;
  params.append("mode", "new");

  empGroup newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
  sFillList();
}

void empGroups::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("empgrp_id", _empgrp->id());

  empGroup newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
  sFillList();
}

void empGroups::sView()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("empgrp_id", _empgrp->id());

  empGroup newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void empGroups::sFillList()
{
  q.prepare("SELECT * FROM empgrp ORDER BY empgrp_name;" );
  q.exec();
  _empgrp->populate(q);
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}
