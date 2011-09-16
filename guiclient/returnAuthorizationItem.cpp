/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
*/

#include "returnAuthorizationItem.h"

#include <QMessageBox>
#include <QSqlError>
#include <QValidator>
#include <QVariant>

#include "priceList.h"
#include "taxDetail.h"
#include "storedProcErrorLookup.h"
#include "returnAuthItemLotSerial.h"

returnAuthorizationItem::returnAuthorizationItem(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

#ifndef Q_WS_MAC
  _listPrices->setMaximumWidth(25);
  _saleListPrices->setMaximumWidth(25);
#endif

  _mode = cNew;
  _raitemid = -1;
  _raheadid = -1;
  _custid = -1;
  _priceRatio = 1.0;
  _qtyAuthCache = 0.0;
  _shiptoid = -1;
  _taxzoneid = -1;
  _qtyinvuomratio = 1.0;
  _priceinvuomratio = 1.0;
  _invuomid = -1;
  _orderId = -1;
  _preferredWarehousid = -1;
  _preferredShipWarehousid = -1;
  _status = "O";
  _creditmethod = "";
  _qtycredited = 0;
  _soldQty = 0;
  _coitemid = -1;
  _crmacctid = -1;
  _unitcost = 0;
  _origsoid = -1;
  _costmethod = ""; 
  _coitemitemsiteid = -1;

  connect(_discountFromSale,     SIGNAL(lostFocus()),                    this, SLOT(sCalculateFromDiscount()));
  connect(_saleDiscountFromSale, SIGNAL(lostFocus()),                    this, SLOT(sCalculateSaleFromDiscount()));
  connect(_extendedPrice,        SIGNAL(valueChanged()),                 this, SLOT(sCalculateTax()));
  connect(_item,                 SIGNAL(newId(int)),                     this, SLOT(sPopulateItemInfo()));
  connect(_item,                 SIGNAL(warehouseIdChanged(int)),        this, SLOT(sPopulateItemsiteInfo()));
  connect(_shipWhs,              SIGNAL(newID(int)),                     this, SLOT(sPopulateItemsiteInfo()));
  connect(_listPrices,           SIGNAL(clicked()),                      this, SLOT(sListPrices()));
  connect(_saleListPrices,       SIGNAL(clicked()),                      this, SLOT(sSaleListPrices()));
  connect(_netUnitPrice,         SIGNAL(valueChanged()),                 this, SLOT(sCalculateDiscountPrcnt()));
  connect(_netUnitPrice,         SIGNAL(valueChanged()),                 this, SLOT(sCalculateExtendedPrice()));
  connect(_netUnitPrice,         SIGNAL(idChanged(int)),                 this, SLOT(sPriceGroup()));
  connect(_saleNetUnitPrice,     SIGNAL(valueChanged()),                 this, SLOT(sCalculateSaleDiscountPrcnt()));
  connect(_saleNetUnitPrice,     SIGNAL(valueChanged()),                 this, SLOT(sCalculateSaleExtendedPrice()));
  connect(_qtyAuth,              SIGNAL(textChanged(const QString&)),    this, SLOT(sCalculateExtendedPrice()));
  connect(_qtyAuth,              SIGNAL(textChanged(const QString&)),    this, SLOT(sCalculateSaleExtendedPrice()));
  connect(_qtyAuth,              SIGNAL(textChanged(const QString&)),    this, SLOT(sPopulateOrderInfo()));
  connect(_qtyAuth,              SIGNAL(textChanged(const QString&)),    this, SLOT(sCalcWoUnitCost()));
  connect(_save,                 SIGNAL(clicked()),                      this, SLOT(sSaveClicked()));
  connect(_taxLit,               SIGNAL(leftClickedURL(const QString&)), this, SLOT(sTaxDetail()));
  connect(_taxType,              SIGNAL(newID(int)),                     this, SLOT(sCalculateTax()));
  connect(_qtyUOM,               SIGNAL(newID(int)),                     this, SLOT(sQtyUOMChanged()));
  connect(_qtyUOM,               SIGNAL(newID(int)),                     this, SLOT(sPopulateOrderInfo()));
  connect(_pricingUOM,           SIGNAL(newID(int)),                     this, SLOT(sPriceUOMChanged()));
  connect(_disposition,          SIGNAL(currentIndexChanged(int)),       this, SLOT(sDispositionChanged()));
  connect(_shipWhs,              SIGNAL(newID(int)),                     this, SLOT(sDetermineAvailability()));
  connect(_scheduledDate,        SIGNAL(newDate(const QDate&)),          this, SLOT(sDetermineAvailability()));
  connect(_qtyAuth,              SIGNAL(lostFocus()),                    this, SLOT(sDetermineAvailability()));
  connect(_createOrder,          SIGNAL(toggled(bool)),                  this, SLOT(sHandleWo(bool)));
  connect(_showAvailability,     SIGNAL(toggled(bool)),                  this, SLOT(sDetermineAvailability()));
  connect(this,                  SIGNAL(rejected()),                     this, SLOT(rejectEvent()));
  connect(_authLotSerial,        SIGNAL(toggled(bool)),                  _qtyUOM, SLOT(setDisabled(bool)));
      
  connect(_new,                  SIGNAL(clicked()),	                     this, SLOT(sNew()));
  connect(_edit,	               SIGNAL(clicked()),	                     this, SLOT(sEdit()));
  connect(_delete,	             SIGNAL(clicked()),	                     this, SLOT(sDelete()));
    
  _raitemls->addColumn(tr("Lot/Serial"),  -1,           Qt::AlignLeft , true, "ls_number"  );
  _raitemls->addColumn(tr("Warranty"),	  _dateColumn,  Qt::AlignRight, true, "lsreg_expiredate"  );
  _raitemls->addColumn(tr("Registered"),  _qtyColumn,   Qt::AlignRight, true, "raitemls_qtyregistered"  );
  _raitemls->addColumn(tr("Authorized"),  _qtyColumn,   Qt::AlignRight, true, "raitemls_qtyauthorized"  );
  _raitemls->addColumn(tr("Received"),    _qtyColumn,   Qt::AlignRight, true, "raitemls_qtyreceived"  );

  _item->setType(ItemLineEdit::cSold | ItemLineEdit::cActive);
  _item->addExtraClause( QString("(itemsite_active)") );  // ItemLineEdit::cActive doesn't compare against the itemsite record
  _item->addExtraClause( QString("(itemsite_sold)") );    // ItemLineEdit::cSold doesn't compare against the itemsite record

  _orderQty->setValidator(omfgThis->qtyVal());
  _qtyAuth->setValidator(omfgThis->qtyVal());
  _discountFromSale->setValidator(omfgThis->negPercentVal());
  _saleDiscountFromSale->setValidator(omfgThis->negPercentVal());
  _taxType->setEnabled(_privileges->check("OverrideTax"));
  _availabilityGroup->setEnabled(_showAvailability->isChecked());

  _qtySold->setPrecision(omfgThis->qtyVal());
  _qtyReceived->setPrecision(omfgThis->qtyVal());
  _qtyShipped->setPrecision(omfgThis->qtyVal());
  _onHand->setPrecision(omfgThis->qtyVal());
  _allocated->setPrecision(omfgThis->qtyVal());
  _unallocated->setPrecision(omfgThis->qtyVal());
  _onOrder->setPrecision(omfgThis->qtyVal());
  _available->setPrecision(omfgThis->qtyVal());
  _discountFromList->setPrecision(omfgThis->percentVal());
  
  //If not multi-warehouse hide whs controls
  if (!_metrics->boolean("MultiWhs"))
  {
    _warehouseLit->hide();
    _warehouse->hide();
    _shipWhs->hide();
    _shipWhsLit->hide();
  } 

  //Remove lot/serial  if no lot/serial tracking
  if (!_metrics->boolean("LotSerialControl"))
    _tab->removeTab(_tab->indexOf(_lotserial));
  
  adjustSize();

  _altcosAccntid->setType(GLCluster::cRevenue | GLCluster::cExpense);
  
  q.exec("BEGIN;"); //In case problems or we cancel out
}

returnAuthorizationItem::~returnAuthorizationItem()
{
    // no need to delete child widgets, Qt does it all for us
}

void returnAuthorizationItem::languageChange()
{
    retranslateUi(this);
}

