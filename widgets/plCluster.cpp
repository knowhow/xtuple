/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include <QLabel>
//#include <QLineEdit>
//#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>

#include <xsqlquery.h>
#include <parameter.h>

#include "plCluster.h"

PlanOrdLineEdit::PlanOrdLineEdit(QWidget* pParent, const char* pName) :
    VirtualClusterLineEdit(pParent, "planord", "planord_id", "formatPloNumber(planord_id)", "item_number", "(item_descrip1 ' ' item_descrip2)", 0, pName)
{
  setTitles(tr("Planned Order"), tr("Planned Orders"));
  setUiName("plannedOrder");
  setEditPriv("CreatePlannedOrders");
  setViewPriv("ViewPlannedOrders");
  setNewPriv("CreatePlannedOrders");

  _query = "SELECT planord_id AS id,"
           "       formatPloNumber(planord_id) AS number, "
           "       item_number AS name, "
           "       (item_descrip1 || ' ' || item_descrip2) AS description, "
           "       NULL AS active, "
           "       warehous_code, item_number, uom_name,"
           "       item_descrip1, item_descrip2 "
           "FROM planord "
           "  JOIN itemsite ON (planord_itemsite_id=itemsite_id) "
           "  JOIN item ON (itemsite_item_id=item_id) "
           "  JOIN whsinfo ON (itemsite_warehous_id=warehous_id) "
           "  JOIN uom ON (item_inv_uom_id=uom_id) "
           "WHERE (true) ";
}

PlanOrdCluster::PlanOrdCluster(QWidget* pParent, const char* pName) :
    VirtualCluster(pParent, pName)
{
    addNumberWidget(new PlanOrdLineEdit(this, pName));
    qDebug("constructor number:%p",_number);
    setLabel("Planned Order #:");

    QHBoxLayout *_firstLineLayout  = new QHBoxLayout(0);
    QHBoxLayout *_orderLayout      = new QHBoxLayout(0);
    QHBoxLayout *_warehouseLayout  = new QHBoxLayout(0);
    QHBoxLayout *_itemLineLayout   = new QHBoxLayout(0);
    QHBoxLayout *_itemNumberLayout = new QHBoxLayout(0);
    QHBoxLayout *_uomLayout        = new QHBoxLayout(0);

    _firstLineLayout  ->setContentsMargins(0, 0, 0, 0);
    _orderLayout      ->setContentsMargins(0, 0, 0, 0);
    _warehouseLayout  ->setContentsMargins(0, 0, 0, 0);
    _itemLineLayout   ->setContentsMargins(0, 0, 0, 0);
    _itemNumberLayout ->setContentsMargins(0, 0, 0, 0);
    _uomLayout        ->setContentsMargins(0, 0, 0, 0);

    _firstLineLayout->addLayout(_orderLayout);

    QLabel *_warehouseLit = new QLabel(tr("Whs.:"), this);
    _warehouseLit->setObjectName("_warehouseLit");
    _warehouseLit->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    _warehouseLayout->addWidget(_warehouseLit);

    _warehouse = new QLabel(this);
    _warehouse->setObjectName("_warehouse");
    _warehouse->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    _warehouseLayout->addWidget(_warehouse);
    _firstLineLayout->addLayout(_warehouseLayout);
    _grid->addLayout(_firstLineLayout,0,2,1,3,Qt::AlignLeft);

    QLabel *_itemNumberLit = new QLabel(tr("Item Number:"), this);
    _itemNumberLit->setObjectName("_itemNumberLit");
    _itemNumberLit->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    _itemNumberLayout->addWidget(_itemNumberLit);

    _itemNumber = new QLabel(this);
    _itemNumber->setObjectName("_itemNumber");
    _itemNumber->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    _itemNumberLayout->addWidget(_itemNumber);
    _itemLineLayout->addLayout(_itemNumberLayout);

    QLabel *_uomLit = new QLabel(tr("UOM:"), this);
    _uomLit->setObjectName("_uomLit");
    _uomLit->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    _uomLayout->addWidget(_uomLit);

    _uom = new QLabel(this);
    _uom->setObjectName("_uom");
    _uom->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    _uomLayout->addWidget(_uom);
    _itemLineLayout->addLayout(_uomLayout);
    _grid->addLayout(_itemLineLayout,1,0,1,3,Qt::AlignLeft);

    _descrip1 = new QLabel(this);
    _descrip1->setObjectName("_descrip1");
    _descrip1->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    _grid->addWidget(_descrip1,2,0,1,3,Qt::AlignLeft);

    _descrip2 = new QLabel(this);
    _descrip2->setObjectName("_descrip2");
    _descrip2->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    _grid->addWidget(_descrip2,3,0,1,3,Qt::AlignLeft);
}

void PlanOrdCluster::sRefresh()
{
  if (_number->model()->rowCount())
  {
    QAbstractItemModel* model = _number->model();

    _warehouse->setText(model->data(model->index(0,WAREHOUSE)).toString());
    _itemNumber->setText(model->data(model->index(0,ITEM_NUMBER)).toString());
    _uom->setText(model->data(model->index(0,UOM)).toString());
    _descrip1->setText(model->data(model->index(0,ITEM_DESCRIP1)).toString());
    _descrip2->setText(model->data(model->index(0,ITEM_DESCRIP2)).toString());
  }
  else
  {
    _warehouse->clear();
    _itemNumber->clear();
    _uom->clear();
    _descrip1->clear();
    _descrip2->clear();
  }
}
