/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "voucher.h"

#include <QAction>
#include <QCloseEvent>
#include <QKeyEvent>
#include <QMenu>
#include <QMessageBox>
#include <QSqlError>
#include <QValidator>
#include <QVariant>

#include "purchaseOrderList.h"
#include "purchaseOrderItem.h"
#include "storedProcErrorLookup.h"
#include "voucherItem.h"
#include "voucherMiscDistrib.h"

voucher::voucher(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  connect(_save,          SIGNAL(clicked()), this, SLOT(sSave()));
  connect(_distributions, SIGNAL(clicked()), this, SLOT(sDistributions()));
  connect(_distributeline, SIGNAL(clicked()), this, SLOT(sDistributeLine()));
  connect(_clear, SIGNAL(clicked()), this, SLOT(sClear()));
  connect(_distributeall, SIGNAL(clicked()), this, SLOT(sDistributeAll()));
  connect(_voucherNumber, SIGNAL(lostFocus()), this, SLOT(sHandleVoucherNumber()));
  connect(_poNumber, SIGNAL(newId(int, const QString&)), this, SLOT(sFillList()));
  connect(_poNumber, SIGNAL(newId(int, const QString&)), this, SLOT(sPopulatePoInfo()));
  connect(_amountToDistribute, SIGNAL(valueChanged()), this, SLOT(sPopulateBalanceDue()));
  connect(_new, SIGNAL(clicked()), this, SLOT(sNewMiscDistribution()));
  connect(_edit, SIGNAL(clicked()), this, SLOT(sEditMiscDistribution()));
  connect(_delete, SIGNAL(clicked()), this, SLOT(sDeleteMiscDistribution()));
  connect(_invoiceDate, SIGNAL(newDate(const QDate&)), this, SLOT(sPopulateDistDate()));
  connect(_terms, SIGNAL(newID(int)), this, SLOT(sPopulateDueDate()));
  connect(_poitem, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*)));
  connect(_amountToDistribute, SIGNAL(idChanged(int)), this, SLOT(sFillList()));
  connect(_amountDistributed, SIGNAL(valueChanged()), this, SLOT(sPopulateBalanceDue()));

  _terms->setType(XComboBox::APTerms);
  _poNumber->setAllowedStatuses(OrderLineEdit::Open);
  _poNumber->setAllowedTypes(OrderLineEdit::Purchase);
  _poNumber->setLabel("");
  _poNumber->findChild<QWidget*>("_name")->hide();
  _poNumber->findChild<QWidget*>("_description")->hide();
  _poNumber->findChild<QWidget*>("_fromLit")->hide();
  _poNumber->findChild<QWidget*>("_from")->hide();
  _poNumber->findChild<QWidget*>("_toLit")->hide();
  _poNumber->findChild<QWidget*>("_to")->hide();
  _poNumber->setMinimumHeight(32);
  
  _poitem->addColumn(tr("#"),               _whsColumn,   Qt::AlignCenter, true,  "poitem_linenumber" );
  _poitem->addColumn(tr("Status"),          _uomColumn,   Qt::AlignCenter, true,  "poitemstatus" );
  _poitem->addColumn(tr("Item Number"),     _itemColumn,  Qt::AlignLeft,   true,  "itemnumber"   );
  _poitem->addColumn(tr("UOM"),             _uomColumn,   Qt::AlignCenter, true,  "uom" );
  _poitem->addColumn(tr("Vend. Item #"),    -1,           Qt::AlignLeft,   true,  "poitem_vend_item_number"   );
  _poitem->addColumn(tr("UOM"),             _uomColumn,   Qt::AlignCenter, true,  "poitem_vend_uom" );
  _poitem->addColumn(tr("Ordered"),         _qtyColumn,   Qt::AlignRight,  true,  "poitem_qty_ordered"  );
  _poitem->addColumn(tr("Invoiced"),        _qtyColumn,   Qt::AlignRight,  false, "qtyinvoiced" );
  _poitem->addColumn(tr("Uninvoiced"),      _qtyColumn,   Qt::AlignRight,  true,  "qtyreceived"  );
  _poitem->addColumn(tr("Rejected"),        _qtyColumn,   Qt::AlignRight,  true,  "qtyrejected"  );
  _poitem->addColumn(tr("Quantity"),        _qtyColumn,   Qt::AlignRight,  false, "invoiceqty" );
  _poitem->addColumn(tr("Amount"),          _moneyColumn, Qt::AlignRight,  true,  "invoiceamount"  );
  _poitem->addColumn(tr("PO Unit Price"),   _moneyColumn, Qt::AlignRight,  true,  "poitem_unitprice" );
  _poitem->addColumn(tr("PO Ext Price"),    _moneyColumn, Qt::AlignRight,  true,  "extprice"  );
  _poitem->addColumn(tr("PO Line Freight"), _moneyColumn, Qt::AlignRight,  true,  "poitem_freight" );

  _miscDistrib->addColumn(tr("Account"),    -1,           Qt::AlignLeft,   true,  "account"  );
  _miscDistrib->addColumn(tr("Amount"),     _moneyColumn, Qt::AlignRight,  true,  "vodist_amount" ); 

  _vendid = -1;

  setWindowModified(false);
}

