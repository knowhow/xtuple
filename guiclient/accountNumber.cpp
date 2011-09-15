/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "accountNumber.h"

#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include <metasql.h>
#include <parameter.h>

accountNumber::accountNumber(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_buttonBox, SIGNAL(accepted()), this, SLOT(sSave()));
  connect(_buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
  connect(_type, SIGNAL(activated(int)), this, SLOT(populateSubTypes()));

  _currency->setLabel(_currencyLit);

  // Until we find out what use there is for an account-level currency, hide it
  _currencyLit->hide();
  _currency->hide();
  
  _subType->setAllowNull(TRUE);
  populateSubTypes();
  
  if (_metrics->value("GLCompanySize").toInt() == 0)
  {
    _company->hide();
    _sep1Lit->hide();
  }

  if (_metrics->value("GLProfitSize").toInt() == 0)
  {
    _profit->hide();
    _sep2Lit->hide();
  }

  if (_metrics->value("GLSubaccountSize").toInt() == 0)
  {
    _sub->hide();
    _sep3Lit->hide();
  }
  if (!_metrics->boolean("ManualForwardUpdate"))
    _forwardUpdate->hide();

  _wasActive = false;
}

accountNumber::~accountNumber()
{
  // no need to delete child widgets, Qt does it all for us
}

void accountNumber::languageChange()
{
  retranslateUi(this);
}

enum SetResponse accountNumber::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("accnt_id", &valid);
  if (valid)
  {
    _accntid = param.toInt();
    populate();
  }

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      _mode = cNew;
      _number->setValidator(new QRegExpValidator(QRegExp(QString("\\S{0,%1}").arg(_metrics->value("GLMainSize").toInt())),
                                                 _number));
      _number->setFocus();
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;
      _number->setEnabled(FALSE);
      _description->setFocus();
    }
    else if (param.toString() == "view")
    {
      _mode = cView;

      _company->setEnabled(FALSE);
      _profit->setEnabled(FALSE);
      _number->setEnabled(FALSE);
      _sub->setEnabled(FALSE);
      _description->setEnabled(FALSE);
      _extReference->setEnabled(FALSE);
      _postIntoClosed->setEnabled(FALSE);
      _forwardUpdate->setEnabled(FALSE);
      _active->setEnabled(FALSE);
      _comments->setEnabled(FALSE);
      _buttonBox->setStandardButtons(QDialogButtonBox::Close);
    }
  }

  return NoError;
}

