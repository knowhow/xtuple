/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "crmaccount.h"

#include <QMessageBox>
#include <QSqlError>
#include <QVariant>
#include <QCloseEvent>

#include <metasql.h>

#include "characteristicAssignment.h"
#include "customer.h"
#include "incident.h"
#include "prospect.h"
#include "taxAuthority.h"
#include "vendor.h"
#include "storedProcErrorLookup.h"
#include "vendorWorkBench.h"
#include "mqlutil.h"
#include "lotSerialRegistration.h"
#include "parameterwidget.h"

#define DEBUG false

crmaccount::crmaccount(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  _todoList = new todoList(this, "todoList", Qt::Widget);
  _todoListTab->layout()->addWidget(_todoList);
  _todoList->setCloseVisible(false);
  _todoList->list()->hideColumn("crmacct_number");
  _todoList->list()->hideColumn("crmacct_name");
  _todoList->parameterWidget()->setDefault(tr("Assigned"), QVariant(), true);
  _todoList->setParameterWidgetVisible(false);
  _todoList->setQueryOnStartEnabled(false);
  
  _contacts = new contacts(this, "contacts", Qt::Widget);
  _allPage->layout()->addWidget(_contacts);
  _contacts->setCloseVisible(false);
  _contacts->list()->hideColumn("crmacct_number");
  _contacts->list()->hideColumn("crmacct_name");
  _contacts->setParameterWidgetVisible(false);
  _contacts->setQueryOnStartEnabled(false);
  
  _oplist = new opportunityList(this, "opportunityList", Qt::Widget);
  _oplistTab->layout()->addWidget(_oplist);
  _oplist->setCloseVisible(false);
  _oplist->list()->hideColumn("crmacct_number");
  _oplist->parameterWidget()->setDefault(tr("User"), QVariant(), true);
  _oplist->setParameterWidgetVisible(false);
  _oplist->setQueryOnStartEnabled(false);
    
  if(!_privileges->check("EditOwner")) _owner->setEnabled(false);

  _owner->setUsername(omfgThis->username());
  _owner->setType(UsernameLineEdit::UsersActive);
    
  connect(_close,		SIGNAL(clicked()), this, SLOT(sClose()));
  connect(_competitor,		SIGNAL(clicked()), this, SLOT(sCompetitor()));
  connect(_deleteCharacteristic,SIGNAL(clicked()), this, SLOT(sDeleteCharacteristic()));
  connect(_deleteReg,		SIGNAL(clicked()), this, SLOT(sDeleteReg()));
  connect(_editCharacteristic,	SIGNAL(clicked()), this, SLOT(sEditCharacteristic()));
  connect(_editReg,		SIGNAL(clicked()), this, SLOT(sEditReg()));
  connect(_newCharacteristic,	SIGNAL(clicked()), this, SLOT(sNewCharacteristic()));
  connect(_newReg,		SIGNAL(clicked()), this, SLOT(sNewReg()));
  connect(_partner,		SIGNAL(clicked()), this, SLOT(sPartner()));
  connect(_prospectButton,	SIGNAL(clicked()), this, SLOT(sProspect()));
  connect(_save,		SIGNAL(clicked()), this, SLOT(sSave()));
  connect(_taxauthButton,	SIGNAL(clicked()), this, SLOT(sTaxAuth()));
  connect(omfgThis, SIGNAL(customersUpdated(int, bool)), this, SLOT(sUpdateRelationships()));
  connect(omfgThis, SIGNAL(prospectsUpdated()),  this, SLOT(sUpdateRelationships()));
  connect(omfgThis, SIGNAL(taxAuthsUpdated(int)),this, SLOT(sUpdateRelationships()));
  connect(omfgThis, SIGNAL(vendorsUpdated()),    this, SLOT(sUpdateRelationships()));
  connect(_customer, SIGNAL(toggled(bool)), this, SLOT(sCustomerToggled()));
  connect(_prospect, SIGNAL(toggled(bool)), this, SLOT(sProspectToggled()));
  connect(_number, SIGNAL(lostFocus()), this, SLOT(sCheckNumber()));
  connect(_primaryButton, SIGNAL(toggled(bool)), this, SLOT(sHandleButtons())); 	 
  connect(_secondaryButton, SIGNAL(toggled(bool)), this, SLOT(sHandleButtons())); 	 
  connect(_allButton, SIGNAL(toggled(bool)), this, SLOT(sHandleButtons()));
  connect(_customerButton, SIGNAL(clicked()), this, SLOT(sCustomer()));
  connect(_vendorButton, SIGNAL(clicked()), this, SLOT(sEditVendor()));
  connect(_contacts, SIGNAL(cntctDetached(int)), this, SLOT(sHandleCntctDetach(int)));

  _charass->addColumn(tr("Characteristic"), _itemColumn, Qt::AlignLeft, true, "char_name");
  _charass->addColumn(tr("Value"),          -1,          Qt::AlignLeft, true, "charass_value");
  
  _reg->addColumn(tr("Lot/Serial")  ,  _itemColumn,  Qt::AlignLeft, true, "ls_number" );
  _reg->addColumn(tr("Item")        ,  _itemColumn,  Qt::AlignLeft, true, "item_number" );
  _reg->addColumn(tr("Description") ,  -1,  	     Qt::AlignLeft, true, "item_descrip1" );
  _reg->addColumn(tr("Qty"       )  ,  _qtyColumn,   Qt::AlignLeft, true, "lsreg_qty" );
  _reg->addColumn(tr("Sold"        ),  _dateColumn,  Qt::AlignLeft, true, "lsreg_solddate" );
  _reg->addColumn(tr("Registered"  ),  _dateColumn,  Qt::AlignLeft, true, "lsreg_regdate" );
  _reg->addColumn(tr("Expires"   )  ,  _dateColumn,  Qt::AlignLeft, true, "lsreg_expiredate" );

  if (_preferences->boolean("XCheckBox/forgetful"))
    _active->setChecked(true);

  _NumberGen    = -1;
  _mode		= cNew;
  _crmacctId    = -1;
  _competitorId	= -1;
  _custId       = -1;
  _partnerId    = -1;
  _prospectId   = -1;
  _taxauthId    = -1;
  _vendId       = -1;
  _comments->setId(-1);
  _documents->setId(-1);
  _modal        = false;
  
  if (!_metrics->boolean("LotSerialControl"))
    _tab->removeTab(_tab->indexOf(_registrationsTab));

}

