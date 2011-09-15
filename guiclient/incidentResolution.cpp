/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "incidentResolution.h"

#include <QMessageBox>
#include <QSqlError>
#include <QValidator>
#include <QVariant>

/*
 *  Constructs a incidentResolution as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
incidentResolution::incidentResolution(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
    setupUi(this);

    // signals and slots connections
    connect(_buttonBox, SIGNAL(accepted()), this, SLOT(sSave()));
    connect(_buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
    connect(_name, SIGNAL(lostFocus()), this, SLOT(sCheck()));
}

incidentResolution::~incidentResolution()
{
}

void incidentResolution::languageChange()
{
    retranslateUi(this);
}

enum SetResponse incidentResolution::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("incdtresolution_id", &valid);
  if (valid)
  {
    _incdtresolutionId = param.toInt();
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

void incidentResolution::sCheck()
{
  _name->setText(_name->text().trimmed());
  if ( (_mode == cNew) && (_name->text().length()) )
  {
    q.prepare( "SELECT incdtresolution_id "
               "FROM incdtresolution "
               "WHERE (UPPER(incdtresolution_name)=UPPER(:incdtresolution_name));" );
    q.bindValue(":incdtresolution_name", _name->text());
    q.exec();
    if (q.first())
    {
      _incdtresolutionId = q.value("incdtresolution_id").toInt();
      _mode = cEdit;
      populate();

      _name->setEnabled(FALSE);
    }
  }
}

void incidentResolution::sSave()
{
  if(_name->text().length() == 0)
  {
    QMessageBox::critical(this, tr("Resolution Name Required"),
      tr("You must enter a Resolution Name to continue.") );
    _name->setFocus();
    return;
  }

  if (_mode == cNew)
  {
    q.exec("SELECT NEXTVAL('incdtresolution_incdtresolution_id_seq') AS _incdtresolution_id");
    if (q.first())
      _incdtresolutionId = q.value("_incdtresolution_id").toInt();
    else if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }

    q.prepare( "INSERT INTO incdtresolution "
               "(incdtresolution_id, incdtresolution_name, incdtresolution_order, incdtresolution_descrip)"
               " VALUES "
               "(:incdtresolution_id, :incdtresolution_name, :incdtresolution_order, :incdtresolution_descrip);" );
  }
  else if (_mode == cEdit)
  {
    q.prepare( "SELECT incdtresolution_id "
               "FROM incdtresolution "
               "WHERE ( (UPPER(incdtresolution_name)=UPPER(:incdtresolution_name))"
               " AND (incdtresolution_id<>:incdtresolution_id) );" );
    q.bindValue(":incdtresolution_id", _incdtresolutionId);
    q.bindValue(":incdtresolution_name", _name->text());
    q.exec();
    if (q.first())
    {
      QMessageBox::warning( this, tr("Cannot Save Incident Resolution"),
                            tr("You may not rename this Incident Resolution with "
			       "the entered value as it is in use by another "
			       "Incident Resolution.") );
      return;
    }
    else if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }

    q.prepare( "UPDATE incdtresolution "
               "SET incdtresolution_name=:incdtresolution_name, "
	       "    incdtresolution_order=:incdtresolution_order, "
	       "    incdtresolution_descrip=:incdtresolution_descrip "
               "WHERE (incdtresolution_id=:incdtresolution_id);" );
  }

  q.bindValue(":incdtresolution_id", _incdtresolutionId);
  q.bindValue(":incdtresolution_name", _name->text());
  q.bindValue(":incdtresolution_order", _order->value());
  q.bindValue(":incdtresolution_descrip", _descrip->toPlainText());
  q.exec();
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  done(_incdtresolutionId);
}

void incidentResolution::populate()
{
  q.prepare( "SELECT * "
             "FROM incdtresolution "
             "WHERE (incdtresolution_id=:incdtresolution_id);" );
  q.bindValue(":incdtresolution_id", _incdtresolutionId);
  q.exec();
  if (q.first())
  {
    _name->setText(q.value("incdtresolution_name").toString());
    _order->setValue(q.value("incdtresolution_order").toInt());
    _descrip->setText(q.value("incdtresolution_descrip").toString());
  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}
