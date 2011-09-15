/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "salesAccount.h"

#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

salesAccount::salesAccount(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_buttonBox, SIGNAL(accepted()), this, SLOT(sSave()));

  _warehouse->populate( "SELECT -1, 'Any'::text AS warehous_code, 0 AS sort "
			"UNION SELECT warehous_id, warehous_code, 1 AS sort "
			"FROM warehous "
			"ORDER BY sort, warehous_code" );

  _customerTypes->setType(ParameterGroup::CustomerType);
  _productCategories->setType(ParameterGroup::ProductCategory);

  if (! _metrics->boolean("EnableReturnAuth"))
  {
    _corLit->setVisible(false);
    _cor->setVisible(false);
    _returnsLit->setVisible(false);
    _returns->setVisible(false);
    _cowLit->setVisible(false);
    _cow->setVisible(false);
  }

  _sales->setType(GLCluster::cRevenue);
  _credit->setType(GLCluster::cRevenue);
  _cos->setType(GLCluster::cExpense);
  _returns->setType(GLCluster::cRevenue);
  _cor->setType(GLCluster::cExpense);
  _cow->setType(GLCluster::cExpense);
}

salesAccount::~salesAccount()
{
  // no need to delete child widgets, Qt does it all for us
}

void salesAccount::languageChange()
{
  retranslateUi(this);
}

enum SetResponse salesAccount::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("salesaccnt_id", &valid);
  if (valid)
  {
    _salesaccntid = param.toInt();
    populate();
  }

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
      _mode = cNew;
    else if (param.toString() == "edit")
    {
      _mode = cEdit;

      _buttonBox->setFocus();
    }
    else if (param.toString() == "view")
    {
      _mode = cView;

      _warehouse->setEnabled(FALSE);
      _customerTypes->setEnabled(FALSE);
      _productCategories->setEnabled(FALSE);
      _sales->setReadOnly(TRUE);
      _credit->setReadOnly(TRUE);
      _cos->setReadOnly(TRUE);
      _returns->setReadOnly(TRUE);
      _cor->setReadOnly(TRUE);
      _cow->setReadOnly(TRUE);
      _buttonBox->clear();
      _buttonBox->addButton(QDialogButtonBox::Close);
      _buttonBox->setFocus();
    }
  }

  return NoError;
}