voucher::~voucher()
{
  // no need to delete child widgets, Qt does it all for us
}

void voucher::languageChange()
{
  retranslateUi(this);
}

enum SetResponse voucher::set(const ParameterList &pParams)
{
  XWidget::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      _mode = cNew;

      q.exec("SELECT NEXTVAL('vohead_vohead_id_seq') AS vohead_id");
      if (q.first())
        _voheadid = q.value("vohead_id").toInt();
      else
      {
        systemError(this, tr("A System Error occurred at %1::%2.")
                          .arg(__FILE__)
                          .arg(__LINE__) );
        return UndefinedError;
      }

      if (_metrics->value("VoucherNumberGeneration") == "A")
      {
        populateNumber();
        _poNumber->setFocus();
      }
      else
        _voucherNumber->setFocus();

      q.prepare("INSERT INTO vohead (vohead_id,   vohead_number, vohead_posted)"
		"            VALUES (:vohead_id, :vohead_number, false);" );
      q.bindValue(":vohead_id",     _voheadid);
      q.bindValue(":vohead_number", _voucherNumber->text());
      q.exec();
      if (q.lastError().type() != QSqlError::NoError)
      {
	systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
	return UndefinedError;
      }
      enableWindowModifiedSetting();
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;

      _voucherNumber->setEnabled(FALSE);
      _poNumber->setEnabled(FALSE);

      _save->setFocus();
    }
    else if (param.toString() == "view")
    {
      _mode = cView;

      _poNumber->setAllowedStatuses(OrderLineEdit::AnyStatus);
 
      _voucherNumber->setEnabled(FALSE);
      _poNumber->setEnabled(FALSE);
      _taxzone->setEnabled(FALSE);
      _amountToDistribute->setEnabled(FALSE);
      _distributionDate->setEnabled(FALSE);
      _invoiceDate->setEnabled(FALSE);
      _dueDate->setEnabled(FALSE);
      _invoiceNum->setEnabled(FALSE);
      _reference->setEnabled(FALSE);
      _distributions->setEnabled(FALSE);
      _miscDistrib->setEnabled(FALSE);
      _new->setEnabled(FALSE);
      _terms->setEnabled(FALSE);
      _terms->setType(XComboBox::Terms);
      _flagFor1099->setEnabled(FALSE);
      _distributeall->setEnabled(FALSE);
      _notes->setEnabled(false);
      _close->setText(tr("&Close"));
      _save->hide();

      _close->setFocus();
      disconnect(_poitem, SIGNAL(valid(bool)), _distributions, SLOT(setEnabled(bool)));
      disconnect(_poitem, SIGNAL(valid(bool)), _distributeline, SLOT(setEnabled(bool)));
      disconnect(_poitem, SIGNAL(valid(bool)), _clear, SLOT(setEnabled(bool)));
      disconnect(_poNumber, SIGNAL(valid(bool)), _distributeall, SLOT(setEnabled(bool)));
    }
  }

  param = pParams.value("pohead_id", &valid);
  if (valid)
    _poNumber->setId(param.toInt(), "PO");

  param = pParams.value("vohead_id", &valid);
  if (valid)
  {
    _voheadid = param.toInt();
    populate();
    enableWindowModifiedSetting();
  }

  return NoError;
}

