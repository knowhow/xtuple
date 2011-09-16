/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "taxAuthorities.h"

#include <QMessageBox>
#include <QSqlError>

#include <parameter.h>

#include "errorReporter.h"
#include "parameterwidget.h"
#include "taxAuthority.h"

taxAuthorities::taxAuthorities(QWidget* parent, const char*, Qt::WFlags fl)
  : display(parent, "taxAuthorities", fl)
{
  setWindowTitle(tr("Tax Authorities"));
  setReportName("TaxAuthoritiesMasterList");
  setMetaSQLOptions("taxAuthorities", "detail");
  setParameterWidgetVisible(true);
  setNewVisible(true);
  setSearchVisible(true);
  setQueryOnStartEnabled(true);

  parameterWidget()->append(tr("Tax Authority Code Pattern"), "taxauth_code_pattern", ParameterWidget::Text);
  parameterWidget()->append(tr("Tax Authority Name Pattern"), "taxauth_name_pattern", ParameterWidget::Text);
  parameterWidget()->append(tr("Street Pattern"), "addr_street_pattern", ParameterWidget::Text);
  parameterWidget()->append(tr("City Pattern"), "addr_city_pattern", ParameterWidget::Text);
  parameterWidget()->append(tr("State Pattern"), "addr_state_pattern", ParameterWidget::Text);
  parameterWidget()->append(tr("Postal Code Pattern"), "addr_postalcode_pattern", ParameterWidget::Text);
  parameterWidget()->append(tr("Country Pattern"), "addr_country_pattern", ParameterWidget::Text);

  connect(omfgThis, SIGNAL(taxAuthsUpdated(int)), this, SLOT(sFillList()));

  list()->addColumn(tr("Code"), 70, Qt::AlignLeft,   true,  "taxauth_code" );
  list()->addColumn(tr("Name"), -1, Qt::AlignLeft,   true,  "taxauth_name" );
  list()->addColumn(tr("Address"), -1, Qt::AlignLeft  , true, "addr_line1" );
  list()->addColumn(tr("City"),    75, Qt::AlignLeft  , true, "addr_city" );
  list()->addColumn(tr("State"),   50, Qt::AlignLeft  , true, "addr_state" );
  list()->addColumn(tr("Country"), 100, Qt::AlignLeft  , true, "addr_country" );
  list()->addColumn(tr("Postal Code"), 75, Qt::AlignLeft  , false, "addr_postalcode" );

  if (_privileges->check("MaintainTaxAuthorities"))
    connect(list(), SIGNAL(itemSelected(int)), this, SLOT(sEdit()));
  else
  {
    newAction()->setEnabled(false);
    connect(list(), SIGNAL(itemSelected(int)), this, SLOT(sView()));
  }
}

void taxAuthorities::sDelete()
{
  if (QMessageBox::question(this, tr("Delete Tax Authority?"),
                            tr("<p>Are you sure you want to delete the "
                               "selected Tax Authority?"),
                            QMessageBox::Yes,
                            QMessageBox::No | QMessageBox::Default) == QMessageBox::No)
    return;

  XSqlQuery delq;
  delq.prepare("DELETE FROM taxauth WHERE (taxauth_id=:taxauth_id);");
  delq.bindValue(":taxauth_id", list()->id());
  delq.exec();
  if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Deleting"),
                           delq, __FILE__, __LINE__))
    return;
  sFillList();
}

void taxAuthorities::sNew()
{
  ParameterList params;
  params.append("mode", "new");

  taxAuthority newdlg(this, "", TRUE);
  newdlg.set(params);
  
  int result = newdlg.exec();
  if (result != XDialog::Rejected)
    sFillList();
}

void taxAuthorities::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("taxauth_id", list()->id());

  taxAuthority newdlg(this, "", TRUE);
  newdlg.set(params);
  
  int result = newdlg.exec();
  if (result != XDialog::Rejected)
    sFillList();
}

void taxAuthorities::sView()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("taxauth_id", list()->id());

  taxAuthority newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void taxAuthorities::sPopulateMenu(QMenu * pMenu, QTreeWidgetItem *, int)
{
  QAction *menuItem;

  menuItem = pMenu->addAction("View...", this, SLOT(sView()));

  menuItem = pMenu->addAction("Edit...", this, SLOT(sEdit()));
  menuItem->setEnabled(_privileges->check("MaintainTaxAuthorities"));

  menuItem = pMenu->addAction("Delete", this, SLOT(sDelete()));
  menuItem->setEnabled(_privileges->check("MaintainTaxAuthorities"));
}

