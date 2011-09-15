/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "creditCard.h"

#include <QMessageBox>
#include <QRegExp>
#include <QSqlError>
#include <QValidator>
#include <QVariant>

#include "storedProcErrorLookup.h"

#define DEBUG false

int creditCard::saveCreditCard(QWidget *parent,
                              int &custId, 
                              QString &ccName, 
                              QString &ccAddress1, 
                              QString &ccAddress2,
                              QString &ccCity, 
                              QString &ccState, 
                              QString &ccZip, 
                              QString &ccCountry,
                              QString &ccNumber,
                              QString &ccType,
                              QString &ccExpireMonth,
                              QString &ccExpireYear,
                              int &ccId,
                              bool ccActive)
{
  bool everythingOK;
  everythingOK = true;
  int mode = cNew;
  int seq = 0;
  
  if (ccId > 0)
    mode = cEdit;

  ccName	= ccName.trimmed();
  ccAddress1	= ccAddress1.trimmed();
  ccAddress2	= ccAddress2.trimmed();
  ccCity	= ccCity.trimmed();
  ccState	= ccState.trimmed();
  ccZip		= ccZip.trimmed();
  ccCountry	= ccCountry.trimmed();
  ccExpireMonth	= ccExpireMonth.trimmed();
  ccExpireYear	= ccExpireYear.trimmed();

  if (ccExpireMonth.length() == 1)
      ccExpireMonth = "0" + ccExpireMonth;

  int mymonth = ccExpireMonth.toInt();
  int myyear = ccExpireYear.toInt();

  struct {
    bool	condition;
    QString	msg;
    int 	result;
  } error[] = {
    { ccName.length() == 0 || ccName.isEmpty(),
      tr("The name of the card holder must be entered"), -3 },
    { ccAddress1.length() == 0 || ccAddress1.isEmpty(),
      tr("The first address line must be entered"), -4 },
    { ccCity.length() == 0 || ccCity.isEmpty(),
      tr("The city must be entered"), -4 },
    { ccState.length() == 0 || ccState.isEmpty(),
      tr("The state must be entered"), -4 },
    { ccZip.length() == 0 || ccZip.isEmpty(),
      tr("The zip code must be entered"), -4 },
    { ccCountry.length() == 0 || ccCountry.isEmpty(),
      tr("The country must be entered"), -4 },
    { ccExpireMonth.length() == 0 || ccExpireMonth.isEmpty() || ccExpireMonth.length() > 2,
      tr("The Expiration Month must be entered"), -5 },
    { mymonth < 1 || mymonth > 12,
      tr("Valid Expiration Months are 01 through 12"), -5 },
    { ccExpireYear.length() != 4,
      tr("Valid Expiration Years are CCYY in format"), -5 },
    { myyear < 1970 || myyear > 2100,
      tr("Valid Expiration Years are 1970 through 2100"), -6 },
    { true, "", -1 }
  };

  int errIndex;
  for (errIndex = 0; ! error[errIndex].condition; errIndex++)
    ;
  if (! error[errIndex].msg.isEmpty())
  {
    QMessageBox::critical(parent, tr("Cannot Save Credit Card Information"),
			  error[errIndex].msg);
    return error[errIndex].result;
  }

  QString key;
  key = omfgThis->_key;
  
  ccNumber = ccNumber.trimmed().remove(QRegExp("[-\\s]"));
  bool allNumeric = QRegExp( "[0-9]{13,16}" ).exactMatch(ccNumber);
  bool hasBeenFormatted = QRegExp( "\\**([0-9]){4}" ).exactMatch(ccNumber); // tricky - repeated *s

  if (DEBUG)
    qDebug("creditCard::sSave() %s allNumeric %d, hasBeenFormatted %d",
           qPrintable(ccNumber.left(4)),
           allNumeric, hasBeenFormatted);

  if (mode == cNew ||
      (mode == cEdit && ! hasBeenFormatted))
  {
    if (!allNumeric || ccNumber.length() == 14)
    {
      QMessageBox::warning( parent, tr("Invalid Credit Card Number"),
                          tr("The credit card number must be all numeric and must be 13, 15 or 16 characters in length") );
      return -1;
    }
  }

  // See if this card exists on file
  if (mode == cNew)
  {
    q.prepare("SELECT ccard_id FROM ccard "
              "WHERE ((ccard_cust_id=:cust_id) "
              "AND (ccard_type=:type) "
              "AND (ccard_number=encrypt(setbytea(:number), setbytea(:key), 'bf'))) ");
    q.bindValue(":cust_id", custId);
    q.bindValue(":type" , ccType);
    q.bindValue(":number", ccNumber);
    q.bindValue(":key", key);
    q.exec();
    if (q.first())
    {
      ccId = q.value("ccard_id").toInt();
      mode = cEdit;
    }
  }

  int cceditreturn = 0;

  if (mode == cEdit && ! hasBeenFormatted)
  {
    // editccnumber validates but does not modify db
    q.prepare("SELECT editccnumber(text(:ccnum), text(:cctype)) AS cc_back;");
    q.bindValue(":ccnum", ccNumber);
    q.bindValue(":cctype", ccType);
    q.exec();
    if (q.first())
      cceditreturn = q.value("cc_back").toInt();
    else if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(parent, q.lastError().databaseText(), __FILE__, __LINE__);
      return -2;
    }
  }

  if (cceditreturn == -10)
  { // don't combine into (-10 && !CCtest): continue on -10 if not strict
    if (!_metrics->boolean("CCTest"))
    {
     QMessageBox::warning( parent, tr("Invalid Credit Card Number"),
      		   storedProcErrorLookup("editccnumber", cceditreturn));
     return -1;
    }
  }
  else if (cceditreturn < 0)
  {
    QMessageBox::warning(parent, tr("Invalid Credit Card Information"),
      		   storedProcErrorLookup("editccnumber", cceditreturn));
    return -1;
  }

  if (mode == cNew)
  {
    q.exec("SELECT NEXTVAL('ccard_ccard_id_seq') AS ccard_id;");
    if (q.first())
      ccId = q.value("ccard_id").toInt();
    else if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(parent, q.lastError().databaseText(), __FILE__, __LINE__);
      return -2;
    }

    q.prepare("SELECT COALESCE(MAX(ccard_seq), 0) + 10 AS ccard_seq FROM ccard WHERE ccard_cust_id =:ccard_cust_id;");
    q.bindValue(":ccard_cust_id", custId);
    q.exec();
    if (q.first())
      seq = q.value("ccard_seq").toInt();
    else if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(parent, q.lastError().databaseText(), __FILE__, __LINE__);
      return -2;
    }

    q.prepare( "INSERT INTO ccard "
               "( ccard_id, ccard_seq, ccard_cust_id, "
               "  ccard_active, ccard_name, ccard_address1,"
               "  ccard_address2,"
               "  ccard_city, ccard_state, ccard_zip,"
               "  ccard_country, ccard_number,"
               "  ccard_month_expired, ccard_year_expired, ccard_type)"
               "VALUES "
               "( :ccard_id,"
               "  :ccard_seq,"
	       "  :ccard_cust_id,"
               "  :ccard_active, "
               "  encrypt(setbytea(:ccard_name), setbytea(:key), 'bf'),"
	       "  encrypt(setbytea(:ccard_address1), setbytea(:key), 'bf'),"
               "  encrypt(setbytea(:ccard_address2), setbytea(:key), 'bf'),"
               "  encrypt(setbytea(:ccard_city), setbytea(:key), 'bf'),"
	       "  encrypt(setbytea(:ccard_state), setbytea(:key), 'bf'),"
               "  encrypt(setbytea(:ccard_zip), setbytea(:key), 'bf'),"
               "  encrypt(setbytea(:ccard_country), setbytea(:key), 'bf'),"
               "  encrypt(setbytea(:ccard_number), setbytea(:key), 'bf'),"
               "  encrypt(setbytea(:ccard_month_expired), setbytea(:key), 'bf'),"
               "  encrypt(setbytea(:ccard_year_expired), setbytea(:key), 'bf'),"
               "  :ccard_type );" );
  }
  else if (mode == cEdit)
    q.prepare( "UPDATE ccard "
               "SET ccard_active=:ccard_active, "
               "    ccard_name=encrypt(setbytea(:ccard_name), setbytea(:key), 'bf'),"
	       "    ccard_address1=encrypt(setbytea(:ccard_address1), setbytea(:key), 'bf'),"
               "    ccard_address2=encrypt(setbytea(:ccard_address2), setbytea(:key), 'bf'), "
               "    ccard_city=encrypt(setbytea(:ccard_city), setbytea(:key), 'bf'),"
	       "    ccard_state=encrypt(setbytea(:ccard_state), setbytea(:key), 'bf'),"
	       "    ccard_zip=encrypt(setbytea(:ccard_zip), setbytea(:key), 'bf'),"
               "    ccard_country=encrypt(setbytea(:ccard_country), setbytea(:key), 'bf'),"
               "    ccard_month_expired=encrypt(setbytea(:ccard_month_expired), setbytea(:key), 'bf'),"
               "    ccard_year_expired=encrypt(setbytea(:ccard_year_expired), setbytea(:key), 'bf'),"
              "    ccard_type=:ccard_type "
               "WHERE (ccard_id=:ccard_id);" );

  q.bindValue(":ccard_id", ccId);
  q.bindValue(":ccard_seq", seq);
  q.bindValue(":ccard_cust_id", custId);
  q.bindValue(":ccard_active", QVariant(ccActive));
  q.bindValue(":ccard_number", ccNumber);
  q.bindValue(":ccard_name", ccName);
  q.bindValue(":ccard_address1", ccAddress1);
  q.bindValue(":ccard_address2", ccAddress2);
  q.bindValue(":ccard_city",	 ccCity);
  q.bindValue(":ccard_state",	 ccState);
  q.bindValue(":ccard_zip",	 ccZip);
  q.bindValue(":ccard_country",	 ccCountry);
  q.bindValue(":ccard_month_expired",ccExpireMonth);
  q.bindValue(":ccard_year_expired",ccExpireYear);
  q.bindValue(":key", key);
  q.bindValue(":ccard_type", ccType);
  q.exec();
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(parent, q.lastError().databaseText(), __FILE__, __LINE__);
    return -2;
  }

  // TODO: combine with UPDATE above?
  if (mode == cEdit && ! hasBeenFormatted)     // user wants to change the cc number
  {
    q.prepare("UPDATE ccard SET ccard_number=encrypt(setbytea(:ccard_number), setbytea(:key), 'bf') "
              "WHERE (ccard_id=:ccard_id);" );
    q.bindValue(":ccard_number", ccNumber);
    q.bindValue(":key",          key);
    q.bindValue(":ccard_id",     ccId);
    q.exec();
    if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(parent, q.lastError().databaseText(), __FILE__, __LINE__);
      return -2;
    }
  }

  return ccId;
}

