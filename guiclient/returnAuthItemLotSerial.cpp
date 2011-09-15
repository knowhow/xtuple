/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "returnAuthItemLotSerial.h"

#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

/*
 *  Constructs a returnAuthItemLotSerial as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
returnAuthItemLotSerial::returnAuthItemLotSerial(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  _crmacctid = -1;
  _raitemid = -1;
  _raitemlsid = -1;
  _warehouseid = -1;
  
  // signals and slots connections
  connect(_save, SIGNAL(clicked()), this, SLOT(sSave()));
  connect(_lotSerial, SIGNAL(newId(int)), this, SLOT(sCheck()));
  connect(_cancel, SIGNAL(clicked()), this, SLOT(close()));

  _qtyAuth->setValidator(omfgThis->qtyVal());
  _qtyRegistered->setPrecision(omfgThis->qtyVal());
  _qtyReceived->setPrecision(omfgThis->qtyVal());
  
  adjustSize();
}

/*
 *  Destroys the object and frees any allocated resources
 */
returnAuthItemLotSerial::~returnAuthItemLotSerial()
{
  // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void returnAuthItemLotSerial::languageChange()
{
  retranslateUi(this);
}

enum SetResponse returnAuthItemLotSerial::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("item_id", &valid);
  if (valid)
  {
    _item->setId(param.toInt());
    _lotSerial->setItemId(param.toInt());
    if (_warehouseid != -1)
      populateItemsite();
  }
  
  param = pParams.value("raitem_id", &valid);
  if (valid)
  {
    _raitemid = param.toInt();
    q.prepare("SELECT crmacct_id "
		"FROM raitem,rahead,crmacct "
		"WHERE ((raitem_id=:raitem_id) "
		"AND (raitem_rahead_id=rahead_id) "
		"AND (crmacct_cust_id=rahead_cust_id)); ");
    q.bindValue(":raitem_id", _raitemid);
    q.exec();
    if (q.first())
      _crmacctid = q.value("crmacct_id").toInt();
    else if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      done(-1);
    }
  }
    
  param = pParams.value("warehouse_id", &valid);
  if (valid)
  {
     _warehouseid = param.toInt();
     if (_item->id() != -1)
       populateItemsite();
  }
    
  param = pParams.value("uom", &valid);
  if (valid)
    _qtyUOM->setText(param.toString());

  param = pParams.value("ls_id", &valid);
  if (valid)
    _lotSerial->setId(param.toInt());

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      _mode = cNew;
      _lotSerial->setFocus();
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;
      _qtyAuth->setFocus();
    }
    else if (param.toString() == "view")
    {
      _mode = cView;

      _lotSerial->setEnabled(FALSE);
      _qtyAuth->setEnabled(FALSE);
      _cancel->setText(tr("&Close"));
      _save->hide();

      _cancel->setFocus();
    }
  }

  return NoError;
}

void returnAuthItemLotSerial::sSave()
{
  if (_lotSerial->id() == -1)
  {
    QMessageBox::information( this, tr("No Lot/Serial Number Selected"),
                              tr("You must select a Lot/Serial Number.") );
    _lotSerial->setFocus();
    return;
  }
  if (_mode == cNew)
  {
    XSqlQuery number;
    number.exec("SELECT NEXTVAL('raitemls_raitemls_id_seq') AS raitemls_id;");
    if (number.first())
    {
      _raitemlsid = number.value("raitemls_id").toInt();

      q.prepare( "INSERT INTO raitemls "
                 "( raitemls_id,raitemls_raitem_id,raitemls_ls_id,raitemls_qtyauthorized,raitemls_qtyreceived ) "
                 "VALUES "
                 "( :raitemls_id,:raitem_id,:ls_id,:qty,0 );" );
    }
  }
  else if (_mode == cEdit)
    q.prepare( "UPDATE raitemls "
               "SET raitemls_ls_id=:ls_id,raitemls_qtyauthorized=:qty "
               "WHERE (raitemls_id=:raitemls_id);" );

  q.bindValue(":raitemls_id", _raitemlsid);
  q.bindValue(":raitem_id", _raitemid);
  q.bindValue(":ls_id", _lotSerial->id());
  q.bindValue(":qty", _qtyAuth->toDouble());
  q.exec();
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    done(-1);
  }
  else
    done(_raitemlsid);
}

