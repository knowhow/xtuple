

/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "department.h"

#include <QVariant>
#include <QMessageBox>
#include <QSqlError>

department::department(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  // signals and slots connections
  connect(_buttonBox, SIGNAL(rejected()), this, SLOT(sClose()));
  connect(_buttonBox, SIGNAL(accepted()), this, SLOT(sSave()));

  _deptid = -1;
}

department::~department()
{
  // no need to delete child widgets, Qt does it all for us
}

void department::languageChange()
{
  retranslateUi(this);
}

enum SetResponse department::set(const ParameterList& pParams)
{
  XDialog::set(pParams);
  QVariant        param;
  bool        valid;

  param = pParams.value("dept_id", &valid);
  if (valid)
  {
    _deptid = param.toInt();
    populate();
  }

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      _mode = cNew;
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
       _number->setEnabled(false);
       _name->setEnabled(false);
       _buttonBox->clear();
       _buttonBox->addButton(QDialogButtonBox::Close);
       _buttonBox->setFocus();
    }
  }

  return NoError;
}

void department::sSave()
{
  QString number = _number->text().trimmed().toUpper();

  if (number.isEmpty())
  {
    QMessageBox::critical(this, tr("Cannot Save Department"),
                          tr("You must enter a Department Number"));
    _number->setFocus();
    return;
  }
  if (_name->text().trimmed().isEmpty())
  {
    QMessageBox::critical(this, tr("Cannot Save Department"),
                          tr("You must enter a Department Name"));
    _name->setFocus();
    return;
  }

  q.prepare("SELECT dept_id"
            "  FROM dept"
            " WHERE((dept_id != :dept_id)"
            "   AND (dept_number=:dept_number));");
  q.bindValue(":dept_id", _deptid);
  q.bindValue(":dept_number", number);
  q.exec();
  if(q.first())
  {
    QMessageBox::critical(this, tr("Cannot Save Department"),
                          tr("The Number you entered already exists. Please choose a different Number."));
    return;
  }
  
  if (_mode == cNew)
  {
    q.exec("SELECT NEXTVAL('dept_dept_id_seq') AS dept_id;");
    if (q.first())
      _deptid =  q.value("dept_id").toInt();
    else
    {
      systemError(this, tr("A System Error occurred at %1::%2\n\n%3")
                          .arg(__FILE__)
                          .arg(__LINE__)
                          .arg(q.lastError().databaseText()));
      return;
    }
    q.prepare("INSERT INTO dept ( dept_id,  dept_number,  dept_name ) "
              "       VALUES    (:dept_id, :dept_number, :dept_name );");
  }
  else if (_mode == cEdit)
  {
    q.prepare("UPDATE dept "
              "SET dept_id=:dept_id, "
              "    dept_number=:dept_number, "
              "    dept_name=:dept_name "
              "WHERE (dept_id=:dept_id);");
  }
  q.bindValue(":dept_id",        _deptid);
  q.bindValue(":dept_number",        number);
  q.bindValue(":dept_name",        _name->text().trimmed());

  q.exec();
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, tr("A System Error occurred at %1::%2\n\n%3")
                        .arg(__FILE__)
                        .arg(__LINE__)
                        .arg(q.lastError().databaseText()));
    return;
  }

  done(_deptid);
}

void department::sClose()
{
  close();
}

void department::populate()
{
  q.prepare("SELECT dept_number, dept_name "
            "FROM dept "
            "WHERE (dept_id=:dept_id);");
  q.bindValue(":dept_id", _deptid);
  q.exec();
  if (q.first())
  {
    _number->setText(q.value("dept_number"));
    _name->setText(q.value("dept_name"));
  }
  else
    systemError(this, tr("A System Error occurred at %1::%2\n\n%3")
                        .arg(__FILE__)
                        .arg(__LINE__)
                        .arg(q.lastError().databaseText()));
}