crmaccount::~crmaccount()
{
    // no need to delete child widgets, Qt does it all for us
}

void crmaccount::languageChange()
{
    retranslateUi(this);
}

enum SetResponse crmaccount::set(const ParameterList &pParams)
{
  XWidget::set(pParams);
  QVariant param;
  bool     valid;

  if (_mode == cView)
  {
    _customer->setEnabled(false);
    _prospect->setEnabled(false);
    _taxauth->setEnabled(false);
    _vendor->setEnabled(false);
    _partner->setEnabled(false);
    _competitor->setEnabled(false);
  }
  else
  {
    _customer->setEnabled(_privileges->check("MaintainCustomerMasters") && !_modal);
    _prospect->setEnabled(_privileges->check("MaintainProspectMasters") && !_modal);
    _taxauth->setEnabled(_privileges->check("MaintainTaxAuthorities") && !_modal);
    _vendor->setEnabled(_privileges->check("MaintainVendors") && !_modal);
    _partner->setEnabled(_privileges->check("MaintainPartners"));
    _competitor->setEnabled(_privileges->check("MaintainCompetitorMasters"));
  }

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      _mode = cNew;
      _number->setFocus();

      q.prepare("SELECT createCrmAcct('TEMPORARY' || (last_value + 1), '',"
                "		      false, 'O', NULL, NULL, NULL, NULL,"
                "		      NULL, NULL, NULL, NULL) AS result "
                "FROM crmacct_crmacct_id_seq;");
      q.bindValue(":crmacct_id", _crmacctId);
      q.exec();
      if (q.first())
      {
        _crmacctId = q.value("result").toInt();
        _todoList->parameterWidget()->setDefault(tr("CRM Account"), _crmacctId, true);
        _contacts->setCrmacctid(_crmacctId);
        _oplist->parameterWidget()->setDefault(tr("CRM Account"), _crmacctId, true);
        if (_crmacctId < 0)
        {
          QMessageBox::critical(this, tr("Error creating Initial Account"),
                storedProcErrorLookup("createCrmAcct", _crmacctId));
          _crmacctId = -1;
          _todoList->parameterWidget()->setDefault(tr("CRM Account"), _crmacctId, true);
          _contacts->setCrmacctid(_crmacctId);
          _oplist->parameterWidget()->setDefault(tr("CRM Account"), _crmacctId, true);
          return UndefinedError;
        }
        _comments->setId(_crmacctId);
        _documents->setId(_crmacctId);
      }
      else if (q.lastError().type() != QSqlError::NoError)
      {
        systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
        return UndefinedError;
      }
      _number->clear();

      if(((_metrics->value("CRMAccountNumberGeneration") == "A") ||
          (_metrics->value("CRMAccountNumberGeneration") == "O"))
       && _number->text().isEmpty() )
      {
        q.exec("SELECT fetchCRMAccountNumber() AS number;");
        if (q.first())
        {
          _number->setText(q.value("number"));
          _NumberGen = q.value("number").toInt();
        }
      }
      
      connect(_charass, SIGNAL(valid(bool)), _editCharacteristic, SLOT(setEnabled(bool)));
      connect(_charass, SIGNAL(valid(bool)), _deleteCharacteristic, SLOT(setEnabled(bool)));
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;   
      
