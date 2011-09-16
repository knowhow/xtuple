/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "voucherItem.h"

// #include <QCloseEvent>
#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include "voucherItemDistrib.h"
#include "enterPoitemReceipt.h"
#include "splitReceipt.h"
#include "taxDetail.h"

voucherItem::voucherItem(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_new, SIGNAL(clicked()), this, SLOT(sNew()));
  connect(_edit, SIGNAL(clicked()), this, SLOT(sEdit()));
  connect(_delete, SIGNAL(clicked()), this, SLOT(sDelete()));
  connect(_save, SIGNAL(clicked()), this, SLOT(sSave()));
  connect(_uninvoiced, SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)), this, SLOT(sToggleReceiving(QTreeWidgetItem*)));
  connect(_uninvoiced, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*)), this, SLOT(sPopulateMenu(QMenu*, QTreeWidgetItem*)));
  connect(_freightToVoucher, SIGNAL(lostFocus()), this, SLOT(sFillList()));
  connect(_vodist, SIGNAL(populated()), this, SLOT(sCalculateTax()));
  connect(_taxtype,	  SIGNAL(newID(int)), this, SLOT(sCalculateTax()));
  connect(_taxLit, SIGNAL(leftClickedURL(const QString&)), this, SLOT(sTaxDetail()));

  _item->setReadOnly(TRUE);
  
  _unitPrice->setPrecision(omfgThis->priceVal());
  _extPrice->setPrecision(omfgThis->moneyVal());
  _lineFreight->setPrecision(omfgThis->moneyVal());

  _ordered->setValidator(omfgThis->qtyVal());
  _received->setValidator(omfgThis->qtyVal());
  _rejected->setValidator(omfgThis->qtyVal());
  _uninvoicedReceived->setValidator(omfgThis->qtyVal());
  _uninvoicedRejected->setValidator(omfgThis->qtyVal());

  _qtyToVoucher->setValidator(omfgThis->qtyVal());
  
  _vodist->addColumn(tr("Cost Element"), -1,           Qt::AlignLeft, true, "costelem_type");
  _vodist->addColumn(tr("Amount"),       _priceColumn, Qt::AlignRight,true, "vodist_amount");

  _uninvoiced->addColumn(tr("Receipt/Reject"), -1,          Qt::AlignCenter, true, "action");
  _uninvoiced->addColumn(tr("Date"),           _dateColumn, Qt::AlignCenter, true, "item_date");
  _uninvoiced->addColumn(tr("Qty."),           _qtyColumn,  Qt::AlignRight,  true, "qty");
  _uninvoiced->addColumn(tr("Tagged"),         _ynColumn,   Qt::AlignCenter, true, "f_tagged");
  
  _rejectedMsg = tr("The application has encountered an error and must "
                    "stop editing this Voucher Item.\n%1");

  _inTransaction = TRUE;
  q.exec("BEGIN;"); //Lot's of things can happen in here that can cause problems if cancelled out.  Let's make it easy to roll it back.
}

voucherItem::~voucherItem()
{
  // no need to delete child widgets, Qt does it all for us
}

void voucherItem::languageChange()
{
  retranslateUi(this);
}

