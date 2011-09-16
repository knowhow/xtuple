/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "vendorAddress.h"

#include <QMessageBox>
#include <QSqlError>
#include <QVariant>
#include <vendorcluster.h>

vendorAddress::vendorAddress(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_save, SIGNAL(clicked()), this, SLOT(sSave()));
  connect(_number, SIGNAL(lostFocus()), this, SLOT(sCheck()));
}

vendorAddress::~vendorAddress()
{
  // no need to delete child widgets, Qt does it all for us
}

void vendorAddress::languageChange()
{
  retranslateUi(this);
}

enum SetResponse vendorAddress::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("vend_id", &valid);
  if (valid)
    _vendid = param.toInt();

  param = pParams.value("vendaddr_id", &valid);
  if (valid)
  {
    _vendaddrid = param.toInt();
    populate();
  }

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      _mode = cNew;
      _number->setFocus();

      q.prepare("SELECT crmacct_id "
                "FROM vendinfo "
                "  JOIN crmacct ON (crmacct_vend_id=vend_id) "
                "WHERE (vend_id=:vend_id);");
      q.bindValue(":vend_id", _vendid);
      q.exec();
      if(q.first())
      {
        _address->setSearchAcct(q.value("crmacct_id").toInt());
        _contact->setSearchAcct(q.value("crmacct_id").toInt());
      }
      else if (q.lastError().type() != QSqlError::NoError)
      {
	systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
	return UndefinedError;
      }
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;
      _number->setFocus();
    }
    else if (param.toString() == "view")
    {
      _mode = cView;

      _number->setEnabled(FALSE);
      _name->setEnabled(FALSE);
      _contact->setEnabled(FALSE);
      _address->setEnabled(FALSE);
      _notes->setReadOnly(TRUE);
      _close->setText(tr("&Close"));
      _save->hide();

      _close->setFocus();
    }
  }

  return NoError;
}

void vendorAddress::sSave()
{
  if (! q.exec("BEGIN"))
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  XSqlQuery rollback;
  rollback.prepare("ROLLBACK;");

  int saveResult = _address->save(AddressCluster::CHECK);
  if (-2 == saveResult)
  {
    int answer = QMessageBox::question(this,
		    tr("Question Saving Address"),
		    tr("There are multiple uses of this Vendor "
		       "Address.\nWhat would you like to do?"),
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
    systemError(this, tr("There was an error saving this address (%1).\n"
			 "Check the database server log for errors.")
		      .arg(saveResult), __FILE__, __LINE__);
    rollback.exec();
    _address->setFocus();
    return;
  }

  if (_mode == cNew)
  {
    q.exec("SELECT NEXTVAL('vendaddr_vendaddr_id_seq') AS vendaddr_id;");
    if (q.first())
      _vendaddrid = q.value("vendaddr_id").toInt();
    else
    {
      rollback.exec();
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }

    q.prepare( "INSERT INTO vendaddrinfo "
               "( vendaddr_id, vendaddr_vend_id,"
               "  vendaddr_code, vendaddr_name,"
               "  vendaddr_cntct_id, vendaddr_comments, vendaddr_addr_id ) "
               "VALUES "
               "( :vendaddr_id, :vendaddr_vend_id,"
               "  :vendaddr_code, :vendaddr_name,"
               "  :vendaddr_cntct_id, :vendaddr_comments, :vendaddr_addr_id );");
  }
  else if (_mode == cEdit)
    q.prepare( "UPDATE vendaddrinfo "
               "SET vendaddr_code=:vendaddr_code, vendaddr_name=:vendaddr_name,"
               "    vendaddr_cntct_id=:vendaddr_cntct_id,"
	       "    vendaddr_comments=:vendaddr_comments, "
               "    vendaddr_addr_id=:vendaddr_addr_id "
               "WHERE (vendaddr_id=:vendaddr_id);" );

  q.bindValue(":vendaddr_id", _vendaddrid);
  q.bindValue(":vendaddr_vend_id", _vendid);
  q.bindValue(":vendaddr_code", _number->text().trimmed());
  q.bindValue(":vendaddr_name", _name->text().trimmed());
  if (_contact->id() > 0)
    q.bindValue(":vendaddr_cntct_id", _contact->id());
  if (_address->id() > 0)
    q.bindValue(":vendaddr_addr_id", _address->id());
  q.bindValue(":vendaddr_comments", _notes->toPlainText().trimmed());
  q.exec();
  if (q.lastError().type() != QSqlError::NoError)
  {
    rollback.exec();
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  q.exec("COMMIT;");

  done(_vendaddrid);
}

void vendorAddress::sCheck()
{
  if ((_mode == cNew) && (_number->text().length()))
  {
    q.prepare( "SELECT vendaddr_id "
               "FROM vendaddr "
               "WHERE ( (vendaddr_vend_id=:vend_id)"
               " AND (UPPER(vendaddr_code)=UPPER(:vendaddr_code)) );" );
    q.bindValue(":vend_id", _vendid);
    q.bindValue(":vendaddr_code", _number->text().trimmed());
    q.exec();
    if (q.first())
    {
      _vendaddrid = q.value("vendaddr_id").toInt();
      _mode = cEdit;
      populate();

      _number->setEnabled(FALSE);
    }
  }
}

void vendorAddress::populate()
{
  q.prepare( "SELECT vendaddrinfo.*, crmacct_id "
             "FROM vendaddrinfo "
             " JOIN crmacct ON (vendaddr_vend_id=crmacct_vend_id) "
             "WHERE (vendaddr_id=:vendaddr_id);" );
  q.bindValue(":vendaddr_id", _vendaddrid);
  q.exec();
  if (q.first())
  {
    _vendid = q.value("vendaddr_vend_id").toInt();
    _number->setText(q.value("vendaddr_code"));
    _name->setText(q.value("vendaddr_name"));
    _contact->setId(q.value("vendaddr_cntct_id").toInt());
    _address->setId(q.value("vendaddr_addr_id").toInt());
    _notes->setText(q.value("vendaddr_comments").toString());
    _address->setSearchAcct(q.value("crmacct_id").toInt());
    _contact->setSearchAcct(q.value("crmacct_id").toInt());
  }
}
