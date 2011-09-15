/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "address.h"

#include <QAction>
#include <QMenu>
#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include <parameter.h>

#include "addresscluster.h"
#include "characteristicAssignment.h"
#include "contact.h"
#include "inputManager.h"
#include "shipTo.h"
#include "vendor.h"
#include "vendorAddress.h"
#include "warehouse.h"

address::address(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
    setupUi(this);

    connect(_editAddrUse, SIGNAL(clicked()), this, SLOT(sEdit()));
    connect(_viewAddrUse, SIGNAL(clicked()), this, SLOT(sView()));
    connect(_buttonBox, SIGNAL(accepted()), this, SLOT(sSave()));
    connect(_buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
    connect(_uses, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*)), this, SLOT(sPopulateMenu(QMenu*)));
    connect(_newCharacteristic, SIGNAL(clicked()), this, SLOT(sNewCharacteristic()));
    connect(_editCharacteristic, SIGNAL(clicked()), this, SLOT(sEditCharacteristic()));
    connect(_deleteCharacteristic, SIGNAL(clicked()), this, SLOT(sDeleteCharacteristic()));

    _uses->addColumn(tr("Used by"),	 50, Qt::AlignLeft, true, "type");
    _uses->addColumn(tr("First Name\nor Number"),50, Qt::AlignLeft, true, "cntct_first_name");
    _uses->addColumn(tr("Last Name\nor Name"),	 -1, Qt::AlignLeft, true, "cntct_last_name");
    _uses->addColumn(tr("CRM Account"),	 80, Qt::AlignLeft, true, "crmacct_number");
    _uses->addColumn(tr("Phone"),	100, Qt::AlignLeft, true, "cntct_phone");
    _uses->addColumn(tr("Alternate"),	100, Qt::AlignLeft, true, "cntct_phone2");
    _uses->addColumn(tr("Fax"),		100, Qt::AlignLeft, true, "cntct_fax");
    _uses->addColumn(tr("E-Mail"),	100, Qt::AlignLeft, true, "cntct_email");
    _uses->addColumn(tr("Web Address"),	100, Qt::AlignLeft, true, "cntct_webaddr");

    _charass->addColumn(tr("Characteristic"), _itemColumn, Qt::AlignLeft, true, "char_name");
    _charass->addColumn(tr("Value"),          -1,          Qt::AlignLeft, true, "charass_value");
}

address::~address()
{
  // no need to delete child widgets, Qt does it all for us
}

void address::languageChange()
{
  retranslateUi(this);
}

enum SetResponse address::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("addr_id", &valid);
  if (valid)
  {
    _captive = TRUE;
    _addr->setId(param.toInt());
    sPopulate();
  }

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      _mode = cNew;
      q.exec("SELECT fetchNextNumber('AddressNumber') AS result;");
      q.first();
      _addr->setNumber(q.value("result").toString());
      _addr->setLine1("Address" + QDateTime::currentDateTime().toString());
      int addrSaveResult = _addr->save(AddressCluster::CHANGEONE);
      if (addrSaveResult < 0)
      {
	systemError(this, tr("There was an error creating a new address (%).\n"
			     "Check the database server log for errors.")
			  .arg(addrSaveResult),
		    __FILE__, __LINE__);
	return UndefinedError;
      }
      _comments->setId(_addr->id());
      _addr->setLine1("");
      connect(_charass, SIGNAL(valid(bool)), _editCharacteristic, SLOT(setEnabled(bool)));
      connect(_charass, SIGNAL(valid(bool)), _deleteCharacteristic, SLOT(setEnabled(bool)));
      _addr->setFocus();
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

      _editAddrUse->hide();
      disconnect(_uses, SIGNAL(itemSelected(int)), _editAddrUse, SLOT(animateClick()));
      connect(_uses, SIGNAL(itemSelected(int)), _viewAddrUse, SLOT(animateClick()));

      _addr->setEnabled(FALSE);
      _notes->setEnabled(FALSE);
      _comments->setEnabled(FALSE);
      _newCharacteristic->setEnabled(FALSE);
      _editCharacteristic->setEnabled(FALSE);
      _deleteCharacteristic->setEnabled(FALSE);
      _editAddrUse->setEnabled(FALSE);
      _charass->setEnabled(FALSE);
      _buttonBox->setStandardButtons(QDialogButtonBox::Close);
    }
  }

  return NoError;
}

void address::sSave()
{
   internalSave();
   done(_addr->id());
}

