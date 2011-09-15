/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "incidentResolutions.h"

#include <QAction>
#include <QMenu>
#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include <openreports.h>

#include "incidentResolution.h"

incidentResolutions::incidentResolutions(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  connect(_close,  SIGNAL(clicked()), this, SLOT(close()));
  connect(_delete, SIGNAL(clicked()), this, SLOT(sDelete()));
  connect(_edit,   SIGNAL(clicked()), this, SLOT(sEdit()));
  connect(_incidentResolutions, SIGNAL(populateMenu(QMenu *, QTreeWidgetItem *, int)), this, SLOT(sPopulateMenu(QMenu*)));
  connect(_incidentResolutions, SIGNAL(valid(bool)), _view, SLOT(setEnabled(bool)));
  connect(_new,   SIGNAL(clicked()), this, SLOT(sNew()));
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_view,  SIGNAL(clicked()), this, SLOT(sView()));

  _incidentResolutions->addColumn(tr("Order"),  _seqColumn, Qt::AlignRight, true,  "incdtresolution_order");
  _incidentResolutions->addColumn(tr("Resolution"),      100, Qt::AlignLeft, true, "incdtresolution_name" );
  _incidentResolutions->addColumn(tr("Description"),    -1, Qt::AlignLeft, true,  "incdtresolution_descrip" );


  if (_privileges->check("MaintainIncidentResolutions"))
  {
    connect(_incidentResolutions, SIGNAL(valid(bool)), _edit, SLOT(setEnabled(bool)));
    connect(_incidentResolutions, SIGNAL(valid(bool)), _delete, SLOT(setEnabled(bool)));
    connect(_incidentResolutions, SIGNAL(itemSelected(int)), _edit, SLOT(animateClick()));
  }
  else
  {
    _new->setEnabled(FALSE);
    connect(_incidentResolutions, SIGNAL(itemSelected(int)), _view, SLOT(animateClick()));
  }

  sFillList();
}

incidentResolutions::~incidentResolutions()
{
    // no need to delete child widgets, Qt does it all for us
}

void incidentResolutions::languageChange()
{
    retranslateUi(this);
}

void incidentResolutions::sFillList()
{
  q.prepare( "SELECT incdtresolution_id, incdtresolution_order, "
	     "       incdtresolution_name, firstLine(incdtresolution_descrip) AS incdtresolution_descrip "
             "FROM incdtresolution "
             "ORDER BY incdtresolution_order, incdtresolution_name;" );
  q.exec();
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
  _incidentResolutions->populate(q);
}

void incidentResolutions::sDelete()
{
  q.prepare( "DELETE FROM incdtresolution "
             "WHERE (incdtresolution_id=:incdtresolution_id);" );
  q.bindValue(":incdtresolution_id", _incidentResolutions->id());
  q.exec();
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  sFillList();
}

void incidentResolutions::sNew()
{
  ParameterList params;
  params.append("mode", "new");

  incidentResolution newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void incidentResolutions::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("incdtresolution_id", _incidentResolutions->id());

  incidentResolution newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void incidentResolutions::sView()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("incdtresolution_id", _incidentResolutions->id());

  incidentResolution newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void incidentResolutions::sPopulateMenu( QMenu *pMenu)
{
  QAction *menuItem;

  menuItem = pMenu->addAction(tr("Edit..."), this, SLOT(sEdit()));
  menuItem->setEnabled(_privileges->check("MaintainIncidentResolutions"));

  menuItem = pMenu->addAction(tr("View..."), this, SLOT(sView()));

  menuItem = pMenu->addAction(tr("Delete"), this, SLOT(sDelete()));
  menuItem->setEnabled(_privileges->check("MaintainIncidentResolutions"));
}

void incidentResolutions::sPrint()
{
  orReport report("IncidentResolutionsList");
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}
