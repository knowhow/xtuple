/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "vendor.h"

#include <QCloseEvent>
#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include <metasql.h>
#include <openreports.h>

#include "addresscluster.h"
#include "comment.h"
#include "crmaccount.h"
#include "errorReporter.h"
#include "guiErrorCheck.h"
#include "storedProcErrorLookup.h"
#include "taxRegistration.h"
#include "vendorAddress.h"
#include "xcombobox.h"

#define DEBUG false

vendor::vendor(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  connect(_crmacct, SIGNAL(clicked()), this, SLOT(sCrmAccount()));
  connect(_save, SIGNAL(clicked()), this, SLOT(sSave()));
  connect(_printAddresses, SIGNAL(clicked()), this, SLOT(sPrintAddresses()));
  connect(_newAddress, SIGNAL(clicked()), this, SLOT(sNewAddress()));
  connect(_editAddress, SIGNAL(clicked()), this, SLOT(sEditAddress()));
  connect(_viewAddress, SIGNAL(clicked()), this, SLOT(sViewAddress()));
  connect(_deleteAddress, SIGNAL(clicked()), this, SLOT(sDeleteAddress()));
  connect(_deleteTaxreg, SIGNAL(clicked()), this, SLOT(sDeleteTaxreg()));
  connect(_editTaxreg,   SIGNAL(clicked()), this, SLOT(sEditTaxreg()));
  connect(_newTaxreg,    SIGNAL(clicked()), this, SLOT(sNewTaxreg()));
  connect(_viewTaxreg,   SIGNAL(clicked()), this, SLOT(sViewTaxreg()));
  connect(_next, SIGNAL(clicked()), this, SLOT(sNext()));
  connect(_previous, SIGNAL(clicked()), this, SLOT(sPrevious()));
  connect(_mainButton, SIGNAL(clicked()), this, SLOT(sHandleButtons()));
  connect(_altButton, SIGNAL(clicked()), this, SLOT(sHandleButtons()));
  connect(_checksButton, SIGNAL(clicked()), this, SLOT(sHandleButtons()));
  connect(_number, SIGNAL(textEdited(const QString&)), this, SLOT(sNumberEdited()));
  connect(_number, SIGNAL(lostFocus()), this, SLOT(sCheck()));

  connect(_address, SIGNAL(addressChanged(QString,QString,QString,QString,QString,QString, QString)),
          _contact2, SLOT(setNewAddr(QString,QString,QString,QString,QString,QString, QString)));

  connect(_address, SIGNAL(addressChanged(QString,QString,QString,QString,QString,QString, QString)),
          _contact1, SLOT(setNewAddr(QString,QString,QString,QString,QString,QString, QString)));

  _defaultCurr->setLabel(_defaultCurrLit);

  QRegExp tmpregex = QRegExp(_metrics->value("EFTAccountRegex"));
  _accountValidator = new QRegExpValidator (tmpregex, this);
  tmpregex = QRegExp(_metrics->value("EFTRoutingRegex"));
  _routingValidator = new QRegExpValidator(tmpregex, this);

  _routingNumber->setValidator(_routingValidator);
  _achAccountNumber->setValidator(_accountValidator);

  _vendaddr->addColumn(tr("Number"), 70, Qt::AlignLeft, true, "vendaddr_code");
  _vendaddr->addColumn(tr("Name"),   50, Qt::AlignLeft, true, "vendaddr_name");
  _vendaddr->addColumn(tr("City"),   -1, Qt::AlignLeft, true, "addr_city");
  _vendaddr->addColumn(tr("State"),  -1, Qt::AlignLeft, true, "addr_state");
  _vendaddr->addColumn(tr("Country"),-1, Qt::AlignLeft, true, "addr_country");
  _vendaddr->addColumn(tr("Postal Code"),-1, Qt::AlignLeft, true, "addr_postalcode");

  _taxreg->addColumn(tr("Tax Authority"), 100, Qt::AlignLeft, true, "taxauth_code");
  _taxreg->addColumn(tr("Tax Zone"),      100, Qt::AlignLeft, true, "taxzone_code");
  _taxreg->addColumn(tr("Registration #"), -1, Qt::AlignLeft, true, "taxreg_number");

  _accountType->append(0, "Checking", "K");
  _accountType->append(1, "Savings",  "C");

  _transmitStack->setCurrentIndex(0);
  if (_metrics->boolean("EnableBatchManager") &&
      ! (_metrics->boolean("ACHSupported") && _metrics->boolean("ACHEnabled")))
  {
    _checksButton->hide();
  }
  else if (! _metrics->boolean("EnableBatchManager") &&
           (_metrics->boolean("ACHSupported") && _metrics->boolean("ACHEnabled")))
  {
    _checksButton->hide();
    _transmitStack->setCurrentIndex(1);
  }
  else if (! _metrics->boolean("EnableBatchManager") &&
           ! (_metrics->boolean("ACHSupported") && _metrics->boolean("ACHEnabled")))
    ediTab->setVisible(false);
  // else defaults are OK

  if (_metrics->boolean("ACHSupported") && _metrics->boolean("ACHEnabled") && omfgThis->_key.isEmpty())
    _checksButton->setEnabled(false);

  _account->setType(GLCluster::cRevenue | GLCluster::cExpense |
                    GLCluster::cAsset | GLCluster::cLiability);

  _vendid      = -1;
  _crmacctid   = -1;
  _ignoreClose = false;
  _NumberGen   = -1;
}

