/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "scripts.h"

#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include <parameter.h>
#include "scriptEditor.h"
#include "guiclient.h"

scripts::scripts(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  connect(_new, SIGNAL(clicked()), this, SLOT(sNew()));
  connect(_edit, SIGNAL(clicked()), this, SLOT(sEdit()));
  connect(_delete, SIGNAL(clicked()), this, SLOT(sDelete()));
  
  connect(_script, SIGNAL(newId(int)), this, SLOT(sHandleButtons()));

  _script->addColumn(tr("Name"),   _itemColumn, Qt::AlignLeft,  true, "script_name");
  _script->addColumn(tr("Description"),     -1, Qt::AlignLeft,  true, "script_notes");
  _script->addColumn(tr("Order"),    _ynColumn, Qt::AlignCenter,true, "script_order");
  _script->addColumn(tr("Enabled"),  _ynColumn, Qt::AlignCenter,true, "script_enabled");
  _script->addColumn(tr("Package"),_itemColumn, Qt::AlignLeft,  true, "nspname");

  sFillList();
}

scripts::~scripts()
{
  // no need to delete child widgets, Qt does it all for us
}

void scripts::languageChange()
{
  retranslateUi(this);
}

void scripts::sNew()
{
  ParameterList params;
  params.append("mode", "new");

  scriptEditor *newdlg = new scriptEditor(this, "", Qt::Window);
  newdlg->set(params);

  omfgThis->handleNewWindow(newdlg);
  connect(newdlg, SIGNAL(destroyed()), this, SLOT(sFillList()));
}

void scripts::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("script_id", _script->id());

  scriptEditor *newdlg = new scriptEditor(this, "", Qt::Window);
  newdlg->set(params);

  omfgThis->handleNewWindow(newdlg);
  connect(newdlg, SIGNAL(destroyed()), this, SLOT(sFillList()));
}

void scripts::sDelete()
{
  if ( QMessageBox::warning(this, tr("Delete Script?"),
                            tr("<p>Are you sure that you want to completely "
			       "delete the selected script?"),
			    QMessageBox::Yes,
			    QMessageBox::No | QMessageBox::Default) == QMessageBox::Yes)
  {
    q.prepare( "DELETE FROM script "
               "WHERE (script_id=:script_id);" );
    q.bindValue(":script_id", _script->id());
    q.exec();
    if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }

    sFillList();
  }
}

void scripts::sFillList()
{
  XSqlQuery r;
  r.exec("SELECT script_id, script_name, script_order, script_enabled, script_notes,"
         "       CASE WHEN nspname='public' THEN ''"
         "            ELSE nspname END AS nspname,"
         "       CASE WHEN (pkghead_id IS NULL) THEN 0"
	 "	      ELSE 1 END AS xtindentrole"
         "  FROM script, pg_class, pg_namespace"
         "  LEFT OUTER JOIN pkghead ON (nspname=pkghead_name)"
         "  WHERE ((script.tableoid=pg_class.oid)"
         "    AND  (relnamespace=pg_namespace.oid))"
         " UNION "
         " SELECT -1, pkghead_name, NULL, NULL, pkghead_descrip, pkghead_name, 0 "
         " FROM script, pg_class, pg_namespace"
         "   RIGHT OUTER JOIN pkghead ON (nspname=pkghead_name)"
         " WHERE ((script.tableoid=pg_class.oid)"
         "   AND  (relnamespace=pg_namespace.oid))"
         " ORDER BY nspname, xtindentrole, script_name, script_order, script_id;" );
  
  _script->populate(r);
  if (r.lastError().type() != QSqlError::NoError)
  {
    systemError(this, r.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void scripts::sHandleButtons()
{
    if (_script->id() < 0)
    {
        _delete->setEnabled(false);
        _edit->setEnabled(false);       
    }
}
