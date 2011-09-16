/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "contact.h"

#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include "characteristicAssignment.h"
#include "contactcluster.h"
#include "crmaccount.h"
#include "customer.h"
#include "employee.h"
#include "inputManager.h"
#include "prospect.h"
#include "shipTo.h"
#include "storedProcErrorLookup.h"
#include "vendor.h"
#include "vendorAddress.h"
#include "warehouse.h"
#include "xsqlquery.h"
#include <time.h>

contact::contact(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  // Legacy compatibility removed
  // For legacy compatibility
  //_save = _buttonBox->button(QDialogButtonBox::Save);
  //_save->setObjectName("_save");

  connect(_deleteCharacteristic,  SIGNAL(clicked()), this, SLOT(sDeleteCharass()));
  connect(_detachUse,		  SIGNAL(clicked()), this, SLOT(sDetachUse()));
  connect(_editCharacteristic,    SIGNAL(clicked()), this, SLOT(sEditCharass()));
  connect(_editUse,  		  SIGNAL(clicked()), this, SLOT(sEditUse()));
  connect(_newCharacteristic,     SIGNAL(clicked()), this, SLOT(sNewCharass()));
  connect(_buttonBox,	          SIGNAL(accepted()), this, SLOT(sSave()));
  connect(_buttonBox, SIGNAL(rejected()), this, SLOT(sClose()));
  connect(_uses, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*)), this, SLOT(sPopulateUsesMenu(QMenu*)));
  connect(_uses, SIGNAL(valid(bool)), this, SLOT(sHandleValidUse(bool)));
  connect(_viewUse,	          SIGNAL(clicked()), this, SLOT(sViewUse()));
  connect(omfgThis,        SIGNAL(vendorsUpdated()), this, SLOT(sFillList()));
  connect(omfgThis,      SIGNAL(prospectsUpdated()), this, SLOT(sFillList()));
  connect(omfgThis,     SIGNAL(warehousesUpdated()), this, SLOT(sFillList()));
  connect(omfgThis, SIGNAL(crmAccountsUpdated(int)), this, SLOT(sFillList()));
  connect(omfgThis, SIGNAL(customersUpdated(int,bool)), this, SLOT(sFillList())); 
  connect(_crmAccount, SIGNAL(newId(int)), _contact, SLOT(setSearchAcct(int)));

  _charass->addColumn(tr("Characteristic"), _itemColumn, Qt::AlignLeft, true, "char_name");
  _charass->addColumn(tr("Value"),          -1,          Qt::AlignLeft, true, "charass_value");

  _uses->addColumn(tr("Used by"),         100, Qt::AlignLeft, true, "type");
  _uses->addColumn(tr("Number"), _orderColumn, Qt::AlignLeft, true, "number");
  _uses->addColumn(tr("Name"),	           -1, Qt::AlignLeft, true, "name");
  _uses->addColumn(tr("Role"),	           -1, Qt::AlignLeft, true, "role");
  _uses->addColumn(tr("Active"),    _ynColumn, Qt::AlignCenter,true, "active");
  _uses->addColumn(tr("Owner"),   _userColumn, Qt::AlignLeft,  false,"owner");

  _activeCache = false;

  _contact->setMinimalLayout(false);
  _contact->setAccountVisible(false);
  _contact->setInitialsVisible(false);
  _contact->setActiveVisible(false);
  _contact->setOwnerVisible(false);
  _contact->setListVisible(false);

  _owner->setType(UsernameLineEdit::UsersActive);
  if(!_privileges->check("EditOwner")) _owner->setEnabled(false);

}

contact::~contact()
{
    // no need to delete child widgets, Qt does it all for us
}

void contact::languageChange()
{
    retranslateUi(this);
}