bool voucher::sSave()
{
  if (! _poNumber->isValid())
  {
    QMessageBox::critical( this, tr("Cannot Save Voucher"),
                           tr("<p>You must enter an PO Number before you may "
                              "save this Voucher.") );
    _poNumber->setFocus();
    return false;
  }

  if (!_invoiceDate->isValid())
  {
    QMessageBox::critical( this, tr("Cannot Save Voucher"),
                           tr("<p>You must enter an Invoice Date before you "
                              "may save this Voucher.") );
    _invoiceDate->setFocus();
    return false;
  }

  if (!_dueDate->isValid())
  {
    QMessageBox::critical( this, tr("Cannot Save Voucher"),
                           tr("<p>You must enter a Due Date before you may "
                              "save this Voucher.") );
    _dueDate->setFocus();
    return false;
  }

  if (!_distributionDate->isValid())
  {
    QMessageBox::critical( this, tr("Cannot Save Voucher"),
                           tr("<p>You must enter a Distribution Date before "
                              "you may save this Voucher.") );
    _distributionDate->setFocus();
    return false;
  }

  if ( (_metrics->boolean("ReqInvRegVoucher")) && (_invoiceNum->text().trimmed().length() == 0) )
  {
    QMessageBox::critical( this, tr("Cannot Save Voucher"),
                           tr("<p>You must enter a Vendor Invoice Number "
                              "before you may save this Voucher.") );
    _invoiceNum->setFocus();
    return false;
  }
  else if (_invoiceNum->text().trimmed().length() > 0)
  {
    q.prepare( "SELECT vohead_id "
               "FROM vohead, pohead "
               "WHERE ( (vohead_pohead_id=pohead_id)"
               " AND (vohead_invcnumber=:vohead_invcnumber)"
               " AND (pohead_vend_id=:vend_id)"
               " AND (vohead_id<>:vohead_id) );" );
    q.bindValue(":vohead_invcnumber", _invoiceNum->text().trimmed());
    q.bindValue(":vend_id",   _vendid);
    q.bindValue(":vohead_id", _voheadid);
    q.exec();
    if (q.first())
    {
      if (QMessageBox::question( this, windowTitle(),
                             tr("<p>A Voucher for this Vendor has already been "
                                "entered with the same Vendor Invoice Number. "
                                "Are you sure you want to use this number again?" ),
                              QMessageBox::No | QMessageBox::Yes,
                              QMessageBox::No) == QMessageBox::No)
      {
        _invoiceNum->setFocus();
        return false;
      }
    }
  }

  q.prepare( "UPDATE vohead "
	     "SET vohead_pohead_id=:vohead_pohead_id,"
             "    vohead_taxzone_id=:vohead_taxzone_id,  "
	     "    vohead_vend_id=:vohead_vend_id,"
	     "    vohead_terms_id=:vohead_terms_id,"
	     "    vohead_distdate=:vohead_distdate,"
	     "    vohead_docdate=:vohead_docdate,"
	     "    vohead_duedate=:vohead_duedate,"
	     "    vohead_invcnumber=:vohead_invcnumber,"
	     "    vohead_reference=:vohead_reference,"
	     "    vohead_amount=:vohead_amount,"
	     "    vohead_1099=:vohead_1099, "
	     "    vohead_curr_id=:vohead_curr_id, "
	     "    vohead_notes=:vohead_notes "
	     "WHERE (vohead_id=:vohead_id);" );

  q.bindValue(":vohead_id", _voheadid);
  q.bindValue(":vohead_pohead_id", _poNumber->id());
  if (_taxzone->isValid())
    q.bindValue(":vohead_taxzone_id", _taxzone->id());
  q.bindValue(":vohead_vend_id",  _vendid);
  q.bindValue(":vohead_terms_id", _terms->id());
  q.bindValue(":vohead_distdate", _distributionDate->date());
  q.bindValue(":vohead_docdate", _invoiceDate->date());
  q.bindValue(":vohead_duedate", _dueDate->date());
  q.bindValue(":vohead_invcnumber", _invoiceNum->text().trimmed());
  q.bindValue(":vohead_reference", _reference->text().trimmed());
  q.bindValue(":vohead_amount", _amountToDistribute->localValue());
  q.bindValue(":vohead_1099", QVariant(_flagFor1099->isChecked()));
  q.bindValue(":vohead_curr_id", _amountToDistribute->id());
  q.bindValue(":vohead_notes", _notes->toPlainText());
  q.exec();
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return false;
  }

  omfgThis->sVouchersUpdated();

  _voheadid = -1;

  if (cNew != _mode)
  {
    setWindowModified(false);
    close();
    return true;
  }

  _poNumber->setEnabled(true);
  _poNumber->setId(-1);
  _amountToDistribute->clear();
  _amountDistributed->clear();
  _balance->clear();
  _flagFor1099->setChecked(false);
  _invoiceDate->setNull();
  _distributionDate->setNull();
  _dueDate->setNull();
  _invoiceNum->clear();
  _reference->clear();
  _poitem->clear();
  _miscDistrib->clear();
  _notes->setText("");

  setWindowModified(false);

  ParameterList params;
  params.append("mode", "new");
  set(params);
  return true;
}

void voucher::sHandleVoucherNumber()
{
  if (_voucherNumber->text().length() == 0)
  {
    if ((_metrics->value("VoucherNumberGeneration") == "A") ||
        (_metrics->value("VoucherNumberGeneration") == "O"))
      populateNumber();
    else
    {
      QMessageBox::critical( this, tr("Enter Voucher Number"),
                             tr("<p>You must enter a valid Voucher Number "
                                "before continuing") );

      _voucherNumber->setFocus();
      return;
    }
  }
  else
  {
    q.prepare( "SELECT vohead_id "
               "FROM vohead "
               "WHERE (vohead_number=:vohead_number);" );
    q.bindValue(":vohead_number", _voucherNumber->text());
    q.exec();
    if (q.first())
    {
      _voheadid = q.value("vohead_id").toInt();

      _voucherNumber->setEnabled(FALSE);
      _poNumber->setEnabled(FALSE);

      _mode = cEdit;
      populate();

      return;
    }
  }
}