      connect(_charass, SIGNAL(valid(bool)), _editCharacteristic, SLOT(setEnabled(bool)));
      connect(_charass, SIGNAL(valid(bool)), _deleteCharacteristic, SLOT(setEnabled(bool)));
      _name->setFocus();
    }
    else if (param.toString() == "view")
    {
      _mode = cView;

      _owner->setEnabled(FALSE);
      _number->setEnabled(FALSE);
      _name->setEnabled(FALSE);
      _active->setEnabled(FALSE);
      _typeBox->setEnabled(FALSE);
      _primary->setEnabled(FALSE);
      _secondary->setEnabled(FALSE);
      _notes->setEnabled(FALSE);
      _comments->setEnabled(FALSE);
      _documents->setReadOnly(TRUE);
      _parentCrmacct->setEnabled(FALSE);
      _newCharacteristic->setEnabled(FALSE);
      _editCharacteristic->setEnabled(FALSE);
      _save->hide();
      _newReg->setEnabled(false);
      disconnect(_reg, SIGNAL(valid(bool)), _editReg, SLOT(setEnabled(bool)));
      disconnect(_reg, SIGNAL(valid(bool)), _deleteReg, SLOT(setEnabled(bool)));
      disconnect(_reg, SIGNAL(valid(bool)), _editReg, SLOT(animateClick()));

      _close->setFocus();

      ParameterList params;
      params.append("mode", "view");
      _contacts->set(params);
    }
  }
  
  param = pParams.value("crmacct_id", &valid);
  if (valid)
  {
    _crmacctId = param.toInt();
    _todoList->parameterWidget()->setDefault(tr("CRM Account"), _crmacctId, true);
    _contacts->setCrmacctid(_crmacctId);
    _oplist->parameterWidget()->setDefault(tr("CRM Account"), _crmacctId, true);
    sPopulate();
  }

  int scripted;
  scripted = pParams.inList("scripted");
  if(scripted)
  {
	  _tab->setCurrentIndex(4);
  }
  
  if(_metrics->value("CRMAccountNumberGeneration") == "A")
    _number->setEnabled(FALSE);

  return NoError;
}

void crmaccount::sClose()
{
  if (_mode == cNew)
  {
    if (QMessageBox::question(this, tr("Close without saving?"),
			      tr("<p>Are you sure you want to close this window "
				 "without saving the new CRM Account?"),
			      QMessageBox::Yes, QMessageBox::No) == QMessageBox::No)
      return;

    XSqlQuery rollback;
    rollback.prepare("ROLLBACK;");

    struct {
      QString spName;
      int     spArg;
    } toDelete[] = {
      {	"deleteCustomer",	_custId },
      { "deleteProspect",	_prospectId },
      { "deleteVendor",		_vendId },
      { "deleteTaxAuthority",	_taxauthId },
      /* any others? */
      { "deleteCRMAccount",	_crmacctId }
    };

    q.exec("BEGIN;");

    q.prepare("SELECT MIN(detachContact(cntct_id, :crmacct_id)) AS returnVal "
	      "FROM cntct "
	      "WHERE (cntct_crmacct_id=:crmacct_id);");
    q.bindValue(":crmacct_id", _crmacctId);
    q.exec();
    if (q.first())
    {
      int returnVal = q.value("returnVal").toInt();
      if (returnVal < 0)
      {
	rollback.exec();
	systemError(this, tr("Error detaching Contact from CRM Account (%1).")
			  .arg(returnVal), __FILE__, __LINE__);
	return;
      }
    }
    else if (q.lastError().type() != QSqlError::NoError)
    {
      rollback.exec();
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }

    for (unsigned int i = 0; i < sizeof(toDelete) / sizeof(toDelete[0]); i++)
    {
      if (toDelete[i].spArg <= 0)
	continue;

      q.prepare(QString("SELECT %1(%2) AS returnVal;")
			.arg(toDelete[i].spName)
			.arg(toDelete[i].spArg));
      q.exec();
      if (q.first())
      {
	int returnVal = q.value("returnVal").toInt();
	if (returnVal < 0)
	{
	  rollback.exec();
	  systemError(this, storedProcErrorLookup(toDelete[i].spName, returnVal),
		      __FILE__, __LINE__);
	  return;
	}
      }
      else if (q.lastError().type() != QSqlError::NoError)
      {
	rollback.exec();
	systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
	return;
      }
    } // for each thing to delete

    q.exec("COMMIT;");
    omfgThis->sCrmAccountsUpdated(_crmacctId);
  } // if cNew

  close();

}

