/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "incidentSeverity.h"

#include <QMessageBox>
#include <QSqlError>
#include <QValidator>
#include <QVariant>

/*
 *  Constructs a incidentSeverity as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
incidentSeverity::incidentSeverity(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
    setupUi(this);

    // signals and slots connections
    connect(_buttonBox, SIGNAL(accepted()), this, SLOT(sSave()));
    connect(_buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
    connect(_name, SIGNAL(lostFocus()), this, SLOT(sCheck()));
}

incidentSeverity::~incidentSeverity()
{
}

void incidentSeverity::languageChange()
{
    retranslateUi(this);
}

enum SetResponse incidentSeverity::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("incdtseverity_id", &valid);
  if (valid)
  {
    _incdtseverityId = param.toInt();
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

void incidentSeverity::sCheck()
{
  _name->setText(_name->text().trimmed());
  if ( (_mode == cNew) && (_name->text().length()) )
  {
    q.prepare( "SELECT incdtseverity_id "
               "FROM incdtseverity "
               "WHERE (UPPER(incdtseverity_name)=UPPER(:incdtseverity_name));" );
    q.bindValue(":incdtseverity_name", _name->text());
    q.exec();
    if (q.first())
    {
      _incdtseverityId = q.value("incdtseverity_id").toInt();
      _mode = cEdit;
      populate();

      _name->setEnabled(FALSE);
    }
  }
}

void incidentSeverity::sSave()
{
  if(_name->text().length() == 0)
  {
    QMessageBox::critical(this, tr("Severity Name Required"),
      tr("You must enter a Severity Name to continue.") );
    _name->setFocus();
    return;
  }

  if (_mode == cNew)
  {
    q.exec("SELECT NEXTVAL('incdtseverity_incdtseverity_id_seq') AS _incdtseverity_id");
    if (q.first())
      _incdtseverityId = q.value("_incdtseverity_id").toInt();
    else if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }

    q.prepare( "INSERT INTO incdtseverity "
               "(incdtseverity_id, incdtseverity_name, incdtseverity_order, incdtseverity_descrip)"
               " VALUES "
               "(:incdtseverity_id, :incdtseverity_name, :incdtseverity_order, :incdtseverity_descrip);" );
  }
  else if (_mode == cEdit)
  {
    q.prepare( "SELECT incdtseverity_id "
               "FROM incdtseverity "
               "WHERE ( (UPPER(incdtseverity_name)=UPPER(:incdtseverity_name))"
               " AND (incdtseverity_id<>:incdtseverity_id) );" );
    q.bindValue(":incdtseverity_id", _incdtseverityId);
    q.bindValue(":incdtseverity_name", _name->text());
    q.exec();
    if (q.first())
    {
      QMessageBox::warning( this, tr("Cannot Save Incident Severity"),
                            tr("You may not rename this Incident Severity with "
			       "the entered value as it is in use by another "
			       "Incident Severity.") );
      return;
    }
    else if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }

    q.prepare( "UPDATE incdtseverity "
               "SET incdtseverity_name=:incdtseverity_name, "
	       "    incdtseverity_order=:incdtseverity_order, "
	       "    incdtseverity_descrip=:incdtseverity_descrip "
               "WHERE (incdtseverity_id=:incdtseverity_id);" );
  }

  q.bindValue(":incdtseverity_id", _incdtseverityId);
  q.bindValue(":incdtseverity_name", _name->text());
  q.bindValue(":incdtseverity_order", _order->value());
  q.bindValue(":incdtseverity_descrip", _descrip->toPlainText());
  q.exec();
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  done(_incdtseverityId);
}

void incidentSeverity::populate()
{
  q.prepare( "SELECT * "
             "FROM incdtseverity "
             "WHERE (incdtseverity_id=:incdtseverity_id);" );
  q.bindValue(":incdtseverity_id", _incdtseverityId);
  q.exec();
  if (q.first())
  {
    _name->setText(q.value("incdtseverity_name").toString());
    _order->setValue(q.value("incdtseverity_order").toInt());
    _descrip->setText(q.value("incdtseverity_descrip").toString());
  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}
