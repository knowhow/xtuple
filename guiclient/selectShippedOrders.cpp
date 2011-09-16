/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "selectShippedOrders.h"

selectShippedOrders::selectShippedOrders(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
  : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  // signals and slots connections
  connect(_close, SIGNAL(clicked()), this, SLOT(reject()));
  connect(_select, SIGNAL(clicked()), this, SLOT(sSelect()));

  _customerType->setType(ParameterGroup::CustomerType);
}

selectShippedOrders::~selectShippedOrders()
{
  // no need to delete child widgets, Qt does it all for us
}

void selectShippedOrders::languageChange()
{
  retranslateUi(this);
}

void selectShippedOrders::sSelect()
{
  if (_customerType->isAll())
    q.prepare("SELECT selectUninvoicedShipments(:warehous_id) AS result;");
  else if (_customerType->isSelected())
  {
    q.prepare("SELECT selectUninvoicedShipments(:warehous_id, :custtype_id) AS result;");
    q.bindValue(":custtype_id", _customerType->id());
  }
  else if (_customerType->isPattern())
  {
    q.prepare("SELECT selectUninvoicedShipments(:warehous_id, :custtype_pattern) AS result;");
    q.bindValue(":custtype_pattern", _customerType->pattern());
  }

  q.bindValue(":warehous_id", ((_warehouse->isSelected()) ? _warehouse->id() : -1));
  q.exec();

  accept();
}