void crmaccount::sSave()
{

  int returnVal = 0;
  XSqlQuery rollback;
  rollback.prepare("ROLLBACK;");

  QString incomplete = tr("<p>The %1 relationship is selected but no "
			  "appropriate data have been created. Either click the"
			  " %2 button to enter the data or unselect %1.");
  struct {
    bool	condition;
    QString	msg;
    QWidget*	widget;
  } error[] = {
    { _number->text().trimmed().length() == 0,
      tr("<p>You must enter a number for this CRM Account before saving it."),
      _number
    },
    { _name->text().trimmed().length() == 0,
      tr("<p>You must enter a name for this CRM Account before saving it."),
      _name
    },
    { _customer->isChecked() && _custId <= 0,
      incomplete.arg(_customer->text()).arg(_customerButton->text()),
      _customerButton
    },
    { _prospect->isChecked() && _prospectId <= 0,
      incomplete.arg(_prospect->text()).arg(_prospectButton->text()),
      _prospectButton
    },
    { _vendor->isChecked() && _vendId <= 0,
      incomplete.arg(_vendor->text()).arg(_vendorButton->text()),
      _vendorButton
    },
    { _taxauth->isChecked() && _taxauthId <= 0,
      incomplete.arg(_taxauth->text()).arg(_taxauthButton->text()),
      _taxauthButton
    },
    { true, "", NULL }
  }; // error[]

  int errIndex;
  for (errIndex = 0; ! error[errIndex].condition; errIndex++)
    ;
  if (! error[errIndex].msg.isEmpty())
  {
    QMessageBox::critical(this, tr("Cannot Save CRM Account"),
			  error[errIndex].msg);
    error[errIndex].widget->setFocus();
    return;
  }

  q.prepare( "SELECT crmacct_number "
	     "FROM crmacct "
	     "WHERE ((crmacct_number=:number)"
	     " AND   (crmacct_id<>:crmacct_id) );" );

  q.bindValue(":crmacct_id", _crmacctId);
  q.bindValue(":number", _number->text());
  q.exec();
  if (q.first())
  {
    QMessageBox::critical(this, tr("Cannot Save CRM Account"),
			  storedProcErrorLookup("createCrmAcct", -1));
    _number->setFocus();
    return;
  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  QString spName = "";
  int	  spArg = 0;
  int	  answer = QMessageBox::No;

  if (_prospectId > 0 && !_prospect->isChecked())
  {
    answer = QMessageBox::question(this, tr("Delete Prospect?"),
		    tr("<p>Are you sure you want to delete %1 as a Prospect?")
		      .arg(_number->text()),
		    QMessageBox::Yes, QMessageBox::No | QMessageBox::Default);
    if (QMessageBox::No == answer)
    {
      _prospect->setChecked(true);
      return;
    }
    spName = "deleteProspect";
    spArg = _prospectId;
  }

  if (_custId > 0 && !_customer->isChecked())
  {
    answer = QMessageBox::question(this, tr("Delete Customer?"),
		    tr("<p>Are you sure you want to delete %1 as a Customer?")
		      .arg(_number->text()),
		    QMessageBox::Yes, QMessageBox::No | QMessageBox::Default);
    if (QMessageBox::No == answer)
    {
      _customer->setChecked(true);
      return;
    }
    spName = "deleteCustomer";
    spArg = _custId;
  }

  if (_vendId > 0 && ! _vendor->isChecked())
  {
    answer = QMessageBox::question(this, tr("Delete Vendor?"),
		    tr("<p>Are you sure you want to delete %1 as a Vendor?")
		      .arg(_number->text()),
		    QMessageBox::Yes, QMessageBox::No | QMessageBox::Default);
    if (QMessageBox::No == answer)
    {
      _vendor->setChecked(true);
      return;
    }
  }

  if (_taxauthId > 0 && ! _taxauth->isChecked())
  {
    answer = QMessageBox::question(this, tr("Delete Tax Authority?"),
		    tr("<p>Are you sure you want to delete %1 as a Tax Authority?")
		      .arg(_number->text()),
		    QMessageBox::Yes, QMessageBox::No | QMessageBox::Default);
    if (QMessageBox::No == answer)
    {
      _taxauth->setChecked(true);
      return;
    }
  }
  
  q.exec("BEGIN;");
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  if (! spName.isEmpty())
  {
    q.exec(QString("SELECT %1(%2) AS returnVal;").arg(spName).arg(spArg));
    if (q.first())
    {
      returnVal = q.value("returnVal").toInt();
      if (returnVal < 0)
      {
        rollback.exec();
        systemError(this, storedProcErrorLookup(spName, returnVal), __FILE__, __LINE__);
        return;
      }
    } 
    else if (q.lastError().type() != QSqlError::NoError)
    {
      rollback.exec();
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
    if(spName == "deleteProspect")
      _prospectId = -1;
    else
      _custId = -1;
  }

  if (! _taxauth->isChecked() && _taxauthId > 0)
  {
    q.prepare("SELECT deleteTaxAuthority(:taxauthid) AS returnVal;");
    q.bindValue(":taxauthid", _taxauthId);
    q.exec();
    if (q.first())
    {
      returnVal = q.value("returnVal").toInt();
      if (returnVal < 0)
      {
        rollback.exec();
        systemError(this, storedProcErrorLookup("deleteTaxAuthority", returnVal), __FILE__, __LINE__);
        return;
      }
    } 
    else if (q.lastError().type() != QSqlError::NoError)
    {
      rollback.exec();
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
    _taxauthId = -1;
  }

  if (! _vendor->isChecked() && _vendId > 0)
  {
    q.prepare("SELECT deleteVendor(:vendid) AS returnVal;");
    q.bindValue(":vendid", _vendId);
    q.exec();
    if (q.first())
    {
      returnVal = q.value("returnVal").toInt();
      if (returnVal < 0)
      {
        rollback.exec();
        systemError(this, storedProcErrorLookup("deleteVendor", returnVal), __FILE__, __LINE__);
        return;
      }
    } 
    else if (q.lastError().type() != QSqlError::NoError)
    {
      rollback.exec();
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
    _vendId = -1;
  }

  if (saveNoErrorCheck() < 0)	// do the heavy lifting
  {
    rollback.exec();
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  q.exec("COMMIT;");
  _NumberGen = -1;
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  omfgThis->sCrmAccountsUpdated(_crmacctId);
  omfgThis->sCustomersUpdated(-1, TRUE);
  omfgThis->sProspectsUpdated();
  omfgThis->sVendorsUpdated();
  omfgThis->sTaxAuthsUpdated(-1);
  emit saved(_crmacctId);
  close();
}

int crmaccount::saveNoErrorCheck()
{
  q.prepare("UPDATE crmacct "
	    "SET crmacct_id=:crmacct_id,"
	    "    crmacct_number=UPPER(:crmacct_number),"
	    "    crmacct_name=:crmacct_name,"
	    "    crmacct_parent_id=:crmacct_parent_id,"
	    "    crmacct_active=:crmacct_active,"
	    "    crmacct_type=:crmacct_type,"
	    "    crmacct_cust_id=:crmacct_cust_id,"
	    "    crmacct_competitor_id=:crmacct_competitor_id,"
	    "    crmacct_partner_id=:crmacct_partner_id,"
	    "    crmacct_prospect_id=:crmacct_prospect_id,"
	    "    crmacct_taxauth_id=:crmacct_taxauth_id,"
	    "    crmacct_vend_id=:crmacct_vend_id,"
	    "    crmacct_cntct_id_1=:crmacct_cntct_id_1,"
	    "    crmacct_cntct_id_2 =:crmacct_cntct_id_2,"
            "    crmacct_owner_username =:crmacct_owner_username,"
	    "    crmacct_notes=:crmacct_notes "
	    "WHERE (crmacct_id=:crmacct_id);" );
  
  q.bindValue(":crmacct_id",		_crmacctId);
  q.bindValue(":crmacct_number",	_number->text());
  q.bindValue(":crmacct_name",		_name->text());
  q.bindValue(":crmacct_active",	QVariant(_active->isChecked()));
  q.bindValue(":crmacct_type",		_organization->isChecked() ? "O" : "I");
  q.bindValue(":crmacct_notes",		_notes->toPlainText());
  if (_custId > 0)	 q.bindValue(":crmacct_cust_id",	_custId);
  if (_competitorId > 0) q.bindValue(":crmacct_competitor_id",  _competitorId);
  if (_partnerId > 0)	 q.bindValue(":crmacct_partner_id",	_partnerId);
  if (_prospectId > 0)	 q.bindValue(":crmacct_prospect_id",    _prospectId);
  if (_taxauthId > 0)	 q.bindValue(":crmacct_taxauth_id",	_taxauthId);
  if (_vendor->isChecked())   q.bindValue(":crmacct_vend_id",	_vendId);
  if (_primary->id() > 0)   q.bindValue(":crmacct_cntct_id_1",	_primary->id());
  if (_secondary->id() > 0) q.bindValue(":crmacct_cntct_id_2",	_secondary->id());
  if (_parentCrmacct->id() > 0)
  q.bindValue(":crmacct_parent_id", _parentCrmacct->id());
  q.bindValue(":crmacct_owner_username", _owner->username());

  if (! q.exec())
  {
    return -1;
  }

  return 0;
}

void crmaccount::sNewCharacteristic()
{
  ParameterList params;
  params.append("mode", "new");
  params.append("crmacct_id", _crmacctId);

  characteristicAssignment newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sGetCharacteristics();
}

void crmaccount::sEditCharacteristic()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("charass_id", _charass->id());

  characteristicAssignment newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sGetCharacteristics();
}

void crmaccount::sDeleteCharacteristic()
{
  q.prepare( "DELETE FROM charass "
             "WHERE (charass_id=:charass_id);" );
  q.bindValue(":charass_id", _charass->id());
  q.exec();

  sGetCharacteristics();
}

void crmaccount::sGetCharacteristics()
{
  q.prepare( "SELECT charass_id, char_name, "
             " CASE WHEN char_type < 2 THEN "
             "   charass_value "
             " ELSE "
             "   formatDate(charass_value::date) "
             "END AS charass_value "
             "FROM charass, char "
             "WHERE ( (charass_target_type='CRMACCT')"
             " AND (charass_char_id=char_id)"
             " AND (charass_target_id=:crmacct_id) ) "
             "ORDER BY char_order, char_name;" );
  q.bindValue(":crmacct_id", _crmacctId);
  q.exec();
  _charass->populate(q);
}

void crmaccount::sPopulate()
{
  q.prepare( "SELECT * "
             "FROM crmacct "
             "WHERE (crmacct_id=:crmacct_id);" );
  q.bindValue(":crmacct_id", _crmacctId);
  q.exec();
  if (q.first())
  {
    _number->setText(q.value("crmacct_number").toString());
    _name->setText(q.value("crmacct_name").toString());
    _active->setChecked(q.value("crmacct_active").toBool());

    QString acctType = q.value("crmacct_type").toString();
    if (acctType == "O")
      _organization->setChecked(true);
    else if (acctType == "I")
      _individual->setChecked(true);
    else
      qWarning("crmaccount::sPopulate() - acctType '%s' incorrect", acctType.toLatin1().data());

    _custId	= q.value("crmacct_cust_id").toInt();
    _competitorId = q.value("crmacct_competitor_id").toInt();
    _partnerId	= q.value("crmacct_partner_id").toInt();
    _prospectId	= q.value("crmacct_prospect_id").toInt();
    _vendId	= q.value("crmacct_vend_id").toInt();
    _taxauthId	= q.value("crmacct_taxauth_id").toInt();
    _primary->setId(q.value("crmacct_cntct_id_1").toInt());
    _secondary->setId(q.value("crmacct_cntct_id_2").toInt());
    _notes->setText(q.value("crmacct_notes").toString());
    _parentCrmacct->setId(q.value("crmacct_parent_id").toInt());
    _comments->setId(_crmacctId);
    _documents->setId(_crmacctId);
    _primary->setSearchAcct(_crmacctId);
    _secondary->setSearchAcct(_crmacctId);
    _owner->setUsername(q.value("crmacct_owner_username").toString());

    _customer->setDisabled(_customer->isChecked());
    _prospect->setChecked(_prospectId > 0);
    _prospect->setDisabled(_customer->isChecked());
    _taxauth->setChecked(_taxauthId > 0);
    
    if (_custId > 0)
    {
      _customer->setChecked(true);
      if (_privileges->check("MaintainCustomerMasters") || 
          _privileges->check("ViewCustomerMasters") )
        _customerButton->setEnabled(true);
    }
    if (_vendId > 0)
    {
      _vendor->setChecked(true);
      _vendorButton->setText("Vendor");
      disconnect(_vendorButton, SIGNAL(clicked()), this, SLOT(sEditVendor()));
      
      QMenu * vendorMenu = new QMenu;
      QAction *menuItem;
      menuItem=vendorMenu->addAction(tr("Edit..."), this, SLOT(sEditVendor()));
      menuItem->setEnabled(_privileges->check("MaintainVendors") &&
                           (_mode == cEdit));
      menuItem=vendorMenu->addAction(tr("View..."), this, SLOT(sViewVendor()));
      menuItem->setEnabled(_privileges->check("ViewVendors") ||
                           _privileges->check("MaintainVendors"));
      menuItem=vendorMenu->addAction(tr("Workbench..."), this, SLOT(sVendorInfo()));
      menuItem->setEnabled(_privileges->check("MaintainVendors"));
      _vendorButton->setMenu(vendorMenu);  
      
    }
    _partner->setChecked(_partnerId > 0);
    _competitor->setChecked(_competitorId > 0);
  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  sGetCharacteristics();
  sPopulateRegistrations();
  _contacts->sFillList();
  _todoList->sFillList();
  _oplist->sFillList();
}

void crmaccount::sCompetitor()
{
  _competitorId = (_competitor->isChecked() ? 1 : 0);
}

void crmaccount::sCustomer()
{
  ParameterList params;
  
  if (saveNoErrorCheck() < 0)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  if (! _customer->isChecked())	// don't know how we got here
    return;

  QString confirm;

  if (_prospectId > 0)
  {
	if (QMessageBox::question(this, tr("Convert"),
		tr("<p>Are you sure you want to convert this "
		       "Prospect to a Customer?"),
		QMessageBox::Yes,
		QMessageBox::No | QMessageBox::Default) == QMessageBox::No)
	return;
	
	params.append("prospect_id",	  _prospectId);
	params.append("mode","new");
	
  }
  else if (_custId <= 0)
  {
    params.append("crmacct_id",	  _crmacctId);
    params.append("mode","new");
  } 	
  else
  {
    params.append("cust_id",_custId);
    params.append("mode",		  (!_privileges->check("MaintainCustomerMasters")) ? "view" : "edit");
  }

  customer *newdlg = new customer(this);
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
  
}

void crmaccount::sPartner()
{
  _partnerId = (_partner->isChecked() ? 1 : 0);
}

void crmaccount::sProspect()
{
  if (saveNoErrorCheck() < 0)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  if (! _prospect->isChecked())	// don't know how we got here
    return;

  QString spName = "";
  int	  spArg = 0;

  if (_custId > 0)
  {
    spName = "convertCustomerToProspect";
    spArg = _custId;
    if (QMessageBox::question(this, tr("Convert"),
		    tr("<p>Are you sure you want to convert this Customer to "
		       "a Prospect and delete its Customer information?"),
		    QMessageBox::Yes,
		    QMessageBox::No | QMessageBox::Default) == QMessageBox::No)
      return;
  }
  else if (_prospectId <= 0)
  {
    spName = "createProspect";
    spArg = _crmacctId;
  }

  if (! spName.isEmpty())
  {
    q.exec(QString("SELECT %1(%2) AS returnVal;").arg(spName).arg(spArg));
    if (q.first())
    {
      int returnVal = q.value("returnVal").toInt();
      if (returnVal < 0)
      {
	systemError(this, storedProcErrorLookup(spName, returnVal), __FILE__, __LINE__);
	return;
      }
      _custId = -1;
      _prospectId = returnVal;
    } 
    else if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }

    omfgThis->sProspectsUpdated();
    omfgThis->sCustomersUpdated(_custId, true);
  }

  ParameterList params;
  params.append("crmacct_id",	  _crmacctId);
  params.append("crmacct_number", _number->text());
  params.append("crmacct_name",	  _name->text());
  params.append("prospect_id",	  _prospectId);
  params.append("mode",		  (_mode == cView) ? "view" : "edit");

  prospect *newdlg = new prospect(this);
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void crmaccount::sTaxAuth()
{
  if (saveNoErrorCheck() < 0)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  ParameterList params;
  params.append("crmacct_id", _crmacctId);
  params.append("crmacct_number", _number->text());
  params.append("crmacct_name", _name->text());
  if (_taxauthId <= 0 && _privileges->check("MaintainTaxAuthorities") &&
      (_mode == cEdit || _mode == cNew))
    params.append("mode", "new");
  else if (_taxauthId <= 0)
    systemError(this, tr("No existing Tax Authorities to View"));
  else if (_taxauthId > 0 && _privileges->check("MaintainTaxAuthorities") &&
	    (_mode == cEdit || _mode == cNew))
  {
    params.append("taxauth_id", _taxauthId);
    params.append("mode", "edit");
  }
  else if (_taxauthId > 0 && _mode == cView)
  {
    params.append("taxauth_id", _taxauthId);
    params.append("mode", "view");
  }

  taxAuthority newdlg(this);
  newdlg.set(params);
  newdlg.exec();
}

void crmaccount::sEditVendor()
{
  if (saveNoErrorCheck() < 0)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  ParameterList params;
  params.append("crmacct_id", _crmacctId);
  params.append("crmacct_number", _number->text());
  params.append("crmacct_name", _name->text());
  if (_vendId <= 0)
    params.append("mode", "new");
  else
  {
    params.append("vend_id", _vendId);
    params.append("mode", "edit");
  }

  vendor *newdlg = new vendor(this);
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void crmaccount::sViewVendor()
{
  if (saveNoErrorCheck() < 0)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
  
  ParameterList params;
  params.append("vend_id", _vendId);
  params.append("mode", "view");

  vendor *newdlg = new vendor(this);
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void crmaccount::sUpdateRelationships()
{
  q.prepare( "SELECT * "
             "FROM crmacct "
             "WHERE (crmacct_id=:crmacct_id);" );
  q.bindValue(":crmacct_id", _crmacctId);
  q.exec();
  if (q.first())
  {
    _custId	= q.value("crmacct_cust_id").toInt();
    _competitorId = q.value("crmacct_competitor_id").toInt();
    _partnerId	= q.value("crmacct_partner_id").toInt();
    _prospectId	= q.value("crmacct_prospect_id").toInt();
    _taxauthId	= q.value("crmacct_taxauth_id").toInt();
    _vendId	= q.value("crmacct_vend_id").toInt();

    _customer->setChecked(_custId > 0);
    _customer->setDisabled(_customer->isChecked());
    _prospect->setChecked(_prospectId > 0);
    _prospect->setDisabled(_customer->isChecked());
    _taxauth->setChecked(_taxauthId > 0);
    _vendor->setChecked(_vendId > 0);
    _partner->setChecked(_partnerId > 0);
    _competitor->setChecked(_competitorId > 0);
    _owner->setUsername(q.value("crmacct_owner_username").toString());
  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
  
  _contacts->sFillList();
}

void crmaccount::doDialog(QWidget *parent, const ParameterList & pParams)
{
  //XDialog newdlg(parent);
  //QVBoxLayout * vbox = new QVBoxLayout(&newdlg);
  crmaccount * ci = new crmaccount(parent);
  //vbox->addWidget(ci);
  //newdlg.setWindowTitle(ci->windowTitle());
  //ParameterList params;
  //params = pParams;
  //params.append("modal");
  ci->set(pParams);
  //newdlg.exec();
  omfgThis->handleNewWindow(ci);
}

void crmaccount::sVendorInfo()
{
  ParameterList params;
  params.append("vend_id", _vendId);

  vendorWorkBench *newdlg = new vendorWorkBench(this, "vendorWorkBench", Qt::Window);
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void crmaccount::sCustomerToggled()
{
  if (_customer->isChecked())
    _prospect->setChecked(FALSE);
}

void crmaccount::sProspectToggled()
{
  if (_prospect->isChecked())
    _customer->setChecked(FALSE);
}

void crmaccount::sPopulateRegistrations()
{
  if (_metrics->boolean("LotSerialControl"))
  {
     q.prepare( "SELECT lsreg_id,ls_number,item_number, item_descrip1, "
             "lsreg_qty,'qty' AS lsreg_qty_xtnumericrole,lsreg_solddate,"
	     "lsreg_regdate,lsreg_expiredate, "
	     "CASE WHEN lsreg_expiredate <= current_date THEN "
	     "  'expired' "
	     "END AS lsreg_expiredate_qtforegroundrole "
             "FROM lsreg,ls,item  "
             "WHERE ((lsreg_ls_id=ls_id) "
	     "AND (ls_item_id=item_id) "
             "AND (lsreg_crmacct_id=:crmacct_id));");
    q.bindValue(":crmacct_id", _crmacctId);
    q.exec();
    _reg->clear();
    _reg->populate(q);
    if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }
}

void crmaccount::sNewReg()
{
  ParameterList params;
  params.append("mode", "new");
  params.append("crmacct_id", _crmacctId);

  lotSerialRegistration newdlg(this, "", TRUE);
  newdlg.set(params);
  
  newdlg.exec();
  sPopulateRegistrations();
}

void crmaccount::sEditReg()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("lsreg_id", _reg->id());

  lotSerialRegistration newdlg(this, "", TRUE);
  newdlg.set(params);
  
  newdlg.exec();
  sPopulateRegistrations();
}

void crmaccount::sDeleteReg()
{
  q.prepare( "DELETE FROM lsreg "
             "WHERE (lsreg_id=:lsreg_id);"
             "DELETE FROM charass "
             "WHERE ((charass_target_type='LSR') "
             "AND (charass_target_id=:lsreg_id))" );
  q.bindValue(":lsreg_id", _reg->id());
  q.exec();
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  sPopulateRegistrations();
}

void crmaccount::sCheckNumber()
{
  if (DEBUG) qDebug("crmaccount::sCheckNumber()");
  if (_number->text().length())
  {
    _number->setText(_number->text().trimmed().toUpper());

    XSqlQuery newq;
    newq.prepare("SELECT crmacct_id "
                 "FROM crmacct "
                 "WHERE ((UPPER(crmacct_number)=UPPER(:crmacct_number))"
                 "   AND (crmacct_id!=:crmacct_id));" );
    newq.bindValue(":crmacct_number", _number->text());
    newq.bindValue(":crmacct_id",     _crmacctId);
    newq.exec();
    if (newq.first())
    {
      XSqlQuery query;
      if(cNew == _mode && -1 != _NumberGen && _number->text().toInt() != _NumberGen)
      {
        query.prepare( "SELECT releaseCRMAccountNumber(:Number);" );
        query.bindValue(":Number", _NumberGen);
        query.exec();
        _NumberGen = -1;
      }
      // Delete temporary
      query.prepare( "DELETE FROM crmacct WHERE (crmacct_id=:crmacct_id);" );
      query.bindValue(":crmacct_id", _crmacctId);
      query.exec();
      if (query.lastError().type() != QSqlError::NoError)
      {
	systemError(this, query.lastError().databaseText(), __FILE__, __LINE__);
	return;
      }
      
      _crmacctId = newq.value("crmacct_id").toInt();
      _todoList->parameterWidget()->setDefault(tr("CRM Account"), _crmacctId, true);
      _contacts->setCrmacctid(_crmacctId);
      _mode = cEdit;
      sPopulate();

      connect(_charass, SIGNAL(valid(bool)), _editCharacteristic, SLOT(setEnabled(bool)));
      connect(_charass, SIGNAL(valid(bool)), _deleteCharacteristic, SLOT(setEnabled(bool)));
      _name->setFocus();
      _number->setEnabled(FALSE);
    }
    else if (newq.lastError().type() != QSqlError::NoError)
    {
      systemError(this, newq.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }
}

void crmaccount::closeEvent(QCloseEvent *pEvent)
{
  if(cNew == _mode && -1 != _NumberGen)
  {
    XSqlQuery query;
    query.prepare( "SELECT releaseCRMAccountNumber(:Number);" );
    query.bindValue(":Number", _NumberGen);
    query.exec();
    _NumberGen = -1;
  }
  XWidget::closeEvent(pEvent);
}

void crmaccount::sHandleButtons() 	 
{ 	 
  if (_primaryButton->isChecked()) 	 
    _widgetStack->setCurrentIndex(_widgetStack->indexOf(_primaryPage)); 	 
  else if (_secondaryButton->isChecked()) 	 
    _widgetStack->setCurrentIndex(_widgetStack->indexOf(_secondaryPage)); 	 
  else 	 
    _widgetStack->setCurrentIndex(_widgetStack->indexOf(_allPage)); 	 
}

void crmaccount::sHandleCntctDetach(int cntctId)
{
  if (_primary->id() == cntctId)
    _primary->clear();
  if (_secondary->id() == cntctId)
    _secondary->clear();
}