enum SetResponse returnAuthorizationItem::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("rahead_id", &valid);
  if (valid)
  {
    _raheadid = param.toInt();
    q.prepare("SELECT *, "
              "       COALESCE(cust_preferred_warehous_id,-1) AS prefwhs, "
              "       COALESCE(rahead_orig_cohead_id,-1) AS cohead_id, "
              "       crmacct_id "
              "FROM rahead, custinfo, crmacct "
              "WHERE ( (rahead_cust_id=cust_id) "
              "AND (rahead_id=:rahead_id) "
              "AND (rahead_cust_id=crmacct_cust_id) );");
    q.bindValue(":rahead_id", _raheadid);
    q.exec();
    if (q.first())
    {
      _authNumber->setText(q.value("rahead_number").toString());
      _taxzoneid = q.value("rahead_taxzone_id").toInt();
      _tax->setId(q.value("rahead_curr_id").toInt());
      _rsnCode->setId(q.value("rahead_rsncode_id").toInt());
      _custid = q.value("rahead_cust_id").toInt();
      _shiptoid = q.value("rahead_shipto_id").toInt();
      _netUnitPrice->setId(q.value("rahead_curr_id").toInt());
      _netUnitPrice->setEffective(q.value("rahead_authdate").toDate());
      _saleNetUnitPrice->setId(q.value("rahead_curr_id").toInt());
      _saleNetUnitPrice->setEffective(q.value("rahead_authdate").toDate());
      _preferredWarehousid = q.value("prefwhs").toInt();
      _creditmethod = q.value("rahead_creditmethod").toString();
      _crmacctid = q.value("crmacct_id").toInt();
    }
    else if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return UndefinedError;
    }
  }

  param = pParams.value("warehous_id", &valid);
  if (valid)
  {
    _preferredWarehousid = param.toInt();
    _warehouse->setId(_preferredWarehousid);
  }

  param = pParams.value("shipwarehous_id", &valid);
  if (valid)
  {
    _preferredShipWarehousid = param.toInt();
    _shipWhs->setId(_preferredShipWarehousid);
  }

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      _mode = cNew;
      
      connect(_discountFromSale, SIGNAL(lostFocus()), this, SLOT(sCalculateFromDiscount()));
      connect(_saleDiscountFromSale, SIGNAL(lostFocus()), this, SLOT(sCalculateSaleFromDiscount()));

      q.prepare( "SELECT (COALESCE(MAX(raitem_linenumber), 0) + 1) AS n_linenumber "
                 "FROM raitem "
                 "WHERE (raitem_rahead_id=:rahead_id);" );
      q.bindValue(":rahead_id", _raheadid);
      q.exec();
      if (q.first())
        _lineNumber->setText(q.value("n_linenumber").toString());
      else if (q.lastError().type() == QSqlError::NoError)
      {
        systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
        return UndefinedError;
      }

      q.prepare( "SELECT rahead_disposition "
                 "FROM rahead "
                 "WHERE (rahead_id=:rahead_id);" );
      q.bindValue(":rahead_id", _raheadid);
      q.exec();
      if (q.first())
      {

        if (q.value("rahead_disposition").toString() == "C") // credit
          sDispositionChanged();
        else if (q.value("rahead_disposition").toString() == "R") // return
          _disposition->setCurrentIndex(1);
        else if (q.value("rahead_disposition").toString() == "P") // replace
          _disposition->setCurrentIndex(2);
        else if (q.value("rahead_disposition").toString() == "V")  // service
          _disposition->setCurrentIndex(3);
        else if (q.value("rahead_disposition").toString() == "M")  // ship
          _disposition->setCurrentIndex(4);
      }
      else if (q.lastError().type() == QSqlError::NoError)
      {
        systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
        return UndefinedError;
      }


      _origSoNumber->hide();
      _origSoNumberLit->hide();
      _origSoLineNumber->hide();
      _origSoLineNumberLit->hide();
      _newSoNumber->hide();
      _newSoNumberLit->hide();
      _newSoLineNumber->hide();
      _newSoLineNumberLit->hide();
      _qtySold->hide();
      _qtySoldLit->hide();
      _discountFromSalePrcntLit->hide();
      _discountFromSale->hide();
      _salePrice->hide();
      _salePriceLit->hide();
      _item->setFocus();
      _comments->setType(Comments::ReturnAuthItem);
      _comments->setEnabled(false);
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;

      _item->setReadOnly(TRUE);
      _warehouse->setEnabled(FALSE);
      _shipWhs->setEnabled(FALSE);
      _qtyAuth->setFocus();
      _comments->setType(Comments::ReturnAuthItem);
      _comments->setEnabled(true);

      connect(_discountFromSale, SIGNAL(lostFocus()), this, SLOT(sCalculateFromDiscount()));
      connect(_saleDiscountFromSale, SIGNAL(lostFocus()), this, SLOT(sCalculateSaleFromDiscount()));
 
      _save->setFocus();
    }
    else if (param.toString() == "view")
    {
      _mode = cView;

      _item->setReadOnly(TRUE);
      _warehouse->setEnabled(FALSE);
      _shipWhs->setEnabled(FALSE);
      _disposition->setEnabled(FALSE);
      _qtyAuth->setEnabled(FALSE);
      _qtyUOM->setEnabled(FALSE);
      _netUnitPrice->setEnabled(FALSE);
      _saleNetUnitPrice->setEnabled(FALSE);
      _listPrices->setEnabled(FALSE);
      _saleListPrices->setEnabled(FALSE);
      _pricingUOM->setEnabled(FALSE);
      _salePricingUOM->setEnabled(FALSE);
      _discountFromSale->setEnabled(FALSE);
      _saleDiscountFromSale->setEnabled(FALSE);
      _notes->setReadOnly(TRUE);
      _taxType->setEnabled(FALSE);
      _rsnCode->setEnabled(FALSE);
      _altcosAccntid->setEnabled(FALSE);
      _showAvailability->setEnabled(FALSE);
      _createOrder->setEnabled(FALSE);
      _scheduledDate->setEnabled(FALSE);
      _warranty->setEnabled(FALSE);
      _comments->setType(Comments::ReturnAuthItem);
      _comments->setReadOnly(TRUE);

      _save->hide();
      _close->setText(tr("&Close"));
      _close->setFocus();
    }
  }

  param = pParams.value("orig_cohead_id", &valid);
  if (valid)
    _origsoid = param.toInt();

  param = pParams.value("raitem_id", &valid);
  if (valid)
  {
    _raitemid = param.toInt();
    populate();
  }

  return NoError; 
}

void returnAuthorizationItem::sSaveClicked()
{
  if (sSave())
  {
    q.exec("COMMIT;");
    done(_raitemid);
  }
}

bool returnAuthorizationItem::sSave()
{ 
  // credit, return, replace, service, ship
  const char *dispositionTypes[] = { "C", "R", "P", "V", "S" };
  
  if (!(_scheduledDate->isValid()) && _scheduledDate->isEnabled())
  {
    QMessageBox::warning( this, windowTitle(),
                          tr("<p>You must enter a valid Schedule Date.") );
    _scheduledDate->setFocus();
    return false;
  }
  
  if ( (_coitemid != -1) && (_qtyAuth->toDouble() > _soldQty) )
    if (QMessageBox::question(this, tr("Over Authorize"),
            tr("<p>The authorized quantity exceeds "
                "the original sold quantity on the "
                "original Sales Order.  "
                "Do you want to correct the quantity?"),
               QMessageBox::Yes | QMessageBox::Default,
               QMessageBox::No  | QMessageBox::Escape) == QMessageBox::Yes)
  {
      _qtyAuth->setFocus();
      return false;
  }

  if (_mode == cNew)
  {
    q.exec("SELECT NEXTVAL('raitem_raitem_id_seq') AS _raitem_id");
    if (q.first())
    {
      _raitemid  = q.value("_raitem_id").toInt();
      _comments->setId(_raitemid);
    }
    else if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      reject();
    }

    q.prepare( "INSERT INTO raitem "
               "( raitem_id, raitem_rahead_id, raitem_linenumber, raitem_itemsite_id,"
               "  raitem_disposition, raitem_qtyauthorized, "
               "  raitem_qty_uom_id, raitem_qty_invuomratio,"
               "  raitem_price_uom_id, raitem_price_invuomratio,"
               "  raitem_unitprice, raitem_taxtype_id, "
               "  raitem_notes, raitem_rsncode_id, raitem_cos_accnt_id, "
               "  raitem_scheddate, raitem_warranty, raitem_coitem_itemsite_id, "
               "  raitem_saleprice, raitem_unitcost ) "
               "SELECT :raitem_id, :rahead_id, :raitem_linenumber, rcv.itemsite_id,"
               "       :raitem_disposition, :raitem_qtyauthorized,"
               "       :qty_uom_id, :qty_invuomratio,"
               "       :price_uom_id, :price_invuomratio,"
               "       :raitem_unitprice, :raitem_taxtype_id, "
               "       :raitem_notes, :raitem_rsncode_id, :raitem_cos_accnt_id, "
               "       :raitem_scheddate, :raitem_warranty, shp.itemsite_id, "
               "       :raitem_saleprice, :raitem_unitcost "
               "FROM itemsite rcv "
               "  LEFT OUTER JOIN itemsite shp ON "
               "        (shp.itemsite_item_id=rcv.itemsite_item_id) "
               "    AND (shp.itemsite_warehous_id=COALESCE(:shipWhs_id,-1)) "
               "WHERE ( (rcv.itemsite_item_id=:item_id)"
               " AND (rcv.itemsite_warehous_id=:warehous_id) );" );
  }
  else
  {
    q.prepare( "UPDATE raitem "
               "SET raitem_disposition=:raitem_disposition, "
               "    raitem_qtyauthorized=:raitem_qtyauthorized, "
               "    raitem_qty_uom_id=:qty_uom_id,"
               "    raitem_qty_invuomratio=:qty_invuomratio,"
               "    raitem_price_uom_id=:price_uom_id,"
               "    raitem_price_invuomratio=:price_invuomratio,"
               "    raitem_unitprice=:raitem_unitprice,"
               "    raitem_taxtype_id=:raitem_taxtype_id, "
               "    raitem_notes=:raitem_notes,"
               "    raitem_rsncode_id=:raitem_rsncode_id, "
               "    raitem_cos_accnt_id=:raitem_cos_accnt_id, "
               "    raitem_scheddate=:raitem_scheddate, "
               "    raitem_warranty=:raitem_warranty, "
               "    raitem_saleprice=:raitem_saleprice, "
               "    raitem_coitem_itemsite_id=:coitem_itemsite_id, "
               "    raitem_unitcost=:raitem_unitcost "
               "WHERE (raitem_id=:raitem_id);" );
    
     if (_disposition->currentIndex() >= 2)
     {     
       if (_coitemitemsiteid == -1)
       {
         XSqlQuery coitemsite;
         coitemsite.prepare("SELECT itemsite_id "
                        "FROM itemsite "
                        "WHERE ((itemsite_item_id=:item_id)"
                        " AND (itemsite_warehous_id=:warehous_id) "
                        " AND (itemsite_active) "
                        " AND (itemsite_sold));");
         coitemsite.bindValue(":item_id", _item->id());
         coitemsite.bindValue(":warehous_id",_shipWhs->id());
         coitemsite.exec();
         if (coitemsite.first())
           _coitemitemsiteid=coitemsite.value("itemsite_id").toInt();
         else
         {
             QMessageBox::critical( this, tr("Item site not found"),
             tr("<p>No valid item site record was found for the selected "
                "item at the selected shipping site.") );
             return false;
         }
       }
       q.bindValue(":coitem_itemsite_id",_coitemitemsiteid);
     }
  }

  q.bindValue(":raitem_id", _raitemid);
  q.bindValue(":rahead_id", _raheadid);
  q.bindValue(":raitem_linenumber", _lineNumber->text().toInt());
  q.bindValue(":raitem_qtyauthorized", _qtyAuth->toDouble());
  q.bindValue(":raitem_disposition", QString(dispositionTypes[_disposition->currentIndex()]));
  q.bindValue(":qty_uom_id", _qtyUOM->id());
  q.bindValue(":qty_invuomratio", _qtyinvuomratio);
  q.bindValue(":price_uom_id", _pricingUOM->id());
  q.bindValue(":price_invuomratio", _priceinvuomratio);
  q.bindValue(":raitem_unitprice", _netUnitPrice->localValue());
  if (_taxType->isValid())
    q.bindValue(":raitem_taxtype_id", _taxType->id());
  q.bindValue(":raitem_notes", _notes->toPlainText());
  q.bindValue(":raitem_notes", _notes->toPlainText());
  if (_rsnCode->isValid())
    q.bindValue(":raitem_rsncode_id", _rsnCode->id());
  q.bindValue(":item_id", _item->id());
  q.bindValue(":warehous_id", _warehouse->id());
  if (_disposition->currentIndex() >= 2) // replace, service, or ship
    q.bindValue(":shipWhs_id", _shipWhs->id());
  if (_altcosAccntid->id() != -1)
    q.bindValue(":raitem_cos_accnt_id", _altcosAccntid->id()); 
  q.bindValue(":raitem_scheddate", _scheduledDate->date());
  q.bindValue(":raitem_warranty",QVariant(_warranty->isChecked()));
  q.bindValue(":raitem_saleprice", _saleNetUnitPrice->localValue());
  if (_costmethod=="A")
    q.bindValue(":raitem_unitcost", _unitCost->localValue());
  q.exec();
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    reject();
  }
  