void voucher::sPopulate()
{
  setWindowTitle(tr("Voucher for P/O # %1").arg(_poNumber->number()));
}

void voucher::sDistributions()
{
  saveDetail();
  QList<XTreeWidgetItem*> selected = _poitem->selectedItems();
  for (int i = 0; i < selected.size(); i++)
  {
      ParameterList params;
      params.append("mode", "new");
      params.append("vohead_id", _voheadid);
      params.append("poitem_id",  ((XTreeWidgetItem*)(selected[i]))->id());
      params.append("curr_id", _amountToDistribute->id());
      params.append("effective", _amountToDistribute->effective());

      voucherItem newdlg(this, "", TRUE);
      newdlg.set(params);
      if (newdlg.exec() == XDialog::Rejected)
      {
        // nothing to do:
        // voucherItem wraps itself in a transaction and rolls back on reject
      }
      else
      {
        _poNumber->setEnabled(FALSE);
      }
  }
  sFillList();
  sPopulateDistributed();
}

void voucher::sDistributeLine()
{
  saveDetail();
  QList<XTreeWidgetItem*> selected = _poitem->selectedItems();
  for (int i = 0; i < selected.size(); i++)
  {
    q.prepare("SELECT distributeVoucherLine(:vohead_id,:poitem_id,:curr_id) "
              "AS result;");
    q.bindValue(":vohead_id", _voheadid);
    q.bindValue(":poitem_id",  ((XTreeWidgetItem*)(selected[i]))->id());
    q.bindValue(":curr_id", _amountToDistribute->id());
    q.exec();
    if (q.first())
    {
      int result = q.value("result").toInt();
      if (result < 0)
        systemError(this,
                    storedProcErrorLookup("distributeVoucherLine", result),
                    __FILE__, __LINE__);
      else
      {
        _poNumber->setEnabled(FALSE);
      }
    }
    else if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }
  sFillList();
  sPopulateDistributed();
}

void voucher::sClear()
{
    QList<XTreeWidgetItem*> selected = _poitem->selectedItems();
    for (int i = 0; i < selected.size(); i++)
    {
        QString sql( "DELETE FROM vodist "
             "  WHERE ((vodist_poitem_id=:poitem_id) "
             "  AND (vodist_vohead_id=:vohead_id)); "

             "  UPDATE recv SET recv_vohead_id=NULL, recv_voitem_id=NULL "
             "  WHERE ((recv_vohead_id=:vohead_id) "
             "  AND (recv_order_type='PO') "
             "  AND (recv_orderitem_id=:poitem_id)); "

             "  DELETE FROM voitem "
             "  WHERE ((voitem_poitem_id=:poitem_id) "
             "  AND (voitem_vohead_id=:vohead_id)); "

             "  UPDATE poreject SET poreject_vohead_id=NULL, poreject_voitem_id=NULL "
             "  WHERE ((poreject_vohead_id=:vohead_id) "
             "  AND (poreject_poitem_id=:poitem_id)); " );
        q.prepare(sql);
        q.bindValue(":vohead_id", _voheadid);
        q.bindValue(":poitem_id", ((XTreeWidgetItem*)(selected[i]))->id());
        q.exec();

        if (q.lastError().type() != QSqlError::NoError)
        {
        systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
        return;
        }
     }
    _poNumber->setEnabled(true);
    sFillList();
    sPopulateDistributed();
}

void voucher::sDistributeAll()
{
  saveDetail();
  _poitem->selectAll();
  QList<XTreeWidgetItem*> selected = _poitem->selectedItems();
  for (int i = 0; i < selected.size(); i++)
  {
    q.prepare("SELECT distributeVoucherLine(:vohead_id,:poitem_id,:curr_id) "
              "AS result;");
    q.bindValue(":vohead_id", _voheadid);
    q.bindValue(":poitem_id", ((XTreeWidgetItem*)(selected[i]))->id());
    q.bindValue(":curr_id", _amountToDistribute->id());
    q.exec();
    if (q.first())
    {
      int result = q.value("result").toInt();
      if (result < 0)
        systemError(this,
                    storedProcErrorLookup("distributeVoucherLine", result),
                    __FILE__, __LINE__);
      else
      {
        _poNumber->setEnabled(FALSE);
      }
    }
    else if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }
  sFillList();
  sPopulateDistributed();
}

void voucher::sNewMiscDistribution()
{
  saveDetail();
  ParameterList params;
  params.append("mode", "new");
  params.append("vohead_id", _voheadid);
  params.append("curr_id", _amountToDistribute->id());
  params.append("curr_effective", _amountToDistribute->effective());
  params.append("amount", _balance->localValue());
  if (_taxzone->isValid())
    params.append("taxzone_id", _taxzone->id());

  voucherMiscDistrib newdlg(this, "", TRUE);
  newdlg.set(params);
  if (newdlg.exec() != XDialog::Rejected)
  {
    sFillMiscList();
    sPopulateDistributed();
  }
}