enum SetResponse voucherItem::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("curr_id", &valid);
  if (valid)
  {
    _freightToVoucher->setId(param.toInt());
	_tax->setId(param.toInt());
  }

  param = pParams.value("effective", &valid);
  if (valid)
  {
	_freightToVoucher->setEffective(param.toDate());
	_tax->setEffective(param.toDate());
  }

  param = pParams.value("vohead_id", &valid);
  if (valid)
  {
    _voheadid = param.toInt();
    q.prepare("SELECT vohead_taxzone_id "
	          "FROM vohead "
	          "WHERE (vohead_id = :vohead_id);");
    q.bindValue(":vohead_id", _voheadid);
    q.exec();
    if (q.first())
      _taxzoneid = q.value("vohead_taxzone_id").toInt();
    else if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return UndefinedError;
    }
	else
	  _taxzoneid = -1;
  }
  else
    _voheadid = -1;

  param = pParams.value("poitem_id", &valid);
  if (valid)
  {
    _poitemid = param.toInt();

    q.prepare( "SELECT pohead_number, poitem_linenumber, poitem_taxtype_id, "
               "       COALESCE(itemsite_id, -1) AS itemsiteid,"
               "       poitem_vend_item_number, poitem_vend_uom, poitem_vend_item_descrip,"
               "       poitem_duedate,"
               "       poitem_qty_ordered,"
               "       poitem_qty_received,"
               "       poitem_qty_returned,"
               "       ( SELECT COALESCE(SUM(porecv_qty), 0)"
               "                    FROM porecv"
               "                    WHERE ( (porecv_posted)"
               "                     AND (NOT porecv_invoiced)"
       	       "                     AND (porecv_vohead_id IS NULL)"
               "                     AND (porecv_poitem_id=poitem_id) ) ) AS f_received,"
               "       ( SELECT COALESCE(SUM(poreject_qty), 0)"
               "                    FROM poreject"
               "                    WHERE ( (poreject_posted)"
               "                     AND (NOT poreject_invoiced)"
               "                     ANd (poreject_vohead_id IS NULL)"
               "                     AND (poreject_poitem_id=poitem_id) ) ) AS f_rejected,"
               "       poitem_unitprice,"
               "       poitem_unitprice * poitem_qty_ordered AS f_extprice,"
               "       poitem_freight "
               "FROM pohead, "
	       " poitem LEFT OUTER JOIN itemsite ON (poitem_itemsite_id=itemsite_id) "
           "WHERE ( (poitem_pohead_id=pohead_id)"
               " AND (poitem_id=:poitem_id) );" );
    q.bindValue(":poitem_id", _poitemid);
    q.exec();
    if (q.first())
    {
      _poNumber->setText(q.value("pohead_number").toString());
      _lineNumber->setText(q.value("poitem_linenumber").toString());
      _vendItemNumber->setText(q.value("poitem_vend_item_number").toString());
      _vendUOM->setText(q.value("poitem_vend_uom").toString());
      _vendDescription->setText(q.value("poitem_vend_item_descrip").toString());
      _dueDate->setDate(q.value("poitem_duedate").toDate());
      _ordered->setText(q.value("poitem_qty_ordered").toDouble());
      _received->setText(q.value("poitem_qty_received").toDouble());
      _rejected->setText(q.value("poitem_qty_returned").toDouble());
      _uninvoicedReceived->setText(q.value("f_received").toDouble());
      _uninvoicedRejected->setText(q.value("f_rejected").toDouble());
      _unitPrice->setText(q.value("poitem_unitprice").toDouble());
      _extPrice->setText(q.value("f_extprice").toDouble());
      _lineFreight->setText(q.value("poitem_freight").toDouble());
	  _taxtype->setId(q.value("poitem_taxtype_id").toInt());
      if (q.value("itemsiteid") != -1)
        _item->setItemsiteid(q.value("itemsiteid").toInt());
    }
    else if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, _rejectedMsg.arg(q.lastError().databaseText()),
                  __FILE__, __LINE__);
      reject();
      return UndefinedError;
    }
  }
  else
    _poitemid = -1;

  if ( (_voheadid != -1) && (_poitemid != -1) )
  {
    q.prepare( "SELECT voitem_id, voitem_close, voitem_taxtype_id, "
               "       voitem_qty,"
               "       voitem_freight "
               "FROM voitem "
               "WHERE ( (voitem_vohead_id=:vohead_id)"
               " AND (voitem_poitem_id=:poitem_id) );" );
    q.bindValue(":vohead_id", _voheadid);
    q.bindValue(":poitem_id", _poitemid);
    q.exec();
    if (q.first())
    {
      _voitemid = q.value("voitem_id").toInt();
      _closePoitem->setChecked(q.value("voitem_close").toBool());
      _qtyToVoucher->setText(q.value("voitem_qty").toDouble());
      _freightToVoucher->setLocalValue(q.value("voitem_freight").toDouble());
      _taxtype->setId(q.value("voitem_taxtype_id").toInt());
    }
    else if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, _rejectedMsg.arg(q.lastError().databaseText()),
                  __FILE__, __LINE__);
      reject();
      return UndefinedError;
    }
    else
    {
      _voitemid = -1;
      _closePoitem->setChecked(FALSE);
      _qtyToVoucher->clear();
      _freightToVoucher->clear();
    }

	q.prepare( "SELECT SUM(COALESCE(taxhist_tax, 0.00)) AS taxamt "
	           "FROM voitem LEFT OUTER JOIN voitemtax "
			   " ON (voitem_id = taxhist_parent_id) "
               "WHERE ( (voitem_vohead_id=:vohead_id)"
               " AND (voitem_poitem_id=:poitem_id) );" );
    q.bindValue(":vohead_id", _voheadid);
    q.bindValue(":poitem_id", _poitemid);
    q.exec();
    if (q.first())
	  _tax->setLocalValue(q.value("taxamt").toDouble());
	else if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, _rejectedMsg.arg(q.lastError().databaseText()),
                  __FILE__, __LINE__);
      reject();
      return UndefinedError;
    }
    else
      _tax->clear();
  }

  sFillList();
  _saved = TRUE;
  return NoError;
}

