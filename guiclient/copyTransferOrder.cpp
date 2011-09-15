/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "copyTransferOrder.h"

#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include "inputManager.h"
#include "storedProcErrorLookup.h"
#include "transferOrder.h"

copyTransferOrder::copyTransferOrder(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_buttonBox,	SIGNAL(accepted()), this, SLOT(sCopy()));
  connect(_to,	       SIGNAL(newId(int)), this, SLOT(populate()));

  _captive = FALSE;
  _to->setAllowedTypes(OrderLineEdit::Transfer);
  _to->setLabel("");

  _item->addColumn(tr("#"),       _seqColumn, Qt::AlignRight, true, "toitem_linenumber");
  _item->addColumn(tr("Item"),   _itemColumn, Qt::AlignLeft,  true, "item_number");
  _item->addColumn(tr("Description"),     -1, Qt::AlignLeft,  true, "description");
  _item->addColumn(tr("Ordered"), _qtyColumn, Qt::AlignRight, true, "toitem_qty_ordered");
}

copyTransferOrder::~copyTransferOrder()
{
  // no need to delete child widgets, Qt does it all for us
}

void copyTransferOrder::languageChange()
{
  retranslateUi(this);
}

enum SetResponse copyTransferOrder::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("tohead_id", &valid);
  if (valid)
  {
    _captive = TRUE;
    _to->setAllowedType("TO");
    _to->setId(param.toInt());
    _to->setEnabled(FALSE);
    populate();
    _buttonBox->setFocus();
  }

  return NoError;
}

void copyTransferOrder::populate()
{
  _item->clear();
  if (_to->id() == -1)
    _orderDate->clear();
  else
  {
    _orderDate->setDate(omfgThis->dbDate(), true);

    q.prepare("SELECT toitem.*,"
	      "       item_number,"
	      "       (item_descrip1 || ' ' || item_descrip2) AS description,"
	      "       'qty' AS toitem_qty_ordered_xtnumericrole "
	      "FROM toitem JOIN item ON (item_id=toitem_item_id) "
	      "WHERE ((toitem_status <> 'X')"
	      "  AND  (toitem_tohead_id=:tohead_id)) "
	      "ORDER BY toitem_linenumber;" );
    q.bindValue(":tohead_id", _to->id());
    q.exec();
    _item->populate(q);
    if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }
}

void copyTransferOrder::sCopy()
{
  q.prepare("SELECT copyTransferOrder(:tohead_id, :scheddate) AS result;");
  q.bindValue(":tohead_id",	_to->id());

  if (_reschedule->isChecked())
    q.bindValue(":scheddate", _scheduleDate->date());
  else
    q.bindValue(":scheddate", QDate::currentDate());

  int toheadid = 0;

  q.exec();
  if (q.first())
  {
    toheadid = q.value("result").toInt();
    if (toheadid < 0)
    {
      QMessageBox::critical(this, tr("Could Not Copy Transfer Order"),
			  storedProcErrorLookup("copyTransferOrder", toheadid));
      return;
    }
    transferOrder::editTransferOrder(toheadid, true);
  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  omfgThis->sTransferOrdersUpdated(toheadid);
  if (_captive)
    done(toheadid);
  else
  {
    _to->setId(-1);
    _to->setFocus();
  }
}