void accountNumber::sSave()
{
  if (_mode == cEdit && _wasActive && !_active->isChecked())
  {
    QString glsum("SELECT SUM(gltrans_amount) AS bal"
                  "  FROM gltrans"
                  " WHERE gltrans_accnt_id=<? value(\"accnt_id\") ?>;");
    ParameterList pl;
    pl.append("accnt_id", _accntid);
    MetaSQLQuery mm(glsum);
    q = mm.toQuery(pl);
    if(q.first() && q.value("bal").toInt() != 0)
    {
      if(QMessageBox::warning(this, tr("Account has Balance"),
                            tr("<p>This Account has a balance. "
			       "Are you sure you want to mark it inactive?"),
                            QMessageBox::Yes, QMessageBox::No | QMessageBox::Default) == QMessageBox::No)
      {
        return;
      }
    }
  }

  QString sql("SELECT accnt_id "
              "FROM ONLY accnt "
              "WHERE ( (accnt_number=<? value(\"accnt_number\") ?>)"
	      "<? if exists(\"accnt_company\") ?>"
	      " AND (accnt_company=<? value(\"accnt_company\") ?>)"
	      "<? endif ?>"
	      "<? if exists(\"accnt_profit\") ?>"
	      " AND (accnt_profit=<? value(\"accnt_profit\") ?>)"
	      "<? endif ?>"
	      "<? if exists(\"accnt_sub\") ?>"
	      " AND (accnt_sub=<? value(\"accnt_sub\") ?>)"
	      "<? endif ?>"
	      "<? if exists(\"accnt_id\") ?>"
	      " AND (accnt_id<><? value(\"accnt_id\") ?>)"
	      "<? endif ?>"
	      " );" );

  ParameterList params;
  params.append("accnt_number", _number->text().trimmed());

  if (_metrics->value("GLCompanySize").toInt())
    params.append("accnt_company", _company->currentText());

  if (_metrics->value("GLProfitSize").toInt())
    params.append("accnt_profit", _profit->currentText());

  if (_metrics->value("GLSubaccountSize").toInt())
    params.append("accnt_sub", _sub->currentText());

  if (_mode == cEdit)
    params.append("accnt_id", _accntid);

  MetaSQLQuery mql(sql);
  q = mql.toQuery(params);
  if (q.first())
  {
    QMessageBox::warning( this, tr("Cannot Save Account"),
                          tr("<p>This Account cannot be saved as an Account "
			     "with the same number already exists.") );
    return;
  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  if (_mode == cNew)
  {
    if(_number->text().trimmed().isEmpty())
    {
      QMessageBox::warning(this, tr("No Account Number"),
			   tr("<p>You must specify an account number before "
			      "you may save this record."));
      return;
    }

    q.exec("SELECT NEXTVAL('accnt_accnt_id_seq') AS _accnt_id;");
    if (q.first())
      _accntid = q.value("_accnt_id").toInt();
    else if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }

    q.prepare( "INSERT INTO accnt "
               "( accnt_id,"
               "  accnt_company, accnt_profit, accnt_number, accnt_sub,"
               "  accnt_closedpost, accnt_forwardupdate, accnt_active,"
               "  accnt_type, accnt_descrip, accnt_extref, accnt_comments, "
	       "  accnt_subaccnttype_code, accnt_curr_id ) "
               "VALUES "
               "( :accnt_id,"
               "  :accnt_company, :accnt_profit, :accnt_number, :accnt_sub,"
               "  :accnt_closedpost, :accnt_forwardupdate, :accnt_active,"
               "  :accnt_type, :accnt_descrip, :accnt_extref, :accnt_comments,"
               "  (SELECT subaccnttype_code FROM subaccnttype WHERE subaccnttype_id=:accnt_subaccnttype_id), "
	       "  :accnt_curr_id );" );
  }
  else if (_mode == cEdit)
    q.prepare( "UPDATE accnt "
               "SET accnt_company=:accnt_company, accnt_profit=:accnt_profit,"
               "    accnt_number=:accnt_number, accnt_sub=:accnt_sub,"
               "    accnt_closedpost=:accnt_closedpost, accnt_forwardupdate=:accnt_forwardupdate,"
               "    accnt_active=:accnt_active,"
               "    accnt_type=:accnt_type, accnt_descrip=:accnt_descrip, accnt_extref=:accnt_extref,"
               "    accnt_comments=:accnt_comments,"
               "    accnt_subaccnttype_code=(SELECT subaccnttype_code FROM subaccnttype WHERE subaccnttype_id=:accnt_subaccnttype_id),"
	       "    accnt_curr_id=:accnt_curr_id "
               "WHERE (accnt_id=:accnt_id);" );

  q.bindValue(":accnt_id", _accntid);
  q.bindValue(":accnt_company", _company->currentText());
  q.bindValue(":accnt_profit", _profit->currentText());
  q.bindValue(":accnt_number", _number->text());
  q.bindValue(":accnt_sub", _sub->currentText());
  q.bindValue(":accnt_descrip", _description->text());
  q.bindValue(":accnt_extref", _extReference->text());
  q.bindValue(":accnt_closedpost",    QVariant(_postIntoClosed->isChecked()));
  q.bindValue(":accnt_forwardupdate", QVariant(_forwardUpdate->isChecked()));
  q.bindValue(":accnt_active", QVariant(_active->isChecked()));
  q.bindValue(":accnt_comments", _comments->toPlainText());
  q.bindValue(":accnt_curr_id", _currency->id());
  q.bindValue(":accnt_subaccnttype_id", _subType->id());

  if (_type->currentIndex() == 0)
    q.bindValue(":accnt_type", "A");
  else if (_type->currentIndex() == 1)
    q.bindValue(":accnt_type", "L");
  else if (_type->currentIndex() == 2)
    q.bindValue(":accnt_type", "E");
  else if (_type->currentIndex() == 3)
    q.bindValue(":accnt_type", "R");
  else if (_type->currentIndex() == 4)
    q.bindValue(":accnt_type", "Q");

  q.exec();
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  done(_accntid);
}