vendor::~vendor()
{
  // no need to delete child widgets, Qt does it all for us
}

void vendor::languageChange()
{
  retranslateUi(this);
}

SetResponse vendor::set(const ParameterList &pParams)
{
  XWidget::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("crmacct_id", &valid);
  if (valid)
    _crmacctid = param.toInt();

  param = pParams.value("vend_id", &valid);
  if (valid)
    _vendid = param.toInt();

  if (_vendid > 0 || _crmacctid > 0)
    if (! sPopulate())
      return UndefinedError;

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      _mode = cNew;

      XSqlQuery idq;
      idq.exec("SELECT NEXTVAL('vend_vend_id_seq') AS vend_id;");
      if (idq.first())
        _vendid = idq.value("vend_id").toInt();
      else if (ErrorReporter::error(QtCriticalMsg, this, tr("Getting Id"),
                                    idq, __FILE__, __LINE__))
        return UndefinedError;

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

      if (_privileges->check("MaintainVendorAddresses"))
      {
        connect(_vendaddr, SIGNAL(valid(bool)), _editAddress, SLOT(setEnabled(bool)));
        connect(_vendaddr, SIGNAL(valid(bool)), _deleteAddress, SLOT(setEnabled(bool)));
        connect(_vendaddr, SIGNAL(itemSelected(int)), _editAddress, SLOT(animateClick()));
      }
      else
      {
        _newAddress->setEnabled(FALSE);
        connect(_vendaddr, SIGNAL(itemSelected(int)), _viewAddress, SLOT(animateClick()));
      }

      _number->setFocus();
      emit newId(_vendid);
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;

      if (_privileges->check("MaintainVendorAddresses"))
      {
        connect(_vendaddr, SIGNAL(valid(bool)), _editAddress, SLOT(setEnabled(bool)));
        connect(_vendaddr, SIGNAL(valid(bool)), _deleteAddress, SLOT(setEnabled(bool)));
        connect(_vendaddr, SIGNAL(itemSelected(int)), _editAddress, SLOT(animateClick()));
      }
      else
      {
        _newAddress->setEnabled(FALSE);
        connect(_vendaddr, SIGNAL(itemSelected(int)), _viewAddress, SLOT(animateClick()));
      }

      _save->setFocus();
    }
    else if (param.toString() == "view")
    {
      _mode = cView;

      _number->setEnabled(FALSE);
      _vendtype->setEnabled(FALSE);
      _active->setEnabled(FALSE);
      _name->setEnabled(FALSE);
      _accountNumber->setEnabled(FALSE);
      _defaultTerms->setEnabled(FALSE);
      _defaultShipVia->setEnabled(FALSE);
      _defaultCurr->setEnabled(FALSE);
      _contact1->setEnabled(FALSE);
      _contact2->setEnabled(FALSE);
      _address->setEnabled(FALSE);
      _notes->setReadOnly(TRUE);
      _poComments->setReadOnly(TRUE);
      _poItems->setEnabled(FALSE);
      _restrictToItemSource->setEnabled(FALSE);
      _receives1099->setEnabled(FALSE);
      _qualified->setEnabled(FALSE);
      _newAddress->setEnabled(FALSE);
      _defaultFOBGroup->setEnabled(false);
      _taxzone->setEnabled(false);
      _match->setEnabled(false);
      _newTaxreg->setEnabled(false);
      _comments->setReadOnly(TRUE);

      _achGroup->setEnabled(false);
      _routingNumber->setEnabled(false);
      _achAccountNumber->setEnabled(false);
      _individualId->setEnabled(false);
      _individualName->setEnabled(false);
      _accountType->setEnabled(false);
      _distribGroup->setEnabled(false);

      _save->hide();
      _close->setText(tr("&Close"));

      disconnect(_taxreg, SIGNAL(valid(bool)), _deleteTaxreg, SLOT(setEnabled(bool)));
      disconnect(_taxreg, SIGNAL(valid(bool)), _editTaxreg, SLOT(setEnabled(bool)));
      disconnect(_taxreg, SIGNAL(itemSelected(int)), _editTaxreg, SLOT(animateClick()));
      connect(_taxreg, SIGNAL(itemSelected(int)), _viewTaxreg, SLOT(animateClick()));

      _close->setFocus();
    }
  }

  if(_metrics->value("CRMAccountNumberGeneration") == "A")
    _number->setEnabled(FALSE);

  if(cNew == _mode || !pParams.inList("showNextPrev"))
  {
    _next->hide();
    _previous->hide();
  }

  return NoError;
}

int vendor::id() const
{
  return _vendid;
}