void voucherItem::sSave()
{
  if (_qtyToVoucher->toDouble() <= 0.0)
  {
    QMessageBox::critical( this, tr("Cannot Save Voucher Item"),
                           tr("You must enter a postive Quantity to Voucher before saving this Voucher Item") );
    _qtyToVoucher->setFocus();
    return;
  }

  // Check to make sure there is at least distribution for this Voucher Item
  q.prepare( "SELECT vodist_id "
             "FROM vodist "
             "WHERE ( (vodist_vohead_id=:vohead_id)"
             " AND (vodist_poitem_id=:poitem_id) ) "
             "LIMIT 1;" );
  q.bindValue(":vohead_id", _voheadid);
  q.bindValue(":poitem_id", _poitemid);
  q.exec();
  if (!q.first())
  {
    QMessageBox::critical( this, tr("Cannot Save Voucher Item"),
                           tr("You must make at least one distribution for this Voucher Item before you may save it.") );
    return;
  }
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, _rejectedMsg.arg(q.lastError().databaseText()),
                __FILE__, __LINE__);
    reject();
    return;
  }

  // Check for vendor matching requirement
  q.prepare( "SELECT vend_id "
             " FROM vendinfo,pohead,poitem "
	     " WHERE (	(vend_id=pohead_vend_id) "
	     " AND (pohead_id=poitem_pohead_id) "
	     " AND (poitem_id=:poitem_id) "
	     " AND (vend_match) ); " );
  q.bindValue(":poitem_id", _poitemid);
  q.exec();
  if (q.first())
  {
    q.prepare( "SELECT formatMoney(poitem_unitprice * :voitem_qty) AS f_povalue FROM poitem "
		" WHERE ((poitem_unitprice * :voitem_qty) <> "
		" (SELECT SUM(vodist_amount) "
		"	FROM vodist " 
		"       WHERE ( (vodist_vohead_id=:vohead_id) "
		"       AND (vodist_poitem_id=:poitem_id) ) )"
		" AND (poitem_id=:poitem_id) ); " );
    q.bindValue(":vohead_id", _voheadid);
    q.bindValue(":poitem_id", _poitemid);
    q.bindValue(":voitem_qty", _qtyToVoucher->toDouble());
    q.exec();
  	if (q.first())
    {
    QString msg;
    msg = "The P/O value of ";
    msg.append( q.value("f_povalue").toString() );
    msg.append( " does not match the total distributed value.\nInvoice matching is required for this vendor.\nStop and correct?" );
    if ( QMessageBox::warning( this, tr("Invoice Value Mismatch"), msg, tr("Yes"), tr("No"), QString::null ) != 1 )
          return;
    }
  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, _rejectedMsg.arg(q.lastError().databaseText()),
                __FILE__, __LINE__);
    reject();
    return;
  }

  // Update the qty vouchered
  q.prepare( "UPDATE voitem "
             "SET voitem_close=:voitem_close,"
             "    voitem_freight=:voitem_freight, "
			 "    voitem_taxtype_id=:voitem_taxtype_id "
             "WHERE (voitem_id=:voitem_id);"
             "UPDATE vodist "
             "SET vodist_qty=:qty "
             "WHERE ((vodist_vohead_id=:vohead_id)"
             " AND (vodist_poitem_id=:poitem_id) );" );
  q.bindValue(":qty", _qtyToVoucher->toDouble());
  q.bindValue(":poitem_id", _poitemid);
  q.bindValue(":voitem_id", _voitemid);
  q.bindValue(":vohead_id", _voheadid);
  q.bindValue(":voitem_close", QVariant(_closePoitem->isChecked()));
  q.bindValue(":voitem_freight", _freightToVoucher->localValue());
  if (_taxtype->id() != -1)
    q.bindValue(":voitem_taxtype_id", _taxtype->id());
  q.exec();
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, _rejectedMsg.arg(q.lastError().databaseText()),
                __FILE__, __LINE__);
    reject();
    return;
  }
  q.exec("COMMIT;");
  
  _inTransaction = FALSE;
  accept();
}

