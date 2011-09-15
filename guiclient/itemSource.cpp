/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "itemSource.h"

#include <QAction>
#include <QCloseEvent>
#include <QMenu>
#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include "crmacctcluster.h"
#include "itemSourcePrice.h"
#include "xcombobox.h"
#include <metasql.h>
#include <parameter.h>
#include "mqlutil.h"

itemSource::itemSource(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_add,                SIGNAL(clicked()), this, SLOT(sAdd()));
  connect(_close,              SIGNAL(clicked()), this, SLOT(reject()));
  connect(_delete,             SIGNAL(clicked()), this, SLOT(sDelete()));
  connect(_edit,               SIGNAL(clicked()), this, SLOT(sEdit()));
  connect(_itemsrcp,SIGNAL(populateMenu(QMenu*, XTreeWidgetItem*)), this, SLOT(sPopulateMenu(QMenu*)));
  connect(_save,               SIGNAL(clicked()), this, SLOT(sSaveClicked()));
  connect(_vendor,            SIGNAL(newId(int)), this, SLOT(sVendorChanged(int)));
  connect(_vendorCurrency,    SIGNAL(newID(int)), this, SLOT(sFillPriceList()));
  connect(this,               SIGNAL(rejected()), this, SLOT(sRejected()));

//  TODO method doesn't exist?
//  connect(_vendorUOM, SIGNAL(textChanged()), this, SLOT(sClearVendorUOM()));
//  connect(_invVendorUOMRatio, SIGNAL(textChanged(QString)), this, SLOT(sClearVendorUOM()));


  _item->setType(ItemLineEdit::cGeneralPurchased | ItemLineEdit::cGeneralManufactured | ItemLineEdit::cTooling);
  _item->setDefaultType(ItemLineEdit::cGeneralPurchased);

  _captive = false;
  _new = false;
  
  QString base;
  q.exec("SELECT currConcat(baseCurrID()) AS base;");
  if (q.first())
    base = q.value("base").toString();
  else
    base = tr("Base");

  _itemsrcp->addColumn(tr("Qty Break"),                   _qtyColumn, Qt::AlignRight,true, "itemsrcp_qtybreak");
  _itemsrcp->addColumn(tr("Unit Price"),                          -1, Qt::AlignRight,true, "itemsrcp_price");
  _itemsrcp->addColumn(tr("Currency"),               _currencyColumn, Qt::AlignLeft, true, "currabbr");
  _itemsrcp->addColumn(tr("Unit Price\n(%1)").arg(base),_moneyColumn, Qt::AlignRight,true, "itemsrcp_price_base");
  
  if (omfgThis->singleCurrency())
  {
    _itemsrcp->hideColumn(1);
    _itemsrcp->hideColumn(2);
    _itemsrcp->headerItem()->setText(3, tr("Unit Price"));
  }

  _invVendorUOMRatio->setValidator(omfgThis->ratioVal());
  _minOrderQty->setValidator(omfgThis->qtyVal());
  _multOrderQty->setValidator(omfgThis->qtyVal());

  _vendorCurrency->setType(XComboBox::Currencies);
  _vendorCurrency->setLabel(_vendorCurrencyLit);
  
  q.exec("SELECT MAX(itemsrc_id),itemsrc_manuf_name, itemsrc_manuf_name FROM itemsrc GROUP BY itemsrc_manuf_name;");
  _manufName->populate(q);
  _manufName->setCurrentIndex(0);
}

itemSource::~itemSource()
{
    // no need to delete child widgets, Qt does it all for us
}

void itemSource::languageChange()
{
    retranslateUi(this);
}

