/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "users.h"

#include <QVariant>
#include <QMessageBox>
//#include <QStatusBar>
#include <parameter.h>
#include <openreports.h>
#include "user.h"

users::users(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  // signals and slots connections
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_new, SIGNAL(clicked()), this, SLOT(sNew()));
  connect(_edit, SIGNAL(clicked()), this, SLOT(sEdit()));
  connect(_usr, SIGNAL(itemSelected(int)), _edit, SLOT(animateClick()));
  connect(_usr, SIGNAL(valid(bool)), _edit, SLOT(setEnabled(bool)));
  connect(_close, SIGNAL(clicked()), this, SLOT(close()));
  connect(_showInactive, SIGNAL(toggled(bool)), this, SLOT(sFillList()));

  _usr->addColumn(tr("Username"),    80, Qt::AlignLeft,   true,  "usr_username"   );
  _usr->addColumn(tr("Proper Name"), -1, Qt::AlignLeft,   true,  "usr_propername"   );
  _usr->addColumn(tr("Status"),      50, Qt::AlignCenter, true,  "status" );

  q.exec("SELECT userCanCreateUsers(CURRENT_USER) AS cancreate;");
  if (q.first())
    _new->setEnabled(q.value("cancreate").toBool());
  else
    systemError(this, tr("A System Error occurred at %1::%2.")
                      .arg(__FILE__)
                      .arg(__LINE__) );
     
  sFillList();
}

users::~users()
{
  // no need to delete child widgets, Qt does it all for us
}

void users::languageChange()
{
  retranslateUi(this);
}

void users::sFillList()
{
  QString sql( "SELECT usr_id, usr_username, usr_propername,"
               "       CASE WHEN (usr_active) THEN :active"
               "            ELSE :inactive"
               "       END AS status "
               "FROM usr ");

  if (!_showInactive->isChecked())
    sql += "WHERE (usr_active) ";

  sql += "ORDER BY usr_username;";

  q.prepare(sql);
  q.bindValue(":active", tr("Active"));
  q.bindValue(":inactive", tr("Inactive"));
  q.exec();
  _usr->populate(q);
}

void users::sNew()
{
  ParameterList params;
  params.append("mode", "new");

  user newdlg(this);
  newdlg.set(params);

  newdlg.exec();
  sFillList();
}

void users::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("username", _usr->selectedItems().first()->text(0));

  user newdlg(this);
  newdlg.set(params);

  newdlg.exec();
  sFillList();
}

void users::sPrint()
{
  ParameterList params;

  if(_showInactive->isChecked())
    params.append("showInactive");

  orReport report("UsersMasterList", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

