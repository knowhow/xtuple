/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "externalShipping.h"
#include "externalShippingList.h"

bool externalShippingList::userHasPriv()
{
  bool retval = _privileges->check("MaintainExternalShipping");
  return retval;
}

void externalShippingList::showEvent(QShowEvent *event)
{
  if (event) {
    _ship->setTable();
    _ship->setColumn(tr("So Number"),                 100, Qt::AlignLeft, true, "so_number");
    _ship->setColumn(tr("Shipment Number"),           100, Qt::AlignLeft, true, "shipment_number");
    _ship->setColumn(tr("Package Tracking Number"),   100, Qt::AlignLeft, true, "package_tracking_number");
    _ship->setColumn(tr("Void"),                       40, Qt::AlignLeft, true, "void");
    _ship->setColumn(tr("Billing Option"),            100, Qt::AlignLeft, false, "billing_option");
    _ship->setColumn(tr("Weight"),                     60, Qt::AlignRight,true,  "weight");
    _ship->setColumn(tr("Base Freight"),               60, Qt::AlignRight,false, "base_freight");
    _ship->setColumn(tr("Base Freight Currency"),      80, Qt::AlignLeft, false, "base_freight_currency");
    _ship->setColumn(tr("Total Freight"),              60, Qt::AlignRight,true,  "total_freight");
    _ship->setColumn(tr("Total Freight Currency"),     80, Qt::AlignLeft, true,  "total_freight_currency");
    _ship->setColumn(tr("Package Type"),              100, Qt::AlignLeft, false, "billing_option");
    _ship->setColumn(tr("Tracking Number"),           100, Qt::AlignLeft, false, "tracking_number");
    _ship->setColumn(tr("Last Updated"),              100, Qt::AlignLeft, false, "last_updated");
    _ship->setColumnFormat("weight", XSqlTableModel::Qty);
    _ship->setColumnFormat("base_freight", XSqlTableModel::Curr);
    _ship->setColumnFormat("total_freight", XSqlTableModel::Curr);
    _ship->select();
  }

  XWidget::showEvent(event);
}

void externalShippingList::setVisible(bool visible)
{
  if (! visible)
    XWidget::setVisible(false);
  else if (!userHasPriv())
  {
    systemError(this,
	        tr("You do not have sufficient privilege to view this window."),
		__FILE__, __LINE__);
    close();
  }
  else
    XWidget::setVisible(true);
}

externalShippingList::externalShippingList(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);
  
  connect(_delete, SIGNAL(clicked()), this, SLOT(sDelete()));
  connect(_new, SIGNAL(clicked()), this, SLOT(sNew()));
  connect(_edit, SIGNAL(clicked()), this, SLOT(sEdit()));
  connect(_view, SIGNAL(clicked()), this, SLOT(sView()));
}

externalShippingList::~externalShippingList()
{
    // no need to delete child widgets, Qt does it all for us
}

void externalShippingList::languageChange()
{
    retranslateUi(this);
}

void externalShippingList::sNew()
{
  sOpen(cNew);
}

void externalShippingList::sEdit()
{
  sOpen(cEdit);
}

void externalShippingList::sView()
{
  sOpen(cView);
}

void externalShippingList::sDelete()
{
  _ship->removeSelected();
  _ship->save();
}

void externalShippingList::sOpen(int mode)
{
  if (mode == cNew)
    _ship->clearSelection();
    
  ParameterList params;
  params.append("mode", mode);
  params.append("currentIndex",_ship->currentIndex());
  
  externalShipping newdlg(this, "", TRUE);
  newdlg.setModel(_ship->model());
  newdlg.set(params);
  newdlg.exec();
}