void voucherItem::sNew()
{
  q.prepare( "SELECT (poitem_unitprice * :voitem_qty - "
		" (SELECT COALESCE(SUM(vodist_amount),0) "
		"	FROM vodist " 
		"       WHERE ( (vodist_vohead_id=:vohead_id) "
		"       AND (vodist_poitem_id=:poitem_id) ) " 
		" ) ) AS f_amount FROM poitem "
		" WHERE (poitem_id=:poitem_id); " );
  q.bindValue(":vohead_id", _voheadid);
  q.bindValue(":poitem_id", _poitemid);
  q.bindValue(":voitem_qty", _qtyToVoucher->toDouble());
  q.exec();
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, _rejectedMsg.arg(q.lastError().databaseText()),
                __FILE__, __LINE__);
    reject();
    return;
  }

  ParameterList params;
  params.append("vohead_id", _voheadid);
  params.append("poitem_id", _poitemid);
  params.append("mode", "new");
  params.append("curr_id", _freightToVoucher->id());
  params.append("effective", _freightToVoucher->effective());
  if (q.first())
  	params.append("amount", q.value("f_amount").toDouble() );
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, _rejectedMsg.arg(q.lastError().databaseText()),
                __FILE__, __LINE__);
    reject();
    return;
  }

  voucherItemDistrib newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void voucherItem::sEdit()
{
  ParameterList params;
  params.append("vodist_id", _vodist->id());
  params.append("mode", "edit");
  params.append("curr_id", _freightToVoucher->id());
  params.append("effective", _freightToVoucher->effective());

  voucherItemDistrib newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void voucherItem::sDelete()
{
  q.prepare( "DELETE FROM vodist "
             "WHERE (vodist_id=:vodist_id);" );
  q.bindValue(":vodist_id", _vodist->id());
  q.exec();
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, _rejectedMsg.arg(q.lastError().databaseText()),
                __FILE__, __LINE__);
    reject();
    return;
  }

  sFillList();
}