enum SetResponse itemSource::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("itemsrc_id", &valid);
  if (valid)
  {
    _itemsrcid = param.toInt();
    populate();
  }

  param = pParams.value("item_id", &valid);
  if (valid)
  {
    _item->setId(param.toInt());
    _item->setEnabled(FALSE);
  }

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      _mode = cNew;
      _new = true;

      q.exec("SELECT NEXTVAL('itemsrc_itemsrc_id_seq') AS _itemsrc_id;");
      if (q.first())
        _itemsrcid = q.value("_itemsrc_id").toInt();
      else if (q.lastError().type() != QSqlError::NoError)
      {
        systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
        return UndefinedError;
      }
      _captive = true;
      
      connect(_itemsrcp, SIGNAL(valid(bool)), _edit, SLOT(setEnabled(bool)));
      connect(_itemsrcp, SIGNAL(valid(bool)), _delete, SLOT(setEnabled(bool)));
      connect(_itemsrcp, SIGNAL(itemSelected(int)), _edit, SLOT(animateClick()));

      _item->setFocus();
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;

      connect(_itemsrcp, SIGNAL(valid(bool)), _edit, SLOT(setEnabled(bool)));
      connect(_itemsrcp, SIGNAL(valid(bool)), _delete, SLOT(setEnabled(bool)));
      connect(_itemsrcp, SIGNAL(itemSelected(int)), _edit, SLOT(animateClick()));

      _item->setReadOnly(TRUE);
      _vendor->setEnabled(FALSE);

      _save->setFocus();
    }
    else if (param.toString() == "view")
    {
      _mode = cView;

      _item->setReadOnly(TRUE);
      _active->setEnabled(FALSE);
	  _default->setEnabled(FALSE);
      _vendor->setEnabled(FALSE);
      _vendorItemNumber->setEnabled(FALSE);
      _vendorItemDescrip->setEnabled(FALSE);
      _vendorUOM->setEnabled(FALSE);
      _invVendorUOMRatio->setEnabled(FALSE);
      _vendorRanking->setEnabled(FALSE);
      _minOrderQty->setEnabled(FALSE);
      _multOrderQty->setEnabled(FALSE);
      _leadTime->setEnabled(FALSE);
      _notes->setEnabled(FALSE);
      _upcCode->setEnabled(FALSE);
      _add->setEnabled(FALSE);
      _delete->setEnabled(FALSE);
      _close->setText(tr("&Close"));
      _save->hide();

      _close->setFocus();
    }
    if (param.toString() == "copy")
    {
      _mode = cCopy;
      _new = true;
      _captive = true;
      int itemsrcidold = _itemsrcid;

      q.exec("SELECT NEXTVAL('itemsrc_itemsrc_id_seq') AS _itemsrc_id;");
      if (q.first())
        _itemsrcid = q.value("_itemsrc_id").toInt();
      else if (q.lastError().type() != QSqlError::NoError)
      {
        systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
        return UndefinedError;
      }
      
      connect(_itemsrcp, SIGNAL(valid(bool)), _edit, SLOT(setEnabled(bool)));
      connect(_itemsrcp, SIGNAL(valid(bool)), _delete, SLOT(setEnabled(bool)));
      connect(_itemsrcp, SIGNAL(itemSelected(int)), _edit, SLOT(animateClick()));
      
      _item->setReadOnly(TRUE);
      _vendorItemNumber->setText(_vendorItemNumber->text().prepend("Copy Of "));
      
      if (sSave())
      {
        q.prepare("INSERT INTO itemsrcp ( "
                  "itemsrcp_itemsrc_id, itemsrcp_qtybreak, "
                  "itemsrcp_price, itemsrcp_updated, itemsrcp_curr_id) "
                  "SELECT :itemsrcid, itemsrcp_qtybreak, "
                  "itemsrcp_price, current_date, itemsrcp_curr_id "
                  "FROM itemsrcp "
                  "WHERE (itemsrcp_itemsrc_id=:itemsrcidold); ");
        q.bindValue(":itemsrcid", _itemsrcid);
        q.bindValue(":itemsrcidold", itemsrcidold);
        q.exec();
        sFillPriceList();
      }
    }
  }

  return NoError;
}

void itemSource::sSaveClicked()
{
  _captive = false;
  sSave();
}

