/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "salesOrder.h"
#include <stdlib.h>

#include <QAction>
#include <QCloseEvent>
#include <QKeyEvent>
#include <QMenu>
#include <QMessageBox>
#include <QSqlError>
#include <QValidator>
#include <QVariant>

#include <metasql.h>
#include "creditCard.h"
#include "creditcardprocessor.h"
#include "crmacctcluster.h"
#include "customer.h"
#include "distributeInventory.h"
#include "issueLineToShipping.h"
#include "mqlutil.h"
#include "salesOrderItem.h"
#include "storedProcErrorLookup.h"
#include "taxBreakdown.h"
#include "freightBreakdown.h"
#include "printPackingList.h"
#include "printSoForm.h"
#include "prospect.h"
#include "allocateARCreditMemo.h"
#include "reserveSalesOrderItem.h"
#include "dspReservations.h"
#include "purchaseRequest.h"
#include "purchaseOrder.h"
#include "workOrder.h"

#define cNewQuote   (0x20 | cNew)
#define cEditQuote  (0x20 | cEdit)
#define cViewQuote  (0x20 | cView)

#define ISQUOTE(mode) (((mode) & 0x20) == 0x20)
#define ISORDER(mode) (!ISQUOTE(mode))

#define ISNEW(mode)   (((mode) & 0x0F) == cNew)
#define ISEDIT(mode)  (((mode) & 0x0F) == cEdit)
#define ISVIEW(mode)  (((mode) & 0x0F) == cView)

#define cClosed       0x01
#define cActiveOpen   0x02
#define cInactiveOpen 0x04
#define cCanceled     0x08

#define iDontUpdate   1
#define iAskToUpdate  2
#define iJustUpdate   3

salesOrder::salesOrder(QWidget *parent, const char *name, Qt::WFlags fl)
  : XWidget(parent, name, fl)
{
  setupUi(this);

  sCheckValidContacts();

  connect(_action,              SIGNAL(clicked()),                              this,         SLOT(sAction()));
  connect(_authorize,           SIGNAL(clicked()),                              this,         SLOT(sAuthorizeCC()));
  connect(_charge,              SIGNAL(clicked()),                              this,         SLOT(sChargeCC()));
  connect(_clear,               SIGNAL(pressed()),                              this,         SLOT(sClear()));
  connect(_copyToShipto,        SIGNAL(clicked()),                              this,         SLOT(sCopyToShipto()));
  connect(_cust,                SIGNAL(newId(int)),                             this,         SLOT(sPopulateCustomerInfo(int)));
  connect(_delete,              SIGNAL(clicked()),                              this,         SLOT(sDelete()));
  connect(_downCC,              SIGNAL(clicked()),                              this,         SLOT(sMoveDown()));
  connect(_edit,                SIGNAL(clicked()),                              this,         SLOT(sEdit()));
  connect(_editCC,              SIGNAL(clicked()),                              this,         SLOT(sEditCreditCard()));
  connect(_new,                 SIGNAL(clicked()),                              this,         SLOT(sNew()));
  connect(_newCC,               SIGNAL(clicked()),                              this,         SLOT(sNewCreditCard()));
  //connect(_newCust,             SIGNAL(clicked()),                              this,         SLOT(sNewCust()));
  connect(_orderNumber,         SIGNAL(lostFocus()),                            this,         SLOT(sHandleOrderNumber()));
  connect(_orderNumber,         SIGNAL(textChanged(const QString &)),           this,         SLOT(sSetUserEnteredOrderNumber()));
  connect(_save,                SIGNAL(clicked()),                              this,         SLOT(sSave()));
  connect(_saveAndAdd,          SIGNAL(clicked()),                              this,         SLOT(sSaveAndAdd()));
  connect(_shippingCharges,     SIGNAL(newID(int)),                             this,         SLOT(sHandleShipchrg(int)));
  connect(_shipVia,             SIGNAL(textChanged(const QString &)),           this,         SLOT(sFillItemList()));
  connect(_shipToAddr,          SIGNAL(changed()),                              this,         SLOT(sConvertShipTo()));
  connect(_shipToName,          SIGNAL(textChanged(const QString &)),           this,         SLOT(sConvertShipTo()));
  connect(_shipTo,              SIGNAL(newId(int)),                             this,         SLOT(sParseShipToNumber()));
  connect(_showCanceled,        SIGNAL(toggled(bool)),                          this,         SLOT(sFillItemList()));
  connect(_soitem,              SIGNAL(populateMenu(QMenu*,QTreeWidgetItem *)), this,         SLOT(sPopulateMenu(QMenu *)));
  connect(_soitem,              SIGNAL(itemSelectionChanged()),                 this,         SLOT(sHandleButtons()));
  connect(_taxZone,             SIGNAL(newID(int)),                             this,         SLOT(sTaxZoneChanged()));
  connect(_taxLit,              SIGNAL(leftClickedURL(const QString &)),        this,         SLOT(sTaxDetail()));
  connect(_freightLit,          SIGNAL(leftClickedURL(const QString &)),        this,         SLOT(sFreightDetail()));
  connect(_upCC,                SIGNAL(clicked()),                              this,         SLOT(sMoveUp()));
  connect(_viewCC,              SIGNAL(clicked()),                              this,         SLOT(sViewCreditCard()));
  connect(_warehouse,           SIGNAL(newID(int)),                             this,         SLOT(sPopulateFOB(int)));
  connect(_issueStock,          SIGNAL(clicked()),                              this,         SLOT(sIssueStock()));
  connect(_issueLineBalance,    SIGNAL(clicked()),                              this,         SLOT(sIssueLineBalance()));
  connect(_reserveStock,        SIGNAL(clicked()),                              this,         SLOT(sReserveStock()));
  connect(_reserveLineBalance,  SIGNAL(clicked()),                              this,         SLOT(sReserveLineBalance()));
  connect(_subtotal,            SIGNAL(valueChanged()),                         this,         SLOT(sCalculateTotal()));
  connect(_miscCharge,          SIGNAL(valueChanged()),                         this,         SLOT(sCalculateTotal()));
  connect(_freight,             SIGNAL(valueChanged()),                         this,         SLOT(sFreightChanged()));
  if (_privileges->check("ApplyARMemos"))
    connect(_allocatedCMLit,    SIGNAL(leftClickedURL(const QString &)),        this,         SLOT(sCreditAllocate()));
  connect(_allocatedCM,         SIGNAL(valueChanged()),                         this,         SLOT(sCalculateTotal()));
  connect(_outstandingCM,       SIGNAL(valueChanged()),                         this,         SLOT(sCalculateTotal()));
  connect(_authCC,              SIGNAL(valueChanged()),                         this,         SLOT(sCalculateTotal()));
  connect(_more,                SIGNAL(clicked()),                              this,         SLOT(sHandleMore()));
  connect(_orderDate,           SIGNAL(newDate(QDate)),                         this,         SLOT(sOrderDateChanged()));
  connect(_shipDate,            SIGNAL(newDate(QDate)),                         this,         SLOT(sShipDateChanged()));
  connect(_cust,                SIGNAL(newCrmacctId(int)),                      _billToAddr,  SLOT(setSearchAcct(int)));
  connect(_cust,                SIGNAL(newCrmacctId(int)),                      _shipToAddr,  SLOT(setSearchAcct(int)));

  connect(_billToAddr,          SIGNAL(addressChanged(QString,QString,QString,QString,QString,QString, QString)),
          _billToCntct, SLOT(setNewAddr(QString,QString,QString,QString,QString,QString, QString)));

  connect(_shipToAddr,          SIGNAL(addressChanged(QString,QString,QString,QString,QString,QString, QString)),
          _shipToCntct, SLOT(setNewAddr(QString,QString,QString,QString,QString,QString, QString)));

  _saved = false;

  setFreeFormShipto(false);

  _soheadid          = -1;
  _orderNumberGen    = 0;

  _numSelected       = 0;

  _calcfreight       = false;
  _freightCache      = 0;
  _taxzoneidCache    = -1;
  _custtaxzoneid     = -1;
  _amountOutstanding = 0.0;
  _crmacctid         =-1;
  _locked            =false;

  _captive       = FALSE;

  _ignoreSignals = TRUE;

  _orderCurrency->setLabel(_orderCurrencyLit);

  _shipTo->setNameVisible(false);
  _shipTo->setDescriptionVisible(false);

  _orderNumber->setValidator(omfgThis->orderVal());
  _CCCVV->setValidator(new QIntValidator(100, 9999, this));
  _weight->setValidator(omfgThis->weightVal());
  _commission->setValidator(omfgThis->percentVal());

  _origin->append(0, tr("Customer"),   "C");
  _origin->append(1, tr("Internet"),   "I");
  _origin->append(2, tr("Sales Rep."), "S");

  _soitem->addColumn(tr("#"),           _seqColumn, Qt::AlignCenter,true, "f_linenumber");
  _soitem->addColumn(tr("Kit Seq. #"),  _seqColumn, Qt::AlignRight, false,"coitem_subnumber");
  _soitem->addColumn(tr("Item"),       _itemColumn, Qt::AlignLeft,  true, "item_number");
  _soitem->addColumn(tr("Type"),       _itemColumn, Qt::AlignLeft,  false,"item_type");
  _soitem->addColumn(tr("Description"),         -1, Qt::AlignLeft,  true, "description");
  _soitem->addColumn(tr("Site"),        _whsColumn, Qt::AlignCenter,true, "warehous_code");
  _soitem->addColumn(tr("Status"),   _statusColumn, Qt::AlignCenter,true, "enhanced_status");
  _soitem->addColumn(tr("Firm"),                 0, Qt::AlignCenter,false, "coitem_firm");
  _soitem->addColumn(tr("Sched. Date"),_dateColumn, Qt::AlignCenter,true, "coitem_scheddate");
  _soitem->addColumn(tr("Ordered"),     _qtyColumn, Qt::AlignRight, true, "coitem_qtyord");
  _soitem->addColumn(tr("Qty UOM"),     (int)(_uomColumn*1.5), Qt::AlignLeft,  true, "qty_uom");
  _soitem->addColumn(tr("Shipped"),     _qtyColumn, Qt::AlignRight, true, "qtyshipped");
  _soitem->addColumn(tr("At Shipping"), _qtyColumn, Qt::AlignRight, false, "qtyatshipping");
  _soitem->addColumn(tr("Balance"),     _qtyColumn, Qt::AlignRight, false, "balance");
  _soitem->addColumn(tr("Price UOM"),   _uomColumn, Qt::AlignLeft,  false, "price_uom");
  _soitem->addColumn(tr("Price"),     _priceColumn, Qt::AlignRight, true, "coitem_price");
  _soitem->addColumn(tr("Extended"),  _priceColumn, Qt::AlignRight, true, "extprice");
  _soitem->addColumn(tr("Cust. Price"),     _priceColumn, Qt::AlignRight, false, "coitem_custprice");
  _soitem->addColumn(tr("Cust. Discount"),  _priceColumn, Qt::AlignRight, false, "discountfromcust");
  _soitem->addColumn(tr("Supply Type"), _itemColumn, Qt::AlignCenter, false, "spplytype");
  _soitem->addColumn(tr("Order Number"),_itemColumn, Qt::AlignCenter, false, "ordrnumbr");

  _cc->addColumn(tr("Sequence"),_itemColumn, Qt::AlignLeft, true, "ccard_seq");
  _cc->addColumn(tr("Type"),    _itemColumn, Qt::AlignLeft, true, "type");
  _cc->addColumn(tr("Number"),  _itemColumn, Qt::AlignRight,true, "f_number");
  _cc->addColumn(tr("Active"),  _itemColumn, Qt::AlignLeft, true, "ccard_active");
  _cc->addColumn(tr("Name"),    _itemColumn, Qt::AlignLeft, true, "ccard_name");
  _cc->addColumn(tr("Expiration Date"),  -1, Qt::AlignLeft, true, "expiration");

  _quotestatusLit->hide();
  _quotestaus->hide();

  sPopulateFOB(_warehouse->id());

  _ignoreSignals = FALSE;

  if (!_privileges->check("ShowMarginsOnSalesOrder"))
  {
    _margin->hide();
    _marginLit->hide();
  }

  _project->setType(ProjectLineEdit::SalesOrder);
  if (!_metrics->boolean("UseProjects"))
    _project->hide();

  // If not multi-warehouse hide whs control
  if (!_metrics->boolean("MultiWhs"))
  {
    _shippingWhseLit->hide();
    _warehouse->hide();
  }

  if (!_metrics->boolean("EnableReturnAuth"))
    _holdType->removeItem(4);

  if (!_metrics->boolean("CCAccept") || !_privileges->check("ProcessCreditCards"))
  {
    _salesOrderInformation->removeTab(_salesOrderInformation->indexOf(_creditCardPage));
  }

  if (_metrics->boolean("EnableSOReservations"))
  {
    _requireInventory->setChecked(true);
    _requireInventory->setEnabled(false);
  }

  if (!_metrics->boolean("AlwaysShowSaveAndAdd"))
    _saveAndAdd->hide();

  _more->setChecked(_preferences->boolean("SoShowAll"));

  _miscChargeAccount->setType(GLCluster::cRevenue | GLCluster::cExpense);

  sHandleMore();
}

salesOrder::~salesOrder()
{
  // no need to delete child widgets, Qt does it all for us
}

void salesOrder::languageChange()
{
  retranslateUi(this);
}

enum SetResponse salesOrder:: set(const ParameterList &pParams)
{
  XWidget::set(pParams);
  QVariant  param;
  bool      valid;

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      setObjectName("salesOrder new");
      _mode = cNew;

      _cust->setType(CLineEdit::ActiveCustomers);
      _salesRep->setType(XComboBox::SalesRepsActive);
      _comments->setType(Comments::SalesOrder);
      _documents->setType(Documents::SalesOrder);
      _calcfreight = _metrics->boolean("CalculateFreight");

      connect(omfgThis, SIGNAL(salesOrdersUpdated(int, bool)), this, SLOT(sHandleSalesOrderEvent(int, bool)));
    }
    else if (param.toString() == "newQuote")
    {
      _mode = cNewQuote;

      _cust->setType(CLineEdit::ActiveCustomersAndProspects);
      _salesRep->setType(XComboBox::SalesRepsActive);
      _calcfreight = _metrics->boolean("CalculateFreight");
      _action->hide();

      _CCAmount->hide();
      _CCCVVLit->hide();
      _CCCVV->hide();
      _newCC->hide();
      _editCC->hide();
      _viewCC->hide();
      _upCC->hide();
      _downCC->hide();
      _authorize->hide();
      _charge->hide();
      _quotestatusLit->show();
      _quotestaus->show();
      _quotestaus->setText("Open");

      connect(omfgThis, SIGNAL(quotesUpdated(int, bool)), this, SLOT(sHandleSalesOrderEvent(int, bool)));
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;

      if (_metrics->boolean("AlwaysShowSaveAndAdd"))
        _saveAndAdd->setEnabled(true);
      else
        _saveAndAdd->hide();
      _comments->setType(Comments::SalesOrder);
      _documents->setType(Documents::SalesOrder);
      _cust->setType(CLineEdit::AllCustomers);

      connect(omfgThis, SIGNAL(salesOrdersUpdated(int, bool)), this, SLOT(sHandleSalesOrderEvent(int, bool)));
    }
    else if (param.toString() == "editQuote")
    {
      _mode = cEditQuote;

      _cust->setType(CLineEdit::AllCustomersAndProspects);
      _action->setEnabled(FALSE);
      _action->hide();

      _CCAmount->hide();
      _CCCVVLit->hide();
      _CCCVV->hide();
      _newCC->hide();
      _editCC->hide();
      _viewCC->hide();
      _upCC->hide();
      _downCC->hide();
      _authorize->hide();
      _charge->hide();
      _quotestatusLit->show();
      _quotestaus->show();

      connect(omfgThis, SIGNAL(quotesUpdated(int, bool)), this, SLOT(sHandleSalesOrderEvent(int, bool)));
    }
    else if (param.toString() == "view")
    {
      setViewMode();
      _cust->setType(CLineEdit::AllCustomers);

      _issueStock->hide();
      _issueLineBalance->hide();

      _reserveStock->hide();
      _reserveLineBalance->hide();
    }
    else if (param.toString() == "viewQuote")
    {
      _mode = cViewQuote;

      _orderNumber->setEnabled(FALSE);
      _packDate->setEnabled(FALSE);
      _cust->setReadOnly(TRUE);
      _warehouse->setEnabled(FALSE);
      _salesRep->setEnabled(FALSE);
      _commission->setEnabled(FALSE);
      _taxZone->setEnabled(FALSE);
      _terms->setEnabled(FALSE);
      _terms->setType(XComboBox::Terms);
      _origin->setEnabled(FALSE);
      _fob->setEnabled(FALSE);
      _shipVia->setEnabled(FALSE);
      _shippingCharges->setEnabled(FALSE);
      _shippingForm->setEnabled(FALSE);
      _miscCharge->setEnabled(FALSE);
      _miscChargeDescription->setEnabled(FALSE);
      _miscChargeAccount->setReadOnly(TRUE);
      _freight->setEnabled(FALSE);
      _orderComments->setEnabled(FALSE);
      _shippingComments->setEnabled(FALSE);
      _custPONumber->setEnabled(FALSE);
      _holdType->setEnabled(FALSE);
      _edit->setText(tr("View"));
      _cust->setType(CLineEdit::AllCustomersAndProspects);
      _comments->setReadOnly(true);
      _documents->setReadOnly(true);
      _copyToShipto->setEnabled(FALSE);
      _orderCurrency->setEnabled(FALSE);
      _save->hide();
      _clear->hide();
      _action->hide();
      _delete->hide();
      _quotestatusLit->show();
      _quotestaus->show();
    }
  }

  if (ISNEW(_mode))
  {
    _ignoreSignals = TRUE;

    populateOrderNumber();
    if (_orderNumber->text().isEmpty())
      _orderNumber->setFocus();
    else
      _cust->setFocus();

    _ignoreSignals = FALSE;

    if (ISORDER(_mode))
      q.exec("SELECT NEXTVAL('cohead_cohead_id_seq') AS head_id;");
    else  // (ISQUOTE(_mode))
      q.exec("SELECT NEXTVAL('quhead_quhead_id_seq') AS head_id;");
    if (q.first())
    {
      _soheadid = q.value("head_id").toInt();
      _comments->setId(_soheadid);
      _documents->setId(_soheadid);
      _orderDateCache = omfgThis->dbDate();
      _orderDate->setDate(_orderDateCache, true);
    }
    else if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return UndefinedError;
    }

    if (ISORDER(_mode))
    {
      populateCMInfo();
      populateCCInfo();
      sFillCcardList();
    }

    _captive = FALSE;
    _edit->setEnabled(FALSE);
    _action->setEnabled(FALSE);
    _delete->setEnabled(FALSE);
    _close->setText("&Cancel");

    connect(_cust, SIGNAL(valid(bool)), _new, SLOT(setEnabled(bool)));
  }
  else if (ISEDIT(_mode))
  {
    _captive = TRUE;
    _orderNumber->setEnabled(FALSE);
    _cust->setReadOnly(TRUE);
    _orderCurrency->setEnabled(FALSE);

    connect(_cust, SIGNAL(valid(bool)), _new, SLOT(setEnabled(bool)));

    _new->setFocus();
  }
  else if (ISVIEW(_mode))
  {
    _CCAmount->hide();
    _CCCVVLit->hide();
    _CCCVV->hide();
    _newCC->hide();
    _editCC->hide();
    _viewCC->hide();
    _upCC->hide();
    _downCC->hide();
    _authorize->hide();
    _charge->hide();
    _project->setReadOnly(true);

    _close->setFocus();
  }

  if (ISQUOTE(_mode))
  {
    setWindowTitle(tr("Quote"));

    _comments->setType(Comments::Quote);
    _documents->setType(Documents::Quote);

    _saveAndAdd->hide();
    _fromQuote->hide();
    _fromQuoteLit->hide();
    _shipComplete->hide();
    _allocatedCM->hide();
    _allocatedCMLit->hide();
    _outstandingCM->hide();
    _outstandingCMLit->hide();
    _authCC->hide();
    _authCCLit->hide();
    _balance->hide();
    _balanceLit->hide();
    _holdType->hide();
    _holdTypeLit->hide();
    _shippingCharges->hide();
    _shippingChargesLit->hide();
    _shippingForm->hide();
    _shippingFormLit->hide();
    _printSO->hide();

    _salesOrderInformation->removeTab(_salesOrderInformation->indexOf(_creditCardPage));
    _showCanceled->hide();
    _total->setBaseVisible(true);
  }
  else
  {
    _printSO->setChecked(_metrics->boolean("DefaultPrintSOOnSave"));
    _expireLit->hide();
    _expire->hide();
  }

  if (_metrics->boolean("HideSOMiscCharge"))
  {
    _miscChargeDescriptionLit->hide();
    _miscChargeDescription->hide();
    _miscChargeAccountLit->hide();
    _miscChargeAccount->hide();
    _miscChargeAmountLit->hide();
    _miscCharge->hide();
  }

  if (ISQUOTE(_mode) || !_metrics->boolean("EnableSOShipping"))
  {
    _requireInventory->hide();
    _issueStock->hide();
    _issueLineBalance->hide();
    _amountAtShippingLit->hide();
    _amountAtShipping->hide();
    _soitem->hideColumn("qtyatshipping");
    _soitem->hideColumn("balance");
  }
  else
    _soitem->setSelectionMode(QAbstractItemView::ExtendedSelection);

  if (ISQUOTE(_mode) || !_metrics->boolean("EnableSOReservations"))
  {
    _reserveStock->hide();
    _reserveLineBalance->hide();
  }

  param = pParams.value("cust_id", &valid);
  if (valid)
    _cust->setId(param.toInt());

  param = pParams.value("ophead_id", &valid);
  if (valid)
    _opportunity->setId(param.toInt());

  param = pParams.value("sohead_id", &valid);
  if (valid)
  {
    _soheadid = param.toInt();
    if (cEdit == _mode)
      setObjectName(QString("salesOrder edit %1").arg(_soheadid));
    else if (cView == _mode)
      setObjectName(QString("salesOrder view %1").arg(_soheadid));
      populate();
    populateCMInfo();
    populateCCInfo();
    sFillCcardList();
  }

  param = pParams.value("quhead_id", &valid);
  if (valid)
  {
    _soheadid = param.toInt();
    populate();
  }

  if ( (pParams.inList("enableSaveAndAdd")) && ((_mode == cNew) || (_mode == cEdit)) )
  {
    _saveAndAdd->show();
    _saveAndAdd->setEnabled(true);
  }

  if (ISNEW(_mode) || ISEDIT(_mode))
  {
    _orderDate->setEnabled(_privileges->check("OverrideSODate"));
    _packDate->setEnabled(_privileges->check("AlterPackDate"));
  }
  else
  {
    _orderDate->setEnabled(FALSE);
    _packDate->setEnabled(FALSE);
  }

  param = pParams.value("captive", &valid);
  if (valid)
    _captive = true;

  return NoError;
}

