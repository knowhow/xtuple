/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "taxAuthority.h"

#include <QCloseEvent>
#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include "storedProcErrorLookup.h"

#include "crmaccount.h"
#include "errorReporter.h"
#include "guiErrorCheck.h"

taxAuthority::taxAuthority(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_code,    SIGNAL(lostFocus()), this,     SLOT(sCheck()));
  connect(_crmacct, SIGNAL(clicked()),   this,     SLOT(sCrmAccount()));
  connect(_save,    SIGNAL(clicked()),   this,     SLOT(sSave()));

  _crmacctid = -1;
  _mode      = cView;
  _taxauthid = -1;
  _NumberGen = -1;
}

taxAuthority::~taxAuthority()
{
  // no need to delete child widgets, Qt does it all for us
}

void taxAuthority::languageChange()
{
  retranslateUi(this);
}

enum SetResponse taxAuthority::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("crmacct_id", &valid);
  if (valid)
    _crmacctid = param.toInt();

  param = pParams.value("taxauth_id", &valid);
  if (valid)
    _taxauthid = param.toInt();
  
  if (_taxauthid > 0 || _crmacctid > 0)
    if (! sPopulate())
      return UndefinedError;

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      _mode = cNew;

      if(((_metrics->value("CRMAccountNumberGeneration") == "A") ||
          (_metrics->value("CRMAccountNumberGeneration") == "O"))
       && _code->text().isEmpty() )
      {
        XSqlQuery numq;
        numq.exec("SELECT fetchCRMAccountNumber() AS number;");
        if (numq.first())
        {
          _code->setText(numq.value("number"));
          _NumberGen = numq.value("number").toInt();
        }
      }
      else
        _code->setFocus();
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;
      _save->setFocus();
    }
    else if (param.toString() == "view")
    {
      _mode = cView;

      _close->setText(tr("&Close"));
      _save->hide();

      _close->setFocus();
    }
  }

  bool canEdit = (cNew == _mode || cEdit == _mode);
  _code->setEnabled(FALSE);

  _code->setEnabled(canEdit &&
                    _metrics->value("CRMAccountNumberGeneration") != "A");
  _address->setEnabled(canEdit);
  _county->setEnabled(canEdit);
  // _crmacct is handled in sPopulate()
  _currency->setEnabled(canEdit);
  _extref->setEnabled(canEdit);
  _glaccnt->setReadOnly(! canEdit);
  _name->setEnabled(canEdit);

  return NoError;
}

void taxAuthority::sCheck()
{
  _code->setText(_code->text().trimmed());
  if ( (_mode == cNew) && (_code->text().length()) )
  {
    if(cNew == _mode && -1 != _NumberGen && _code->text().toInt() != _NumberGen)
    {
      XSqlQuery query;
      query.prepare( "SELECT releaseCRMAccountNumber(:Number);" );
      query.bindValue(":Number", _NumberGen);
      query.exec();
      _NumberGen = -1;
    }

    XSqlQuery getq;
    getq.prepare( "SELECT taxauth_id "
               "FROM taxauth "
               "WHERE (UPPER(taxauth_code)=UPPER(:taxauth_code));" );
    getq.bindValue(":taxauth_code", _code->text());
    getq.exec();
    if (getq.first())
    {
      _taxauthid = getq.value("taxauth_id").toInt();
      _mode = cEdit;
      sPopulate();

      _code->setEnabled(FALSE);
    }
    else if (ErrorReporter::error(QtCriticalMsg, this,
                                  tr("Error checking for existing Tax Authority"),
                                  getq, __FILE__, __LINE__))
      return;
  }
}

