/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "labelForm.h"

#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

labelForm::labelForm(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
    setupUi(this);

    connect(_buttonBox, SIGNAL(accepted()), this, SLOT(sSave()));
}

labelForm::~labelForm()
{
    // no need to delete child widgets, Qt does it all for us
}

void labelForm::languageChange()
{
    retranslateUi(this);
}

enum SetResponse labelForm::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("labelform_id", &valid);
  if (valid)
  {
    _labelformid = param.toInt();
    populate();
  }

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      _mode = cNew;
      _name->setFocus();
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;
      _buttonBox->setFocus();
    }
    else if (param.toString() == "view")
    {
      _mode = cView;

      _name->setEnabled(FALSE);
      _report->setEnabled(FALSE);
      _labelsPerPage->setEnabled(FALSE);
      _buttonBox->clear();
      _buttonBox->addButton(QDialogButtonBox::Close);
      _buttonBox->setFocus();
    }
  }

  return NoError;
}

void labelForm::sSave()
{
  if (_name->text().length() == 0)
  {
    QMessageBox::warning( this, tr("Format Name is Invalid"),
                          tr("<p>You must enter a valid name for this Label Format.") );
    _name->setFocus();
    return;
  }

  else if (_report->currentIndex() == -1)
  {
    QMessageBox::warning( this, tr("Report Name is Invalid"),
                          tr("<p>You must enter a select Report for this Label Format.") );
    _report->setFocus();
    return;
  }

  if (_mode == cNew)
  {
    q.prepare( "SELECT labelform_id "
         	     "FROM labelform "
               "WHERE (labelform_name=:labelform_name);" );
    q.bindValue(":labelform_name", _name->text());
    q.exec();
    if (q.first())
    {
      QMessageBox::warning( this, tr("Format Name is Invalid"),
                            tr("<p>This Label Form Name already exists.") );
      _name->setFocus();
      return;
    }
    else if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }

    q.exec("SELECT NEXTVAL('labelform_labelform_id_seq') AS _labelform_id");
    if (q.first())
      _labelformid = q.value("_labelform_id").toInt();
    else if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }

    q.prepare( "INSERT INTO labelform "
               "(labelform_id, labelform_name, labelform_report_name, labelform_perpage) "
               "VALUES "
               "(:labelform_id, :labelform_name, :labelform_report_name, :labelform_perpage);" );

  }
  if (_mode == cEdit)
    q.prepare( "UPDATE labelform "
               "SET labelform_name=:labelform_name, labelform_report_name=:labelform_report_name,"
               "    labelform_perpage=:labelform_perpage "
               "WHERE (labelform_id=:labelform_id);" );

  q.bindValue(":labelform_id", _labelformid);
  q.bindValue(":labelform_name", _name->text());
  q.bindValue(":labelform_report_name", _report->code());
  q.bindValue(":labelform_perpage", _labelsPerPage->value());
  q.exec();
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  done(_labelformid);
}

void labelForm::populate()
{
  q.prepare( "SELECT * "
       	     "FROM labelform "
	     "WHERE (labelform_id=:labelform_id);" );
  q.bindValue(":labelform_id", _labelformid);
  q.exec();
  if (q.first())
  {
    _name->setText(q.value("labelform_name").toString());
    _report->setCode(q.value("labelform_report_name").toString());
    _labelsPerPage->setValue(q.value("labelform_perpage").toInt());
  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}
