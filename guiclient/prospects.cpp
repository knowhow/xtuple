/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "prospects.h"

#include <QAction>
#include <QMenu>
#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include "errorReporter.h"
#include "parameterwidget.h"
#include "prospect.h"
#include "storedProcErrorLookup.h"

prospects::prospects(QWidget* parent, const char*, Qt::WFlags fl)
  : display(parent, "prospects", fl)
{
  setWindowTitle(tr("Prospects"));
  setMetaSQLOptions("prospects", "detail");
  setParameterWidgetVisible(true);
  setNewVisible(true);
  setSearchVisible(true);
  setQueryOnStartEnabled(true);

  parameterWidget()->append(tr("Show Inactive"), "showInactive", ParameterWidget::Exists);
  parameterWidget()->append(tr("Prospect Number Pattern"), "prospect_number_pattern", ParameterWidget::Text);
  parameterWidget()->append(tr("Prospect Name Pattern"), "prospect_name_pattern", ParameterWidget::Text);
  parameterWidget()->append(tr("Contact Name Pattern"), "cntct_name_pattern", ParameterWidget::Text);
  parameterWidget()->append(tr("Phone Pattern"), "cntct_phone_pattern", ParameterWidget::Text);
  parameterWidget()->append(tr("Email Pattern"), "cntct_email_pattern", ParameterWidget::Text);
  parameterWidget()->append(tr("Street Pattern"), "addr_street_pattern", ParameterWidget::Text);
  parameterWidget()->append(tr("City Pattern"), "addr_city_pattern", ParameterWidget::Text);
  parameterWidget()->append(tr("State Pattern"), "addr_state_pattern", ParameterWidget::Text);
  parameterWidget()->append(tr("Postal Code Pattern"), "addr_postalcode_pattern", ParameterWidget::Text);
  parameterWidget()->append(tr("Country Pattern"), "addr_country_pattern", ParameterWidget::Text);

  if (_privileges->check("MaintainProspectMasters"))
    connect(list(), SIGNAL(itemSelected(int)), this, SLOT(sEdit()));
  else
  {
    newAction()->setEnabled(FALSE);
    connect(list(), SIGNAL(itemSelected(int)), this, SLOT(sView()));
  }

  list()->addColumn(tr("Number"),  _orderColumn, Qt::AlignCenter, true, "prospect_number" );
  list()->addColumn(tr("Name"),    -1,           Qt::AlignLeft,   true, "prospect_name"   );
  list()->addColumn(tr("First"),   50, Qt::AlignLeft  , true, "cntct_first_name" );
  list()->addColumn(tr("Last"),    -1, Qt::AlignLeft  , true, "cntct_last_name" );
  list()->addColumn(tr("Phone"),   100, Qt::AlignLeft  , true, "cntct_phone" );
  list()->addColumn(tr("Email"),   100, Qt::AlignLeft  , true, "cntct_email" );
  list()->addColumn(tr("Address"), -1, Qt::AlignLeft  , false, "addr_line1" );
  list()->addColumn(tr("City"),    75, Qt::AlignLeft  , false, "addr_city" );
  list()->addColumn(tr("State"),   50, Qt::AlignLeft  , false, "addr_state" );
  list()->addColumn(tr("Country"), 100, Qt::AlignLeft  , false, "addr_country" );
  list()->addColumn(tr("Postal Code"), 75, Qt::AlignLeft  , false, "addr_postalcode" );

  connect(omfgThis, SIGNAL(prospectsUpdated()), SLOT(sFillList()));
}

void prospects::sNew()
{
  ParameterList params;
  params.append("mode", "new");

  prospect *newdlg = new prospect();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void prospects::sEdit()
{
  ParameterList params;
  params.append("prospect_id", list()->id());
  params.append("mode", "edit");

  prospect *newdlg = new prospect();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void prospects::sView()
{
  ParameterList params;
  params.append("prospect_id", list()->id());
  params.append("mode", "view");

  prospect *newdlg = new prospect();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void prospects::sDelete()
{
  if (QMessageBox::question(this, tr("Delete?"),
                            tr("<p>Are you sure you want to delete the "
                               "selected Prospect?"),
                            QMessageBox::Yes,
                            QMessageBox::No | QMessageBox::Default) == QMessageBox::No)
    return;

  XSqlQuery delq;
  delq.prepare("DELETE FROM prospect WHERE (prospect_id=:prospect_id);");
  delq.bindValue(":prospect_id", list()->id());
  delq.exec();
  if (ErrorReporter::error(QtCriticalMsg, this, tr("Error deleting"),
                           delq, __FILE__, __LINE__))
    return;
}

void prospects::sPopulateMenu(QMenu * pMenu, QTreeWidgetItem *, int)
{
  QAction *menuItem;

  menuItem = pMenu->addAction("View...", this, SLOT(sView()));

  menuItem = pMenu->addAction("Edit...", this, SLOT(sEdit()));
  menuItem->setEnabled(_privileges->check("MaintainProspectMasters"));

  menuItem = pMenu->addAction("Delete", this, SLOT(sDelete()));
  menuItem->setEnabled(_privileges->check("MaintainProspectMasters"));
}
