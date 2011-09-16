/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */


#include "purchaseOrderList.h"

#include <QVariant>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QLayout>
#include <QToolTip>

#include <parameter.h>
#include <metasql.h>

#include "xtreewidget.h"

#include "pocluster.h"

purchaseOrderList::purchaseOrderList(QWidget* parent, const char* name, bool modal, Qt::WFlags fl) :
  QDialog(parent, fl)
{
  setObjectName(name ? name : "purchaseOrderList");
  setModal(modal);

  _poheadid = -1;
  _type = (cPOUnposted | cPOOpen | cPOClosed);

  setWindowTitle(tr("Purchase Orders"));

  QHBoxLayout *purchaseOrderListLayout = new QHBoxLayout(this);
  QVBoxLayout *tableLayout  = new QVBoxLayout();
  QVBoxLayout *buttonLayout = new QVBoxLayout();
  purchaseOrderListLayout->addLayout(tableLayout);
  purchaseOrderListLayout->addLayout(buttonLayout);

  QHBoxLayout *vendLayout = new QHBoxLayout();
  _vend = new VendorCluster(this, "_vend");
  vendLayout->addWidget(_vend);
  vendLayout->addItem(new QSpacerItem(20, 20, QSizePolicy::Expanding,
				      QSizePolicy::Minimum));
  tableLayout->addLayout(vendLayout);

  QLabel *_poheadLit = new QLabel(tr("&Purchase Orders:"), this);
  _poheadLit->setObjectName("_poheadLit");
  tableLayout->addWidget( _poheadLit );

  _pohead = new XTreeWidget(this);
  _pohead->setObjectName("_pohead");
  _poheadLit->setBuddy(_pohead);
  tableLayout->addWidget(_pohead);

  _close = new QPushButton(tr("&Cancel"), this);
  _close->setObjectName("_close");
  buttonLayout->addWidget( _close );

  _select = new QPushButton(tr("&Select"), this);
  _select->setObjectName("_select");
  _select->setEnabled( FALSE );
  _select->setDefault( TRUE );
  buttonLayout->addWidget( _select );

  QSpacerItem* spacer = new QSpacerItem( 20, 20, QSizePolicy::Minimum, QSizePolicy::Expanding );
  buttonLayout->addItem( spacer );

  resize( QSize(550, 350).expandedTo(minimumSizeHint()) );
  //clearWState( WState_Polished );

    // signals and slots connections
  connect(_close,  SIGNAL(clicked()),		this,	SLOT(sClose()	     ));
  connect(_pohead, SIGNAL(itemSelected(int)), _select,	SLOT(animateClick()  ));
  connect(_pohead, SIGNAL(valid(bool)),       _select,	SLOT(setEnabled(bool)));
  connect(_select, SIGNAL(clicked()),		this,	SLOT(sSelect()	     ));
  connect(_vend,   SIGNAL(newId(int)),		this,	SLOT(sFillList()     ));

  _type = 0;

  _pohead->addColumn(tr("Number"),   _orderColumn, Qt::AlignRight, true, "pohead_number");
  _pohead->addColumn(tr("Vendor"),             -1, Qt::AlignLeft,  true, "vend_name");
  _pohead->addColumn(tr("Agent"),     _itemColumn, Qt::AlignCenter,true, "pohead_agent_username");
  _pohead->addColumn(tr("Order Date"),_dateColumn, Qt::AlignLeft,  true, "pohead_orderdate");
  _pohead->addColumn(tr("First Item"),_itemColumn, Qt::AlignLeft,  true, "item_number");

  _pohead->setFocus();
}

void purchaseOrderList::set(const ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("pohead_id", &valid);
  if (valid)
    _poheadid = param.toInt();
    
  param = pParams.value("poType", &valid);
  if (valid)
    _type = param.toInt();
 
  sFillList();
}

void purchaseOrderList::sSelect()
{
  done(_pohead->id());
}

void purchaseOrderList::sClose()
{
  done(_poheadid);
}

void purchaseOrderList::sFillList()
{
  QString sql( "SELECT pohead_id, pohead_number,"
               "       vend_name, pohead_agent_username,"
	       "       pohead_orderdate,"
	       "       item_number "
               "FROM vend, pohead LEFT OUTER JOIN"
	       "     poitem ON (poitem_pohead_id=pohead_id AND poitem_linenumber=1)"
	       "     LEFT OUTER JOIN itemsite ON (poitem_itemsite_id=itemsite_id)"
	       "     LEFT OUTER JOIN item ON (itemsite_item_id=item_id) "
               "WHERE ( (pohead_vend_id=vend_id)"
	       "<? if exists(\"typeList\") ?>"
	       "  AND   (pohead_status IN ("
               "<? foreach(\"typeList\") ?>"
               "  <? if not isfirst(\"typeList\") ?>"
               "    ,"
               "  <? endif ?>"
               "  <? value(\"typeList\") ?>"
               "<? endforeach ?>"
               "                          ))"
	       "<? endif ?>"
	       "<? if exists(\"vend_id\") ?>"
	       "  AND   (pohead_vend_id=<? value(\"vend_id\") ?>)"
	       "<? endif ?>"
	       " ) "
	       "ORDER BY pohead_number DESC;" );
  ParameterList params;
  if (_vend->isValid())
    params.append("vend_id", _vend->id());

  QStringList typeList;
  if (_type & cPOUnposted)
    typeList += "U";

  if (_type & cPOOpen)
    typeList += "O";

  if (_type & cPOClosed)
    typeList += "C";

  if (! typeList.isEmpty())
    params.append("typeList", typeList);

  MetaSQLQuery mql(sql);
  XSqlQuery poheadq = mql.toQuery(params);
  _pohead->populate(poheadq, _poheadid);
}