void salesOrder::sSave()
{
  if (save(false))
  {
    if (_printSO->isChecked())
    {
      ParameterList params;
      params.append("sohead_id", _soheadid);

      printSoForm newdlgX(this, "", true);
      newdlgX.set(params);
      newdlgX.exec();
    }

    if (_captive)
      close();
    else
      clear();
  }
}

void salesOrder::sSaveAndAdd()
{
  if (save(false))
  {
    q.prepare("SELECT addToPackingListBatch(:sohead_id) AS result;");
    q.bindValue(":sohead_id", _soheadid);
    q.exec();

    if (_printSO->isChecked())
    {
      ParameterList params;
      params.append("sohead_id", _soheadid);

      printPackingList newdlgP(this, "", true);
      newdlgP.set(params);
      newdlgP.exec();

      printSoForm newdlgS(this, "", true);
      newdlgS.set(params);
      newdlgS.exec();
    }

    if (_captive)
      close();
    else
      clear();
  }
}

bool salesOrder::save(bool partial)
{
  //  Make sure that all of the required field have been populated
  if (!_orderDate->isValid())
  {
    QMessageBox::warning( this, tr("Cannot Save Sales Order"),
                              tr("You must enter an Order Date for this order before you may save it.") );
    _orderDate->setFocus();
    return FALSE;
  }

  if ( (!_shipDate->isValid()) && (_metrics->value("soPriceEffective") == "ScheduleDate") )
  {
    QMessageBox::warning( this, tr("Cannot Save Sales Order"),
                              tr("You must enter an Scheduled Date for this order before you may save it.") );
    _shipDate->setFocus();
    return FALSE;
  }

  if (!_cust->isValid())
  {
    QMessageBox::warning( this, tr("Cannot Save Sales Order"),
                              tr("You must select a Customer for this order before you may save it.") );
    _cust->setFocus();
    return FALSE;
  }

  if (!_salesRep->isValid())
  {
    QMessageBox::warning( this, tr("Cannot Save Sales Order"),
                              tr("You must select a Sales Rep. for this order before you may save it.") );
    _salesRep->setFocus();
    return FALSE;
  }

  if (!_terms->isValid())
  {
    QMessageBox::warning( this, tr("Cannot Save Sales Order"),
                              tr("You must select the Terms for this order before you may save it.") );
    _terms->setFocus();
    return FALSE;
  }

  if ( (_shipTo->id() == -1) && (!_shipToAddr->isEnabled()) )
  {
    QMessageBox::warning( this, tr("Cannot Save Sales Order"),
                              tr("You must select a Ship-To for this order before you may save it.") );
    _shipTo->setFocus();
    return FALSE;
  }

  if (_total->localValue() < 0)
  {
    QMessageBox::information(this, tr("Total Less than Zero"),
                              tr("<p>The Total must be a positive value.") );
    _cust->setFocus();
    return FALSE;
  }

  if (_opportunity->isValid())
  {
    q.prepare( "SELECT crmacct_cust_id, crmacct_prospect_id "
               "FROM crmacct JOIN ophead ON (crmacct_id = ophead_crmacct_id) "
               "WHERE (ophead_id = :ophead_id);" );
    q.bindValue(":ophead_id", _opportunity->id());
    q.exec();
    if (q.first())
    {
      if (q.value("crmacct_cust_id").toInt() == 0 && q.value("crmacct_prospect_id").toInt() == 0)
      {
        QMessageBox::warning( this, tr("Cannot Save Sales Order"),
                              tr("Only opportunities from Customers or Prospects can be related.") );
        _opportunity->setFocus();
        return FALSE;
      }
    }
    else if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return FALSE;
    }
  }

  if (ISORDER(_mode))
  {
    if (_usesPos && !partial)
    {
      if (_custPONumber->text().trimmed().length() == 0)
      {
        QMessageBox::warning( this, tr("Cannot Save Sales Order"),
                                tr("You must enter a Customer P/O for this Sales Order before you may save it.") );
        _custPONumber->setFocus();
        return FALSE;
      }

      if (!_blanketPos)
      {
        q.prepare( "SELECT cohead_id"
                   "  FROM cohead"
                   " WHERE ((cohead_cust_id=:cohead_cust_id)"
                   "   AND  (cohead_id<>:cohead_id)"
                   "   AND  (UPPER(cohead_custponumber) = UPPER(:cohead_custponumber)) )"
                   " UNION "
                   "SELECT quhead_id"
                   "  FROM quhead"
                   " WHERE ((quhead_cust_id=:cohead_cust_id)"
                   "   AND  (quhead_id<>:cohead_id)"
                   "   AND  (UPPER(quhead_custponumber) = UPPER(:cohead_custponumber)) );" );
        q.bindValue(":cohead_cust_id", _cust->id());
        q.bindValue(":cohead_id", _soheadid);
        q.bindValue(":cohead_custponumber", _custPONumber->text());
        q.exec();
        if (q.first())
        {
          QMessageBox::warning( this, tr("Cannot Save Sales Order"),
                                tr("<p>This Customer does not use Blanket P/O "
                                     "Numbers and the P/O Number you entered has "
                                     "already been used for another Sales Order."
                                     "Please verify the P/O Number and either"
                                     "enter a new P/O Number or add to the"
                                     "existing Sales Order." ) );
          _custPONumber->setFocus();
          return FALSE;
        }
        else if (q.lastError().type() != QSqlError::NoError)
        {
          systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
          return FALSE;
        }
      }
    }
  }

  if (!partial && _soitem->topLevelItemCount() == 0)
  {
    QMessageBox::warning( this, tr("Create Line Items for this Order"),
                          tr("<p>You must create at least one Line Item for "
                               "this order before you may save it."));
    _new->setFocus();
    return FALSE;
  }

  if (_orderNumber->text().toInt() == 0)
  {
    QMessageBox::warning( this, tr("Invalid S/O # Entered"),
                          tr( "<p>You must enter a valid Number for this "
                                "order before you may save it." ) );
    _orderNumber->setFocus();
  }

  //  We can't post a Misc. Charge without a Sales Account
  if ( (!_miscCharge->isZero()) && (!_miscChargeAccount->isValid()) )
  {
    QMessageBox::warning( this, tr("No Misc. Charge Account Number"),
                          tr("<p>You may not enter a Misc. Charge without "
                               "indicating the G/L Sales Account number for the "
                               "charge.  Please set the Misc. Charge amount to 0 "
                               "or select a Misc. Charge Sales Account." ) );
    _salesOrderInformation->setCurrentIndex(_salesOrderInformation->indexOf(_lineItemsPage));
    _miscChargeAccount->setFocus();
    return FALSE;
  }

  if ((_mode == cEdit) || ((_mode == cNew) && _saved))
    q.prepare( "UPDATE cohead "
               "SET cohead_custponumber=:custponumber, cohead_shipto_id=:shipto_id, cohead_cust_id=:cust_id,"
               "    cohead_billtoname=:billtoname, cohead_billtoaddress1=:billtoaddress1,"
               "    cohead_billtoaddress2=:billtoaddress2, cohead_billtoaddress3=:billtoaddress3,"
               "    cohead_billtocity=:billtocity, cohead_billtostate=:billtostate, cohead_billtozipcode=:billtozipcode,"
               "    cohead_billtocountry=:billtocountry,"
               "    cohead_shiptoname=:shiptoname, cohead_shiptoaddress1=:shiptoaddress1,"
               "    cohead_shiptoaddress2=:shiptoaddress2, cohead_shiptoaddress3=:shiptoaddress3,"
               "    cohead_shiptocity=:shiptocity, cohead_shiptostate=:shiptostate, cohead_shiptozipcode=:shiptozipcode,"
               "    cohead_shiptocountry=:shiptocountry,"
               "    cohead_orderdate=:orderdate, cohead_packdate=:packdate,"
               "    cohead_salesrep_id=:salesrep_id, cohead_commission=:commission,"
               "    cohead_taxzone_id=:taxzone_id, cohead_terms_id=:terms_id, cohead_origin=:origin,"
               "    cohead_fob=:fob, cohead_shipvia=:shipvia, cohead_warehous_id=:warehous_id,"
               "    cohead_freight=:freight, cohead_calcfreight=:calcfreight,"
               "    cohead_misc=:misc, cohead_misc_accnt_id=:misc_accnt_id, cohead_misc_descrip=:misc_descrip,"
               "    cohead_holdtype=:holdtype,"
               "    cohead_ordercomments=:ordercomments, cohead_shipcomments=:shipcomments,"
               "    cohead_shipchrg_id=:shipchrg_id, cohead_shipform_id=:shipform_id,"
               "    cohead_prj_id=:prj_id,"
               "    cohead_ophead_id=:ophead_id,"
               "    cohead_curr_id = :curr_id,"
               "    cohead_shipcomplete=:cohead_shipcomplete,"
               "    cohead_shipto_cntct_id=:shipto_cntct_id,"
               "    cohead_shipto_cntct_honorific=:shipto_cntct_honorific,"
               "    cohead_shipto_cntct_first_name=:shipto_cntct_first_name,"
               "    cohead_shipto_cntct_middle=:shipto_cntct_middle,"
               "    cohead_shipto_cntct_last_name=:shipto_cntct_last_name,"
               "    cohead_shipto_cntct_suffix=:shipto_cntct_suffix,"
               "    cohead_shipto_cntct_phone=:shipto_cntct_phone,"
               "    cohead_shipto_cntct_title=:shipto_cntct_title,"
               "    cohead_shipto_cntct_fax=:shipto_cntct_fax,"
               "    cohead_shipto_cntct_email=:shipto_cntct_email,"
               "    cohead_billto_cntct_id=:billto_cntct_id,"
               "    cohead_billto_cntct_honorific=:billto_cntct_honorific,"
               "    cohead_billto_cntct_first_name=:billto_cntct_first_name,"
               "    cohead_billto_cntct_middle=:billto_cntct_middle,"
               "    cohead_billto_cntct_last_name=:billto_cntct_last_name,"
               "    cohead_billto_cntct_suffix=:billto_cntct_suffix,"
               "    cohead_billto_cntct_phone=:billto_cntct_phone,"
               "    cohead_billto_cntct_title=:billto_cntct_title,"
               "    cohead_billto_cntct_fax=:billto_cntct_fax,"
               "    cohead_billto_cntct_email=:billto_cntct_email "
               "WHERE (cohead_id=:id);" );
  else if (_mode == cNew)
  {
    if (_metrics->boolean("AutoCreateProjectsForOrders"))
    {
      XSqlQuery prj;
      prj.prepare("INSERT INTO prj (prj_number, prj_name, prj_descrip, prj_status, prj_so, prj_wo, prj_po) "
                  " VALUES (:number, :number, :descrip, 'O', TRUE, TRUE, TRUE) "
                  "RETURNING prj_id");
      prj.bindValue(":number", _orderNumber->text());
      prj.bindValue(":descrip", tr("Auto Generated Project from Sales Order."));
      prj.exec();
      if (prj.first())
        _project->setId(prj.value("prj_id").toInt());
    }
    q.prepare("INSERT INTO cohead "
              "(cohead_id, cohead_number, cohead_cust_id,"
              "    cohead_custponumber, cohead_shipto_id,"
              "    cohead_billtoname, cohead_billtoaddress1,"
              "    cohead_billtoaddress2, cohead_billtoaddress3,"
              "    cohead_billtocity, cohead_billtostate, cohead_billtozipcode,"
              "    cohead_billtocountry,"
              "    cohead_shiptoname, cohead_shiptoaddress1,"
              "    cohead_shiptoaddress2, cohead_shiptoaddress3,"
              "    cohead_shiptocity, cohead_shiptostate, cohead_shiptozipcode,"
              "    cohead_shiptocountry,"
              "    cohead_orderdate, cohead_packdate,"
              "    cohead_salesrep_id, cohead_commission,"
              "    cohead_taxzone_id, cohead_terms_id, cohead_origin,"
              "    cohead_fob, cohead_shipvia, cohead_warehous_id,"
              "    cohead_freight, cohead_calcfreight,"
              "    cohead_misc, cohead_misc_accnt_id, cohead_misc_descrip,"
              "    cohead_holdtype,"
              "    cohead_ordercomments, cohead_shipcomments,"
              "    cohead_shipchrg_id, cohead_shipform_id,"
              "    cohead_prj_id, cohead_ophead_id,"
              "    cohead_curr_id,"
              "    cohead_shipcomplete,"
              "    cohead_shipto_cntct_id,"
              "    cohead_shipto_cntct_honorific,"
              "    cohead_shipto_cntct_first_name,"
              "    cohead_shipto_cntct_middle,"
              "    cohead_shipto_cntct_last_name,"
              "    cohead_shipto_cntct_suffix,"
              "    cohead_shipto_cntct_phone,"
              "    cohead_shipto_cntct_title,"
              "    cohead_shipto_cntct_fax,"
              "    cohead_shipto_cntct_email,"
              "    cohead_billto_cntct_id,"
              "    cohead_billto_cntct_honorific,"
              "    cohead_billto_cntct_first_name,"
              "    cohead_billto_cntct_middle,"
              "    cohead_billto_cntct_last_name,"
              "    cohead_billto_cntct_suffix,"
              "    cohead_billto_cntct_phone,"
              "    cohead_billto_cntct_title,"
              "    cohead_billto_cntct_fax,"
              "    cohead_billto_cntct_email)"
              "    VALUES (:id,:number, :cust_id,"
              "    :custponumber,:shipto_id,"
              "    :billtoname, :billtoaddress1,"
              "    :billtoaddress2, :billtoaddress3,"
              "    :billtocity, :billtostate, :billtozipcode,"
              "    :billtocountry,"
              "    :shiptoname, :shiptoaddress1,"
              "    :shiptoaddress2, :shiptoaddress3,"
              "    :shiptocity, :shiptostate, :shiptozipcode,"
              "    :shiptocountry,"
              "    :orderdate, :packdate,"
              "    :salesrep_id, :commission,"
              "    :taxzone_id, :terms_id, :origin,"
              "    :fob, :shipvia, :warehous_id,"
              "    :freight, :calcfreight,"
              "    :misc, :misc_accnt_id, :misc_descrip,"
              "    :holdtype,"
              "    :ordercomments, :shipcomments,"
              "    :shipchrg_id, :shipform_id,"
              "    :prj_id, :ophead_id,"
              "    :curr_id,"
              "    :cohead_shipcomplete,"
              "    :shipto_cntct_id,"
              "    :shipto_cntct_honorific,"
              "    :shipto_cntct_first_name,"
              "    :shipto_cntct_middle,"
              "    :shipto_cntct_last_name,"
              "    :shipto_cntct_suffix,"
              "    :shipto_cntct_phone,"
              "    :shipto_cntct_title,"
              "    :shipto_cntct_fax,"
              "    :shipto_cntct_email,"
              "    :billto_cntct_id,"
              "    :billto_cntct_honorific,"
              "    :billto_cntct_first_name,"
              "    :billto_cntct_middle,"
              "    :billto_cntct_last_name,"
              "    :billto_cntct_suffix,"
              "    :billto_cntct_phone,"
              "    :billto_cntct_title,"
              "    :billto_cntct_fax,"
              "    :billto_cntct_email) ");
  }
  else if ((_mode == cEditQuote) || ((_mode == cNewQuote) && _saved))
    q.prepare( "UPDATE quhead "
               "SET quhead_custponumber=:custponumber, quhead_shipto_id=:shipto_id,"
               "    quhead_billtoname=:billtoname, quhead_billtoaddress1=:billtoaddress1,"
               "    quhead_billtoaddress2=:billtoaddress2, quhead_billtoaddress3=:billtoaddress3,"
               "    quhead_billtocity=:billtocity, quhead_billtostate=:billtostate, quhead_billtozip=:billtozipcode,"
               "    quhead_billtocountry=:billtocountry,"
               "    quhead_shiptoname=:shiptoname, quhead_shiptoaddress1=:shiptoaddress1,"
               "    quhead_shiptoaddress2=:shiptoaddress2, quhead_shiptoaddress3=:shiptoaddress3,"
               "    quhead_shiptocity=:shiptocity, quhead_shiptostate=:shiptostate, quhead_shiptozipcode=:shiptozipcode,"
               "    quhead_shiptocountry=:shiptocountry,"
               "    quhead_quotedate=:orderdate, quhead_packdate=:packdate,"
               "    quhead_salesrep_id=:salesrep_id, quhead_commission=:commission,"
               "    quhead_taxzone_id=:taxzone_id, quhead_terms_id=:terms_id,"
               "    quhead_origin=:origin, quhead_shipvia=:shipvia, quhead_fob=:fob,"
               "    quhead_freight=:freight, quhead_calcfreight=:calcfreight,"
               "    quhead_misc=:misc, quhead_misc_accnt_id=:misc_accnt_id, quhead_misc_descrip=:misc_descrip,"
               "    quhead_ordercomments=:ordercomments, quhead_shipcomments=:shipcomments,"
               "    quhead_prj_id=:prj_id, quhead_ophead_id=:ophead_id, quhead_warehous_id=:warehous_id,"
               "    quhead_curr_id = :curr_id, quhead_expire=:expire,"
               "    quhead_shipto_cntct_id=:shipto_cntct_id,"
               "    quhead_shipto_cntct_honorific=:shipto_cntct_honorific,"
               "    quhead_shipto_cntct_first_name=:shipto_cntct_first_name,"
               "    quhead_shipto_cntct_middle=:shipto_cntct_middle,"
               "    quhead_shipto_cntct_last_name=:shipto_cntct_last_name,"
               "    quhead_shipto_cntct_suffix=:shipto_cntct_suffix,"
               "    quhead_shipto_cntct_phone=:shipto_cntct_phone,"
               "    quhead_shipto_cntct_title=:shipto_cntct_title,"
               "    quhead_shipto_cntct_fax=:shipto_cntct_fax,"
               "    quhead_shipto_cntct_email=:shipto_cntct_email,"
               "    quhead_billto_cntct_id=:billto_cntct_id,"
               "    quhead_billto_cntct_honorific=:billto_cntct_honorific,"
               "    quhead_billto_cntct_first_name=:billto_cntct_first_name,"
               "    quhead_billto_cntct_middle=:billto_cntct_middle,"
               "    quhead_billto_cntct_last_name=:billto_cntct_last_name,"
               "    quhead_billto_cntct_suffix=:billto_cntct_suffix,"
               "    quhead_billto_cntct_phone=:billto_cntct_phone,"
               "    quhead_billto_cntct_title=:billto_cntct_title,"
               "    quhead_billto_cntct_fax=:billto_cntct_fax,"
               "    quhead_billto_cntct_email=:billto_cntct_email "
               "WHERE (quhead_id=:id);" );
  else if (_mode == cNewQuote)
    q.prepare( "INSERT INTO quhead ("
               "    quhead_id, quhead_number, quhead_cust_id,"
               "    quhead_custponumber, quhead_shipto_id,"
               "    quhead_billtoname, quhead_billtoaddress1,"
               "    quhead_billtoaddress2, quhead_billtoaddress3,"
               "    quhead_billtocity, quhead_billtostate, quhead_billtozip,"
               "    quhead_billtocountry,"
               "    quhead_shiptoname, quhead_shiptoaddress1,"
               "    quhead_shiptoaddress2, quhead_shiptoaddress3,"
               "    quhead_shiptocity, quhead_shiptostate, quhead_shiptozipcode,"
               "    quhead_shiptocountry,"
               "    quhead_quotedate, quhead_packdate,"
               "    quhead_salesrep_id, quhead_commission,"
               "    quhead_taxzone_id, quhead_terms_id,"
               "    quhead_origin, quhead_shipvia, quhead_fob,"
               "    quhead_freight, quhead_calcfreight,"
               "    quhead_misc, quhead_misc_accnt_id, quhead_misc_descrip,"
               "    quhead_ordercomments, quhead_shipcomments,"
               "    quhead_prj_id, quhead_ophead_id, quhead_warehous_id,"
               "    quhead_curr_id, quhead_expire,"
               "    quhead_shipto_cntct_id,"
               "    quhead_shipto_cntct_honorific,"
               "    quhead_shipto_cntct_first_name,"
               "    quhead_shipto_cntct_middle,"
               "    quhead_shipto_cntct_last_name,"
               "    quhead_shipto_cntct_suffix,"
               "    quhead_shipto_cntct_phone,"
               "    quhead_shipto_cntct_title,"
               "    quhead_shipto_cntct_fax,"
               "    quhead_shipto_cntct_email,"
               "    quhead_billto_cntct_id,"
               "    quhead_billto_cntct_honorific,"
               "    quhead_billto_cntct_first_name,"
               "    quhead_billto_cntct_middle,"
               "    quhead_billto_cntct_last_name,"
               "    quhead_billto_cntct_suffix,"
               "    quhead_billto_cntct_phone,"
               "    quhead_billto_cntct_title,"
               "    quhead_billto_cntct_fax,"
               "    quhead_billto_cntct_email,"
               "    quhead_status)"
               "    VALUES ("
               "    :id, :number, :cust_id,"
               "    :custponumber, :shipto_id,"
               "    :billtoname, :billtoaddress1,"
               "    :billtoaddress2, :billtoaddress3,"
               "    :billtocity, :billtostate, :billtozipcode,"
               "    :billtocountry,"
               "    :shiptoname, :shiptoaddress1,"
               "    :shiptoaddress2, :shiptoaddress3,"
               "    :shiptocity, :shiptostate, :shiptozipcode,"
               "    :shiptocountry,"
               "    :orderdate, :packdate,"
               "    :salesrep_id, :commission,"
               "    :taxzone_id, :terms_id,"
               "    :origin, :shipvia, :fob,"
               "    :freight, :calcfreight,"
               "    :misc, :misc_accnt_id, :misc_descrip,"
               "    :ordercomments, :shipcomments,"
               "    :prj_id, :ophead_id, :warehous_id,"
               "    :curr_id, :expire,"
               "    :shipto_cntct_id,"
               "    :shipto_cntct_honorific,"
               "    :shipto_cntct_first_name,"
               "    :shipto_cntct_middle,"
               "    :shipto_cntct_last_name,"
               "    :shipto_cntct_suffix,"
               "    :shipto_cntct_phone,"
               "    :shipto_cntct_title,"
               "    :shipto_cntct_fax,"
               "    :shipto_cntct_email,"
               "    :billto_cntct_id,"
               "    :billto_cntct_honorific,"
               "    :billto_cntct_first_name,"
               "    :billto_cntct_middle,"
               "    :billto_cntct_last_name,"
               "    :billto_cntct_suffix,"
               "    :billto_cntct_phone,"
               "    :billto_cntct_title,"
               "    :billto_cntct_fax,"
               "    :billto_cntct_email,"
               "    :quhead_status) ");
  q.bindValue(":id", _soheadid );
  q.bindValue(":number", _orderNumber->text());
  q.bindValue(":orderdate", _orderDate->date());

  if (_packDate->isValid())
    q.bindValue(":packdate", _packDate->date());
  else
    q.bindValue(":packdate", _orderDate->date());

  q.bindValue(":cust_id", _cust->id());
  if (_warehouse->id() != -1)
    q.bindValue(":warehous_id", _warehouse->id());
  q.bindValue(":custponumber", _custPONumber->text().trimmed());
  if (_shipTo->id() > 0)
    q.bindValue(":shipto_id", _shipTo->id());
  q.bindValue(":billtoname",                _billToName->text());
  q.bindValue(":billtoaddress1",        _billToAddr->line1());
  q.bindValue(":billtoaddress2",        _billToAddr->line2());
  q.bindValue(":billtoaddress3",        _billToAddr->line3());
  q.bindValue(":billtocity",                _billToAddr->city());
  q.bindValue(":billtostate",                _billToAddr->state());
  q.bindValue(":billtozipcode",                _billToAddr->postalCode());
  q.bindValue(":billtocountry",                _billToAddr->country());
  q.bindValue(":shiptoname",                _shipToName->text());
  q.bindValue(":shiptoaddress1",        _shipToAddr->line1());
  q.bindValue(":shiptoaddress2",        _shipToAddr->line2());
  q.bindValue(":shiptoaddress3",        _shipToAddr->line3());
  q.bindValue(":shiptocity",                _shipToAddr->city());
  q.bindValue(":shiptostate",                _shipToAddr->state());
  q.bindValue(":shiptozipcode",                _shipToAddr->postalCode());
  q.bindValue(":shiptocountry",                _shipToAddr->country());
  q.bindValue(":ordercomments", _orderComments->toPlainText());
  q.bindValue(":shipcomments", _shippingComments->toPlainText());
  q.bindValue(":fob", _fob->text());
  q.bindValue(":shipvia", _shipVia->currentText());

  if (_shipToCntct->isValid())
    q.bindValue(":shipto_cntct_id", _shipToCntct->id());

  q.bindValue(":shipto_cntct_honorific", _shipToCntct->honorific());
  q.bindValue(":shipto_cntct_first_name", _shipToCntct->first());
  q.bindValue(":shipto_cntct_middle", _shipToCntct->middle());
  q.bindValue(":shipto_cntct_last_name", _shipToCntct->last());
  q.bindValue(":shipto_cntct_suffix", _shipToCntct->suffix());
  q.bindValue(":shipto_cntct_phone", _shipToCntct->phone());
  q.bindValue(":shipto_cntct_title", _shipToCntct->title());
  q.bindValue(":shipto_cntct_fax", _shipToCntct->fax());
  q.bindValue(":shipto_cntct_email", _shipToCntct->emailAddress());

  if (_billToCntct->isValid())
    q.bindValue(":billto_cntct_id", _billToCntct->id());

  q.bindValue(":billto_cntct_honorific", _billToCntct->honorific());
  q.bindValue(":billto_cntct_first_name", _billToCntct->first());
  q.bindValue(":billto_cntct_middle", _billToCntct->middle());
  q.bindValue(":billto_cntct_last_name", _billToCntct->last());
  q.bindValue(":billto_cntct_suffix", _billToCntct->suffix());
  q.bindValue(":billto_cntct_phone", _billToCntct->phone());
  q.bindValue(":billto_cntct_title", _billToCntct->title());
  q.bindValue(":billto_cntct_fax", _billToCntct->fax());
  q.bindValue(":billto_cntct_email", _billToCntct->emailAddress());

  if (_salesRep->id() != -1)
    q.bindValue(":salesrep_id", _salesRep->id());
  if (_taxZone->isValid())
    q.bindValue(":taxzone_id", _taxZone->id());
  if (_terms->id() != -1)
    q.bindValue(":terms_id", _terms->id());
  q.bindValue(":shipchrg_id", _shippingCharges->id());
  q.bindValue(":shipform_id", _shippingForm->id());
  q.bindValue(":freight", _freight->localValue());
  q.bindValue(":calcfreight", _calcfreight);
  q.bindValue(":commission", (_commission->toDouble() / 100.0));
  q.bindValue(":misc", _miscCharge->localValue());
  if (_miscChargeAccount->id() != -1)
    q.bindValue(":misc_accnt_id", _miscChargeAccount->id());
  q.bindValue(":misc_descrip", _miscChargeDescription->text().trimmed());
  q.bindValue(":curr_id", _orderCurrency->id());
  q.bindValue(":cohead_shipcomplete", QVariant(_shipComplete->isChecked()));
  if (_project->isValid())
    q.bindValue(":prj_id", _project->id());
  if (_opportunity->isValid())
    q.bindValue(":ophead_id", _opportunity->id());
  if (_expire->isValid())
    q.bindValue(":expire", _expire->date());

  if (_holdType->currentIndex() == 0)
    q.bindValue(":holdtype", "N");
  else if (_holdType->currentIndex() == 1)
    q.bindValue(":holdtype", "C");
  else if (_holdType->currentIndex() == 2)
    q.bindValue(":holdtype", "S");
  else if (_holdType->currentIndex() == 3)
    q.bindValue(":holdtype", "P");
  else if (_holdType->currentIndex() == 4)
    q.bindValue(":holdtype", "R");

  q.bindValue(":origin", _origin->code());
  q.bindValue(":quhead_status", "O");

  q.exec();
  if (q.lastError().type() != QSqlError::NoError)
  {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return false;
  }

  // if this is a new so record and we haven't saved already
  // then we need to lock this record.
  if ((cNew == _mode) && (!_saved) && !_locked)
  {
    // should I bother to check because no one should have this but us?
    q.prepare("SELECT tryLock(oid::integer, :head_id) AS locked "
              "FROM pg_class "
              "WHERE relname=:table;");
    q.bindValue(":head_id", _soheadid);
    if (ISORDER(_mode))
      q.bindValue(":table", "cohead");
    else
      q.bindValue(":table", "quhead");
    q.exec();
    if (q.first())
      _locked = q.value("locked").toBool();
    else if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return false;
    }
  }

  _saved = true;

  if (!partial)
  {
    if ((cNew == _mode) && _metrics->boolean("AutoAllocateCreditMemos"))
    {
      sAllocateCreditMemos();
    }

    if ( (_mode == cNew) || (_mode == cEdit) )
    {
      omfgThis->sSalesOrdersUpdated(_soheadid);
      omfgThis->sProjectsUpdated(_soheadid);
    }
    else if ( (_mode == cNewQuote) || (_mode == cEditQuote) )
      omfgThis->sQuotesUpdated(_soheadid);
  }
  else
  {
    populateCMInfo();
    populateCCInfo();
  }

  emit saved(_soheadid);

  return TRUE;
}

