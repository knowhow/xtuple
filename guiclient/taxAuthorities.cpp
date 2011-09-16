/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "taxAuthorities.h"

#include <QMessageBox>
#include <parameter.h>

#include "taxAuthority.h"
#include "parameterwidget.h"

/*
 *  Constructs a taxAuthorities as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
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

  connect(omfgThis, SIGNAL(taxAuthsUpdated(int)), this, SLOT(sFillList(int)));

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
  q.prepare("SELECT deleteTaxAuthority(:taxauth_id) AS result;");
  q.bindValue(":taxauth_id", list()->id());
  q.exec();
  if (q.first())
  {
    if(q.value("result").toInt() < 0)
    {
      QMessageBox::warning( this, tr("Cannot Delete Tax Authority"),
                            tr( "You cannot delete the selected Tax Authority because there are currently items assigned to it.\n"
                                "You must first re-assign these items before deleting the selected Tax Authority." ) );
      return;
    }

    omfgThis->sTaxAuthsUpdated(list()->id());
    sFillList();
  }
  else
    systemError(this, tr("A System Error occurred at %1::%2.")
                      .arg(__FILE__)
                      .arg(__LINE__) );
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