enum SetResponse contact::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  QVariant param;
  bool     valid;


  param = pParams.value("cntct_id", &valid);
  if (valid)
  {
    _captive = TRUE;
    _contact->setId(param.toInt());
    _comments->setId(_contact->id());
    _documents->setId(_contact->id());
    sPopulate();
  }

  param = pParams.value("crmacct_id", &valid);
  if (valid)
  {
    _crmAccount->setId(param.toInt());
  }

  param = pParams.value("addr_line1", &valid);
  if (valid)
    _contact->addressWidget()->setLine1(param.toString());

  param = pParams.value("addr_line2", &valid);
  if (valid)
    _contact->addressWidget()->setLine2(param.toString());

  param = pParams.value("addr_line3", &valid);
  if (valid)
    _contact->addressWidget()->setLine3(param.toString());

  param = pParams.value("addr_city", &valid);
  if (valid)
    _contact->addressWidget()->setCity(param.toString());

  param = pParams.value("addr_state", &valid);
  if (valid)
    _contact->addressWidget()->setState(param.toString());

  param = pParams.value("addr_postalcode", &valid);
  if (valid)
    _contact->addressWidget()->setPostalCode(param.toString());

  param = pParams.value("addr_country", &valid);
  if (valid)
    _contact->addressWidget()->setCountry(param.toString());

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      _mode = cNew;
      q.exec("SELECT fetchNextNumber('ContactNumber') AS result;");
      q.first();
      _number->setText(q.value("result").toString());
      _contact->setNumber(_number->text());
      _contact->setFirst("Contact" + QDateTime::currentDateTime().toString());
      int cntctSaveResult = _contact->save(AddressCluster::CHANGEONE);
      if (cntctSaveResult < 0)
      {
	systemError(this, tr("There was an error creating a new contact (%).\n"
			     "Check the database server log for errors.")
			  .arg(cntctSaveResult),
		    __FILE__, __LINE__);
	return UndefinedError;
      }
      _comments->setId(_contact->id());
      _documents->setId(_contact->id());
      _contact->setFirst("");
      connect(_charass, SIGNAL(valid(bool)), _editCharacteristic, SLOT(setEnabled(bool)));
      connect(_charass, SIGNAL(valid(bool)), _deleteCharacteristic, SLOT(setEnabled(bool)));
      _contact->setOwnerUsername(omfgThis->username());
      _contact->setFocus();
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;

      connect(_charass, SIGNAL(valid(bool)), _editCharacteristic, SLOT(setEnabled(bool)));
      connect(_charass, SIGNAL(valid(bool)), _deleteCharacteristic, SLOT(setEnabled(bool)));
    }
    else if (param.toString() == "view")
    {
      _mode = cView;

      _buttonBox->setStandardButtons(QDialogButtonBox::Close);

      _contact->setEnabled(FALSE);
      _notes->setEnabled(FALSE);
      _comments->setEnabled(FALSE);
      _documents->setEnabled(FALSE);
      _newCharacteristic->setEnabled(FALSE);
      _editCharacteristic->setEnabled(FALSE);
      _deleteCharacteristic->setEnabled(FALSE);
      _charass->setEnabled(FALSE);
    }
  }

  return NoError;
}

