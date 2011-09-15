/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "customerFormAssignment.h"

#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

customerFormAssignment::customerFormAssignment(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_buttonBox, SIGNAL(accepted()), this, SLOT(sSave()));
}

customerFormAssignment::~customerFormAssignment()
{
  // no need to delete child widgets, Qt does it all for us
}

void customerFormAssignment::languageChange()
{
  retranslateUi(this);
}

enum SetResponse customerFormAssignment::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("custform_id", &valid);
  if (valid)
  {
    _custformid = param.toInt();
    populate();
  }

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      _mode = cNew;
      _customerType->setFocus();
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;
      _buttonBox->setFocus();
    }
    else if (param.toString() == "view")
    {
      _mode = cView;

      _customerTypeGroup->setEnabled(FALSE);
      _invoiceForm->setEnabled(FALSE);
      _creditMemoForm->setEnabled(FALSE);
      _statementForm->setEnabled(FALSE);
      _quoteForm->setEnabled(FALSE);
      _packingListForm->setEnabled(FALSE);
      _soPickListForm->setEnabled(FALSE);
      _buttonBox->clear();
      _buttonBox->addButton(QDialogButtonBox::Close);
      _buttonBox->setFocus();
    }
  }

  return NoError;
}

void customerFormAssignment::sSave()
{
  int custtypeid   = -1;
  QString custtype = ""; 

  if (_selectedCustomerType->isChecked())
    custtypeid = _customerTypes->id();
  else if (_customerTypePattern->isChecked())
    custtype = _customerType->text().trimmed();

  q.prepare("SELECT custform_id"
            "  FROM custform"
            " WHERE((custform_id != :custform_id)"
            "   AND (custform_custtype = :custform_custtype)"
            "   AND (custform_custtype_id=:custform_custtype_id))");
  q.bindValue(":custform_id", _custformid);
  q.bindValue(":custform_custtype", custtype);
  q.bindValue(":custform_custtype_id", custtypeid);
  q.exec();
  if(q.first())
  {
    QMessageBox::critical(this, tr("Duplicate Entry"),
      tr("The Customer Type specified is already in the database.") );
    return;
  }

  if (_mode == cNew)
  {
    q.exec("SELECT NEXTVAL('custform_custform_id_seq') AS _custformid");
    if (q.first())
      _custformid = q.value("_custformid").toInt();
    else if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }

    q.prepare( "INSERT INTO custform "
               "( custform_id, custform_custtype, custform_custtype_id,"
               "  custform_invoice_report_name, custform_creditmemo_report_name,"
               "  custform_statement_report_name, custform_quote_report_name,"
               "   custform_packinglist_report_name, custform_sopicklist_report_name ) "
               "VALUES "
               "( :custform_id, :custform_custtype, :custform_custtype_id,"
               "  :custform_invoice_report_name, :custform_creditmemo_report_name,"
               "  :custform_statement_report_name, :custform_quote_report_name,"
               "  :custform_packinglist_report_name, :custform_sopicklist_report_name );" );
  }
  else if (_mode == cEdit)
    q.prepare( "UPDATE custform "
               "SET custform_custtype=:custform_custtype, custform_custtype_id=:custform_custtype_id,"
               "    custform_invoice_report_name=:custform_invoice_report_name,"
               "    custform_creditmemo_report_name=:custform_creditmemo_report_name,"
               "    custform_statement_report_name=:custform_statement_report_name,"
               "    custform_quote_report_name=:custform_quote_report_name,"
               "    custform_packinglist_report_name=:custform_packinglist_report_name,"
	       "    custform_sopicklist_report_name=:custform_sopicklist_report_name "
               "WHERE (custform_id=:custform_id);" );

  q.bindValue(":custform_id", _custformid);
  q.bindValue(":custform_custtype", custtype);
  q.bindValue(":custform_custtype_id", custtypeid);
  q.bindValue(":custform_invoice_report_name", _invoiceForm->code());
  q.bindValue(":custform_creditmemo_report_name", _creditMemoForm->code());
  q.bindValue(":custform_statement_report_name", _statementForm->code());
  q.bindValue(":custform_quote_report_name", _quoteForm->code());
  q.bindValue(":custform_packinglist_report_name", _packingListForm->code());
  q.bindValue(":custform_sopicklist_report_name", _soPickListForm->code());
  q.exec();
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  done(_custformid);
}

void customerFormAssignment::populate()
{
  q.prepare( "SELECT custform_custtype_id, custform_custtype,"
             "       custform_invoice_report_name, custform_creditmemo_report_name,"
             "       custform_statement_report_name, custform_quote_report_name,"
             "       custform_packinglist_report_name, custform_sopicklist_report_name "
             "FROM custform "
             "WHERE (custform_id=:custform_id);" );
  q.bindValue(":custform_id", _custformid);
  q.exec();
  if (q.first())
  {
    if (q.value("custform_custtype_id").toInt() == -1)
    {
      _customerTypePattern->setChecked(TRUE);
      _customerType->setText(q.value("custform_custtype").toString());
    }
    else
    {
      _selectedCustomerType->setChecked(TRUE);
      _customerTypes->setId(q.value("custform_custtype_id").toInt());
    }

    _invoiceForm->setCode(q.value("custform_invoice_report_name").toString());
    _creditMemoForm->setCode(q.value("custform_creditmemo_report_name").toString());
    _statementForm->setCode(q.value("custform_statement_report_name").toString());
    _quoteForm->setCode(q.value("custform_quote_report_name").toString());
    _packingListForm->setCode(q.value("custform_packinglist_report_name").toString());
    _soPickListForm->setCode(q.value("custform_sopicklist_report_name").toString());
  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}
