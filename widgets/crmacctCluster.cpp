/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include <QMessageBox>
#include <QSqlError>

#include <metasql.h>

#include "crmacctcluster.h"
#include "custcluster.h"
#include "xcombobox.h"

CRMAcctCluster::CRMAcctCluster(QWidget* pParent, const char* pName) :
    VirtualCluster(pParent, pName)
{
    addNumberWidget(new CRMAcctLineEdit(this, pName));
    setNameVisible(true);
    setSubtype(CRMAcctLineEdit::Crmacct);
}

void CRMAcctCluster::setSubtype(const CRMAcctLineEdit::CRMAcctSubtype subtype)
{
  // TODO: make this do something useful
  if (_number->inherits("CRMAcctLineEdit"))
    (qobject_cast<CRMAcctLineEdit*>(_number))->setSubtype(subtype);
}

///////////////////////////////////////////////////////////////////////

CRMAcctLineEdit::CRMAcctSubtype CRMAcctCluster::subtype() const
{
  if (_number->inherits("CRMAcctLineEdit"))
    return (qobject_cast<CRMAcctLineEdit*>(_number))->subtype();
  return CRMAcctLineEdit::Crmacct;
}

CRMAcctLineEdit::CRMAcctLineEdit(QWidget* pParent, const char* pName) :
    VirtualClusterLineEdit(pParent, "crmacct", "crmacct_id", "crmacct_number", "crmacct_name", 0, 0, pName, "crmacct_active")
{
  setTitles(tr("CRM Account"), tr("CRM Accounts"));
  setUiName("crmaccount");
  setEditPriv("MaintainCRMAccounts");
  setViewPriv("ViewCRMAccounts");
  setNewPriv("MaintainCRMAccounts");

    setSubtype(Crmacct);

    _query = "SELECT crmacct_id AS id, crmacct_number AS number, crmacct_name AS name,"
           "       crmacct_active AS active, addr.*,"
           "       formatAddr(addr_line1, addr_line2, addr_line3, '', '') AS street,"
           "       cntct.* "
           "    FROM crmacct LEFT OUTER JOIN"
           "         cntct ON (crmacct_cntct_id_1=cntct_id) LEFT OUTER JOIN"
           "         addr ON (cntct_addr_id=addr_id)"
           "    WHERE (true ) ";
}

VirtualList* CRMAcctLineEdit::listFactory()
{
  return new CRMAcctList(this);
}

VirtualSearch* CRMAcctLineEdit::searchFactory()
{
  return new CRMAcctSearch(this);
}

///////////////////////////////////////////////////////////////////////

void CRMAcctLineEdit::setSubtype(const CRMAcctSubtype subtype)
{
  _subtype = subtype;
  //TODO: refigure everything about this line edit, including find the id for the current text
}

CRMAcctLineEdit::CRMAcctSubtype CRMAcctLineEdit::subtype() const
{
  return _subtype;
}

///////////////////////////////////////////////////////////////////////

CRMAcctList::CRMAcctList(QWidget* pParent, const char* pName, bool, Qt::WFlags pFlags) :
  VirtualList(pParent, pFlags)
{
  _parent = pParent;

  if (!pName)
    setObjectName("CRMAcctList");

  _listTab->setColumnCount(0);

  _listTab->addColumn(tr("Number"),      80, Qt::AlignCenter,true, "number");
  _listTab->addColumn(tr("Name"),        75, Qt::AlignLeft,  true, "name"  );
  _listTab->addColumn(tr("First"),      100, Qt::AlignLeft,  true, "cntct_first_name");
  _listTab->addColumn(tr("Last"),       100, Qt::AlignLeft,  true, "cntct_last_name");
  _listTab->addColumn(tr("Phone"),      100, Qt::AlignLeft,  true, "cntct_phone");
  _listTab->addColumn(tr("Email"),      100, Qt::AlignLeft,  true, "cntct_email");
  _listTab->addColumn(tr("Address"),    100, Qt::AlignLeft|Qt::AlignTop,true,"street");
  _listTab->addColumn(tr("City"),        75, Qt::AlignLeft,  true, "addr_city");
  _listTab->addColumn(tr("State"),       50, Qt::AlignLeft,  true, "addr_state");
  _listTab->addColumn(tr("Country"),    100, Qt::AlignLeft,  true, "addr_country");
  _listTab->addColumn(tr("Postal Code"), 75, Qt::AlignLeft,  true, "addr_postalcode");

  _showInactive = false;	// must be before inherits() checks

  if (_parent->inherits("CRMAcctCluster")) // handles Crmacct, Competitor, Partner, Prospect, Taxauth
    setSubtype((qobject_cast<CRMAcctCluster*>(_parent))->subtype());
  else if (_parent->inherits("CRMAcctLineEdit"))
    setSubtype((qobject_cast<CRMAcctLineEdit*>(_parent))->subtype());
  else if (_parent->inherits("CLineEdit") || _parent->inherits("CustCluster") ||
           _parent->inherits("CustInfo"))
  {
    CLineEdit::CLineEditTypes type = CLineEdit::AllCustomersAndProspects;

    if (_parent->inherits("CLineEdit"))
      type = (qobject_cast<CLineEdit*>(_parent))->type();
    else if (_parent->inherits("CustCluster"))
      type = (qobject_cast<CustCluster*>(_parent))->type();
 //   else if (_parent->inherits("CustInfo"))
 //     type = (qobject_cast<CustInfo*>(_parent))->type();

    switch (type)
    {
      case CLineEdit::AllCustomers:
	setSubtype(CRMAcctLineEdit::Cust);
	_showInactive = true;
	break;

      case CLineEdit::ActiveCustomers:
	setSubtype(CRMAcctLineEdit::Cust);
	_showInactive = false;
	break;

      case CLineEdit::AllProspects:
	setSubtype(CRMAcctLineEdit::Prospect);
	_showInactive = true;
	break;

      case CLineEdit::ActiveProspects:
	setSubtype(CRMAcctLineEdit::Prospect);
	_showInactive = false;
	break;

      case CLineEdit::AllCustomersAndProspects:
	setSubtype(CRMAcctLineEdit::CustAndProspect);
	_showInactive = true;
	break;

      case CLineEdit::ActiveCustomersAndProspects:
	setSubtype(CRMAcctLineEdit::CustAndProspect);
	_showInactive = false;
	break;

    }
  }
  else if (_parent->inherits("VendorLineEdit") || _parent->inherits("VendorCluster"))
    setSubtype(CRMAcctLineEdit::Vend);
  else
    setSubtype(CRMAcctLineEdit::Crmacct);

  resize(800, size().height());
}