void vendor::sSave()
{
  QList<GuiErrorCheck> errors;
  errors << GuiErrorCheck(_number->text().trimmed().isEmpty(), _number,
                          tr("Please enter a Number for this new Vendor."))
         << GuiErrorCheck(_name->text().trimmed().isEmpty(), _name,
                          tr("Please enter a Name for this new Vendor."))
         << GuiErrorCheck(_defaultTerms->id() == -1, _defaultTerms,
                          tr("You must select a Terms code for this Vendor."))
         << GuiErrorCheck(_vendtype->id() == -1, _vendtype,
                          tr("You must select a Vendor Type for this Vendor."))
//         << GuiErrorCheck(_accountSelected->isChecked() &&
//                          !_account->isValid(),
//                          _account
//                          tr("You must select a Default Distribution Account for this Vendor."))
//         << GuiErrorCheck(_expcatSelected->isChecked() &&
//                          !_expcat->isValid(),
//                          _expcat
//                          tr("You must select a Default Distribution Expense Category for this Vendor."))
//         << GuiErrorCheck(_taxSelected->isChecked() &&
//                          !_taxCode->isValid(),
//                          _taxCode
//                          tr("You must select a Default Distribution Tax Code for this Vendor."))
         << GuiErrorCheck(_achGroup->isChecked() &&
                          ! _routingNumber->hasAcceptableInput() &&
                          !omfgThis->_key.isEmpty(),
                          _routingNumber,
                          tr("The Routing Number is not valid."))
         << GuiErrorCheck(_achGroup->isChecked() &&
                          ! _achAccountNumber->hasAcceptableInput() &&
                          !omfgThis->_key.isEmpty(), _achAccountNumber,
                          tr("The Account Number is not valid."))
         << GuiErrorCheck(_achGroup->isChecked() &&
                          _useACHSpecial->isChecked() &&
                          _individualName->text().trimmed().isEmpty() &&
                          !omfgThis->_key.isEmpty(),
                          _individualName,
                          tr("Please enter an Individual Name if EFT Check "
                             "Printing is enabled and '%1' is checked.")
                            .arg(_useACHSpecial->title()))
    ;

  if (_number->text().trimmed().toUpper() != _cachedNumber.toUpper())
  {
    XSqlQuery dupq;
    dupq.prepare("SELECT vend_name "
                 "FROM vendinfo "
                 "WHERE (UPPER(vend_number)=UPPER(:vend_number)) "
                 "  AND (vend_id<>:vend_id);" );
    dupq.bindValue(":vend_number", _number->text().trimmed());
    dupq.bindValue(":vend_id", _vendid);
    dupq.exec();
    if (dupq.first())
      GuiErrorCheck(true, _number,
			    tr("<p>The newly entered Vendor Number cannot be "
                               "used as it is already used by the Vendor '%1'. "
                               "Please correct or enter a new Vendor Number." )
			     .arg(q.value("vend_name").toString()) );
  }

  if (GuiErrorCheck::reportErrors(this, tr("Cannot Save Vendor"), errors))
    return;

  XSqlQuery rollback;
  rollback.prepare("ROLLBACK;");

  XSqlQuery begin("BEGIN;");
  if (ErrorReporter::error(QtCriticalMsg, this, tr("Database Error"),
                           begin, __FILE__, __LINE__))
    return;

  int saveResult = _address->save(AddressCluster::CHECK);
  if (-2 == saveResult)
  {
    int answer = QMessageBox::question(this,
		    tr("Question Saving Address"),
		    tr("<p>There are multiple uses of this Vendor's "
		       "Address. What would you like to do?"),
		    tr("Change This One"),
		    tr("Change Address for All"),
		    tr("Cancel"),
		    2, 2);
    if (0 == answer)
      saveResult = _address->save(AddressCluster::CHANGEONE);
    else if (1 == answer)
      saveResult = _address->save(AddressCluster::CHANGEALL);
  }
  if (saveResult < 0)	// not else-if: this is error check for CHANGE{ONE,ALL}
  {
    rollback.exec();
    ErrorReporter::error(QtCriticalMsg, this, tr("Error Saving Address"),
                         tr("<p>There was an error saving this address (%1). "
			 "Check the database server log for errors.")
                         .arg(saveResult), __FILE__, __LINE__);
    _address->setFocus();
    return;
  }

  QString sql;
  if (_mode == cEdit)
  {
    sql = "UPDATE vendinfo "
          "SET vend_number=<? value(\"vend_number\") ?>,"
          "    vend_accntnum=<? value(\"vend_accntnum\") ?>,"
          "    vend_active=<? value(\"vend_active\") ?>,"
          "    vend_vendtype_id=<? value(\"vend_vendtype_id\") ?>,"
          "    vend_name=<? value(\"vend_name\") ?>,"
          "    vend_cntct1_id=<? value(\"vend_cntct1_id\") ?>,"
          "    vend_cntct2_id=<? value(\"vend_cntct2_id\") ?>,"
	  "    vend_addr_id=<? value(\"vend_addr_id\") ?>,"
          "    vend_po=<? value(\"vend_po\") ?>,"
          "    vend_restrictpurch=<? value(\"vend_restrictpurch\") ?>,"
          "    vend_1099=<? value(\"vend_1099\") ?>,"
          "    vend_qualified=<? value(\"vend_qualified\") ?>,"
          "    vend_comments=<? value(\"vend_comments\") ?>,"
          "    vend_pocomments=<? value(\"vend_pocomments\") ?>,"
          "    vend_fobsource=<? value(\"vend_fobsource\") ?>,"
          "    vend_fob=<? value(\"vend_fob\") ?>,"
          "    vend_terms_id=<? value(\"vend_terms_id\") ?>,"
          "    vend_shipvia=<? value(\"vend_shipvia\") ?>,"
	  "    vend_curr_id=<? value(\"vend_curr_id\") ?>,"
          "    vend_taxzone_id=<? value(\"vend_taxzone_id\") ?>,"
          "    vend_match=<? value(\"vend_match\") ?>,"
          "    vend_ach_enabled=<? value(\"vend_ach_enabled\") ?>,"
          "<? if exists(\"key\") ?>"
          "    vend_ach_routingnumber=encrypt(setbytea(<? value(\"vend_ach_routingnumber\") ?>),"
          "                             setbytea(<? value(\"key\") ?>), 'bf'),"
          "    vend_ach_accntnumber=encrypt(setbytea(<? value(\"vend_ach_accntnumber\") ?>),"
          "                           setbytea(<? value(\"key\") ?>), 'bf'),"
          "<? endif ?>"
          "    vend_ach_use_vendinfo=<? value(\"vend_ach_use_vendinfo\") ?>,"
          "    vend_ach_accnttype=<? value(\"vend_ach_accnttype\") ?>,"
          "    vend_ach_indiv_number=<? value(\"vend_ach_indiv_number\") ?>,"
          "    vend_ach_indiv_name=<? value(\"vend_ach_indiv_name\") ?>,"
          "    vend_accnt_id=<? value(\"vend_accnt_id\") ?>,"
          "    vend_expcat_id=<? value(\"vend_expcat_id\") ?>,"
          "    vend_tax_id=<? value(\"vend_tax_id\") ?> "
          "WHERE (vend_id=<? value(\"vend_id\") ?>);" ;
  }
  else if (_mode == cNew)
    sql = "INSERT INTO vendinfo "
          "( vend_id, vend_number, vend_accntnum,"
          "  vend_active, vend_vendtype_id, vend_name,"
          "  vend_cntct1_id, vend_cntct2_id, vend_addr_id,"
          "  vend_po, vend_restrictpurch,"
          "  vend_1099, vend_qualified,"
          "  vend_comments, vend_pocomments,"
          "  vend_fobsource, vend_fob,"
          "  vend_terms_id, vend_shipvia, vend_curr_id,"
          "  vend_taxzone_id, vend_match, vend_ach_enabled,"
          "  vend_ach_routingnumber, vend_ach_accntnumber,"
          "  vend_ach_use_vendinfo,"
          "  vend_ach_accnttype, vend_ach_indiv_number,"
          "  vend_ach_indiv_name,"
          "  vend_accnt_id, vend_expcat_id, vend_tax_id) "
          "VALUES "
          "( <? value(\"vend_id\") ?>,"
          "  <? value(\"vend_number\") ?>,"
          "  <? value(\"vend_accntnum\") ?>,"
          "  <? value(\"vend_active\") ?>,"
          "  <? value(\"vend_vendtype_id\") ?>,"
          "  <? value(\"vend_name\") ?>,"
          "  <? value(\"vend_cntct1_id\") ?>,"
          "  <? value(\"vend_cntct2_id\") ?>,"
          "  <? value(\"vend_addr_id\") ?>,"
          "  <? value(\"vend_po\") ?>,"
          "  <? value(\"vend_restrictpurch\") ?>,"
          "  <? value(\"vend_1099\") ?>,"
          "  <? value(\"vend_qualified\") ?>,"
          "  <? value(\"vend_comments\") ?>,"
          "  <? value(\"vend_pocomments\") ?>,"
          "  <? value(\"vend_fobsource\") ?>,"
          "  <? value(\"vend_fob\") ?>,"
          "  <? value(\"vend_terms_id\") ?>,"
          "  <? value(\"vend_shipvia\") ?>,"
          "  <? value(\"vend_curr_id\") ?>, "
          "  <? value(\"vend_taxzone_id\") ?>,"
          "  <? value(\"vend_match\") ?>,"
          "  <? value(\"vend_ach_enabled\") ?>,"
          "<? if exists(\"key\") ?>"
          "  encrypt(setbytea(<? value(\"vend_ach_routingnumber\") ?>),"
          "          setbytea(<? value(\"key\") ?>), 'bf'),"
          "  encrypt(setbytea(<? value(\"vend_ach_accntnumber\") ?>),"
          "          setbytea(<? value(\"key\") ?>), 'bf'),"
          "<? else ?>"
          "  '',"
          "  '',"
          "<? endif ?>"
          "  <? value(\"vend_ach_use_vendinfo\") ?>,"
          "  <? value(\"vend_ach_accnttype\") ?>,"
          "  <? value(\"vend_ach_indiv_number\") ?>,"
          "  <? value(\"vend_ach_indiv_name\") ?>,"
          "  <? value(\"vend_accnt_id\") ?>,"
          "  <? value(\"vend_expcat_id\") ?>,"
          "  <? value(\"vend_tax_id\") ?> "
          "   );"  ;

  ParameterList params;
  params.append("vend_id", _vendid);
  params.append("vend_vendtype_id", _vendtype->id());
  params.append("vend_terms_id", _defaultTerms->id());
  params.append("vend_curr_id", _defaultCurr->id());

  params.append("vend_number",   _number->text().trimmed().toUpper());
  params.append("vend_accntnum", _accountNumber->text().trimmed());
  params.append("vend_name",     _name->text().trimmed());

  if (_contact1->id() > 0)
    params.append("vend_cntct1_id", _contact1->id());		// else NULL
  if (_contact2->id() > 0)
    params.append("vend_cntct2_id", _contact2->id());		// else NULL
  if (_address->id() > 0)
    params.append("vend_addr_id", _address->id());		// else NULL

  params.append("vend_comments",   _notes->toPlainText());
  params.append("vend_pocomments", _poComments->toPlainText());
  params.append("vend_shipvia",    _defaultShipVia->text());

  params.append("vend_active",        QVariant(_active->isChecked()));
  params.append("vend_po",            QVariant(_poItems->isChecked()));
  params.append("vend_restrictpurch", QVariant(_restrictToItemSource->isChecked()));
  params.append("vend_1099",          QVariant(_receives1099->isChecked()));
  params.append("vend_qualified",     QVariant(_qualified->isChecked()));
  params.append("vend_match",         QVariant(_match->isChecked()));

  if (!omfgThis->_key.isEmpty())
    params.append("key",                   omfgThis->_key);
  params.append("vend_ach_enabled",      QVariant(_achGroup->isChecked()));
  params.append("vend_ach_routingnumber",_routingNumber->text().trimmed());
  params.append("vend_ach_accntnumber",  _achAccountNumber->text().trimmed());
  params.append("vend_ach_use_vendinfo", QVariant(! _useACHSpecial->isChecked()));
  params.append("vend_ach_indiv_number", _individualId->text().trimmed());
  params.append("vend_ach_indiv_name",   _individualName->text().trimmed());

  params.append("vend_ach_accnttype",  _accountType->code());

  if(_taxzone->isValid())
    params.append("vend_taxzone_id", _taxzone->id());

  if (_useWarehouseFOB->isChecked())
  {
    params.append("vend_fobsource", "W");
    params.append("vend_fob", "");
  }
  else if (_useVendorFOB)
  {
    params.append("vend_fobsource", "V");
    params.append("vend_fob", _vendorFOB->text().trimmed());
  }

  if(_accountSelected->isChecked() && _account->isValid())
  {
    params.append("vend_accnt_id", _account->id());
    params.append("vend_expcat_id", -1);
    params.append("vend_tax_id", -1);
  }
  else if (_expcatSelected->isChecked() && _expcat->isValid())
  {
    params.append("vend_accnt_id", -1);
    params.append("vend_expcat_id", _expcat->id());
    params.append("vend_tax_id", -1);
  }
  else if (_taxSelected->isChecked() && _taxCode->isValid())
  {
    params.append("vend_accnt_id", -1);
    params.append("vend_expcat_id", -1);
    params.append("vend_tax_id", _taxCode->id());
  }
  else
  {
    params.append("vend_accnt_id", -1);
    params.append("vend_expcat_id", -1);
    params.append("vend_tax_id", -1);
  }

  MetaSQLQuery mql(sql);
  XSqlQuery upsq = mql.toQuery(params);
  if (upsq.lastError().type() != QSqlError::NoError)
  {
    rollback.exec();
    ErrorReporter::error(QtCriticalMsg, this, tr("Error Saving Vendor"),
                         q, __FILE__, __LINE__);
    return;
  }


  XSqlQuery commit("COMMIT;");
  _NumberGen = -1;
  omfgThis->sVendorsUpdated();
  emit saved(_vendid);
  if (_mode == cNew)
    emit newId(_vendid);

  if(!_ignoreClose)
    close();
}