void contact::sPopulateUsesMenu(QMenu* pMenu)
{
  QAction *menuItem;
  QString editStr = tr("Edit...");
  QString viewStr = tr("View...");
  QString detachStr = tr("Detach");
  bool editPriv = false;
  bool viewPriv = false;

  switch (_uses->altId())
  {
    case 1:
    case 2:
    editPriv =
        (omfgThis->username() == _uses->currentItem()->rawValue("owner") && _privileges->check("MaintainPersonalCRMAccounts")) ||
        (omfgThis->username() != _uses->currentItem()->rawValue("owner") && _privileges->check("MaintainAllCRMAccounts"));

    viewPriv =
        (omfgThis->username() == _uses->currentItem()->rawValue("owner") && _privileges->check("ViewPersonalCRMAccounts")) ||
        (omfgThis->username() != _uses->currentItem()->rawValue("owner") && _privileges->check("ViewAllCRMAccounts"));

      menuItem = pMenu->addAction(editStr, this, SLOT(sEditCRMAccount()));
      menuItem->setEnabled(editPriv && (cView != _mode));
      menuItem = pMenu->addAction(viewStr, this, SLOT(sViewCRMAccount()));
      menuItem->setEnabled(viewPriv);
      menuItem = pMenu->addAction(detachStr, this, SLOT(sDetachUse()));
      menuItem->setEnabled(editPriv && (cView != _mode));
      break;

    case 3:
    case 4:
      menuItem = pMenu->addAction(editStr, this, SLOT(sEditCustomer()));
      menuItem->setEnabled(_privileges->check("MaintainCustomerMasters") && (cView != _mode));
      menuItem = pMenu->addAction(viewStr, this, SLOT(sViewCustomer()));
      menuItem->setEnabled(_privileges->check("ViewCustomerMasters"));
      menuItem = pMenu->addAction(detachStr, this, SLOT(sDetachUse()));
      menuItem->setEnabled(_privileges->check("MaintainCustomerMasters") && (cView != _mode));
      break;

    case 5:
    case 6:
      menuItem = pMenu->addAction(editStr, this, SLOT(sEditVendor()));
      menuItem->setEnabled(_privileges->check("MaintainVendors") && (cView != _mode));
      menuItem = pMenu->addAction(viewStr, this, SLOT(sViewVendor()));
      menuItem->setEnabled(_privileges->check("ViewVendors"));
      menuItem = pMenu->addAction(detachStr, this, SLOT(sDetachUse()));
      menuItem->setEnabled(_privileges->check("MaintainVendors") && (cView != _mode));
      break;

    case 7:
      editPriv =
          (omfgThis->username() != _uses->currentItem()->rawValue("owner") && _privileges->check("MaintainProspectMasters"));

      viewPriv =
          (omfgThis->username() != _uses->currentItem()->rawValue("owner") && _privileges->check("ViewProspectMasters"));

      menuItem = pMenu->addAction(editStr, this, SLOT(sEditProspect()));
      menuItem->setEnabled(editPriv && (cView != _mode));
      menuItem = pMenu->addAction(viewStr, this, SLOT(sViewProspect()));
      menuItem->setEnabled(viewPriv);
      menuItem = pMenu->addAction(detachStr, this, SLOT(sDetachUse()));
      menuItem->setEnabled(editPriv && (cView != _mode));
      break;

    case 8:
      menuItem = pMenu->addAction(editStr, this, SLOT(sEditShipto()));
      menuItem->setEnabled(_privileges->check("MaintainShiptos") && (cView != _mode));
      menuItem = pMenu->addAction(viewStr, this, SLOT(sViewShipto()));
      menuItem->setEnabled(_privileges->check("ViewShiptos"));
      menuItem = pMenu->addAction(detachStr, this, SLOT(sDetachUse()));
      menuItem->setEnabled(_privileges->check("MaintainShiptos") && (cView != _mode));
      break;

    case 9:
      menuItem = pMenu->addAction(editStr, this, SLOT(sEditVendorAddress()));
      menuItem->setEnabled(_privileges->check("MaintainVendorAddresses") && (cView != _mode));
      menuItem = pMenu->addAction(viewStr, this, SLOT(sViewVendorAddress()));
      menuItem->setEnabled(_privileges->check("ViewVendorAddresses"));
      menuItem = pMenu->addAction(detachStr, this, SLOT(sDetachUse()));
      menuItem->setEnabled(_privileges->check("MaintainVendorAddresses") && (cView != _mode));
      break;

    case 10:
      menuItem = pMenu->addAction(editStr, this, SLOT(sEditWarehouse()));
      menuItem->setEnabled(_privileges->check("MaintainWarehouses") && (cView != _mode));
      menuItem = pMenu->addAction(viewStr, this, SLOT(sViewWarehouse()));
      menuItem->setEnabled(_privileges->check("ViewWarehouses"));
      menuItem = pMenu->addAction(detachStr, this, SLOT(sDetachUse()));
      menuItem->setEnabled(_privileges->check("MaintainWarehouses") && (cView != _mode));
      break;

    case 11:
      menuItem = pMenu->addAction(editStr, this, SLOT(sEditEmployee()));
      menuItem->setEnabled(_privileges->check("MaintainEmployees") && (cView != _mode));
      menuItem = pMenu->addAction(viewStr, this, SLOT(sViewEmployee()));
      menuItem->setEnabled(_privileges->check("ViewEmployees"));
      menuItem = pMenu->addAction(detachStr, this, SLOT(sDetachUse()));
      menuItem->setEnabled(_privileges->check("MaintainEmployees") && (cView != _mode));

    default:
      break;
  }
}

void contact::sClose()
{
  if (_mode == cNew)
  {
    q.prepare("SELECT deleteContact(:cntct_id) AS returnVal;");
    q.bindValue(":cntct_id", _contact->id());
    q.exec();
    if (q.first())
    {
      int returnVal = q.value("returnVal").toInt();
      if (returnVal < 0)
      {
	systemError(this, storedProcErrorLookup("deleteContact", returnVal),
		    __FILE__, __LINE__);
	return;
      }
    }
    else if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }

  reject();
}

