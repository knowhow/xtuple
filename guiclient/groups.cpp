/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "groups.h"

#include <QVariant>
#include <QMessageBox>
//#include <QStatusBar>

#include <parameter.h>
#include <openreports.h>
#include "group.h"
#include "guiclient.h"

/*
 *  Constructs a groups as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
groups::groups(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

//  (void)statusBar();

  // signals and slots connections
  connect(_new, SIGNAL(clicked()), this, SLOT(sNew()));
  connect(_edit, SIGNAL(clicked()), this, SLOT(sEdit()));
  connect(_view, SIGNAL(clicked()), this, SLOT(sView()));
  connect(_delete, SIGNAL(clicked()), this, SLOT(sDelete()));
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));

//  statusBar()->hide();
  _print->hide();
  
  
  _list->addColumn(tr("Name"),        _itemColumn, Qt::AlignLeft, true, "grp_name" );
  _list->addColumn(tr("Description"), -1,          Qt::AlignLeft, true, "grp_descrip" );

  if (_privileges->check("MaintainGroups"))
  {
    connect(_list, SIGNAL(valid(bool)), _edit, SLOT(setEnabled(bool)));
    connect(_list, SIGNAL(valid(bool)), _delete, SLOT(setEnabled(bool)));
    connect(_list, SIGNAL(itemSelected(int)), _edit, SLOT(animateClick()));
  }
  else
  {
    connect(_list, SIGNAL(itemSelected(int)), _view, SLOT(animateClick()));
    _new->setEnabled(FALSE);
  }

   sFillList();
}

/*
 *  Destroys the object and frees any allocated resources
 */
groups::~groups()
{
  // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void groups::languageChange()
{
  retranslateUi(this);
}

void groups::sDelete()
{
  q.prepare( "SELECT usrgrp_id "
             "FROM usrgrp "
             "WHERE (usrgrp_grp_id=:grp_id);" );
  q.bindValue(":grp_id", _list->id());
  q.exec();
  if (q.first())
  {
    QMessageBox::critical( this, tr("Cannot Delete Group"),
                           tr( "The selected Group cannot be deleted as there are one or more Users currently assigned to it.\n"
                               "You must reassign these Users before you may delete the selected Group." ) );
    return;
  }

  q.prepare( "DELETE FROM grppriv"
             " WHERE (grppriv_grp_id=:grp_id);"
             "DELETE FROM grp "
             " WHERE (grp_id=:grp_id);" );
  q.bindValue(":grp_id", _list->id());
  q.exec();

  sFillList();
}

void groups::sNew()
{
  ParameterList params;
  params.append("mode", "new");

  group newdlg(this, "", TRUE);
  newdlg.set(params);
  
  newdlg.exec();
  sFillList();
}

void groups::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("grp_id", _list->id());

  group newdlg(this, "", TRUE);
  newdlg.set(params);
  
  newdlg.exec();
  sFillList();
}

void groups::sView()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("grp_id", _list->id());

  group newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void groups::sFillList()
{
  _list->populate( "SELECT grp_id, grp_name, grp_descrip "
                   "  FROM grp "
                   " ORDER BY grp_name;" );
}

void groups::sPrint()
{
  orReport report("GroupMasterList");
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}


