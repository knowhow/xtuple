/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "customCommands.h"

#include <QSqlError>

#include <parameter.h>

#include "customCommand.h"
#include "guiclient.h"

customCommands::customCommands(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

//  (void)statusBar();

  connect(_delete, SIGNAL(clicked()), this, SLOT(sDelete()));
  connect(_edit, SIGNAL(clicked()), this, SLOT(sEdit()));
  connect(_new, SIGNAL(clicked()), this, SLOT(sNew()));

  _commands->addColumn(tr("Module"), _itemColumn, Qt::AlignCenter,true, "cmd_module");
  _commands->addColumn(tr("Menu Label"),      -1, Qt::AlignLeft,  true, "cmd_title");
  _commands->addColumn(tr("Package"),_itemColumn, Qt::AlignLeft,  false,"nspname");

  sFillList();
}

customCommands::~customCommands()
{
  // no need to delete child widgets, Qt does it all for us
}

void customCommands::languageChange()
{
  retranslateUi(this);
}

void customCommands::sNew()
{
  ParameterList params;
  params.append("mode", "new");

  customCommand newdlg(this, "", TRUE);
  newdlg.set(params);
  if(newdlg.exec() == XDialog::Accepted)
    sFillList();
}

void customCommands::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("cmd_id", _commands->id());

  customCommand newdlg(this, "", TRUE);
  newdlg.set(params);
  if(newdlg.exec() == XDialog::Accepted)
    sFillList();
}

void customCommands::sDelete()
{
  q.prepare("BEGIN; "
            "DELETE FROM cmdarg WHERE (cmdarg_cmd_id=:cmd_id); "
            "DELETE FROM cmd WHERE (cmd_id=:cmd_id); "
            "SELECT updateCustomPrivs(); "
            "COMMIT; ");
  q.bindValue(":cmd_id", _commands->id());
  if(q.exec())
    sFillList();
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void customCommands::sFillList()
{
  q.exec("SELECT cmd.*,"
         "       CASE WHEN nspname='public' THEN ''"
         "            ELSE nspname END AS nspname"
         "  FROM cmd, pg_class, pg_namespace"
         "  WHERE ((cmd.tableoid=pg_class.oid)"
         "    AND  (relnamespace=pg_namespace.oid)"
         "    AND  (cmd_module IN ("
         "        'Products','Inventory','Schedule','Purchase', "
         "        'Manufacture','CRM','Sales','Accounting','System'))) "
         " ORDER BY cmd_module, cmd_title;");
  _commands->populate(q);
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}