void contact::sSave()
{
   if(_contact->first().isEmpty() && _contact->last().isEmpty()  )
    {
           QMessageBox::information(this, tr("Contact Blank"),
                            tr("<p>You must fill in a contact first or last name as a minimum before saving."));
           return;
    }


  if (_activeCache && ! _contact->active())
  {
    q.prepare("SELECT EXISTS(SELECT 1 FROM crmacct WHERE(crmacct_active"
	      "                                   AND (crmacct_cntct_id_1=:id)"
	      "                                    OR (crmacct_cntct_id_2=:id))"
	      "        UNION SELECT 2 FROM custinfo WHERE(cust_active"
	      "                                   AND (cust_cntct_id=:id)"
	      "                                    OR (cust_corrcntct_id=:id))"
	      "        UNION SELECT 3 FROM vendinfo WHERE(vend_active"
	      "                                   AND (vend_cntct1_id=:id)"
	      "                                    OR (vend_cntct2_id=:id))"
	      "        UNION SELECT 4 FROM prospect WHERE (prospect_active"
	      "                                   AND (prospect_cntct_id=:id))"
	      "        UNION SELECT 5 FROM shiptoinfo WHERE (shipto_active"
	      "                                   AND (shipto_cntct_id=:id))"
	      "        UNION SELECT 6 FROM vendaddrinfo WHERE"
	      "                                   (vendaddr_cntct_id=:id)"
	      "        UNION SELECT 7 FROM whsinfo WHERE (warehous_active"
	      "                                   AND (warehous_cntct_id=:id))"
	      " ) AS inuse;");
    q.bindValue(":id", _contact->id());
    q.exec();
    if (q.first() && q.value("inuse").toBool())
    {
      QMessageBox::information(this, tr("Cannot make Contact inactive"),
			    tr("<p>You may not mark this Contact as not "
			       "Active when this person is a Contact "
			       "for an active CRM Account, Customer, "
			       "Vendor, or Prospect."));
      return;
    }
    else if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }

  /* save the address first so we can check for multiple uses, then save the
     contact with less error checking because this is contact maintenance
   */
  AddressCluster* addr = _contact->addressWidget();
  int saveResult = addr->save(AddressCluster::CHECK);
  if (-2 == saveResult)
  {
    int answer = 2;	// Cancel
    answer = QMessageBox::question(this, tr("Question Saving Address"),
		    tr("There are multiple Contacts sharing this Address.\n"
		       "What would you like to do?"),
		    tr("Change This One"),
		    tr("Change Address for All"),
		    tr("Cancel"),
		    2, 2);

    if (0 == answer)
      saveResult = addr->save(AddressCluster::CHANGEONE);
    else if (1 == answer)
      saveResult = addr->save(AddressCluster::CHANGEALL);
    else
      return;
  }
  if (saveResult < 0)	// check from errors for CHECK and CHANGE* saves
  {
    systemError(this, tr("There was an error saving this Address (%1).\n"
			 "Check the database server log for errors.")
		      .arg(saveResult),
		__FILE__, __LINE__);
    return;
  }

  _contact->setAddress(saveResult);
  _contact->setNotes(_notes->toPlainText());

  saveResult = _contact->save(AddressCluster::CHANGEALL);
  if (saveResult < 0)
  {
    systemError(this, tr("There was an error saving this Contact (%1).\n"
			 "Check the database server log for errors.")
		      .arg(saveResult),
		__FILE__, __LINE__);
    return;
  }

  // Would be better to get to these through contact widget, but for now...
  XSqlQuery cntctUpd;
  cntctUpd.prepare("UPDATE cntct SET "
          " cntct_active = :active, "
          " cntct_crmacct_id = :crmacct_id, "
          " cntct_owner_username = :owner_username "
          "WHERE (cntct_id=:cntct_id);");
  cntctUpd.bindValue(":active", QVariant(_active->isChecked()));
  if (_crmAccount->id() != -1)
    cntctUpd.bindValue(":crmacct_id", _crmAccount->id());
  cntctUpd.bindValue(":owner_username", _owner->username());
  cntctUpd.bindValue(":cntct_id", _contact->id());
  cntctUpd.exec();
  if (cntctUpd.lastError().type() != QSqlError::NoError)
  {
    systemError(this, cntctUpd.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  done(_contact->id());
}

void contact::sPopulate()
{
  _number->setText(_contact->number());
  _active->setChecked(_contact->active());
  _crmAccount->setId(_contact->crmAcctId());
  _owner->setUsername(_contact->owner());
  _notes->setText(_contact->notes());
  _activeCache = _contact->active();
  sFillList();
}

void contact::sNewCharass()
{
  ParameterList params;
  params.append("mode", "new");
  params.append("cntct_id", _contact->id());

  characteristicAssignment newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void contact::sEditCharass()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("charass_id", _charass->id());

  characteristicAssignment newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void contact::sDeleteCharass()
{
  q.prepare( "DELETE FROM charass "
             "WHERE (charass_id=:charass_id);" );
  q.bindValue(":charass_id", _charass->id());
  q.exec();
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  sFillList();
}

void contact::sFillList()
{
  q.prepare( "SELECT charass_id, char_name, "
             " CASE WHEN char_type < 2 THEN "
             "   charass_value "
             " ELSE "
             "   formatDate(charass_value::date) "
             "END AS charass_value "
             "FROM charass, char "
             "WHERE ((charass_target_type='CNTCT')"
             " AND   (charass_char_id=char_id)"
             " AND   (charass_target_id=:cntct_id) ) "
             "ORDER BY char_order, char_name;" );
  q.bindValue(":cntct_id", _contact->id());
  q.exec();
  _charass->populate(q);
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  q.prepare("SELECT crmacct_id AS id, 1 AS altId, :crmacct AS type,"
	    "       crmacct_number AS number,"
	    "       crmacct_name AS name, :primary AS role,"
            "       (crmacct_active) AS active,"
            "       crmacct_owner_username AS owner"
	    "  FROM crmacct WHERE (crmacct_cntct_id_1=:id)"
            "UNION "
	    "SELECT crmacct_id AS id, 2 AS altId, :crmacct AS type,"
	    "       crmacct_number AS number,"
	    "       crmacct_name AS name, :secondary AS role,"
            "       (crmacct_active) AS active,"
            "       crmacct_owner_username AS owner"
            "  FROM crmacct WHERE (crmacct_cntct_id_2=:id)"
	    "UNION "
	    "SELECT cust_id AS id, 3 AS altId, :cust AS type,"
	    "       cust_number AS number,"
	    "       cust_name AS name, :billing AS role,"
            "       (cust_active) AS active,"
            "       '' AS owner"
	    "  FROM custinfo WHERE (cust_cntct_id=:id)"
	    "UNION "
	    "SELECT cust_id AS id, 4 AS altId, :cust AS type,"
	    "       cust_number AS number,"
	    "       cust_name AS name, :correspond AS role,"
            "       (cust_active) AS active,"
            "       '' AS owner"
	    "  FROM custinfo WHERE (cust_corrcntct_id=:id)"
	    "UNION "
	    "SELECT vend_id AS id, 5 AS altId, :vend AS type,"
	    "       vend_number AS number,"
	    "       vend_name AS name, :primary AS role,"
            "       (vend_active) AS active,"
            "       '' AS owner"
	    "  FROM vendinfo WHERE (vend_cntct1_id=:id)"
	    "UNION "
	    "SELECT vend_id AS id, 6 AS altId, :vend AS type,"
	    "       vend_number AS number,"
	    "       vend_name AS name, :secondary AS role,"
            "       (vend_active) AS active,"
            "       '' AS owner"
	    "  FROM vendinfo WHERE (vend_cntct2_id=:id)"
	    "UNION "
	    "SELECT prospect_id AS id, 7 AS altId, :prospect AS type,"
	    "       prospect_number AS number,"
	    "       prospect_name AS name, '' AS role,"
            "       (prospect_active) AS active,"
            "       '' AS owner"
	    "  FROM prospect WHERE (prospect_cntct_id=:id)"
	    "UNION "
	    "SELECT shipto_id AS id, 8 AS altId, :shipto AS type,"
	    "       shipto_num AS number,"
	    "       shipto_name AS name, '' AS role,"
            "       (shipto_active) AS active,"
            "       '' AS owner"
	    "  FROM shiptoinfo WHERE (shipto_cntct_id=:id)"
	    "UNION "
	    "SELECT vendaddr_id AS id, 9 AS altId, :vendaddr AS type,"
	    "       vendaddr_code AS number,"
	    "       vendaddr_name AS name, '' AS role,"
            "       (true) AS active,"
            "       '' AS owner"
	    "  FROM vendaddrinfo WHERE (vendaddr_cntct_id=:id)"
	    "UNION SELECT warehous_id AS id, 10 AS altId, :whs AS type,"
	    "       warehous_code AS number,"
	    "       warehous_descrip AS name, '' AS role,"
            "       (warehous_active) AS active,"
            "       '' AS owner"
	    "  FROM whsinfo WHERE (warehous_cntct_id=:id)"
	    "UNION SELECT emp_id AS id, 11 AS altId, :emp AS type,"
	    "       emp_code AS number,"
	    "       emp_number AS name, '' AS role,"
            "       (emp_active) AS active,"
            "       '' AS owner"
	    "  FROM emp WHERE (emp_cntct_id=:id)"
	    "ORDER BY type, number;");
  q.bindValue(":id",		_contact->id());
  q.bindValue(":primary",	tr("Primary Contact"));
  q.bindValue(":secondary",	tr("Secondary Contact"));
  q.bindValue(":billing",	tr("Billing Contact"));
  q.bindValue(":correspond",	tr("Correspondence Contact"));
  q.bindValue(":crmacct",	tr("CRM Account"));
  q.bindValue(":cust",		tr("Customer"));
  q.bindValue(":vend",		tr("Vendor"));
  q.bindValue(":prospect",	tr("Prospect"));
  q.bindValue(":shipto",	tr("Ship-To Address"));
  q.bindValue(":vendaddr",	tr("Vendor Address"));
  q.bindValue(":whs",		tr("Site"));
  q.bindValue(":emp",		tr("Employee"));
  q.exec();
  _uses->populate(q, true);
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void contact::sDetachUse()
{
  QString question;
  XSqlQuery detachq;
  switch (_uses->altId())
  {
    case 1:
      question = tr("Are you sure that you want to remove this Contact as "
		    "the Primary Contact for this CRM Account?");
      detachq.prepare("UPDATE crmacct SET crmacct_cntct_id_1 = NULL "
		      "WHERE (crmacct_id=:id);");
      break;
    case 2:
      question = tr("Are you sure that you want to remove this Contact as "
		    "the Secondary Contact for this CRM Account?");
      detachq.prepare("UPDATE crmacct SET crmacct_cntct_id_2 = NULL "
		      "WHERE (crmacct_id=:id);");
      break;

    case 3:
      question = tr("Are you sure that you want to remove this Contact as "
		    "the Billing Contact for this Customer?");
      detachq.prepare("UPDATE custinfo SET cust_cntct_id = NULL "
		      "WHERE (cust_id=:id);");
      break;
    case 4:
      question = tr("Are you sure that you want to remove this Contact as "
		    "the Correspondence Contact for this Customer?");
      detachq.prepare("UPDATE custinfo SET cust_corrcntct_id = NULL "
		      "WHERE (cust_id=:id);");
      break;

    case 5:
      question = tr("Are you sure that you want to remove this Contact as "
		    "the Primary Contact for this Vendor?");
      detachq.prepare("UPDATE vendinfo SET vend_cntct1_id = NULL "
		      "WHERE (vend_id=:id);");
      break;

    case 6:
      question = tr("Are you sure that you want to remove this Contact as "
		    "the Secondary Contact for this Vendor?");
      detachq.prepare("UPDATE vendinfo SET vend_cntct2_id = NULL "
		      "WHERE (vend_id=:id);");
      break;

    case 7:
      question = tr("Are you sure that you want to remove this Contact as "
		    "the Contact for this Prospect?");
      detachq.prepare("UPDATE prospect SET prospect_cntct_id = NULL "
		      "WHERE (prospect_id=:id);");
      break;

    case 8:
      question = tr("Are you sure that you want to remove this Contact as "
		    "the Contact for this Ship-To Address?");
      detachq.prepare("UPDATE shiptoinfo SET shipto_cntct_id = NULL "
		      "WHERE (shipto_id=:id);");
      break;

    case 9:
      question = tr("Are you sure that you want to remove this Contact as "
		    "the Contact for this Vendor Address?");
      detachq.prepare("UPDATE vendaddrinfo SET vendaddr_cntct_id = NULL "
		      "WHERE (vendaddr_id=:id);");
      break;

    case 10:
      question = tr("Are you sure that you want to remove this Contact as "
		    "the Contact for this Site?");
      detachq.prepare("UPDATE whsinfo SET warehous_cntct_id = NULL "
		      "WHERE (warehous_id=:id);");
      break;

    case 11:
      question = tr("Are you sure that you want to remove this Contact as "
		    "the Contact for this Employee?");
      detachq.prepare("UPDATE emp SET emp_cntct_id = NULL "
		      "WHERE (emp_id=:id);");
      break;

    default:
      break;
  }

  if (! question.isEmpty() &&
      QMessageBox::question(this, tr("Detach Contact?"), question,
		    QMessageBox::Yes,
		    QMessageBox::No | QMessageBox::Default) == QMessageBox::No)
    return;

  detachq.bindValue(":id", _uses->id());
  detachq.exec();
  if (detachq.lastError().type() != QSqlError::NoError)
  {
    systemError(this, detachq.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
  sFillList();
}

void contact::sEditUse()
{
  switch (_uses->altId())
  {
    case 1:
    case 2:
      sEditCRMAccount();
      break;

    case 3:
    case 4:
      sEditCustomer();
      break;

    case 5:
    case 6:
      sEditVendor();
      break;

    case 7:
      sEditProspect();
      break;

    case 8:
      sEditShipto();
      break;

    case 9:
      sEditVendorAddress();
      break;

    case 10:
      sEditWarehouse();
      break;

    case 11:
      sEditEmployee();
      break;

    default:
      break;
  }
}

void contact::sViewUse()
{
  switch (_uses->altId())
  {
    case 1:
    case 2:
      sViewCRMAccount();
      break;

    case 3:
    case 4:
      sViewCustomer();
      break;

    case 5:
    case 6:
      sViewVendor();
      break;

    case 7:
      sViewProspect();
      break;

    case 8:
      sViewShipto();
      break;

    case 9:
      sViewVendorAddress();
      break;

    case 10:
      sViewWarehouse();
      break;

    case 11:
      sViewEmployee();
      break;

    default:
      break;
  }
}

void contact::sHandleValidUse(bool valid)
{
  bool editPriv = (
                  (_uses->altId() == 1 && _privileges->check("MaintainAllCRMAccounts")) ||
                  (_uses->altId() == 1 && _privileges->check("MaintainPersonalCRMAccounts")
                                       && _uses->currentItem()->rawValue("owner") == omfgThis->username()) ||
                  (_uses->altId() == 2 && _privileges->check("MaintainAllCRMAccounts")) ||
                  (_uses->altId() == 2 && _privileges->check("MaintainPersonalCRMAccounts")
                                       && _uses->currentItem()->rawValue("owner") == omfgThis->username()) ||
                  (_uses->altId() == 3 && _privileges->check("MaintainCustomerMasters")) ||
		  (_uses->altId() == 4 && _privileges->check("MaintainCustomerMasters")) ||
		  (_uses->altId() == 5 && _privileges->check("MaintainVendors")) ||
		  (_uses->altId() == 6 && _privileges->check("MaintainVendors")) ||
                  (_uses->altId() == 7 && _privileges->check("MaintainProspectMasters")) ||
                  (_uses->altId() == 8 && _privileges->check("MaintainShiptos")) ||
		  (_uses->altId() == 9 && _privileges->check("MaintainVendorAddresses")) ||
		  (_uses->altId() ==10 && _privileges->check("MaintainWarehouses")) ||
		  (_uses->altId() ==11 && _privileges->check("MaintainEmployees")) 
  );
  bool viewPriv = (
                  (_uses->altId() == 1 && _privileges->check("ViewAllCRMAccounts")) ||
                  (_uses->altId() == 1 && _privileges->check("ViewPersonalCRMAccounts")
                                       && _uses->currentItem()->rawValue("owner") == omfgThis->username()) ||
                  (_uses->altId() == 2 && _privileges->check("ViewAllCRMAccounts")) ||
                  (_uses->altId() == 2 && _privileges->check("ViewPersonalCRMAccounts")
                                       && _uses->currentItem()->rawValue("owner") == omfgThis->username()) ||
                  (_uses->altId() == 3 && _privileges->check("ViewCustomerMasters")) ||
		  (_uses->altId() == 4 && _privileges->check("ViewCustomerMasters")) ||
		  (_uses->altId() == 5 && _privileges->check("ViewVendors")) ||
		  (_uses->altId() == 6 && _privileges->check("ViewVendors")) ||
                  (_uses->altId() == 7 && _privileges->check("ViewProspectMasters")) ||
                  (_uses->altId() == 8 && _privileges->check("ViewShiptos")) ||
		  (_uses->altId() == 9 && _privileges->check("ViewVendorAddresses")) ||
		  (_uses->altId() ==10 && _privileges->check("ViewWarehouses"))  ||
		  (_uses->altId() ==11 && _privileges->check("ViewEmployees")) 
  );

  disconnect(_uses, SIGNAL(itemSelected(int)), _editUse, SLOT(animateClick()));
  disconnect(_uses, SIGNAL(itemSelected(int)), _viewUse, SLOT(animateClick()));
  if (_mode != cView && editPriv)
  {
    _detachUse->setEnabled(valid);
    _editUse->setEnabled(valid);
    _viewUse->setEnabled(valid);
    connect(_uses, SIGNAL(itemSelected(int)), (_mode == cEdit) ? _editUse : _viewUse, SLOT(animateClick()));
  }
  else if (viewPriv)
  {
    _detachUse->setEnabled(false);
    _editUse->setEnabled(false);
    _viewUse->setEnabled(true);
    connect(_uses, SIGNAL(itemSelected(int)), _viewUse, SLOT(animateClick()));
  }
  else
  {
    _detachUse->setEnabled(false);
    _editUse->setEnabled(false);
    _viewUse->setEnabled(false);
  }
}

void contact::sEditCRMAccount()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("crmacct_id",	_uses->id());
  params.append("modal");
  crmaccount * newdlg = new crmaccount(this);
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void contact::sViewCRMAccount()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("crmacct_id",	_uses->id());
  params.append("modal");
  crmaccount * newdlg = new crmaccount(this);
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void contact::sEditCustomer()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("cust_id",	_uses->id());
  customer *newdlg = new customer(0, "custForContact", Qt::Dialog);
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg, Qt::WindowModal);
}

void contact::sViewCustomer()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("cust_id",	_uses->id());
  customer *newdlg = new customer(0, "custForContact", Qt::Dialog);
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg, Qt::WindowModal);
}

void contact::sEditEmployee()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("emp_id",	_uses->id());
  employee newdlg(this);
  newdlg.set(params);
  if (newdlg.exec() == XDialog::Accepted)
    sFillList();
}

void contact::sViewEmployee()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("emp_id",	_uses->id());
  employee newdlg(this);
  newdlg.set(params);
  newdlg.exec();
}