void accountNumber::populate()
{
  q.prepare( "SELECT accnt_company, accnt_profit, accnt_number, accnt_sub,"
             "       accnt_closedpost, accnt_forwardupdate,"
             "       accnt_type, accnt_descrip, accnt_extref, accnt_comments, subaccnttype_id, "
             "       accnt_curr_id, "
             "       accnt_active, "
             "       CASE WHEN (gltrans_id IS NULL) THEN false ELSE true END AS used "
             "FROM accnt "
             "  LEFT OUTER JOIN subaccnttype ON (subaccnttype_code=accnt_subaccnttype_code) "
             "  LEFT OUTER JOIN gltrans ON (accnt_id=gltrans_accnt_id) "
             "WHERE (accnt_id=:accnt_id)"
             "LIMIT 1" );
  q.bindValue(":accnt_id", _accntid);
  q.exec();
  if (q.first())
  {
    if (_metrics->value("GLCompanySize").toInt())
      _company->setText(q.value("accnt_company"));

    if (_metrics->value("GLProfitSize").toInt())
      _profit->setText(q.value("accnt_profit"));

    if (_metrics->value("GLSubaccountSize").toInt())
      _sub->setText(q.value("accnt_sub"));

    _number->setText(q.value("accnt_number"));
    _description->setText(q.value("accnt_descrip"));
    _extReference->setText(q.value("accnt_extref"));
    _postIntoClosed->setChecked(q.value("accnt_closedpost").toBool());
    _forwardUpdate->setChecked(q.value("accnt_forwardupdate").toBool());
    _active->setChecked(q.value("accnt_active").toBool());
    _wasActive = _active->isChecked();
    _comments->setText(q.value("accnt_comments").toString());
    _currency->setId(q.value("accnt_curr_id").toInt());

    if (q.value("accnt_type").toString() == "A")
      _type->setCurrentIndex(0);
    else if (q.value("accnt_type").toString() == "L")
      _type->setCurrentIndex(1);
    else if (q.value("accnt_type").toString() == "E")
      _type->setCurrentIndex(2);
    else if (q.value("accnt_type").toString() == "R")
      _type->setCurrentIndex(3);
    else if (q.value("accnt_type").toString() == "Q")
      _type->setCurrentIndex(4);

    _type->setDisabled(q.value("used").toBool());

    populateSubTypes();
    _subType->setId(q.value("subaccnttype_id").toInt());
  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void accountNumber::populateSubTypes()
{
  XSqlQuery sub;
  sub.prepare("SELECT subaccnttype_id, (subaccnttype_code||'-'||subaccnttype_descrip) "
              "FROM subaccnttype "
              "WHERE (subaccnttype_accnt_type=:subaccnttype_accnt_type) "
              "ORDER BY subaccnttype_code; ");
  if (_type->currentIndex() == 0)
    sub.bindValue(":subaccnttype_accnt_type", "A");
  else if (_type->currentIndex() == 1)
    sub.bindValue(":subaccnttype_accnt_type", "L");
  else if (_type->currentIndex() == 2)
    sub.bindValue(":subaccnttype_accnt_type", "E");
  else if (_type->currentIndex() == 3)
    sub.bindValue(":subaccnttype_accnt_type", "R");
  else if (_type->currentIndex() == 4)
    sub.bindValue(":subaccnttype_accnt_type", "Q");
  sub.exec();
  _subType->populate(sub);
  if (sub.lastError().type() != QSqlError::NoError)
  {
    systemError(this, sub.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
  
}

