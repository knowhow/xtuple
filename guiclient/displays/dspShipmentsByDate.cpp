/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

// TODO: add checkboxes to distinguish between sales and transfer orders
#include "dspShipmentsByDate.h"

#include <QAction>
#include <QMenu>
#include <QMessageBox>
#include <QVariant>

#include "inputManager.h"
#include "printShippingForm.h"

dspShipmentsByDate::dspShipmentsByDate(QWidget* parent, const char*, Qt::WFlags fl)
  : display(parent, "dspShipmentsByDate", fl)
{
  setupUi(optionsWidget());
  setWindowTitle(tr("Shipments by Date"));
  setListLabel(tr("Shipments"));
  setReportName("ShipmentsByDate");
  setMetaSQLOptions("shipments", "detail");
  setUseAltId(true);

  list()->setRootIsDecorated(true);
  list()->addColumn(tr("Shipment #"),         _orderColumn, Qt::AlignLeft,   true,  "shiphead_number"  );
  list()->addColumn(tr("Order Type"),	                 80, Qt::AlignLeft,   true,  "shiphead_order_type"  );
  list()->addColumn(tr("Ship Date"),           _itemColumn, Qt::AlignCenter, true,  "shiphead_shipdate");
  list()->addColumn(tr("#"),                    _seqColumn, Qt::AlignCenter, true,  "linenumber");
  list()->addColumn(tr("S/O #/Item"),          _itemColumn, Qt::AlignLeft,   true,  "order_item"  );
  list()->addColumn(tr("Customer/Description"),         -1, Qt::AlignLeft,   true,  "cust_desc"  );
  list()->addColumn(tr("Site"),                 _whsColumn, Qt::AlignCenter, true,  "warehous_code");
  list()->addColumn(tr("Ordered"),              _qtyColumn, Qt::AlignRight,  true,  "qtyord" );
  list()->addColumn(tr("Shipped"),              _qtyColumn, Qt::AlignRight,  true,  "qtyshipped" );
  list()->addColumn(tr("Tracking #"),           _qtyColumn, Qt::AlignRight,  true,  "shiphead_tracknum" );
  list()->addColumn(tr("Freight at Shipping"),  _qtyColumn, Qt::AlignRight,  true,  "shiphead_freight" );
  list()->addColumn(tr("Currency"),        _currencyColumn, Qt::AlignRight,  true,  "freight_curr_abbr" );
}

void dspShipmentsByDate::languageChange()
{
  display::languageChange();
  retranslateUi(this);
}

enum SetResponse dspShipmentsByDate::set(const ParameterList &pParams)
{ 
  XWidget::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("startDate", &valid);
  if (valid)
    _dates->setStartDate(param.toDate());
  
  param = pParams.value("endDate", &valid);
  if (valid)
    _dates->setEndDate(param.toDate());
  
  if (pParams.inList("run"))
  {
    sFillList();
    return NoError_Run;
  }

  return NoError;
}

void dspShipmentsByDate::sPopulateMenu(QMenu *pMenu, QTreeWidgetItem *, int)
{
  QAction *menuItem;

  menuItem = pMenu->addAction(tr("Print Shipping Form..."), this, SLOT(sPrintShippingForm()));
  menuItem->setEnabled(_privileges->check("PrintBillsOfLading"));
}

bool dspShipmentsByDate::setParams(ParameterList & params)
{
  if (!_dates->allValid())
  {
    QMessageBox::warning( this, tr("Enter a Valid Start Date and End Date"),
                          tr("You must enter a valid Start Date and End Date for this report.") );
    _dates->setFocus();
    return false;
  }

  if (_metrics->boolean("MultiWhs"))
    params.append("MultiWhs");
  _dates->appendValue(params);

  return true;
}

void dspShipmentsByDate::sPrintShippingForm()
{
  ParameterList params;
  params.append("shiphead_id", list()->id());

  printShippingForm newdlg(this);
  newdlg.set(params);
  newdlg.exec();
}