bool itemSource::sSave()
{
  if (!_item->isValid())
  {
    QMessageBox::critical( this, tr("Cannot Save Item Source"),
                           tr( "You must select an Item that this Item Source represents\n"
                               "before you may save this Item Source." ) );
    _item->setFocus();
    return false;
  }

  if (!_vendor->isValid())
  {
    QMessageBox::critical( this, tr("Cannot Save Item Source"),
                           tr( "You must select this Vendor that this Item Source is sold by\n"
                               "before you may save this Item Source." ) );
    _item->setFocus();
    return false;
  }

  if(_mode == cNew || _mode == cCopy)
  {
    q.prepare( "SELECT itemsrc_id "
               "  FROM itemsrc "
               " WHERE ((itemsrc_item_id=:item_id) "
               "   AND (itemsrc_vend_id=:vend_id) "
               "   AND (itemsrc_vend_item_number=:itemsrc_vend_item_number) "
               "   AND (UPPER(itemsrc_manuf_name)=UPPER(:itemsrc_manuf_name)) "
               "   AND (UPPER(itemsrc_manuf_item_number)=UPPER(:itemsrc_manuf_item_number)) ) ");
    q.bindValue(":item_id", _item->id());
    q.bindValue(":vend_id", _vendor->id());
    q.bindValue(":itemsrc_vend_item_number", _vendorItemNumber->text());
    q.bindValue(":itemsrc_manuf_name", _manufName->currentText());
    q.bindValue(":itemsrc_manuf_item_number", _manufItemNumber->text());
    q.exec();
    if(q.first())
    {
      QMessageBox::critical( this, tr("Cannot Save Item Source"),
                            tr("An Item Source already exists for the Item Number, Vendor, Vendor Item, Manfacturer Name and Manufacturer Item Number you have specified.\n"));
      return false;
    }
    else if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return false;
    }
  }

  if (_vendorUOM->text().length() == 0)
  {
    QMessageBox::critical( this, tr("Cannot Save Item Source"),
                          tr( "You must indicate the Unit of Measure that this Item Source is sold in\n"
                               "before you may save this Item Source." ) );
    _vendorUOM->setFocus();
    return false;
  }

  if (_invVendorUOMRatio->toDouble() == 0.0)
  {
    QMessageBox::critical( this, tr("Cannot Save Item Source"),
                          tr( "You must indicate the Ratio of Inventory to Vendor Unit of Measures\n"
                               "before you may save this Item Source." ) );
    _invVendorUOMRatio->setFocus();
    return false;
  }

  if(_active->isChecked())
  {
    q.prepare("SELECT item_id "
              "FROM item "
              "WHERE ((item_id=:item_id)"
              "  AND  (item_active)) "
              "LIMIT 1; ");
    q.bindValue(":item_id", _item->id());
    q.exec();
    if (!q.first())         
    { 
      QMessageBox::warning( this, tr("Cannot Save Item Source"),
        tr("This Item Source refers to an inactive Item and must be marked as inactive.") );
      return false;
    }
  }
    
  if (_mode == cNew || _mode == cCopy)
    q.prepare( "INSERT INTO itemsrc "
               "( itemsrc_id, itemsrc_item_id, itemsrc_active, itemsrc_default, itemsrc_vend_id,"
               "  itemsrc_vend_item_number, itemsrc_vend_item_descrip,"
               "  itemsrc_vend_uom, itemsrc_invvendoruomratio,"
               "  itemsrc_minordqty, itemsrc_multordqty, itemsrc_upccode,"
               "  itemsrc_leadtime, itemsrc_ranking,"
               "  itemsrc_comments, itemsrc_manuf_name, "
               "  itemsrc_manuf_item_number, itemsrc_manuf_item_descrip ) "
               "VALUES "
               "( :itemsrc_id, :itemsrc_item_id, :itemsrc_active, :itemsrc_default, :itemsrc_vend_id,"
               "  :itemsrc_vend_item_number, :itemsrc_vend_item_descrip,"
               "  :itemsrc_vend_uom, :itemsrc_invvendoruomratio,"
               "  :itemsrc_minordqty, :itemsrc_multordqty, :itemsrc_upccode,"
               "  :itemsrc_leadtime, :itemsrc_ranking,"
               "  :itemsrc_comments, :itemsrc_manuf_name, "
               "  :itemsrc_manuf_item_number, :itemsrc_manuf_item_descrip );" );
  if (_mode == cEdit)
    q.prepare( "UPDATE itemsrc "
               "SET itemsrc_active=:itemsrc_active,"
			   "    itemsrc_default=:itemsrc_default,"
               "    itemsrc_vend_id=:itemsrc_vend_id,"
               "    itemsrc_vend_item_number=:itemsrc_vend_item_number,"
               "    itemsrc_vend_item_descrip=:itemsrc_vend_item_descrip,"
               "    itemsrc_vend_uom=:itemsrc_vend_uom,"
               "    itemsrc_invvendoruomratio=:itemsrc_invvendoruomratio,"
			   "    itemsrc_upccode=:itemsrc_upccode,"
               "    itemsrc_minordqty=:itemsrc_minordqty, itemsrc_multordqty=:itemsrc_multordqty,"
               "    itemsrc_leadtime=:itemsrc_leadtime, itemsrc_ranking=:itemsrc_ranking,"
               "    itemsrc_comments=:itemsrc_comments, itemsrc_manuf_name=:itemsrc_manuf_name, "
               "    itemsrc_manuf_item_number=:itemsrc_manuf_item_number, "
               "    itemsrc_manuf_item_descrip=:itemsrc_manuf_item_descrip "
               "WHERE (itemsrc_id=:itemsrc_id);" );

  q.bindValue(":itemsrc_id", _itemsrcid);
  q.bindValue(":itemsrc_item_id", _item->id());
  q.bindValue(":itemsrc_active", QVariant(_active->isChecked()));
  q.bindValue(":itemsrc_default", QVariant(_default->isChecked()));
  q.bindValue(":itemsrc_vend_id", _vendor->id());
  q.bindValue(":itemsrc_vend_item_number", _vendorItemNumber->text());
  q.bindValue(":itemsrc_vend_item_descrip", _vendorItemDescrip->toPlainText());
  q.bindValue(":itemsrc_vend_uom", _vendorUOM->text().trimmed());
  q.bindValue(":itemsrc_invvendoruomratio", _invVendorUOMRatio->toDouble());
  q.bindValue(":itemsrc_upccode", _upcCode->text());
  q.bindValue(":itemsrc_minordqty", _minOrderQty->toDouble());
  q.bindValue(":itemsrc_multordqty", _multOrderQty->toDouble());
  q.bindValue(":itemsrc_leadtime", _leadTime->text().toInt());
  q.bindValue(":itemsrc_ranking", _vendorRanking->value());
  q.bindValue(":itemsrc_comments", _notes->toPlainText().trimmed());
  q.bindValue(":itemsrc_manuf_name", _manufName->currentText());
  q.bindValue(":itemsrc_manuf_item_number", _manufItemNumber->text());
  q.bindValue(":itemsrc_manuf_item_descrip", _manufItemDescrip->toPlainText());
  q.exec();
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return false;
  }

  if (_captive)
  {
    if (_mode != cCopy)
    {
      _vendor->setEnabled(FALSE);
    }
    _mode = cEdit;
    _item->setReadOnly(TRUE);
    _captive = false;
  }
  else
    done(_itemsrcid);
    
  return true;
}