void address::internalSave(AddressCluster::SaveFlags flag)
{
  _addr->setNotes(_notes->toPlainText());

  int saveResult = _addr->save(flag);
  if (-2 == saveResult)
  {
    int answer = QMessageBox::question(this,
		    tr("Saving Shared Address"),
		    tr("There are multiple Contacts sharing this Address.\n"
		       "If you save this Address, the Address for all "
		       "of these Contacts will be changed. Would you like to "
		       "save this Address?"),
		    QMessageBox::No | QMessageBox::Default, QMessageBox::Yes);
    if (QMessageBox::No == answer)
      return;
    saveResult = _addr->save(AddressCluster::CHANGEALL);
  }
  if (0 > saveResult)	// NOT else if
  {
    systemError(this, tr("There was an error saving this address (%1).\n"
			 "Check the database server log for errors.")
		      .arg(saveResult),
		__FILE__, __LINE__);
  }
}

void address::sNewCharacteristic()
{
  internalSave();

  ParameterList params;
  params.append("mode", "new");
  params.append("addr_id", _addr->id());

  characteristicAssignment newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sGetCharacteristics();
}

void address::sEditCharacteristic()
{
  internalSave();

  ParameterList params;
  params.append("mode", "edit");
  params.append("charass_id", _charass->id());

  characteristicAssignment newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sGetCharacteristics();
}

void address::sDeleteCharacteristic()
{
  internalSave();

  q.prepare( "DELETE FROM charass "
             "WHERE (charass_id=:charass_id);" );
  q.bindValue(":charass_id", _charass->id());
  q.exec();

  sGetCharacteristics();
}

void address::sGetCharacteristics()
{
  q.prepare( "SELECT charass_id, char_name, "
             " CASE WHEN char_type < 2 THEN "
             "   charass_value "
             " ELSE "
             "   formatDate(charass_value::date) "
             "END AS charass_value "
             "FROM charass, char "
             "WHERE ( (charass_target_type='ADDR')"
             " AND (charass_char_id=char_id)"
             " AND (charass_target_id=:addr_id) ) "
             "ORDER BY char_order, char_name;" );
  q.bindValue(":addr_id", _addr->id());
  q.exec();
  _charass->populate(q);
}

void address::sPopulate()
{
  _notes->setText(_addr->notes());
  _comments->setId(_addr->id());
  sGetCharacteristics();

  XSqlQuery usesQ;
  usesQ.prepare("SELECT cntct_id, 1, :contact AS type, cntct_first_name, "
		"       cntct_last_name, crmacct_number, cntct_phone, "
		"       cntct_phone2, cntct_fax, cntct_email, cntct_webaddr "
		"FROM cntct LEFT OUTER JOIN crmacct ON (cntct_crmacct_id=crmacct_id) "
		"WHERE (cntct_addr_id=:addr_id) "
		"UNION "
		"SELECT shipto_id, 2, :shipto, shipto_name, "
		"       shipto_name, crmacct_number, '',"
		"       '', '', '', '' "
		"FROM shiptoinfo LEFT OUTER JOIN crmacct ON (shipto_cust_id=crmacct_cust_id) "
		"WHERE (shipto_addr_id=:addr_id) "
		"UNION "
		"SELECT vend_id, 3, :vendor, vend_number, "
		"       vend_name, crmacct_number, '',"
		"       '', '', '', '' "
		"FROM vendinfo LEFT OUTER JOIN crmacct ON (vend_id=crmacct_vend_id) "
		"WHERE (vend_addr_id=:addr_id) "
		"UNION "
		"SELECT vendaddr_id, 4, :vendaddr, vendaddr_code, "
		"       vendaddr_name, crmacct_number, '',"
		"       '', '', '', '' "
		"FROM vendaddrinfo LEFT OUTER JOIN crmacct ON (vendaddr_vend_id=crmacct_vend_id) "
		"WHERE (vendaddr_addr_id=:addr_id) "
		"UNION "
		"SELECT warehous_id, 5, :whs, warehous_code, "
		"       warehous_descrip, '', '',"
		"       '', '', '', '' "
		"FROM whsinfo "
		"WHERE (warehous_addr_id=:addr_id) "
		"ORDER BY 3, 5, 4;");
  usesQ.bindValue(":addr_id", _addr->id());
  usesQ.bindValue(":contact",	tr("Contact"));
  usesQ.bindValue(":shipto",	tr("Ship-To"));
  usesQ.bindValue(":vendor",	tr("Vendor"));
  usesQ.bindValue(":vendaddr",	tr("Vendor Address"));
  usesQ.bindValue(":whs",	tr("Site"));
  usesQ.exec();
  _uses->populate(usesQ, true);	// true => use alt id (to distinguish types)
}