void voucher::sEditMiscDistribution()
{
  saveDetail();
  ParameterList params;
  params.append("mode", "edit");
  params.append("vohead_id", _voheadid);
  params.append("vodist_id", _miscDistrib->id());
  params.append("curr_id", _amountToDistribute->id());
  params.append("curr_effective", _amountToDistribute->effective());
  if (_taxzone->isValid())
    params.append("taxzone_id", _taxzone->id());

  voucherMiscDistrib newdlg(this, "", TRUE);
  newdlg.set(params);
  if (newdlg.exec() != XDialog::Rejected)
  {
    sFillMiscList();
    sPopulateDistributed();
  }
}

void voucher::sDeleteMiscDistribution()
{
  saveDetail();
  q.prepare( "DELETE FROM voheadtax "
             "WHERE (taxhist_parent_id=:vohead_id) "
			 " AND  (taxhist_taxtype_id=getadjustmenttaxtypeid()) "
			 " AND  (taxhist_tax_id = (SELECT vodist_tax_id FROM vodist WHERE (vodist_id=:vodist_id))); "
	         "DELETE FROM vodist "
             "WHERE (vodist_id=:vodist_id);");

  q.bindValue(":vohead_id", _voheadid);
  q.bindValue(":vodist_id", _miscDistrib->id());
  q.exec();
  sFillMiscList();
  sPopulateDistributed();
}

void voucher::sFillList()
{  
  if (_poNumber->isValid())
  {
    q.prepare( "SELECT poitem_id, poitem_linenumber,"
               "       CASE WHEN(poitem_status='C') THEN :closed"
               "            WHEN(poitem_status='U') THEN :unposted"
               "            WHEN(poitem_status='O' AND ((poitem_qty_received-poitem_qty_returned) > 0) AND (poitem_qty_ordered>(poitem_qty_received-poitem_qty_returned))) THEN :partial"
               "            WHEN(poitem_status='O' AND ((poitem_qty_received-poitem_qty_returned) > 0) AND (poitem_qty_ordered<=(poitem_qty_received-poitem_qty_returned))) THEN :received"
               "            WHEN(poitem_status='O') THEN :open"
               "            ELSE poitem_status"
               "       END AS poitemstatus,"
               "       COALESCE(item_number, poitem_vend_item_number) AS itemnumber,"
               "       COALESCE(uom_name, poitem_vend_uom) AS uom,"
               "       poitem_vend_item_number, poitem_vend_uom,"
               "       poitem_qty_ordered,"
               "       ( SELECT COALESCE(SUM(porecv_qty), 0)"
               "         FROM porecv"
               "         WHERE ( (porecv_posted)"
               "           AND (porecv_invoiced)"
               "           AND (porecv_poitem_id=poitem_id) ) ) AS qtyinvoiced,"
               "       ( SELECT COALESCE(SUM(porecv_qty), 0)"
               "         FROM porecv"
               "         WHERE ( (porecv_posted)"
               "           AND (NOT porecv_invoiced)"
               "           AND (porecv_vohead_id IS NULL)"
               "           AND (porecv_poitem_id=poitem_id) ) ) AS qtyreceived,"
               "       ( SELECT COALESCE(SUM(poreject_qty), 0)"
               "         FROM poreject"
               "         WHERE ( (poreject_posted)"
               "           AND (NOT poreject_invoiced)"
               "           AND (poreject_vohead_id IS NULL)"
               "           AND (poreject_poitem_id=poitem_id) ) ) AS qtyrejected,"
               "       ( SELECT COALESCE(SUM(vodist_qty), 0)"
               "         FROM vodist"
               "         WHERE vodist_poitem_id=poitem_id"
               "           AND vodist_vohead_id=:vohead_id ) AS invoiceqty, "
               "       ( SELECT COALESCE(SUM(vodist_amount), 0)"
               "         FROM vodist"
               "         WHERE vodist_poitem_id=poitem_id"
               "           AND vodist_vohead_id=:vohead_id ) "
               "     + ( SELECT COALESCE(SUM(COALESCE(voitem_freight,0)), 0)"
               "         FROM voitem"
               "         WHERE voitem_poitem_id=poitem_id"
               "           AND   voitem_vohead_id=:vohead_id ) AS invoiceamount,"
               "       poitem_unitprice,"
               "       (poitem_unitprice * poitem_qty_ordered) AS extprice,"
               "       poitem_freight,"
               "       'qty' AS poitem_qty_ordered_xtnumericrole,"
               "       'qty' AS qtyinvoiced_xtnumericrole,"
               "       'qty' AS qtyreceived_xtnumericrole,"
               "       'qty' AS qtyrejected_xtnumericrole,"
               "       'qty' AS invoiceqty_xtnumericrole,"
               "       'curr' AS invoiceamount_xtnumericrole,"
               "       'curr' AS poitem_unitprice_xtnumericrole,"
               "       'curr' AS extprice_xtnumericrole,"
               "       'curr' AS poitem_freight_xtnumericrole "
               "FROM poitem LEFT OUTER JOIN"
               "     ( itemsite JOIN item"
               "       ON (itemsite_item_id=item_id) JOIN uom ON (item_inv_uom_id=uom_id) )"
               "     ON (poitem_itemsite_id=itemsite_id), pohead "
               "WHERE (poitem_pohead_id=:pohead_id) "
               "  AND pohead_id = poitem_pohead_id "
               "GROUP BY poitem_id, poitem_linenumber, poitem_status,"
               "         item_number, uom_name,"
               "         poitem_vend_item_number, poitem_vend_uom,"
               "         poitem_unitprice, poitem_freight,"
               "         poitem_qty_ordered, poitem_qty_received, poitem_qty_returned,"
               "         itemsite_id, pohead_curr_id "
               "ORDER BY poitem_linenumber;" );
    q.bindValue(":vohead_id", _voheadid);
    q.bindValue(":pohead_id", _poNumber->id());
    q.bindValue(":closed", tr("Closed"));
    q.bindValue(":unposted", tr("Unposted"));
    q.bindValue(":partial", tr("Partial"));
    q.bindValue(":received", tr("Received"));
    q.bindValue(":open", tr("Open"));
    q.exec();
    _poitem->populate(q);
    if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }
  else
    _poitem->clear();
}