void vendor::sCheck()
{
  _number->setText(_number->text().trimmed().toUpper());
  if (_number->text().length() && _cachedNumber != _number->text())
  {
    if(cNew == _mode && -1 != _NumberGen && _number->text().toInt() != _NumberGen)
    {
      XSqlQuery query;
      query.prepare( "SELECT releaseCRMAccountNumber(:Number);" );
      query.bindValue(":Number", _NumberGen);
      query.exec();
      ErrorReporter::error(QtCriticalMsg, this, tr("Releasing Number"),
                            query, __FILE__, __LINE__);
      _NumberGen = -1;
    }

    XSqlQuery dupq;
    dupq.prepare("SELECT vend_id, 1 AS type"
                 "  FROM vendinfo "
                 " WHERE (vend_number=:vend_number)"
                 "  AND (vend_id!=:vendid)"
                 " UNION "
                 "SELECT crmacct_id, 2 AS type "
                 "  FROM crmacct "
                 " WHERE (crmacct_number=:vend_number)"
                 "  AND (crmacct_vend_id!=:vendid)"
                 " ORDER BY type;");
    dupq.bindValue(":vend_number", _number->text());
    dupq.bindValue(":vendid",      _vendid);
    dupq.exec();
    if (dupq.first())
    {
      if ((dupq.value("type").toInt() == 1) && (_notice))
      {
        if (QMessageBox::question(this, tr("Vendor Exists"),
                tr("<p>This number is currently "
                     "used by an existing Vendor. "
                     "Do you want to edit "
                     "that Vendor?"),
                QMessageBox::Yes,
                QMessageBox::No | QMessageBox::Default) == QMessageBox::No)
        {
          _number->setText(_cachedNumber);
          _number->setFocus();
          return;
        }
        _vendid = dupq.value("vend_id").toInt();
        _mode = cEdit;
        sPopulate();
        _name->setFocus();
      }
      else if ( (_mode == cEdit) &&
                ((dupq.value("type").toInt() == 2) ) &&
                (_notice))
      {
        QMessageBox::critical(this, tr("Invalid Number"),
                              tr("<p>This number is currently "
                                 "assigned to another CRM account."));
        _number->setText(_cachedNumber);
        _number->setFocus();
        _notice = false;
        return;
      }
      else if ((dupq.value("type").toInt() == 2) && (_notice))
      {
        if (QMessageBox::question(this, tr("Convert"),
                tr("<p>This number is currently assigned to CRM Account. "
                   "Do you want to convert the CRM Account to a Vendor?"),
                QMessageBox::Yes,
                QMessageBox::No | QMessageBox::Default) == QMessageBox::No)
        {
          _number->clear();
          _number->setFocus();
          return;
        }
        _vendid = -1;
        _crmacctid = dupq.value("vend_id").toInt();
        sPopulate();
      }
    }
    else if (ErrorReporter::error(QtCriticalMsg, this, tr("Getting Vendor"),
                                  dupq, __FILE__, __LINE__))
      return;
  }
}

