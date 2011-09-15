/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "incidentPriority.h"

#include <QMessageBox>
#include <QSqlError>
#include <QValidator>
#include <QVariant>

/*
 *  Constructs a incidentPriority as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
incidentPriority::incidentPriority(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
    setupUi(this);

    // signals and slots connections
    connect(_buttonBox, SIGNAL(accepted()), this, SLOT(sSave()));
    connect(_buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
    connect(_name, SIGNAL(lostFocus()), this, SLOT(sCheck()));
}

incidentPriority::~incidentPriority()
{
}

void incidentPriority::languageChange()
{
    retranslateUi(this);
}

enum SetResponse incidentPriority::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("incdtpriority_id", &valid);
  if (valid)
  {
    _incdtpriorityId = param.toInt();
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
      _name->setFocus();
    }
    else if (param.toString() == "view")
    {
      _mode = cView;

      _name->setEnabled(FALSE);
      _order->setEnabled(FALSE);
      _descrip->setEnabled(FALSE);
      _buttonBox->clear();
      _buttonBox->addButton(QDialogButtonBox::Close);
      _buttonBox->setFocus();
    }
  }

  return NoError;
}

void incidentPriority::sCheck()
{
  _name->setText(_name->text().trimmed());
  if ( (_mode == cNew) && (_name->text().length()) )
  {
    q.prepare( "SELECT incdtpriority_id "
               "FROM incdtpriority "
               "WHERE (UPPER(incdtpriority_name)=UPPER(:incdtpriority_name));" );
    q.bindValue(":incdtpriority_name", _name->text());
    q.exec();
    if (q.first())
    {
      _incdtpriorityId = q.value("incdtpriority_id").toInt();
      _mode = cEdit;
      populate();

      _name->setEnabled(FALSE);
    }
  }
}

void incidentPriority::sSave()
{
  if(_name->text().length() == 0)
  {
    QMessageBox::critical(this, tr("Priority Name Required"),
      tr("You must enter a Priority Name to continue.") );
    _name->setFocus();
    return;
  }

  if (_mode == cNew)
  {
    q.exec("SELECT NEXTVAL('incdtpriority_incdtpriority_id_seq') AS _incdtpriority_id");
    if (q.first())
      _incdtpriorityId = q.value("_incdtpriority_id").toInt();
    else if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }

    q.prepare( "INSERT INTO incdtpriority "
               "(incdtpriority_id, incdtpriority_name, incdtpriority_order, incdtpriority_descrip)"
               " VALUES "
               "(:incdtpriority_id, :incdtpriority_name, :incdtpriority_order, :incdtpriority_descrip);" );
  }
  else if (_mode == cEdit)
  {
    q.prepare( "SELECT incdtpriority_id "
               "FROM incdtpriority "
               "WHERE ( (UPPER(incdtpriority_name)=UPPER(:incdtpriority_name))"
               " AND (incdtpriority_id<>:incdtpriority_id) );" );
    q.bindValue(":incdtpriority_id", _incdtpriorityId);
    q.bindValue(":incdtpriority_name", _name->text());
    q.exec();
    if (q.first())
    {
      QMessageBox::warning( this, tr("Cannot Save Incident Priority"),
                            tr("You may not rename this Incident Priority with "
			       "the entered value as it is in use by another "
			       "Incident Priority.") );
      return;
    }
    else if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }

    q.prepare( "UPDATE incdtpriority "
               "SET incdtpriority_name=:incdtpriority_name, "
	       "    incdtpriority_order=:incdtpriority_order, "
	       "    incdtpriority_descrip=:incdtpriority_descrip "
               "WHERE (incdtpriority_id=:incdtpriority_id);" );
  }

  q.bindValue(":incdtpriority_id", _incdtpriorityId);
  q.bindValue(":incdtpriority_name", _name->text());
  q.bindValue(":incdtpriority_order", _order->value());
  q.bindValue(":incdtpriority_descrip", _descrip->toPlainText());
  q.exec();
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  done(_incdtpriorityId);
}

void incidentPriority::populate()
{
  q.prepare( "SELECT * "
             "FROM incdtpriority "
             "WHERE (incdtpriority_id=:incdtpriority_id);" );
  q.bindValue(":incdtpriority_id", _incdtpriorityId);
  q.exec();
  if (q.first())
  {
    _name->setText(q.value("incdtpriority_name").toString());
    _order->setValue(q.value("incdtpriority_order").toInt());
    _descrip->setText(q.value("incdtpriority_descrip").toString());
  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}
