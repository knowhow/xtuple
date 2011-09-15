/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "incidentSeverities.h"

#include <QAction>
#include <QMenu>
#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include <openreports.h>
#include "incidentSeverity.h"

incidentSeverities::incidentSeverities(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  connect(_close,  SIGNAL(clicked()), this, SLOT(close()));
  connect(_delete, SIGNAL(clicked()), this, SLOT(sDelete()));
  connect(_edit,   SIGNAL(clicked()), this, SLOT(sEdit()));
  connect(_incidentSeverities, SIGNAL(populateMenu(QMenu *, QTreeWidgetItem *, int)), this, SLOT(sPopulateMenu(QMenu*)));
  connect(_incidentSeverities, SIGNAL(valid(bool)), _view, SLOT(setEnabled(bool)));
  connect(_new,   SIGNAL(clicked()), this, SLOT(sNew()));
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_view,  SIGNAL(clicked()), this, SLOT(sView()));

  _incidentSeverities->addColumn(tr("Order"),  _seqColumn, Qt::AlignRight, true, "incdtseverity_order");
  _incidentSeverities->addColumn(tr("Severity"),      100, Qt::AlignLeft, true, "incdtseverity_name" );
  _incidentSeverities->addColumn(tr("Description"),    -1, Qt::AlignLeft, true, "incdtseverity_descrip" );


  if (_privileges->check("MaintainIncidentSeverities"))
  {
    connect(_incidentSeverities, SIGNAL(valid(bool)), _edit, SLOT(setEnabled(bool)));
    connect(_incidentSeverities, SIGNAL(valid(bool)), _delete, SLOT(setEnabled(bool)));
    connect(_incidentSeverities, SIGNAL(itemSelected(int)), _edit, SLOT(animateClick()));
  }
  else
  {
    _new->setEnabled(FALSE);
    connect(_incidentSeverities, SIGNAL(itemSelected(int)), _view, SLOT(animateClick()));
  }

  sFillList();
}

incidentSeverities::~incidentSeverities()
{
    // no need to delete child widgets, Qt does it all for us
}

void incidentSeverities::languageChange()
{
    retranslateUi(this);
}

void incidentSeverities::sFillList()
{
  q.prepare( "SELECT incdtseverity_id, incdtseverity_order, "
	     "       incdtseverity_name, firstLine(incdtseverity_descrip) AS incdtseverity_descrip "
             "FROM incdtseverity "
             "ORDER BY incdtseverity_order, incdtseverity_name;" );
  q.exec();
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
  _incidentSeverities->populate(q);
}

void incidentSeverities::sDelete()
{
  q.prepare( "DELETE FROM incdtseverity "
             "WHERE (incdtseverity_id=:incdtseverity_id);" );
  q.bindValue(":incdtseverity_id", _incidentSeverities->id());
  q.exec();
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  sFillList();
}

void incidentSeverities::sNew()
{
  ParameterList params;
  params.append("mode", "new");

  incidentSeverity newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void incidentSeverities::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("incdtseverity_id", _incidentSeverities->id());

  incidentSeverity newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void incidentSeverities::sView()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("incdtseverity_id", _incidentSeverities->id());

  incidentSeverity newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void incidentSeverities::sPopulateMenu( QMenu *pMenu)
{
  QAction *menuItem;

  menuItem = pMenu->addAction(tr("Edit..."), this, SLOT(sEdit()));
  menuItem->setEnabled(_privileges->check("MaintainIncidentSeverities"));

  menuItem = pMenu->addAction(tr("View..."), this, SLOT(sView()));

  menuItem = pMenu->addAction(tr("Delete"), this, SLOT(sDelete()));
  menuItem->setEnabled(_privileges->check("MaintainIncidentSeverities"));
}

void incidentSeverities::sPrint()
{
  orReport report("IncidentSeveritiesList");
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}
