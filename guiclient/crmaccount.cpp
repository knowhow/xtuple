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
#include "employee.h"
#include "errorReporter.h"
#include "guiErrorCheck.h"
#include "incident.h"
#include "lotSerialRegistration.h"
#include "mqlutil.h"
#include "parameterwidget.h"
#include "prospect.h"
#include "salesRep.h"
#include "storedProcErrorLookup.h"
#include "taxAuthority.h"
#include "user.h"
#include "vendor.h"
#include "vendorWorkBench.h"

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
  _todoList->parameterWidget()->setDefault(tr("User"), QVariant(), true);
  _todoList->parameterWidget()->append("", "hasContext", ParameterWidget::Exists, true);
  _todoList->setParameterWidgetVisible(false);
  _todoList->setQueryOnStartEnabled(false);

  _contacts = new contacts(this, "contacts", Qt::Widget);
  _allPage->layout()->addWidget(_contacts);
  _contacts->setCloseVisible(false);
  _contacts->list()->hideColumn("crmacct_number");
  _contacts->list()->hideColumn("crmacct_name");
  _contacts->parameterWidget()->append("", "hasContext", ParameterWidget::Exists, true);
  _contacts->setParameterWidgetVisible(false);
  _contacts->setQueryOnStartEnabled(false);

  _owner->setUsername(omfgThis->username());
  _owner->setType(UsernameLineEdit::UsersActive);

  connect(_close,               SIGNAL(clicked()), this, SLOT(sClose()));
  connect(_competitor,          SIGNAL(clicked()), this, SLOT(sCompetitor()));
  connect(_customerButton,      SIGNAL(clicked()), this, SLOT(sCustomer()));
  connect(_deleteCharacteristic,SIGNAL(clicked()), this, SLOT(sDeleteCharacteristic()));
  connect(_deleteReg,           SIGNAL(clicked()), this, SLOT(sDeleteReg()));
  connect(_editCharacteristic,  SIGNAL(clicked()), this, SLOT(sEditCharacteristic()));
  connect(_editReg,             SIGNAL(clicked()), this, SLOT(sEditReg()));
  connect(_employeeButton,      SIGNAL(clicked()), this, SLOT(sEmployee()));
  connect(_newCharacteristic,   SIGNAL(clicked()), this, SLOT(sNewCharacteristic()));
  connect(_newReg,              SIGNAL(clicked()), this, SLOT(sNewReg()));
  connect(_partner,             SIGNAL(clicked()), this, SLOT(sPartner()));
  connect(_prospectButton,      SIGNAL(clicked()), this, SLOT(sProspect()));
  connect(_salesrepButton,      SIGNAL(clicked()), this, SLOT(sSalesRep()));
  connect(_save,                SIGNAL(clicked()), this, SLOT(sSave()));
  connect(_taxauthButton,       SIGNAL(clicked()), this, SLOT(sTaxAuth()));
  connect(_userButton,          SIGNAL(clicked()), this, SLOT(sUser()));
  connect(_vendorButton,        SIGNAL(clicked()), this, SLOT(sEditVendor()));

  connect(omfgThis, SIGNAL(customersUpdated(int, bool)), this, SLOT(sUpdateRelationships()));
  connect(omfgThis, SIGNAL(employeeUpdated(int)), this, SLOT(sUpdateRelationships()));
  connect(omfgThis, SIGNAL(prospectsUpdated()),   this, SLOT(sUpdateRelationships()));
  connect(omfgThis, SIGNAL(salesRepUpdated(int)), this, SLOT(sUpdateRelationships()));
  connect(omfgThis, SIGNAL(taxAuthsUpdated(int)), this, SLOT(sUpdateRelationships()));
  connect(omfgThis, SIGNAL(vendorsUpdated()),     this, SLOT(sUpdateRelationships()));
  connect(omfgThis, SIGNAL(userUpdated(QString)), this, SLOT(sUpdateRelationships()));
  connect(_customer, SIGNAL(toggled(bool)), this, SLOT(sCustomerToggled()));
  connect(_prospect, SIGNAL(toggled(bool)), this, SLOT(sProspectToggled()));
  connect(_number, SIGNAL(lostFocus()), this, SLOT(sCheckNumber()));
  connect(_primaryButton, SIGNAL(toggled(bool)), this, SLOT(sHandleButtons()));         
  connect(_secondaryButton, SIGNAL(toggled(bool)), this, SLOT(sHandleButtons()));       
  connect(_allButton, SIGNAL(toggled(bool)), this, SLOT(sHandleButtons()));
  connect(_contacts, SIGNAL(cntctDetached(int)), this, SLOT(sHandleCntctDetach(int)));

  _charass->addColumn(tr("Characteristic"), _itemColumn, Qt::AlignLeft, true, "char_name");
  _charass->addColumn(tr("Value"),          -1,          Qt::AlignLeft, true, "charass_value");

  _reg->addColumn(tr("Lot/Serial")  ,  _itemColumn,  Qt::AlignLeft, true, "ls_number" );
  _reg->addColumn(tr("Item")        ,  _itemColumn,  Qt::AlignLeft, true, "item_number" );
  _reg->addColumn(tr("Description") ,  -1,           Qt::AlignLeft, true, "item_descrip1" );
  _reg->addColumn(tr("Qty"       )  ,  _qtyColumn,   Qt::AlignLeft, true, "lsreg_qty" );
  _reg->addColumn(tr("Sold"        ),  _dateColumn,  Qt::AlignLeft, true, "lsreg_solddate" );
  _reg->addColumn(tr("Registered"  ),  _dateColumn,  Qt::AlignLeft, true, "lsreg_regdate" );
  _reg->addColumn(tr("Expires"   )  ,  _dateColumn,  Qt::AlignLeft, true, "lsreg_expiredate" );

  if (_preferences->boolean("XCheckBox/forgetful"))
    _active->setChecked(true);

  _NumberGen    = -1;
  _mode         = cView;
  _crmacctId    = -1;   // TODO: setId(-1)?
  _competitorId = -1;
  _custId       = -1;
  _empId        = -1;
  _partnerId    = -1;
  _prospectId   = -1;
  _salesrepId   = -1;
  _taxauthId    = -1;
  _username     = "";
  _vendId       = -1;
  _comments->setId(-1);
  _documents->setId(-1);
  _modal        = parent && (parent->inherits("customer") ||
                             parent->inherits("employee") ||
                             parent->inherits("prospect") ||
                             parent->inherits("salesRep") ||
                             parent->inherits("taxAuthority") ||
                             parent->inherits("user") ||
                             parent->inherits("vendor"));
  if (_modal)
    setWindowModality(Qt::WindowModal);
  _canCreateUsers = false;

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

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      _mode = cNew;
      _number->setFocus();

      XSqlQuery insq;
      insq.prepare("INSERT INTO crmacct(crmacct_number, crmacct_name,"
                   "                    crmacct_active, crmacct_type)"
                   "  SELECT 'TEMPORARY' || (last_value + 1), '', true, 'O'"
                   "    FROM crmacct_crmacct_id_seq"
                   " RETURNING crmacct_id AS result;");
      insq.bindValue(":crmacct_id", _crmacctId);
      insq.exec();
      if (insq.first())
        setId(insq.value("result").toInt());
      else if (ErrorReporter::error(QtCriticalMsg, this,
                             tr("Error creating Initial Account"), insq,
                             __FILE__, __LINE__))
        return UndefinedError;

      _number->clear();

      if(((_metrics->value("CRMAccountNumberGeneration") == "A") ||
          (_metrics->value("CRMAccountNumberGeneration") == "O"))
       && _number->text().isEmpty() )
      {
        XSqlQuery numq;
        numq.exec("SELECT fetchCRMAccountNumber() AS number;");
        if (numq.first())
        {
          _number->setText(numq.value("number"));
          _NumberGen = numq.value("number").toInt();
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

      _save->hide();
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
    setId(param.toInt());

  /* TODO: "scripted" was added for issue 6804 but
           a) no xtuple scripts seem to use it and
           b) it isn't sufficiently precise to be useful anyway
           why don't scripts just call setCurrentIndex if they care?
   */
  if(pParams.inList("scripted"))
    _tab->setCurrentIndex(_tab->indexOf(_todoListTab));

  bool canEdit = (_mode == cNew || _mode == cEdit);

  _active->setEnabled(canEdit);
  _comments->setEnabled(canEdit);
  _documents->setReadOnly(! canEdit);
  _name->setEnabled(canEdit);
  _newCharacteristic->setEnabled(canEdit);
  _newReg->setEnabled(canEdit);
  _notes->setEnabled(canEdit);
  _number->setEnabled(canEdit && _metrics->value("CRMAccountNumberGeneration") != "A");
  _owner->setEnabled(canEdit  && _privileges->check("EditOwner"));
  _parentCrmacct->setEnabled(canEdit);
  _primary->setEnabled(canEdit);
  _secondary->setEnabled(canEdit);
  _typeBox->setEnabled(canEdit);

  sHandleChildButtons();

  return NoError;
}

void crmaccount::sClose()
{
  if (_mode == cNew)
  {
    if (QMessageBox::question(this, tr("Close without saving?"),
                              tr("<p>Are you sure you want to close this window "
                                 "without saving the new CRM Account?"),
                              QMessageBox::Yes,
                              QMessageBox::No | QMessageBox::Default) == QMessageBox::No)
      return;

    XSqlQuery rollback;
    rollback.prepare("ROLLBACK;");

    XSqlQuery begin("BEGIN;");

    XSqlQuery detachq;
    detachq.prepare("SELECT MIN(detachContact(cntct_id, :crmacct_id)) AS returnVal "
              "FROM cntct "
              "WHERE (cntct_crmacct_id=:crmacct_id);");
    detachq.bindValue(":crmacct_id", _crmacctId);
    detachq.exec();
    if (detachq.first())
    {
      int returnVal = detachq.value("returnVal").toInt();
      if (returnVal < 0)
      {
        rollback.exec();
        ErrorReporter::error(QtCriticalMsg, this,
                             tr("Error detaching Contact from CRM Account."),
                             storedProcErrorLookup("detachContact", returnVal),
                             __FILE__, __LINE__);
        return;
      }
    }
    else if (detachq.lastError().type() != QSqlError::NoError)
    {
      rollback.exec();
      ErrorReporter::error(QtCriticalMsg, this,
                           tr("Error detaching Contact from CRM Account"),
                           detachq, __FILE__, __LINE__);
      return;
    }

    // TODO: handle _username
    // can't delete in alpha order because of dependencies
    struct {
      QString table; QString keycol; int keyval;
    } toDelete[] = {
      { "emp",       "emp_id",       _empId },
      { "salesrep",  "salesrep_id",  _salesrepId },
      { "custinfo",  "cust_id",      _custId },
      { "prospect",  "prospect_id",  _prospectId },
      { "vendinfo",  "vend_id",      _vendId },
      { "taxauth",   "taxauth_id",   _taxauthId }
    };

    XSqlQuery deleteq;
    for (unsigned int i = 0; i < sizeof(toDelete) / sizeof(toDelete[0]); i++)
    {
      if (toDelete[i].keyval <= 0)
        continue;

      deleteq.prepare(QString("DELETE FROM %1 WHERE %2=%3")
                        .arg(toDelete[i].table, toDelete[i].keycol)
                        .arg(toDelete[i].keyval));
      deleteq.exec();
      if (deleteq.lastError().type() != QSqlError::NoError)
      {
        rollback.exec();
        ErrorReporter::error(QtCriticalMsg, this,
                             tr("Error deleting a child record"), deleteq,
                             __FILE__, __LINE__);
        return;
      }
    } // for each thing to delete

    deleteq.prepare("DELETE FROM crmacct WHERE crmacct_id=:crmacct_id;");
    deleteq.bindValue(":crmacct_id", _crmacctId);
    deleteq.exec();
    if (deleteq.lastError().type() != QSqlError::NoError)
    {
      rollback.exec();
      ErrorReporter::error(QtCriticalMsg, this,
                           tr("Error deleting the initial CRM Account record"),
                           deleteq, __FILE__, __LINE__);
      return;
    }

    XSqlQuery commit("COMMIT;");
    omfgThis->sCrmAccountsUpdated(_crmacctId);
  } // if cNew

  close();
}

void crmaccount::sSave()
{
  bool numberIsUsed = false;
  XSqlQuery dupq;
  dupq.prepare( "SELECT crmacct_number "
             "FROM crmacct "
             "WHERE ((crmacct_number=:number)"
             " AND   (crmacct_id<>:crmacct_id) );" );

  dupq.bindValue(":crmacct_id", _crmacctId);
  dupq.bindValue(":number", _number->text());
  dupq.exec();
  if (dupq.first())
    numberIsUsed = true;
  else if (ErrorReporter::error(QtCriticalMsg, this,
                          tr("Error looking for duplicate CRM Account Number"),
                          dupq, __FILE__, __LINE__))
    return;

  QString incomplete = tr("The %1 relationship is selected but no appropriate "
                          "data have been created. Either click the %2 button "
                          " to enter the data or unselect the check box.");
  QList<GuiErrorCheck> errors;
  errors<< GuiErrorCheck(numberIsUsed, _number,
                         tr("This CRM Account Number is already in use by an "
                            "existing CRM Account. Please choose a different "
                            "number and save again."))
        << GuiErrorCheck(_number->text().isEmpty(), _number,
                         tr("You must enter a number for this CRM Account "
                            "before saving it."))
        << GuiErrorCheck(_name->text().isEmpty(), _name,
                         tr("You must enter a name for this CRM Account before "
                            "saving it."))
        << GuiErrorCheck(_parentCrmacct->id() == _crmacctId, _parentCrmacct,
                         tr("This CRM Account cannot be a parent "
                            "to itself."))
        << GuiErrorCheck(_customer->isChecked() && _prospectId > 0, _customerButton,
                         tr("This CRM Account is a Prospect but it is marked "
                            "as a Customer. Either mark it as a Prospect or "
                            "click %1 to convert it before saving.")
                         .arg(_customerButton->text()))
        << GuiErrorCheck(_customer->isChecked() && _custId <= 0, _customerButton,
                         incomplete.arg(tr("Customer"),
                                        _customerButton->text()))
        << GuiErrorCheck(_employee->isChecked() && _empId <= 0, _employeeButton,
                         incomplete.arg(tr("Employee"),
                                        _employeeButton->text()))
        << GuiErrorCheck(_prospect->isChecked() && _custId > 0, _prospectButton,
                         tr("This CRM Account is a Customer but it is marked "
                            "as a Prospect. Either mark it as a Customer or "
                            "click %1 to convert it before saving.")
                         .arg(_prospectButton->text()))
        << GuiErrorCheck(_prospect->isChecked() && _prospectId <= 0,
                         _prospectButton,
                         incomplete.arg(tr("Prospect"),
                                        _prospectButton->text()))
        << GuiErrorCheck(_salesrep->isChecked() && _salesrepId <= 0,
                         _salesrepButton,
                         incomplete.arg(tr("Sales Rep"),
                                        _salesrepButton->text()))
        << GuiErrorCheck(_taxauth->isChecked() && _taxauthId <= 0,
                         _taxauthButton,
                         incomplete.arg(tr("Tax Authority"),
                                        _taxauthButton->text()))
        << GuiErrorCheck(_user->isChecked() && _username.isEmpty(), _userButton,
                         incomplete.arg(tr("User"), _userButton->text()))
        << GuiErrorCheck(_vendor->isChecked() && _vendId <= 0, _vendorButton,
                         incomplete.arg(tr("Vendor"), _vendorButton->text()))
  ;
  if (GuiErrorCheck::reportErrors(this, tr("Cannot Save CRM Account"), errors))
    return;

  // TODO: handle _username
  // this must be ordered to avoid foreign key dependency errors
  struct {
      QString type; QString table; QString keycol; QCheckBox *box; int *idvar;
  } toDelete[] = {
    { tr("Employee"),  "emp",      "emp_id",       _employee,  &_empId      },
    { tr("Sales Rep"), "salesrep", "salesrep_id",  _salesrep,  &_salesrepId },
    { tr("Customer"),  "custinfo", "cust_id",      _customer,  &_custId     },
    { tr("Prospect"),  "prospect", "prospect_id",  _prospect,  &_prospectId },
    { tr("Vendor"),    "vendinfo", "vend_id",      _vendor,    &_vendId     },
    { tr("Tax Auth"),  "taxauth",  "taxauth_id",   _taxauth,   &_taxauthId  }
  };

  for (unsigned int i = 0; i < sizeof(toDelete) / sizeof(toDelete[0]); i++)
  {
    if (*(toDelete[i].idvar) > 0 &&
        ! toDelete[i].box->isChecked() &&
        QMessageBox::question(this, tr("Delete %1?").arg(toDelete[i].type),
                              tr("<p>Are you sure you want to delete %1 as a %2?")
                                .arg(_number->text(), toDelete[i].type),
                              QMessageBox::Yes,
                              QMessageBox::No | QMessageBox::Default) == QMessageBox::No)
    {
      toDelete[i].box->setChecked(true);
      return;
    }
  }

  XSqlQuery rollback;
  rollback.prepare("ROLLBACK;");

  XSqlQuery begin("BEGIN;");
  if (ErrorReporter::error(QtCriticalMsg, this, tr("Database Error"),
                           begin, __FILE__, __LINE__))
    return;

  XSqlQuery deleteq;
  for (unsigned int i = 0; i < sizeof(toDelete) / sizeof(toDelete[0]); i++)
  {
    if (toDelete[i].box->isChecked())
      continue;

    deleteq.prepare(QString("DELETE FROM %1 WHERE (%2=%3);")
                      .arg(toDelete[i].table, toDelete[i].keycol)
                      .arg(*(toDelete[i].idvar)));
    deleteq.exec();
    if (deleteq.lastError().type() != QSqlError::NoError)
    {
      rollback.exec();
      ErrorReporter::error(QtCriticalMsg, this,
                           tr("Error deleting %1").arg(toDelete[i].type),
                           deleteq, __FILE__, __LINE__);
      return;
    }

    *(toDelete[i].idvar) = -1;
  } // for each thing to delete

  QSqlError err = saveNoErrorCheck();
  if (err.type() != QSqlError::NoError)
  {
    rollback.exec();
    ErrorReporter::error(QtCriticalMsg, this, tr("Error saving CRM Account"),
                         err, __FILE__, __LINE__);
    return;
  }

  XSqlQuery commit("COMMIT;");
  _NumberGen = -1;
  if (commit.lastError().type() != QSqlError::NoError)
  {
    ErrorReporter::error(QtCriticalMsg, this, tr("Database Error"),
                         commit, __FILE__, __LINE__);
    return;
  }

  omfgThis->sCustomersUpdated(-1, TRUE);
  omfgThis->sEmployeeUpdated(-1);
  omfgThis->sProspectsUpdated();
  omfgThis->sTaxAuthsUpdated(-1);
  omfgThis->sSalesRepUpdated(-1);
  omfgThis->sUserUpdated("");
  omfgThis->sVendorsUpdated();

  emit saved(_crmacctId);
  close();
}

QSqlError crmaccount::saveNoErrorCheck()
{
  XSqlQuery updateq;
  updateq.prepare("UPDATE crmacct "
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
                  "    crmacct_notes=:crmacct_notes,"
                  "    crmacct_emp_id=:crmacct_emp_id,"
                  "    crmacct_salesrep_id=:crmacct_salesrep_id,"
                  "    crmacct_usr_username=:crmacct_usr_username"
                  " WHERE (crmacct_id=:crmacct_id);" );

  updateq.bindValue(":crmacct_id",      _crmacctId);
  updateq.bindValue(":crmacct_number",  _number->text());
  updateq.bindValue(":crmacct_name",    _name->text());
  updateq.bindValue(":crmacct_active",  QVariant(_active->isChecked()));
  updateq.bindValue(":crmacct_type",    _organization->isChecked() ? "O" : "I");
  updateq.bindValue(":crmacct_notes",   _notes->toPlainText());

  if (_custId > 0)
    updateq.bindValue(":crmacct_cust_id",        _custId);
  if (_competitorId > 0)
    updateq.bindValue(":crmacct_competitor_id",  _competitorId);
  if (_partnerId > 0)
    updateq.bindValue(":crmacct_partner_id",     _partnerId);
  if (_prospectId > 0)
    updateq.bindValue(":crmacct_prospect_id",    _prospectId);
  if (_taxauthId > 0)
    updateq.bindValue(":crmacct_taxauth_id",     _taxauthId);
  if (_vendId > 0)
    updateq.bindValue(":crmacct_vend_id",        _vendId);
  if (_primary->id() > 0)
    updateq.bindValue(":crmacct_cntct_id_1",     _primary->id());
  if (_secondary->id() > 0)
    updateq.bindValue(":crmacct_cntct_id_2",     _secondary->id());
  if (_parentCrmacct->id() > 0)
    updateq.bindValue(":crmacct_parent_id",      _parentCrmacct->id());
  if (_empId > 0)
    updateq.bindValue(":crmacct_emp_id",         _empId);
  if (_salesrepId > 0)
    updateq.bindValue(":crmacct_salesrep_id",    _salesrepId);
  if (! _username.isEmpty())
    updateq.bindValue(":crmacct_usr_username",   _username);
  if (! _owner->username().isEmpty())
    updateq.bindValue(":crmacct_owner_username", _owner->username());

  updateq.exec();
  // no error check here. this method is specifically "saveNoErrorCheck()"

  omfgThis->sCrmAccountsUpdated(_crmacctId);

  return updateq.lastError();
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
  XSqlQuery delcaq;
  delcaq.prepare( "DELETE FROM charass "
             "WHERE (charass_id=:charass_id);" );
  delcaq.bindValue(":charass_id", _charass->id());
  delcaq.exec();
  if (ErrorReporter::error(QtCriticalMsg, this,
                           tr("Error deleting Characteristic Assignment"),
                           delcaq, __FILE__, __LINE__))
    return;

  sGetCharacteristics();
}

void crmaccount::sGetCharacteristics()
{
  XSqlQuery getcaq;
  getcaq.prepare( "SELECT charass_id, char_name, "
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
  getcaq.bindValue(":crmacct_id", _crmacctId);
  getcaq.exec();
  _charass->populate(getcaq);

  if (ErrorReporter::error(QtCriticalMsg, this,
                           tr("Error getting Characteristic Assignments"),
                           getcaq, __FILE__, __LINE__))
    return;
}

int crmaccount::id()
{
  return _crmacctId;
}

void crmaccount::setId(int id)
{
  _crmacctId = id;
  _todoList->parameterWidget()->setDefault(tr("CRM Account"), _crmacctId, true);
  _contacts->setCrmacctid(_crmacctId);
  sPopulate();

  if (cView != _mode)
    _mode = cEdit;

  emit newId(_crmacctId);
}

void crmaccount::sPopulate()
{
  XSqlQuery getq;
  getq.prepare( "SELECT *, userCanCreateUsers(getEffectiveXtUser()) AS cancreateusers "
             "FROM crmacct "
             "WHERE (crmacct_id=:crmacct_id);" );
  getq.bindValue(":crmacct_id", _crmacctId);
  getq.exec();
  if (getq.first())
  {
    _number->setText(getq.value("crmacct_number").toString());
    _name->setText(getq.value("crmacct_name").toString());
    _active->setChecked(getq.value("crmacct_active").toBool());

    QString acctType = getq.value("crmacct_type").toString();
    if (acctType == "O")
      _organization->setChecked(true);
    else if (acctType == "I")
      _individual->setChecked(true);
    else
      qWarning("crmaccount::sPopulate() - acctType '%s' incorrect",
               qPrintable(acctType));

    _canCreateUsers = getq.value("cancreateusers").toBool();
    _competitorId = getq.value("crmacct_competitor_id").toInt();
    _custId     = getq.value("crmacct_cust_id").toInt();
    _empId      = getq.value("crmacct_emp_id").toInt();
    _partnerId  = getq.value("crmacct_partner_id").toInt();
    _prospectId = getq.value("crmacct_prospect_id").toInt();
    _salesrepId = getq.value("crmacct_salesrep_id").toInt();
    _taxauthId  = getq.value("crmacct_taxauth_id").toInt();
    _username   = getq.value("crmacct_usr_username").toString();
    _vendId     = getq.value("crmacct_vend_id").toInt();

    _primary->setId(getq.value("crmacct_cntct_id_1").toInt());
    _secondary->setId(getq.value("crmacct_cntct_id_2").toInt());
    _notes->setText(getq.value("crmacct_notes").toString());
    _parentCrmacct->setId(getq.value("crmacct_parent_id").toInt());
    _comments->setId(_crmacctId);
    _documents->setId(_crmacctId);
    _primary->setSearchAcct(_crmacctId);
    _secondary->setSearchAcct(_crmacctId);
    _owner->setUsername(getq.value("crmacct_owner_username").toString());

    sHandleChildButtons();
  }
  else if (ErrorReporter::error(QtCriticalMsg, this,
                                tr("Error reading the CRM Account"),
                                getq, __FILE__, __LINE__))
    return;

  sGetCharacteristics();
  sPopulateRegistrations();
  _contacts->sFillList();
  _todoList->sFillList();
}

void crmaccount::sCompetitor()
{
  _competitorId = (_competitor->isChecked() ? 1 : 0);
}

void crmaccount::sCustomer()
{
  if ((cNew == _mode || cEdit == _mode) &&
      _privileges->check("MaintainAllCRMAccounts"))
  {
    QSqlError err = saveNoErrorCheck();
    if (ErrorReporter::error(QtCriticalMsg, this, tr("Error saving CRM Account"),
                             err, __FILE__, __LINE__))
      return;
  }

  ParameterList params;
  if (_prospectId > 0 && (cNew == _mode || cEdit == _mode) &&
      _privileges->check("MaintainProspectMasters") &&
      _privileges->check("MaintainCustomerMasters"))
  {
    int quotecount = 0;
    if (_privileges->check("ConvertQuotes"))
    {
      XSqlQuery quq;
      quq.prepare("SELECT COUNT(*) AS quotecount"
                  "  FROM quhead"
                  " WHERE (COALESCE(quhead_expire, endOfTime()) >= CURRENT_DATE)"
                  "   AND (quhead_cust_id=:id);");
      quq.bindValue(":id", _custId);
      quq.exec();
      if (quq.first())
        quotecount = quq.value("quotecount").toInt();
      // ignore errors
    }

    if (QMessageBox::question(this, tr("Convert"),
                              tr("<p>Are you sure you want to convert this "
                                 "Prospect to a Customer?"),
                              QMessageBox::Yes,
                              QMessageBox::No | QMessageBox::Default) == QMessageBox::No)
      return;

    bool convertquotes = quotecount != 0 &&
                         (QMessageBox::question(this, tr("Convert"),
                                                tr("<p>Do you want to convert all of the "
                                                   "Quotes for the Prospect to "
                                                   "Sales Orders?"),
                                  QMessageBox::Yes | QMessageBox::Default,
                                  QMessageBox::No) == QMessageBox::Yes);

    XSqlQuery convertq;
    convertq.prepare("SELECT convertProspectToCustomer(:id, :convertquotes);");
    convertq.bindValue(":id", _prospectId);
    convertq.bindValue(":convertquotes", convertquotes);
    convertq.exec();
    if (convertq.first())
    {
      _custId     = _prospectId;
      _prospectId = -1;

      omfgThis->sProspectsUpdated();
      omfgThis->sCustomersUpdated(_custId, true);
    } // convert prospect to customer
    else if (ErrorReporter::error(QtCriticalMsg, this, tr("Convert Error"),
                                  convertq, __FILE__, __LINE__))
      return;

    params.append("cust_id", _custId);
    params.append("mode",    "edit");
  }
  else if (_custId <= 0 && (cNew == _mode || cEdit == _mode) &&
           _privileges->check("MaintainCustomerMasters"))
  {
    params.append("crmacct_id", _crmacctId);
    params.append("mode",       "new");
  }     
  else if (_custId > 0)
  {
    params.append("cust_id", _custId);
    params.append("mode",    (_privileges->check("MaintainCustomerMasters") &&
                              (cNew == _mode || cEdit == _mode)) ?
                             "edit" : "view");
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
  if ((cNew == _mode || cEdit == _mode) &&
      _privileges->check("MaintainAllCRMAccounts"))
  {
    QSqlError err = saveNoErrorCheck();
    if (ErrorReporter::error(QtCriticalMsg, this, tr("Error saving CRM Account"),
                             err, __FILE__, __LINE__))
      return;
  }

  ParameterList params;
  if (_custId > 0 && (cNew == _mode || cEdit == _mode) &&
      _privileges->check("MaintainProspectMasters") &&
      _privileges->check("MaintainCustomerMasters"))
  {
    if (QMessageBox::question(this, tr("Convert"),
                    tr("<p>Are you sure you want to convert this Customer to "
                       "a Prospect and delete its Customer information?"),
                    QMessageBox::Yes,
                    QMessageBox::No | QMessageBox::Default) == QMessageBox::No)
      return;

    XSqlQuery createq;
    createq.prepare("SELECT convertCustomerToProspect(:id) AS prospect_id;");
    createq.bindValue(":id", _custId);
    createq.exec();
    if (createq.first())
    {
      _prospectId = createq.value("prospect_id").toInt();
      _custId = -1;

      omfgThis->sProspectsUpdated();
      omfgThis->sCustomersUpdated(_prospectId, true);
    }
    else if (ErrorReporter::error(QtCriticalMsg, this,
                                  tr("Error creating Prospect"),
                                  createq, __FILE__, __LINE__))
      return;

    params.append("prospect_id", _prospectId);
    params.append("mode",       "edit");
  }
  else if (_prospectId <= 0 && (cNew == _mode || cEdit == _mode) &&
           _privileges->check("MaintainProspectMasters"))
  {
    params.append("crmacct_id", _crmacctId);
    params.append("mode",       "new");
  }
  else if (_prospectId > 0)
  {
    params.append("prospect_id", _prospectId);
    params.append("mode",        (_privileges->check("MaintainProspectMasters") &&
                                  (cNew == _mode || cEdit == _mode)) ?
                                  "edit" : "view");
  }

  prospect *newdlg = new prospect(this);
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void crmaccount::sTaxAuth()
{
  if ((cNew == _mode || cEdit == _mode) &&
      _privileges->check("MaintainAllCRMAccounts"))
  {
    QSqlError err = saveNoErrorCheck();
    if (ErrorReporter::error(QtCriticalMsg, this, tr("Error saving CRM Account"),
                             err, __FILE__, __LINE__))
      return;
  }

  ParameterList params;
  if (_taxauthId <= 0 && _privileges->check("MaintainTaxAuthorities") &&
      (cNew == _mode || cEdit == _mode))
  {
    params.append("crmacct_id", _crmacctId);
    params.append("mode",       "new");
  }
  else
  {
    params.append("taxauth_id", _taxauthId);
    params.append("mode",       (_privileges->check("MaintainTaxAuthorities") &&
                                 (cNew == _mode || cEdit == _mode)) ?
                                  "edit" : "view");
  }

  taxAuthority *newdlg = new taxAuthority(this);
  newdlg->set(params);
  newdlg->exec();
}

void crmaccount::sEditVendor()
{
  if ((cNew == _mode || cEdit == _mode) &&
      _privileges->check("MaintainAllCRMAccounts"))
  {
    QSqlError err = saveNoErrorCheck();
    if (ErrorReporter::error(QtCriticalMsg, this, tr("Error saving CRM Account"),
                             err, __FILE__, __LINE__))
      return;
  }

  ParameterList params;
  if (_vendId <= 0 && _privileges->check("MaintainVendors") &&
      (cNew == _mode || cEdit == _mode))
  {
    params.append("crmacct_id", _crmacctId);
    params.append("mode",       "new");
  }
  else
  {
    params.append("vend_id", _vendId);
    params.append("mode",    (_privileges->check("MaintainVendors") &&
                                  (cNew == _mode || cEdit == _mode)) ?
                                "edit" : "view");
  }

  vendor *newdlg = new vendor(this);
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void crmaccount::sViewVendor()
{
  if ((cNew == _mode || cEdit == _mode) &&
      _privileges->check("MaintainAllCRMAccounts"))
  {
    QSqlError err = saveNoErrorCheck();
    if (ErrorReporter::error(QtCriticalMsg, this, tr("Error saving CRM Account"),
                             err, __FILE__, __LINE__))
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
  if (DEBUG)
    qDebug("crmaccount::sUpdateRelationships() entered");

  XSqlQuery getq;
  getq.prepare("SELECT * FROM crmacct WHERE (crmacct_id=:crmacct_id);" );
  getq.bindValue(":crmacct_id", _crmacctId);
  getq.exec();
  if (getq.first())
  {
    _number->setText(getq.value("crmacct_number").toString());
    _name->setText(getq.value("crmacct_name").toString());
    _competitorId = getq.value("crmacct_competitor_id").toInt();
    _custId     = getq.value("crmacct_cust_id").toInt();
    _empId      = getq.value("crmacct_emp_id").toInt();
    _partnerId  = getq.value("crmacct_partner_id").toInt();
    _prospectId = getq.value("crmacct_prospect_id").toInt();
    _salesrepId = getq.value("crmacct_salesrep_id").toInt();
    _taxauthId  = getq.value("crmacct_taxauth_id").toInt();
    _username   = getq.value("crmacct_usr_username").toString();
    _vendId     = getq.value("crmacct_vend_id").toInt();

    sHandleChildButtons();
    _owner->setUsername(getq.value("crmacct_owner_username").toString());
  }
  else if (ErrorReporter::error(QtCriticalMsg, this,
                                tr("Error reading the CRM Account"),
                                getq, __FILE__, __LINE__))
    return;

  _contacts->sFillList();
}

void crmaccount::doDialog(QWidget *parent, const ParameterList & pParams)
{
  crmaccount * ci = new crmaccount(parent);
  ci->set(pParams);
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
    XSqlQuery lsq;
    lsq.prepare( "SELECT lsreg_id,ls_number,item_number, item_descrip1, "
             "lsreg_qty,'qty' AS lsreg_qty_xtnumericrole,lsreg_solddate,"
             "lsreg_regdate,lsreg_expiredate, "
             "CASE WHEN lsreg_expiredate <= current_date THEN "
             "  'expired' "
             "END AS lsreg_expiredate_qtforegroundrole "
             "FROM lsreg,ls,item  "
             "WHERE ((lsreg_ls_id=ls_id) "
             "AND (ls_item_id=item_id) "
             "AND (lsreg_crmacct_id=:crmacct_id));");
    lsq.bindValue(":crmacct_id", _crmacctId);
    lsq.exec();
    _reg->clear();
    _reg->populate(lsq);
    if (ErrorReporter::error(QtCriticalMsg, this,
                             tr("Error collecting Lot/Serial data"),
                             lsq, __FILE__, __LINE__))
      return;
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
  XSqlQuery delq;
  delq.prepare( "DELETE FROM lsreg "
             "WHERE (lsreg_id=:lsreg_id);"
             "DELETE FROM charass "
             "WHERE ((charass_target_type='LSR') "
             "AND (charass_target_id=:lsreg_id))" );
  delq.bindValue(":lsreg_id", _reg->id());
  delq.exec();
  if (ErrorReporter::error(QtCriticalMsg, this,
                           tr("Error deleting Lot/Serial data"),
                           delq, __FILE__, __LINE__))
    return;

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
      if (ErrorReporter::error(QtCriticalMsg, this,
                               tr("Error deleting temporary CRM Account"),
                               query, __FILE__, __LINE__))
        return;

      setId(newq.value("crmacct_id").toInt());
      sPopulate();

      connect(_charass, SIGNAL(valid(bool)), _editCharacteristic, SLOT(setEnabled(bool)));
      connect(_charass, SIGNAL(valid(bool)), _deleteCharacteristic, SLOT(setEnabled(bool)));
      _name->setFocus();
      _number->setEnabled(FALSE);
    }
    else if (ErrorReporter::error(QtCriticalMsg, this, tr("Database Error"),
                                  newq, __FILE__, __LINE__))
      return;
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
    ErrorReporter::error(QtCriticalMsg, this, tr("Database Error"),
                         query, __FILE__, __LINE__);
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

void crmaccount::sHandleChildButtons()
{
  bool canEdit = (_mode == cNew || _mode == cEdit);

  if (DEBUG)
    qDebug("sHandleChildButtons() entered with competitor %d, customer %d, "
           "employee %d, partner %d, prospect %d, salesrep %d, taxauth %d, "
           "user %s, vendor %d\tcanEdit %d and _modal %d",
           _competitorId, _custId, _empId, _partnerId, _prospectId,
           _salesrepId, _taxauthId, qPrintable(_username), _vendId,
           canEdit, _modal);

  _competitor->setChecked(_competitorId > 0);
  _competitor->setEnabled(canEdit &&
                          _privileges->check("MaintainCompetitorMasters"));

  _customer->setChecked(_custId > 0);
  _customer->setEnabled(canEdit && !_modal &&
                        _privileges->check("MaintainCustomerMasters"));
  _customerButton->setEnabled(_custId > 0 &&
                              ! (parent() && parent()->inherits("customer")) &&
                              (_privileges->check("MaintainCustomerMasters") ||
                               _privileges->check("ViewCustomerMasters") ));

  _employee->setChecked(_empId > 0);
  _employee->setEnabled(canEdit && !_modal &&
                        _privileges->check("MaintainEmployees"));
  _employeeButton->setEnabled(_empId > 0 &&
                              ! (parent() && parent()->inherits("employee")) &&
                              (_privileges->check("MaintainEmployees") ||
                               _privileges->check("ViewEmployees")));

  _partner->setChecked(_partnerId > 0);
  _partner->setEnabled(canEdit && _privileges->check("MaintainPartners"));

  _prospect->setChecked(_prospectId > 0);
  _prospect->setEnabled(canEdit && !_modal &&
                        _privileges->check("MaintainProspectMasters"));
  _prospectButton->setEnabled(_prospectId > 0 &&
                              ! (parent() && parent()->inherits("prospect")) &&
                              (_privileges->check("MaintainProspectMasters") ||
                               _privileges->check("ViewProspectMasters")));

  _salesrep->setChecked(_salesrepId > 0);
  _salesrep->setEnabled(canEdit && !_modal &&
                        _privileges->check("MaintainSalesReps"));
  _salesrepButton->setEnabled(_salesrepId > 0 &&
                              ! (parent() && parent()->inherits("salesRep")) &&
                              (_privileges->check("MaintainSalesReps") ||
                               _privileges->check("ViewSalesReps")));

  _taxauth->setChecked(_taxauthId > 0);
  _taxauth->setEnabled(canEdit && !_modal &&
                       _privileges->check("MaintainTaxAuthorities"));
  _taxauthButton->setEnabled(_taxauthId > 0 &&
                             ! (parent() && parent()->inherits("taxAuthority")) &&
                             (_privileges->check("MaintainTaxAuthorities") ||
                              _privileges->check("ViewTaxAuthorities")));

  // only allow turning ON the _user checkbox from here, never off
  _user->setChecked(! _username.isEmpty());
  _user->setEnabled(canEdit && !_modal && _privileges->check("MaintainUsers") &&
                    _canCreateUsers    && _username.isEmpty());
  _userButton->setEnabled(! _username.isEmpty() &&
                          ! (parent() && parent()->inherits("user")) &&
                          _privileges->check("MaintainUsers"));

  _vendor->setChecked(_vendId > 0);
  _vendor->setEnabled(canEdit && !_modal &&
                      ! (parent() && parent()->inherits("vendor")) &&
                      _privileges->check("MaintainVendors"));
  if (_vendId > 0 &&
      (_privileges->check("MaintainVendors") ||
       _privileges->check("ViewVendors")))
  {
    _vendorButton->setText(tr("Vendor"));
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
  else
    _vendorButton->setEnabled(false);
}

void crmaccount::sHandleCntctDetach(int cntctId)
{
  if (_primary->id() == cntctId)
    _primary->clear();
  if (_secondary->id() == cntctId)
    _secondary->clear();
}

void crmaccount::sEmployee()
{
  if ((cNew == _mode || cEdit == _mode) &&
      _privileges->check("MaintainAllCRMAccounts"))
  {
    QSqlError err = saveNoErrorCheck();
    if (ErrorReporter::error(QtCriticalMsg, this, tr("Error saving CRM Account"),
                             err, __FILE__, __LINE__))
      return;
  }

  ParameterList params;
  if (_empId <= 0 && _privileges->check("MaintainEmployees") &&
      (cNew == _mode || cEdit == _mode))
  {
    params.append("crmacct_id", _crmacctId);
    params.append("mode",       "new");
  }
  else
  {
    params.append("emp_id", _empId);
    params.append("mode",   (_privileges->check("MaintainEmployees") &&
                             (cNew == _mode || cEdit == _mode)) ?
                             "edit" : "view");
  }

  employee *newdlg = new employee(this);
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void crmaccount::sSalesRep()
{
  if ((cNew == _mode || cEdit == _mode) &&
      _privileges->check("MaintainAllCRMAccounts"))
  {
    QSqlError err = saveNoErrorCheck();
    if (ErrorReporter::error(QtCriticalMsg, this, tr("Error saving CRM Account"),
                             err, __FILE__, __LINE__))
      return;
  }

  ParameterList params;
  if (_salesrepId <= 0 && _privileges->check("MaintainSalesReps") &&
      (cNew == _mode || cEdit == _mode))
  {
    params.append("crmacct_id", _crmacctId);
    params.append("mode",       "new");
  }
  else
  {
    params.append("salesrep_id", _salesrepId);
    params.append("mode",        (_privileges->check("MaintainSalesReps") &&
                                  (cNew == _mode || cEdit == _mode)) ?
                                  "edit" : "view");
  }

  salesRep *newdlg = new salesRep(this);
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void crmaccount::sUser()
{
  if ((cNew == _mode || cEdit == _mode) &&
      _privileges->check("MaintainAllCRMAccounts"))
  {
    QSqlError err = saveNoErrorCheck();
    if (ErrorReporter::error(QtCriticalMsg, this, tr("Error saving CRM Account"),
                             err, __FILE__, __LINE__))
      return;
  }

  ParameterList params;
  if (_username.isEmpty() && _privileges->check("MaintainUsers") &&
      (cNew == _mode || cEdit == _mode))
  {
    params.append("crmacct_id", _crmacctId);
    params.append("mode",       "new");
  }
  else
  {
    params.append("username", _username);
    params.append("mode",     (_privileges->check("MaintainUsers") &&
                               (cNew == _mode || cEdit == _mode)) ?
                               "edit" : "view");
  }

  user *newdlg = new user(this);
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}
