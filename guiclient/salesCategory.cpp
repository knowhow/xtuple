/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "salesCategory.h"

#include <QVariant>
#include <QMessageBox>

salesCategory::salesCategory(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  _salescatid = -1;

  // signals and slots connections
  connect(_buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
  connect(_buttonBox, SIGNAL(accepted()), this, SLOT(sSave()));
  connect(_category, SIGNAL(lostFocus()), this, SLOT(sCheck()));

  _sales->setType(GLCluster::cRevenue);
  _prepaid->setType(GLCluster::cRevenue);
  _araccnt->setType(GLCluster::cAsset);
}

salesCategory::~salesCategory()
{
  // no need to delete child widgets, Qt does it all for us
}

void salesCategory::languageChange()
{
  retranslateUi(this);
}

enum SetResponse salesCategory::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("salescat_id", &valid);
  if (valid)
  {
    _salescatid = param.toInt();
    populate();
  }

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      _mode = cNew;
      _category->setFocus();
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;
      _category->setFocus();
    }
    else if (param.toString() == "copy")
    {
      _mode = cCopy;
      _salescatid = -1;
      _category->clear();
      _category->setFocus();
    }
    else if (param.toString() == "view")
    {
      _mode = cView;

      _category->setEnabled(FALSE);
      _active->setEnabled(FALSE);
      _description->setEnabled(FALSE);
      _sales->setReadOnly(TRUE);
      _prepaid->setReadOnly(TRUE);
      _araccnt->setReadOnly(TRUE);
      _buttonBox->clear();
      _buttonBox->addButton(QDialogButtonBox::Close);
      _buttonBox->setFocus();
    }
  }

  return NoError;
}

void salesCategory::sCheck()
{
  _category->setText(_category->text().trimmed().toUpper());
  if ((_mode == cNew) && (_category->text().length() != 0))
  {
    q.prepare( "SELECT salescat_id "
               "FROM salescat "
               "WHERE (UPPER(salescat_name)=:salescat_name);" );
    q.bindValue(":salescat_name", _category->text().trimmed());
    q.exec();
    if (q.first())
    {
      _salescatid = q.value("salescat_id").toInt();
      _mode = cEdit;
      populate();

      _category->setEnabled(FALSE);
    }
  }
}

void salesCategory::sSave()
{
  if(_category->text().trimmed().isEmpty())
  {
    QMessageBox::warning(this, tr("Cannot Save Sales Category"),
      tr("You must specify a name for the Sales Category."));
    _category->setFocus();
    return;
  }

  q.prepare("SELECT salescat_id"
            "  FROM salescat"
            " WHERE((salescat_name=:salescat_name)"
            "   AND (salescat_id != :salescat_id))");
  q.bindValue(":salescat_id", _salescatid);
  q.bindValue(":salescat_name", _category->text().trimmed());
  q.exec();
  if(q.first())
  {
    QMessageBox::warning(this, tr("Cannot Save Sales Category"),
      tr("You cannot specify a duplicate name for the Sales Category."));
    _category->setFocus();
    return;
  }

  if (!_sales->isValid())
  {
    QMessageBox::warning( this, tr("Cannot Save Sales Category"),
                          tr("You must select a Sales Account Number for this Sales Category before you may save it.") );
    _sales->setFocus();
    return;
  }

  if (!_prepaid->isValid())
  {
    QMessageBox::warning( this, tr("Cannot Save Sales Category"),
                          tr("You must select a Prepaid Account Number for this Sales Category before you may save it.") );
    _prepaid->setFocus();
    return;
  }

  if (!_araccnt->isValid())
  {
    QMessageBox::warning( this, tr("Cannot Save Sales Category"),
                          tr("You must select an A/R Account Number for this Sales Category before you may save it.") );
    _araccnt->setFocus();
    return;
  }

  if ( (_mode == cNew) || (_mode == cCopy) )
  {
    q.exec("SELECT NEXTVAL('salescat_salescat_id_seq') AS salescat_id");
    if (q.first())
      _salescatid = q.value("salescat_id").toInt();

    q.prepare( "INSERT INTO salescat"
               "( salescat_id, salescat_name, salescat_active, salescat_descrip,"
               "  salescat_sales_accnt_id, salescat_prepaid_accnt_id, salescat_ar_accnt_id ) "
               "VALUES "
               "( :salescat_id, :salescat_name, :salescat_active, :salescat_descrip,"
               "  :salescat_sales_accnt_id, :salescat_prepaid_accnt_id, :salescat_ar_accnt_id );" );
  }
  else if (_mode == cEdit)
    q.prepare( "UPDATE salescat "
               "SET salescat_name=:salescat_name, salescat_active=:salescat_active,"
               "    salescat_descrip=:salescat_descrip,"
               "    salescat_sales_accnt_id=:salescat_sales_accnt_id,"
               "    salescat_prepaid_accnt_id=:salescat_prepaid_accnt_id,"
               "    salescat_ar_accnt_id=:salescat_ar_accnt_id "
               "WHERE (salescat_id=:salescat_id);" );

  q.bindValue(":salescat_id", _salescatid);
  q.bindValue(":salescat_name", _category->text().trimmed());
  q.bindValue(":salescat_active", QVariant(_active->isChecked()));
  q.bindValue(":salescat_descrip", _description->text().trimmed());
  q.bindValue(":salescat_sales_accnt_id", _sales->id());
  q.bindValue(":salescat_prepaid_accnt_id", _prepaid->id());
  q.bindValue(":salescat_ar_accnt_id", _araccnt->id());
  q.exec();

  done(_salescatid);
}

void salesCategory::populate()
{
  q.prepare( "SELECT * "
             "FROM salescat "
             "WHERE (salescat_id=:salescat_id);" );
  q.bindValue(":salescat_id", _salescatid);
  q.exec();
  if (q.first())
  {
    if (_mode != cCopy)
    {
      _category->setText(q.value("salescat_name").toString());
      _description->setText(q.value("salescat_descrip").toString());
      _active->setChecked(q.value("salescat_active").toBool());
    }
    else
      _active->setChecked(TRUE);

    _sales->setId(q.value("salescat_sales_accnt_id").toInt());
    _prepaid->setId(q.value("salescat_prepaid_accnt_id").toInt());
    _araccnt->setId(q.value("salescat_ar_accnt_id").toInt());
  }
}
 