bool vendor::sPopulate()
{
  if (DEBUG)
    qDebug("vendor::sPopulate() entered with _vendid %d and _crmacctid %d",
           _vendid, _crmacctid);

  MetaSQLQuery mql(
            "<? if exists('vend_id') ?>"
            "SELECT vendinfo.*, crmacct_id, crmacct_owner_username, "
            "<? if exists('key') ?>"
            "       CASE WHEN LENGTH(vend_ach_routingnumber) > 0 THEN"
            "       formatbytea(decrypt(setbytea(vend_ach_routingnumber),"
            "                           setbytea(<? value('key') ?>), 'bf'))"
            "            ELSE '' END AS routingnum,"
            "       CASE WHEN LENGTH(vend_ach_accntnumber) > 0 THEN"
            "       formatbytea(decrypt(setbytea(vend_ach_accntnumber),"
            "                           setbytea(<? value('key') ?>), 'bf'))"
            "            ELSE '' END AS accntnum "
            "<? else ?>"
            "       <? value('na') ?> AS routingnum,"
            "       <? value('na') ?> AS accntnum "
            "<? endif ?>"
            "FROM vendinfo "
            "  JOIN crmacct ON (vend_id=crmacct_vend_id) "
            "WHERE (vend_id=<? value('vend_id') ?>);"
            "<? elseif exists('crmacct_id') ?>"
            "SELECT crmacct_number AS vend_number, crmacct_name   AS vend_name,"
            "       crmacct_active AS vend_active,"
            "       crmacct_cntct_id_1 AS vend_cntct1_id,"
            "       crmacct_cntct_id_2 AS vend_cntct2_id,"
            "       fetchMetricText('DefaultPOShipVia') AS vend_shipvia,"
            "       NULL AS vend_accntnum,    NULL AS vend_vendtype_id,"
            "       NULL AS vend_name,        NULL AS vend_addr_id,"
            "       fetchMetricValue('DefaultTerms') AS vend_terms_id,"
            "       NULL  AS vend_curr_id,"
            "       FALSE AS vend_po,         FALSE AS vend_restrictpurch,"
            "       FALSE AS vend_1099,       NULL AS vend_match,"
            "       FALSE  AS vend_qualified, NULL AS vend_comments,"
            "       NULL AS vend_pocomments,  NULL AS vend_taxzone_id,"
            "       'W'  AS vend_fobsource,   NULL AS vend_fob,"
            "       NULL AS vend_ach_enabled, NULL AS routingnum,"
            "       NULL AS accntnum,         NULL AS vend_ach_use_vendinfo,"
            "       NULL AS vend_ach_indiv_number, NULL AS vend_ach_indiv_name,"
            "       NULL AS vend_ach_accnttype,"
            "       crmacct_id, crmacct_owner_username"
            "  FROM crmacct"
            " WHERE crmacct_id=<? value('crmacct_id') ?>;"
            "<? endif ?>");
  ParameterList params;
  if (_vendid > 0)
    params.append("vend_id", _vendid);
  else if (_crmacctid > 0)
    params.append("crmacct_id", _crmacctid);

  params.append("key",     omfgThis->_key);
  params.append("na",      tr("N/A"));
  XSqlQuery getq = mql.toQuery(params);
  if (getq.first())
  {
    _notice = FALSE;
    _cachedNumber = getq.value("vend_number").toString();

    _crmacctid = getq.value("crmacct_id").toInt();
    _crmowner = getq.value("crmacct_owner_username").toString();
    _number->setText(getq.value("vend_number"));
    _accountNumber->setText(getq.value("vend_accntnum"));
    _vendtype->setId(getq.value("vend_vendtype_id").toInt());
    _active->setChecked(getq.value("vend_active").toBool());
    _name->setText(getq.value("vend_name"));
    _contact1->setId(getq.value("vend_cntct1_id").toInt());
    _contact1->setSearchAcct(_crmacctid);
    _contact2->setId(getq.value("vend_cntct2_id").toInt());
    _contact2->setSearchAcct(_crmacctid);
    _address->setId(getq.value("vend_addr_id").toInt());
    _defaultTerms->setId(getq.value("vend_terms_id").toInt());
    _defaultShipVia->setText(getq.value("vend_shipvia").toString());
    _defaultCurr->setId(getq.value("vend_curr_id").toInt());
    _poItems->setChecked(getq.value("vend_po").toBool());
    _restrictToItemSource->setChecked(getq.value("vend_restrictpurch").toBool());
    _receives1099->setChecked(getq.value("vend_1099").toBool());
    _match->setChecked(getq.value("vend_match").toBool());
    _qualified->setChecked(getq.value("vend_qualified").toBool());
    _notes->setText(getq.value("vend_comments").toString());
    _poComments->setText(getq.value("vend_pocomments").toString());

    _taxzone->setId(getq.value("vend_taxzone_id").toInt());

    if (getq.value("vend_fobsource").toString() == "V")
    {
      _useVendorFOB->setChecked(TRUE);
      _vendorFOB->setText(getq.value("vend_fob"));
    }
    else
      _useWarehouseFOB->setChecked(TRUE);

    _achGroup->setChecked(getq.value("vend_ach_enabled").toBool());
    _routingNumber->setText(getq.value("routingnum").toString());
    _achAccountNumber->setText(getq.value("accntnum").toString());
    _useACHSpecial->setChecked(! getq.value("vend_ach_use_vendinfo").toBool());
    _individualId->setText(getq.value("vend_ach_indiv_number").toString());
    _individualName->setText(getq.value("vend_ach_indiv_name").toString());

    _accountType->setCode(getq.value("vend_ach_accnttype").toString());

    _account->setId(getq.value("vend_accnt_id").toInt());
    if(getq.value("vend_expcat_id").toInt() != -1)
    {
      _expcatSelected->setChecked(TRUE);
      _expcat->setId(getq.value("vend_expcat_id").toInt());
    }
    if(getq.value("vend_tax_id").toInt() != -1)
    {
      _taxSelected->setChecked(TRUE);
      _taxCode->setId(getq.value("vend_tax_id").toInt());
    }

    sFillAddressList();
    sFillTaxregList();

    _comments->setId(_crmacctid);
    _address->setSearchAcct(_crmacctid);

    emit newId(_vendid);
  }
  else if (ErrorReporter::error(QtCriticalMsg, this, tr("Getting Vendor"),
                                getq, __FILE__, __LINE__))
    return false;
  else
  {
    ErrorReporter::error(QtCriticalMsg, this, tr("Getting Vendor"),
                         tr("Could not find the Vendor information. Perhaps "
                            "the Vendor and CRM Account have been disconnected."),
                __FILE__, __LINE__);
    return false;
  }

  _crmacct->setEnabled(_crmacctid > 0 &&
                       (_privileges->check("MaintainAllCRMAccounts") ||
                        _privileges->check("ViewAllCRMAccounts") ||
                        (omfgThis->username() == _crmowner && _privileges->check("MaintainPersonalCRMAccounts")) ||
                        (omfgThis->username() == _crmowner && _privileges->check("ViewPersonalCRMAccounts"))));

  emit populated();
  return true;
}