void salesOrder::sPopulateMenu(QMenu *pMenu)
{
  if ((_mode == cNew) || (_mode == cEdit))
  {
    QAction *menuItem;
    bool  didsomething = false;
    if (_numSelected == 1)
    {
      didsomething = true;
      if (_lineMode == cClosed)
        pMenu->addAction(tr("Open Line..."), this, SLOT(sAction()));
      else if (_lineMode == cActiveOpen)
      {
        pMenu->addAction(tr("Edit Line..."), this, SLOT(sEdit()));
        menuItem = pMenu->addAction(tr("Firm Line..."), this, SLOT(sFirm()));
        menuItem->setEnabled(_privileges->check("FirmSalesOrder"));
        pMenu->addAction(tr("Close Line..."), this, SLOT(sAction()));
      }
      else if (_lineMode == cInactiveOpen)
      {
        pMenu->addAction(tr("Edit Line..."), this, SLOT(sEdit()));
        menuItem = pMenu->addAction(tr("Firm Line..."), this, SLOT(sFirm()));
        menuItem->setEnabled(_privileges->check("FirmSalesOrder"));
        pMenu->addAction(tr("Close Line..."), this, SLOT(sAction()));
        pMenu->addAction(tr("Delete Line..."), this, SLOT(sDelete()));
      }
      else
      {
        menuItem = pMenu->addAction(tr("Soften Line..."), this, SLOT(sSoften()));
        menuItem->setEnabled(_privileges->check("FirmSalesOrder"));
      }
    }

    if (_metrics->boolean("EnableSOReservations"))
    {
      if (didsomething)
        pMenu->addSeparator();

      menuItem = pMenu->addAction(tr("Show Reservations..."), this, SLOT(sShowReservations()));

      pMenu->addSeparator();

      menuItem = pMenu->addAction(tr("Unreserve Stock"), this, SLOT(sUnreserveStock()));
      menuItem->setEnabled(_privileges->check("MaintainReservations"));
      menuItem = pMenu->addAction(tr("Reserve Stock..."), this, SLOT(sReserveStock()));
      menuItem->setEnabled(_privileges->check("MaintainReservations"));
      menuItem = pMenu->addAction(tr("Reserve Line Balance"), this, SLOT(sReserveLineBalance()));
      menuItem->setEnabled(_privileges->check("MaintainReservations"));

      didsomething = true;
    }

    if (_metrics->boolean("EnableSOShipping"))
    {
      if (didsomething)
        pMenu->addSeparator();

      menuItem = pMenu->addAction(tr("Return Stock"), this, SLOT(sReturnStock()));
      menuItem->setEnabled(_privileges->check("IssueStockToShipping"));
      menuItem = pMenu->addAction(tr("Issue Stock..."), this, SLOT(sIssueStock()));
      menuItem->setEnabled(_privileges->check("IssueStockToShipping"));
      menuItem = pMenu->addAction(tr("Issue Line Balance"), this, SLOT(sIssueLineBalance()));
      menuItem->setEnabled(_privileges->check("IssueStockToShipping"));

      didsomething = true;
    }
    XSqlQuery createOrder;
    createOrder.prepare( "SELECT coitem_order_type "
                         "FROM coitem "
                         "WHERE (coitem_id=:coitem_id);");
    QList<XTreeWidgetItem *> selected = _soitem->selectedItems();
    for (int i = 0; i < selected.size(); i++)
    {
      createOrder.bindValue(":coitem_id", ((XTreeWidgetItem *)(selected[i]))->id());
      createOrder.exec();
      if (createOrder.first())
      {
        if (didsomething)
          pMenu->addSeparator();
        if (createOrder.value("coitem_order_type").toString() == "P")
        {
          XSqlQuery checkPO;
          checkPO.prepare( "SELECT pohead_id "
                           "FROM pohead JOIN poitem ON (pohead_id=poitem_pohead_id) "
                           "     RIGHT OUTER JOIN coitem ON (poitem_id=coitem_order_id) "
                           "WHERE (coitem_id=:coitem_id);" );
          checkPO.bindValue(":coitem_id", ((XTreeWidgetItem *)(selected[i]))->id());
          checkPO.exec();
          if (checkPO.first())
          {
            menuItem = pMenu->addAction(tr("View Purchase Order..."), this, SLOT(sViewPO()));
            menuItem->setEnabled(_privileges->check("ViewPurchaseOrders"));

            menuItem = pMenu->addAction(tr("Edit Purchase Order..."), this, SLOT(sMaintainPO()));
            menuItem->setEnabled(_privileges->check("MaintainPurchaseOrders"));
          }
          else if (checkPO.lastError().type() != QSqlError::NoError)
          {
            systemError(this, checkPO.lastError().databaseText(), __FILE__, __LINE__);
            return;
          }
        }
        else if (createOrder.value("coitem_order_type").toString() == "R")
        {
          XSqlQuery checkPR;
          checkPR.prepare( "SELECT pr_id "
                           "FROM pr JOIN coitem ON (pr_id=coitem_order_id) "
                           "WHERE (coitem_id=:coitem_id);" );
          checkPR.bindValue(":coitem_id", ((XTreeWidgetItem *)(selected[i]))->id());
          checkPR.exec();
          if (checkPR.first())
          {
            menuItem = pMenu->addAction(tr("Release P/R..."), this, SLOT(sReleasePR()));
            menuItem->setEnabled(_privileges->check("MaintainPurchaseOrders"));

            menuItem = pMenu->addAction(tr("View Purchase Request..."), this, SLOT(sViewPR()));
            menuItem->setEnabled(_privileges->check("ViewPurchaseRequests"));
          }
          else if (checkPR.lastError().type() != QSqlError::NoError)
          {
            systemError(this, checkPR.lastError().databaseText(), __FILE__, __LINE__);
            return;
          }
        }
        else if (createOrder.value("coitem_order_type").toString() == "W")
        {
          XSqlQuery checkWO;
          checkWO.prepare( "SELECT wo_id "
                           "FROM wo JOIN coitem ON (wo_id=coitem_order_id)"
                           "WHERE (coitem_id=:coitem_id);" );
          checkWO.bindValue(":coitem_id", ((XTreeWidgetItem *)(selected[i]))->id());
          checkWO.exec();
          if (checkWO.first())
          {
            menuItem = pMenu->addAction(tr("View Work Order..."), this, SLOT(sViewWO()));
            menuItem->setEnabled(_privileges->check("ViewWorkOrders"));

            menuItem = pMenu->addAction(tr("Edit Work Order..."), this, SLOT(sMaintainWO()));
            menuItem->setEnabled(_privileges->check("MaintainWorkOrders"));
          }
          else if (checkWO.lastError().type() != QSqlError::NoError)
          {
            systemError(this, checkWO.lastError().databaseText(), __FILE__, __LINE__);
            return;
          }
        }
      }
      else if (createOrder.lastError().type() != QSqlError::NoError)
      {
            systemError(this, createOrder.lastError().databaseText(), __FILE__, __LINE__);
        return;
      }
      didsomething = true;
    }
  }
}

void salesOrder::populateOrderNumber()
{
  if (_mode == cNew)
  {
    if ( (_metrics->value("CONumberGeneration") == "A") ||
         (_metrics->value("CONumberGeneration") == "O")   )
    {
      q.exec("SELECT fetchSoNumber() AS sonumber;");
      if (q.first())
      {
        _orderNumber->setText(q.value("sonumber").toString());
        _orderNumberGen = q.value("sonumber").toInt();
        _userEnteredOrderNumber = FALSE;

        if (_metrics->value("CONumberGeneration") == "A")
          _orderNumber->setEnabled(FALSE);
      }
      else if (q.lastError().type() != QSqlError::NoError)
      {
            systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
        return;
      }
    }
    _userEnteredOrderNumber = FALSE;
  }
  else if (_mode == cNewQuote)
  {
    if ( (_metrics->value("QUNumberGeneration") == "A") ||
         (_metrics->value("QUNumberGeneration") == "O") ||
         (_metrics->value("QUNumberGeneration") == "S") )
    {
      if (_metrics->value("QUNumberGeneration") == "S")
        q.prepare("SELECT fetchSoNumber() AS qunumber;");
      else
        q.prepare("SELECT fetchQuNumber() AS qunumber;");

      q.exec();
      if (q.first())
      {
        _orderNumber->setText(q.value("qunumber").toString());
        _orderNumberGen = q.value("qunumber").toInt();
        _userEnteredOrderNumber = FALSE;

        if ( (_metrics->value("QUNumberGeneration") == "A") ||
             (_metrics->value("QUNumberGeneration") == "S") )
          _orderNumber->setEnabled(FALSE);
      }
      else if (q.lastError().type() != QSqlError::NoError)
      {
            systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
        return;
      }
      _userEnteredOrderNumber = FALSE;
    }
  }
}

void salesOrder::sSetUserEnteredOrderNumber()
{
  _userEnteredOrderNumber = TRUE;
}

void salesOrder::sHandleOrderNumber()
{
  if (_ignoreSignals || !isActiveWindow())
    return;

  if (_orderNumber->text().length() == 0)
  {
    if (_mode == cNew)
    {
      if ( (_metrics->value("CONumberGeneration") == "A") ||
           (_metrics->value("CONumberGeneration") == "O") )
        populateOrderNumber();
      else
      {
        QMessageBox::warning( this, tr("Enter S/O #"),
                              tr( "You must enter a S/O # for this Sales Order before you may continue." ) );
        _orderNumber->setFocus();
        return;
      }
    }
    else if (_mode == cNewQuote)
    {
      if ( (_metrics->value("QUNumberGeneration") == "A") ||
           (_metrics->value("QUNumberGeneration") == "O") ||
           (_metrics->value("QUNumberGeneration") == "S") )
        populateOrderNumber();
      else
      {
        QMessageBox::warning( this, tr("Enter Quote #"),
                              tr( "You must enter a Quote # for this Quote before you may continue." ) );
        _orderNumber->setFocus();
        return;
      }
    }
  }
  else
  {
    XSqlQuery query;
    if ( (_mode == cNew) && (_userEnteredOrderNumber) )
    {
      query.prepare("SELECT deleteSO(:sohead_id, :sohead_number) AS result;");
      query.bindValue(":sohead_id", _soheadid);
      query.bindValue(":sohead_number", _orderNumber->text());
      query.exec();
      if (query.first())
      {
        int result = query.value("result").toInt();
        if (result < 0)
        {
          systemError(this, storedProcErrorLookup("deleteSO", result), __FILE__, __LINE__);
          return;
        }
      }
      else if (query.lastError().type() != QSqlError::NoError)
      {
          systemError(this, query.lastError().databaseText(), __FILE__, __LINE__);
        return;
      }

      query.prepare( "SELECT cohead_id "
                     "FROM cohead "
                     "WHERE (cohead_number=:cohead_number);" );
      query.bindValue(":cohead_number", _orderNumber->text());
      query.exec();
      if (query.first())
      {
        _mode      = cEdit;
        _soheadid  = query.value("cohead_id").toInt();
        populate();
        _orderNumber->setEnabled(FALSE);
        _cust->setReadOnly(TRUE);
        populateCMInfo();
        populateCCInfo();
        sFillCcardList();
      }
      else
      {
        QString orderNumber = _orderNumber->text();
        if (_metrics->value("CONumberGeneration") == "O")
        {
          query.prepare( "SELECT releaseSoNumber(:orderNumber);" );
          query.bindValue(":orderNumber", _orderNumberGen);
          query.exec();
          _orderNumber->setText(orderNumber);
          _userEnteredOrderNumber = FALSE;
          _orderNumber->setEnabled(FALSE);
        }
        else
        {
          _orderNumber->setText(orderNumber);
          _orderNumber->setEnabled(FALSE);
        }
      }
    }
    else if ((_mode == cNewQuote) && (_userEnteredOrderNumber))
    {
      query.prepare("SELECT deleteQuote(:quhead_id, :quhead_number) AS result;");
      query.bindValue(":quhead_id", _soheadid);
      query.bindValue(":quhead_number", _orderNumberGen);
      query.exec();
      if (query.first())
      {
        int result = query.value("result").toInt();
        if (result < 0)
        {
          systemError(this, storedProcErrorLookup("deleteQuote", result), __FILE__, __LINE__);
          return;
        }
      }
      else if (query.lastError().type() != QSqlError::NoError)
      {
          systemError(this, query.lastError().databaseText(), __FILE__, __LINE__);
        return;
      }

      query.prepare( "SELECT quhead_id "
                     "FROM quhead "
                     "WHERE (quhead_number=:quhead_number);" );
      query.bindValue(":quhead_number", _orderNumber->text());
      query.exec();
      if (query.first())
      {
        QMessageBox::warning( this, tr("Quote Order Number Already exists."),
                              tr( "<p>The Quote Order Number you have entered "
                                    "already exists. Please enter a new one." ) );
          clear();
        _orderNumber->setFocus();
        return;
      }
      else
      {
        QString orderNumber = _orderNumber->text();
        if ((_metrics->value("QUNumberGeneration") == "S") ||
            (_metrics->value("QUNumberGeneration") == "A"))
        {
          clear();
          if (_metrics->value("QUNumberGeneration") == "S")
            query.prepare( "SELECT releaseSoNumber(:orderNumber);" );
          else
            query.prepare( "SELECT releaseQUNumber(:orderNumber);" );
          query.bindValue(":orderNumber", _orderNumberGen);
          query.exec();
          _orderNumber->setText(orderNumber);
          _userEnteredOrderNumber = FALSE;
          _orderNumber->setEnabled(FALSE);
        }
        else
        {
          _orderNumber->setText(orderNumber);
          _mode = cNewQuote;
          _orderNumber->setEnabled(FALSE);
        }
      }
    }
  }
}

void salesOrder::sPopulateFOB(int pWarehousid)
{
  XSqlQuery fob;
  fob.prepare( "SELECT warehous_fob "
               "FROM warehous "
               "WHERE (warehous_id=:warehous_id);" );
  fob.bindValue(":warehous_id", pWarehousid);
  fob.exec();
  if (fob.first())
    _fob->setText(fob.value("warehous_fob"));
}

