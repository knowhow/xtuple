/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "calendars.h"

#include <QMessageBox>
#include <QVariant>

#include <parameter.h>

#include "calendar.h"

calendars::calendars(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  connect(_new, SIGNAL(clicked()), this, SLOT(sNew()));
  connect(_edit, SIGNAL(clicked()), this, SLOT(sEdit()));
  connect(_delete, SIGNAL(clicked()), this, SLOT(sDelete()));
  
  _calhead->addColumn(tr("Name"), _itemColumn, Qt::AlignLeft, true, "calhead_name");
  _calhead->addColumn(tr("Description"),   -1, Qt::AlignLeft, true, "calhead_descrip");

  sFillList();
}

calendars::~calendars()
{
  // no need to delete child widgets, Qt does it all for us
}

void calendars::languageChange()
{
  retranslateUi(this);
}

void calendars::sNew()
{
  int calType = QMessageBox::information( this, tr("New Calendar Type?"),
                                          tr("What type of Calendar do you want to create?"),
                                          tr("Cancel"), tr("Absolute"), tr("Relative"), 1, 0 );

  if (calType != 0)
  {
    ParameterList params;
    params.append("mode", "new");

    if (calType == 1)
      params.append("type", "absolute");
    else if (calType == 2)
      params.append("type", "relative");

    calendar newdlg(this, "", TRUE);
    newdlg.set(params);
    if (newdlg.exec() != XDialog::Rejected)
      sFillList();
  }
}

void calendars::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("calhead_id", _calhead->id());

  calendar newdlg(this, "", TRUE);
  newdlg.set(params);
  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void calendars::sDelete()
{
  QString sql( "DELETE FROM calhead "
               "WHERE (calhead_id=:calhead_id);" );

  if (_calhead->altId() == 1)
    sql += "DELETE FROM acalitem "
           "WHERE (acalitem_calhead_id=:calhead_id);";
  else if (_calhead->altId() == 2)
    sql += "DELETE FROM rcalitem "
           "WHERE (rcalitem_calhead_id=:calhead_id);";

  q.prepare(sql);
  q.bindValue(":calhead_id", _calhead->id());
  q.exec();

  sFillList();
}

void calendars::sFillList()
{
  _calhead->populate( "SELECT calhead_id,"
                      "       CASE WHEN (calhead_type='A') THEN 1"
                      "            WHEN (calhead_type='R') THEN 2"
                      "            ELSE 3"
                      "       END,"
                      "       calhead_name, calhead_descrip "
                      "FROM calhead "
                      "ORDER BY calhead_name;", TRUE );
}