void itemSource::sAdd()
{
  if (_mode == cNew || _mode == cCopy)
  {
    if (!sSave())
      return;
  }
  
  ParameterList params;
  params.append("mode", "new");
  params.append("itemsrc_id", _itemsrcid);
  params.append("curr_id", _vendorCurrency->id());

  itemSourcePrice newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillPriceList();
}

void itemSource::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("itemsrcp_id", _itemsrcp->id());

  itemSourcePrice newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillPriceList();
}

void itemSource::sDelete()
{
//  Make sure the user is sure
  if (  QMessageBox::warning( this, tr("Delete Item Source Price"),
                              tr("Are you sure you want to delete this Item Source price?"),
                              tr("&Delete"), tr("&Cancel"), 0, 0, 1)  == 0  )
  {
    q.prepare( "DELETE FROM itemsrcp "
               "WHERE (itemsrcp_id=:itemsrcp_id);" );
    q.bindValue(":itemsrcp_id", _itemsrcp->id());
    q.exec();
    if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }

    sFillPriceList();
  }
}

void itemSource::sPopulateMenu(QMenu *pMenu)
{
  QAction *menuItem;

  menuItem = pMenu->addAction("Edit Item Source Price...", this, SLOT(sEdit()));
  menuItem->setEnabled(_privileges->check("MaintainItemSources"));

  menuItem = pMenu->addAction("Delete Item Source Price...", this, SLOT(sDelete()));
  menuItem->setEnabled(_privileges->check("MaintainItemSources"));
}

