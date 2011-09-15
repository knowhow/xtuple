/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "incidentPriorities.h"

#include <QAction>
#include <QMenu>
#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include <openreports.h>

#include "incidentPriority.h"

incidentPriorities::incidentPriorities(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  connect(_close,  SIGNAL(clicked()), this, SLOT(close()));
  connect(_delete, SIGNAL(clicked()), this, SLOT(sDelete()));
  connect(_edit,   SIGNAL(clicked()), this, SLOT(sEdit()));
  connect(_incidentPriorities, SIGNAL(populateMenu(QMenu *, QTreeWidgetItem *, int)), this, SLOT(sPopulateMenu(QMenu*)));
  connect(_incidentPriorities, SIGNAL(valid(bool)), _view, SLOT(setEnabled(bool)));
  connect(_new,   SIGNAL(clicked()), this, SLOT(sNew()));
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_view,  SIGNAL(clicked()), this, SLOT(sView()));

  _incidentPriorities->addColumn(tr("Order"),  _seqColumn, Qt::AlignRight, true, "incdtpriority_order");
  _incidentPriorities->addColumn(tr("Priority"),      100, Qt::AlignLeft, true, "incdtpriority_name" );
  _incidentPriorities->addColumn(tr("Description"),    -1, Qt::AlignLeft, true, "incdtpriority_descrip" );

  if (_privileges->check("MaintainIncidentPriorities"))
  {
    connect(_incidentPriorities, SIGNAL(valid(bool)), _edit, SLOT(setEnabled(bool)));
    connect(_incidentPriorities, SIGNAL(valid(bool)), _delete, SLOT(setEnabled(bool)));
    connect(_incidentPriorities, SIGNAL(itemSelected(int)), _edit, SLOT(animateClick()));
  }
  else
  {
    _new->setEnabled(FALSE);
    connect(_incidentPriorities, SIGNAL(itemSelected(int)), _view, SLOT(animateClick()));
  }

  sFillList();
}

incidentPriorities::~incidentPriorities()
{
    // no need to delete child widgets, Qt does it all for us
}

void incidentPriorities::languageChange()
{
    retranslateUi(this);
}

void incidentPriorities::sFillList()
{
  q.prepare( "SELECT incdtpriority_id, incdtpriority_order, "
	     "       incdtpriority_name, firstLine(incdtpriority_descrip) AS incdtpriority_descrip "
             "FROM incdtpriority "
             "ORDER BY incdtpriority_order, incdtpriority_name;" );
  q.exec();
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
  _incidentPriorities->populate(q);
}

void incidentPriorities::sDelete()
{
  q.prepare( "DELETE FROM incdtpriority "
             "WHERE (incdtpriority_id=:incdtpriorityid);" );
  q.bindValue(":incdtpriorityid", _incidentPriorities->id());
  q.exec();
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  sFillList();
}

void incidentPriorities::sNew()
{
  ParameterList params;
  params.append("mode", "new");

  incidentPriority newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void incidentPriorities::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("incdtpriority_id", _incidentPriorities->id());

  incidentPriority newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void incidentPriorities::sView()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("incdtpriority_id", _incidentPriorities->id());

  incidentPriority newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void incidentPriorities::sPopulateMenu( QMenu *pMenu)
{
  QAction *menuItem;

  menuItem = pMenu->addAction(tr("Edit..."), this, SLOT(sEdit()));
  menuItem->setEnabled(_privileges->check("MaintainIncidentPriorities"));

  menuItem = pMenu->addAction(tr("View..."), this, SLOT(sView()));

  menuItem = pMenu->addAction(tr("Delete"), this, SLOT(sDelete()));
  menuItem->setEnabled(_privileges->check("MaintainIncidentPriorities"));
}

void incidentPriorities::sPrint()
{
  orReport report("IncidentPrioritiesList");
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}
