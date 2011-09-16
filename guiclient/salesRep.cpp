/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "salesRep.h"

#include <QSqlError>
#include <QValidator>
#include <QVariant>

#include "crmaccount.h"
#include "employee.h"
#include "errorReporter.h"
#include "guiErrorCheck.h"

salesRep::salesRep(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_buttonBox,       SIGNAL(accepted()), this, SLOT(sSave()));
  connect(_crmaccountButton, SIGNAL(clicked()), this, SLOT(sCrmaccount()));
  connect(_number,         SIGNAL(lostFocus()), this, SLOT(sCheck()));

  _commPrcnt->setValidator(omfgThis->percentVal());

  _mode = cView;
  _crmacctid  = -1;
  _empid      = -1;
  _salesrepid = -1;
  _NumberGen  = -1;
}

salesRep::~salesRep()
{
  // no need to delete child widgets, Qt does it all for us
}

void salesRep::languageChange()
{
  retranslateUi(this);
}

enum SetResponse salesRep::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("crmacct_id", &valid);
  if (valid)
    _crmacctid = param.toInt();

  param = pParams.value("salesrep_id", &valid);
  if (valid)
    _salesrepid = param.toInt();

  if (_salesrepid > 0 || _crmacctid > 0)
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
      else
        _number->setFocus();
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;
      _name->setFocus();
    }
    else if (param.toString() == "view")
    {
      _mode = cView;

      _buttonBox->clear();
      _buttonBox->addButton(QDialogButtonBox::Close);
      _buttonBox->setFocus();
    }
  }

  bool canEdit = (cNew == _mode || cEdit == _mode);
  _number->setEnabled(canEdit);
  _name->setEnabled(canEdit);
  _active->setEnabled(canEdit);
  _commPrcnt->setEnabled(canEdit);

  return NoError;
}

void salesRep::sCheck()
{
  _number->setText(_number->text().trimmed());
  if ((_mode == cNew) && (_number->text().length()))
  {
    if (cNew == _mode && -1 != _NumberGen && _number->text().toInt() != _NumberGen)
    {
      XSqlQuery query;
      query.prepare("SELECT releaseCRMAccountNumber(:Number);");
      query.bindValue(":Number", _NumberGen);
      query.exec();
      _NumberGen = -1;
    }

    XSqlQuery getq;
    getq.prepare( "SELECT salesrep_id "
               "FROM salesrep "
               "WHERE (UPPER(salesrep_number)=UPPER(:salesrep_number));" );
    getq.bindValue(":salesrep_number", _number->text());
    getq.exec();
    if (getq.first())
    {
      _salesrepid = getq.value("salesrep_id").toInt();
      _mode = cEdit;
      sPopulate();

      _number->setEnabled(FALSE);
    }
    else if (ErrorReporter::error(QtCriticalMsg, this,
                                  tr("Error checking for existing Sales Rep"),
                                  getq, __FILE__, __LINE__))
      return;
  }
}