// Is the first SELECT here responsible for the bug where the Currency kept disappearing?
void salesOrder::sPopulateCustomerInfo(int pCustid)
{
  _holdType->setCurrentIndex(0);

  if (pCustid != -1)
  {
    QString sql("SELECT cust_name, addr_id, "
                "       cust_salesrep_id, cust_shipchrg_id, cust_shipform_id,"
                "       cust_commprcnt AS commission,"
                "       cust_creditstatus, cust_terms_id,"
                "       cust_taxzone_id, cust_cntct_id,"
                "       cust_ffshipto, cust_ffbillto, cust_usespos,"
                "       cust_blanketpos, cust_shipvia,"
                "       COALESCE(shipto_id, -1) AS shiptoid,"
                "       cust_preferred_warehous_id, "
                "       cust_curr_id, COALESCE(crmacct_id,-1) AS crmacct_id, "
                "       true AS iscustomer "
                "FROM custinfo "
                "  LEFT OUTER JOIN cntct  ON (cust_cntct_id=cntct_id) "
                "  LEFT OUTER JOIN addr   ON (cntct_addr_id=addr_id) "
                "  LEFT OUTER JOIN shipto ON ((shipto_cust_id=cust_id)"
                "                         AND (shipto_default)) "
                "LEFT OUTER JOIN crmacct ON (crmacct_cust_id = cust_id) "
                "WHERE (cust_id=<? value(\"cust_id\") ?>) "
                "<? if exists(\"isQuote\") ?>"
                "UNION "
                "SELECT prospect_name AS cust_name, addr_id, "
                "       prospect_salesrep_id AS cust_salesrep_id, NULL AS cust_shipchrg_id,"
                "       NULL AS cust_shipform_id,"
                "       0.0 AS commission,"
                "       NULL AS cust_creditstatus, NULL AS cust_terms_id,"
                "       prospect_taxzone_id AS cust_taxzone_id, prospect_cntct_id AS cust_cntct_id, "
                "       TRUE AS cust_ffshipto, NULL AS cust_ffbillto, "
                "       NULL AS cust_usespos, NULL AS cust_blanketpos,"
                "       NULL AS cust_shipvia,"
                "       -1 AS shiptoid,"
                "       prospect_warehous_id AS cust_preferred_warehous_id, "
                "       NULL AS cust_curr_id, COALESCE(crmacct_id,-1) AS crmacct_id, "
                "       false AS iscustomer "
                "FROM prospect "
                "  LEFT OUTER JOIN cntct  ON (prospect_cntct_id=cntct_id) "
                "  LEFT OUTER JOIN addr   ON (cntct_addr_id=addr_id) "
                "  LEFT OUTER JOIN crmacct ON (crmacct_prospect_id = prospect_id) "
                "WHERE (prospect_id=<? value(\"cust_id\") ?>) "
                "<? endif ?>"
                ";" );

    MetaSQLQuery  mql(sql);
    ParameterList params;
    params.append("cust_id", pCustid);
    if (ISQUOTE(_mode))
      params.append("isQuote");
    XSqlQuery cust = mql.toQuery(params);
    if (cust.first())
    {
      if (_mode == cNew)
      {
        if ( (cust.value("cust_creditstatus").toString() == "H") &&
             (!_privileges->check("CreateSOForHoldCustomer")) )
        {
          QMessageBox::warning( this, tr("Selected Customer on Credit Hold"),
                                tr( "<p>The selected Customer has been placed "
                                      "on a Credit Hold and you do not have "
                                      "privilege to create Sales Orders for "
                                      "Customers on Credit Hold.  The selected "
                                      "Customer must be taken off of Credit Hold "
                                      "before you may create a new Sales Order "
                                      "for the Customer." ) );
          _cust->setId(-1);
          _billToAddr->setId(-1);
          _billToName->clear();
          _shipTo->setCustid(-1);
          _cust->setFocus();
          return;
        }

        if ( (cust.value("cust_creditstatus").toString() == "W") &&
             (!_privileges->check("CreateSOForWarnCustomer")) )
        {
          QMessageBox::warning( this, tr("Selected Customer on Credit Warning"),
                                tr( "<p>The selected Customer has been placed on "
                                      "a Credit Warning and you do not have "
                                      "privilege to create Sales Orders for "
                                      "Customers on Credit Warning.  The "
                                      "selected Customer must be taken off of "
                                      "Credit Warning before you may create a "
                                      "new Sales Order for the Customer." ) );
          _cust->setId(-1);
          _billToAddr->setId(-1);
          _billToName->clear();
          _shipTo->setCustid(-1);
          _cust->setFocus();
          return;
        }

        if ( (cust.value("cust_creditstatus").toString() == "H") || (cust.value("cust_creditstatus").toString() == "W") )
          _holdType->setCurrentIndex(1);
      }

      _billToName->setText(cust.value("cust_name").toString());
      _billToAddr->setId(cust.value("addr_id").toInt());
      sFillCcardList();
      _usesPos     = cust.value("cust_usespos").toBool();
      _blanketPos  = cust.value("cust_blanketpos").toBool();
      _salesRep->setId(cust.value("cust_salesrep_id").toInt());
      _shippingCharges->setId(cust.value("cust_shipchrg_id").toInt());
      _shippingForm->setId(cust.value("cust_shipform_id").toInt());
      _commission->setDouble(cust.value("commission").toDouble() * 100);
      _terms->setId(cust.value("cust_terms_id").toInt());
      _custtaxzoneid = cust.value("cust_taxzone_id").toInt();

      _billToCntct->setId(cust.value("cust_cntct_id").toInt());
      _billToCntct->setSearchAcct(cust.value("crmacct_id").toInt());
      _shipToCntct->setSearchAcct(cust.value("crmacct_id").toInt());
      _crmacctid=cust.value("crmacct_id").toInt();

      if (ISNEW(_mode))
        _taxZone->setId(cust.value("cust_taxzone_id").toInt());
      _shipVia->setText(cust.value("cust_shipvia"));

      _orderCurrency->setId(cust.value("cust_curr_id").toInt());

      if (cust.value("cust_preferred_warehous_id").toInt() > 0)
        _warehouse->setId(cust.value("cust_preferred_warehous_id").toInt());

      setFreeFormShipto(cust.value("cust_ffshipto").toBool());
      _shipTo->setCustid(pCustid);

      if (ISNEW(_mode) && cust.value("shiptoid").toInt() != -1)
        populateShipto(cust.value("shiptoid").toInt());
      else
      {
        _ignoreSignals = true;
        _shipTo->setId(cust.value("shiptoid").toInt());
        _shipToName->clear();
        _shipToAddr->clear();
        _shipToCntct->clear();
        _ignoreSignals = false;
      }

      if ((_mode == cNew) || (_mode == cNewQuote ) || (_mode == cEdit) || (_mode == cEditQuote ))
      {
        bool ffBillTo = cust.value("cust_ffbillto").toBool();
        _billToName->setEnabled(ffBillTo);
        _billToAddr->setEnabled(ffBillTo);
        _billToCntct->setEnabled(ffBillTo);

        if (!cust.value("iscustomer").toBool())
          _shipTo->setEnabled(false);
      }
    }
    else if (cust.lastError().type() != QSqlError::NoError)
    {
      systemError(this, cust.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }
  else
  {
    _salesRep->setCurrentIndex(-1);
    _commission->clear();
    _terms->setCurrentIndex(-1);
    _taxZone->setCurrentIndex(-1);
    _taxzoneidCache  = -1;
    _custtaxzoneid   = -1;
    _shipTo->setCustid(-1);
    _shipToName->clear();
    _shipToAddr->clear();
    _shipToCntct->clear();
    _billToCntct->clear();
  }
}

void salesOrder::sParseShipToNumber()
{
  if (_ignoreSignals)
    return;

  populateShipto(_shipTo->id());
  if (_soitem->topLevelItemCount())
    sRecalculatePrice();
}

void salesOrder::populateShipto(int pShiptoid)
{
  if (pShiptoid != -1)
  {
    XSqlQuery shipto;
    shipto.prepare( "SELECT shipto_num, shipto_name, shipto_addr_id, "
                    "       cntct_phone, shipto_cntct_id,"
                    "       shipto_shipvia, shipto_shipcomments,"
                    "       shipto_shipchrg_id, shipto_shipform_id,"
                    "       COALESCE(shipto_taxzone_id, -1) AS shipto_taxzone_id,"
                    "       shipto_salesrep_id, shipto_commission AS commission "
                    "FROM shiptoinfo LEFT OUTER JOIN "
                    "     cntct ON (shipto_cntct_id = cntct_id) "
                    "WHERE (shipto_id=:shipto_id);" );
    shipto.bindValue(":shipto_id", pShiptoid);
    shipto.exec();
    if (shipto.first())
    {
      //  Populate the dlg with the shipto information
      _ignoreSignals=true;
      if (_shipTo->id() != pShiptoid)
        _shipTo->setId(pShiptoid);
      _shipToName->setText(shipto.value("shipto_name").toString());
      _shipToAddr->setId(shipto.value("shipto_addr_id").toInt());
      _shipToCntct->setId(shipto.value("shipto_cntct_id").toInt());
      _shippingCharges->setId(shipto.value("shipto_shipchrg_id").toInt());
      _shippingForm->setId(shipto.value("shipto_shipform_id").toInt());
      _salesRep->setId(shipto.value("shipto_salesrep_id").toInt());
      _commission->setDouble(shipto.value("commission").toDouble() * 100);
      _shipVia->setText(shipto.value("shipto_shipvia"));
      _shippingComments->setText(shipto.value("shipto_shipcomments").toString());
      if ( (ISNEW(_mode)) && (shipto.value("shipto_taxzone_id").toInt() > 0) )
        _taxZone->setId(shipto.value("shipto_taxzone_id").toInt());
      _ignoreSignals=false;
    }
    else if (shipto.lastError().type() != QSqlError::NoError)
    {
      systemError(this, shipto.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }
  else
  {
    _shipTo->setId(-1);
    _shipToName->clear();
    _shipToAddr->clear();
    _shipToCntct->clear();
    _shippingComments->clear();
  }

  if (_saved)
    save(true);

  sFillItemList();
}

void salesOrder::sConvertShipTo()
{
  if (!_ignoreSignals)
  {
    //  Convert the captive shipto to a free-form shipto
    _shipTo->blockSignals(true);
    _shipTo->setId(-1);
    _shipTo->setCustid(_cust->id());
    _shipTo->blockSignals(false);
  }
}

void salesOrder::sNew()
{
  if ( !_saved && ((_mode == cNew) || (_mode == cNewQuote)) )
  {
    if (!save(true))
      return;
    // TODO - why populate?
    //else
    //  populate();
  }

  ParameterList params;
  params.append("sohead_id", _soheadid);
  params.append("cust_id", _cust->id());
  params.append("shipto_id", _shipTo->id());
  params.append("orderNumber", _orderNumber->text());
  params.append("curr_id", _orderCurrency->id());
  params.append("orderDate", _orderDate->date());
  params.append("taxzone_id", _taxZone->id());
  if (_warehouse->id() != -1)
    params.append("warehous_id", _warehouse->id());
  if (_shipDate->isValid())
    params.append("shipDate", _shipDate->date());

  if ((_mode == cNew) || (_mode == cEdit))
    params.append("mode", "new");
  else if ((_mode == cNewQuote) || (_mode == cEditQuote))
    params.append("mode", "newQuote");

  salesOrderItem newdlg(this);
  newdlg.set(params);

  newdlg.exec();
    sFillItemList();
}

void salesOrder::sCopyToShipto()
{
  _shipTo->setId(-1);
  _shipToName->setText(_billToName->text());
  _shipToAddr->setId(_billToAddr->id());
  if (_billToAddr->id() <= 0)
  {
    _shipToAddr->setLine1(_billToAddr->line1());
    _shipToAddr->setLine2(_billToAddr->line2());
    _shipToAddr->setLine3(_billToAddr->line3());
    _shipToAddr->setCity(_billToAddr->city());
    _shipToAddr->setState(_billToAddr->state());
    _shipToAddr->setPostalCode(_billToAddr->postalCode());
    _shipToAddr->setCountry(_billToAddr->country());
  }

  _shipToCntct->setId(_billToCntct->id());
  _taxZone->setId(_custtaxzoneid);
}

void salesOrder::sEdit()
{
  ParameterList params;
  params.append("soitem_id", _soitem->id());
  params.append("cust_id", _cust->id());
  params.append("shipto_id", _shipTo->id());
  params.append("orderNumber", _orderNumber->text());
  params.append("curr_id", _orderCurrency->id());
  params.append("orderDate", _orderDate->date());
  params.append("taxzone_id", _taxZone->id());

  if (_mode == cView)
    params.append("mode", "view");
  else if (_mode == cViewQuote)
    params.append("mode", "viewQuote");
  else if (((_mode == cNew) || (_mode == cEdit)) &&
           _soitem->currentItem()->rawValue("coitem_subnumber").toInt() != 0)
    params.append("mode", "view");
  else if ((_mode == cNew) || (_mode == cEdit))
    params.append("mode", "edit");
  else if ((_mode == cNewQuote) || (_mode == cEditQuote))
    params.append("mode", "editQuote");

  salesOrderItem newdlg(this);
  newdlg.set(params);
  newdlg.exec();

  if ( ( (_mode == cNew) || (_mode == cNewQuote) || (_mode == cEdit) || (_mode == cEditQuote) ) )
    sFillItemList();
}

void salesOrder::sHandleButtons()
{
  XTreeWidgetItem *selected = 0;
  _numSelected = 0;

  QList<XTreeWidgetItem *> selectedlist = _soitem->selectedItems();
  _numSelected = selectedlist.size();
  if (_numSelected > 0)
    selected = (XTreeWidgetItem *)(selectedlist[0]);

  if (selected)
  {
    _issueStock->setEnabled(_privileges->check("IssueStockToShipping"));
    _issueLineBalance->setEnabled(_privileges->check("IssueStockToShipping"));
    _reserveStock->setEnabled(_privileges->check("MaintainReservations"));
    _reserveLineBalance->setEnabled(_privileges->check("MaintainReservations"));

    if ( (_numSelected == 1) && (!selected->rawValue("coitem_firm").toBool()) )
    {
      _edit->setEnabled(TRUE);
      _delete->setEnabled(TRUE);
      int lineMode = selected->altId();

      if (ISQUOTE(_mode))
      {
        _action->setText(tr("Close"));
        _action->setEnabled(FALSE);
        _delete->setEnabled(TRUE);
      }
      else
      {
        if (lineMode == 1)
        {
          _lineMode = cClosed;

          _action->setText(tr("Open"));
          _action->setEnabled(TRUE);
          _delete->setEnabled(FALSE);
        }
        else if (lineMode == 2)
        {
          _lineMode = cActiveOpen;

          _action->setText(tr("Close"));
          _action->setEnabled(TRUE);
          _delete->setEnabled(FALSE);
        }
        else if (lineMode == 3)
        {
          _lineMode = cInactiveOpen;

          _action->setText(tr("Close"));
          _action->setEnabled(TRUE);
          _delete->setEnabled(TRUE);
        }
        else if (lineMode == 4)
        {
          _lineMode = cCanceled;

          _action->setEnabled(FALSE);
          _delete->setEnabled(FALSE);
        }
        else
        {
          _action->setEnabled(FALSE);
          _delete->setEnabled(FALSE);
        }

        if (1 == lineMode ||                                  // closed
            4 == lineMode ||                                  // cancelled
            selected->rawValue("item_type").toString() == "K" // kit item
            )
        {
          _issueStock->setEnabled(FALSE);
          _issueLineBalance->setEnabled(FALSE);
          _reserveStock->setEnabled(FALSE);
          _reserveLineBalance->setEnabled(FALSE);
          for (int i = 0; i < selected->childCount(); i++)
          {
            if (selected->child(i)->altId() == 1 ||
                selected->child(i)->altId() == 2 ||
                selected->child(i)->altId() == 4)
            {
              _delete->setEnabled(FALSE);
              break;
            }
          }
        }

        if (!selected->rawValue("coitem_subnumber").toInt() == 0)
        {
          _edit->setText(tr("View"));
          _delete->setEnabled(FALSE);
        }
        else if (cNew == _mode || cEdit == _mode || cNewQuote == _mode || cEditQuote == _mode)
        {
          _edit->setText(tr("&Edit"));
        }
      }
    }
    else
    {
      _lineMode = 0;
      _edit->setEnabled(FALSE);
      _action->setEnabled(FALSE);
      _delete->setEnabled(FALSE);
    }
  }
  else
  {
    _edit->setEnabled(FALSE);
    _action->setEnabled(FALSE);
    _delete->setEnabled(FALSE);
    _issueStock->setEnabled(FALSE);
    _issueLineBalance->setEnabled(FALSE);
    _reserveStock->setEnabled(FALSE);
    _reserveLineBalance->setEnabled(FALSE);
  }
}

void salesOrder::sFirm()
{
  if (_lineMode == cCanceled)
    return;

  if ( (_mode == cNew) || (_mode == cEdit) )
  {
    q.prepare( "UPDATE coitem "
               "SET coitem_firm=true "
               "WHERE (coitem_id=:coitem_id);" );
    q.bindValue(":coitem_id", _soitem->id());
    q.exec();
    sFillItemList();
  }
}

void salesOrder::sSoften()
{
  if (_lineMode == cCanceled)
    return;

  if ( (_mode == cNew) || (_mode == cEdit) )
  {
    q.prepare( "UPDATE coitem "
               "SET coitem_firm=false "
               "WHERE (coitem_id=:coitem_id);" );
    q.bindValue(":coitem_id", _soitem->id());
    q.exec();
    sFillItemList();
  }
}

void salesOrder::sAction()
{
  if (_lineMode == cCanceled)
    return;

  if ( (_mode == cNew) || (_mode == cEdit) )
  {
    if (_lineMode == cClosed)
      q.prepare( "UPDATE coitem "
                 "SET coitem_status='O' "
                 "WHERE (coitem_id=:coitem_id);" );
    else
    {
      q.prepare( "SELECT COALESCE(SUM(coship_qty), 0) - coitem_qtyshipped AS atshipping"
                 "  FROM coitem LEFT OUTER JOIN coship ON (coship_coitem_id=coitem_id)"
                 " WHERE (coitem_id=:coitem_id)"
                 " GROUP BY coitem_qtyshipped;");
      q.bindValue(":coitem_id", _soitem->id());
      q.exec();
      if (q.first() && q.value("atshipping").toDouble() > 0)
      {
        QMessageBox::information(this, tr("Cannot Close Item"),
                                 tr("The item cannot be Closed at this time as there is inventory at shipping.") );
        return;
      }
      if (_metrics->boolean("EnableSOReservations"))
        sUnreserveStock();
      q.prepare( "UPDATE coitem "
                 "SET coitem_status='C' "
                 "WHERE (coitem_id=:coitem_id);" );
    }
    q.bindValue(":coitem_id", _soitem->id());
    q.exec();
    sFillItemList();
  }
}

void salesOrder::sDelete()
{
  if ( (_mode == cEdit) || (_mode == cNew) )
  {
    if (QMessageBox::question(this, tr("Delete Selected Line Item?"),
                              tr("<p>Are you sure that you want to delete the "
                                   "selected Line Item?"),
                              QMessageBox::Yes,
                              QMessageBox::No | QMessageBox::Default) == QMessageBox::Yes)
    {
      if (_metrics->boolean("EnableSOReservations"))
        sUnreserveStock();

      q.prepare( "SELECT deleteSOItem(:soitem_id) AS result;");
      q.bindValue(":soitem_id", _soitem->id());
      q.exec();
      if (q.first())
      {
        int result = q.value("result").toInt();
        if (result == -20)
          QMessageBox::information(this, tr("Cannot Delete Purchase Order Item"),
                                    tr("<p>Purchase Order Item and its associated "
                                        "Purchase Order will remain open and must be "
                                        "deleted separately if desired."));
        else if (result == -10)
          QMessageBox::critical(this, tr("Cannot Delete Sales Order Item"),
                                    tr("<p>Purchase Order Item associated with this "
                                     "Sales Order Item is Closed or has Receipts. "
                                     "You may not delete the Sales Order line, "
                                     "but instead must cancel it."));
        else if (result < 0)
          systemError(this, storedProcErrorLookup("deleteSOItem", result),  __FILE__, __LINE__);
      }
      else if (q.lastError().type() != QSqlError::NoError)
          systemError(this, q.lastError().databaseText(),                   __FILE__, __LINE__);

          sFillItemList();

      if (_soitem->topLevelItemCount() == 0)
      {
        if (QMessageBox::question(this, tr("Cancel Sales Order?"),
                                  tr("<p>You have deleted all of the Line "
                                       "Items for this Sales Order. Would you "
                                       "like to cancel this Sales Order?"),
                                  QMessageBox::Yes,
                                  QMessageBox::No | QMessageBox::Default) == QMessageBox::Yes)
        {
          q.prepare( "SELECT deleteSO(:sohead_id, :sohead_number) AS result;");
          q.bindValue(":sohead_id", _soheadid);
          q.bindValue(":sohead_number", _orderNumber->text());
          q.exec();
          if (q.first())
          {
            int result = q.value("result").toInt();
            if (result < 0)
              systemError(this, storedProcErrorLookup("deleteSO", result),  __FILE__, __LINE__);
          }
          else if (q.lastError().type() != QSqlError::NoError)
              systemError(this, q.lastError().databaseText(),               __FILE__, __LINE__);

          omfgThis->sSalesOrdersUpdated(_soheadid);
          _captive = false;
          clear();
        }
      }
    }
  }
  else if ( (_mode == cNewQuote) || (_mode == cEditQuote) )
  {
    q.prepare( "DELETE FROM quitem "
               "WHERE (quitem_id=:quitem_id);" );
    q.bindValue(":quitem_id", _soitem->id());
    q.exec();
    sFillItemList();

    if (_soitem->topLevelItemCount() == 0)
    {
      if ( QMessageBox::question(this, tr("Cancel Quote?"),
                                 tr("<p>You have deleted all of the order "
                                      "lines for this Quote. Would you like to "
                                      "cancel this Quote?."),
                                 QMessageBox::Yes,
                                 QMessageBox::No | QMessageBox::Default) == QMessageBox::Yes)
      {
        q.prepare("SELECT deleteQuote(:quhead_id, :quhead_number) AS result;");
        q.bindValue(":quhead_id", _soheadid);
        q.bindValue(":quhead_number", _orderNumber->text());
        q.exec();
        if (q.first() && !q.value("result").toBool())
          systemError(this, tr("Could not delete Quote."),  __FILE__, __LINE__);
        else if (q.lastError().type() != QSqlError::NoError)
          systemError(this, q.lastError().databaseText(),   __FILE__, __LINE__);

        omfgThis->sQuotesUpdated(_soheadid);
        _captive = false;
        clear();
      }
    }
  }
}

void salesOrder::populate()
{
  if ( (_mode == cNew) || (_mode == cEdit) || (_mode == cView) )
  {
    XSqlQuery so;
    if (_mode == cEdit && !_locked)
    {
      // Lock the record
      so.prepare("SELECT tryLock(oid::integer, :sohead_id) AS locked "
                 "FROM pg_class "
                 "WHERE relname=:table;");
      so.bindValue(":sohead_id", _soheadid);
      if (ISORDER(_mode))
        so.bindValue(":table", "cohead");
      else
        so.bindValue(":table", "quhead");
      so.exec();
      if (so.first())
      {
        if (so.value("locked").toBool() != true)
        {
          QMessageBox::critical( this, tr("Record Currently Being Edited"),
                                 tr("<p>The record you are trying to edit is currently being edited "
                                      "by another user. Continue in View Mode.") );
          setViewMode();
        }
        else
          _locked = true;
      }
      else
      {
        QMessageBox::critical( this, tr("Cannot Lock Record for Editing"),
                               tr("<p>There was an unexpected error while trying to lock the record "
                                    "for editing. Please report this to your administator.") );
        setViewMode();
      }
    }
    so.prepare( "SELECT cohead.*,"
                "       COALESCE(cohead_shipto_id,-1) AS cohead_shipto_id,"
                "       cohead_commission AS commission,"
                "       COALESCE(cohead_taxzone_id,-1) AS taxzone_id,"
                "       COALESCE(cohead_warehous_id,-1) as cohead_warehous_id,"
                "       cust_name, cust_ffshipto, cust_blanketpos,"
                "       COALESCE(cohead_misc_accnt_id,-1) AS cohead_misc_accnt_id,"
                "       CASE WHEN(cohead_wasquote) THEN COALESCE(cohead_quote_number, cohead_number)"
                "            ELSE formatBoolYN(cohead_wasquote)"
                "       END AS fromQuote,"
                "       COALESCE(cohead_prj_id,-1) AS cohead_prj_id, "
                "       COALESCE(cohead_ophead_id,-1) AS cohead_ophead_id "
                "FROM custinfo, cohead "
                "WHERE ( (cohead_cust_id=cust_id)"
                " AND (cohead_id=:cohead_id) );" );
    so.bindValue(":cohead_id", _soheadid);
    so.exec();
    if (so.first())
    {
      _orderNumber->setText(so.value("cohead_number").toString());
      _orderNumber->setEnabled(FALSE);

      _orderDateCache = so.value("cohead_orderdate").toDate();
      _orderDate->setDate(_orderDateCache, true);
      _packDate->setDate(so.value("cohead_packdate").toDate());

      _fromQuote->setText(so.value("fromQuote").toString());

      _cust->setId(so.value("cohead_cust_id").toInt());

        setFreeFormShipto(so.value("cust_ffshipto").toBool());
      _blanketPos = so.value("cust_blanketpos").toBool();

      _warehouse->setId(so.value("cohead_warehous_id").toInt());
      _salesRep->setId(so.value("cohead_salesrep_id").toInt());
      _commission->setDouble(so.value("commission").toDouble() * 100);
      _taxzoneidCache = so.value("taxzone_id").toInt();
      _taxZone->setId(so.value("taxzone_id").toInt());
      _terms->setId(so.value("cohead_terms_id").toInt());
      _orderCurrency->setId(so.value("cohead_curr_id").toInt());
      _project->setId(so.value("cohead_prj_id").toInt());
      _opportunity->setId(so.value("cohead_ophead_id").toInt());

      _shipToCntct->setId(so.value("cohead_shipto_cntct_id").toInt());
      _shipToCntct->setHonorific(so.value("cohead_shipto_cntct_honorific").toString());
      _shipToCntct->setFirst(so.value("cohead_shipto_cntct_first_name").toString());
      _shipToCntct->setMiddle(so.value("cohead_shipto_cntct_middle").toString());
      _shipToCntct->setLast(so.value("cohead_shipto_cntct_last_name").toString());
      _shipToCntct->setSuffix(so.value("cohead_shipto_cntct_suffix").toString());
      _shipToCntct->setPhone(so.value("cohead_shipto_cntct_phone").toString());
      _shipToCntct->setTitle(so.value("cohead_shipto_cntct_title").toString());
      _shipToCntct->setFax(so.value("cohead_shipto_cntct_fax").toString());
      _shipToCntct->setEmailAddress(so.value("cohead_shipto_cntct_email").toString());

      _billToCntct->setId(so.value("cohead_billto_cntct_id").toInt());
      _billToCntct->setHonorific(so.value("cohead_billto_cntct_honorific").toString());
      _billToCntct->setFirst(so.value("cohead_billto_cntct_first_name").toString());
      _billToCntct->setMiddle(so.value("cohead_billto_cntct_middle").toString());
      _billToCntct->setLast(so.value("cohead_billto_cntct_last_name").toString());
      _billToCntct->setSuffix(so.value("cohead_billto_cntct_suffix").toString());
      _billToCntct->setPhone(so.value("cohead_billto_cntct_phone").toString());
      _billToCntct->setTitle(so.value("cohead_billto_cntct_title").toString());
      _billToCntct->setFax(so.value("cohead_billto_cntct_fax").toString());
      _billToCntct->setEmailAddress(so.value("cohead_billto_cntct_email").toString());

      _billToName->setText(so.value("cohead_billtoname").toString());
      if (_billToAddr->line1() !=so.value("cohead_billtoaddress1").toString() ||
          _billToAddr->line2() !=so.value("cohead_billtoaddress2").toString() ||
          _billToAddr->line3() !=so.value("cohead_billtoaddress3").toString() ||
          _billToAddr->city()  !=so.value("cohead_billtocity").toString() ||
          _billToAddr->state() !=so.value("cohead_billtostate").toString() ||
          _billToAddr->postalCode()!=so.value("cohead_billtozipcode").toString() ||
          _billToAddr->country()!=so.value("cohead_billtocountry").toString() )
      {
        _billToAddr->setId(-1);

        _billToAddr->setLine1(so.value("cohead_billtoaddress1").toString());
        _billToAddr->setLine2(so.value("cohead_billtoaddress2").toString());
        _billToAddr->setLine3(so.value("cohead_billtoaddress3").toString());
        _billToAddr->setCity(so.value("cohead_billtocity").toString());
        _billToAddr->setState(so.value("cohead_billtostate").toString());
        _billToAddr->setPostalCode(so.value("cohead_billtozipcode").toString());
        _billToAddr->setCountry(so.value("cohead_billtocountry").toString());
      }

      _ignoreSignals=true;
      _shipToName->setText(so.value("cohead_shiptoname").toString());
      if (_shipToAddr->line1() !=so.value("cohead_shiptoaddress1").toString() ||
          _shipToAddr->line2() !=so.value("cohead_shiptoaddress2").toString() ||
          _shipToAddr->line3() !=so.value("cohead_shiptoaddress3").toString() ||
          _shipToAddr->city()  !=so.value("cohead_shiptocity").toString() ||
          _shipToAddr->state() !=so.value("cohead_shiptostate").toString() ||
          _shipToAddr->postalCode()!=so.value("cohead_shiptozipcode").toString() ||
          _shipToAddr->country()!=so.value("cohead_shiptocountry").toString() )
      {
        _shipToAddr->setId(-1);

        _shipToAddr->setLine1(so.value("cohead_shiptoaddress1").toString());
        _shipToAddr->setLine2(so.value("cohead_shiptoaddress2").toString());
        _shipToAddr->setLine3(so.value("cohead_shiptoaddress3").toString());
        _shipToAddr->setCity(so.value("cohead_shiptocity").toString());
        _shipToAddr->setState(so.value("cohead_shiptostate").toString());
        _shipToAddr->setPostalCode(so.value("cohead_shiptozipcode").toString());
        _shipToAddr->setCountry(so.value("cohead_shiptocountry").toString());
      }

      _shipTo->setId(so.value("cohead_shipto_id").toInt());
      _ignoreSignals=false;

      if (_mode == cView)
        _shipTo->setEnabled(FALSE);

      _origin->setCode(so.value("cohead_origin").toString());
      _custPONumber->setText(so.value("cohead_custponumber"));
      _shipVia->setText(so.value("cohead_shipvia"));

      _fob->setText(so.value("cohead_fob"));

      if (so.value("cohead_holdtype").toString() == "N")
        _holdType->setCurrentIndex(0);
      else if (so.value("cohead_holdtype").toString() == "C")
        _holdType->setCurrentIndex(1);
      else if (so.value("cohead_holdtype").toString() == "S")
        _holdType->setCurrentIndex(2);
      else if (so.value("cohead_holdtype").toString() == "P")
        _holdType->setCurrentIndex(3);
      else if (so.value("cohead_holdtype").toString() == "R")
        _holdType->setCurrentIndex(4);

      _miscCharge->setLocalValue(so.value("cohead_misc").toDouble());
      _miscChargeDescription->setText(so.value("cohead_misc_descrip"));
      _miscChargeAccount->setId(so.value("cohead_misc_accnt_id").toInt());

      _orderComments->setText(so.value("cohead_ordercomments").toString());
      _shippingComments->setText(so.value("cohead_shipcomments").toString());
      _shippingCharges->setId(so.value("cohead_shipchrg_id").toInt());
      _shippingForm->setId(so.value("cohead_shipform_id").toInt());

      _calcfreight = so.value("cohead_calcfreight").toBool();
      // Auto calculated _freight is populated in sFillItemList
      if (!_calcfreight)
      {
        disconnect(_freight, SIGNAL(valueChanged()), this, SLOT(sFreightChanged()));
        _freight->setLocalValue(so.value("cohead_freight").toDouble());
        connect(_freight, SIGNAL(valueChanged()), this, SLOT(sFreightChanged()));
      }

      _shipComplete->setChecked(so.value("cohead_shipcomplete").toBool());

      _comments->setId(_soheadid);
      _documents->setId(_soheadid);

      // Check for link to Return Authorization
      if (_metrics->boolean("EnableReturnAuth"))
      {
        q.prepare("SELECT rahead_number "
                  "FROM rahead "
                  "WHERE (rahead_new_cohead_id=:sohead_id);");
        q.bindValue(":sohead_id",_soheadid);
        q.exec();
        if (q.first())
        {
          _fromQuoteLit->setText(tr("From Return Authorization:"));
          _fromQuote->setText(q.value("rahead_number").toString());
        }
      }

      sFillItemList();
    }
    else if (so.lastError().type() != QSqlError::NoError)
    {
      systemError(this, so.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }
  else if (  (_mode == cNewQuote) ||(_mode == cEditQuote) || (_mode == cViewQuote) )
  {
    XSqlQuery qu;
    qu.prepare( "SELECT quhead.*,"
                "       COALESCE(quhead_shipto_id,-1) AS quhead_shipto_id,"
                "       quhead_commission AS commission,"
                "       COALESCE(quhead_taxzone_id, -1) AS quhead_taxzone_id,"
                "       cust_ffshipto, cust_blanketpos,"
                "       COALESCE(quhead_misc_accnt_id,-1) AS quhead_misc_accnt_id, "
                "       COALESCE(quhead_ophead_id,-1) AS quhead_ophead_id, "
                "       CASE WHEN quhead_status IN ('O','') THEN 'Open' "
                "         ELSE CASE WHEN quhead_status ='C' THEN 'Converted' "
                "         END "
                "       END AS status "
                "FROM quhead, custinfo "
                "WHERE ( (quhead_cust_id=cust_id)"
                " AND (quhead_id=:quhead_id) )"
                "UNION "
                "SELECT quhead.*,"
                "       COALESCE(quhead_shipto_id,-1) AS quhead_shipto_id,"
                "       quhead_commission AS commission,"
                "       COALESCE(quhead_taxzone_id, -1) AS quhead_taxzone_id,"
                "       TRUE AS cust_ffshipto, NULL AS cust_blanketpos,"
                "       COALESCE(quhead_misc_accnt_id, -1) AS quhead_misc_accnt_id, "
                "       COALESCE(quhead_ophead_id,-1) AS quhead_ophead_id, "
                "       CASE WHEN quhead_status IN ('O','') THEN 'Open' "
                "         ELSE CASE WHEN quhead_status ='C' THEN 'Converted' "
                "          END "
                "       END AS status "
                "FROM quhead, prospect "
                "WHERE ( (quhead_cust_id=prospect_id)"
                " AND (quhead_id=:quhead_id) )"
                ";" );
    qu.bindValue(":quhead_id", _soheadid);
    qu.exec();
    if (qu.first())
    {
      _orderNumber->setText(qu.value("quhead_number").toString());
      _orderNumber->setEnabled(FALSE);

      _orderDateCache = qu.value("quhead_quotedate").toDate();
      _orderDate->setDate(_orderDateCache, true);
      _packDate->setDate(qu.value("quhead_packdate").toDate());
      if (!qu.value("quhead_expire").isNull())
        _expire->setDate(qu.value("quhead_expire").toDate());

      _cust->setId(qu.value("quhead_cust_id").toInt());

        setFreeFormShipto(qu.value("cust_ffshipto").toBool());
      _blanketPos = qu.value("cust_blanketpos").toBool();

      _warehouse->setId(qu.value("quhead_warehous_id").toInt());
      _salesRep->setId(qu.value("quhead_salesrep_id").toInt());
      _commission->setDouble(qu.value("commission").toDouble() * 100);
      _taxzoneidCache = qu.value("quhead_taxzone_id").toInt();
      _taxZone->setId(qu.value("quhead_taxzone_id").toInt());
      _terms->setId(qu.value("quhead_terms_id").toInt());
      _orderCurrency->setId(qu.value("quhead_curr_id").toInt());
      _project->setId(qu.value("quhead_prj_id").toInt());
      _opportunity->setId(qu.value("quhead_ophead_id").toInt());

      _billToName->setText(qu.value("quhead_billtoname").toString());
      _billToAddr->setLine1(qu.value("quhead_billtoaddress1").toString());
      _billToAddr->setLine2(qu.value("quhead_billtoaddress2").toString());
      _billToAddr->setLine3(qu.value("quhead_billtoaddress3").toString());
      _billToAddr->setCity(qu.value("quhead_billtocity").toString());
      _billToAddr->setState(qu.value("quhead_billtostate").toString());
      _billToAddr->setPostalCode(qu.value("quhead_billtozip").toString());
      _billToAddr->setCountry(qu.value("quhead_billtocountry").toString());

      _shipToCntct->setId(qu.value("quhead_shipto_cntct_id").toInt());
      _shipToCntct->setHonorific(qu.value("quhead_shipto_cntct_honorific").toString());
      _shipToCntct->setFirst(qu.value("quhead_shipto_cntct_first_name").toString());
      _shipToCntct->setMiddle(qu.value("quhead_shipto_cntct_middle").toString());
      _shipToCntct->setLast(qu.value("quhead_shipto_cntct_last_name").toString());
      _shipToCntct->setSuffix(qu.value("quhead_shipto_cntct_suffix").toString());
      _shipToCntct->setPhone(qu.value("quhead_shipto_cntct_phone").toString());
      _shipToCntct->setTitle(qu.value("quhead_shipto_cntct_title").toString());
      _shipToCntct->setFax(qu.value("quhead_shipto_cntct_fax").toString());
      _shipToCntct->setEmailAddress(qu.value("quhead_shipto_cntct_email").toString());

      _billToCntct->setId(qu.value("quhead_billto_cntct_id").toInt());
      _billToCntct->setHonorific(qu.value("quhead_billto_cntct_honorific").toString());
      _billToCntct->setFirst(qu.value("quhead_billto_cntct_first_name").toString());
      _billToCntct->setMiddle(qu.value("quhead_billto_cntct_middle").toString());
      _billToCntct->setLast(qu.value("quhead_billto_cntct_last_name").toString());
      _billToCntct->setSuffix(qu.value("quhead_billto_cntct_suffix").toString());
      _billToCntct->setPhone(qu.value("quhead_billto_cntct_phone").toString());
      _billToCntct->setTitle(qu.value("quhead_billto_cntct_title").toString());
      _billToCntct->setFax(qu.value("quhead_billto_cntct_fax").toString());
      _billToCntct->setEmailAddress(qu.value("quhead_billto_cntct_email").toString());

      _ignoreSignals=true;
      _shipToName->setText(qu.value("quhead_shiptoname").toString());
      if (_shipToAddr->line1() !=qu.value("quhead_shiptoaddress1").toString() ||
          _shipToAddr->line2() !=qu.value("quhead_shiptoaddress2").toString() ||
          _shipToAddr->line3() !=qu.value("quhead_shiptoaddress3").toString() ||
          _shipToAddr->city()  !=qu.value("quhead_shiptocity").toString() ||
          _shipToAddr->state() !=qu.value("quhead_shiptostate").toString() ||
          _shipToAddr->postalCode()!=qu.value("quhead_shiptozipcode").toString() ||
          _shipToAddr->country()!=qu.value("quhead_shiptocountry").toString() )
      {
        _shipToAddr->setId(-1);

        _shipToAddr->setLine1(qu.value("quhead_shiptoaddress1").toString());
        _shipToAddr->setLine2(qu.value("quhead_shiptoaddress2").toString());
        _shipToAddr->setLine3(qu.value("quhead_shiptoaddress3").toString());
        _shipToAddr->setCity(qu.value("quhead_shiptocity").toString());
        _shipToAddr->setState(qu.value("quhead_shiptostate").toString());
        _shipToAddr->setPostalCode(qu.value("quhead_shiptozipcode").toString());
        _shipToAddr->setCountry(qu.value("quhead_shiptocountry").toString());
      }

      _shipTo->setId(qu.value("quhead_shipto_id").toInt());
      _ignoreSignals=false;

      if (_mode == cViewQuote)
        _shipTo->setEnabled(FALSE);

      _origin->setCode(qu.value("quhead_origin").toString());
      _custPONumber->setText(qu.value("quhead_custponumber"));
      _shipVia->setText(qu.value("quhead_shipvia"));

      _fob->setText(qu.value("quhead_fob"));

      _calcfreight = qu.value("quhead_calcfreight").toBool();
      // Auto calculated _freight is populated in sFillItemList
      if (!_calcfreight)
      {
        disconnect(_freight, SIGNAL(valueChanged()), this, SLOT(sFreightChanged()));
        _freight->setLocalValue(qu.value("quhead_freight").toDouble());
        connect(_freight, SIGNAL(valueChanged()), this, SLOT(sFreightChanged()));
      }

      _miscCharge->setLocalValue(qu.value("quhead_misc").toDouble());
      _miscChargeDescription->setText(qu.value("quhead_misc_descrip"));
      _miscChargeAccount->setId(qu.value("quhead_misc_accnt_id").toInt());

      _orderComments->setText(qu.value("quhead_ordercomments").toString());
      _shippingComments->setText(qu.value("quhead_shipcomments").toString());

      _quotestaus->setText(qu.value("status"));

      _comments->setId(_soheadid);
      _documents->setId(_soheadid);
      sFillItemList();
      // TODO - a partial save is not saving everything
      save(false);
    }
    else if (qu.lastError().type() != QSqlError::NoError)
    {
      systemError(this, qu.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }
}

void salesOrder::sFillItemList()
{
  if (ISORDER(_mode))
    q.prepare( "SELECT COALESCE(MIN(coitem_scheddate),:ship_date) AS shipdate "
               "FROM coitem "
               "WHERE ((coitem_status <> 'X')"
               "  AND  (coitem_cohead_id=:head_id));" );
  else
    q.prepare( "SELECT COALESCE(MIN(quitem_scheddate),:ship_date) AS shipdate "
               "FROM quitem "
               "WHERE (quitem_quhead_id=:head_id);" );

  q.bindValue(":head_id", _soheadid);
  q.bindValue(":ship_date", _shipDate->date());
  q.exec();
  if (q.first())
  {
    _shipDateCache = q.value("shipdate").toDate();
    _shipDate->setDate(_shipDateCache);

    if (ISNEW(_mode))
      _packDate->setDate(q.value("shipdate").toDate());
  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  _soitem->clear();
  if (ISORDER(_mode))
  {
    MetaSQLQuery mql = mqlLoad("salesOrderItems", "list");

    ParameterList params;
    if (!_showCanceled->isChecked())
      params.append("excludeCancelled", true);

    params.append("sohead_id", _soheadid);
    XSqlQuery fl = mql.toQuery(params);
    _soitem->populate(fl, true);
    if (fl.lastError().type() != QSqlError::NoError)
    {
      systemError(this, fl.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }

    _cust->setReadOnly(fl.size() || !ISNEW(_mode));
    _amountAtShipping->setLocalValue(0.0);
    QString sql = "SELECT ROUND(((COALESCE(SUM(shipitem_qty),0)-coitem_qtyshipped) *"
          "                  coitem_qty_invuomratio) *"
          "           (coitem_price / coitem_price_invuomratio),2) AS shippingAmount "
          "  FROM coitem LEFT OUTER JOIN "
          "       (shipitem JOIN shiphead ON (shipitem_shiphead_id=shiphead_id"
          "                               AND shiphead_order_id=:cohead_id"
          "                               AND shiphead_order_type='SO')) ON (shipitem_orderitem_id=coitem_id)"
          " WHERE ((coitem_cohead_id=:cohead_id)";

    if (!_showCanceled->isChecked())
      sql += " AND (coitem_status != 'X') ";

    sql += ") GROUP BY coitem_id, coitem_qtyshipped, coitem_qty_invuomratio,"
           "coitem_price, coitem_price_invuomratio;";
    q.prepare(sql);
    q.bindValue(":cohead_id", _soheadid);
    q.bindValue(":cust_id", _cust->id());
    q.exec();
    while (q.next())
      _amountAtShipping->setLocalValue(_amountAtShipping->localValue() +
                                       q.value("shippingAmount").toDouble());
    if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }
  else if (ISQUOTE(_mode))
  {
    XSqlQuery fl;
    fl.prepare( "SELECT quitem_id,"
                "       quitem_linenumber AS f_linenumber,"
                "       0 AS coitem_subnumber, item_type,"
                "       item_number, (item_descrip1 || ' ' || item_descrip2) AS description,"
                "       warehous_code, '' AS enhanced_status,"
                "       quitem_scheddate AS coitem_scheddate,"
                "       quom.uom_name AS qty_uom,"
                "       quitem_qtyord AS coitem_qtyord,"
                "       0 AS qtyshipped, 0 AS qtyatshipping, 0 AS balance,"
                "       puom.uom_name AS price_uom,"
                "       quitem_price AS coitem_price,"
                "       ROUND((quitem_qtyord * quitem_qty_invuomratio) *"
                "             (quitem_price / quitem_price_invuomratio),2) AS extprice,"
                "       'qty' AS coitem_qtyord_xtnumericrole,"
                "       'qty' AS qtyshipped_xtnumericrole,"
                "       'qty' AS balance_xtnumericrole,"
                "       'qty' AS qtyatshipping_xtnumericrole,"
                "       'salesprice' AS coitem_price_xtnumericrole,"
                "       'curr' AS extprice_xtnumericrole "
                "  FROM item, uom AS quom, uom AS puom,"
                "       quitem LEFT OUTER JOIN (itemsite JOIN warehous ON (itemsite_warehous_id=warehous_id)) ON (quitem_itemsite_id=itemsite_id) "
                " WHERE ( (quitem_item_id=item_id)"
                "   AND   (quitem_qty_uom_id=quom.uom_id)"
                "   AND   (quitem_price_uom_id=puom.uom_id)"
                "   AND   (quitem_quhead_id=:quhead_id) ) "
                "ORDER BY quitem_linenumber;" );
    fl.bindValue(":quhead_id", _soheadid);
    fl.exec();
    _cust->setReadOnly(fl.size() || !ISNEW(_mode));
    _soitem->populate(fl);
    if (fl.lastError().type() != QSqlError::NoError)
    {
      systemError(this, fl.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }

  //  Determine the subtotal
  if (ISORDER(_mode))
    q.prepare( "SELECT SUM(round((coitem_qtyord * coitem_qty_invuomratio) * (coitem_price / coitem_price_invuomratio),2)) AS subtotal,"
               "       SUM(round((coitem_qtyord * coitem_qty_invuomratio) * currToCurr(baseCurrId(), cohead_curr_id, stdCost(item_id), cohead_orderdate),2)) AS totalcost "
               "FROM coitem, cohead, itemsite, item "
               "WHERE ( (coitem_cohead_id=:head_id)"
               " AND (coitem_cohead_id=cohead_id)"
               " AND (coitem_itemsite_id=itemsite_id)"
               " AND (coitem_status <> 'X')"
               " AND (itemsite_item_id=item_id) );" );
  else
    q.prepare( "SELECT SUM(round((quitem_qtyord * quitem_qty_invuomratio) * (quitem_price / quitem_price_invuomratio),2)) AS subtotal,"
               "       SUM(round((quitem_qtyord * quitem_qty_invuomratio) * currToCurr(baseCurrId(), quhead_curr_id, stdCost(item_id), quhead_quotedate),2)) AS totalcost "
               "  FROM quitem, quhead, item "
               " WHERE ( (quitem_quhead_id=:head_id)"
               "   AND   (quitem_quhead_id=quhead_id)"
               "   AND   (quitem_item_id=item_id) );" );
  q.bindValue(":head_id", _soheadid);
  q.exec();
  if (q.first())
  {
    _subtotal->setLocalValue(q.value("subtotal").toDouble());
    _margin->setLocalValue(q.value("subtotal").toDouble() - q.value("totalcost").toDouble());
  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  if (ISORDER(_mode))
    q.prepare("SELECT SUM(COALESCE(coitem_qtyord * coitem_qty_invuomratio, 0.00) *"
              "           COALESCE(item_prodweight, 0.00)) AS netweight,"
              "       SUM(COALESCE(coitem_qtyord * coitem_qty_invuomratio, 0.00) *"
              "           (COALESCE(item_prodweight, 0.00) +"
              "            COALESCE(item_packweight, 0.00))) AS grossweight "
              "FROM coitem, itemsite, item, cohead "
              "WHERE ((coitem_itemsite_id=itemsite_id)"
              " AND (itemsite_item_id=item_id)"
              " AND (coitem_cohead_id=cohead_id)"
              " AND (coitem_status<>'X')"
              " AND (coitem_cohead_id=:head_id)) "
              "GROUP BY cohead_freight;");
  else if (ISQUOTE(_mode))
    q.prepare("SELECT SUM(COALESCE(quitem_qtyord, 0.00) *"
              "           COALESCE(item_prodweight, 0.00)) AS netweight,"
              "       SUM(COALESCE(quitem_qtyord, 0.00) *"
              "           (COALESCE(item_prodweight, 0.00) +"
              "            COALESCE(item_packweight, 0.00))) AS grossweight "
              "  FROM quitem, item, quhead "
              " WHERE ( (quitem_item_id=item_id)"
              "   AND   (quitem_quhead_id=quhead_id)"
              "   AND   (quitem_quhead_id=:head_id)) "
              " GROUP BY quhead_freight;");
  q.bindValue(":head_id", _soheadid);
  q.exec();
  if (q.first())
    _weight->setDouble(q.value("grossweight").toDouble());
  else if (q.lastError().type() != QSqlError::NoError)
  {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  if (_calcfreight)
  {
    if (ISORDER(_mode))
      q.prepare("SELECT SUM(freightdata_total) AS freight "
                "FROM freightDetail('SO', :head_id, :cust_id, :shipto_id, :orderdate, :shipvia, :curr_id);");
    else if (ISQUOTE(_mode))
      q.prepare("SELECT SUM(freightdata_total) AS freight "
                "FROM freightDetail('QU', :head_id, :cust_id, :shipto_id, :orderdate, :shipvia, :curr_id);");
    q.bindValue(":head_id", _soheadid);
    q.bindValue(":cust_id", _cust->id());
    q.bindValue(":shipto_id", _shipTo->id());
    q.bindValue(":orderdate", _orderDate->date());
    q.bindValue(":shipvia", _shipVia->currentText());
    q.bindValue(":curr_id", _orderCurrency->id());
    q.exec();
    if (q.first())
    {
      _freightCache = q.value("freight").toDouble();
      disconnect(_freight, SIGNAL(valueChanged()), this, SLOT(sFreightChanged()));
      _freight->setLocalValue(_freightCache);
      connect(_freight, SIGNAL(valueChanged()), this, SLOT(sFreightChanged()));
    }
    else if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }

  sCalculateTax();  // triggers sCalculateTotal();

  _orderCurrency->setEnabled(_soitem->topLevelItemCount() == 0);
}

void salesOrder::sCalculateTotal()
{
  _total->setLocalValue(_subtotal->localValue() + _tax->localValue() + _miscCharge->localValue() + _freight->localValue());

  double balance = _total->localValue() - _allocatedCM->localValue() - _authCC->localValue() - _amountOutstanding;
  if (balance < 0)
    balance = 0;
  _balance->setLocalValue(balance);
  _CCAmount->setLocalValue(balance);
  if (ISVIEW(_mode) || balance==0)
  {
    _authorize->hide();
    _charge->hide();
  }
  else
  {
    _authorize->setVisible(_metrics->boolean("CCEnablePreauth"));
    _charge->setVisible(_metrics->boolean("CCEnableCharge"));
  }
}

bool salesOrder::deleteForCancel()
{
  XSqlQuery query;

  if (ISNEW(_mode) &&
      _soitem->topLevelItemCount() > 0 &&
      !_captive)
  {
    int answer;
    if (_mode == cNew)
      answer = QMessageBox::question(this, tr("Delete Sales Order?"),
                                      tr("<p>Are you sure you want to delete this "
                                          "Sales Order and its associated Line Items?"),
                                     QMessageBox::Yes,
                                     QMessageBox::No | QMessageBox::Default);
    else
      answer = QMessageBox::question(this, tr("Delete Quote?"),
                                      tr("<p>Are you sure you want to delete this "
                                          "Quote and its associated Line Items?"),
                                     QMessageBox::Yes,
                                     QMessageBox::No | QMessageBox::Default);
    if (answer == QMessageBox::No)
      return false;
  }

  if (_mode == cNew &&
      !_captive)
  {
    query.prepare("SELECT deleteSO(:sohead_id, :sohead_number) AS result;");
    query.bindValue(":sohead_id", _soheadid);
    query.bindValue(":sohead_number", _orderNumber->text());
    query.exec();
    if (query.first())
    {
      int result = query.value("result").toInt();
      if (result < 0)
        systemError(this, storedProcErrorLookup("deleteSO", result),
                    __FILE__, __LINE__);
    }
    else if (query.lastError().type() != QSqlError::NoError)
        systemError(this, query.lastError().databaseText(), __FILE__, __LINE__);

    if ((_metrics->value("CONumberGeneration") == "A") ||
        (_metrics->value("CONumberGeneration") == "O"))
    {
      query.prepare( "SELECT releaseSONumber(:orderNumber);" );
      query.bindValue(":orderNumber", _orderNumber->text());
      query.exec();
      if (query.lastError().type() != QSqlError::NoError)
        systemError(this, query.lastError().databaseText(), __FILE__, __LINE__);
    }
  }
  else if (_mode == cNewQuote &&
           !_captive)
  {
    query.prepare("SELECT deleteQuote(:head_id, :quhead_number) AS result;");
    query.bindValue(":head_id", _soheadid);
    query.bindValue(":quhead_number", _orderNumberGen);
    query.exec();
    if (query.first())
    {
      int result = query.value("result").toInt();
      if (result < 0)
        systemError(this, storedProcErrorLookup("deleteQuote", result),
                    __FILE__, __LINE__);
    }
    else if (query.lastError().type() != QSqlError::NoError)
        systemError(this, query.lastError().databaseText(), __FILE__, __LINE__);

    if ((_metrics->value("QUNumberGeneration") == "S") ||
        (_metrics->value("QUNumberGeneration") == "A") ||
        (_metrics->value("QUNumberGeneration") == "O"))
    {
      if (_metrics->value("QUNumberGeneration") == "S")
        query.prepare( "SELECT releaseSoNumber(:orderNumber);" );
      else
        query.prepare( "SELECT releaseQUNumber(:orderNumber);" );
      query.bindValue(":orderNumber", _orderNumberGen);
      query.exec();
      if (query.lastError().type() != QSqlError::NoError)
        systemError(this, query.lastError().databaseText(), __FILE__, __LINE__);
    }
  }

  if (cView != _mode && _locked)
  {
    query.prepare("SELECT pg_advisory_unlock(oid::integer, :sohead_id) AS result "
                  "FROM pg_class "
                  "WHERE relname=:table;");
    query.bindValue(":sohead_id", _soheadid);
    if (ISORDER(_mode))
      query.bindValue(":table", "cohead");
    else
      query.bindValue(":table", "quhead");
    query.exec();
    if (query.first() && !query.value("result").toBool())
        systemError(this, tr("Could not release this Sales Order record."),
                  __FILE__, __LINE__);
    else if (query.lastError().type() != QSqlError::NoError)
        systemError(this, query.lastError().databaseText(), __FILE__, __LINE__);
    else
      _locked=false;
  }

  return true;
}

void salesOrder::sClear()
{
  if (!deleteForCancel())
    return;

  _captive = false;
  clear();
}

void salesOrder::clear()
{
  if (cView != _mode && _locked)
  {
    q.prepare("SELECT pg_advisory_unlock(oid::integer, :sohead_id) AS result "
              "FROM pg_class "
              "WHERE relname=:table;");
    q.bindValue(":sohead_id", _soheadid);
    if (ISORDER(_mode))
      q.bindValue(":table", "cohead");
    else
      q.bindValue(":table", "quhead");
    q.exec();
    if (q.first() && !q.value("result").toBool())
      systemError(this, tr("Could not release this Sales Order record."),
                  __FILE__, __LINE__);
    else if (q.lastError().type() != QSqlError::NoError)
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    else
      _locked=false;
  }

  _salesOrderInformation->setCurrentIndex(0);

  _orderNumber->setEnabled(TRUE);
  _orderNumberGen = 0;
  _orderNumber->clear();

  _shipDate->clear();
  _ignoreSignals = true;
  _cust->setId(-1);
  _shipTo->setId(-1);
  _ignoreSignals = false;
  _warehouse->setId(_preferences->value("PreferredWarehouse").toInt());
  _salesRep->setCurrentIndex(-1);
  _commission->clear();
  _billToAddr->setId(-1);
  _shipToAddr->setId(-1);
  _billToName->clear();
  _shipToName->clear();
  _taxZone->setCurrentIndex(-1);
  _taxzoneidCache  = -1;
  _custtaxzoneid   = -1;
  _terms->setCurrentIndex(-1);
  _origin->setCurrentIndex(0);
  _shipVia->setCurrentIndex(-1);
  _shippingCharges->setCurrentIndex(-1);
  _shippingForm->setCurrentIndex(-1);
  _holdType->setCurrentIndex(0);
  _calcfreight   = _metrics->boolean("CalculateFreight");
  _freightCache  = 0;
  disconnect(_freight, SIGNAL(valueChanged()), this, SLOT(sFreightChanged()));
  _freight->clear();
  connect(_freight, SIGNAL(valueChanged()), this, SLOT(sFreightChanged()));
  _orderComments->clear();
  _shippingComments->clear();
  _custPONumber->clear();
  _miscCharge->clear();
  _miscChargeDescription->clear();
  _miscChargeAccount->setId(-1);
  _subtotal->clear();
  _tax->clear();
  _miscCharge->clear();
  _total->clear();
  _orderCurrency->setCurrentIndex(0);
  _orderCurrency->setEnabled(true);
  _weight->clear();
  _allocatedCM->clear();
  _outstandingCM->clear();
  _authCC->clear();
  _balance->clear();
  _CCAmount->clear();
  _CCCVV->clear();
  _project->setId(-1);
  _fromQuoteLit->setText(tr("From Quote:"));

  _fromQuote->setText(tr("No"));

  _shipComplete->setChecked(false);

  if ( (_mode == cEdit) || (_mode == cNew) )
  {
    _mode = cNew;
    setObjectName("salesOrder new");
    _orderDateCache = omfgThis->dbDate();
    _orderDate->setDate(_orderDateCache, true);
  }
  else if ( (_mode == cEditQuote) || (_mode == cNewQuote) )
    _mode = cNewQuote;

  populateOrderNumber();
  if (_orderNumber->text().isEmpty())
    _orderNumber->setFocus();
  else
    _cust->setFocus();

  XSqlQuery headid;
  if (ISORDER(_mode))
    headid.exec("SELECT NEXTVAL('cohead_cohead_id_seq') AS _soheadid");
  else
    headid.exec("SELECT NEXTVAL('quhead_quhead_id_seq') AS _soheadid");

  if (headid.first())
  {
    _soheadid = headid.value("_soheadid").toInt();
    _comments->setId(_soheadid);
    _documents->setId(_soheadid);
    if (ISORDER(_mode))
    {
      populateCMInfo();
      populateCCInfo();
      sFillCcardList();
    }
  }
  else if (headid.lastError().type() != QSqlError::NoError)
    systemError(this, headid.lastError().databaseText(), __FILE__, __LINE__);

  _soitem->clear();
  _cust->setReadOnly(FALSE);

  _saved = false;
}

void salesOrder::closeEvent(QCloseEvent *pEvent)
{
  if (!deleteForCancel())
  {
    pEvent->ignore();
    return;
  }

  disconnect(_orderNumber, SIGNAL(lostFocus()), this, SLOT(sHandleOrderNumber()));

  if (cNew == _mode && _saved)
    omfgThis->sSalesOrdersUpdated(-1);
  else if (cNewQuote == _mode && _saved)
    omfgThis->sQuotesUpdated(-1);

  _preferences->set("SoShowAll", _more->isChecked());

  XWidget::closeEvent(pEvent);
}

void salesOrder::sHandleShipchrg(int pShipchrgid)
{
  if ( (_mode == cView) || (_mode == cViewQuote) )
    _freight->setEnabled(FALSE);
  else
  {
    XSqlQuery query;
    query.prepare( "SELECT shipchrg_custfreight "
                   "FROM shipchrg "
                   "WHERE (shipchrg_id=:shipchrg_id);" );
    query.bindValue(":shipchrg_id", pShipchrgid);
    query.exec();
    if (query.first())
    {
      if (query.value("shipchrg_custfreight").toBool())
      {
        _calcfreight = _metrics->boolean("CalculateFreight");
        _freight->setEnabled(TRUE);
        sFillItemList();
      }
      else
      {
        _calcfreight   = FALSE;
        _freightCache  = 0;
        _freight->setEnabled(FALSE);
        disconnect(_freight, SIGNAL(valueChanged()), this, SLOT(sFreightChanged()));
        _freight->clear();
        connect(_freight, SIGNAL(valueChanged()), this, SLOT(sFreightChanged()));
        sCalculateTax();
      }
    }
  }
}

void salesOrder::sHandleSalesOrderEvent(int pSoheadid, bool)
{
  if (pSoheadid == _soheadid)
    sFillItemList();
}

void salesOrder::sTaxDetail()
{
  XSqlQuery taxq;
  if (!ISVIEW(_mode))
  {
    if (ISORDER(_mode))
      taxq.prepare("UPDATE cohead SET cohead_taxzone_id=:taxzone_id, "
                   "  cohead_freight=:freight,"
                   "  cohead_orderdate=:date "
                   "WHERE (cohead_id=:head_id);");
    else
      taxq.prepare("UPDATE quhead SET quhead_taxzone_id=:taxzone_id, "
                   "  quhead_freight=:freight,"
                   "  quhead_quotedate=:date "
                   "WHERE (quhead_id=:head_id);");
    if (_taxZone->isValid())
      taxq.bindValue(":taxzone_id",        _taxZone->id());
    taxq.bindValue(":freight",        _freight->localValue());
    taxq.bindValue(":date",        _orderDate->date());
    taxq.bindValue(":head_id", _soheadid);
    taxq.exec();
    if (taxq.lastError().type() != QSqlError::NoError)
    {
      systemError(this, taxq.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }

  ParameterList params;
  params.append("order_id", _soheadid);
  if (ISORDER(_mode))
    params.append("order_type", "S");
  else
    params.append("order_type", "Q");

  // mode => view since there are no fields to hold modified tax data
  params.append("mode", "view");

  taxBreakdown newdlg(this, "", TRUE);
  if (newdlg.set(params) == NoError && newdlg.exec() == XDialog::Accepted)
  {
    populate();
  }
}

void salesOrder::sFreightDetail()
{
  ParameterList params;
  params.append("calcfreight", _calcfreight);
  if (ISORDER(_mode))
    params.append("order_type", "SO");
  else
    params.append("order_type", "QU");
  params.append("order_id", _soheadid);
  params.append("document_number", _orderNumber->text());
  params.append("cust_id", _cust->id());
  params.append("shipto_id", _shipTo->id());
  params.append("orderdate", _orderDate->date());
  params.append("shipvia", _shipVia->currentText());
  params.append("curr_id", _orderCurrency->id());

  // mode => view since there are no fields to hold modified freight data
  params.append("mode", "view");

  freightBreakdown newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
  if (_saved)
    populate();
}

void salesOrder::setFreeFormShipto(bool pFreeForm)
{
  _ffShipto = pFreeForm;

  // If we are in view mode it doesn't matter as we
  // always want these fields disabled.
  if ( (_mode == cView) || (_mode == cViewQuote) )
    _ffShipto = false;

  _shipToName->setEnabled(_ffShipto);
  _shipToAddr->setEnabled(_ffShipto);
  _shipToCntct->setEnabled(_ffShipto);

  _copyToShipto->setEnabled(_ffShipto);
}

void salesOrder::setViewMode()
{
  if (cEdit == _mode)
  {
    // Undo some changes set for the edit mode
    _captive = false;

    disconnect( _cust,    SIGNAL(valid(bool)),                    _new, SLOT(setEnabled(bool)));
    disconnect( omfgThis, SIGNAL(salesOrdersUpdated(int, bool)),  this, SLOT(sHandleSalesOrderEvent(int, bool)));

    _new->setEnabled(false);
  }

  _mode = cView;
  setObjectName(QString("salesOrder view %1").arg(_soheadid));

  _orderNumber->setEnabled(FALSE);
  _packDate->setEnabled(FALSE);
  _shipDate->setEnabled(FALSE);
  _opportunity->setEnabled(FALSE);
  _cust->setReadOnly(TRUE);
  _warehouse->setEnabled(FALSE);
  _salesRep->setEnabled(FALSE);
  _commission->setEnabled(FALSE);
  _taxZone->setEnabled(FALSE);
  _terms->setEnabled(FALSE);
  _origin->setEnabled(FALSE);
  _fob->setEnabled(FALSE);
  _shipVia->setEnabled(FALSE);
  _shippingCharges->setEnabled(FALSE);
  _shippingForm->setEnabled(FALSE);
  _miscCharge->setEnabled(FALSE);
  _miscChargeDescription->setEnabled(FALSE);
  _miscChargeAccount->setReadOnly(TRUE);
  _miscChargeAccount->setEnabled(FALSE);
  _freight->setEnabled(FALSE);
  _orderComments->setEnabled(FALSE);
  _shippingComments->setEnabled(FALSE);
  _custPONumber->setEnabled(FALSE);
  _holdType->setEnabled(FALSE);
  _edit->setText(tr("View"));
  _comments->setType(Comments::SalesOrder);
  _comments->setReadOnly(true);
  _documents->setType(Documents::SalesOrder);
  _documents->setReadOnly(true);
  _shipComplete->setEnabled(false);
  setFreeFormShipto(false);
  _orderCurrency->setEnabled(FALSE);
  _printSO->setEnabled(FALSE);
  _save->hide();
  _clear->hide();
  _project->setReadOnly(true);
  if (_metrics->boolean("AlwaysShowSaveAndAdd"))
    _saveAndAdd->setEnabled(false);
  else
    _saveAndAdd->hide();
  _action->hide();
  _delete->hide();
}

void salesOrder::newSalesOrder(int pCustid, QWidget *parent)
{
  // Check for an Item window in new mode already.
  if (pCustid == -1)
  {
    QWidgetList list = omfgThis->windowList();
    for (int i = 0; i < list.size(); i++)
    {
      QWidget *w = list.at(i);
      if (QString::compare(w->objectName(), "salesOrder new")==0)
      {
        w->setFocus();
        if (omfgThis->showTopLevel())
        {
          w->raise();
          w->activateWindow();
        }
        return;
      }
    }
  }

  // If none found then create one.
  ParameterList params;
  params.append("mode", "new");
  if (pCustid != -1)
    params.append("cust_id", pCustid);

  salesOrder *newdlg = new salesOrder(parent);
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void salesOrder::editSalesOrder( int pId, bool enableSaveAndAdd, QWidget *parent )
{
  // Check for an Item window in edit mode for the specified salesOrder already.
  QString     n    = QString("salesOrder edit %1").arg(pId);
  QWidgetList list = omfgThis->windowList();
  for (int i = 0; i < list.size(); i++)
  {
    QWidget *w = list.at(i);
    if (QString::compare(w->objectName(), n)==0)
    {
      w->setFocus();
      if (omfgThis->showTopLevel())
      {
        w->raise();
        w->activateWindow();
      }
      return;
    }
  }

  // If none found then create one.
  ParameterList params;
  params.append("mode", "edit");
  params.append("sohead_id", pId);
  if (enableSaveAndAdd)
    params.append("enableSaveAndAdd");

  salesOrder *newdlg = new salesOrder(parent);
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void salesOrder::viewSalesOrder( int pId, QWidget *parent )
{
  // Check for an Item window in edit mode for the specified salesOrder already.
  QString     n    = QString("salesOrder view %1").arg(pId);
  QWidgetList list = omfgThis->windowList();
  for (int i = 0; i < list.size(); i++)
  {
    QWidget *w = list.at(i);
    if (QString::compare(w->objectName(), n)==0)
    {
      w->setFocus();
      if (omfgThis->showTopLevel())
      {
        w->raise();
        w->activateWindow();
      }
      return;
    }
  }

  // If none found then create one.
  ParameterList params;
  params.append("mode", "view");
  params.append("sohead_id", pId);

  salesOrder *newdlg = new salesOrder(parent);
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void salesOrder::populateCMInfo()
{
  if (cNew != _mode && cEdit != _mode && cView != _mode)
    return;

  // Allocated C/M's
  q.prepare("SELECT COALESCE(SUM(currToCurr(aropenalloc_curr_id, :curr_id,"
            "                               aropenalloc_amount, :effective)),0) AS amount"
            "  FROM aropenalloc, aropen"
            " WHERE ( (aropenalloc_doctype='S')"
            "  AND    (aropenalloc_doc_id=:doc_id)"
            "  AND    (aropenalloc_aropen_id=aropen_id) ); ");
  q.bindValue(":doc_id",    _soheadid);
  q.bindValue(":curr_id",   _allocatedCM->id());
  q.bindValue(":effective", _allocatedCM->effective());
  q.exec();
  if (q.first())
    _allocatedCM->setLocalValue(q.value("amount").toDouble());
  else
    _allocatedCM->setLocalValue(0);

  // Unallocated C/M's
  q.prepare("SELECT SUM(amount) AS f_amount"
            " FROM (SELECT aropen_id,"
            "        currToCurr(aropen_curr_id, :curr_id,"
            "               noNeg(aropen_amount - aropen_paid - SUM(COALESCE(aropenalloc_amount,0))),"
            "               :effective) AS amount "
            "       FROM cohead, aropen LEFT OUTER JOIN aropenalloc ON (aropenalloc_aropen_id=aropen_id)"
            "       WHERE ( (aropen_cust_id=cohead_cust_id)"
            "         AND   (aropen_doctype IN ('C', 'R'))"
            "         AND   (aropen_open)"
            "         AND   (cohead_id=:cohead_id) )"
            "       GROUP BY aropen_id, aropen_amount, aropen_paid, aropen_curr_id) AS data; ");
  q.bindValue(":cohead_id", _soheadid);
  q.bindValue(":curr_id",   _outstandingCM->id());
  q.bindValue(":effective", _outstandingCM->effective());
  q.exec();
  if (q.first())
    _outstandingCM->setLocalValue(q.value("f_amount").toDouble());
  else
    _outstandingCM->setLocalValue(0);
}

void salesOrder::populateCCInfo()
{
  if (cNew != _mode && cEdit != _mode && cView != _mode)
    return;

  int ccValidDays = _metrics->value("CCValidDays").toInt();
  if (ccValidDays < 1)
    ccValidDays = 7;

  q.prepare("SELECT COALESCE(SUM(currToCurr(payco_curr_id, :curr_id,"
            "                               payco_amount, :effective)),0) AS amount"
            "  FROM ccpay, payco"
            " WHERE ( (ccpay_status = 'A')"
            "   AND   (date_part('day', CURRENT_TIMESTAMP - ccpay_transaction_datetime) < :ccValidDays)"
            "   AND   (payco_ccpay_id=ccpay_id)"
            "   AND   (payco_cohead_id=:cohead_id) ); ");
  q.bindValue(":cohead_id", _soheadid);
  q.bindValue(":ccValidDays", ccValidDays);
  q.bindValue(":curr_id",   _authCC->id());
  q.bindValue(":effective", _authCC->effective());
  q.exec();
  if (q.first())
    _authCC->setLocalValue(q.value("amount").toDouble());
  else
    _authCC->setLocalValue(0);
}

void salesOrder::sNewCreditCard()
{
  ParameterList params;
  params.append("mode", "new");
  params.append("cust_id", _cust->id());

  creditCard newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillCcardList();
}

void salesOrder::sEditCreditCard()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("cust_id", _cust->id());
  params.append("ccard_id", _cc->id());

  creditCard newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillCcardList();
}

void salesOrder::sViewCreditCard()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("cust_id", _cust->id());
  params.append("ccard_id", _cc->id());

  creditCard newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void salesOrder::sMoveUp()
{
  q.prepare("SELECT moveCcardUp(:ccard_id) AS result;");
  q.bindValue(":ccard_id", _cc->id());
  q.exec();

    sFillCcardList();
}

void salesOrder::sMoveDown()
{
  q.prepare("SELECT moveCcardDown(:ccard_id) AS result;");
  q.bindValue(":ccard_id", _cc->id());
  q.exec();

    sFillCcardList();
}

void salesOrder::sFillCcardList()
{
  if (ISQUOTE(_mode) || (!_metrics->boolean("CCAccept") || !_privileges->check("ProcessCreditCards")))
    return;

  q.prepare( "SELECT expireCreditCard(:cust_id, setbytea(:key));");
  q.bindValue(":cust_id", _cust->id());
  q.bindValue(":key", omfgThis->_key);
  q.exec();

  MetaSQLQuery  mql = mqlLoad("creditCards", "detail");
  ParameterList params;
  params.append("cust_id",         _cust->id());
  params.append("masterCard",      tr("MasterCard"));
  params.append("visa",            tr("VISA"));
  params.append("americanExpress", tr("American Express"));
  params.append("discover",        tr("Discover"));
  params.append("other",           tr("Other"));
  params.append("key",             omfgThis->_key);
  params.append("activeonly",      true);
  XSqlQuery cl = mql.toQuery(params);
  _cc->populate(cl);
  if (cl.lastError().type() != QSqlError::NoError)
  {
    systemError(this, cl.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void salesOrder::sAuthorizeCC()
{
  if (!okToProcessCC())
    return;

  CreditCardProcessor *cardproc = CreditCardProcessor::getProcessor();
  if (!cardproc)
  {
    QMessageBox::critical(this, tr("Credit Card Processing Error"),
                          CreditCardProcessor::errorMsg());
    return;
  }

  if (!cardproc->errorMsg().isEmpty())
  {
    QMessageBox::warning( this, tr("Credit Card Error"), cardproc->errorMsg() );
    return;
  }

  _authorize->setEnabled(false);
  _charge->setEnabled(false);

  int     ccpayid    = -1;
  QString sonumber   = _orderNumber->text();
  QString ponumber   = _custPONumber->text();
  int     returnVal  = cardproc->authorize(_cc->id(), _CCCVV->text().toInt(),
                                           _CCAmount->localValue(),
                                           _tax->localValue(),
                                           (_tax->isZero() && _taxZone->id() == -1),
                                           _freight->localValue(), 0,
                                           _CCAmount->id(),
                                           sonumber, ponumber, ccpayid,
                                           QString("cohead"), _soheadid);
  if (returnVal < 0)
    QMessageBox::critical(this, tr("Credit Card Processing Error"),
                          cardproc->errorMsg());
  else if (returnVal > 0)
    QMessageBox::warning(this, tr("Credit Card Processing Warning"),
                         cardproc->errorMsg());
  else if (!cardproc->errorMsg().isEmpty())
    QMessageBox::information(this, tr("Credit Card Processing Note"),
                             cardproc->errorMsg());
  else
    _CCAmount->clear();

  _authorize->setEnabled(true);
  _charge->setEnabled(true);

  populateCMInfo();
  populateCCInfo();
  sFillCcardList();
  _CCCVV->clear();
}

void salesOrder::sChargeCC()
{
  if (!okToProcessCC())
    return;

  CreditCardProcessor *cardproc = CreditCardProcessor::getProcessor();
  if (!cardproc)
  {
    QMessageBox::critical(this, tr("Credit Card Processing Error"),
                          CreditCardProcessor::errorMsg());
    return;
  }

  if (!cardproc->errorMsg().isEmpty())
  {
    QMessageBox::warning( this, tr("Credit Card Error"), cardproc->errorMsg() );
    return;
  }

  _authorize->setEnabled(false);
  _charge->setEnabled(false);

  int     ccpayid    = -1;
  QString ordernum   = _orderNumber->text();
  QString refnum     = _custPONumber->text();
  int     returnVal  = cardproc->charge(_cc->id(), _CCCVV->text().toInt(),
                                        _CCAmount->localValue(),
                                        _tax->localValue(),
                                        (_tax->isZero() && _taxZone->id() == -1),
                                        _freight->localValue(), 0,
                                        _CCAmount->id(),
                                        ordernum, refnum, ccpayid,
                                        QString("cohead"), _soheadid);
  if (returnVal < 0)
    QMessageBox::critical(this, tr("Credit Card Processing Error"),
                          cardproc->errorMsg());
  else if (returnVal > 0)
    QMessageBox::warning(this, tr("Credit Card Processing Warning"),
                         cardproc->errorMsg());
  else if (!cardproc->errorMsg().isEmpty())
    QMessageBox::information(this, tr("Credit Card Processing Note"),
                             cardproc->errorMsg());
  else
    _CCAmount->clear();

  _authorize->setEnabled(true);
  _charge->setEnabled(true);

  populateCMInfo();
  populateCCInfo();
  sFillCcardList();
  _CCCVV->clear();
}

bool salesOrder::okToProcessCC()
{
  if (_usesPos)
  {
    if (_custPONumber->text().trimmed().length() == 0)
    {
      QMessageBox::warning( this, tr("Cannot Process Credit Card Transaction"),
                              tr("<p>You must enter a Customer P/O for this "
                                 "Sales Order before you may process a credit"
                                 "card transaction.") );
      _custPONumber->setFocus();
      return false;
    }

    if (!_blanketPos)
    {
      q.prepare( "SELECT cohead_id"
                 "  FROM cohead"
                 " WHERE ((cohead_cust_id=:cohead_cust_id)"
                 "   AND  (cohead_id<>:cohead_id)"
                 "   AND  (UPPER(cohead_custponumber) = UPPER(:cohead_custponumber)) )"
                 " UNION "
                 "SELECT quhead_id"
                 "  FROM quhead"
                 " WHERE ((quhead_cust_id=:cohead_cust_id)"
                 "   AND  (quhead_id<>:cohead_id)"
                 "   AND  (UPPER(quhead_custponumber) = UPPER(:cohead_custponumber)) );" );
      q.bindValue(":cohead_cust_id", _cust->id());
      q.bindValue(":cohead_id", _soheadid);
      q.bindValue(":cohead_custponumber", _custPONumber->text());
      q.exec();
      if (q.first())
      {
        QMessageBox::warning( this, tr("Cannot Process Credit Card Transaction"),
                              tr("<p>This Customer does not use Blanket P/O "
                                   "Numbers and the P/O Number you entered has "
                                   "already been used for another Sales Order. "
                                   "Please verify the P/O Number and either "
                                   "enter a new P/O Number or add to the "
                                   "existing Sales Order." ) );
        _custPONumber->setFocus();
        return false;
      }
    }
  }

  return true;
}

void salesOrder::sReturnStock()
{
  XSqlQuery rollback;
  rollback.prepare("ROLLBACK;");

  q.exec("BEGIN;"); // because of possible lot, serial, or location distribution cancelations
  q.prepare("SELECT returnItemShipments(:soitem_id) AS result;");
  QList<XTreeWidgetItem *> selected = _soitem->selectedItems();
  for (int i = 0; i < selected.size(); i++)
  {
    q.bindValue(":soitem_id", ((XTreeWidgetItem *)(selected[i]))->id());
    q.exec();
    if (q.first())
    {
      int result = q.value("result").toInt();
      if (result < 0)
      {
        rollback.exec();
        systemError(this, storedProcErrorLookup("returnItemShipments", result) +
                    tr("<br>Line Item %1").arg(selected[i]->text(0)),
                    __FILE__, __LINE__);
        return;
      }
      if (distributeInventory::SeriesAdjust(q.value("result").toInt(), this) == XDialog::Rejected)
      {
        rollback.exec();
        QMessageBox::information( this, tr("Return Stock"), tr("Transaction Canceled") );
        return;
      }

    }
    else if (q.lastError().type() != QSqlError::NoError)
    {
      rollback.exec();
      systemError(this, tr("Line Item %1\n").arg(selected[i]->text(0)) +
                  q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }

  q.exec("COMMIT;");

    sFillItemList();
}

void salesOrder::sIssueStock()
{
  bool update = FALSE;
  QList<XTreeWidgetItem *> selected = _soitem->selectedItems();
  for (int i = 0; i < selected.size(); i++)
  {
    XTreeWidgetItem *soitem = (XTreeWidgetItem *)(selected[i]);
    if (soitem->altId() != 1 && soitem->altId() != 4)
    {
      ParameterList params;
      params.append("soitem_id", soitem->id());

      if (_requireInventory->isChecked())
        params.append("requireInventory");

      issueLineToShipping newdlg(this, "", TRUE);
      newdlg.set(params);
      if (newdlg.exec() != XDialog::Rejected)
        update = TRUE;
    }
  }

  if (update)
    sFillItemList();
}

void salesOrder::sIssueLineBalance()
{
  bool job = false;
  QList<XTreeWidgetItem *> selected = _soitem->selectedItems();
  for (int i = 0; i < selected.size(); i++)
  {
    XTreeWidgetItem *soitem = (XTreeWidgetItem *)(selected[i]);
    if (soitem->altId() != 1 && soitem->altId() != 4)
    {
      if (_requireInventory->isChecked())
      {
        q.prepare("SELECT itemsite_id, item_number, warehous_code, itemsite_costmethod, "
                  "       (roundQty(item_fractional, noNeg(coitem_qtyord - coitem_qtyshipped + coitem_qtyreturned - "
                  "          ( SELECT COALESCE(SUM(coship_qty), 0) "
                  "              FROM coship, cosmisc "
                  "             WHERE ( (coship_coitem_id=coitem_id) "
                  "               AND   (coship_cosmisc_id=cosmisc_id) "
                  "               AND   (NOT cosmisc_shipped) ) ) ) ) <= itemsite_qtyonhand) AS isqtyavail "
                  "  FROM coitem, itemsite, item, warehous "
                  " WHERE ((coitem_itemsite_id=itemsite_id) "
                  "   AND (itemsite_item_id=item_id) "
                  "   AND (itemsite_warehous_id=warehous_id) "
                  "   AND (coitem_id=:soitem_id)); ");
        q.bindValue(":soitem_id", soitem->id());
        q.exec();
        while (q.next())
        {
          if (q.value("itemsite_costmethod").toString() == "J")
            job = true;

          if (!(q.value("isqtyavail").toBool()) && q.value("itemsite_costmethod").toString() != "J")
          {
            QMessageBox::critical(this, tr("Insufficient Inventory"),
                                  tr("<p>There is not enough Inventory to issue the amount required"
                                       " of Item %1 in Site %2.")
                                  .arg(q.value("item_number").toString())
                                  .arg(q.value("warehous_code").toString()) );
            return;
          }
        }
      }

      q.prepare("SELECT itemsite_id, itemsite_costmethod, item_number, warehous_code, "
                "       (COALESCE((SELECT SUM(itemloc_qty) "
                "                    FROM itemloc "
                "                   WHERE (itemloc_itemsite_id=itemsite_id)), 0.0) >= roundQty(item_fractional, "
                "                          noNeg( coitem_qtyord - coitem_qtyshipped + coitem_qtyreturned - "
                "                          ( SELECT COALESCE(SUM(coship_qty), 0) "
                "                          FROM coship, cosmisc "
                "                          WHERE ( (coship_coitem_id=coitem_id) "
                "                          AND (coship_cosmisc_id=cosmisc_id) "
                "                          AND (NOT cosmisc_shipped) ) ) ) "
                "                         )) AS isqtyavail "
                "  FROM coitem, itemsite, item, warehous "
                " WHERE ((coitem_itemsite_id=itemsite_id) "
                "   AND (itemsite_item_id=item_id) "
                "   AND (itemsite_warehous_id=warehous_id) "
                "   AND (NOT ((item_type = 'R') OR (itemsite_controlmethod = 'N'))) "
                "   AND ((itemsite_controlmethod IN ('L', 'S')) OR (itemsite_loccntrl)) "
                "   AND (coitem_id=:soitem_id)); ");
      q.bindValue(":soitem_id", soitem->id());
      q.exec();
      while (q.next())
      {
        if (!q.value("isqtyavail").toBool() && q.value("itemsite_costmethod").toString() != "J")
        {
          QMessageBox::critical(this, tr("Insufficient Inventory"),
                                  tr("<p>Item Number %1 in Site %2 is a Multiple Location or "
                                     "Lot/Serial controlled Item which is short on Inventory. "
                                     "This transaction cannot be completed as is. Please make "
                                     "sure there is sufficient Quantity on Hand before proceeding.")
                                .arg(q.value("item_number").toString())
                                .arg(q.value("warehous_code").toString()));
          return;
        }
      }

      int       invhistid      = 0;
      int       itemlocSeries  = 0;
      XSqlQuery rollback;
      rollback.prepare("ROLLBACK;");

      q.exec("BEGIN;"); // because of possible lot, serial, or location distribution cancelations
      // If this is a lot/serial controlled job item, we need to post production first
      if (job)
      {
        XSqlQuery prod;
        prod.prepare("SELECT postSoItemProduction(:soitem_id, now()) AS result;");
        prod.bindValue(":soitem_id", _soitem->id());
        prod.exec();
        if (prod.first())
        {
          itemlocSeries = prod.value("result").toInt();

          if (itemlocSeries < 0)
          {
            rollback.exec();
                      systemError(this, storedProcErrorLookup("postProduction", itemlocSeries),
                        __FILE__, __LINE__);
            return;
          }
          else if (distributeInventory::SeriesAdjust(itemlocSeries, this) == XDialog::Rejected)
          {
            rollback.exec();
            QMessageBox::information( this, tr("Issue to Shipping"), tr("Issue Canceled") );
            return;
          }

          // Need to get the inventory history id so we can auto reverse the distribution when issuing
          prod.prepare("SELECT invhist_id "
                       "FROM invhist "
                       "WHERE ((invhist_series = :itemlocseries) "
                       " AND (invhist_transtype = 'RM')); ");
          prod.bindValue(":itemlocseries", itemlocSeries);
          prod.exec();
          if (prod.first())
            invhistid = prod.value("invhist_id").toInt();
          else
          {
            rollback.exec();
                      systemError(this, tr("Inventory history not found"),
                        __FILE__, __LINE__);
            return;
          }
        }
      }

      q.prepare("SELECT issueLineBalanceToShipping('SO', :soitem_id, now(), :itemlocseries, :invhist_id) AS result;");
      ;
      q.bindValue(":soitem_id", soitem->id());
      if (invhistid)
        q.bindValue(":invhist_id", invhistid);
      if (itemlocSeries)
        q.bindValue(":itemlocseries", itemlocSeries);
      q.exec();
      if (q.first())
      {
        int result = q.value("result").toInt();
        if (result < 0)
        {
          rollback.exec();
                      systemError(this, storedProcErrorLookup("issueLineBalanceToShipping", result) +
                      tr("<br>Line Item %1").arg(selected[i]->text(0)),
                      __FILE__, __LINE__);
          return;
        }

        if (distributeInventory::SeriesAdjust(q.value("result").toInt(), this) == XDialog::Rejected)
        {
          rollback.exec();
          QMessageBox::information( this, tr("Issue to Shipping"), tr("Transaction Canceled") );
          return;
        }

        q.exec("COMMIT;");
      }
      else
      {
        rollback.exec();
        systemError(this, tr("Line Item %1\n").arg(selected[i]->text(0)) +
                    q.lastError().databaseText(), __FILE__, __LINE__);
        return;
      }
    }
  }

  sFillItemList();
}

void salesOrder::sFreightChanged()
{
  if (_freight->localValue() == _freightCache)
    return;

  if (_freight->isEnabled())
  {
    if (_calcfreight)
    {
      int answer;
      answer = QMessageBox::question(this, tr("Manual Freight?"),
                                     tr("<p>Manually editing the freight will disable "
                                          "automatic Freight recalculations.  Are you "
                                          "sure you want to do this?"),
                                     QMessageBox::Yes,
                                     QMessageBox::No | QMessageBox::Default);
      if (answer == QMessageBox::Yes)
        _calcfreight = false;
      else
      {
        disconnect(_freight, SIGNAL(valueChanged()), this, SLOT(sFreightChanged()));
        _freight->setLocalValue(_freightCache);
        connect(_freight, SIGNAL(valueChanged()), this, SLOT(sFreightChanged()));
      }
    }
    else if ( (!_calcfreight) &&
              (_freight->localValue() == 0) &&
              (_metrics->boolean("CalculateFreight")))
    {
      int answer;
      answer = QMessageBox::question(this, tr("Automatic Freight?"),
                                     tr("<p>Manually clearing the freight will enable "
                                          "automatic Freight recalculations.  Are you "
                                          "sure you want to do this?"),
                                     QMessageBox::Yes,
                                     QMessageBox::No | QMessageBox::Default);
      if (answer == QMessageBox::Yes)
      {
        _calcfreight = true;
        disconnect(_freight, SIGNAL(valueChanged()), this, SLOT(sFreightChanged()));
        _freight->setLocalValue(_freightCache);
        connect(_freight, SIGNAL(valueChanged()), this, SLOT(sFreightChanged()));
      }
    }
    else
      _freightCache = _freight->localValue();
  }

  save(true);
  sCalculateTax();
}

void salesOrder::sCalculateTax()
{
  XSqlQuery taxq;
  taxq.prepare( "SELECT SUM(tax) AS tax "
                "FROM ("
                "SELECT ROUND(SUM(taxdetail_tax),2) AS tax "
                "FROM tax "
                " JOIN calculateTaxDetailSummary(:type, :cohead_id, 'T') ON (taxdetail_tax_id=tax_id)"
                "GROUP BY tax_id) AS data;" );

  taxq.bindValue(":cohead_id", _soheadid);
  if (ISQUOTE(_mode))
    taxq.bindValue(":type","Q");
  else
    taxq.bindValue(":type","S");
  taxq.exec();
  if (taxq.first())
    _tax->setLocalValue(taxq.value("tax").toDouble());
  else if (taxq.lastError().type() != QSqlError::NoError)
  {
    systemError(this, taxq.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
  sCalculateTotal();
}

void salesOrder::sTaxZoneChanged()
{
  if (_taxZone->id() != _taxzoneidCache && _saved)
    save(true);

  sCalculateTax();
  _taxzoneidCache=_taxZone->id();
}

void salesOrder::sReserveStock()
{
  QList<XTreeWidgetItem *> selected = _soitem->selectedItems();
  for (int i = 0; i < selected.size(); i++)
  {
    ParameterList params;
    params.append("soitem_id", ((XTreeWidgetItem *)(selected[i]))->id());

    reserveSalesOrderItem newdlg(this, "", true);
    newdlg.set(params);
    newdlg.exec();
  }

  sFillItemList();
}

void salesOrder::sReserveLineBalance()
{
  q.prepare("SELECT reserveSoLineBalance(:soitem_id) AS result;");
  QList<XTreeWidgetItem *> selected = _soitem->selectedItems();
  for (int i = 0; i < selected.size(); i++)
  {
    q.bindValue(":soitem_id", ((XTreeWidgetItem *)(selected[i]))->id());
    q.exec();
    if (q.first())
    {
      int result = q.value("result").toInt();
      if (result < 0)
      {
        systemError(this, storedProcErrorLookup("reserveSoLineBalance", result) +
                    tr("<br>Line Item %1").arg(selected[i]->text(0)),
                    __FILE__, __LINE__);
        return;
      }
    }
    else if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, tr("Line Item %1\n").arg(selected[i]->text(0)) +
                  q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }

  sFillItemList();
}

void salesOrder::sUnreserveStock()
{
  q.prepare("SELECT unreserveSoLineQty(:soitem_id) AS result;");
  QList<XTreeWidgetItem *> selected = _soitem->selectedItems();
  for (int i = 0; i < selected.size(); i++)
  {
    q.bindValue(":soitem_id", ((XTreeWidgetItem *)(selected[i]))->id());
    q.exec();
    if (q.first())
    {
      int result = q.value("result").toInt();
      if (result < 0)
      {
        systemError(this, storedProcErrorLookup("unreservedSoLineQty", result) +
                    tr("<br>Line Item %1").arg(selected[i]->text(0)),
                    __FILE__, __LINE__);
        return;
      }
    }
    else if (q.lastError().type() != QSqlError::NoError)
    {
        systemError(this, tr("Line Item %1\n").arg(selected[i]->text(0)) +
                  q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }
}

void salesOrder::sShowReservations()
{
  QList<XTreeWidgetItem *> selected = _soitem->selectedItems();
  for (int i = 0; i < selected.size(); i++)
  {
    ParameterList params;
    params.append("soitem_id", ((XTreeWidgetItem *)(selected[i]))->id());
    params.append("run");

    dspReservations *newdlg = new dspReservations();
    newdlg->set(params);
    omfgThis->handleNewWindow(newdlg);
  }
}

void salesOrder::sCreditAllocate()
{
  ParameterList params;
  params.append("doctype", "S");
  params.append("cohead_id", _soheadid);
  params.append("cust_id", _cust->id());
  params.append("total",  _total->localValue());
  params.append("balance",  _balance->localValue());
  params.append("curr_id",   _balance->id());
  params.append("effective", _balance->effective());

  allocateARCreditMemo newdlg(this, "", TRUE);
  if (newdlg.set(params) == NoError && newdlg.exec() == XDialog::Accepted)
  {
    populateCMInfo();
  }
}

void salesOrder::sAllocateCreditMemos()
{
  // Determine the balance I need to select
  // This is the same as in sCalculateTotal except that the Unallocated amount is not included.
  double  balance      = (_subtotal->localValue() + _tax->localValue() + _miscCharge->localValue() + _freight->localValue())-
                         _allocatedCM->localValue() - _authCC->localValue();
  double  initBalance  = balance;
  if (balance > 0)
  {
    // Get the list of Unallocated CM's with amount
    q.prepare("SELECT aropen_id,"
              "       noNeg(aropen_amount - aropen_paid - SUM(COALESCE(aropenalloc_amount,0))) AS amount,"
              "       currToCurr(aropen_curr_id, :curr_id,"
              "                  noNeg(aropen_amount - aropen_paid - SUM(COALESCE(aropenalloc_amount,0))), :effective) AS amount_cocurr"
              "  FROM cohead, aropen LEFT OUTER JOIN aropenalloc ON (aropenalloc_aropen_id=aropen_id)"
              " WHERE ( (aropen_cust_id=cohead_cust_id)"
              "   AND   (aropen_doctype IN ('C', 'R'))"
              "   AND   (aropen_open)"
              "   AND   (cohead_id=:cohead_id) )"
              " GROUP BY aropen_id, aropen_duedate, aropen_amount, aropen_paid, aropen_curr_id "
              "HAVING (noNeg(aropen_amount - aropen_paid - SUM(COALESCE(aropenalloc_amount,0))) > 0)"
              " ORDER BY aropen_duedate; ");
    q.bindValue(":cohead_id", _soheadid);
    q.bindValue(":curr_id",   _balance->id());
    q.bindValue(":effective", _balance->effective());
    q.exec();
    if (q.lastError().type() != QSqlError::NoError)
    {
        systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }

    double    amount     = 0.0;
    double    initAmount = 0.0;
    XSqlQuery allocCM;
    allocCM.prepare("INSERT INTO aropenalloc"
                    "      (aropenalloc_aropen_id, aropenalloc_doctype, aropenalloc_doc_id, "
                    "       aropenalloc_amount, aropenalloc_curr_id)"
                    "VALUES(:aropen_id, 'S', :doc_id, :amount, :curr_id);");

    while (balance > 0.0 && q.next())
    {
      amount     = q.value("amount").toDouble();
      initAmount = _outstandingCM->localValue();

      if (amount <= 0.0)  // if this credit memo does not have a positive value just ignore it
        continue;

      if (amount > balance) // make sure we don't apply more to a credit memo than we have left.
        amount = balance;
      // apply credit memo's to this sales order until the balance is 0.
      allocCM.bindValue(":doc_id", _soheadid);
      allocCM.bindValue(":aropen_id", q.value("aropen_id").toInt());
      allocCM.bindValue(":amount", amount);
      allocCM.bindValue(":curr_id", _balance->id());
      allocCM.exec();
      if (allocCM.lastError().type() == QSqlError::NoError)
        balance -= amount;
      else
        systemError(this, allocCM.lastError().databaseText(), __FILE__, __LINE__);
    }
    _outstandingCM->setLocalValue(initAmount-(initBalance-balance));
    _balance->setLocalValue(initBalance-(initBalance-balance));
    _allocatedCM->setLocalValue(initBalance-balance);
  }
}

void salesOrder::sCheckValidContacts()
{
  if (_shipToCntct->isValid())
    _shipToCntct->setEnabled(true);
  else
    _shipToCntct->setEnabled(false);

  if (_billToCntct->isValid())
    _billToCntct->setEnabled(true);
  else
    _billToCntct->setEnabled(false);
}

void salesOrder::sHandleMore()
{
  _warehouse->setVisible(_more->isChecked());
  _shippingWhseLit->setVisible(_more->isChecked());
  _originatedByLit->setVisible(_more->isChecked());
  _origin->setVisible(_more->isChecked());
  _commissionLit->setVisible(_more->isChecked());
  _commission->setVisible(_more->isChecked());
  _commissionPrcntLit->setVisible(_more->isChecked());
  _taxZoneLit->setVisible(_more->isChecked());
  _taxZone->setVisible(_more->isChecked());
  _shipDateLit->setVisible(_more->isChecked());
  _shipDate->setVisible(_more->isChecked());
  _packDateLit->setVisible(_more->isChecked());
  _packDate->setVisible(_more->isChecked());

  if (ISORDER(_mode))
  {
    _shippingCharges->setVisible(_more->isChecked());
    _shippingChargesLit->setVisible(_more->isChecked());
    _shippingForm->setVisible(_more->isChecked());
    _shippingFormLit->setVisible(_more->isChecked());
  }
  else
  {
    _expireLit->setVisible(_more->isChecked());
    _expire->setVisible(_more->isChecked());
  }
}

void salesOrder::sRecalculatePrice()
{
  if (QMessageBox::question(this, tr("Update all prices?"),
                            tr("Do you want to recalculate all prices for the order including:\n\t- Line items\n\t - Taxes\n\t - Freight ?"),
                            QMessageBox::Yes | QMessageBox::Escape,
                            QMessageBox::No | QMessageBox::Default) == QMessageBox::Yes)
  {
    ParameterList params;
    QString       sql;
    QString       sqlchk;
    if (ISORDER(_mode))
    {
      sql ="UPDATE coitem SET coitem_price=itemprice(item_id, "
           "cohead_cust_id, <? value(\"shipto_id\") ?>, coitem_qtyord, "
           "coitem_qty_uom_id, coitem_price_uom_id, "
           "cohead_curr_id,cohead_orderdate, "
           "<? if exists(\"UseSchedDate\") ?>"
             " coitem_scheddate "
             "<? else ?> "
               "<? value(\"asOf\") ?>"
                 "<? endif ?> "
                   "), "
                   "coitem_custprice=itemprice(item_id, cohead_cust_id, "
                   "<? value(\"shipto_id\") ?>,coitem_qtyord, coitem_qty_uom_id, "
                     "coitem_price_uom_id, cohead_curr_id, cohead_orderdate, "
                     "<? if exists(\"UseSchedDate\") ?>"
                       " coitem_scheddate "
                       "<? else ?> "
                         "<? value(\"asOf\") ?>"
                           "<? endif ?> "
                             ") "
                             "FROM cohead, item, itemsite "
                             "WHERE ( (coitem_status NOT IN ('C','X')) "
                             "AND (NOT coitem_firm) "
                             "<? if exists(\"ignoreDiscounts\") ?>"
                               "AND (coitem_price = coitem_custprice) "
                               "<? endif ?>"
                                 "AND (itemsite_id=coitem_itemsite_id) "
                                 "AND (itemsite_item_id=item_id) "
                                 "AND (coitem_cohead_id=cohead_id) "
                                 "AND (cohead_id=<? value(\"cohead_id\") ?>) );";
      sqlchk ="SELECT MIN(itemprice(item_id, cohead_cust_id, "
              "                     <? value(\"shipto_id\") ?>, coitem_qtyord, "
              "                     coitem_qty_uom_id, coitem_price_uom_id, "
              "                     cohead_curr_id,cohead_orderdate, "
              "                     <? if exists(\"UseSchedDate\") ?> coitem_scheddate "
              "                     <? else ?> <? value(\"asOf\") ?>"
              "                     <? endif ?>)) AS pricechk "
              "FROM cohead, coitem, item, itemsite "
              "WHERE ( (coitem_cohead_id=cohead_id) "
              "  AND   (coitem_status NOT IN ('C','X')) "
              "  AND   (NOT coitem_firm) "
              "<? if exists(\"ignoreDiscounts\") ?>"
              "  AND   (coitem_price = coitem_custprice) "
              "<? endif ?>"
              "  AND   (itemsite_id=coitem_itemsite_id) "
              "  AND   (itemsite_item_id=item_id) "
              "  AND   (coitem_cohead_id=cohead_id) "
              "  AND   (cohead_id=<? value(\"cohead_id\") ?>) );";
    }
    else
    {
      sql = "UPDATE quitem SET quitem_price=itemprice(item_id, "
            "quhead_cust_id, <? value(\"shipto_id\") ?>, quitem_qtyord, "
            "quitem_qty_uom_id, quitem_price_uom_id, "
            "quhead_curr_id,quhead_quotedate, "
            "<? if exists(\"UseSchedDate\") ?>"
              " quitem_scheddate "
              "<? else ?> "
                "<? value(\"asOf\") ?>"
                  "<? endif ?> "
                    "), "
                    "quitem_custprice=itemprice(item_id, quhead_cust_id, "
                    "<? value(\"shipto_id\") ?>,quitem_qtyord, quitem_qty_uom_id, "
                      "quitem_price_uom_id, quhead_curr_id, quhead_quotedate, "
                      "<? if exists(\"UseSchedDate\") ?>"
                        " quitem_scheddate "
                        "<? else ?> "
                          "<? value(\"asOf\") ?>"
                            "<? endif ?> "
                              ") "
                              "FROM quhead, item, itemsite "
                              "WHERE ( (itemsite_id=quitem_itemsite_id) "
                              "<? if exists(\"ignoreDiscounts\") ?>"
                                "AND (quitem_price = quitem_custprice) "
                                "<? endif ?>"
                                  "AND (itemsite_item_id=item_id) "
                                  "AND (quitem_quhead_id=quhead_id) "
                                  "AND (quhead_id=<? value(\"cohead_id\") ?>) );";
      sqlchk ="SELECT MIN(itemprice(item_id, quhead_cust_id, "
              "                     <? value(\"shipto_id\") ?>, quitem_qtyord, "
              "                     quitem_qty_uom_id, quitem_price_uom_id, "
              "                     quhead_curr_id,quhead_quotedate, "
              "                     <? if exists(\"UseSchedDate\") ?> quitem_scheddate "
              "                     <? else ?> <? value(\"asOf\") ?>"
              "                     <? endif ?>)) AS pricechk "
              "FROM quhead, quitem, item, itemsite "
              "WHERE ( (quitem_quhead_id=quhead_id) "
              "  AND   (itemsite_id=quitem_itemsite_id) "
              "<? if exists(\"ignoreDiscounts\") ?>"
              "  AND   (quitem_price = quitem_custprice) "
              "<? endif ?>"
              "  AND   (itemsite_item_id=item_id) "
              "  AND   (quitem_quhead_id=quhead_id) "
              "  AND   (quhead_id=<? value(\"cohead_id\") ?>) );";
    }
    params.append("cohead_id", _soheadid);
    params.append("shipto_id", _shipTo->id());
    if (_metrics->boolean("IgnoreCustDisc"))
      params.append("ignoreDiscounts", true);
    if (_metrics->value("soPriceEffective") == "ScheduleDate")
      params.append("UseSchedDate", true);
    MetaSQLQuery mql(sql);
    MetaSQLQuery mqlchk(sqlchk);
    if (_metrics->value("soPriceEffective") == "OrderDate")
    {
      if (!_orderDate->isValid())
      {
        QMessageBox::critical(this,tr("Order Date Required"),tr("Prices can not be recalculated without a valid Order Date."));
        _orderDate->setFocus();
        return;
      }
      params.append("asOf", _orderDate->date());
    }
    else if (_metrics->value("soPriceEffective") == "ScheduleDate")
    {
      if (!_orderDate->isValid())
      {
        QMessageBox::critical(this,tr("Schedule Date Required"),tr("Prices can not be recalculated without a valid Schedule Date."));
        _shipDate->setFocus();
        return;
      }
      params.append("asOf", _shipDate->date());
    }
    else
      params.append("asOf", omfgThis->dbDate());

    XSqlQuery itempricechk = mqlchk.toQuery(params);
    if (itempricechk.first())
    {
      if (itempricechk.value("pricechk").toDouble() == -9999.0)
      {
        // User expected an update, so let them know and reset
        QMessageBox::critical(this, tr("Customer Cannot Buy at Quantity"),
                              tr("<p>One or more items are marked as exclusive and "
                                   "no qualifying price schedule was found. " ) );
        return;
      }
    }
    else if (itempricechk.lastError().type() != QSqlError::NoError)
    {
      systemError(this, itempricechk.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }

    XSqlQuery setitemprice = mql.toQuery(params);
    if (setitemprice.lastError().type() != QSqlError::NoError)
    {
      systemError(this, setitemprice.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }

    _calcfreight = _metrics->boolean("CalculateFreight");
    sFillItemList();
  }
}

void salesOrder::sOrderDateChanged()
{
  if (_orderDate->date() == _orderDateCache ||
      !_orderDate->isValid())
    return;
  else if (!_soitem->topLevelItemCount() ||
           !_orderDateCache.isValid() ||
           _metrics->value("soPriceEffective") != "OrderDate")
  {
    _orderDateCache = _orderDate->date();
    return;
  }

  sRecalculatePrice();
  _orderDateCache = _orderDate->date();
  sFillItemList();
  omfgThis->sSalesOrdersUpdated(_soheadid);
}

void salesOrder::sShipDateChanged()
{
  if (_shipDate->date() == _shipDateCache ||
      !_shipDate->isValid())
    return;
  else if (!_soitem->topLevelItemCount())
  {
    _shipDateCache = _shipDate->date();
    return;
  }

  QString       sql;
  XSqlQuery     upd;
  ParameterList params;
  params.append("cohead_id", _soheadid);
  params.append("newDate", _shipDate->date());
  params.append("ignoreCustDisc", _metrics->value("IgnoreCustDisc"));

  if (QMessageBox::question(this, tr("Update all schedule dates?"),
                                  tr("Changing this date will update the Schedule Date on all editable line items. "
                               "Is this what you want to do?"),
                            QMessageBox::Yes | QMessageBox::Default,
                            QMessageBox::No | QMessageBox::Escape) == QMessageBox::Yes)
  {
    // Validate first
    if (ISORDER(_mode))
    {
      sql = "SELECT DISTINCT valid FROM ( "
            "  SELECT customerCanPurchase(itemsite_item_id, cohead_cust_id, "
            "                             cohead_shipto_id, <? value(\"newDate\") ?>) AS valid "
            "  FROM cohead "
            "   JOIN coitem ON (cohead_id=coitem_cohead_id) "
            "   JOIN itemsite ON (coitem_itemsite_id=itemsite_id) "
            "   WHERE ( (cohead_id=<? value(\"cohead_id\") ?>) "
            "   AND (coitem_status NOT IN ('C','X')) )"
            ") data "
            "ORDER BY valid; ";
    }
    else
    {
      sql = "SELECT DISTINCT valid FROM ( "
            "  SELECT customerCanPurchase(itemsite_item_id, quhead_cust_id, "
            "                             quhead_shipto_id, <? value(\"newDate\") ?>) AS valid "
            "  FROM quhead "
            "   JOIN quitem ON (quhead_id=quitem_quhead_id) "
            "   JOIN itemsite ON (quitem_itemsite_id=itemsite_id) "
            "   WHERE (quhead_id=<? value(\"cohead_id\") ?>) "
            ") data "
            "ORDER BY valid; ";
    }

    MetaSQLQuery vmql(sql);
    upd = vmql.toQuery(params);
    if (upd.first())
    {
      if (upd.size() == 2)  // Both valid and invalid records
      {
        if (QMessageBox::warning(this, tr("Can not reschedule all Items"),
                                  tr("Some exclusive items may not be rescheduled because there is no "
                                    "valid price schedule for the date entered.  Proceed rescheduling "
                                    "only qualifying items?"),
                                 QMessageBox::Yes | QMessageBox::Default,
                                 QMessageBox::No | QMessageBox::Escape) == QMessageBox::No)
        {
          _shipDate->setDate(_shipDateCache);
          return;
        }
      }
      else if (!upd.value("valid").toBool())  // No valid items
      {
        QMessageBox::warning(this, tr("Can not reschedule Items"),
                                  tr("No Items can be rescheduled because there are no "
                                "valid price schedules for the date entered."));
        _shipDate->setDate(_shipDateCache);
        return;
      }
    }
    else if (upd.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      _shipDate->setDate(_shipDateCache);
      return;
    }

    // Now execute
    if (ISORDER(_mode))
    {
      sql = "UPDATE coitem SET coitem_scheddate = <? value(\"newDate\") ?> "
            "FROM cohead,item,itemsite "
            "WHERE ( (coitem_status NOT IN ('C','X'))"
            "  AND (NOT coitem_firm)"
            "  AND (itemsite_id=coitem_itemsite_id) "
            "  AND (itemsite_item_id=item_id) "
            "  AND (cohead_id=<? value(\"cohead_id\") ?>) "
            "  AND (coitem_cohead_id=cohead_id) "
            "  AND (customerCanPurchase(itemsite_item_id, cohead_cust_id, cohead_shipto_id, <? value(\"newDate\") ?>) ) );";
      if (QMessageBox::question(this, tr("Reschedule Work Order?"),
                                tr("<p>Should any associated W/O's "
                                   "be rescheduled to reflect this change?"),
                                QMessageBox::Yes | QMessageBox::Default,
                                QMessageBox::No | QMessageBox::Escape) == QMessageBox::Yes)
      {
        sql = sql +
              "SELECT changeWoDates(wo_id, "
              "                     wo_startdate + (<? value(\"newDate\") ?> - wo_duedate),"
              "                     <? value(\"newDate\") ?>, TRUE) AS result "
              "FROM cohead JOIN coitem ON (coitem_cohead_id=cohead_id AND coitem_order_type='W') "
              "            JOIN wo ON (wo_id=coitem_order_id) "
              "            JOIN itemsite ON (itemsite_id=coitem_itemsite_id) "
              "WHERE ( (coitem_status NOT IN ('C','X'))"
              "  AND (NOT coitem_firm)"
              "  AND (cohead_id=<? value(\"cohead_id\") ?>) "
              "  AND (customerCanPurchase(itemsite_item_id, cohead_cust_id, cohead_shipto_id, <? value(\"newDate\") ?>) ) );";
      }
    }
    else
    {
      sql = "UPDATE quitem SET quitem_scheddate = <? value(\"newDate\") ?> "
            "FROM quhead,item,itemsite "
            "WHERE ( (itemsite_id=quitem_itemsite_id) "
            "  AND (itemsite_item_id=item_id) "
            "  AND (quhead_id=<? value(\"cohead_id\") ?>) "
            "  AND (quitem_quhead_id=quhead_id) "
            "  AND (customerCanPurchase(itemsite_item_id, quhead_cust_id, quhead_shipto_id, <? value(\"newDate\") ?>) ) );";
    }

    MetaSQLQuery mql(sql);
    upd = mql.toQuery(params);
    if (upd.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      _shipDate->setDate(_shipDateCache);
      return;
    }
  }
  else
  {
    _shipDate->setDate(_shipDateCache);
    return;
  }

  if (_metrics->value("soPriceEffective") == "ScheduleDate")
    sRecalculatePrice();
  _shipDateCache = _shipDate->date();
  sFillItemList();
  omfgThis->sSalesOrdersUpdated(_soheadid);
}

void salesOrder::sViewWO()
{
  QList<XTreeWidgetItem *> selected = _soitem->selectedItems();
  for (int i = 0; i < selected.size(); i++)
  {
    XSqlQuery wo;
    wo.prepare("SELECT coitem_order_id FROM coitem WHERE (coitem_id=:soitem_id);");
    wo.bindValue(":soitem_id", ((XTreeWidgetItem *)(selected[i]))->id());
    wo.exec();
    if (wo.first())
    {
      ParameterList params;
      params.append("wo_id",  wo.value("coitem_order_id").toInt());
      params.append("mode", "view");

      workOrder *newdlg = new workOrder();
      newdlg->set(params);
      omfgThis->handleNewWindow(newdlg);
    }
    else if (wo.lastError().type() != QSqlError::NoError)
    {
      systemError(this, wo.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }
}

void salesOrder::sMaintainWO()
{
  QList<XTreeWidgetItem *> selected = _soitem->selectedItems();
  for (int i = 0; i < selected.size(); i++)
  {
    XSqlQuery wo;
    wo.prepare("SELECT coitem_order_id FROM coitem WHERE (coitem_id=:soitem_id);");
    wo.bindValue(":soitem_id", ((XTreeWidgetItem *)(selected[i]))->id());
    wo.exec();
    if (wo.first())
    {
      ParameterList params;
      params.append("wo_id",  wo.value("coitem_order_id").toInt());
      params.append("mode", "edit");

      workOrder *newdlg = new workOrder();
      newdlg->set(params);
      omfgThis->handleNewWindow(newdlg);
    }
    else if (wo.lastError().type() != QSqlError::NoError)
    {
      systemError(this, wo.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }
}

void salesOrder::sViewPO()
{
  QList<XTreeWidgetItem *> selected = _soitem->selectedItems();
  for (int i = 0; i < selected.size(); i++)
  {
    XSqlQuery po;
    po.prepare("SELECT poitem_pohead_id "
               "FROM coitem JOIN poitem ON (coitem_order_id = poitem_id) "
               "WHERE (coitem_id=:soitem_id);");
    po.bindValue(":soitem_id", ((XTreeWidgetItem *)(selected[i]))->id());
    po.exec();
    if (po.first())
    {
      ParameterList params;
      params.append("pohead_id", po.value("poitem_pohead_id").toInt());
      params.append("mode", "view");

      purchaseOrder *newdlg = new purchaseOrder();
      newdlg->set(params);
      omfgThis->handleNewWindow(newdlg);
    }
    else if (po.lastError().type() != QSqlError::NoError)
    {
      systemError(this, po.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }
}

void salesOrder::sMaintainPO()
{
  QList<XTreeWidgetItem *> selected = _soitem->selectedItems();
  for (int i = 0; i < selected.size(); i++)
  {
    XSqlQuery po;
    po.prepare("SELECT poitem_pohead_id "
               "FROM coitem JOIN poitem ON (coitem_order_id = poitem_id) "
               "WHERE (coitem_id=:soitem_id);");
    po.bindValue(":soitem_id", ((XTreeWidgetItem *)(selected[i]))->id());
    po.exec();
    if (po.first())
    {
      ParameterList params;
      params.append("pohead_id", po.value("poitem_pohead_id").toInt());
      params.append("mode", "edit");

      purchaseOrder *newdlg = new purchaseOrder();
      newdlg->set(params);
      omfgThis->handleNewWindow(newdlg);
    }
    else if (po.lastError().type() != QSqlError::NoError)
    {
      systemError(this, po.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }
}

void salesOrder::sReleasePR()
{
  QList<XTreeWidgetItem *> selected = _soitem->selectedItems();
  for (int i = 0; i < selected.size(); i++)
  {
    XSqlQuery pr;
    pr.prepare("SELECT coitem_order_id FROM coitem WHERE (coitem_id=:soitem_id);");
    pr.bindValue(":soitem_id", ((XTreeWidgetItem *)(selected[i]))->id());
    pr.exec();
    if (pr.first())
    {
      ParameterList params;
      params.append("mode", "releasePr");
      params.append("pr_id", pr.value("coitem_order_id").toInt());
      purchaseOrder *newdlg = new purchaseOrder();
      if (newdlg->set(params) == NoError)
        omfgThis->handleNewWindow(newdlg);
      else
        delete newdlg;
    }
    else if (pr.lastError().type() != QSqlError::NoError)
    {
      systemError(this, pr.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }
}

void salesOrder::sViewPR()
{
  QList<XTreeWidgetItem *> selected = _soitem->selectedItems();
  for (int i = 0; i < selected.size(); i++)
  {
    XSqlQuery pr;
    pr.prepare("SELECT pr_itemsite_id, pr_qtyreq, pr_duedate, pr_order_id, coitem_order_id "
               "FROM pr JOIN coitem ON (pr_id = coitem_order_id) "
               "WHERE (coitem_id=:soitem_id);");
    pr.bindValue(":soitem_id", ((XTreeWidgetItem *)(selected[i]))->id());
    pr.exec();
    if (pr.first())
    {
      ParameterList params;
      params.append("mode", "view");
      params.append("pr_id", pr.value("coitem_order_id").toInt());
      purchaseRequest *newdlg = new purchaseRequest();
      if (newdlg->set(params) == NoError)
        omfgThis->handleNewWindow(newdlg);
      else
        delete newdlg;
    }
    else if (pr.lastError().type() != QSqlError::NoError)
    {
      systemError(this, pr.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }
}
