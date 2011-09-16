/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspCustomersByCharacteristic.h"

#include <QAction>
#include <QMenu>
#include <QVariant>

#include "characteristicAssignment.h"
#include "customer.h"
#include "guiclient.h"

dspCustomersByCharacteristic::dspCustomersByCharacteristic(QWidget* parent, const char*, Qt::WFlags fl)
  : display(parent, "dspCustomersByCharacteristic", fl)
{
  setupUi(optionsWidget());
  setWindowTitle(tr("Customers by Characteristic"));
  setListLabel(tr("Characteristic"));
  setReportName("CustomersByCharacteristic");
  setMetaSQLOptions("customer", "detail");
  setUseAltId(true);

  _char->populate( "SELECT char_id, char_name "
                   "FROM char "
                   "WHERE (char_customers) "
                   "ORDER BY char_name;" );

  list()->addColumn(tr("Active"),         _ynColumn,   Qt::AlignLeft,  true, "cust_active");
  list()->addColumn(tr("Number"),         _itemColumn, Qt::AlignLeft,  true, "cust_number");
  list()->addColumn(tr("Name"),           -1,          Qt::AlignLeft,  true, "cust_name");
  list()->addColumn(tr("Characteristic"), _itemColumn, Qt::AlignCenter,true, "char_name");
  list()->addColumn(tr("Value"),          _itemColumn, Qt::AlignLeft,  true, "charass_value");
  list()->setDragString("custid=");

  connect(omfgThis, SIGNAL(itemsUpdated(int, bool)), this, SLOT(sFillList(int, bool)));
}

void dspCustomersByCharacteristic::languageChange()
{
  display::languageChange();
  retranslateUi(this);
}

bool dspCustomersByCharacteristic::setParams(ParameterList &params)
{
  params.append("char_id", _char->id());
  params.append("value", _value->text());

  if (_showInactive->isChecked())
    params.append("showInactive");
  if (_emptyValue->isChecked())
    params.append("emptyValue");
  if(_charIsSet->isChecked())
    params.append("hasCharacteristic");

  return true;
}

void dspCustomersByCharacteristic::sPopulateMenu(QMenu *pMenu, QTreeWidgetItem * /*pSelected*/, int)
{
  QAction *menuItem;

  menuItem = pMenu->addAction(tr("Edit Customer..."), this, SLOT(sEdit()));
  if (!_privileges->check("MaintainCustomerMasters"))
    menuItem->setEnabled(false);

  menuItem = pMenu->addAction(tr("View Customer..."), this, SLOT(sView()));

  q.prepare( "SELECT charass_id FROM charass WHERE (charass_target_id=:cust_id)" );
  q.bindValue(":cust_id", list()->id());
  q.exec();
  if (!(q.first()))
  {
    menuItem = pMenu->addAction(tr("New Characteristic..."), this, SLOT(sNewCharacteristic()));
    if (!_privileges->check("MaintainCustomerMasters"))
      menuItem->setEnabled(false);
  }
  else
  {
    menuItem = pMenu->addAction(tr("Edit Characteristic..."), this, SLOT(sEditCharacteristic()));
    if (!_privileges->check("MaintainCustomerMasters"))
      menuItem->setEnabled(false);
  }
}

void dspCustomersByCharacteristic::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("cust_id", list()->id());

  customer *newdlg = new customer();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspCustomersByCharacteristic::sView()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("cust_id", list()->id());

  customer *newdlg = new customer();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspCustomersByCharacteristic::sNewCharacteristic()
{
  ParameterList params;
  params.append("mode", "new");
  params.append("cust_id", list()->id());
  params.append("char_id", _char->id());

  characteristicAssignment newdlg(this, "", true);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void dspCustomersByCharacteristic::sEditCharacteristic()
{
  ParameterList params;
  params.append("mode", "edit");

  q.prepare( "SELECT charass_id FROM charass WHERE (charass_target_id=:cust_id)" );
  q.bindValue(":cust_id", list()->id());
  q.exec();
  if (q.first())
  {
    int _charassId = q.value("charass_id").toInt();
    params.append("charass_id", _charassId);
  }
  characteristicAssignment newdlg(this, "", true);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