void voucherItem::sToggleReceiving(QTreeWidgetItem *pItem)
{
  double n;
  QString s;
  XTreeWidgetItem* item = (XTreeWidgetItem*)pItem;
  if(item->id() == -1)
    return;
  if (item->text(3) == "Yes")
  {
    item->setText(3, "No");
    if (item->text(0) == "Receiving")
    {
    	n = _qtyToVoucher->toDouble();
    	_qtyToVoucher->setText(item->text(2));
    	n = n - _qtyToVoucher->toDouble();
    	_qtyToVoucher->setText(s.setNum(n));

      n = _uninvoicedReceived->toDouble();
      _uninvoicedReceived->setText(item->text(2));
      n = n + _uninvoicedReceived->toDouble();
      _uninvoicedReceived->setText(s.setNum(n));
    }
    else
    {
    	n = _qtyToVoucher->toDouble();
    	_qtyToVoucher->setText(item->text(2));
    	n = n - _qtyToVoucher->toDouble();
    	_qtyToVoucher->setText(s.setNum(n));

      n = _uninvoicedRejected->toDouble();
      _uninvoicedRejected->setText(item->text(2));
      n = n + _rejected->toDouble();
      _uninvoicedRejected->setText(s.setNum(n));
    }
  }
  else 
  {
    item->setText(3, "Yes");
    if (item->text(0) == "Receiving")
    {
    	n = _qtyToVoucher->toDouble();
    	_qtyToVoucher->setText(item->text(2));
    	n = n + _qtyToVoucher->toDouble();
    	_qtyToVoucher->setText(s.setNum(n));

      n = _uninvoicedReceived->toDouble();
      _uninvoicedReceived->setText(item->text(2));
      n = n - _uninvoicedReceived->toDouble();
      _uninvoicedReceived->setText(s.setNum(n));
    }
    else
    {
      n = _qtyToVoucher->toDouble();
      _qtyToVoucher->setText(item->text(2));
      n = n + _qtyToVoucher->toDouble();
      _qtyToVoucher->setText(s.setNum(n));

      n = _uninvoicedRejected->toDouble();
      _uninvoicedRejected->setText(item->text(2));
      n = n + _uninvoicedRejected->toDouble();
      _uninvoicedRejected->setText(s.setNum(n));
    }
  }

  // Check PO Close flag

  if ( ((_ordered->toDouble() <= (_received->toDouble() - _rejected->toDouble()))) && (_uninvoicedReceived->toDouble() == 0) && (_uninvoicedRejected->toDouble() == 0) )
        _closePoitem->setChecked(true);
  else
	_closePoitem->setChecked(false);
  
  // Save the voitem information
  if (_voitemid != -1)
  {
    q.prepare( "UPDATE voitem "
               "SET voitem_qty=:voitem_qty "
               "WHERE (voitem_id=:voitem_id);" );
    q.bindValue(":voitem_id", _voitemid);
  }
  else
  {
  // Get next voitem id
    q.prepare("SELECT NEXTVAL('voitem_voitem_id_seq') AS voitemid");
    q.exec();
    if (q.first())
      _voitemid = (q.value("voitemid").toInt());
    else if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, _rejectedMsg.arg(q.lastError().databaseText()),
                  __FILE__, __LINE__);
      reject();
      return;
    }
    
    q.prepare( "INSERT INTO voitem "
               "(voitem_id, voitem_vohead_id, voitem_poitem_id, voitem_close, voitem_qty, voitem_freight) "
               "VALUES "
               "(:voitem_id, :vohead_id, :poitem_id, :voitem_close, :voitem_qty, :voitem_freight);" );
  }

  q.bindValue(":voitem_id", _voitemid);
  q.bindValue(":vohead_id", _voheadid);
  q.bindValue(":poitem_id", _poitemid);
  q.bindValue(":voitem_close", QVariant(_closePoitem->isChecked()));
  q.bindValue(":voitem_qty", _qtyToVoucher->toDouble());
  q.bindValue(":voitem_freight", _freightToVoucher->localValue());
  q.exec();
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, _rejectedMsg.arg(q.lastError().databaseText()),
                __FILE__, __LINE__);
    reject();
    return;
  }
  
  // Update the receipt record
  if (item->text(3) == "Yes")
  {
    if (item->altId() == 1)
      q.prepare( "UPDATE recv "
                 "SET recv_vohead_id=:vohead_id,recv_voitem_id=:voitem_id "
                 "WHERE (recv_id=:target_id);" );
    else if (item->altId() == 2)
      q.prepare( "UPDATE poreject "
                 "SET poreject_vohead_id=:vohead_id,poreject_voitem_id=:voitem_id "
                 "WHERE (poreject_id=:target_id);" );
  }
  else
  {
    if (item->altId() == 1)
      q.prepare( "UPDATE recv "
                 "SET recv_vohead_id=NULL,recv_voitem_id=NULL "
                 "WHERE ((recv_id=:target_id)"
                 "  AND  (recv_vohead_id=:vohead_id));" );
    else if (item->altId() == 2)
      q.prepare( "UPDATE poreject "
                 "SET poreject_vohead_id=NULL,poreject_voitem_id=NULL "
                 "WHERE ((poreject_id=:target_id)"
                 "  AND  (poreject_vohead_id=:vohead_id));" );
  }

  q.bindValue(":vohead_id", _voheadid);
  q.bindValue(":voitem_id", _voitemid);
  q.bindValue(":target_id", item->id());
  q.exec();
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, _rejectedMsg.arg(q.lastError().databaseText()),
                __FILE__, __LINE__);
    reject();
    return;
  }

}

