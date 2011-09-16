/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "shipTo.h"

#include <QCloseEvent>
#include <QMessageBox>
#include <QSqlError>
#include <QValidator>
#include <QVariant>

#include "addresscluster.h"

#define DEBUG false

shipTo::shipTo(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_shipToNumber, SIGNAL(lostFocus()), this, SLOT(sPopulateNumber()));
  connect(_save, SIGNAL(clicked()), this, SLOT(sSave()));
  connect(_salesRep, SIGNAL(newID(int)), this, SLOT(sPopulateCommission(int)));
  connect(_address, SIGNAL(addressChanged(QString,QString,QString,QString,QString,QString, QString)),
          _contact, SLOT(setNewAddr(QString,QString,QString,QString,QString,QString, QString)));

  _shipZone->populate( "SELECT shipzone_id, shipzone_name "
                       "FROM shipzone "
                       "ORDER BY shipzone_name;" );

  _commission->setValidator(omfgThis->percentVal());

  _shiptoid = -1;
}

shipTo::~shipTo()
{
    // no need to delete child widgets, Qt does it all for us
}

void shipTo::languageChange()
{
    retranslateUi(this);
}

enum SetResponse shipTo::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("cust_id", &valid);
  if (valid)
    _custid = param.toInt();

  param = pParams.value("shipto_id", &valid);
  if (valid)
  {
    _shiptoid = param.toInt();
    populate();
  }

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      _mode = cNew;

      XSqlQuery cust;
      cust.prepare( "SELECT cust_number, cust_name, cust_taxzone_id, "
                 "       cust_salesrep_id, cust_shipform_id, cust_shipvia, cust_shipchrg_id, "
                 "       crmacct_id "
                 "FROM custinfo "
                 "  JOIN crmacct ON (cust_id=crmacct_cust_id) "
                 "WHERE (cust_id=:cust_id);" );
      cust.bindValue(":cust_id", _custid);
      cust.exec();
      if (cust.first())
      {
        _custNum->setText(cust.value("cust_number").toString());
        _custName->setText(cust.value("cust_name").toString());
        _salesRep->setId(cust.value("cust_salesrep_id").toInt());
        _shipform->setId(cust.value("cust_shipform_id").toInt());
        _taxzone->setId(cust.value("cust_taxzone_id").toInt());
        _contact->setSearchAcct(cust.value("crmacct_id").toInt());
        _address->setSearchAcct(cust.value("crmacct_id").toInt());
        _shipchrg->setId(cust.value("cust_shipchrg_id").toInt());

	//  Handle the free-form Ship Via
        _shipVia->setId(-1);
        QString shipvia = cust.value("cust_shipvia").toString().trimmed();
        if (shipvia.length())
        {
          for (int counter = 0; counter < _shipVia->count(); counter++)
            if (_shipVia->itemText(counter) == shipvia)
              _shipVia->setCurrentIndex(counter);

          if (_shipVia->id() == -1)
          {
            _shipVia->addItem(shipvia);
            _shipVia->setCurrentIndex(_shipVia->count() - 1);
          }
        }
      }
      if (cust.lastError().type() != QSqlError::NoError)
      {
	    systemError(this, cust.lastError().databaseText(), __FILE__, __LINE__);
	    return UndefinedError;
      }
      _save->setEnabled(false);
      _shipToNumber->setFocus();
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;
      _shipToNumber->setEnabled(FALSE);
      _save->setFocus();
    }
    else if (param.toString() == "view")
    {
      _mode = cView;

      _shipToNumber->setEnabled(FALSE);
      _active->setEnabled(FALSE);
      _default->setEnabled(FALSE);
      _name->setEnabled(FALSE);
      _contact->setEnabled(FALSE);
      _address->setEnabled(FALSE);
      _salesRep->setEnabled(FALSE);
      _commission->setEnabled(FALSE);
      _shipZone->setEnabled(FALSE);
      _taxzone->setEnabled(FALSE);
      _shipVia->setEnabled(FALSE);
      _shipform->setEnabled(FALSE);
      _shipchrg->setEnabled(FALSE);
      _comments->setEnabled(FALSE);
      _shippingComments->setEnabled(FALSE);
      _close->setText(tr("&Close"));
      _save->hide();

      _close->setFocus();
    }
  }

  return NoError;
}

int shipTo::id() const
{
  return _shiptoid;
}