void contact::sEditProspect()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("prospect_id",	_uses->id());
  prospect *newdlg = new prospect(0, "prospectForContact", Qt::Dialog);
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg, Qt::WindowModal);
}

void contact::sViewProspect()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("prospect_id",	_uses->id());
  prospect *newdlg = new prospect(0, "prospectForContact", Qt::Dialog);
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg, Qt::WindowModal);
}

void contact::sEditShipto()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("shipto_id",	_uses->id());
  shipTo newdlg(this, "", true);
  newdlg.set(params);
  if (newdlg.exec() == XDialog::Accepted)
    sFillList();
}

void contact::sViewShipto()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("shipto_id",	_uses->id());
  shipTo newdlg(this, "", true);
  newdlg.set(params);
  newdlg.exec();
}

void contact::sEditVendorAddress()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("vendaddr_id",	_uses->id());
  vendorAddress newdlg(this, "", true);
  newdlg.set(params);
  if (newdlg.exec() == XDialog::Accepted)
    sFillList();
}

void contact::sViewVendorAddress()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("vendaddr_id",	_uses->id());
  vendorAddress newdlg(this, "", true);
  newdlg.set(params);
  newdlg.exec();
}

void contact::sEditVendor()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("vend_id",	_uses->id());
  vendor *newdlg = new vendor(0, "vendorForContact", Qt::Dialog);
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg, Qt::WindowModal);
}

void contact::sViewVendor()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("vend_id",	_uses->id());
  vendor *newdlg = new vendor(0, "vendorForContact", Qt::Dialog);
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg, Qt::WindowModal);
}

void contact::sEditWarehouse()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("warehous_id",	_uses->id());
  warehouse newdlg(this, "", true);
  newdlg.set(params);
  if (newdlg.exec() == XDialog::Accepted)
    sFillList();
}

void contact::sViewWarehouse()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("warehous_id",	_uses->id());
  warehouse newdlg(this, "", true);
  newdlg.set(params);
  newdlg.exec();
}
