/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "copyPurchaseOrder.h"

#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include "inputManager.h"
#include "purchaseOrderList.h"
#include "storedProcErrorLookup.h"

copyPurchaseOrder::copyPurchaseOrder(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_buttonBox,     SIGNAL(accepted()), this,          SLOT(sCopy()));
  connect(_po,   SIGNAL(newId(int, QString)), this,          SLOT(sPopulatePoInfo(int)));
  connect(_reschedule, SIGNAL(toggled(bool)), _scheduleDate, SLOT(setEnabled(bool)));

  _captive = false;

  //omfgThis->inputManager()->notify(cBCPurchaseOrder, this, _po, SLOT(setId(int)));

  _poitem->addColumn(tr("#"),       _seqColumn, Qt::AlignRight,true, "poitem_linenumber" );
  _poitem->addColumn(tr("Item"),   _itemColumn, Qt::AlignLeft, true, "item_number");
  _poitem->addColumn(tr("Description"),     -1, Qt::AlignLeft, true, "item_descrip");
  _poitem->addColumn(tr("Site"),    _whsColumn, Qt::AlignCenter,true, "warehous_code");
  _poitem->addColumn(tr("Ordered"), _qtyColumn, Qt::AlignRight, true, "poitem_qty_ordered");
  _poitem->addColumn(tr("Price"),   _qtyColumn, Qt::AlignRight, true, "poitem_unitprice");
  _poitem->addColumn(tr("Extended"),_qtyColumn, Qt::AlignRight, true, "extended");

  _currency->setType(XComboBox::Currencies);
  _currency->setLabel(_currencyLit);
}

copyPurchaseOrder::~copyPurchaseOrder()
{
  // no need to delete child widgets, Qt does it all for us
}

void copyPurchaseOrder::languageChange()
{
  retranslateUi(this);
}

enum SetResponse copyPurchaseOrder::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("pohead_id", &valid);
  if (valid)
  {
    _captive = true;
    _po->setId(param.toInt());
    _po->setEnabled(false);

    _buttonBox->setFocus();
  }

  return NoError;
}

void copyPurchaseOrder::sPopulatePoInfo(int)
{
  if (_po->id() != -1)
  {
    q.prepare( "SELECT pohead_orderdate,"
              "        vend_id, vend_phone1, pohead_curr_id "
              "FROM pohead, vend "
              "WHERE ( (pohead_vend_id=vend_id)"
              " AND (pohead_id=:pohead_id) );" );
    q.bindValue(":pohead_id", _po->id());
    q.exec();
    if (q.first())
    {
      _orderDate->setDate(q.value("pohead_orderdate").toDate());
      _vend->setId(q.value("vend_id").toInt());
      _vendPhone->setText(q.value("vend_phone1").toString());
      _currency->setId(q.value("pohead_curr_id").toInt());
    }
    else if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }

    q.prepare( "SELECT poitem.*,"
               "       COALESCE(item_number,poitem_vend_item_number) AS item_number,"
               "       COALESCE((item_descrip1 || ' ' || item_descrip2),poitem_vend_item_descrip) AS item_descrip,"
               "       warehous_code,"
               "       poitem_qty_ordered * poitem_unitprice AS extended,"
               "       'qty' AS poitem_qty_ordered_xtnumericrole,"
               "       'price' AS poitem_unitprice_xtnumericrole,"
               "       'curr' AS extended_xtnumericrole "
               "FROM poitem LEFT OUTER JOIN "
               "      (itemsite JOIN item ON (itemsite_item_id=item_id) "
               "                JOIN warehous ON (itemsite_warehous_id=warehous_id) ) "
               "      ON (poitem_itemsite_id=itemsite_id) "
               "WHERE (poitem_pohead_id=:pohead_id) "
               "ORDER BY poitem_linenumber;" );
    q.bindValue(":pohead_id", _po->id());
    q.exec();
    _poitem->populate(q);
    if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }
  else
  {
    _orderDate->clear();
    _vend->setId(-1);
    _vendPhone->clear();
    _poitem->clear();
  }
}

void copyPurchaseOrder::sCopy()
{
  q.prepare("SELECT copyPo(:pohead_id, :vend_id, :scheddate, :recheck) AS pohead_id;");
  q.bindValue(":pohead_id",     _po->id());
  q.bindValue(":vend_id",       _vend->id());

  if (_reschedule->isChecked())
    q.bindValue(":scheddate", _scheduleDate->date());
  else
    q.bindValue(":scheddate", QDate::currentDate());

  q.bindValue(":recheck", QVariant(_recheck->isChecked()));

  int poheadid = 0;

  q.exec();
  if (q.first())
  {
    poheadid = q.value("pohead_id").toInt();
    if (poheadid < 0)
    {
      QMessageBox::critical(this, tr("Could Not Copy Purchase Order"),
                            storedProcErrorLookup("copyPo", poheadid));
      return;
    }
  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  if (_captive)
  {
    omfgThis->sPurchaseOrdersUpdated(poheadid, true);
    done(poheadid);
  }
  else
  {
    _po->setId(-1);
    _po->setFocus();
  }
}