void vendor::sPrintAddresses()
{
  ParameterList params;
  params.append("vend_id", _vendid);

  orReport report("VendorAddressList", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void vendor::sNewAddress()
{
  ParameterList params;
  params.append("mode", "new");
  params.append("vend_id", _vendid);

  vendorAddress newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillAddressList();
}

void vendor::sEditAddress()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("vendaddr_id", _vendaddr->id());

  vendorAddress newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillAddressList();
}

void vendor::sViewAddress()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("vendaddr_id", _vendaddr->id());

  vendorAddress newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void vendor::sDeleteAddress()
{
  XSqlQuery delq;
  delq.prepare( "DELETE FROM vendaddrinfo "
             "WHERE (vendaddr_id=:vendaddr_id);" );
  delq.bindValue(":vendaddr_id", _vendaddr->id());
  delq.exec();
  sFillAddressList();
  if (ErrorReporter::error(QtCriticalMsg, this, tr("Getting Vendor Addresses"),
                           delq, __FILE__, __LINE__))
    return;
}

void vendor::sFillAddressList()
{
  XSqlQuery addrq;
  addrq.prepare("SELECT vendaddr_id, vendaddr_code, vendaddr_name,"
                "       addr_city,   addr_state,    addr_country,"
                "       addr_postalcode"
                " FROM vendaddrinfo"
                " LEFT OUTER JOIN addr ON (vendaddr_addr_id=addr_id)"
                " WHERE (vendaddr_vend_id=:vend_id)"
                " ORDER BY vendaddr_code;" );
  addrq.bindValue(":vend_id", _vendid);
  addrq.exec();
  _vendaddr->populate(addrq);
  if (ErrorReporter::error(QtCriticalMsg, this, tr("Getting Vendor Addresses"),
                           addrq, __FILE__, __LINE__))
    return;
}

void vendor::sFillTaxregList()
{
  XSqlQuery taxreg;
  taxreg.prepare("SELECT taxreg_id, taxreg_taxauth_id,"
                 "       taxauth_code, taxzone_code, taxreg_number"
                 "  FROM taxreg"
                 "  JOIN taxauth ON (taxreg_taxauth_id=taxauth_id)"
                 "  LEFT OUTER JOIN taxzone ON (taxreg_taxzone_id=taxzone_id)"
                 " WHERE ((taxreg_rel_type='V') "
                 "    AND (taxreg_rel_id=:vend_id));");
  taxreg.bindValue(":vend_id", _vendid);
  taxreg.exec();
  _taxreg->clear();
  _taxreg->populate(taxreg, true);
  if (ErrorReporter::error(QtCriticalMsg, this, tr("Getting Tax Registrations"),
                           taxreg, __FILE__, __LINE__))
    return;
}

void vendor::sNewTaxreg()
{
  ParameterList params;
  params.append("mode", "new");
  params.append("taxreg_rel_id", _vendid);
  params.append("taxreg_rel_type", "V");

  taxRegistration newdlg(this, "", TRUE);
  if (newdlg.set(params) == NoError && newdlg.exec() != XDialog::Rejected)
    sFillTaxregList();
}

void vendor::sEditTaxreg()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("taxreg_id", _taxreg->id());

  taxRegistration newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.set(params) == NoError && newdlg.exec() != XDialog::Rejected)
    sFillTaxregList();
}