bool taxAuthority::sSave()
{
  QList<GuiErrorCheck> errors;
  errors << GuiErrorCheck(_code->text().isEmpty(), _code,
                          tr("You must enter a valid Code."))
    ;

  XSqlQuery dupq;
  dupq.prepare("SELECT taxauth_id "
               "FROM taxauth "
               "WHERE ( (taxauth_id<>:taxauth_id)"
               " AND (UPPER(taxauth_code)=UPPER(:taxauth_code)) );");
  dupq.bindValue(":taxauth_id", _taxauthid);
  dupq.bindValue(":taxauth_code", _code->text().trimmed());
  dupq.exec();
  if (dupq.first())
    errors << GuiErrorCheck(true, _code,
                            tr("A Tax Authority with the entered code already "
                               "exists. You may not create a Tax Authority "
                               "with this code."));
  else if (ErrorReporter::error(QtCriticalMsg, this,
                                tr("Cannot Create Tax Authority"),
                                dupq, __FILE__, __LINE__))
    return false;

  if (GuiErrorCheck::reportErrors(this, tr("Cannot Save Tax Authority"), errors))
    return false;

  XSqlQuery rollback;
  rollback.prepare("ROLLBACK;");

  XSqlQuery begin("BEGIN;");

  int saveResult = _address->save(AddressCluster::CHECK);
  if (-2 == saveResult)
  {
    int answer = QMessageBox::question(this,
                   tr("Question Saving Address"),
                   tr("<p>There are multiple uses of this "
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
  if (saveResult < 0)  // not else-if: this is error check for CHANGE{ONE,ALL}
  {
    rollback.exec();
    ErrorReporter::error(QtCriticalMsg, this, tr("Error saving Address"),
                          storedProcErrorLookup("saveAddr", saveResult),
                          __FILE__, __LINE__);
    _address->setFocus();
    return false;
  }

  XSqlQuery upsq;
  if (_mode == cEdit)
  {
    upsq.prepare( "UPDATE taxauth "
               "   SET taxauth_code=:taxauth_code,"
               "       taxauth_name=:taxauth_name,"
               "       taxauth_extref=:taxauth_extref,"
               "       taxauth_curr_id=:taxauth_curr_id,"
               "       taxauth_addr_id=:taxauth_addr_id,"
               "       taxauth_county=:taxauth_county,"
               "       taxauth_accnt_id=:taxauth_accnt_id "
               "WHERE (taxauth_id=:taxauth_id) "
               "RETURNING taxauth_id;" );
    upsq.bindValue(":taxauth_id", _taxauthid);
  }
  else if (_mode == cNew)
    upsq.prepare( "INSERT INTO taxauth "
               "( taxauth_id, taxauth_code, taxauth_name, taxauth_extref,"
               "  taxauth_curr_id, taxauth_addr_id, taxauth_county,"
               "  taxauth_accnt_id) "
               "VALUES "
               "( DEFAULT, :taxauth_code, :taxauth_name, :taxauth_extref,"
               "  :taxauth_curr_id, :taxauth_addr_id, :taxauth_county,"
               "  :taxauth_accnt_id) "
               "RETURNING taxauth_id;" );

  upsq.bindValue(":taxauth_code", _code->text().trimmed());
  upsq.bindValue(":taxauth_name", _name->text());
  upsq.bindValue(":taxauth_extref", _extref->text());
  if(_currency->isValid())
    upsq.bindValue(":taxauth_curr_id", _currency->id());
  if(_address->isValid())
    upsq.bindValue(":taxauth_addr_id", _address->id());
  upsq.bindValue(":taxauth_county",    _county->text());
  if(_glaccnt->isValid())
    upsq.bindValue(":taxauth_accnt_id", _glaccnt->id());
  upsq.exec();
  if (upsq.first())
    _taxauthid = upsq.value("taxauth_id").toInt();
  else if (upsq.lastError().type() != QSqlError::NoError)
  {
    rollback.exec();
    ErrorReporter::error(QtCriticalMsg, this, tr("Cannot save Tax Authority"),
                         upsq, __FILE__, __LINE__);
    return false;
  }

  XSqlQuery commit("COMMIT;");
  _NumberGen = -1;

  omfgThis->sTaxAuthsUpdated(_taxauthid);

  done(_taxauthid);
  return true;
}

bool taxAuthority::sPopulate()
{
  XSqlQuery getq;
  if (_taxauthid > 0)
  {
    getq.prepare("SELECT taxauth_code, taxauth_name,"
                 "       COALESCE(taxauth_curr_id,-1) AS curr_id,"
                 "       COALESCE(taxauth_addr_id,-1) AS addr_id,"
                 "       taxauth_county, taxauth_accnt_id,"
                 "       taxauth_extref, crmacct_id, crmacct_owner_username"
                 "  FROM taxauth"
                 "  JOIN crmacct ON (crmacct_taxauth_id=taxauth_id)"
                 " WHERE (taxauth_id=:id);");
    getq.bindValue(":id", _taxauthid);
  }
  else if (_crmacctid > 0)
  {
    getq.prepare("SELECT crmacct_number AS taxauth_code,"
                 "       crmacct_name   AS taxauth_name,"
                 "       -1 AS curr_id, -1 AS addr_id,"
                 "       NULL AS taxauth_county, NULL AS taxauth_accnt_id,"
                 "       NULL AS taxauth_extref, crmacct_id, crmacct_owner_username"
                 "  FROM crmacct"
                 " WHERE (crmacct_id=:id);");
    getq.bindValue(":id", _crmacctid);
  }

  getq.exec();
  if (getq.first())
  {
    _code->setText(getq.value("taxauth_code").toString());
    _name->setText(getq.value("taxauth_name").toString());
    _extref->setText(getq.value("taxauth_extref").toString());
    _currency->setId(getq.value("curr_id").toInt());
    _address->setId(getq.value("addr_id").toInt());
    _crmacctid = getq.value("crmacct_id").toInt();
    _crmowner = getq.value("crmacct_owner_username").toString();
    _county->setText(getq.value("taxauth_county").toString());
    _glaccnt->setId(getq.value("taxauth_accnt_id").toInt());
  }
  else if (ErrorReporter::error(QtCriticalMsg, this, tr("Database Error"),
                                getq, __FILE__, __LINE__))
    return false;
  
  _address->setSearchAcct(_crmacctid);

  _crmacct->setEnabled(_crmacctid > 0 &&
                       (_privileges->check("MaintainAllCRMAccounts") ||
                        _privileges->check("ViewAllCRMAccounts") ||
                        (omfgThis->username() == _crmowner && _privileges->check("MaintainPersonalCRMAccounts")) ||
                        (omfgThis->username() == _crmowner && _privileges->check("ViewPersonalCRMAccounts"))));

  return true;
}

void taxAuthority::done( int r )
{
  XDialog::done( r );
  close();
}

void taxAuthority::closeEvent(QCloseEvent *pEvent)
{
  if(cNew == _mode && -1 != _NumberGen)
  {
    XSqlQuery query;
    query.prepare( "SELECT releaseCRMAccountNumber(:Number);" );
    query.bindValue(":Number", _NumberGen);
    query.exec();
    _NumberGen = -1;
  }
  XDialog::closeEvent(pEvent);
}

void taxAuthority::sCrmAccount()
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
  omfgThis->handleNewWindow(newdlg, Qt::WindowModal);
}