bool salesRep::save()
{
  QList<GuiErrorCheck> errors;
  errors << GuiErrorCheck(_number->text().isEmpty(), _number,
                          tr("You must enter a Number for this Sales Rep."))
         << GuiErrorCheck(! _commPrcnt->isValid() && _commPrcnt->text().isEmpty(),
                          _commPrcnt,
                          tr("You must enter a Commission Rate for this Sales Rep."))
  ;
  if (GuiErrorCheck::reportErrors(this, tr("Cannot Save Sales Rep"), errors))
    return false;

  XSqlQuery saveq;
  if (_mode == cNew)
    saveq.prepare( "INSERT INTO salesrep ("
                   "  salesrep_number,  salesrep_active,"
                   "  salesrep_name,    salesrep_commission"
                   ") VALUES ("
                   "  :salesrep_number, :salesrep_active,"
                   "  :salesrep_name,   :salesrep_commission"
                   ") RETURNING salesrep_id;" );
  else if (_mode == cEdit)
  {
    saveq.prepare( "UPDATE salesrep "
                   "SET salesrep_active=:salesrep_active, "
                   "    salesrep_number=:salesrep_number,"
                   "    salesrep_name=:salesrep_name, "
                   "    salesrep_commission=:salesrep_commission "
                   "WHERE (salesrep_id=:salesrep_id) "
                   "RETURNING salesrep_id;" );
    saveq.bindValue(":salesrep_id", _salesrepid);
  }

  saveq.bindValue(":salesrep_number",     _number->text());
  saveq.bindValue(":salesrep_name",       _name->text());
  saveq.bindValue(":salesrep_commission", (_commPrcnt->toDouble() / 100));
  saveq.bindValue(":salesrep_active",     QVariant(_active->isChecked()));
  saveq.exec();
  if (saveq.first())
    _salesrepid = saveq.value("salesrep_id").toInt();
  else if (ErrorReporter::error(QtCriticalMsg, this,
                                tr("Error saving Sales Rep"),
                                saveq, __FILE__, __LINE__))
    return false;

  XSqlQuery crmq;
  crmq.prepare("SELECT crmacct_id, crmacct_emp_id"
               "  FROM crmacct"
               " WHERE crmacct_salesrep_id=:id;");
  crmq.bindValue(":id", _salesrepid);
  crmq.exec();
  if (crmq.first())
  {
    _crmacctid = crmq.value("crmacct_id").toInt();
    _empid     = crmq.value("crmacct_emp_id").toInt();
  }
  else if (ErrorReporter::error(QtCriticalMsg, this, tr("Database Error"),
                                crmq, __FILE__, __LINE__))
    return false;
  else if (ErrorReporter::error(QtCriticalMsg, this, tr("CRM Account Error"),
                          tr("<p>The Sales Rep should now have a CRM Account, "
                             "but that CRM Account could not be found. Please "
                             "check the database server log for errors.")))
    return false;

  omfgThis->sSalesRepUpdated(_salesrepid);
  return true;
}

void salesRep::sSave()
{
  if (save())
    done(_salesrepid);
}

void salesRep::sCrmaccount()
{
  if (cNew == _mode || cEdit == _mode)
    if (! save())
      return;

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
  params.append("modal");

  crmaccount *newdlg = new crmaccount(this);
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

bool salesRep::sPopulate()
{
  XSqlQuery getq;
  if (_salesrepid > 0)
  {
    getq.prepare("SELECT salesrep_number AS number, salesrep_active AS active,"
                 "       salesrep_name   AS name,   salesrep_commission AS comm,"
                 "       crmacct_id,                crmacct_emp_id,"
                 "       crmacct_owner_username"
                 "  FROM salesrep"
                 "  JOIN crmacct ON (salesrep_id=crmacct_salesrep_id)"
                 " WHERE (salesrep_id=:id);" );
    getq.bindValue(":id", _salesrepid);
  }
  else if (_crmacctid > 0)
  {
    getq.prepare("SELECT crmacct_number AS number, crmacct_active AS active,"
                 "       crmacct_name   AS name,   NULL AS comm,"
                 "       crmacct_id,               crmacct_emp_id,"
                 "       crmacct_owner_username"
                 "  FROM crmacct"
                 " WHERE (crmacct_id=:id);" );
    getq.bindValue(":id", _crmacctid);
  }

  getq.exec();
  if (getq.first())
  {
    _number->setText(getq.value("number").toString());
    _active->setChecked(getq.value("active").toBool());
    _name->setText(getq.value("name").toString());
    QVariant commission = getq.value("comm");
    if (! commission.isNull())
      _commPrcnt->setDouble(commission.toDouble() * 100);
    _crmacctid = getq.value("crmacct_id").toInt();
    _empid     = getq.value("crmacct_emp_id").toInt();
    _crmowner  = getq.value("crmacct_owner_username").toString();

    _number->setEnabled(FALSE);
  }
  else if (ErrorReporter::error(QtCriticalMsg, this, tr("Database Error"),
                                getq, __FILE__, __LINE__))
    return false;

  _crmaccountButton->setEnabled(_crmacctid > 0 &&
                                (_privileges->check("MaintainAllCRMAccounts") ||
                                 _privileges->check("ViewAllCRMAccounts") ||
                                (omfgThis->username() == _crmowner && _privileges->check("MaintainPersonalCRMAccounts")) ||
                                (omfgThis->username() == _crmowner && _privileges->check("ViewPersonalCRMAccounts"))));
  return true;
}
