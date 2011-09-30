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

#include <mqlutil.h>
#include <parameter.h>

#include "errorReporter.h"
#include "guiclient.h"
#include "scriptEditor.h"

scripts::scripts(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  connect(_byPackage, SIGNAL(toggled(bool)), this, SLOT(sFillList()));
  connect(_delete,        SIGNAL(clicked()), this, SLOT(sDelete()));
  connect(_edit,          SIGNAL(clicked()), this, SLOT(sEdit()));
  connect(_new,           SIGNAL(clicked()), this, SLOT(sNew()));
  connect(_script,       SIGNAL(newId(int)), this, SLOT(sHandleButtons()));

  _script->addColumn(tr("Name"),   _itemColumn, Qt::AlignLeft,  true, "script_name");
  _script->addColumn(tr("Description"),     -1, Qt::AlignLeft,  true, "script_notes");
  _script->addColumn(tr("Order"),    _ynColumn, Qt::AlignCenter,true, "script_order");
  _script->addColumn(tr("Enabled"),  _ynColumn, Qt::AlignCenter,true, "script_enabled");
  _script->addColumn(tr("Package"),_itemColumn, Qt::AlignLeft,  true, "pkgname");

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
  if (QMessageBox::question(this, tr("Delete Script?"),
                            tr("<p>Are you sure that you want to completely "
			       "delete the selected script?"),
			    QMessageBox::Yes,
			    QMessageBox::No | QMessageBox::Default) == QMessageBox::Yes)
  {
    XSqlQuery delq;
    delq.prepare( "DELETE FROM script WHERE (script_id=:script_id);" );
    delq.bindValue(":script_id", _script->id());
    delq.exec();
    if (ErrorReporter::error(QtCriticalMsg, this, tr("Deleting Script"),
                             delq, __FILE__, __LINE__))
      return;

    sFillList();
  }
}

bool scripts::setParams(ParameterList &params)
{
  if (_byPackage->isChecked())
    params.append("byPackage");

  return true;
}

void scripts::sFillList()
{
  QString errmsg;
  bool    ok;
  MetaSQLQuery  getm = MQLUtil::mqlLoad("scripts", "detail", errmsg, &ok);
  if (! ok)
  {
    ErrorReporter::error(QtCriticalMsg, this, tr("Getting Scripts"),
                         errmsg, __FILE__, __LINE__);
    return;
  }

  ParameterList getp;
  if (! setParams(getp))
    return;

  XSqlQuery getq = getm.toQuery(getp);
  _script->populate(getq);
  if (ErrorReporter::error(QtCriticalMsg, this, tr("Getting Scripts"),
                           getq, __FILE__, __LINE__))
    return;
}

void scripts::sHandleButtons()
{
    if (_script->id() < 0)
    {
        _delete->setEnabled(false);
        _edit->setEnabled(false);       
    }
}