void voucher::sFillMiscList()
{
  if (_poNumber->isValid())
  {
    q.prepare( "SELECT vodist_id, (formatGLAccount(accnt_id) || ' - ' || accnt_descrip) AS account,"
               "       vodist_amount, 'curr' AS vodist_amount_xtnumericrole "
               "FROM vodist, accnt "
               "WHERE ( (vodist_poitem_id=-1)"
               " AND (vodist_accnt_id=accnt_id)"
               " AND (vodist_vohead_id=:vohead_id) ) "
               "UNION ALL "
               "SELECT vodist_id, (expcat_code || ' - ' || expcat_descrip) AS account,"
               "       vodist_amount, 'curr' AS vodist_amount_xtnumericrole "
               "  FROM vodist, expcat "
               " WHERE ( (vodist_poitem_id=-1)"
               "   AND   (vodist_expcat_id=expcat_id)"
               "   AND   (vodist_vohead_id=:vohead_id) ) "
               "UNION ALL "
               "SELECT vodist_id, (tax_code || ' - ' || tax_descrip) AS account,"
               "       vodist_amount, 'curr' AS vodist_amount_xtnumericrole "
               "  FROM vodist, tax "
               " WHERE ( (vodist_poitem_id=-1)"
               "   AND   (vodist_tax_id=tax_id)"
               "   AND   (vodist_vohead_id=:vohead_id) ) " 
               "ORDER BY account;" );
    q.bindValue(":vohead_id", _voheadid);
    q.exec();
    _miscDistrib->populate(q);
    if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }
}