void shipTo::sSave()
{
  if (_name->text().length() == 0)
  {
      QMessageBox::warning( this, tr("Cannot Save Ship To"),
                            tr("You must enter a valid Name.") );
      return;
  }
  
  XSqlQuery rollback;
  rollback.prepare("ROLLBACK;");

  if (! q.exec("BEGIN"))
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  int saveResult = _address->save(AddressCluster::CHECK);
  if (-2 == saveResult)
  {
    int answer = QMessageBox::question(this,
		    tr("Question Saving Address"),
		    tr("<p>There are multiple uses of this Ship-To "
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
    systemError(this, tr("<p>There was an error saving this address (%1). "
			 "Check the database server log for errors.")
		      .arg(saveResult), __FILE__, __LINE__);
    rollback.exec();
    _address->setFocus();
    return;
  }

  q.prepare( "UPDATE shiptoinfo "
             "SET shipto_active=:shipto_active, shipto_default=:shipto_default,"
             "    shipto_name=:shipto_name, shipto_cntct_id=:shipto_cntct_id,"
             "    shipto_commission=:shipto_commission,"
             "    shipto_comments=:shipto_comments, shipto_shipcomments=:shipto_shipcomments,"
             "    shipto_taxzone_id=:shipto_taxzone_id, shipto_salesrep_id=:shipto_salesrep_id, shipto_shipzone_id=:shipto_shipzone_id,"
             "    shipto_shipvia=:shipto_shipvia, shipto_shipform_id=:shipto_shipform_id, shipto_shipchrg_id=:shipto_shipchrg_id,"
             "    shipto_addr_id=:shipto_addr_id "
             "WHERE (shipto_id=:shipto_id);" );

  q.bindValue(":shipto_id", _shiptoid);
  q.bindValue(":shipto_active", QVariant(_active->isChecked()));
  q.bindValue(":shipto_default", QVariant(_default->isChecked()));
  q.bindValue(":shipto_cust_id", _custid);
  q.bindValue(":shipto_num", _shipToNumber->text().trimmed());
  q.bindValue(":shipto_name", _name->text());
  if (_contact->id() > 0)
    q.bindValue(":shipto_cntct_id", _contact->id());
  if (_address->id() > 0)
    q.bindValue(":shipto_addr_id", _address->id());
  q.bindValue(":shipto_commission", (_commission->toDouble() / 100));
  q.bindValue(":shipto_comments", _comments->toPlainText());
  q.bindValue(":shipto_shipcomments", _shippingComments->toPlainText());
  q.bindValue(":shipto_shipvia", _shipVia->currentText());
  if (_taxzone->isValid())
    q.bindValue(":shipto_taxzone_id",  _taxzone->id());
  if (_salesRep->id() != -1)
    q.bindValue(":shipto_salesrep_id", _salesRep->id());
  if (_shipZone->isValid())
    q.bindValue(":shipto_shipzone_id", _shipZone->id());
  if (_shipform->id() != -1)
    q.bindValue(":shipto_shipform_id", _shipform->id());
  if (_shipchrg->id() != -1)
  q.bindValue(":shipto_shipchrg_id", _shipchrg->id());
  q.exec();
  if (q.lastError().type() != QSqlError::NoError)
  {
    rollback.exec();
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  q.exec("COMMIT;");

  if (_mode == cNew)
    emit newId(_shiptoid);
  _mode = 0;

  done(_shiptoid);
}

void shipTo::populate()
{
  q.prepare( "SELECT cust_number, cust_name, shipto_active, shipto_default,"
             "       shipto_cust_id,"
             "       shipto_num, shipto_name, shipto_cntct_id,"
             "       shipto_shipvia, shipto_commission,"
             "       shipto_comments, shipto_shipcomments,"
             "       COALESCE(shipto_salesrep_id,-1) AS shipto_salesrep_id, shipto_taxzone_id, COALESCE(shipto_shipzone_id,-1) AS shipto_shipzone_id,"
             "       COALESCE(shipto_shipform_id,-1) AS shipto_shipform_id, shipto_shipchrg_id,"
	     "       shipto_addr_id,"
             "       crmacct_id "
             "FROM shiptoinfo "
             "  LEFT OUTER JOIN cust ON (shipto_cust_id=cust_id) "
             "  LEFT OUTER JOIN crmacct ON (cust_id=crmacct_cust_id) "
             "WHERE (shipto_id=:shipto_id);" );
  q.bindValue(":shipto_id", _shiptoid);
  q.exec();
  if (q.first())
  {
    double commission = q.value("shipto_commission").toDouble();
    _custid = q.value("shipto_cust_id").toInt();
    _custNum->setText(q.value("cust_number").toString());
    _custName->setText(q.value("cust_name").toString());
    _active->setChecked(q.value("shipto_active").toBool());
    _default->setChecked(q.value("shipto_default").toBool());
    _shipToNumber->setText(q.value("shipto_num"));
    _name->setText(q.value("shipto_name"));
    _contact->setId(q.value("shipto_cntct_id").toInt());
    _contact->setSearchAcct(q.value("crmacct_id").toInt());
    _address->setSearchAcct(q.value("crmacct_id").toInt());
    _comments->setText(q.value("shipto_comments").toString());
    _shippingComments->setText(q.value("shipto_shipcomments").toString());
    _taxzone->setId(q.value("shipto_taxzone_id").toInt());
    _shipZone->setId(q.value("shipto_shipzone_id").toInt());
    _shipform->setId(q.value("shipto_shipform_id").toInt());
    _shipchrg->setId(q.value("shipto_shipchrg_id").toInt());
    _address->setId(q.value("shipto_addr_id").toInt());

    //  Handle the free-form Ship Via
    _shipVia->setCurrentIndex(-1);
    QString shipvia = q.value("shipto_shipvia").toString();
    if (shipvia.trimmed().length() != 0)
    {
      for (int counter = 0; counter < _shipVia->count(); counter++)
        if (_shipVia->itemText(counter) == shipvia)
          _shipVia->setCurrentIndex(counter);

      if (_shipVia->id() == -1)
      {
        _shipVia->addItem(shipvia);
        _shipVia->setCurrentIndex(_shipVia->count() - 1);
      }
    }

    _salesRep->setId(q.value("shipto_salesrep_id").toInt());
    _commission->setDouble(commission * 100);

    emit newId(_shiptoid);
    emit populated();
  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void shipTo::sPopulateNumber()
{
  if (_shipToNumber->text().length() == 0)
  {
    q.prepare( "SELECT (COALESCE(MAX(CAST(shipto_num AS INTEGER)), 0) + 1) AS n_shipto_num "
               "  FROM shipto "
               " WHERE ((shipto_cust_id=:cust_id)"
               "   AND  (shipto_num~'^[0-9]*$') )" );
    q.bindValue(":cust_id", _custid);
    q.exec();
    if (q.first())
      _shipToNumber->setText(q.value("n_shipto_num"));
    else if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }
  else
  {
    q.prepare( "SELECT shipto_id "
               "FROM shipto "
               "WHERE ( (shipto_cust_id=:cust_id)"
               " AND (UPPER(shipto_num)=UPPER(:shipto_num)) );" );
    q.bindValue(":cust_id", _custid);
    q.bindValue(":shipto_num", _shipToNumber->text());
    q.exec();
    if (q.first())
    {
      _mode = cEdit;
      _shiptoid = q.value("shipto_id").toInt();
      populate();

      _shipToNumber->setEnabled(FALSE);
      _name->setFocus();
    }
    else if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }

  if (_mode == cNew)
  {
    q.exec("SELECT NEXTVAL('shipto_shipto_id_seq') AS shipto_id;");
    if (q.first())
      _shiptoid = q.value("shipto_id").toInt();
    else if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }

    q.prepare( "INSERT INTO shiptoinfo "
               "( shipto_id, shipto_cust_id, shipto_active, shipto_num, shipto_commission ) "
               "VALUES "
               "( :shipto_id, :shipto_cust_id, :shipto_active, :shipto_num, :shipto_commission );" );

    q.bindValue(":shipto_id", _shiptoid);
    q.bindValue(":shipto_active", QVariant(_active->isChecked()));
    q.bindValue(":shipto_cust_id", _custid);
    q.bindValue(":shipto_num", _shipToNumber->text().trimmed());
    q.bindValue(":shipto_commission", (_commission->toDouble() / 100));
    q.exec();
    if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }
  _save->setEnabled(true);
}

void shipTo::sPopulateCommission(int pSalesrepid)
{
  if (_mode != cView)
  {
    q.prepare( "SELECT salesrep_commission "
               "FROM salesrep "
               "WHERE (salesrep_id=:salesrep_id);" );
    q.bindValue(":salesrep_id", pSalesrepid);
    q.exec();
    if (q.first())
      _commission->setDouble(q.value("salesrep_commission").toDouble() * 100);
    else if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }
}

void shipTo::closeEvent(QCloseEvent * /*pEvent*/)
{
  if (DEBUG)
    qDebug("%s::closeEvent() _mode = %d", qPrintable(objectName()), _mode);
  if (_mode == cNew)
  {
    q.prepare( "DELETE FROM shiptoinfo "
               "WHERE (shipto_id=:shipto_id);" );
    q.bindValue(":shipto_id", _shiptoid);
    q.exec();
  }
}