void returnAuthItemLotSerial::sCheck()
{
  q.prepare( "SELECT raitemls_id "
             "FROM raitemls "
             "WHERE ((raitemls_raitem_id=:raitem_id)"
             " AND (raitemls_ls_id=:ls_id));" );
  q.bindValue(":raitem_id", _raitemid);
  q.bindValue(":ls_id", _lotSerial->id());
  q.exec();
  if (q.first())
  {
    _raitemlsid = q.value("raitemls_id").toInt();
    _mode = cEdit;
    populate();
  }
  else
  {
    _raitemlsid = -1;
    _mode = cNew;
    _qtyAuth->setDouble(0);
    populateLotSerial();
  }
}

void returnAuthItemLotSerial::populate()
{    
  q.prepare( "SELECT itemsite_item_id, uom_name,raitemls_ls_id, raitemls_qtyauthorized,"
		"raitemls_qtyreceived,"
                "COALESCE(SUM(lsreg_qty / raitem_qty_invuomratio),0) AS qtyregistered, "
		"itemsite_warehous_id "
		"FROM raitemls "
		"  LEFT OUTER JOIN lsreg ON (lsreg_crmacct_id=:crmacct_id) "
		"                        AND (lsreg_ls_id=raitemls_ls_id), "
		"raitem, uom, itemsite "
		"WHERE ((raitemls_id=:raitemls_id) "
		"AND (raitemls_raitem_id=raitem_id) "
		"AND (uom_id=raitem_qty_uom_id) "
		"AND (itemsite_id=raitem_itemsite_id)) "
                "GROUP BY itemsite_item_id,uom_name,raitemls_ls_id,raitemls_qtyauthorized,raitemls_qtyreceived, "
                "itemsite_warehous_id" );
  q.bindValue(":raitemls_id", _raitemlsid);
  q.bindValue(":crmacct_id", _crmacctid);
  q.exec();
  if (q.first())
  {
    _item->setId(q.value("itemsite_item_id").toInt());
    _lotSerial->setId(q.value("raitemls_ls_id").toInt());
    _qtyAuth->setDouble(q.value("raitemls_qtyauthorized").toDouble());
    _qtyUOM->setText(q.value("uom_name").toString());
    _qtyReceived->setDouble(q.value("raitemls_qtyreceived").toDouble());
    _qtyRegistered->setDouble(q.value("qtyregistered").toDouble());
    _warehouseid = q.value("itemsite_warehous_id").toInt();
    populateItemsite();
  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    done(-1);
  }
}

void returnAuthItemLotSerial::populateItemsite()
{
  q.prepare("SELECT itemsite_controlmethod "
	"FROM itemsite "
	"WHERE (itemsite_item_id=:item_id) "
	"AND (itemsite_warehous_id=:warehous_id)); ");
  q.bindValue(":item_id", _item->id());
  q.bindValue(":warehous_id", _warehouseid);
  if (q.first())
  {
    if (q.value("itemsite_controlmethod").toString() == "S")
    {
      _qtyAuth->setEnabled(false);
      _qtyAuth->setDouble(1);
    }
  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    done(-1);
  }
}

void returnAuthItemLotSerial::populateLotSerial()
{
  q.prepare( "SELECT COALESCE(SUM(lsreg_qty / raitem_qty_invuomratio),0) AS qtyregistered "
		"FROM lsreg, raitem "
		"WHERE ((lsreg_ls_id=:ls_id) "
		"AND (lsreg_crmacct_id=:crmacct_id) "
                "AND (raitem_id=:raitem_id));" );
  q.bindValue(":ls_id", _lotSerial->id());
  q.bindValue(":crmacct_id", _crmacctid);
  q.bindValue(":raitem_id", _raitemid);
  q.exec();
  if (q.first())
  {
    _qtyRegistered->setDouble(q.value("qtyregistered").toDouble());
    _qtyReceived->setDouble(0);
  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    done(-1);
  }
}

void returnAuthItemLotSerial::closeEvent()
{
  done(0);
}