void CRMAcctList::setId(const int id)
{
  _id = id;
  _listTab->setId(id);
}

void CRMAcctList::setShowInactive(const bool show)
{
  if (_showInactive != show)
  {
    _showInactive = show;
    sFillList();
  }
}

void CRMAcctList::setSubtype(const CRMAcctLineEdit::CRMAcctSubtype subtype)
{
  _subtype = subtype;

  switch (subtype)
  {
  case CRMAcctLineEdit::Cust:
    setWindowTitle(tr("Search For Customer"));
    _query =
	"SELECT cust_id AS id, cust_number AS number, cust_name AS name,"
        "       addr.*, "
	"       formatAddr(addr_line1, addr_line2, addr_line3, '', '') AS street,"
	"       cntct.* "
	"    FROM custinfo LEFT OUTER JOIN"
	"         cntct ON (cust_cntct_id=cntct_id) LEFT OUTER JOIN"
	"         addr ON (cntct_addr_id=addr_id)"
	"<? if exists(\"activeOnly\") ?>"
	"WHERE cust_active "
	"<? endif ?>"
	"ORDER BY number;"  ;
    break;

  case CRMAcctLineEdit::Prospect:
    setWindowTitle(tr("Search For Prospect"));
    _query =
	"SELECT prospect_id AS id, prospect_number AS number, prospect_name AS name,"
        "       addr.*,"
	"       formatAddr(addr_line1, addr_line2, addr_line3, '', '') AS street,"
	"       cntct.* "
	"    FROM prospect LEFT OUTER JOIN"
	"         cntct ON (prospect_cntct_id=cntct_id) LEFT OUTER JOIN"
	"         addr ON (cntct_addr_id=addr_id)"
	"<? if exists(\"activeOnly\") ?>"
	"WHERE prospect_active "
	"<? endif ?>"
	"ORDER BY number;"  ;
    break;

  case CRMAcctLineEdit::Taxauth:
    setWindowTitle(tr("Search For Tax Authority"));
    _listTab->hideColumn(2);
    _listTab->hideColumn(3);
    _listTab->hideColumn(4);
    _query =
	"SELECT taxauth_id AS id, taxauth_code AS number, taxauth_name AS name,"
        "       addr.*,"
	"       formatAddr(addr_line1, addr_line2, addr_line3, '', '') AS street,"
	"       '' AS cntct_first_name, '' AS cntct_last_name, "
	"       '' AS cntct_phone "
	"    FROM taxauth LEFT OUTER JOIN"
	"         addr ON (taxauth_addr_id=addr_id)"
	/*
	"<? if exists(\"activeOnly\") ?>"
	"WHERE taxauth_active "
	"<? endif ?>"
	*/
	"ORDER BY number;"  ;
    break;

  case CRMAcctLineEdit::Vend:
    _listTab->addColumn("Vend. Type", _itemColumn, Qt::AlignLeft, true, "type");
    setWindowTitle(tr("Search For Vendor"));
    _query =
	"SELECT vend_id AS id, vend_number AS number, vend_name AS name, vendtype_code AS type,"
        "       addr.*,"
	"       formatAddr(addr_line1, addr_line2, addr_line3, '', '') AS street,"
	"       cntct.* "
	"    FROM vendtype JOIN vendinfo"
        "           ON (vend_vendtype_id=vendtype_id) LEFT OUTER JOIN"
	"         cntct ON (vend_cntct1_id=cntct_id) LEFT OUTER JOIN"
	"         addr ON (vend_addr_id=addr_id)"
	"<? if exists(\"activeOnly\") ?>"
	"WHERE vend_active "
	"<? endif ?>"
	"ORDER BY number;"  ;
    break;

  case CRMAcctLineEdit::CustAndProspect:
    setWindowTitle(tr("Search For Customer or Prospect"));
    _query =
	"SELECT cust_id AS id, cust_number AS number, cust_name AS name,"
        "       addr.*, "
	"       formatAddr(addr_line1, addr_line2, addr_line3, '', '') AS street,"
	"       cntct.* "
	"    FROM custinfo LEFT OUTER JOIN"
	"         cntct ON (cust_cntct_id=cntct_id) LEFT OUTER JOIN"
	"         addr ON (cntct_addr_id=addr_id)"
	"<? if exists(\"activeOnly\") ?>"
	"WHERE cust_active "
	"<? endif ?>"
	"UNION "
	"SELECT prospect_id AS id, prospect_number AS number, prospect_name AS name,"
        "       addr.*,"
	"       formatAddr(addr_line1, addr_line2, addr_line3, '', '') AS street,"
	"       cntct.* "
	"    FROM prospect LEFT OUTER JOIN"
	"         cntct ON (prospect_cntct_id=cntct_id) LEFT OUTER JOIN"
	"         addr ON (cntct_addr_id=addr_id)"
	"<? if exists(\"activeOnly\") ?>"
	"WHERE prospect_active "
	"<? endif ?>"
	"ORDER BY number;"  ;
    break;

  case CRMAcctLineEdit::Crmacct:
  case CRMAcctLineEdit::Competitor:
  case CRMAcctLineEdit::Partner:
  default:
    setWindowTitle(tr("Search For CRM Account"));
    _query =
	"SELECT crmacct_id AS id, crmacct_number AS number, crmacct_name AS name,"
        "       addr.*,"
	"       formatAddr(addr_line1, addr_line2, addr_line3, '', '') AS street,"
	"       cntct.* "
	"    FROM crmacct LEFT OUTER JOIN"
	"         cntct ON (crmacct_cntct_id_1=cntct_id) LEFT OUTER JOIN"
	"         addr ON (cntct_addr_id=addr_id)"
	"<? if exists(\"activeOnly\") ?>"
	"WHERE crmacct_active "
	"<? endif ?>"
	"ORDER BY number;"  ;
    break;
  }

  sFillList();
}