void vendor::sViewTaxreg()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("taxreg_id", _taxreg->id());

  taxRegistration newdlg(this, "", TRUE);
  if (newdlg.set(params) == NoError)
    newdlg.exec();
}

void vendor::sDeleteTaxreg()
{
  XSqlQuery delq;
  delq.prepare("DELETE FROM taxreg WHERE (taxreg_id=:taxreg_id);");
  delq.bindValue(":taxreg_id", _taxreg->id());
  delq.exec();
  if (ErrorReporter::error(QtCriticalMsg, this, tr("Deleting Tax Registration"),
                           delq, __FILE__, __LINE__))
    return;

  sFillTaxregList();
}

void vendor::sNext()
{
  // Find Next
  q.prepare("SELECT vend_id "
            "  FROM vendinfo"
            " WHERE (:number < vend_number)"
            " ORDER BY vend_number"
            " LIMIT 1;");
  q.bindValue(":number", _number->text());
  q.exec();
  if(!q.first())
  {
    QMessageBox::information(this, tr("At Last Record"),
       tr("You are already on the last record.") );
    return;
  }
  int newid = q.value("vend_id").toInt();

  if(!sCheckSave())
    return;

  clear();

  _vendid = newid;
  sPopulate();
}

void vendor::sPrevious()
{
  XSqlQuery nextq;
  nextq.prepare("SELECT vend_id "
            "  FROM vendinfo"
            " WHERE (:number > vend_number)"
            " ORDER BY vend_number DESC"
            " LIMIT 1;");
  nextq.bindValue(":number", _number->text());
  nextq.exec();
  if (!nextq.first() && nextq.lastError().type() == QSqlError::NoError)
  {
    QMessageBox::information(this, tr("At First Record"),
                             tr("You are already on the first record.") );
    return;
  }
  else if (ErrorReporter::error(QtCriticalMsg, this, tr("Getting Vendor"),
                                nextq, __FILE__, __LINE__))
    return;
  int newid = q.value("vend_id").toInt();

  if(!sCheckSave())
    return;

  clear();

  _vendid = newid;
  sPopulate();
}

