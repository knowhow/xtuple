/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "vendors.h"

#include <QAction>
#include <QMenu>
#include <QMessageBox>

#include <openreports.h>
#include "vendor.h"
#include "storedProcErrorLookup.h"
#include "parameterwidget.h"

vendors::vendors(QWidget* parent, const char*, Qt::WFlags fl)
  : display(parent, "vendors", fl)
{
  setWindowTitle(tr("Vendors"));
  setReportName("VendorMasterList");
  setMetaSQLOptions("vendors", "detail");
  setParameterWidgetVisible(true);
  setNewVisible(true);
  setSearchVisible(true);
  setQueryOnStartEnabled(true);

  parameterWidget()->append(tr("Show Inactive"), "showInactive", ParameterWidget::Exists);
  parameterWidget()->append(tr("Vendor Number Pattern"), "vend_number_pattern", ParameterWidget::Text);
  parameterWidget()->append(tr("Vendor Name Pattern"), "vend_name_pattern", ParameterWidget::Text);
  parameterWidget()->append(tr("Vendor Type Pattern"), "vendtype_code_pattern", ParameterWidget::Text);
  parameterWidget()->append(tr("Contact Name Pattern"), "cntct_name_pattern", ParameterWidget::Text);
  parameterWidget()->append(tr("Phone Pattern"), "cntct_phone_pattern", ParameterWidget::Text);
  parameterWidget()->append(tr("Email Pattern"), "cntct_email_pattern", ParameterWidget::Text);
  parameterWidget()->append(tr("Street Pattern"), "addr_street_pattern", ParameterWidget::Text);
  parameterWidget()->append(tr("City Pattern"), "addr_city_pattern", ParameterWidget::Text);
  parameterWidget()->append(tr("State Pattern"), "addr_state_pattern", ParameterWidget::Text);
  parameterWidget()->append(tr("Postal Code Pattern"), "addr_postalcode_pattern", ParameterWidget::Text);
  parameterWidget()->append(tr("Country Pattern"), "addr_country_pattern", ParameterWidget::Text);

  list()->addColumn(tr("Type"),   _itemColumn, Qt::AlignCenter, true, "vendtype_code");
  list()->addColumn(tr("Number"), _itemColumn, Qt::AlignLeft,   true, "vend_number");
  list()->addColumn(tr("Name"),   -1,          Qt::AlignLeft,   true, "vend_name");
  list()->addColumn(tr("First"),   50, Qt::AlignLeft  , true, "cntct_first_name" );
  list()->addColumn(tr("Last"),    -1, Qt::AlignLeft  , true, "cntct_last_name" );
  list()->addColumn(tr("Phone"),   100, Qt::AlignLeft  , true, "cntct_phone" );
  list()->addColumn(tr("Email"),   100, Qt::AlignLeft  , true, "cntct_email" );
  list()->addColumn(tr("Address"), -1, Qt::AlignLeft  , false, "addr_line1" );
  list()->addColumn(tr("City"),    75, Qt::AlignLeft  , false, "addr_city" );
  list()->addColumn(tr("State"),   50, Qt::AlignLeft  , false, "addr_state" );
  list()->addColumn(tr("Country"), 100, Qt::AlignLeft  , false, "addr_country" );
  list()->addColumn(tr("Postal Code"), 75, Qt::AlignLeft  , false, "addr_postalcode" );

  connect(omfgThis, SIGNAL(vendorsUpdated()), SLOT(sFillList()));

  if (_privileges->check("MaintainVendors"))
    connect(list(), SIGNAL(itemSelected(int)), this, SLOT(sEdit()));
  else
  {
    newAction()->setEnabled(FALSE);
    connect(list(), SIGNAL(itemSelected(int)), this, SLOT(sView()));
  }
}

void vendors::sNew()
{
  ParameterList params;
  params.append("mode", "new");

  vendor *newdlg = new vendor();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void vendors::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("vend_id", list()->id());
  params.append("showNextPrev");

  vendor *newdlg = new vendor();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void vendors::sView()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("vend_id", list()->id());
  params.append("showNextPrev");

  vendor *newdlg = new vendor();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void vendors::sDelete()
{
  QString question = tr("Are you sure that you want to delete this vendor?");
  if (QMessageBox::question(this, tr("Delete Vendor?"),
                              question,
                              QMessageBox::Yes,
                              QMessageBox::No | QMessageBox::Default) == QMessageBox::No)
    return;

  q.prepare("SELECT deleteVendor(:vend_id) AS result;");
  q.bindValue(":vend_id", list()->id());
  q.exec();
  if (q.first())
  {
    int result = q.value("result").toInt();
    if (result < 0)
    {
      QMessageBox::critical( this, tr("Cannot Delete Vendor"),
			     storedProcErrorLookup("deleteVendor", result));
      return;
    }
    omfgThis->sVendorsUpdated();
    sFillList();
  }
}

void vendors::sPopulateMenu(QMenu * pMenu, QTreeWidgetItem *, int)
{
  QAction *menuItem;

  menuItem = pMenu->addAction(tr("Edit Vendor..."), this, SLOT(sEdit()));
  menuItem->setEnabled(_privileges->check("MaintainVendors"));

  menuItem = pMenu->addAction(tr("View Vendor..."), this, SLOT(sView()));
  menuItem->setEnabled(_privileges->check("MaintainVendors") || _privileges->check("ViewVendors"));

  menuItem = pMenu->addAction(tr("Delete Vendor..."), this, SLOT(sDelete()));
  menuItem->setEnabled(_privileges->check("MaintainVendors"));
}