void CRMAcctList::sFillList()
{
  MetaSQLQuery mql(_query);
  ParameterList params;
  if (! _showInactive)
    params.append("activeOnly");

  XSqlQuery fillq = mql.toQuery(params);

  _listTab->populate(fillq);
  if (fillq.lastError().type() != QSqlError::NoError)
  {
    QMessageBox::critical(this, tr("A System Error Occurred at %1::%2")
				.arg(__FILE__).arg(__LINE__),
			  fillq.lastError().databaseText());
    return;
  }
}

///////////////////////////////////////////////////////////////////////

CRMAcctSearch::CRMAcctSearch(QWidget* pParent, Qt::WindowFlags pFlags) :
  VirtualSearch(pParent, pFlags)
{
  // remove the stuff we won't use
  disconnect(_searchDescrip,	SIGNAL(toggled(bool)), this, SLOT(sFillList()));
  selectorsLyt->removeWidget(_searchDescrip);
  delete _searchDescrip;

  _listTab->setColumnCount(0);

  _addressLit	    = new QLabel(tr("Primary Contact Address:"),this);
  _searchStreet	    = new XCheckBox(tr("Street Address"), this);
  _searchCity	    = new XCheckBox(tr("City"),this);
  _searchState	    = new XCheckBox(tr("State"),this);
  _searchPostalCode = new XCheckBox(tr("Postal Code"),this);
  _searchCountry    = new XCheckBox(tr("Country"),this);
  _searchContact    = new XCheckBox(tr("Contact Name"),this);
  _searchPhone	    = new XCheckBox(tr("Contact Phone #"),this);
  _searchEmail	    = new XCheckBox(tr("Contact Email "),this);
  _showInactive	    = new QCheckBox(tr("Show Inactive"),this);
  _searchCombo      = new XCheckBox(tr("Search Combo"),this);
  _comboCombo       = new XComboBox(this, "_comboCombo");
  
  _addressLit->setObjectName("_addressLit");
  _searchStreet->setObjectName("_searchStreet");
  _searchCity->setObjectName("_searchCity");
  _searchState->setObjectName("_searchState");
  _searchPostalCode->setObjectName("_searchPostalCode");
  _searchCountry->setObjectName("_searchCountry");
  _searchContact->setObjectName("_searchContact");
  _searchPhone->setObjectName("_searchPhone");
  _searchEmail->setObjectName("_searchEmail");
  _showInactive->setObjectName("_showInactive");
  _searchCombo->setObjectName("_searchCombo");

  selectorsLyt->removeWidget(_searchName);
  selectorsLyt->removeWidget(_searchNumber);
  QLabel * lbl = new QLabel(tr("Search through:"), this);
  lbl->setObjectName("_searchLit");
  selectorsLyt->addWidget(lbl,                  0, 0);
  selectorsLyt->addWidget(_searchNumber,	1, 0);
  selectorsLyt->addWidget(_searchName,		2, 0);
  selectorsLyt->addWidget(_searchContact,	1, 1);
  selectorsLyt->addWidget(_searchPhone,		2, 1);
  selectorsLyt->addWidget(_searchEmail,		3, 1);
  selectorsLyt->addWidget(_addressLit,		0, 2);
  selectorsLyt->addWidget(_searchStreet,	1, 2);
  selectorsLyt->addWidget(_searchCity,		2, 2);
  selectorsLyt->addWidget(_searchState,		3, 2);
  selectorsLyt->addWidget(_searchPostalCode,	4, 2);
  selectorsLyt->addWidget(_searchCombo,         5, 0);
  selectorsLyt->addWidget(_comboCombo,          5, 1);
  selectorsLyt->addWidget(_searchCountry,	5, 2);
  selectorsLyt->addWidget(_showInactive,	5, 3);

  _listTab->addColumn(tr("Number"),      80, Qt::AlignCenter,true, "number");
  _listTab->addColumn(tr("Name"),        75, Qt::AlignLeft,  true, "name"  );
  _listTab->addColumn(tr("First"),      100, Qt::AlignLeft,  true, "cntct_first_name");
  _listTab->addColumn(tr("Last"),       100, Qt::AlignLeft,  true, "cntct_last_name");
  _listTab->addColumn(tr("Phone"),      100, Qt::AlignLeft,  true, "cntct_phone");
  _listTab->addColumn(tr("Email"),      100, Qt::AlignLeft,  true, "cntct_email");
  _listTab->addColumn(tr("Address"),    100, Qt::AlignLeft|Qt::AlignTop,true,"street");
  _listTab->addColumn(tr("City"),        75, Qt::AlignLeft,  true, "addr_city");
  _listTab->addColumn(tr("State"),       50, Qt::AlignLeft,  true, "addr_state");
  _listTab->addColumn(tr("Country"),    100, Qt::AlignLeft,  true, "addr_country");
  _listTab->addColumn(tr("Postal Code"), 75, Qt::AlignLeft,  true, "addr_postalcode");

  setTabOrder(_search,		_searchNumber);
  setTabOrder(_searchNumber,	_searchName);
  setTabOrder(_searchName,	_searchContact);
  setTabOrder(_searchContact,	_searchPhone);
  setTabOrder(_searchPhone,	_searchEmail);
  setTabOrder(_searchEmail,	_searchStreet);
  setTabOrder(_searchStreet,	_searchCity);
  setTabOrder(_searchCity,	_searchState);
  setTabOrder(_searchState,	_searchPostalCode);
  setTabOrder(_searchPostalCode,_searchCountry);
  setTabOrder(_searchCountry,	_searchCombo);
  setTabOrder(_searchCombo,     _comboCombo);
  setTabOrder(_comboCombo,      _showInactive);
  setTabOrder(_showInactive,	_listTab);
  setTabOrder(_listTab,		_buttonBox);
  setTabOrder(_buttonBox,	_search);

  resize(800, size().height());
  
  _parent = pParent;
  setObjectName("crmacctSearch");
  if (_parent->inherits("CRMAcctCluster")) // handles Crmacct, Competitor, Partner, Prospect, Taxauth
    setSubtype((qobject_cast<CRMAcctCluster*>(_parent))->subtype());
  else if (_parent->inherits("CRMAcctLineEdit"))
    setSubtype((qobject_cast<CRMAcctLineEdit*>(_parent))->subtype());
  else if (_parent->inherits("CLineEdit") || _parent->inherits("CustCluster"))
  {
    CLineEdit::CLineEditTypes type = _parent->inherits("CLineEdit") ?
				  (qobject_cast<CLineEdit*>(_parent))->type() :
				  (qobject_cast<CustCluster*>(_parent))->type();
    switch (type)
    {
      case CLineEdit::AllCustomers:
	setSubtype(CRMAcctLineEdit::Cust);
	_showInactive->setChecked(true);
	break;

      case CLineEdit::ActiveCustomers:
	setSubtype(CRMAcctLineEdit::Cust);
	_showInactive->setChecked(false);
	break;

      case CLineEdit::AllProspects:
	setSubtype(CRMAcctLineEdit::Prospect);
	_showInactive->setChecked(true);
	break;

      case CLineEdit::ActiveProspects:
	setSubtype(CRMAcctLineEdit::Prospect);
	_showInactive->setChecked(false);
	break;

      case CLineEdit::AllCustomersAndProspects:
	setSubtype(CRMAcctLineEdit::CustAndProspect);
	_showInactive->setChecked(true);
	break;

      case CLineEdit::ActiveCustomersAndProspects:
	setSubtype(CRMAcctLineEdit::CustAndProspect);
	_showInactive->setChecked(false);
	break;

    }
  }
  else if (_parent->inherits("VendorCluster")	||
	   _parent->inherits("VendorInfo")	||
	   _parent->inherits("VendorLineEdit"))
  {
    setSubtype(CRMAcctLineEdit::Vend);
  }
  else
    setSubtype(CRMAcctLineEdit::Crmacct);

  // do this late so the constructor can set defaults without triggering queries
  connect(_search,	 SIGNAL(lostFocus()),	this, SLOT(sFillList()));
  connect(_searchStreet, SIGNAL(toggled(bool)), this, SLOT(sFillList()));
  connect(_searchCity,   SIGNAL(toggled(bool)),	this, SLOT(sFillList()));
  connect(_searchState,  SIGNAL(toggled(bool)),	this, SLOT(sFillList()));
  connect(_searchPostalCode,SIGNAL(toggled(bool)), this, SLOT(sFillList()));
  connect(_searchCountry,SIGNAL(toggled(bool)),	this, SLOT(sFillList()));
  connect(_searchContact,SIGNAL(toggled(bool)),	this, SLOT(sFillList()));
  connect(_searchPhone,	 SIGNAL(toggled(bool)),	this, SLOT(sFillList()));
  connect(_searchEmail,	 SIGNAL(toggled(bool)),	this, SLOT(sFillList()));
  connect(_searchCombo,  SIGNAL(toggled(bool)), this, SLOT(sFillList()));
  connect(_comboCombo,   SIGNAL(newID(int)),    this, SLOT(sFillList()));

  _search->setFocus();
}