//  Check to see if a S/O should be re-scheduled
  if (_orderId != -1)
  {
    if (_qtyAuth->toDouble() == 0)
    {
      q.prepare("SELECT deletewo(:woid, true, true) AS result;");
      q.bindValue(":woid", _orderId);
      q.exec();
      if (q.value("result").toInt() < 0)
      {
        systemError(this, storedProcErrorLookup("deleteWo", q.value("result").toInt()),
                    __FILE__, __LINE__);
        reject();
      }
      else if (q.lastError().type() != QSqlError::NoError)
      {
        systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
        reject();
      }
    }
    else if (_scheduledDate->date() != _cScheduledDate)
    {
      if (QMessageBox::question(this, tr("Reschedule W/O?"),
                tr("<p>The Scheduled Date for this Line "
                    "Item has been changed.  Should the "
                    "W/O for this Line Item be Re-"
                    "Scheduled to reflect this change?"),
                   QMessageBox::Yes | QMessageBox::Default,
                   QMessageBox::No  | QMessageBox::Escape) == QMessageBox::Yes)
      {
        q.prepare("SELECT changeWoDates(:wo_id, :schedDate, :schedDate, TRUE) AS result;");
        q.bindValue(":wo_id", _orderId);
        q.bindValue(":schedDate", _scheduledDate->date());
        q.exec();
        if (q.first())
        {
          int result = q.value("result").toInt();
          if (result < 0)
          {
            systemError(this, storedProcErrorLookup("changeWoDates", result),
            __FILE__, __LINE__);
            reject();
          }
          _cScheduledDate = _scheduledDate->date();
        }
        else if (q.lastError().type() != QSqlError::NoError)
        {
          systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
          reject();
        }
      }
    }

    if (_qtyAuth->toDouble() != _cQtyOrdered &&
        _qtyAuth->toDouble() > 0)
    {
      if (_item->itemType() == "M")
      {
        if (QMessageBox::question(this, tr("Change W/O Quantity?"),
                  tr("<p>The quantity authorized for this Return "
                     "Authorization Line Item has been changed. "
                     "Should the quantity required for the "
                     "associated Work Order be changed to "
                     "reflect this?"),
                  QMessageBox::Yes | QMessageBox::Default,
                  QMessageBox::No  | QMessageBox::Escape) == QMessageBox::Yes)
        {
          q.prepare("SELECT changeWoQty(:wo_id, :qty, TRUE) AS result;");
          q.bindValue(":wo_id", _orderId);
          q.bindValue(":qty", _qtyAuth->toDouble() * _qtyinvuomratio);
          q.exec();
          if (q.first())
          {
            int result = q.value("result").toInt();
            if (result < 0)
            {
              systemError(this, storedProcErrorLookup("changeWoQty", result),
              __FILE__, __LINE__);
              reject();
            }
          }
          else if (q.lastError().type() != QSqlError::NoError)
          {
            systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
            reject();
          }
        }
      }
    }
  }
  //If this save has resulted in a link to an shipping S/O, we need to signal that
  if (_disposition->currentIndex() > 1) // replace, service, or ship
  {
    XSqlQuery so;
    so.prepare("SELECT raitem_new_coitem_id, cohead_number, "
               " cust_name "
               "FROM raitem,custinfo,cohead,coitem "
               "WHERE ((raitem_id=:raitem_id) "
               "AND (raitem_new_coitem_id IS NOT NULL) "
               "AND (raitem_new_coitem_id=coitem_id) "
               "AND (cust_id=cohead_cust_id) "
               "AND (coitem_cohead_id=cohead_id));");
    so.bindValue(":raitem_id", _raitemid);
    so.exec();
    if (so.first())
    {
      omfgThis->sSalesOrdersUpdated(q.value("rahead_new_cohead_id").toInt());
      //  If requested, create a new W/O or P/R for this coitem
      if ( ( (_mode == cNew) || (_mode == cEdit) ) &&
         (_createOrder->isChecked()) &&
         (_qtyAuth->toDouble() > 0) &&
         (_orderId == -1) )
      {
        QString chartype;
        if (_item->itemType() == "M")
        {
          q.prepare( "SELECT createWo(:orderNumber, itemsite_id, :qty, itemsite_leadtime, :dueDate, :comments, :parent_type, :parent_id) AS result, itemsite_id "
                     "FROM itemsite "
                     "WHERE ( (itemsite_item_id=:item_id)"
                     " AND (itemsite_warehous_id=:warehous_id) );" );
          q.bindValue(":orderNumber", so.value("cohead_number").toInt());
          q.bindValue(":qty", _orderQty->toDouble());
          q.bindValue(":dueDate", _scheduledDate->date());
          q.bindValue(":comments", so.value("cust_name").toString() + "\n" + _notes->toPlainText());
          q.bindValue(":item_id", _item->id());
          q.bindValue(":warehous_id", _shipWhs->id());
          q.bindValue(":parent_type", QString("S"));
          q.bindValue(":parent_id", so.value("raitem_new_coitem_id").toInt());
          q.exec();
        }
        if (q.first())
        {
          _orderId = q.value("result").toInt();
          if (_orderId < 0)
          {
            QString procname;
            if (_item->itemType() == "M")
              procname = "createWo";
            else
              procname = "unnamed stored procedure";
            systemError(this, storedProcErrorLookup(procname, _orderId),
                __FILE__, __LINE__);
            reject();
          }

          if (_item->itemType() == "M")
          {
            omfgThis->sWorkOrdersUpdated(_orderId, TRUE);

    //  Update the newly created coitem with the newly create wo_id
            q.prepare( "UPDATE coitem "
                       "SET coitem_order_type='W', coitem_order_id=:orderid "
                       "WHERE (coitem_id=:soitem_id);" );
            q.bindValue(":orderid", _orderId);
            q.bindValue(":soitem_id", so.value("raitem_new_coitem_id").toInt());
            q.exec();
            if (q.lastError().type() != QSqlError::NoError)
            {
              systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
              reject();
            }
          }
        }
      }
    }
  }
  _mode = cEdit;
  return true; 
}

