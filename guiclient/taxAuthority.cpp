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

#include <QVariant>
#include <QMessageBox>
#include <QSqlError>
#include <QCloseEvent>

#include "storedProcErrorLookup.h"

/*
 *  Constructs a taxAuthority as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
taxAuthority::taxAuthority(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  // signals and slots connections
  connect(_save, SIGNAL(clicked()), this, SLOT(sSave()));
  connect(_code, SIGNAL(lostFocus()), this, SLOT(sCheck()));
  connect(_crmacct, SIGNAL(newId(int)), _address,  SLOT(setSearchAcct(int)));

  _crmacct->setId(-1);
  _NumberGen = -1;
}

/*
 *  Destroys the object and frees any allocated resources
 */
taxAuthority::~taxAuthority()
{
  // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void taxAuthority::languageChange()
{
  retranslateUi(this);
}

enum SetResponse taxAuthority::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("crmacct_number", &valid);
  if (valid)
    _code->setText(param.toString());

  param = pParams.value("crmacct_name", &valid);
  if (valid)
    _name->setText(param.toString());

  param = pParams.value("crmacct_id", &valid);
  if (valid)
  {
    _crmacct->setId(param.toInt());
  }

  param = pParams.value("taxauth_id", &valid);
  if (valid)
  {
    _taxauthid = param.toInt();
    populate();
  }

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
        q.exec("SELECT fetchCRMAccountNumber() AS number;");
        if (q.first())
        {
          _code->setText(q.value("number"));
          _NumberGen = q.value("number").toInt();
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

      _code->setEnabled(FALSE);
      _name->setEnabled(FALSE);
      _extref->setEnabled(FALSE);
      _currency->setEnabled(FALSE);
      _address->setEnabled(FALSE);
      _county->setEnabled(FALSE);
      _close->setText(tr("&Close"));
      _save->hide();

      _close->setFocus();
    }
  }

  if(_metrics->value("CRMAccountNumberGeneration") == "A")
    _code->setEnabled(FALSE);

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

    q.prepare( "SELECT taxauth_id "
               "FROM taxauth "
               "WHERE (UPPER(taxauth_code)=UPPER(:taxauth_code));" );
    q.bindValue(":taxauth_code", _code->text());
    q.exec();
    if (q.first())
    {
      _taxauthid = q.value("taxauth_id").toInt();
      _mode = cEdit;
      populate();

      _code->setEnabled(FALSE);
    }
  }
}