void CRMAcctSearch::setId(const int id)
{
  _id = id;
  _listTab->setId(id);
}

void CRMAcctSearch::setShowInactive(const bool show)
{
  if (show != _showInactive->isChecked())
    _showInactive->setChecked(show);
}

void CRMAcctSearch::setSubtype(const CRMAcctLineEdit::CRMAcctSubtype subtype)
{
  _subtype = subtype;

  if(subtype != CRMAcctLineEdit::Vend)
  {
    _searchCombo->hide();
    _comboCombo->hide();
  }
 

  switch (subtype)
  {
  case CRMAcctLineEdit::Cust:
    setWindowTitle(tr("Search For Customer"));
    _searchNumber->setText(tr("Customer Number"));
    _searchName->setText(tr("Customer Name"));
    _searchContact->setText(tr("Billing Contact Name"));
    _searchPhone->setText(tr("Billing Contact Phone #"));
    _searchEmail->setText(tr("Billing Contact Email"));
    _addressLit->setText(tr("Billing Contact Address:"));
    break;

  case CRMAcctLineEdit::Prospect:
    setWindowTitle(tr("Search For Prospect"));
    _searchNumber->setText(tr("Prospect Number"));
    _searchName->setText(tr("Prospect Name"));
    break;

  case CRMAcctLineEdit::Taxauth:
    setWindowTitle(tr("Search For Tax Authority"));
    _searchNumber->setText(tr("Tax Authority Code"));
    _searchName->setText(tr("Tax Authority Name"));
    _searchContact->setVisible(false);
    _searchPhone->setVisible(false);
    _searchEmail->setVisible(false);
    _addressLit->setText(tr("Tax Authority Address:"));
    _listTab->hideColumn(2);
    _listTab->hideColumn(3);
    _listTab->hideColumn(4);
    _showInactive->setVisible(false);
    break;

  case CRMAcctLineEdit::Vend:
    setWindowTitle(tr("Search For Vendor"));
    _searchCombo->setText(tr("Vendor Type:"));
    _comboCombo->setType(XComboBox::VendorTypes);
    _searchNumber->setText(tr("Vendor Number"));
    _searchName->setText(tr("Vendor Name"));
    _addressLit->setText(tr("Main Address:"));
    _listTab->addColumn("Vend. Type", _itemColumn, Qt::AlignLeft, true, "type");
    break;

  case CRMAcctLineEdit::CustAndProspect:
    setWindowTitle(tr("Search For Customer or Prospect"));
    _searchNumber->setText(tr("Number"));
    _searchName->setText(tr("Name"));
    _searchContact->setText(tr("Billing or Primary Contact Name"));
    _searchPhone->setText(tr("Billing or Primary Contact Phone #"));
    _searchPhone->setText(tr("Billing or Primary Contact Email"));
    _addressLit->setText(tr("Billing or Primary Contact Address:"));
    break;

  case CRMAcctLineEdit::Crmacct:
  case CRMAcctLineEdit::Competitor:
  case CRMAcctLineEdit::Partner:
  default:
    setWindowTitle(tr("Search For CRM Account"));
    _searchNumber->setText(tr("CRM Account Number"));
    _searchName->setText(tr("CRM Account Name"));
    _searchContact->setText(tr("Primary Contact Name"));
    _searchPhone->setText(tr("Primary Contact Phone #"));
    _searchPhone->setText(tr("Primary Contact Emai"));
    _addressLit->setText(tr("Primary Contact Address:"));
    break;
  }

  sFillList();
}