void salesAccount::sSave()
{
  if (!_sales->isValid())
  {
    QMessageBox::warning( this, tr("Select Sales Account"),
                          tr("You must select a Sales Account for this Assignment.") );
    _sales->setFocus();
    return;
  }

  if (!_credit->isValid())
  {
    QMessageBox::warning( this, tr("Select Credit Memo Account"),
                          tr("You must select a Credit Memo Account for this Assignment.") );
    _credit->setFocus();
    return;
  }

  if (!_cos->isValid())
  {
    QMessageBox::warning( this, tr("Select Cost of Sales Account"), 
                          tr("You must select a Cost of Sales Account for this Assignment.") );
    _cos->setFocus();
    return;
  }

  if (_metrics->boolean("EnableReturnAuth"))
  {

    if (!_returns->isValid())
    {
      QMessageBox::warning( this, tr("Select Returns Account"), 
			    tr("You must select a Returns Account for this Assignment.") );
      _returns->setFocus();
      return;
    }

    if (!_cor->isValid())
    {
      QMessageBox::warning( this, tr("Select Cost of Returns Account"), 
			    tr("You must select a Cost of Returns Account for this Assignment.") );
      _cor->setFocus();
      return;
    }

    if (!_cow->isValid())
    {
      QMessageBox::warning( this, tr("Select Cost of Warranty Account"), 
			    tr("You must select a Cost of Warranty Account for this Assignment.") );
      _cow->setFocus();
      return;
    }
  }

  if (_mode == cNew)
  {
    q.exec("SELECT NEXTVAL('salesaccnt_salesaccnt_id_seq') AS salesaccnt_id;");
    if (q.first())
      _salesaccntid = q.value("salesaccnt_id").toInt();
    else if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }
    
  q.prepare("SELECT salesaccnt_id"
            "  FROM salesaccnt"
            " WHERE((salesaccnt_warehous_id=:salesaccnt_warehous_id)"
            "   AND (salesaccnt_custtype=:salesaccnt_custtype)"
            "   AND (salesaccnt_custtype_id=:salesaccnt_custtype_id)"
            "   AND (salesaccnt_prodcat=:salesaccnt_prodcat)"
            "   AND (salesaccnt_prodcat_id=:salesaccnt_prodcat_id)"
            "   AND (salesaccnt_id != :salesaccnt_id))");
  q.bindValue(":salesaccnt_id", _salesaccntid);
  q.bindValue(":salesaccnt_warehous_id", _warehouse->id());

  if (_customerTypes->isAll())
  {
    q.bindValue(":salesaccnt_custtype", ".*");
    q.bindValue(":salesaccnt_custtype_id", -1);
  }
  else if (_customerTypes->isSelected())
  {
    q.bindValue(":salesaccnt_custtype", "[^a-zA-Z0-9_]");
    q.bindValue(":salesaccnt_custtype_id", _customerTypes->id());
  }
  else
  {
    q.bindValue(":salesaccnt_custtype", _customerTypes->pattern());
    q.bindValue(":salesaccnt_custtype_id", -1);
  }

  if (_productCategories->isAll())
  {
    q.bindValue(":salesaccnt_prodcat", ".*");
    q.bindValue(":salesaccnt_prodcat_id", -1);
  }
  else if (_productCategories->isSelected())
  {
    q.bindValue(":salesaccnt_prodcat", "[^a-zA-Z0-9_]");
    q.bindValue(":salesaccnt_prodcat_id", _productCategories->id());
  }
  else
  {
    q.bindValue(":salesaccnt_prodcat", _productCategories->pattern());
    q.bindValue(":salesaccnt_prodcat_id", -1);
  }
  q.exec();
  if(q.first())
  {
    QMessageBox::warning(this, tr("Cannot Save Sales Account Assignment"),
      tr("You cannot specify a duplicate Warehouse/Customer Type/Product Category for the Sales Account Assignment."));
    _warehouse->setFocus();
    return;
  }

  if (_mode == cNew)
  {
    q.prepare( "INSERT INTO salesaccnt "
               "( salesaccnt_warehous_id,"
               "  salesaccnt_custtype, salesaccnt_custtype_id,"
               "  salesaccnt_prodcat, salesaccnt_prodcat_id,"
               "  salesaccnt_sales_accnt_id,"
               "  salesaccnt_credit_accnt_id,"
               "  salesaccnt_cos_accnt_id,"
	       "  salesaccnt_returns_accnt_id,"
	       "  salesaccnt_cor_accnt_id,"
	       "  salesaccnt_cow_accnt_id) "
               "VALUES "
               "( :salesaccnt_warehous_id,"
               "  :salesaccnt_custtype, :salesaccnt_custtype_id,"
               "  :salesaccnt_prodcat, :salesaccnt_prodcat_id,"
               "  :salesaccnt_sales_accnt_id,"
               "  :salesaccnt_credit_accnt_id,"
               "  :salesaccnt_cos_accnt_id,"
	       "  :salesaccnt_returns_accnt_id,"
	       "  :salesaccnt_cor_accnt_id,"
	       "  :salesaccnt_cow_accnt_id);" );
  }
  else if (_mode == cEdit)
    q.prepare( "UPDATE salesaccnt "
               "SET salesaccnt_warehous_id=:salesaccnt_warehous_id,"
               "    salesaccnt_custtype=:salesaccnt_custtype,"
               "    salesaccnt_custtype_id=:salesaccnt_custtype_id,"
               "    salesaccnt_prodcat=:salesaccnt_prodcat,"
               "    salesaccnt_prodcat_id=:salesaccnt_prodcat_id,"
               "    salesaccnt_sales_accnt_id=:salesaccnt_sales_accnt_id,"
               "    salesaccnt_credit_accnt_id=:salesaccnt_credit_accnt_id,"
               "    salesaccnt_cos_accnt_id=:salesaccnt_cos_accnt_id,"
               "    salesaccnt_returns_accnt_id=:salesaccnt_returns_accnt_id,"
               "    salesaccnt_cor_accnt_id=:salesaccnt_cor_accnt_id,"
               "    salesaccnt_cow_accnt_id=:salesaccnt_cow_accnt_id "
               "WHERE (salesaccnt_id=:salesaccnt_id);" );

  q.bindValue(":salesaccnt_id", _salesaccntid);
  q.bindValue(":salesaccnt_sales_accnt_id", _sales->id());
  q.bindValue(":salesaccnt_credit_accnt_id", _credit->id());
  q.bindValue(":salesaccnt_cos_accnt_id", _cos->id());
  if (_returns->isValid())
    q.bindValue(":salesaccnt_returns_accnt_id",	_returns->id());
  if (_cor->isValid())
    q.bindValue(":salesaccnt_cor_accnt_id",	_cor->id());
  if (_cow->isValid())
    q.bindValue(":salesaccnt_cow_accnt_id",	_cow->id());

  q.bindValue(":salesaccnt_warehous_id", _warehouse->id());

  if (_customerTypes->isAll())
  {
    q.bindValue(":salesaccnt_custtype", ".*");
    q.bindValue(":salesaccnt_custtype_id", -1);
  }
  else if (_customerTypes->isSelected())
  {
    q.bindValue(":salesaccnt_custtype", "[^a-zA-Z0-9_]");
    q.bindValue(":salesaccnt_custtype_id", _customerTypes->id());
  }
  else
  {
    q.bindValue(":salesaccnt_custtype", _customerTypes->pattern());
    q.bindValue(":salesaccnt_custtype_id", -1);
  }

  if (_productCategories->isAll())
  {
    q.bindValue(":salesaccnt_prodcat", ".*");
    q.bindValue(":salesaccnt_prodcat_id", -1);
  }
  else if (_productCategories->isSelected())
  {
    q.bindValue(":salesaccnt_prodcat", "[^a-zA-Z0-9_]");
    q.bindValue(":salesaccnt_prodcat_id", _productCategories->id());
  }
  else
  {
    q.bindValue(":salesaccnt_prodcat", _productCategories->pattern());
    q.bindValue(":salesaccnt_prodcat_id", -1);
  }

  q.exec();

  done(_salesaccntid);
}

