/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "splitReceipt.h"

#include <QMessageBox>
#include <QSqlError>
#include <QValidator>
#include <QVariant>

#include <metasql.h>

#include "mqlutil.h"
#include "storedProcErrorLookup.h"

splitReceipt::splitReceipt(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_split, SIGNAL(clicked()), this, SLOT(sSplit()));

  _toSplit->setValidator(omfgThis->qtyVal());
  _toSplit->setFocus();
  _receiptDate->setDate(QDate::currentDate());

  _recvid	= -1;
}

splitReceipt::~splitReceipt()
{
  // no need to delete child widgets, Qt does it all for us
}

void splitReceipt::languageChange()
{
  retranslateUi(this);
}

enum SetResponse splitReceipt::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("recv_id", &valid);
  if (valid)
  {
    _recvid = param.toInt();
    populate();
  }

  return NoError;
}

void splitReceipt::populate()
{
  ParameterList params;

  MetaSQLQuery popm = mqlLoad("itemReceipt", "populateEdit");
  params.append("recv_id", _recvid);
  q = popm.toQuery(params);

  if (q.first())
  {
    _orderNumber->setText(q.value("order_number").toString());
    _lineNumber->setText(q.value("orderitem_linenumber").toString());
    _received->setText(formatQty(q.value("qtyreceived").toDouble()));
    _receiptDate->setDate(q.value("effective").toDate());
    _freight->setId(q.value("curr_id").toInt());
    _freight->setLocalValue(q.value("recv_freight").toDouble());
  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void splitReceipt::sSplit()
{
  int result = 0;

  q.prepare("SELECT splitReceipt(:recvid, :qty, :freight) AS result;");
  q.bindValue(":recvid",	_recvid);
  q.bindValue(":qty",		_toSplit->toDouble());
  q.bindValue(":freight",	_freight->localValue());
  q.exec();
  if (q.first())
  {
    result = q.value("result").toInt();
    if (result < 0)
    {
      systemError(this, storedProcErrorLookup(QString("splitReceipt"), result),
		  __FILE__, __LINE__);
      return;
    }
  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
  }

  omfgThis->sPurchaseOrderReceiptsUpdated();

  accept();
}