void voucherItem::sFillList()
{
  q.prepare( "SELECT vodist_id,"
             "       COALESCE(costelem_type, :none) AS costelem_type,"
             "       vodist_amount, 'currval' AS vodist_amount_xtnumericrole "
             "FROM vodist "
             "     LEFT OUTER JOIN costelem ON (vodist_costelem_id=costelem_id) "
             "WHERE ( (vodist_poitem_id=:poitem_id)"
             " AND (vodist_vohead_id=:vohead_id) );" );
  q.bindValue(":none", tr("None"));
  q.bindValue(":poitem_id", _poitemid);
  q.bindValue(":vohead_id", _voheadid);
  q.exec();
  _vodist->populate(q);
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, _rejectedMsg.arg(q.lastError().databaseText()),
                __FILE__, __LINE__);
    reject();
    return;
  }

  // Display the total distributed amount
  q.prepare( "SELECT SUM(vodist_amount) AS totalamount "
             "FROM vodist "
             "WHERE ( (vodist_vohead_id=:vohead_id)"
             " AND (vodist_poitem_id=:poitem_id) );" );
  q.bindValue(":vohead_id", _voheadid);
  q.bindValue(":poitem_id", _poitemid);
  q.exec();
  if (q.first())
    _totalDistributed->setLocalValue(q.value("totalamount").toDouble() + _tax->localValue() + 
					_freightToVoucher->localValue());
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, _rejectedMsg.arg(q.lastError().databaseText()),
                __FILE__, __LINE__);
    reject();
    return;
  }
          
  // Fill univoiced receipts list
  q.prepare( "SELECT porecv_id AS item_id, 1 AS item_type, :receiving AS action,"
             "       porecv_date AS item_date,"
             "       porecv_qty AS qty, 'qty' AS qty_xtnumericrole,"
             "       formatBoolYN(porecv_vohead_id=:vohead_id) AS f_tagged,"
             "       0 AS qty_xttotalrole "
             "FROM porecv "
             "WHERE ( (NOT porecv_invoiced)"
             " AND ((porecv_vohead_id IS NULL) OR (porecv_vohead_id=:vohead_id))"
             " AND (porecv_poitem_id=:poitem_id) ) "

             "UNION "
             "SELECT poreject_id AS item_id, 2 AS item_type, :reject AS action,"
             "       poreject_date AS item_date,"
             "       poreject_qty * -1 AS qty, 'qty', "
             "       formatBoolYN(poreject_vohead_id=:vohead_id) AS f_tagged,"
             "       0 AS qty_xttotalrole "
             "FROM poreject "
             "WHERE ( (poreject_posted)"
             " AND (NOT poreject_invoiced)"
             " AND ((poreject_vohead_id IS NULL) OR (poreject_vohead_id=:vohead_id))"
             " AND (poreject_poitem_id=:poitem_id) );" );
  q.bindValue(":receiving", tr("Receiving"));
  q.bindValue(":reject", tr("Reject"));
  q.bindValue(":vohead_id", _voheadid);
  q.bindValue(":poitem_id", _poitemid);
  q.exec();
  _uninvoiced->populate(q, true);
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, _rejectedMsg.arg(q.lastError().databaseText()),
                __FILE__, __LINE__);
    reject();
    return;
  }
 }