void address::sPopulateMenu(QMenu *pMenu)
{
  QAction *menuItem;
  QString editStr = tr("Edit...");
  QString viewStr = tr("View...");

  switch (_uses->altId())
  {
    case 1:
      if (_privileges->check("MaintainContacts") &&
	  (cNew == _mode || cEdit == _mode))
	menuItem = pMenu->addAction(editStr, this, SLOT(sEditContact()));
      else if (_privileges->check("ViewContacts"))
	menuItem = pMenu->addAction(viewStr, this, SLOT(sViewContact()));

      break;

    case 2:	// ship-to
      if (_privileges->check("MaintainShiptos") &&
	  (cNew == _mode || cEdit == _mode))
	menuItem = pMenu->addAction(editStr, this, SLOT(sEditShipto()));
      else if (_privileges->check("ViewShiptos"))
	menuItem = pMenu->addAction(viewStr, this, SLOT(sViewShipto()));

      break;

    case 3:	// vendor
      /* comment out until we make vendor a XDialog or address a XMainWindow
      if (_privileges->check("MaintainVendors") &&
	  (cNew == _mode || cEdit == _mode))
	menuItem = pMenu->addAction(editStr, this, SLOT(sEditVendor()));
      else if (_privileges->check("ViewVendors"))
	menuItem = pMenu->addAction(viewStr, this, SLOT(sViewVendor()));
      */

      break;

    case 4:	// vendaddr
      if (_privileges->check("MaintainVendorAddresses") &&
	  (cNew == _mode || cEdit == _mode))
	menuItem = pMenu->addAction(editStr, this, SLOT(sEditVendorAddress()));
      else if (_privileges->check("ViewVendorAddresses"))
	menuItem = pMenu->addAction(viewStr, this, SLOT(sViewVendorAddress()));

      break;

    case 5:	// warehouse
      if (_privileges->check("MaintainWarehouses") &&
	  (cNew == _mode || cEdit == _mode))
	menuItem = pMenu->addAction(editStr, this, SLOT(sEditWarehouse()));
      else if (_privileges->check("ViewWarehouses"))
	menuItem = pMenu->addAction(viewStr, this, SLOT(sViewWarehouse()));

      break;

    default:
      break;
  }
}

void address::sEdit()
{
  internalSave();
  switch (_uses->altId())
  {
    case 1:
      sEditContact();
      break;

    case 2:
      sEditShipto();
      break;

    case 3:
      sEditVendor();
      break;

    case 4:
      sEditVendorAddress();
      break;

    case 5:
      sEditWarehouse();
      break;

    default:
      break;
  }

  // force AddressCluster to reload its data
  int tmpAddrId = _addr->id();
  _addr->setId(-1);
  _addr->setId(tmpAddrId);
  sPopulate();
}

void address::sView()
{
  switch (_uses->altId())
  {
    case 1:
      sViewContact();
      break;

    case 2:
      sViewShipto();
      break;

    case 3:
      sViewVendor();
      break;

    case 4:
      sViewVendorAddress();
      break;

    case 5:
      sViewWarehouse();
      break;

    default:
      break;
  }
}

void address::sEditContact()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("cntct_id", _uses->id());
  contact newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void address::sViewContact()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("cntct_id", _uses->id());
  contact newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void address::sEditShipto()
{
  ParameterList params;
  shipTo newdlg(this, "", TRUE);
  params.append("mode", "edit");
  params.append("shipto_id", _uses->id());
  newdlg.set(params);
  newdlg.exec();
}

void address::sViewShipto()
{
  ParameterList params;
  shipTo newdlg(this, "", TRUE);
  params.append("mode", "view");
  params.append("shipto_id", _uses->id());
  newdlg.set(params);
  newdlg.exec();
}

void address::sEditVendor()
{
  /* comment out until vendor becomes a XDialog or address a XMainWindow
  ParameterList params;
  vendor newdlg(this, "", TRUE);
  params.append("mode", "view");
  params.append("vend_id", _uses->id());
  newdlg.set(params);
  newdlg.exec();
  */
}

void address::sViewVendor()
{
  /* comment out until vendor becomes a XDialog or address a XMainWindow
  ParameterList params;
  vendor newdlg(this, "", TRUE);
  params.append("mode", "view");
  params.append("vend_id", _uses->id());
  newdlg.set(params);
  newdlg.exec();
  */
}

void address::sEditVendorAddress()
{
  ParameterList params;
  vendorAddress newdlg(this, "", TRUE);
  params.append("mode", "edit");
  params.append("vendaddr_id", _uses->id());
  newdlg.set(params);
  newdlg.exec();
}

void address::sViewVendorAddress()
{
  ParameterList params;
  vendorAddress newdlg(this, "", TRUE);
  params.append("mode", "view");
  params.append("vendaddr_id", _uses->id());
  newdlg.set(params);
  newdlg.exec();
}

void address::sEditWarehouse()
{
  ParameterList params;
  warehouse newdlg(this, "", TRUE);
  params.append("mode", "edit");
  params.append("warehous_id", _uses->id());
  newdlg.set(params);
  newdlg.exec();
}

void address::sViewWarehouse()
{
  ParameterList params;
  warehouse newdlg(this, "", TRUE);
  params.append("mode", "view");
  params.append("warehous_id", _uses->id());
  newdlg.set(params);
  newdlg.exec();
}
