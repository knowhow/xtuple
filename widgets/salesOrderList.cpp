/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */


#include "salesOrderList.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QLayout>
#include <QPushButton>
#include <QVBoxLayout>

#include <parameter.h>

#include "xtreewidget.h"
#include "warehousegroup.h"
#include "xsqlquery.h"

salesOrderList::salesOrderList( QWidget* parent, const char* name, bool modal, Qt::WFlags fl ) : QDialog(parent, fl)
{
  setObjectName(name ? name : "salesOrderList");
  setModal(modal);

  _soheadid = -1;
  _type = (cSoOpen | cSoClosed | cSoReleased);

  setWindowTitle(tr("Sales Orders"));

  QVBoxLayout *mainLayout = new QVBoxLayout(this);
  QVBoxLayout *warehouseLayout = new QVBoxLayout(0);
  QHBoxLayout *topLayout = new QHBoxLayout(0);
  QVBoxLayout *buttonsLayout = new QVBoxLayout(0);
  QVBoxLayout *listLayout = new QVBoxLayout(0);

  mainLayout->setContentsMargins(5, 5, 5, 5);
  warehouseLayout->setContentsMargins(0, 0, 0, 0);
  topLayout->setContentsMargins(0, 0, 0, 0);
  buttonsLayout->setContentsMargins(0, 0, 0, 0);
  listLayout->setContentsMargins(0, 0, 0, 0);

  mainLayout->setSpacing(5);
  warehouseLayout->setSpacing(0);
  topLayout->setSpacing(7);
  buttonsLayout->setSpacing(5);
  listLayout->setSpacing(0);

  _warehouse = new WarehouseGroup(this, "_warehouse");
  warehouseLayout->addWidget(_warehouse);

  QSpacerItem* spacer = new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Preferred);
  warehouseLayout->addItem(spacer);
  topLayout->addLayout(warehouseLayout);

  QSpacerItem* spacer_2 = new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum);
  topLayout->addItem(spacer_2);

  _close = new QPushButton(tr("&Cancel"), this);
  _close->setObjectName("_close");
  buttonsLayout->addWidget(_close);

  _select = new QPushButton(tr("&Select"), this);
  _select->setObjectName("_select");
  _select->setEnabled( FALSE );
  _select->setDefault( TRUE );
  buttonsLayout->addWidget(_select);
  buttonsLayout->addSpacerItem(new QSpacerItem(20, 20, QSizePolicy::Minimum, QSizePolicy::Expanding));
  topLayout->addLayout(buttonsLayout);
  mainLayout->addLayout(topLayout);

  QLabel *_salesOrdersLit = new QLabel(tr("Sales Orders:"), this);
  _salesOrdersLit->setObjectName("_salesOrdersLit");
  listLayout->addWidget(_salesOrdersLit);

  _so = new XTreeWidget(this);
  _so->setObjectName("_so");
  listLayout->addWidget(_so, 1);
  mainLayout->addLayout(listLayout, 1);

  resize( QSize(490, 390).expandedTo(minimumSizeHint()) );

  connect( _close, SIGNAL( clicked() ), this, SLOT( reject() ) );
  connect( _select, SIGNAL( clicked() ), this, SLOT( sSelect() ) );
  connect( _so, SIGNAL( itemSelected(int) ), _select, SLOT( animateClick() ) );
  connect( _so, SIGNAL( valid(bool) ), _select, SLOT( setEnabled(bool) ) );
  connect( _warehouse, SIGNAL(updated()), this, SLOT( sFillList() ) );

  _so->addColumn(tr("Order #"), _orderColumn, Qt::AlignLeft,  true, "cohead_number");
  _so->addColumn(tr("Customer"),          -1, Qt::AlignLeft,  true, "cust_name");
  _so->addColumn(tr("P/O #"),   _orderColumn, Qt::AlignLeft,  true, "cohead_custponumber");
  _so->addColumn(tr("Ordered"),  _dateColumn, Qt::AlignCenter,true, "cohead_orderdate");
  _so->addColumn(tr("Scheduled"),_dateColumn, Qt::AlignCenter,true, "duedate");

  setTabOrder(_warehouse, _so);
  setTabOrder(_so, _select);
  setTabOrder(_select, _close);
  setTabOrder(_close, _warehouse);
  _warehouse->setFocus();
}

void salesOrderList::set(ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("sohead_id", &valid);
  if (valid)
    _soheadid = param.toInt();
    
  param = pParams.value("soType", &valid);
  if (valid)
    _type = param.toInt();
    
  param = pParams.value("cust_id", &valid);
  if (valid)
    _custid = param.toInt();
 
  sFillList();
}

void salesOrderList::sSelect()
{
  done(_so->id());
}

void salesOrderList::sFillList()
{
  QString sql;


  if (_type == cSoAtShipping)
  {
    sql = "SELECT DISTINCT cohead_id, cohead_number, cust_name, cohead_custponumber,"
          "                cohead_orderdate,"
          "                MIN(coitem_scheddate) AS duedate "
          "FROM cosmisc, coship, cohead, coitem, itemsite, cust "
          "WHERE ((cohead_cust_id=cust_id)"
          " AND (coitem_cohead_id=cohead_id)"
          " AND (coitem_itemsite_id=itemsite_id)"
          " AND (coship_coitem_id=coitem_id)"
          " AND (coship_cosmisc_id=cosmisc_id)"
          " AND (NOT cosmisc_shipped)";

    if (_warehouse->isSelected())
      sql += " AND (itemsite_warehous_id=:warehous_id)";

  sql += ") "
         "GROUP BY cohead_id, cohead_number, cust_name,"
         "         cohead_custponumber, cohead_orderdate "
         "ORDER BY cohead_number;";
  }
  else
  {
    bool statusCheck = FALSE;

    sql = "SELECT DISTINCT cohead_id, cohead_number, cust_name, cohead_custponumber,"
          "                cohead_orderdate,"
          "                MIN(coitem_scheddate) AS duedate "
          "FROM cohead, coitem, itemsite, cust "
          "WHERE ((cohead_cust_id=cust_id)"
          " AND (coitem_status <> 'X')"
          " AND (coitem_cohead_id=cohead_id)"
          " AND (coitem_itemsite_id=itemsite_id)";

    if (_warehouse->isSelected())
      sql += " AND (itemsite_warehous_id=:warehous_id)";

    sql += "AND (";

    if (_type & cSoOpen)
    {
      sql += "(coitem_status='O')";
      statusCheck = TRUE;
    }

    if (_type & cSoClosed)
    {
      if (statusCheck)
        sql += " OR ";
      sql += "(coitem_status='C')";
      statusCheck = TRUE;
    }

    if (_type & cSoReleased)
    {
      if (statusCheck)
        sql += " AND ";
      sql += "(cohead_holdtype='N')";
      statusCheck = TRUE;
    }
    
    if (_type & cSoCustomer)
    {
      if (statusCheck)
        sql += " AND ";
      sql += "(cohead_cust_id=:cust_id)";
    }

    sql += ")) "
           "GROUP BY cohead_id, cohead_number, cust_name,"
           "         cohead_custponumber, cohead_orderdate "
           "ORDER BY cohead_number;";
  }

  XSqlQuery q;
  q.prepare(sql);
  _warehouse->bindValue(q);
  q.bindValue(":cust_id", _custid);
  q.exec();
  _so->populate(q, _soheadid);
}