////////////////////////////////////////////////

creditCard::creditCard(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
    setupUi(this);

    _ccardid = 0;

    connect(_save, SIGNAL(clicked()), this, SLOT(sSave()));
}

creditCard::~creditCard()
{
    // no need to delete child widgets, Qt does it all for us
}

void creditCard::languageChange()
{
    retranslateUi(this);
}

const char *_fundsTypes2[] = { "M", "V", "A", "D" };
const int _fundsTypeCount2 = 4;

enum SetResponse creditCard::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("cust_id", &valid);
  if (valid)
    _custid = param.toInt();

  param = pParams.value("ccard_id", &valid);
  if (valid)
  {
    _ccardid = param.toInt();
    populate();
  }

  param = pParams.value("mode", &valid);
  if (valid)
  {
    XSqlQuery cust;
    cust.prepare( "SELECT cust_number, cust_name "
               "FROM cust "
               "WHERE (cust_id=:cust_id);" );
    cust.bindValue(":cust_id", _custid);
    cust.exec();
    if (cust.first())
    {
      _custNum->setText(cust.value("cust_number").toString());
      _custName->setText(cust.value("cust_name").toString());

    }
    else if (cust.lastError().type() != QSqlError::NoError)
      systemError(this, cust.lastError().databaseText(), __FILE__, __LINE__);

    if (param.toString() == "new")
    {
      _mode = cNew;
      _address->setCountry("United States");

      cust.prepare( "SELECT cust_number, cust_name,"
                    "       addr_line1, addr_line2, addr_line3,"
                    "       addr_city, addr_state, addr_country,"
                    "       addr_postalcode "
                    "  FROM custinfo, cntct, addr "
                    " WHERE((cust_id=:cust_id)"
                    "   AND (cust_cntct_id=cntct_id)"
                    "   AND (cntct_addr_id=addr_id));" );
      cust.bindValue(":cust_id", _custid);
      cust.exec();
      if (cust.first())
      {
        _address->setLine1(cust.value("addr_line1").toString());
        _address->setLine3(cust.value("addr_line3").toString());
        _address->setLine2(cust.value("addr_line2").toString());
        _address->setCity(cust.value("addr_city").toString());
        _address->setState(cust.value("addr_state").toString());
        _address->setPostalCode(cust.value("addr_postalcode").toString());
        _address->setCountry(cust.value("addr_country").toString());
      }
      _fundsType2->setFocus();
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;
      _fundsType2->setEnabled(FALSE);
      _save->setFocus();
    }
    else if (param.toString() == "view")
    {
      _mode = cView;

      _fundsType2->setEnabled(FALSE);
      _creditCardNumber->setEnabled(FALSE);
      _active->setEnabled(FALSE);
      _name->setEnabled(FALSE);
      _address->setEnabled(FALSE);
      _expireMonth->setEnabled(FALSE);
      _expireYear->setEnabled(FALSE);
      _close->setText(tr("&Close"));
      _save->hide();

      _close->setFocus();
    }
  }

  return NoError;
}