void voucher::sPopulatePoInfo()
{
  XSqlQuery po;
  po.prepare("SELECT pohead_terms_id, pohead_taxzone_id, vend_1099, "
             "       pohead_curr_id, vend_id, vend_number, vend_name,"
             "       addr_line1, addr_line2"
             "  FROM pohead"
             "  JOIN vendinfo ON (pohead_vend_id=vend_id)"
             "  LEFT OUTER JOIN addr ON (vend_addr_id=addr_id)"
             " WHERE (pohead_id=:pohead_id);" );
  po.bindValue(":pohead_id", _poNumber->id());
  po.exec();
  if (po.first())
  {
    bool gets1099 = po.value("vend_1099").toBool();

    _flagFor1099->setChecked(gets1099);
    _terms->setId(po.value("pohead_terms_id").toInt());
    _taxzone->setId(po.value("pohead_taxzone_id").toInt());
    _amountToDistribute->setId(po.value("pohead_curr_id").toInt());
    _vendid = po.value("vend_id").toInt();
    _vendor->setText(po.value("vend_number").toString());
    _vendName->setText(po.value("vend_name").toString());
    // TODO: replace with a compact AddressCluster when such exists
    _vendAddress1->setText(po.value("addr_line1").toString());
    _vendAddress2->setText(po.value("addr_line2").toString());
  }
  else if (po.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void voucher::sPopulateDistributed()
{
  if (_poNumber->isValid())
  {
    q.prepare( "SELECT (COALESCE(dist,0) + COALESCE(freight,0) + COALESCE(tax,0)) AS distrib"
               "  FROM (SELECT SUM(COALESCE(voitem_freight,0)) AS freight"
               "          FROM voitem"
               "         WHERE (voitem_vohead_id=:vohead_id)) AS data1, "
               "       (SELECT SUM(COALESCE(vodist_amount, 0)) AS dist"
               "          FROM vodist"
               "         WHERE ( (vodist_vohead_id=:vohead_id)"
               "           AND   (vodist_tax_id=-1) )) AS data2, "
               "       (SELECT SUM(tax * -1.0) AS tax "
               "          FROM ("
               "            SELECT ROUND(SUM(taxdetail_tax),2) AS tax "
               "              FROM tax "
               "              JOIN calculateTaxDetailSummary('VO', :vohead_id, 'T') ON (taxdetail_tax_id=tax_id)"
               "             GROUP BY tax_id) AS taxdata) AS data3;" );
    q.bindValue(":vohead_id", _voheadid);
    q.exec();
    if (q.first())
    {
      _amountDistributed->setLocalValue(q.value("distrib").toDouble());
    }
    else if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }
}

void voucher::sPopulateBalanceDue()
{
  _balance->setLocalValue(_amountToDistribute->localValue() - _amountDistributed->localValue());
  if (_balance->isZero())
    _balance->setPaletteForegroundColor(QColor("black"));
  else
    _balance->setPaletteForegroundColor(namedColor("error"));
}

void voucher::populateNumber()
{
  q.exec("SELECT fetchVoNumber() AS vouchernumber;");
  if (q.first())
  {
    _voucherNumber->setText(q.value("vouchernumber").toString());
    _voucherNumber->setEnabled(FALSE);
  }
}
 
void voucher::populate()
{
  XSqlQuery vohead;
  vohead.prepare( "SELECT vohead_number, vohead_pohead_id, vohead_taxzone_id, vohead_terms_id,"
                  "       vohead_distdate, vohead_docdate, vohead_duedate,"
                  "       vohead_invcnumber, vohead_reference,"
                  "       vohead_1099, vohead_amount, vohead_curr_id, vohead_notes "
                  "FROM vohead "
                  "WHERE (vohead_id=:vohead_id);" );
  vohead.bindValue(":vohead_id", _voheadid);
  vohead.exec();
  if (vohead.first())
  {
    _voucherNumber->setText(vohead.value("vohead_number").toString());
    _poNumber->setId(vohead.value("vohead_pohead_id").toInt(), "PO");
    _taxzone->setId(vohead.value("vohead_taxzone_id").toInt());
    _terms->setId(vohead.value("vohead_terms_id").toInt());
    _amountToDistribute->set(vohead.value("vohead_amount").toDouble(),
			     vohead.value("vohead_curr_id").toInt(),
			     vohead.value("vohead_docdate").toDate(), false);

    _distributionDate->setDate(vohead.value("vohead_distdate").toDate(), true);
    _invoiceDate->setDate(vohead.value("vohead_docdate").toDate());
    _dueDate->setDate(vohead.value("vohead_duedate").toDate());
    _invoiceNum->setText(vohead.value("vohead_invcnumber").toString());
    _reference->setText(vohead.value("vohead_reference").toString());
    _flagFor1099->setChecked(vohead.value("vohead_1099").toBool());
    _notes->setText(vohead.value("vohead_notes").toString());

    sFillList();
    sFillMiscList();
    sPopulateDistributed();
  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void voucher::clear()
{
}

void voucher::closeEvent(QCloseEvent *pEvent)
{
  if (isWindowModified())
  {
    QMessageBox::StandardButtons buttons = omfgThis->shuttingDown() ?
                      QMessageBox::Yes | QMessageBox::No :
                      QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel;

    switch (QMessageBox::question(this, tr("Save Voucher?"),
                                  tr("Do you want to save this voucher?"),
                                  buttons, QMessageBox::Yes))
    {
      case QMessageBox::Yes:
        if (! sSave() && ! omfgThis->shuttingDown())
          pEvent->ignore();
        else if (_mode == cNew)
          break; // so we can clean up
        return;
      case QMessageBox::Cancel:
        pEvent->ignore();
        return;
      default:
        break;
    }
  }

  if (_mode == cNew)
  {
    q.prepare("SELECT deleteVoucher(:vohead_id) AS result;");
    q.bindValue(":vohead_id", _voheadid);
    q.exec();
    if (q.first())
    {
      // TODO: change deleteVoucher to return INT instead of current BOOLEAN
      if (! q.value("result").toBool())
        systemError(this, tr("Error deleting temporary Voucher."),
                    __FILE__, __LINE__);
    }
    else if (q.lastError().type() != QSqlError::NoError)
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);

    q.prepare("SELECT releaseVoNumber(:voucherNumber);" );
    q.bindValue(":voucherNumber", _voucherNumber->text());
    q.exec();
    if (q.lastError().type() != QSqlError::NoError)
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
  }

  pEvent->accept();
}

void voucher::sPopulateDistDate()
{
  if ( (_invoiceDate->isValid()) && (!_distributionDate->isValid()) )
  {
    _distributionDate->setDate(_invoiceDate->date(), true);
  }
  sPopulateDueDate();
}

void voucher::sPopulateDueDate()
{
  if ( (_invoiceDate->isValid()) && (!_dueDate->isValid()) )
  {
    q.prepare("SELECT determineDueDate(:terms_id, :invoiceDate) AS duedate;");
    q.bindValue(":terms_id", _terms->id());
    q.bindValue(":invoiceDate", _invoiceDate->date());
    q.exec();
    if (q.first())
      _dueDate->setDate(q.value("duedate").toDate());
  }
}

void voucher::sPopulateMenu( QMenu * pMenu )
{
  pMenu->addAction(tr("View P/O Item..."), this, SLOT(sView()));
}

void voucher::sView()
{
  if(_poitem->id() == -1)
    return;

  ParameterList params;
  params.append("poitem_id", _poitem->id());
  params.append("mode", "view");

  purchaseOrderItem newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void voucher::keyPressEvent( QKeyEvent * e )
{
  if(e->key() == Qt::Key_Return || e->key() == Qt::Key_Enter)
  {
    _save->animateClick();
    e->accept();
  }
  else
    e->ignore();
}

void voucher::saveDetail()
{
  if (_mode != cView)
  {
    q.prepare( "UPDATE vohead "
               "SET vohead_pohead_id=:vohead_pohead_id,"
               "    vohead_taxzone_id=:vohead_taxzone_id,  "
               "    vohead_vend_id=:vohead_vend_id,"
               "    vohead_terms_id=:vohead_terms_id,"
               "    vohead_distdate=COALESCE(:vohead_distdate, CURRENT_DATE),"
               "    vohead_docdate=COALESCE(:vohead_docdate, CURRENT_DATE),"
               "    vohead_duedate=COALESCE(:vohead_duedate, CURRENT_DATE),"
               "    vohead_invcnumber=:vohead_invcnumber,"
               "    vohead_reference=:vohead_reference,"
               "    vohead_amount=:vohead_amount,"
               "    vohead_1099=:vohead_1099, "
               "    vohead_curr_id=:vohead_curr_id, "
               "    vohead_notes=:vohead_notes "
               "WHERE (vohead_id=:vohead_id);" );

    q.bindValue(":vohead_id", _voheadid);
    q.bindValue(":vohead_pohead_id", _poNumber->id());
    if (_taxzone->isValid())
      q.bindValue(":vohead_taxzone_id", _taxzone->id());
    q.bindValue(":vohead_vend_id",  _vendid);
    q.bindValue(":vohead_terms_id", _terms->id());
    q.bindValue(":vohead_distdate", _distributionDate->date());
    q.bindValue(":vohead_docdate", _invoiceDate->date());
    q.bindValue(":vohead_duedate", _dueDate->date());
    q.bindValue(":vohead_invcnumber", _invoiceNum->text().trimmed());
    q.bindValue(":vohead_reference", _reference->text().trimmed());
    q.bindValue(":vohead_amount", _amountToDistribute->localValue());
    q.bindValue(":vohead_1099", QVariant(_flagFor1099->isChecked()));
    q.bindValue(":vohead_curr_id", _amountToDistribute->id());
    q.bindValue(":vohead_notes", _notes->toPlainText());
    q.exec();
    if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }
}

void voucher::enableWindowModifiedSetting()
{
  connect(_amountToDistribute,     SIGNAL(valueChanged()), this, SLOT(sDataChanged()));
  connect(_distributionDate,SIGNAL(newDate(const QDate&)), this, SLOT(sDataChanged()));
  connect(_dueDate,         SIGNAL(newDate(const QDate&)), this, SLOT(sDataChanged()));
  connect(_flagFor1099,             SIGNAL(toggled(bool)), this, SLOT(sDataChanged()));
  connect(_invoiceDate,     SIGNAL(newDate(const QDate&)), this, SLOT(sDataChanged()));
  connect(_invoiceNum,SIGNAL(textChanged(const QString&)), this, SLOT(sDataChanged()));
  connect(_notes,                   SIGNAL(textChanged()), this, SLOT(sDataChanged()));
  connect(_poNumber,                   SIGNAL(newId(int)), this, SLOT(sDataChanged()));
  connect(_reference, SIGNAL(textChanged(const QString&)), this, SLOT(sDataChanged()));
  connect(_taxzone,                    SIGNAL(newID(int)), this, SLOT(sDataChanged()));
  connect(_terms,                      SIGNAL(newID(int)), this, SLOT(sDataChanged()));
}

void voucher::sDataChanged()
{
  setWindowModified(true);
}
