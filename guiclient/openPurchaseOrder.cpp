/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "openPurchaseOrder.h"

#include <QMenu>
#include <QMessageBox>
#include <QSqlError>
#include <QVariant>
#include <QPushButton>

#include <parameter.h>

//#include "purchaseOrder.h"
openPurchaseOrder::openPurchaseOrder(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_select, SIGNAL(clicked()), this, SLOT(sSelect()));
  connect(_close, SIGNAL(clicked()), this, SLOT(reject()));
  _po->addColumn(tr("PO #"),        -1,  Qt::AlignLeft,   true,  "pohead_number");
  _po->addColumn(tr("PO Date"),        -1,  Qt::AlignLeft,   true,  "pohead_orderdate");
  _po->addColumn(tr("Created By"),        -1,  Qt::AlignLeft,   true,  "pohead_agent_username");
}

openPurchaseOrder::~openPurchaseOrder()
{
  // no need to delete child widgets, Qt does it all for us

}

void openPurchaseOrder::languageChange()
{
 // retranslateUi(this);
}

void openPurchaseOrder::sSelect()
{
  done(_po->id());
}

enum SetResponse openPurchaseOrder::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  QVariant param;
  bool     valid;
  param = pParams.value("vend_name", &valid);
  vendor_id = pParams.value("vend_id", &valid).toInt();
  _vend_namelit -> setText(param.toString());

  sFillList();

  return NoError;
}

void openPurchaseOrder::sFillList()
{
  
  q.prepare( "SELECT pohead_id, pohead_number, pohead_orderdate, pohead_agent_username from pohead where pohead_vend_id = :vend_id and pohead_status = 'U' order by pohead_id desc ;" );
  q.bindValue(":vend_id", vendor_id);
  q.exec();
  _po->populate(q,TRUE);
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}