void creditCard::sSave()
{
  if (! _address->line3().isEmpty())
  {
    int answer = QMessageBox::question(this, tr("Third Address Line Ignored"),
				       tr("<p>The third line of the street "
				          "address will not be saved as part "
					  "of the credit card address. The "
					  "xTuple credit card interface "
					  "supports only 2 lines of street "
					  "address.</p><p>Continue processing?"),
					QMessageBox::Yes,
					QMessageBox::No | QMessageBox::Default);
    if (answer == QMessageBox::No)
    {
      _address->setFocus();
      return;
    }
  }

  bool    ccActive      = _active->isChecked();
  QString ccName	= _name->text();
  QString ccAddress1	= _address->line1();
  QString ccAddress2    = _address->line2();
  QString ccCity	= _address->city();
  QString ccState	= _address->state();
  QString ccZip		= _address->postalCode();
  QString ccCountry	= _address->country();
  QString ccNumber      = _creditCardNumber->text();
  QString ccType        = QString(*(_fundsTypes2 + _fundsType2->currentIndex()));
  QString ccExpireMonth	= _expireMonth->text();
  QString ccExpireYear	= _expireYear->text();
  
  int result = saveCreditCard(this,
                              _custid,
                              ccName,
                              ccAddress1,
                              ccAddress2,
                              ccCity,
                              ccState,
                              ccZip,
                              ccCountry,
                              ccNumber,
                              ccType,
                              ccExpireMonth,
                              ccExpireYear,
                              _ccardid,
                              ccActive);
  
  switch (result)
  {
    case -1:
      _creditCardNumber->setFocus();
      break;
    case -2:
      break;
    case -3:
      _name->setFocus();
      break;
    case -4:
      _address->setFocus();
      break;
    case -5:
      _expireMonth->setFocus();
      break;
    case -6:
      _expireYear->setFocus();
      break;
    default:
      done(result);
  }
}