void salesAccount::populate()
{
  q.prepare( "SELECT salesaccnt_warehous_id,"
             "       salesaccnt_custtype, salesaccnt_custtype_id,"
             "       salesaccnt_prodcat, salesaccnt_prodcat_id,"
             "       salesaccnt_sales_accnt_id, salesaccnt_credit_accnt_id,"
             "       salesaccnt_cos_accnt_id,"
             "       salesaccnt_returns_accnt_id,"
             "       salesaccnt_cor_accnt_id,"
             "       salesaccnt_cow_accnt_id "
             "FROM salesaccnt "
             "WHERE (salesaccnt_id=:salesaccnt_id);" );
  q.bindValue(":salesaccnt_id", _salesaccntid);
  q.exec();
  if (q.first())
  {
    _warehouse->setId(q.value("salesaccnt_warehous_id").toInt());

    if (q.value("salesaccnt_custtype_id").toInt() != -1)
      _customerTypes->setId(q.value("salesaccnt_custtype_id").toInt());
    else if (q.value("salesaccnt_custtype").toString() != ".*")
      _customerTypes->setPattern(q.value("salesaccnt_custtype").toString());
  
    if (q.value("salesaccnt_prodcat_id").toInt() != -1)
      _productCategories->setId(q.value("salesaccnt_prodcat_id").toInt());
    else if (q.value("salesaccnt_prodcat").toString() != ".*")
      _productCategories->setPattern(q.value("salesaccnt_prodcat").toString());
  
    _sales->setId(q.value("salesaccnt_sales_accnt_id").toInt());
    _credit->setId(q.value("salesaccnt_credit_accnt_id").toInt());
    _cos->setId(q.value("salesaccnt_cos_accnt_id").toInt());

    if (!q.value("salesaccnt_returns_accnt_id").isNull())
      _returns->setId(q.value("salesaccnt_returns_accnt_id").toInt());
    if (!q.value("salesaccnt_cor_accnt_id").isNull())
      _cor->setId(q.value("salesaccnt_cor_accnt_id").toInt());
    if (!q.value("salesaccnt_cow_accnt_id").isNull())
      _cow->setId(q.value("salesaccnt_cow_accnt_id").toInt());
  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}