bool vendor::sCheckSave()
{
  if(cEdit == _mode)
  {
    switch(QMessageBox::question(this, tr("Save Changes?"),
         tr("Would you like to save any changes before continuing?"), QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel))
    {
      case QMessageBox::Yes:
        _ignoreClose = true;
        sSave();
        _ignoreClose = false;
        break;
      case QMessageBox::No:
        break;
      case QMessageBox::Cancel:
      default:
        return false;
    };
  }
  return true;
}

void vendor::clear()
{
  _cachedNumber = QString::null;
  _crmacctid = -1;
  _vendid = -1;

  _active->setChecked(true);
  _poItems->setChecked(false);
  _restrictToItemSource->setChecked(false);
  _qualified->setChecked(false);
  _match->setChecked(false);
  _receives1099->setChecked(false);

  _vendtype->setId(-1);
  _defaultTerms->setId(-1);
  _defaultCurr->setCurrentIndex(0);
  _taxzone->setId(-1);

  _useWarehouseFOB->setChecked(true);

  _number->clear();
  _name->clear();
  _accountNumber->clear();
  _defaultShipVia->clear();
  _vendorFOB->clear();
  _notes->clear();
  _poComments->clear();

  _address->clear();
  _contact1->clear();
  _contact2->clear();

  _vendaddr->clear();
  _taxreg->clear();

  _achGroup->setChecked(false);
  _routingNumber->clear();
  _achAccountNumber->clear();
  _individualId->clear();
  _individualName->clear();
  _accountType->setCurrentIndex(0);

  _comments->setId(-1);
  _tabs->setCurrentIndex(0);
}

void vendor::closeEvent(QCloseEvent *pEvent)
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

void vendor::sHandleButtons()
{
  if (_mainButton->isChecked())
    _addressStack->setCurrentIndex(0);
  else
    _addressStack->setCurrentIndex(1);

  if (_checksButton->isChecked())
    _transmitStack->setCurrentIndex(1);
  else
    _transmitStack->setCurrentIndex(1);
}

void vendor::sNumberEdited()
{
  _notice = TRUE;
  _number->setText(_number->text().toUpper());
}

void vendor::sCrmAccount()
{
  ParameterList params;
  params.append("crmacct_id", _crmacctid);
  if ((cView == _mode && _privileges->check("ViewAllCRMAccounts")) ||
      (cView == _mode && _privileges->check("ViewPersonalCRMAccounts")
                      && omfgThis->username() == _crmowner) ||
      (cEdit == _mode && _privileges->check("ViewAllCRMAccounts")
                      && ! _privileges->check("MaintainAllCRMAccounts")) ||
      (cEdit == _mode && _privileges->check("ViewPersonalCRMAccounts")
                      && ! _privileges->check("MaintainPersonalCRMAccounts")
                      && omfgThis->username() == _crmowner))
    params.append("mode", "view");
  else if ((cEdit == _mode && _privileges->check("MaintainAllCRMAccounts")) ||
           (cEdit == _mode && _privileges->check("MaintainPersonalCRMAccounts")
                           && omfgThis->username() == _crmowner))
    params.append("mode", "edit");
  else if ((cNew == _mode && _privileges->check("MaintainAllCRMAccounts")) ||
           (cNew == _mode && _privileges->check("MaintainPersonalCRMAccounts")
                          && omfgThis->username() == _crmowner))
    params.append("mode", "edit");
  else
  {
    qWarning("tried to open CRM Account window without privilege");
    return;
  }

  crmaccount *newdlg = new crmaccount();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}