void creditCard::populate()
{
  QString key;
  key = omfgThis->_key;

  q.prepare( "SELECT ccard_active,"
            "       formatbytea(decrypt(setbytea(ccard_name), setbytea(:key), 'bf')) AS ccard_name,"
            "       formatbytea(decrypt(setbytea(ccard_address1), setbytea(:key), 'bf')) AS ccard_address1,"
            "       formatbytea(decrypt(setbytea(ccard_address2), setbytea(:key), 'bf')) AS ccard_address2,"
            "       formatbytea(decrypt(setbytea(ccard_city), setbytea(:key), 'bf')) AS ccard_city,"
            "       formatbytea(decrypt(setbytea(ccard_state), setbytea(:key), 'bf')) AS ccard_state,"
            "       formatbytea(decrypt(setbytea(ccard_zip), setbytea(:key), 'bf')) AS ccard_zip,"
            "       formatbytea(decrypt(setbytea(ccard_country), setbytea(:key), 'bf')) AS ccard_country,"
            "       formatccnumber(decrypt(setbytea(ccard_number), setbytea(:key), 'bf')) AS ccard_number,"
            "       ccard_debit,"
            "       formatbytea(decrypt(setbytea(ccard_month_expired), setbytea(:key), 'bf')) AS ccard_month_expired,"
            "       formatbytea(decrypt(setbytea(ccard_year_expired), setbytea(:key), 'bf')) AS ccard_year_expired,"
            "       ccard_type "
             "FROM ccard "
             "WHERE (ccard_id=:ccard_id);" );
  q.bindValue(":ccard_id", _ccardid);
  q.bindValue(":key",key);
  q.exec();
  if (q.first())
  {
    // _custNum->setText(q.value("cust_number").toString());
    // _custName->setText(q.value("cust_name").toString());
    _active->setChecked(q.value("ccard_active").toBool());
    _creditCardNumber->setText(q.value("ccard_number"));
    _name->setText(q.value("ccard_name"));
    _address->setLine1(q.value("ccard_address1").toString());
    _address->setLine2(q.value("ccard_address2").toString());
    _address->setCity(q.value("ccard_city").toString());
    _address->setState(q.value("ccard_state").toString());
    _address->setPostalCode(q.value("ccard_zip").toString());
    _address->setCountry(q.value("ccard_country").toString());
    _expireMonth->setText(q.value("ccard_month_expired").toString());
    _expireYear->setText(q.value("ccard_year_expired").toString());

    for (int counter = 0; counter < _fundsType2->count(); counter++)
      if (QString(q.value("ccard_type").toString()[0]) == _fundsTypes2[counter])
        _fundsType2->setCurrentIndex(counter);

  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}
