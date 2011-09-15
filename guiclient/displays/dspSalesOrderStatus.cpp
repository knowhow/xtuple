/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspSalesOrderStatus.h"

#include <QSqlError>
#include <QVariant>

#include "inputManager.h"
#include "salesOrderList.h"

dspSalesOrderStatus::dspSalesOrderStatus(QWidget* parent, const char*, Qt::WFlags fl)
  : display(parent, "dspSalesOrderStatus", fl)
{
  setupUi(optionsWidget());
  setWindowTitle(tr("Sales Order Status"));
  setListLabel(tr("Shipment Status"));
  setReportName("SalesOrderStatus");
  setMetaSQLOptions("salesOrderStatus", "detail");

  _so->setAllowedTypes(OrderLineEdit::Sales);

  omfgThis->inputManager()->notify(cBCSalesOrder, this, _so, SLOT(setId(int)));

  list()->addColumn(tr("#"),                   _seqColumn,  Qt::AlignCenter, true,  "coitem_linenumber" );
  list()->addColumn(tr("Item"),                _itemColumn, Qt::AlignLeft,   true,  "item_number"   );
  list()->addColumn(tr("Description"),         -1,          Qt::AlignLeft,   true,  "itemdescrip"   );
  list()->addColumn(tr("Site"),                _whsColumn,  Qt::AlignCenter, true,  "warehous_code" );
  list()->addColumn(tr("Ordered"),             _qtyColumn,  Qt::AlignRight,  true,  "coitem_qtyord"  );
  list()->addColumn(tr("Shipped"),             _qtyColumn,  Qt::AlignRight,  true,  "coitem_qtyshipped"  );
  list()->addColumn(tr("Returned"),            _qtyColumn,  Qt::AlignRight,  true,  "coitem_qtyreturned"  );
  list()->addColumn(tr("Invoiced"),            _qtyColumn,  Qt::AlignRight,  true,  "invoiced"  );
  list()->addColumn(tr("Balance"),             _qtyColumn,  Qt::AlignRight,  true,  "balance"  );
  list()->addColumn(tr("Close Date"),          _dateColumn, Qt::AlignLeft,   true,  "closedate"  );
  list()->addColumn(tr("Close User"),          _itemColumn, Qt::AlignLeft,   true,  "closeuser"  );
  list()->addColumn(tr("Child Ord. #/Status"), _itemColumn, Qt::AlignCenter, true,  "childinfo" );

  _so->setFocus();
}

void dspSalesOrderStatus::languageChange()
{
  display::languageChange();
  retranslateUi(this);
}

enum SetResponse dspSalesOrderStatus::set(const ParameterList &pParams)
{ 
  XWidget::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("sohead_id", &valid);
  if (valid)
    _so->setId(param.toInt());

  return NoError;
}

bool dspSalesOrderStatus::setParams(ParameterList &params)
{
  params.append("sohead_id", _so->id());

  return true;
}

void dspSalesOrderStatus::sFillList()
{
  if (_so->id() != -1)
  {
    q.prepare( "SELECT cohead_number,"
               "       cohead_orderdate,"
               "       cohead_custponumber,"
               "       cust_name, cust_phone "
               "FROM cohead, cust "
               "WHERE ( (cohead_cust_id=cust_id)"
               " AND (cohead_id=:sohead_id) );" );
    q.bindValue(":sohead_id", _so->id());
    q.exec();
    if (q.first())
    {
      _orderDate->setDate(q.value("cohead_orderdate").toDate());
      _poNumber->setText(q.value("cohead_custponumber").toString());
      _custName->setText(q.value("cust_name").toString());
      _custPhone->setText(q.value("cust_phone").toString());
    }
    else if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }

    q.prepare( "SELECT MAX(lastupdated) AS lastupdated "
               "  FROM (SELECT cohead_lastupdated AS lastupdated "
               "          FROM cohead "
               "         WHERE (cohead_id=:sohead_id) "
               "         UNION "
               "        SELECT coitem_lastupdated AS lastupdated "
               "          FROM coitem "
               "         WHERE (coitem_cohead_id=:sohead_id) ) AS data; " );
    q.bindValue(":sohead_id", _so->id());
    q.exec();
    if (q.first())
      _lastUpdated->setDate(q.value("lastupdated").toDate());
    else if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }

    display::sFillList();
  }
  else
  {
    _orderDate->clear();
    _poNumber->clear();
    _custName->clear();
    _custPhone->clear();
    list()->clear();
  }
}