void returnAuthorizationItem::sPopulateItemInfo()
{
  XSqlQuery uom;
  uom.prepare("SELECT uom_id, uom_name"
              "  FROM item"
              "  JOIN uom ON (item_inv_uom_id=uom_id)"
              " WHERE(item_id=:item_id)"
              " UNION "
              "SELECT uom_id, uom_name"
              "  FROM item"
              "  JOIN itemuomconv ON (itemuomconv_item_id=item_id)"
              "  JOIN uom ON (itemuomconv_to_uom_id=uom_id)"
              " WHERE((itemuomconv_from_uom_id=item_inv_uom_id)"
              "   AND (item_id=:item_id))"
              " UNION "
              "SELECT uom_id, uom_name"
              "  FROM item"
              "  JOIN itemuomconv ON (itemuomconv_item_id=item_id)"
              "  JOIN uom ON (itemuomconv_from_uom_id=uom_id)"
              " WHERE((itemuomconv_to_uom_id=item_inv_uom_id)"
              "   AND (item_id=:item_id))"
              " ORDER BY uom_name;");
  uom.bindValue(":item_id", _item->id());
  uom.exec();
  _qtyUOM->populate(uom);
  _pricingUOM->populate(uom);
  _salePricingUOM->populate(uom);

  XSqlQuery item;
  item.prepare( "SELECT item_inv_uom_id, item_price_uom_id,"
                "       iteminvpricerat(item_id) AS iteminvpricerat,"
                "       item_listprice, "
                "       stdCost(item_id) AS stdcost,"
                "       getItemTaxType(item_id, :taxzone_id) AS taxtype_id "
                "  FROM item"
                " WHERE (item_id=:item_id);" );
  item.bindValue(":item_id", _item->id());
  item.bindValue(":taxzone_id", _taxzoneid);
  item.exec();
  if (item.first())
  {
    _priceRatio = item.value("iteminvpricerat").toDouble();
    _qtyUOM->setId(item.value("item_inv_uom_id").toInt());
    _pricingUOM->setId(item.value("item_price_uom_id").toInt());
    _salePricingUOM->setId(item.value("item_price_uom_id").toInt());
    _priceinvuomratio = item.value("iteminvpricerat").toDouble();
    _qtyinvuomratio = 1.0;
    _invuomid = item.value("item_inv_uom_id").toInt();
    _listPrice->setBaseValue(item.value("item_listprice").toDouble());
    _unitCost->setBaseValue(item.value("stdcost").toDouble());
    _taxType->setId(item.value("taxtype_id").toInt()); 
  }
  else if (item.lastError().type() != QSqlError::NoError)
  {
    systemError(this, item.lastError().databaseText(), __FILE__, __LINE__);
    return;
  } 

  if (_item->itemType() == "M" && _costmethod != "J")
    _createOrder->setEnabled(TRUE);
  else
    _createOrder->setEnabled(FALSE);
  if (_costmethod == "J")
    _createOrder->setChecked(TRUE);

  _warehouse->findItemsites(_item->id());
  _shipWhs->findItemsites(_item->id());
  if(_preferredWarehousid > 0)
    _warehouse->setId(_preferredWarehousid);
  if(_preferredShipWarehousid > 0)
    _shipWhs->setId(_preferredShipWarehousid);
}

