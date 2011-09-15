/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "transferOrderList.h"

#include <QPushButton>
#include <QLabel>
#include <QLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>

#include <metasql.h>
#include <parameter.h>

#include "xtreewidget.h"
#include "warehousegroup.h"
#include "salesOrderList.h"

transferOrderList::transferOrderList( QWidget* parent, const char* name, bool modal, Qt::WFlags fl )
  : QDialog(parent, fl)
{
  setObjectName(name ? name : "transferOrderList");
  setModal(modal);

  _toheadid = -1;
  _type = (cToOpen | cToClosed);

  setWindowTitle(tr("Transfer Orders"));

  QVBoxLayout *mainLayout	= new QVBoxLayout(this);
  QHBoxLayout *warehouseLayout	= new QHBoxLayout(0);
  QHBoxLayout *topLayout	= new QHBoxLayout(0);
  QVBoxLayout *buttonsLayout	= new QVBoxLayout(0);
  QVBoxLayout *listLayout	= new QVBoxLayout(0);

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

  _srcwhs = new WarehouseGroup(this, "_srcwhs");
  _srcwhs->setTitle(tr("From Site:"));
  _dstwhs = new WarehouseGroup(this, "_dstwhs");
  _dstwhs->setTitle(tr("To Site:"));
  _dstwhs->setAll();
  warehouseLayout->addWidget(_srcwhs);
  warehouseLayout->addWidget(_dstwhs);

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
  topLayout->addLayout(buttonsLayout);
  mainLayout->addLayout(topLayout);

  QLabel *_transferOrdersLit = new QLabel(tr("Transfer Orders:"), this);
  _transferOrdersLit->setObjectName("_transferOrdersLit");
  listLayout->addWidget(_transferOrdersLit);

  _to = new XTreeWidget(this);
  _to->setObjectName("_to");
  listLayout->addWidget(_to);
  mainLayout->addLayout(listLayout);

  resize( QSize(490, 390).expandedTo(minimumSizeHint()) );

  connect(_close,	SIGNAL(clicked()), this,    SLOT( reject() ) );
  connect(_dstwhs,	SIGNAL(updated()), this,    SLOT( sFillList() ) );
  connect(_select,	SIGNAL(clicked()), this,    SLOT( sSelect() ) );
  connect(_srcwhs,	SIGNAL(updated()), this,    SLOT( sFillList() ) );
  connect(_to,	      SIGNAL(valid(bool)), _select, SLOT( setEnabled(bool) ) );
  connect(_to,	SIGNAL(itemSelected(int)), _select, SLOT( animateClick() ) );

  _to->addColumn(tr("Order #"), _orderColumn, Qt::AlignLeft,  true, "tohead_number");
  _to->addColumn(tr("From Whs."),         -1, Qt::AlignLeft,  true, "tohead_srcname");
  _to->addColumn(tr("To Whs."), _orderColumn, Qt::AlignLeft,  true, "tohead_destname");
  _to->addColumn(tr("Ordered"),  _dateColumn, Qt::AlignCenter, true, "tohead_orderdate");
  _to->addColumn(tr("Scheduled"),_dateColumn, Qt::AlignCenter,true, "scheddate");

  setTabOrder(_srcwhs,	_dstwhs);
  setTabOrder(_dstwhs,	_to);
  setTabOrder(_to,	_select);
  setTabOrder(_select,	_close);
  setTabOrder(_close,	_srcwhs);
  _srcwhs->setFocus();
}

void transferOrderList::set(const ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("tohead_id", &valid);
  if (valid)
    _toheadid = param.toInt();
    
  param = pParams.value("toType", &valid);
  if (valid)
    _type = param.toInt();
 
  sFillList();
}

void transferOrderList::sSelect()
{
  done(_to->id());
}

void transferOrderList::sFillList()
{
  QString sql;

  sql = "SELECT DISTINCT tohead_id, tohead_number, tohead_srcname, "
	"                tohead_destname,"
	"                tohead_orderdate,"
	"                MIN(toitem_schedshipdate) AS scheddate "
	"<? if exists(\"atshipping\") ?>"
	"FROM tohead, toitem, tosmisc, toship "
	"WHERE ((tohead_id=toitem_tohead_id)"
	"  AND  (toitem_status<>'X')"
	"  AND  (toitem_id=toship_toitem_id)"
	"  AND  (toship_tosmisc_id=tosmisc_id)"
	"  AND  (NOT tosmisc_shipped)"
	"<? else ?>"	// not restricted to atshipping
	"FROM tohead, toitem "
	"WHERE ((tohead_id=toitem_tohead_id)"
	"  AND  (toitem_status<>'X')"
	"  <? if exists(\"toitem_statuslist\") ?>"
	"  AND  (toitem_status IN (<? literal(\"toitem_statuslist\") ?>))"
	"  <? endif ?>"
	"<? endif ?>"
	"<? if exists(\"srcwarehous_id\") ?>"
	" AND (tohead_src_warehous_id=<? value(\"srcwarehous_id\") ?>)"
	"<? endif ?>"
	"<? if exists(\"dstwarehous_id\") ?>"
	" AND (tohead_dest_warehous_id=<? value(\"dstwarehous_id\") ?>)"
	"<? endif ?>"
	") "
	"GROUP BY tohead_id, tohead_number, tohead_srcname,"
	"         tohead_destname, tohead_orderdate "
	"ORDER BY tohead_number;";

  ParameterList params;
  if (_type == cToAtShipping)
    params.append("atshipping");
  else
  {
    QString toitem_statuslist;

    bool statusCheck = FALSE;
    if (_type & cToOpen)
    {
      toitem_statuslist += "'O'";
      statusCheck = TRUE;
    }

    if (_type & cToClosed)
    {
      if (statusCheck)
        toitem_statuslist += ", ";
      toitem_statuslist += "'C'";
      statusCheck = TRUE;
    }

    if (statusCheck)
      params.append("toitem_statuslist", toitem_statuslist);
  }

  if (_srcwhs->isSelected())
    params.append("srcwarehous_id", _srcwhs->id());
  if (_dstwhs->isSelected())
    params.append("dstwarehous_id", _dstwhs->id());

  MetaSQLQuery mql(sql);
  XSqlQuery q = mql.toQuery(params);
  _to->populate(q, _toheadid);
}