void taxAuthority::sSave()
{
  if (_code->text().length() == 0)
  {
      QMessageBox::warning( this, tr("Cannot Save Tax Authority"),
                            tr("You must enter a valid Code.") );
      return;
  }
  
  if (_mode == cEdit)
  {
    q.prepare( "SELECT taxauth_id "
               "FROM taxauth "
               "WHERE ( (taxauth_id<>:taxauth_id)"
               " AND (UPPER(taxauth_code)=UPPER(:taxauth_code)) );");
    q.bindValue(":taxauth_id", _taxauthid);
  }
  else
  {
    q.prepare( "SELECT taxauth_id "
               "FROM taxauth "
               "WHERE (taxauth_code=:taxauth_code);");
  }
  q.bindValue(":taxauth_code", _code->text().trimmed());
  q.exec();
  if (q.first())
  {
    QMessageBox::critical( this, tr("Cannot Create Tax Authority"),
			   tr( "A Tax Authority with the entered code already exists."
			       "You may not create a Tax Authority with this code." ) );
    _code->setFocus();
    return;
  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  XSqlQuery rollback;
  rollback.prepare("ROLLBACK;");

  q.exec("BEGIN;");

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
  if (saveResult < 0)	// not else-if: this is error check for CHANGE{ONE,ALL}
  {
    rollback.exec();
    systemError(this, tr("<p>There was an error saving this address (%1). "
			 "Check the database server log for errors.")
		      .arg(saveResult), __FILE__, __LINE__);
    _address->setFocus();
    return;
  }

  if (_mode == cEdit)
  {
    q.prepare( "UPDATE taxauth "
               "   SET taxauth_code=:taxauth_code,"
               "       taxauth_name=:taxauth_name,"
               "       taxauth_extref=:taxauth_extref,"
               "       taxauth_curr_id=:taxauth_curr_id,"
               "       taxauth_addr_id=:taxauth_addr_id,"
	       "       taxauth_county=:taxauth_county,"
	       "       taxauth_accnt_id=:taxauth_accnt_id "
               "WHERE (taxauth_id=:taxauth_id);" );
  }
  else if (_mode == cNew)
  {
    q.exec("SELECT NEXTVAL('taxauth_taxauth_id_seq') AS taxauth_id;");
    if (q.first())
      _taxauthid = q.value("taxauth_id").toInt();
    else if (q.lastError().type() != QSqlError::NoError)
    {
      rollback.exec();
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }

    q.prepare( "INSERT INTO taxauth "
               "( taxauth_id, taxauth_code, taxauth_name, taxauth_extref,"
	       "  taxauth_curr_id, taxauth_addr_id, taxauth_county,"
	       "  taxauth_accnt_id) "
               "VALUES "
               "( :taxauth_id, :taxauth_code, :taxauth_name, :taxauth_extref,"
	       "  :taxauth_curr_id, :taxauth_addr_id, :taxauth_county,"
	       "  :taxauth_accnt_id);" );
  }
  q.bindValue(":taxauth_id", _taxauthid);
  q.bindValue(":taxauth_code", _code->text().trimmed());
  q.bindValue(":taxauth_name", _name->text());
  q.bindValue(":taxauth_extref", _extref->text());
  if(_currency->isValid())
    q.bindValue(":taxauth_curr_id", _currency->id());
  if(_address->isValid())
    q.bindValue(":taxauth_addr_id", _address->id());
  q.bindValue(":taxauth_county",    _county->text());
  if(_glaccnt->isValid())
    q.bindValue(":taxauth_accnt_id", _glaccnt->id());
  q.exec();
  if (q.lastError().type() != QSqlError::NoError)
  {
    rollback.exec();
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  if (_crmacct->id() > 0)
  {
    q.prepare("UPDATE crmacct SET crmacct_taxauth_id = :taxauth_id "
	      "WHERE (crmacct_id=:crmacct_id);");
    q.bindValue(":taxauth_id",	_taxauthid);
    q.bindValue(":crmacct_id",	_crmacct->id());
    q.exec();
    if (q.lastError().type() != QSqlError::NoError)
    {
	rollback.exec();
	systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
	return;
    }
  }
  else
  {
    q.prepare( "SELECT createCrmAcct(:number, :name, :active, :type, NULL, "
	       "      NULL, NULL, NULL, NULL, :taxauthid, NULL, NULL) AS crmacctid;");
    q.bindValue(":number",	_code->text().trimmed());
    q.bindValue(":name",	_name->text().trimmed());
    q.bindValue(":active",	true);
    q.bindValue(":type",	"O");	// TODO - when will this be "I"?
    q.bindValue(":taxauthid",	_taxauthid);
    q.exec();
    if (q.first())
    {
      int crmacctid = q.value("crmacctid").toInt();
      if (crmacctid <= 0)
      {
	rollback.exec();
	systemError(this, storedProcErrorLookup("createCrmAcct", crmacctid),
		    __FILE__, __LINE__);
	return;
      }
      _crmacct->setId(crmacctid);
    }
    else if (q.lastError().type() != QSqlError::NoError)
    {
      rollback.exec();
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }

  q.exec("COMMIT;");
  _NumberGen = -1;

  omfgThis->sTaxAuthsUpdated(_taxauthid);

  done(_taxauthid);
}

void taxAuthority::populate()
{
  q.prepare( "SELECT taxauth_code, taxauth_name,"
             "       taxauth_extref,"
             "       COALESCE(taxauth_curr_id,-1) AS curr_id,"
             "       COALESCE(taxauth_addr_id,-1) AS addr_id,"
	     "       taxauth_county, crmacct_id, taxauth_accnt_id "
             "FROM taxauth LEFT OUTER JOIN "
	     "     crmacct ON (crmacct_taxauth_id=taxauth_id) "
             "WHERE (taxauth_id=:taxauth_id);" );
  q.bindValue(":taxauth_id", _taxauthid);
  q.exec();
  if (q.first())
  {
    _code->setText(q.value("taxauth_code").toString());
    _name->setText(q.value("taxauth_name").toString());
    _extref->setText(q.value("taxauth_extref").toString());
    _currency->setId(q.value("curr_id").toInt());
    _address->setId(q.value("addr_id").toInt());
    if (! q.value("crmacct_id").isNull())
      _crmacct->setId(q.value("crmacct_id").toInt());
    _county->setText(q.value("taxauth_county").toString());
    _glaccnt->setId(q.value("taxauth_accnt_id").toInt());
  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
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