void itemSource::sFillPriceList()
{
  XSqlQuery priceq;
  priceq.prepare("SELECT *,"
                 " currConcat(itemsrcp_curr_id) AS currabbr, "
                 " currToBase(itemsrcp_curr_id, itemsrcp_price,"
                 "            itemsrcp_updated) AS itemsrcp_price_base,"
                 " 'qty' AS itemsrcp_qtybreak_xtnumericrole,"
                 " 'purchprice' AS itemsrcp_price_xtnumericrole,"
                 " 'purchprice' AS itemsrcp_price_base_xtnumericrole "
                 "FROM itemsrcp "
                 "WHERE (itemsrcp_itemsrc_id=:itemsrc_id) "
                 "ORDER BY itemsrcp_qtybreak;" );
  priceq.bindValue(":itemsrc_id", _itemsrcid);
  priceq.exec();
  _itemsrcp->populate(priceq);
  if (priceq.lastError().type() != QSqlError::NoError)
  {
    systemError(this, priceq.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void itemSource::populate()
{
  XSqlQuery itemsrcQ;
  itemsrcQ.prepare( "SELECT * "
             "FROM itemsrc "
             "WHERE (itemsrc_id=:itemsrc_id);" );
  itemsrcQ.bindValue(":itemsrc_id", _itemsrcid);
  itemsrcQ.exec();
  if (itemsrcQ.first())
  {
    _item->setId(itemsrcQ.value("itemsrc_item_id").toInt());
    _active->setChecked(itemsrcQ.value("itemsrc_active").toBool());
	_default->setChecked(itemsrcQ.value("itemsrc_default").toBool());
    _vendor->setId(itemsrcQ.value("itemsrc_vend_id").toInt());
    _vendorItemNumber->setText(itemsrcQ.value("itemsrc_vend_item_number").toString());
    _vendorItemDescrip->setText(itemsrcQ.value("itemsrc_vend_item_descrip").toString());
    _vendorUOM->setText(itemsrcQ.value("itemsrc_vend_uom").toString());
    _invVendorUOMRatio->setDouble(itemsrcQ.value("itemsrc_invvendoruomratio").toDouble());
    _upcCode->setText(itemsrcQ.value("itemsrc_upccode"));
    _minOrderQty->setDouble(itemsrcQ.value("itemsrc_minordqty").toDouble());
    _multOrderQty->setDouble(itemsrcQ.value("itemsrc_multordqty").toDouble());
    _vendorRanking->setValue(itemsrcQ.value("itemsrc_ranking").toInt());
    _leadTime->setValue(itemsrcQ.value("itemsrc_leadtime").toInt());
    _notes->setText(itemsrcQ.value("itemsrc_comments").toString());
    _manufName->setCode(itemsrcQ.value("itemsrc_manuf_name").toString());
    _manufItemNumber->setText(itemsrcQ.value("itemsrc_manuf_item_number").toString());
    _manufItemDescrip->setText(itemsrcQ.value("itemsrc_manuf_item_descrip").toString());
    sFillPriceList();
  }
  else if (itemsrcQ.lastError().type() != QSqlError::NoError)
  {
    systemError(this, itemsrcQ.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void itemSource::sRejected()
{
  if (_new)
  {
    q.prepare( "DELETE FROM itemsrc "
               "WHERE (itemsrc_id=:itemsrc_id);" );
    q.bindValue(":itemsrc_id", _itemsrcid);
    q.exec();
    if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }
}

void itemSource::sVendorChanged( int pId )
{
    q.prepare("SELECT vend_curr_id FROM vend WHERE vend_id = :vend_id;");
    q.bindValue(":vend_id", pId);
    q.exec();
    if (q.first())
	_vendorCurrency->setId(q.value("vend_curr_id").toInt());
    else if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
}