void voucherItem::sCorrectReceiving()
{
  if (enterPoitemReceipt::correctReceipt(_uninvoiced->id(), this) != XDialog::Rejected)
    sFillList();
}

void voucherItem::sSplitReceipt()
{
  ParameterList params;
  params.append("recv_id", _uninvoiced->id());

  splitReceipt newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void voucherItem::sPopulateMenu(QMenu *pMenu,  QTreeWidgetItem *selected)
{
  QAction *menuItem;
  
  if (selected->text(3) == "No")
  {
    menuItem = pMenu->addAction(tr("Correct Receipt..."), this, SLOT(sCorrectReceiving()));
    menuItem->setEnabled(_privileges->check("EnterReceipts"));

    menuItem = pMenu->addAction(tr("Split Receipt..."), this, SLOT(sSplitReceipt()));
    menuItem->setEnabled(_privileges->check("EnterReceipts"));
  }
}

void voucherItem::reject()
{
  if(_inTransaction)
  {
    q.exec("ROLLBACK;");
    _inTransaction = false;
  }
  XDialog::reject();
}

void voucherItem::closeEvent(QCloseEvent * event)
{
  if(_inTransaction)
  {
    q.exec("ROLLBACK;");
    _inTransaction = false;
  }
  XDialog::closeEvent(event);
}

void voucherItem::sCalculateTax()
{
  _saved = FALSE;
  XSqlQuery calcq;
  calcq.prepare( "SELECT SUM(COALESCE(tax, 0.00)) AS totaltax "
                 "FROM (SELECT calculateTax(vohead_taxzone_id, :taxtype_id, "
				 " vohead_docdate, vohead_curr_id, vodist_amount) AS tax "
                 " FROM vohead JOIN vodist ON(vohead_id=vodist_vohead_id) "
                 " WHERE (vohead_id=:vohead_id) "
                 " AND (vodist_poitem_id=:poitem_id) "
				 ") data;"); 
  calcq.bindValue(":vohead_id", _voheadid);
  calcq.bindValue(":taxtype_id", _taxtype->id());
  calcq.bindValue(":poitem_id", _poitemid);
  calcq.exec();
  if (calcq.first())
    _tax->setLocalValue(calcq.value("totaltax").toDouble());
  else if (calcq.lastError().type() != QSqlError::NoError)
  {
    systemError(this, calcq.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void voucherItem::sTaxDetail()
{
  q.prepare( "SELECT SUM(vodist_amount) AS distamount "
             "FROM vodist "
             "WHERE ( (vodist_vohead_id=:vohead_id)"
             " AND (vodist_poitem_id=:poitem_id) );" );
  q.bindValue(":vohead_id", _voheadid);
  q.bindValue(":poitem_id", _poitemid);
  q.exec();
  if (q.first())
    _distamount = q.value("distamount").toDouble();
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, _rejectedMsg.arg(q.lastError().databaseText()),
                __FILE__, __LINE__);
    return;
  }

  taxDetail newdlg(this, "", true);
  ParameterList params;
  params.append("taxzone_id", _taxzoneid);
  params.append("taxtype_id", _taxtype->id());
  params.append("date", _tax->effective());
  params.append("subtotal", _distamount);
  params.append("curr_id", _tax->id());
  params.append("sense", -1);
  
  if(cView == _mode)
    params.append("readOnly");
  
  if(_saved == TRUE)
  {
	params.append("order_id", _voitemid);
    params.append("order_type", "VI");
  }

  newdlg.set(params);
  
  if (newdlg.set(params) == NoError && newdlg.exec())
  {
    if (_taxtype->id() != newdlg.taxtype())
      _taxtype->setId(newdlg.taxtype());
  }
}