void CRMAcctSearch::sFillList()
{
  if (_search->text().trimmed().length() == 0)
    return;

  QString sql;

  sql = "<? if exists(\"custAndProspect\") ?>"
	"SELECT cust_id AS id, cust_number AS number, cust_name AS name,"
        "       addr.*,"
	"       formatAddr(addr_line1, addr_line2, addr_line3, '', '') AS street,"
	"       cntct.* "
	"    FROM custinfo LEFT OUTER JOIN"
	"         cntct ON (cust_cntct_id=cntct_id) LEFT OUTER JOIN"
	"         addr ON (cntct_addr_id=addr_id) "
	"    WHERE "
	"    <? if exists(\"activeOnly\") ?> cust_active AND <? endif ?>"
	"      (false "
	"    <? if exists(\"searchNumber\") ?>"
	"       OR (UPPER(cust_number) ~ <? value(\"searchString\") ?>)"
	"    <? endif ?>"
	"    <? if exists(\"searchName\") ?>"
	"       OR (UPPER(cust_name) ~ <? value(\"searchString\") ?>)"
	"    <? endif ?>"
	"    <? if exists(\"searchContactName\") ?>"
	"       OR (UPPER(cntct_first_name || ' ' || cntct_last_name) "
	"                 ~ <? value(\"searchString\") ?>)"
	"    <? endif ?>"
	"    <? if exists(\"searchPhone\") ?>"
	"       OR (UPPER(cntct_phone || ' ' || cntct_phone2 || ' ' || "
	"                 cntct_fax) ~ <? value(\"searchString\") ?>)"
	"    <? endif ?>"
	"    <? if exists(\"searchEmail\") ?>"
	"       OR (cntct_email ~* <? value(\"searchString\") ?>)"
	"    <? endif ?>"
	"    <? if exists(\"searchStreetAddr\") ?>"
	"       OR (UPPER(addr_line1 || ' ' || addr_line2 || ' ' || "
	"                 addr_line3) ~ <? value(\"searchString\") ?>)"
	"    <? endif ?>"
	"    <? if exists(\"searchCity\") ?>"
	"       OR (UPPER(addr_city) ~ <? value(\"searchString\") ?>)"
	"    <? endif ?>"
	"    <? if exists(\"searchState\") ?>"
	"       OR (UPPER(addr_state) ~ <? value(\"searchString\") ?>)"
	"    <? endif ?>"
	"    <? if exists(\"searchPostalCode\") ?>"
	"       OR (UPPER(addr_postalcode) ~ <? value(\"searchString\") ?>)"
	"    <? endif ?>"
	"    <? if exists(\"searchCountry\") ?>"
	"       OR (UPPER(addr_country) ~ <? value(\"searchString\") ?>)"
	"    <? endif ?>"
	") "
	"UNION "
	"SELECT prospect_id AS id, prospect_number AS number, prospect_name AS name,"
        "       addr.*,"
	"       formatAddr(addr_line1, addr_line2, addr_line3, '', '') AS street,"
	"       cntct.* "
	"    FROM prospect LEFT OUTER JOIN"
	"         cntct ON (prospect_cntct_id=cntct_id) LEFT OUTER JOIN"
	"         addr ON (cntct_addr_id=addr_id) "
	"    WHERE "
	"    <? if exists(\"activeOnly\") ?> prospect_active AND <? endif ?>"
	"      (false "
	"    <? if exists(\"searchNumber\") ?>"
	"       OR (UPPER(prospect_number) ~ <? value(\"searchString\") ?>)"
	"    <? endif ?>"
	"    <? if exists(\"searchName\") ?>"
	"       OR (UPPER(prospect_name) ~ <? value(\"searchString\") ?>)"
	"    <? endif ?>"
	"    <? if exists(\"searchContactName\") ?>"
	"       OR (UPPER(cntct_first_name || ' ' || cntct_last_name) "
	"                 ~ <? value(\"searchString\") ?>)"
	"    <? endif ?>"
	"    <? if exists(\"searchPhone\") ?>"
	"       OR (UPPER(cntct_phone || ' ' || cntct_phone2 || ' ' || "
	"                 cntct_fax) ~ <? value(\"searchString\") ?>)"
	"    <? endif ?>"
	"    <? if exists(\"searchEmail\") ?>"
	"       OR (cntct_email ~* <? value(\"searchString\") ?>)"
	"    <? endif ?>"
	"    <? if exists(\"searchStreetAddr\") ?>"
	"       OR (UPPER(addr_line1 || ' ' || addr_line2 || ' ' || "
	"                 addr_line3) ~ <? value(\"searchString\") ?>)"
	"    <? endif ?>"
	"    <? if exists(\"searchCity\") ?>"
	"       OR (UPPER(addr_city) ~ <? value(\"searchString\") ?>)"
	"    <? endif ?>"
	"    <? if exists(\"searchState\") ?>"
	"       OR (UPPER(addr_state) ~ <? value(\"searchString\") ?>)"
	"    <? endif ?>"
	"    <? if exists(\"searchPostalCode\") ?>"
	"       OR (UPPER(addr_postalcode) ~ <? value(\"searchString\") ?>)"
	"    <? endif ?>"
	"    <? if exists(\"searchCountry\") ?>"
	"       OR (UPPER(addr_country) ~ <? value(\"searchString\") ?>)"
	"    <? endif ?>"
	"<? elseif exists(\"crmacct\") ?>"
	"SELECT crmacct_id AS id, crmacct_number AS number, crmacct_name AS name,"
        "       addr.*,"
	"       formatAddr(addr_line1, addr_line2, addr_line3, '', '') AS street,"
	"       cntct.* "
	"    FROM crmacct LEFT OUTER JOIN"
	"         cntct ON (crmacct_cntct_id_1=cntct_id) LEFT OUTER JOIN"
	"         addr ON (cntct_addr_id=addr_id) "
	"    WHERE "
	"    <? if exists(\"activeOnly\") ?> crmacct_active AND <? endif ?>"
	"      (false "
	"    <? if exists(\"searchNumber\") ?>"
	"       OR (UPPER(crmacct_number) ~ <? value(\"searchString\") ?>)"
	"    <? endif ?>"
	"    <? if exists(\"searchName\") ?>"
	"       OR (UPPER(crmacct_name) ~ <? value(\"searchString\") ?>)"
	"    <? endif ?>"
	"<? elseif exists(\"cust\") ?>"
	"SELECT cust_id AS id, cust_number AS number, cust_name AS name,"
        "       addr.*,"
	"       formatAddr(addr_line1, addr_line2, addr_line3, '', '') AS street,"
	"       cntct.* "
	"    FROM custinfo LEFT OUTER JOIN"
	"         cntct ON (cust_cntct_id=cntct_id) LEFT OUTER JOIN"
	"         addr ON (cntct_addr_id=addr_id) "
	"    WHERE "
	"    <? if exists(\"activeOnly\") ?> cust_active AND <? endif ?>"
	"      (false "
	"    <? if exists(\"searchNumber\") ?>"
	"       OR (UPPER(cust_number) ~ <? value(\"searchString\") ?>)"
	"    <? endif ?>"
	"    <? if exists(\"searchName\") ?>"
	"       OR (UPPER(cust_name) ~ <? value(\"searchString\") ?>)"
	"    <? endif ?>"
	"<? elseif exists(\"prospect\") ?>"
	"SELECT prospect_id AS id, prospect_number AS number, prospect_name AS name,"
        "       addr.*,"
	"       formatAddr(addr_line1, addr_line2, addr_line3, '', '') AS street,"
	"       cntct.* "
	"    FROM prospect LEFT OUTER JOIN"
	"         cntct ON (prospect_cntct_id=cntct_id) LEFT OUTER JOIN"
	"         addr ON (cntct_addr_id=addr_id) "
	"    WHERE "
	"    <? if exists(\"activeOnly\") ?> prospect_active AND <? endif ?>"
	"      (false "
	"    <? if exists(\"searchNumber\") ?>"
	"       OR (UPPER(prospect_number) ~ <? value(\"searchString\") ?>)"
	"    <? endif ?>"
	"    <? if exists(\"searchName\") ?>"
	"       OR (UPPER(prospect_name) ~ <? value(\"searchString\") ?>)"
	"    <? endif ?>"
	"<? elseif exists(\"taxauth\") ?>"
	"SELECT taxauth_id AS id, taxauth_code AS number, taxauth_name AS name,"
        "       addr.*,"
	"       formatAddr(addr_line1, addr_line2, addr_line3, '', '') AS street,"
	"       '' AS cntct_first_name, '' AS cntct_last_name, "
	"       '' AS cntct_phone "
	"    FROM taxauth LEFT OUTER JOIN"
	"         addr ON (taxauth_addr_id=addr_id) "
	"    WHERE "
	"      (false "
	"    <? if exists(\"searchNumber\") ?>"
	"       OR (UPPER(taxauth_code) ~ <? value(\"searchString\") ?>)"
	"    <? endif ?>"
	"    <? if exists(\"searchName\") ?>"
	"       OR (UPPER(taxauth_name) ~ <? value(\"searchString\") ?>)"
	"    <? endif ?>"
	"<? elseif exists(\"vend\") ?>"
	"SELECT vend_id AS id, vend_number AS number, vend_name AS name, vendtype_code AS type,"
        "       addr.*,"
	"       formatAddr(addr_line1, addr_line2, addr_line3, '', '') AS street,"
	"       cntct.* "
	"    FROM vendtype, vendinfo LEFT OUTER JOIN"
	"         cntct ON (vend_cntct1_id=cntct_id) LEFT OUTER JOIN"
	"         addr ON (vend_addr_id=addr_id) "
	"    WHERE "
	"    <? if exists(\"activeOnly\") ?> vend_active AND <? endif ?>"
	"      (vend_vendtype_id=vendtype_id) "
        "    <? if exists(\"combo_id\") ?>"
        "      AND (vendtype_id=<? value(\"combo_id\") ?>)"
        "    <? endif ?>"
        "      AND (false "
	"    <? if exists(\"searchNumber\") ?>"
	"       OR (UPPER(vend_number) ~ <? value(\"searchString\") ?>)"
	"    <? endif ?>"
	"    <? if exists(\"searchName\") ?>"
	"       OR (UPPER(vend_name) ~ <? value(\"searchString\") ?>)"
	"    <? endif ?>"
	"<? endif ?>"
	"<? if exists(\"searchContactName\") ?>"
	"   OR (UPPER(cntct_first_name || ' ' || cntct_last_name) "
	"             ~ <? value(\"searchString\") ?>)"
	"<? endif ?>"
	"<? if exists(\"searchPhone\") ?>"
	"   OR (UPPER(cntct_phone || ' ' || cntct_phone2 || ' ' || "
	"             cntct_fax) ~ <? value(\"searchString\") ?>)"
	"<? endif ?>"
	"<? if exists(\"searchEmail\") ?>"
	"   OR (cntct_email ~* <? value(\"searchString\") ?>)"
	"<? endif ?>"
	"<? if exists(\"searchStreetAddr\") ?>"
	"   OR (UPPER(addr_line1 || ' ' || addr_line2 || ' ' || "
	"             addr_line3) ~ <? value(\"searchString\") ?>)"
	"<? endif ?>"
	"<? if exists(\"searchCity\") ?>"
	"   OR (UPPER(addr_city) ~ <? value(\"searchString\") ?>)"
	"<? endif ?>"
	"<? if exists(\"searchState\") ?>"
	"   OR (UPPER(addr_state) ~ <? value(\"searchString\") ?>)"
	"<? endif ?>"
	"<? if exists(\"searchPostalCode\") ?>"
	"   OR (UPPER(addr_postalcode) ~ <? value(\"searchString\") ?>)"
	"<? endif ?>"
	"<? if exists(\"searchCountry\") ?>"
	"   OR (UPPER(addr_country) ~ <? value(\"searchString\") ?>)"
	"<? endif ?>"
	"  )"
	"ORDER BY number;"  ;

  MetaSQLQuery mql(sql);
  ParameterList params;
  params.append("searchString", _search->text().trimmed().toUpper());

  switch (_subtype)
  {
    case CRMAcctLineEdit::Crmacct:
      params.append("crmacct");
      break;

    case CRMAcctLineEdit::Cust:
      params.append("cust");
      break;

    case CRMAcctLineEdit::Prospect:
      params.append("prospect");
      break;

    case CRMAcctLineEdit::Taxauth:
      params.append("taxauth");
      break;

    case CRMAcctLineEdit::Vend:
      params.append("vend");
      break;

    case CRMAcctLineEdit::CustAndProspect:
      params.append("custAndProspect");
      break;

    case CRMAcctLineEdit::Competitor:
    case CRMAcctLineEdit::Partner:
    default:
      return;
  }

  if (! _showInactive->isChecked())
    params.append("activeOnly");

  if (_searchNumber->isChecked())
    params.append("searchNumber");

  if (_searchName->isChecked())
    params.append("searchName");

  if (_subtype != CRMAcctLineEdit::Taxauth)	// taxauth doesn't have contacts (yet?)
  {
    if (_searchContact->isChecked())
      params.append("searchContactName");

    if (_searchPhone->isChecked())
      params.append("searchPhone");
      
    if (_searchEmail->isChecked())
      params.append("searchEmail");
  }

  if (_searchStreet->isChecked())
    params.append("searchStreetAddr");

  if (_searchCity->isChecked())
    params.append("searchCity");

  if (_searchState->isChecked())
    params.append("searchState");

  if (_searchPostalCode->isChecked())
    params.append("searchPostalCode");

  if (_searchCountry->isChecked())
    params.append("searchCountry");

  if (_searchCombo->isChecked())
    params.append("combo_id", _comboCombo->id());

  XSqlQuery fillq = mql.toQuery(params);

  _listTab->populate(fillq);
  if (fillq.lastError().type() != QSqlError::NoError)
  {
    QMessageBox::critical(this, tr("A System Error Occurred at %1::%2")
				.arg(__FILE__).arg(__LINE__),
			  fillq.lastError().databaseText());
    return;
  }
}