void returnAuthorizationItem::sPopulateItemsiteInfo()
{
  if (_item->isValid())
  {
    XSqlQuery itemsite;
    itemsite.prepare( "SELECT itemsite_leadtime, itemsite_controlmethod, "
                      "       itemsite_createwo, itemsite_createpr, "
                      "       itemsite_costmethod, itemsite_id "
                      "FROM item, itemsite "
                      "WHERE ( (itemsite_item_id=item_id)"
                      " AND (itemsite_warehous_id=:warehous_id)"
                      " AND (item_id=:item_id) );" );
    itemsite.bindValue(":warehous_id", _shipWhs->id());
    itemsite.bindValue(":item_id", _item->id());
    itemsite.exec();
    if (itemsite.first())
    {
      _leadTime = itemsite.value("itemsite_leadtime").toInt();
      _costmethod = itemsite.value("itemsite_costmethod").toString();

      if (_costmethod == "J")
      {
        _createOrder->setChecked(TRUE);
        _createOrder->setEnabled(FALSE);
      }
      else if (cNew == _mode)
      {
        if ( _disposition->currentIndex() == 3 && _costmethod != "J")
        {
          QMessageBox::warning( this, tr("Cannot use Service Disposition"),
                                tr("<p>Only Items Sites using the Job cost method may have a Disposition of Service.") );
          _item->setId(-1);
          _item->setFocus();
          return;
        }

        if (_item->itemType() == "M" && _costmethod != "J")
          _createOrder->setChecked(itemsite.value("itemsite_createwo").toBool());
        else
        {
          _createOrder->setChecked(FALSE);
          _createOrder->setEnabled(FALSE);
        }
      }
      
     if ( _disposition->currentIndex() == 1 || 
          _disposition->currentIndex() == 2)
     {
      if (_costmethod == "A")
      {
        if (cNew != _mode)
          _unitCost->setLocalValue(_unitcost);
        else if (_origsoid != -1)
        {
          XSqlQuery uc;
          uc.prepare("SELECT COALESCE(SUM(cohist_unitcost * cohist_qtyshipped) / "
                     "  SUM(cohist_qtyshipped),0) AS unitcost "
                     "FROM cohist "
                     " JOIN cohead ON ((cohist_doctype='I') "
                     "             AND (cohist_ordernumber=cohead_number)) "
                     "WHERE ((cohead_id=:cohead_id) "
                     "  AND (cohead_itemsite_id=:itemsite_id); ");
          uc.bindValue(":cohead_id", _origsoid);
          uc.bindValue(":itemsite_id", itemsite.value("itemsite_id").toInt());
          uc.exec();
          if (uc.first())
            _unitCost->setLocalValue(uc.value("unitcost").toDouble());
          else if (uc.lastError().type() != QSqlError::NoError)
          {
             systemError(this, itemsite.lastError().databaseText(), __FILE__, __LINE__);
             return;
          }
        }
        _unitCost->setEnabled(cView != _mode);
      }
      else
        _unitCost->setEnabled(_costmethod == "A");
     }
     else
       _unitCost->setEnabled(false);
      
      _tab->setTabEnabled(_tab->indexOf(_lotserial),
      (itemsite.value("itemsite_controlmethod").toString() == "L" ||
       itemsite.value("itemsite_controlmethod").toString() == "S"));
    }
    else if (itemsite.lastError().type() != QSqlError::NoError)
    {
      systemError(this, itemsite.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }
}

void returnAuthorizationItem::populate()
{
  XSqlQuery raitem;
  raitem.prepare("SELECT rahead_number, rahead_curr_id, rahead_authdate, rahead_taxzone_id, raitem.*, "
                 "       formatRaLineNumber(raitem_id) AS linenumber,"
                 "       och.cohead_number AS orig_number, formatSoLineNumber(oc.coitem_id) AS orig_linenumber, "
                 "       nch.cohead_number AS new_number, formatSoLineNumber(nc.coitem_id) AS new_linenumber, "
                 "       COALESCE(raitem_orig_coitem_id,-1) AS ra_coitem_id, oc.coitem_price, "
                 "       oc.coitem_qtyshipped AS qtysold,"   
                 "       raitem_qtyauthorized AS qtyauth,"
                 "       raitem_qtyreceived AS qtyrcvd,"
                 "       nc.coitem_qtyshipped AS qtyshipd,"
                 "       rahead_curr_id AS taxcurr, "
                 "       item_inv_uom_id, "
                 "       COALESCE(nc.coitem_order_id,-1) AS coitem_order_id, "
                 "       nc.coitem_order_type AS coitem_order_type, "
                 "       rcv.itemsite_warehous_id AS itemsite_warehous_id, "
                 "       shp.itemsite_warehous_id AS shipWhs_id, "
                 "       qtyToReceive('RA', raitem_id) AS qtytorcv, "
                 "       crmacct_id, "
                 "       nc.coitem_price AS saleprice, "
                 "       nch.cohead_curr_id AS salecurrid, "
                 "       nch.cohead_orderdate AS saledate "
                 "FROM raitem "
                 "  LEFT OUTER JOIN coitem oc ON (raitem_orig_coitem_id=oc.coitem_id) "
                 "  LEFT OUTER JOIN cohead och ON (oc.coitem_cohead_id=och.cohead_id) "
                 "  LEFT OUTER JOIN coitem nc ON (raitem_new_coitem_id=nc.coitem_id) "
                 "  LEFT OUTER JOIN cohead nch ON (nc.coitem_cohead_id=nch.cohead_id)"
                 "  LEFT OUTER JOIN itemsite shp ON (raitem_coitem_itemsite_id=shp.itemsite_id),"
                 "  rahead, itemsite rcv, item, crmacct "
                 "WHERE ((raitem_rahead_id=rahead_id)"
                 " AND  (raitem_id=:raitem_id) "
                 " AND  (raitem_itemsite_id=rcv.itemsite_id) "
                 " AND  (item_id=rcv.itemsite_item_id) "
                 " AND  (rahead_cust_id=crmacct_cust_id) );" );
  raitem.bindValue(":raitem_id", _raitemid);
  _comments->setId(_raitemid);
  raitem.exec();
  if (raitem.first())
  {
    _authNumber->setText(raitem.value("rahead_number").toString());
    _unitcost = raitem.value("raitem_unitcost").toDouble();

    if (raitem.value("new_number").toInt() > 0)
    {
      _newSoNumber->setText(raitem.value("new_number").toString());
      _newSoLineNumber->setText(raitem.value("new_linenumber").toString());
    }
    else
    {
      _newSoNumberLit->hide();
      _newSoLineNumberLit->hide();
      _newSoNumber->hide();
      _newSoLineNumber->hide();
    }
    if (raitem.value("raitem_disposition").toString() == "C") // credit
      sDispositionChanged();
    else if (raitem.value("raitem_disposition").toString() == "R") // return
      _disposition->setCurrentIndex(1);
    else if (raitem.value("raitem_disposition").toString() == "P") // replace
      _disposition->setCurrentIndex(2);
    else if (raitem.value("raitem_disposition").toString() == "V") // service
      _disposition->setCurrentIndex(3);
    else if (raitem.value("raitem_disposition").toString() == "S") // ship
      _disposition->setCurrentIndex(4);
	  _orderId = raitem.value("coitem_order_id").toInt();
    _netUnitPrice->setId(raitem.value("rahead_curr_id").toInt());
    _netUnitPrice->setEffective(raitem.value("rahead_authdate").toDate());
    _netUnitPrice->setLocalValue(raitem.value("raitem_unitprice").toDouble());
    _saleNetUnitPrice->setId(raitem.value("salecurrid").toInt());
    _saleNetUnitPrice->setEffective(raitem.value("saledate").toDate());
    _saleNetUnitPrice->setLocalValue(raitem.value("saleprice").toDouble());
    // do _item and _taxzone before other tax stuff because of signal cascade
    _taxzoneid = raitem.value("rahead_taxzone_id").toInt();
    _item->setItemsiteid(raitem.value("raitem_itemsite_id").toInt());
    _warehouse->setId(raitem.value("itemsite_warehous_id").toInt());
    _shipWhs->setId(raitem.value("shipWhs_id").toInt());
    _taxType->setId(raitem.value("raitem_taxtype_id").toInt());
    _tax->setId(raitem.value("taxcurr").toInt());
    _invuomid=raitem.value("item_inv_uom_id").toInt();
    _qtyUOM->setId(raitem.value("raitem_qty_uom_id").toInt());
    _pricingUOM->setId(raitem.value("raitem_price_uom_id").toInt());
    _qtyinvuomratio = raitem.value("raitem_qty_invuomratio").toDouble();
    _priceinvuomratio = raitem.value("raitem_price_invuomratio").toDouble();
    _lineNumber->setText(raitem.value("linenumber").toString());
    _qtyAuth->setDouble(raitem.value("qtyauth").toDouble());
    _qtyReceived->setDouble(raitem.value("qtyrcvd").toDouble());
    _qtyShipped->setDouble(raitem.value("qtyshipd").toDouble());
    _notes->setText(raitem.value("raitem_notes").toString());
    _rsnCode->setId(raitem.value("raitem_rsncode_id").toInt());
    _altcosAccntid->setId(raitem.value("raitem_cos_accnt_id").toInt());
    _scheduledDate->setDate(raitem.value("raitem_scheddate").toDate());
    _warranty->setChecked(raitem.value("raitem_warranty").toBool());
    _status = raitem.value("raitem_status").toString();
    _qtycredited = raitem.value("raitem_qtycredited").toDouble();

    _cQtyOrdered = _qtyAuth->toDouble();
    _cScheduledDate = _scheduledDate->date();
    _crmacctid = raitem.value("crmacct_id").toInt();

    _coitemid = raitem.value("ra_coitem_id").toInt();
    _coitemitemsiteid = raitem.value("raitem_coitem_itemsite_id").toInt();
    if (_coitemid != -1)
    {
      _origSoNumber->setText(raitem.value("orig_number").toString());
      _origSoLineNumber->setText(raitem.value("orig_linenumber").toString());
      _qtySold->setDouble(raitem.value("qtysold").toDouble());
      _soldQty=raitem.value("qtysold").toDouble();
      _qtyUOM->setEnabled(FALSE);
      _pricingUOM->setEnabled(FALSE);
      _salePricingUOM->setEnabled(FALSE);
      _salePrice->setId(raitem.value("rahead_curr_id").toInt());
      _salePrice->setEffective(raitem.value("rahead_authdate").toDate());
      _salePrice->setLocalValue(raitem.value("coitem_price").toDouble());
    }
    else
    {
      _origSoNumber->hide();
      _origSoNumberLit->hide();
      _origSoLineNumber->hide();
      _origSoLineNumberLit->hide();
      _discountFromSalePrcntLit->hide();
      _discountFromSale->hide();
      _qtySold->hide();
      _qtySoldLit->hide();
      _salePrice->hide();
      _salePriceLit->hide();
    }
    sDetermineAvailability();
    
    
    
    if (raitem.value("qtyrcvd").toDouble() > 0 || 
        raitem.value("qtyshipd").toDouble() > 0 ||
        raitem.value("qtytorcv").toDouble() > 0 ||
        _qtycredited > 0)
      _disposition->setEnabled(FALSE); 

    if (_orderId != -1)
    {
      XSqlQuery query;

      if (raitem.value("coitem_order_type").toString() == "W")
      {
        query.prepare( "SELECT wo_status,"
                       "       wo_qtyord AS qty,"
                       "       wo_duedate, warehous_id, warehous_code "
                       "FROM wo, itemsite, warehous "
                       "WHERE ((wo_itemsite_id=itemsite_id)"
                       " AND (itemsite_warehous_id=warehous_id)"
                       " AND (wo_id=:wo_id));" );
        query.bindValue(":wo_id", _orderId);
        query.exec();
        if (query.first())
        {
          _createOrder->setChecked(TRUE);

          _orderQty->setDouble(query.value("qty").toDouble());
          _orderDueDate->setDate(query.value("wo_duedate").toDate());
          _orderStatus->setText(query.value("wo_status").toString());

          if ((query.value("wo_status").toString() == "R") ||
              (query.value("wo_status").toString() == "C") ||
              (query.value("wo_status").toString() == "I"))
          {
            _createOrder->setEnabled(FALSE);
            if (_costmethod == "J")
              _qtyAuth->setEnabled(false);
          }
        }
        else
        {
          _orderId = -1;
          _createOrder->setChecked(FALSE);
        }
      }
    }
  }
  else if (raitem.lastError().type() != QSqlError::NoError)
  {
    systemError(this, raitem.lastError().databaseText(), __FILE__, __LINE__);
    return;
  } 
  sFillList();
}

void returnAuthorizationItem::sCalculateExtendedPrice()
{
  _extendedPrice->setLocalValue(((_qtyAuth->toDouble() * _qtyinvuomratio) / _priceinvuomratio) * _netUnitPrice->localValue());
}

void returnAuthorizationItem::sCalculateDiscountPrcnt()
{
  double unitPrice = _netUnitPrice->localValue();

  if (unitPrice == 0.0)
  {
    _discountFromList->setText("N/A");
    _discountFromSale->setText("N/A");
  }
  else
  {
    if (_listPrice->isZero())
      _discountFromList->setText("N/A");
    else
      _discountFromList->setDouble((1 - (unitPrice / _listPrice->localValue())) * 100 );

    if (_salePrice->isZero())
      _discountFromSale->setText("N/A");
    else
      _discountFromSale->setDouble((1 - (unitPrice / _salePrice->localValue())) * 100 );
  }
}

void returnAuthorizationItem::sCalculateFromDiscount()
{
  double discount = _discountFromSale->toDouble() / 100.0;

  if (_salePrice->isZero())
    _discountFromSale->setText(tr("N/A"));
  else
    _netUnitPrice->setLocalValue(_salePrice->localValue() - (_salePrice->localValue() * discount));
}

void returnAuthorizationItem::sPriceGroup()
{
  if (! omfgThis->singleCurrency())
    _priceGroup->setTitle(tr("In %1:").arg(_netUnitPrice->currAbbr())); 
}

void returnAuthorizationItem::sCalculateSaleExtendedPrice()
{
  _saleExtendedPrice->setLocalValue(((_qtyAuth->toDouble() * _qtyinvuomratio) / _priceinvuomratio) * _saleNetUnitPrice->localValue());
}

void returnAuthorizationItem::sCalculateSaleDiscountPrcnt()
{
  double unitPrice = _saleNetUnitPrice->localValue();

  if (unitPrice == 0.0)
  {
    _saleDiscountFromSale->setText("N/A");
  }
  else
  {
    if (_listPrice->isZero())
      _saleDiscountFromSale->setText("N/A");
    else
      _saleDiscountFromSale->setDouble((1 - (unitPrice / _listPrice->localValue())) * 100 );
  }
}

void returnAuthorizationItem::sCalculateSaleFromDiscount()
{
  double discount = _saleDiscountFromSale->toDouble() / 100.0;

  if (_listPrice->isZero())
    _saleDiscountFromSale->setText(tr("N/A"));
  else
    _saleNetUnitPrice->setLocalValue(_listPrice->localValue() - (_listPrice->localValue() * discount));
}

void returnAuthorizationItem::sListPrices()
{
  q.prepare( "SELECT formatSalesPrice(currToCurr(ipshead_curr_id, :curr_id, ipsprice_price, :effective)) AS price"
             "       FROM ipsass, ipshead, ipsprice "
             "       WHERE ( (ipsass_ipshead_id=ipshead_id)"
             "        AND (ipsprice_ipshead_id=ipshead_id)"
             "        AND (ipsprice_item_id=:item_id)"
             "        AND (ipsass_cust_id=:cust_id)"
             "        AND (COALESCE(LENGTH(ipsass_shipto_pattern), 0) = 0)"
             "        AND (CURRENT_DATE BETWEEN ipshead_effective AND (ipshead_expires - 1) ) )"

             "       UNION SELECT formatSalesPrice(ipsprice_price) AS price"
             "       FROM ipsass, ipshead, ipsprice "
             "       WHERE ( (ipsass_ipshead_id=ipshead_id)"
             "        AND (ipsprice_ipshead_id=ipshead_id)"
             "        AND (ipsprice_item_id=:item_id)"
             "        AND (ipsass_shipto_id=:shipto_id)"
             "        AND (ipsass_shipto_id != -1)"
             "        AND (CURRENT_DATE BETWEEN ipshead_effective AND (ipshead_expires - 1)) )"
             
             "       UNION SELECT formatSalesPrice(ipsprice_price) AS price"
             "       FROM ipsass, ipshead, ipsprice, cust "
             "       WHERE ( (ipsass_ipshead_id=ipshead_id)"
             "        AND (ipsprice_ipshead_id=ipshead_id)"
             "        AND (ipsprice_item_id=:item_id)"
             "        AND (ipsass_custtype_id=cust_custtype_id)"
             "        AND (cust_id=:cust_id)"
             "        AND (CURRENT_DATE BETWEEN ipshead_effective AND (ipshead_expires - 1)) )"
             
             "       UNION SELECT formatSalesPrice(ipsprice_price) AS price"
             "       FROM ipsass, ipshead, ipsprice, custtype, cust "
             "       WHERE ( (ipsass_ipshead_id=ipshead_id)"
             "        AND (ipsprice_ipshead_id=ipshead_id)"
             "        AND (ipsprice_item_id=:item_id)"
             "        AND (coalesce(length(ipsass_custtype_pattern), 0) > 0)"
             "        AND (custtype_code ~ ipsass_custtype_pattern)"
             "        AND (cust_custtype_id=custtype_id)"
             "        AND (cust_id=:cust_id)"
             "        AND (CURRENT_DATE BETWEEN ipshead_effective AND (ipshead_expires - 1)))"
             
             "       UNION SELECT formatSalesPrice(ipsprice_price) AS price"
             "       FROM ipsass, ipshead, ipsprice, shipto "
             "       WHERE ( (ipsass_ipshead_id=ipshead_id)"
             "        AND (ipsprice_ipshead_id=ipshead_id)"
             "        AND (ipsprice_item_id=:item_id)"
             "        AND (shipto_id=:shipto_id)"
             "        AND (COALESCE(LENGTH(ipsass_shipto_pattern), 0) > 0)"
             "        AND (shipto_num ~ ipsass_shipto_pattern)"
             "        AND (ipsass_cust_id=:cust_id)"
             "        AND (CURRENT_DATE BETWEEN ipshead_effective AND (ipshead_expires - 1)) )"

             "       UNION SELECT formatSalesPrice(ipsprice_price) AS price"
             "       FROM sale, ipshead, ipsprice "
             "       WHERE ((sale_ipshead_id=ipshead_id)"
             "        AND (ipsprice_ipshead_id=ipshead_id)"
             "        AND (ipsprice_item_id=:item_id)"
             "        AND (CURRENT_DATE BETWEEN sale_startdate AND (sale_enddate - 1)) ) "

             "       UNION SELECT formatSalesPrice(item_listprice - (item_listprice * cust_discntprcnt)) AS price "
             "       FROM item, cust "
             "       WHERE ( (item_sold)"
             "        AND (NOT item_exclusive)"
             "        AND (item_id=:item_id)"
             "        AND (cust_id=:cust_id) );");
  q.bindValue(":item_id", _item->id());
  q.bindValue(":cust_id", _custid);
  q.bindValue(":shipto_id", _shiptoid);
  q.bindValue(":curr_id", _netUnitPrice->id());
  q.bindValue(":effective", _netUnitPrice->effective());
  q.exec();
  if (q.size() == 1)
  {
    q.first();
    _netUnitPrice->setLocalValue(q.value("price").toDouble() * (_priceRatio / _priceinvuomratio));
  }
  else
  {
    ParameterList params;
    params.append("cust_id", _custid);
    params.append("shipto_id", _shiptoid);
    params.append("item_id", _item->id());
    // don't params.append("qty", ...) as we don't know how many were purchased
    params.append("curr_id", _netUnitPrice->id());
    params.append("effective", _netUnitPrice->effective());

    priceList newdlg(this);
    newdlg.set(params);
    if (newdlg.exec() == XDialog::Accepted)
    {
      _netUnitPrice->setLocalValue(newdlg._selectedPrice * (_priceRatio / _priceinvuomratio));
      sCalculateDiscountPrcnt();
    }
  }
}

void returnAuthorizationItem::sSaleListPrices()
{
  q.prepare( "SELECT formatSalesPrice(currToCurr(ipshead_curr_id, :curr_id, ipsprice_price, :effective)) AS price"
             "       FROM ipsass, ipshead, ipsprice "
             "       WHERE ( (ipsass_ipshead_id=ipshead_id)"
             "        AND (ipsprice_ipshead_id=ipshead_id)"
             "        AND (ipsprice_item_id=:item_id)"
             "        AND (ipsass_cust_id=:cust_id)"
             "        AND (COALESCE(LENGTH(ipsass_shipto_pattern), 0) = 0)"
             "        AND (CURRENT_DATE BETWEEN ipshead_effective AND (ipshead_expires - 1) ) )"

             "       UNION SELECT formatSalesPrice(ipsprice_price) AS price"
             "       FROM ipsass, ipshead, ipsprice "
             "       WHERE ( (ipsass_ipshead_id=ipshead_id)"
             "        AND (ipsprice_ipshead_id=ipshead_id)"
             "        AND (ipsprice_item_id=:item_id)"
             "        AND (ipsass_shipto_id=:shipto_id)"
             "        AND (ipsass_shipto_id != -1)"
             "        AND (CURRENT_DATE BETWEEN ipshead_effective AND (ipshead_expires - 1)) )"
             
             "       UNION SELECT formatSalesPrice(ipsprice_price) AS price"
             "       FROM ipsass, ipshead, ipsprice, cust "
             "       WHERE ( (ipsass_ipshead_id=ipshead_id)"
             "        AND (ipsprice_ipshead_id=ipshead_id)"
             "        AND (ipsprice_item_id=:item_id)"
             "        AND (ipsass_custtype_id=cust_custtype_id)"
             "        AND (cust_id=:cust_id)"
             "        AND (CURRENT_DATE BETWEEN ipshead_effective AND (ipshead_expires - 1)) )"
             
             "       UNION SELECT formatSalesPrice(ipsprice_price) AS price"
             "       FROM ipsass, ipshead, ipsprice, custtype, cust "
             "       WHERE ( (ipsass_ipshead_id=ipshead_id)"
             "        AND (ipsprice_ipshead_id=ipshead_id)"
             "        AND (ipsprice_item_id=:item_id)"
             "        AND (coalesce(length(ipsass_custtype_pattern), 0) > 0)"
             "        AND (custtype_code ~ ipsass_custtype_pattern)"
             "        AND (cust_custtype_id=custtype_id)"
             "        AND (cust_id=:cust_id)"
             "        AND (CURRENT_DATE BETWEEN ipshead_effective AND (ipshead_expires - 1)))"
             
             "       UNION SELECT formatSalesPrice(ipsprice_price) AS price"
             "       FROM ipsass, ipshead, ipsprice, shipto "
             "       WHERE ( (ipsass_ipshead_id=ipshead_id)"
             "        AND (ipsprice_ipshead_id=ipshead_id)"
             "        AND (ipsprice_item_id=:item_id)"
             "        AND (shipto_id=:shipto_id)"
             "        AND (COALESCE(LENGTH(ipsass_shipto_pattern), 0) > 0)"
             "        AND (shipto_num ~ ipsass_shipto_pattern)"
             "        AND (ipsass_cust_id=:cust_id)"
             "        AND (CURRENT_DATE BETWEEN ipshead_effective AND (ipshead_expires - 1)) )"

             "       UNION SELECT formatSalesPrice(ipsprice_price) AS price"
             "       FROM sale, ipshead, ipsprice "
             "       WHERE ((sale_ipshead_id=ipshead_id)"
             "        AND (ipsprice_ipshead_id=ipshead_id)"
             "        AND (ipsprice_item_id=:item_id)"
             "        AND (CURRENT_DATE BETWEEN sale_startdate AND (sale_enddate - 1)) ) "

             "       UNION SELECT formatSalesPrice(item_listprice - (item_listprice * cust_discntprcnt)) AS price "
             "       FROM item, cust "
             "       WHERE ( (item_sold)"
             "        AND (NOT item_exclusive)"
             "        AND (item_id=:item_id)"
             "        AND (cust_id=:cust_id) );");
  q.bindValue(":item_id", _item->id());
  q.bindValue(":cust_id", _custid);
  q.bindValue(":shipto_id", _shiptoid);
  q.bindValue(":curr_id", _saleNetUnitPrice->id());
  q.bindValue(":effective", _saleNetUnitPrice->effective());
  q.exec();
  if (q.size() == 1)
  {
    q.first();
    _saleNetUnitPrice->setLocalValue(q.value("price").toDouble() * (_priceRatio / _priceinvuomratio));
  }
  else
  {
    ParameterList params;
    params.append("cust_id", _custid);
    params.append("shipto_id", _shiptoid);
    params.append("item_id", _item->id());
    // don't params.append("qty", ...) as we don't know how many were purchased
    params.append("curr_id", _saleNetUnitPrice->id());
    params.append("effective", _saleNetUnitPrice->effective());

    priceList newdlg(this);
    newdlg.set(params);
    if (newdlg.exec() == XDialog::Accepted)
    {
      _saleNetUnitPrice->setLocalValue(newdlg._selectedPrice * (_priceRatio / _priceinvuomratio));
      sCalculateSaleDiscountPrcnt();
    }
  }
}

void returnAuthorizationItem::sCalculateTax()
{
  XSqlQuery calcq;

  calcq.prepare("SELECT calculateTax(rahead_taxzone_id,:taxtype_id,rahead_authdate,rahead_curr_id,ROUND(:ext,2)) AS tax "
                "FROM rahead "
                "WHERE (rahead_id=:rahead_id); " );

  calcq.bindValue(":rahead_id", _raheadid);
  calcq.bindValue(":taxtype_id", _taxType->id());
  calcq.bindValue(":ext", _extendedPrice->localValue());
  calcq.exec();
  if (calcq.first())
    _tax->setLocalValue(calcq.value("tax").toDouble());
  else if (calcq.lastError().type() != QSqlError::NoError)
  {
    systemError(this, calcq.lastError().databaseText(), __FILE__, __LINE__);
    return;
  } 
}

void returnAuthorizationItem::sTaxDetail()
{
  taxDetail newdlg(this, "", true);
  ParameterList params;
  params.append("taxzone_id",   _taxzoneid);
  params.append("taxtype_id",  _taxType->id());
  params.append("date", _netUnitPrice->effective());
  params.append("curr_id", _netUnitPrice->id());
  params.append("subtotal", _extendedPrice->localValue());
  params.append("readOnly");

  if (newdlg.set(params) == NoError && newdlg.exec())
  {
    if (_taxType->id() != newdlg.taxtype())
      _taxType->setId(newdlg.taxtype());
  }
}

void returnAuthorizationItem::sQtyUOMChanged()
{
  if(_qtyUOM->id() == _invuomid || _item->id() == -1)
    _qtyinvuomratio = 1.0;
  else
  {
    XSqlQuery invuom;
    invuom.prepare("SELECT itemuomtouomratio(item_id, :uom_id, item_inv_uom_id) AS ratio"
                   "  FROM item"
                   " WHERE(item_id=:item_id);");
    invuom.bindValue(":item_id", _item->id());
    invuom.bindValue(":uom_id", _qtyUOM->id());
    invuom.exec();
    if(invuom.first())
      _qtyinvuomratio = invuom.value("ratio").toDouble();
    else
      systemError(this, invuom.lastError().databaseText(), __FILE__, __LINE__);
  }

  if(_qtyUOM->id() != _invuomid)
  {
    _pricingUOM->setId(_qtyUOM->id());
    _pricingUOM->setEnabled(false);
    _salePricingUOM->setId(_qtyUOM->id());
    _salePricingUOM->setEnabled(false);
  }
  else
  {
    _pricingUOM->setEnabled(true);
    _salePricingUOM->setEnabled(true);
  }
  sCalculateExtendedPrice(); 
  sCalculateSaleExtendedPrice(); 
}

void returnAuthorizationItem::sPriceUOMChanged()
{
  if(_pricingUOM->id() == -1 || _qtyUOM->id() == -1)
    return;

  if(_pricingUOM->id() == _invuomid)
    _priceinvuomratio = 1.0;
  else
  {
    XSqlQuery invuom;
    invuom.prepare("SELECT itemuomtouomratio(item_id, :uom_id, item_inv_uom_id) AS ratio"
                   "  FROM item"
                   " WHERE(item_id=:item_id);");
    invuom.bindValue(":item_id", _item->id());
    invuom.bindValue(":uom_id", _pricingUOM->id());
    invuom.exec();
    if(invuom.first())
      _priceinvuomratio = invuom.value("ratio").toDouble();
    else
      systemError(this, invuom.lastError().databaseText(), __FILE__, __LINE__);
  }
  //_ratio->setText(formatUOMRatio(_priceinvuomratio));

  updatePriceInfo(); 
}

void returnAuthorizationItem::updatePriceInfo()
{
  XSqlQuery item;
  item.prepare("SELECT item_listprice"
               "  FROM item"
               " WHERE(item_id=:item_id);");
  item.bindValue(":item_id", _item->id());
  item.exec();
  item.first();
  _listPrice->setBaseValue((item.value("item_listprice").toDouble() * _priceRatio)  * _priceinvuomratio);
  sCalculateExtendedPrice();
} 

void returnAuthorizationItem::sDispositionChanged()
{
  if ( (_disposition->currentIndex() == 3) && // service
       (_item->id() != -1) &&
       (_costmethod != "J") )
  {
    QMessageBox::warning( this, tr("Cannot use Service Disposition"),
                          tr("<p>Only Items Sites using the Job cost method may have a Disposition of Service.") );
    _disposition->setFocus();
    _disposition->setCurrentIndex(_dispositionCache);
    return;
  }
  else if (_disposition->currentIndex() == 3)
  {
    _shipWhs->setId(_warehouse->id());
    _shipWhs->setEnabled(false);
  }
  else if ( (_disposition->currentIndex() < 2) && // credit or return
       (_orderId != -1) )
  {
    QMessageBox::warning( this, tr("Cannot change Disposition"),
                          tr("<p>A work order is associated with this Return. "
                             "First delete the work order, then change this disposition.") );
    _disposition->setFocus();
    _disposition->setCurrentIndex(_dispositionCache);
    return;
  }
  else if (_disposition->currentIndex() < 2)
  {
    _shipWhs->setId(_warehouse->id());
    _shipWhs->setVisible(false);
  }
  else
  {
    _shipWhs->setVisible(true);
    _shipWhs->setEnabled(true);
  }

  if (_mode == cNew)
      _item->addExtraClause( QString("(NOT item_exclusive OR customerCanPurchase(item_id, %1, %2))").arg(_custid).arg(_shiptoid) );

  if (_disposition->currentIndex() > 2) // service or ship
  {
    _netUnitPrice->setLocalValue(0);
    _netUnitPrice->setEnabled(FALSE);
    _listPrices->setEnabled(FALSE);
    _pricingUOM->setEnabled(FALSE);
    _discountFromSale->setEnabled(FALSE);
  }
  else
  {
    _netUnitPrice->setEnabled(TRUE);
    _listPrices->setEnabled(TRUE);
    _pricingUOM->setEnabled(TRUE);
    _discountFromSale->setEnabled(TRUE);
  } 
  
  if (_disposition->currentIndex() >= 2) // replace, service, or ship
  {
    _tab->setTabEnabled(_tab->indexOf(_supply),TRUE);
    _scheduledDate->setEnabled(TRUE);
    _altcosAccntid->setEnabled(TRUE);
    _shipWhsLit->setVisible(TRUE);
    _shipWhs->setVisible(TRUE);
    _saleNetUnitPrice->setEnabled(TRUE);
    _saleListPrices->setEnabled(TRUE);
    _salePricingUOM->setEnabled(TRUE);
    _saleDiscountFromSale->setEnabled(TRUE);
  }
  else
  {
    _tab->setTabEnabled(_tab->indexOf(_supply),FALSE);
    _scheduledDate->clear();
    _scheduledDate->setEnabled(FALSE);
    _altcosAccntid->setEnabled(TRUE);
    _shipWhsLit->setVisible(FALSE);
    _shipWhs->setVisible(FALSE);
    _saleNetUnitPrice->setLocalValue(0);
    _saleNetUnitPrice->setEnabled(FALSE);
    _saleListPrices->setEnabled(FALSE);
    _salePricingUOM->setEnabled(FALSE);
    _saleDiscountFromSale->setEnabled(FALSE);
  } 

  if (_creditmethod == "N")
  {
    _netUnitPrice->setLocalValue(0);
    _netUnitPrice->setEnabled(FALSE);
    _listPrices->setEnabled(FALSE);
    _pricingUOM->setEnabled(FALSE);
    _discountFromSale->setEnabled(FALSE); 
    disconnect(_item, SIGNAL(valid(bool)), _listPrices, SLOT(setEnabled(bool)));
  }

  _dispositionCache = _disposition->currentIndex();
  sPopulateItemInfo();
  sPopulateItemsiteInfo();
}

void returnAuthorizationItem::sHandleWo(bool pCreate)
{
  if (pCreate)
  {
    if (_orderId == -1)
      sPopulateOrderInfo();
  }

  else
  {
    if (_orderId != -1)
    {
      XSqlQuery query;

      if (_item->itemType() == "M")
      {
        if (QMessageBox::question(this, tr("Delete Work Order"),
                                  tr("<p>You are requesting to delete the Work "
                                     "Order created for the Sales Order Item linked to this Return. "
                                     "Are you sure you want to do this?"),
                                  QMessageBox::Yes | QMessageBox::Default,
                                  QMessageBox::No | QMessageBox::Escape) == QMessageBox::Yes)
        {
          query.prepare("UPDATE coitem SET coitem_order_id=-1,coitem_order_type=NULL "
                        "FROM wo "
                        "WHERE ((coitem_id=wo_ordid) "
                        "AND (wo_id=:wo_id)); "
                        "SELECT deleteWo(:wo_id, TRUE) AS result;");
          query.bindValue(":wo_id", _orderId);
          query.exec();
          if (query.first())
          {
            int result = query.value("result").toInt();
            if (result < 0)
            {
              systemError(this, storedProcErrorLookup("deleteWo", result),
                          __FILE__, __LINE__);
              _createOrder->setChecked(true); // if (pCreate) => won't recurse
              return;
            }
            else
            {
              _orderId = -1;
              _orderQty->clear();
              _orderDueDate->clear();
              _orderStatus->clear();
              if (_qtyReceived->toDouble() == 0 && 
                  _qtyShipped->toDouble() == 0 && 
                  _qtycredited == 0 &&
                  _status == "O")
                _disposition->setEnabled(TRUE); 

              _createOrder->setChecked(FALSE);
            }
          }
          else if (query.lastError().type() != QSqlError::NoError)
          {
            systemError(this, query.lastError().databaseText(),
                        __FILE__, __LINE__);
            _createOrder->setChecked(true); // if (pCreate) => won't recurse
            return;
          }

          omfgThis->sWorkOrdersUpdated(-1, TRUE);
        }
        else
          _createOrder->setChecked(TRUE);
      }
    }
  }
}

void returnAuthorizationItem::sPopulateOrderInfo()
{
  if (_createOrder->isChecked())
  {
    _orderDueDate->setDate(_scheduledDate->date());

    if (_createOrder->isChecked())
    {
      XSqlQuery qty;
      qty.prepare( "SELECT validateOrderQty(itemsite_id, :qty, TRUE) AS qty "
                   "FROM itemsite "
                   "WHERE ((itemsite_item_id=:item_id)"
                   " AND (itemsite_warehous_id=:warehous_id));" );
      qty.bindValue(":qty", _qtyAuth->toDouble() * _qtyinvuomratio);
      qty.bindValue(":item_id", _item->id());
      qty.bindValue(":warehous_id", ((_item->itemType() == "M") ? _shipWhs->id() : _shipWhs->id()));
      qty.exec();
      if (qty.first())
        _orderQty->setDouble(qty.value("qty").toDouble());

      else if (qty.lastError().type() != QSqlError::NoError)
      {
        systemError(this, qty.lastError().databaseText(), __FILE__, __LINE__);
        return;
      }
    }
  }
}

void returnAuthorizationItem::sDetermineAvailability()
{
  if(  (_item->id()==_availabilityLastItemid)
    && (_shipWhs->id()==_availabilityLastWarehousid)
    && (_scheduledDate->date()==_availabilityLastSchedDate)
    && (_showAvailability->isChecked()==_availabilityLastShow)
    && ((_qtyAuth->toDouble() * _qtyinvuomratio)==_availabilityQtyOrdered) )
    return;

  _availabilityLastItemid = _item->id();
  _availabilityLastWarehousid = _shipWhs->id();
  _availabilityLastSchedDate = _scheduledDate->date();
  _availabilityLastShow = _showAvailability->isChecked();
  _availabilityQtyOrdered = (_qtyAuth->toDouble() * _qtyinvuomratio);

  if ((_item->isValid()) && (_scheduledDate->isValid()) && (_showAvailability->isChecked()) )
  {
    XSqlQuery availability;
    availability.prepare( "SELECT itemsite_id,"
                          "       qoh,"
                          "       allocated,"
                          "       noNeg(qoh - allocated) AS unallocated,"
                          "       ordered,"
                          "       (qoh - allocated + ordered) AS available,"
                          "       itemsite_leadtime "
                          "FROM ( SELECT itemsite_id, itemsite_qtyonhand AS qoh,"
                          "              qtyAllocated(itemsite_id, DATE(:date)) AS allocated,"
                          "              qtyOrdered(itemsite_id, DATE(:date)) AS ordered, "
                          "              itemsite_leadtime "
                          "       FROM itemsite, item "
                          "       WHERE ((itemsite_item_id=item_id)"
                          "        AND (item_id=:item_id)"
                          "        AND (itemsite_warehous_id=:warehous_id)) ) AS data;" );
    availability.bindValue(":date", _scheduledDate->date());
    availability.bindValue(":item_id", _item->id());
    availability.bindValue(":warehous_id", _shipWhs->id());
    availability.exec();
    if (availability.first())
    {
      _onHand->setDouble(availability.value("qoh").toDouble());
      _allocated->setDouble(availability.value("allocated").toDouble());
      _unallocated->setDouble(availability.value("unallocated").toDouble());
      _onOrder->setDouble(availability.value("ordered").toDouble());
      _available->setDouble(availability.value("available").toDouble());
      _leadtime->setText(availability.value("itemsite_leadtime").toString());

      QString stylesheet;
      if (availability.value("available").toDouble() < _availabilityQtyOrdered)
        stylesheet = QString("* { color: %1; }").arg(namedColor("error").name());
      _available->setStyleSheet(stylesheet);
    }
    else if (availability.lastError().type() != QSqlError::NoError)
    {
      systemError(this, availability.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }
  else
  {
    _onHand->clear();
    _allocated->clear();
    _unallocated->clear();
    _onOrder->clear();
    _available->clear();
    _leadtime->clear();
  }
}

void returnAuthorizationItem::sCalcWoUnitCost()
{
  if (_costmethod == "J" && _orderId > -1 && _qtyAuth->toDouble() != 0)
  {
    q.prepare("SELECT COALESCE(SUM(wo_postedvalue),0) AS wo_value "
              "FROM wo,raitem "
              "WHERE ((wo_ordtype='S') "
              "  AND  (wo_ordid=raitem_new_coitem_id) "
              "  AND  (raitem_id=:raitem_id));");
	q.bindValue(":raitem_id", _raitemid);
	q.exec();
	if (q.first())
      _unitCost->setBaseValue(q.value("wo_value").toDouble() / _qtyAuth->toDouble() * _qtyinvuomratio);
  }
}

void returnAuthorizationItem::sNew()
{
	if (sSave())
	{
		ParameterList params;
		params.append("raitem_id", _raitemid);
		params.append("item_id", _item->id());
		params.append("warehouse_id", _warehouse->id());
		params.append("uom", _qtyUOM->currentText());
		params.append("mode", "new");

		returnAuthItemLotSerial newdlg(this, "", TRUE);
		newdlg.set(params);
		int raitemlsid = newdlg.exec();
		if (raitemlsid > 0)
			populate();
		else if (raitemlsid == -1)
			reject();
	}
}

void returnAuthorizationItem::sEdit()
{
	if (sSave())
	{
		bool fill;
		fill = FALSE;
		QList<XTreeWidgetItem*> selected = _raitemls->selectedItems();
		for (int i = 0; i < selected.size(); i++)
		{
			ParameterList params;
      params.append("raitem_id", _raitemid);			
      params.append("item_id", _item->id());
      params.append("warehouse_id", _warehouse->id());
      params.append("uom", _qtyUOM->currentText());
      params.append("ls_id", ((XTreeWidgetItem*)(selected[i]))->altId());

			if (_mode==cView)
				params.append("mode", "view");
			else
				params.append("mode", "edit");

			returnAuthItemLotSerial newdlg(this, "", TRUE);
			newdlg.set(params);
	  
			int raitemlsid = newdlg.exec();
			if (raitemlsid > 0)
				fill = TRUE;
			else if (raitemlsid == -1)
				reject();
		}
		if (fill)
			populate();
	}
}

void returnAuthorizationItem::sDelete()
{
  QList<XTreeWidgetItem*> selected = _raitemls->selectedItems();
  for (int i = 0; i < selected.size(); i++)
  {
        QString sql ( "DELETE FROM raitemls WHERE (raitemls_id=:raitemls_id);" );
        q.prepare(sql);
        q.bindValue(":raitemls_id",  ((XTreeWidgetItem*)(selected[i]))->id());
        q.exec();
        if (q.lastError().type() != QSqlError::NoError)
        {
           systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
           reject();
        }
  }
  populate(); 
}

void returnAuthorizationItem::sFillList()
{ 
  q.prepare("SELECT raitemls_id,ls_id,ls_number, "
            "       MAX(lsreg_expiredate) AS lsreg_expiredate, "
            "       COALESCE(SUM(lsreg_qty / raitem_qty_invuomratio),0) AS raitemls_qtyregistered, "
            "       raitemls_qtyauthorized, raitemls_qtyreceived, "
            "      'qty' AS raitemls_qtyregistered_xtnumericrole, "
            "      'qty' AS raitemls_qtyauthorized_xtnumericrole, "
            "      'qty' AS raitemls_qtyreceived_xtnumericrole "
            "FROM raitemls "
            "  LEFT OUTER JOIN lsreg ON (lsreg_ls_id=raitemls_ls_id) "
            "                       AND (lsreg_crmacct_id=:crmacct_id), "
            "  ls, raitem "
            "WHERE ( (raitemls_raitem_id=raitem_id) "
            "  AND   (raitem_id=:raitem_id) "
            "  AND   (raitemls_ls_id=ls_id) ) "
            "GROUP BY raitemls_id,ls_id,ls_number,raitemls_qtyauthorized,"
            "         raitemls_qtyreceived "
            "ORDER BY ls_number;" );
  q.bindValue(":raitem_id", _raitemid);
  q.bindValue(":crmacct_id", _crmacctid);
  q.bindValue(":na", tr("N/A"));
  q.exec();
  _raitemls->populate(q,true);
  _authLotSerial->setDisabled(q.first());
  _authLotSerial->setChecked(q.first());
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    reject();
  }
}

void returnAuthorizationItem::rejectEvent()
{
  q.exec("ROLLBACK");
}



