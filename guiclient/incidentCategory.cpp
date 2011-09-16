/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "incidentCategory.h"

#include <QMessageBox>
#include <QSqlError>
#include <QValidator>
#include <QVariant>

/*
 *  Constructs a incidentCategory as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
incidentCategory::incidentCategory(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
    setupUi(this);

    // signals and slots connections
    connect(_buttonBox, SIGNAL(accepted()), this, SLOT(sSave()));
    connect(_buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
    connect(_name, SIGNAL(lostFocus()), this, SLOT(sCheck()));
    
    if (!_metrics->boolean("EnableBatchManager"))
    {
      _ediprofile->hide();
      _ediprofileLit->hide();
    }
    else
      _ediprofile->populate("SELECT ediprofile_id, ediprofile_name "
                            "FROM xtbatch.ediprofile "
                            "WHERE (ediprofile_type='email');");
}

incidentCategory::~incidentCategory()
{
}

void incidentCategory::languageChange()
{
    retranslateUi(this);
}

enum SetResponse incidentCategory::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("incdtcat_id", &valid);
  if (valid)
  {
    _incdtcatId = param.toInt();
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

void incidentCategory::sCheck()
{
  _name->setText(_name->text().trimmed());
  if ( (_mode == cNew) && (_name->text().length()) )
  {
    q.prepare( "SELECT incdtcat_id "
               "FROM incdtcat "
               "WHERE (UPPER(incdtcat_name)=UPPER(:incdtcat_name));" );
    q.bindValue(":incdtcat_name", _name->text());
    q.exec();
    if (q.first())
    {
      _incdtcatId = q.value("incdtcat_id").toInt();
      _mode = cEdit;
      populate();

      _name->setEnabled(FALSE);
    }
  }
}

void incidentCategory::sSave()
{
  if (_mode == cNew)
  {
    q.exec("SELECT NEXTVAL('incdtcat_incdtcat_id_seq') AS _incdtcat_id");
    if (q.first())
      _incdtcatId = q.value("_incdtcat_id").toInt();
    else if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }

    q.prepare( "INSERT INTO incdtcat "
               "(incdtcat_id, incdtcat_name, incdtcat_order, incdtcat_descrip, incdtcat_ediprofile_id)"
               " VALUES "
               "(:incdtcat_id, :incdtcat_name, :incdtcat_order, :incdtcat_descrip, :incdtcat_ediprofile_id );" );
  }
  else if (_mode == cEdit)
  {
    q.prepare( "SELECT incdtcat_id "
               "FROM incdtcat "
               "WHERE ( (UPPER(incdtcat_name)=UPPER(:incdtcat_name))"
               " AND (incdtcat_id<>:incdtcat_id) );" );
    q.bindValue(":incdtcat_id", _incdtcatId);
    q.bindValue(":incdtcat_name", _name->text());
    q.exec();
    if (q.first())
    {
      QMessageBox::warning( this, tr("Cannot Save Incident Category"),
                            tr("You may not rename this Incident Category with "
			       "the entered value as it is in use by another "
			       "Incident Category.") );
      return;
    }
    else if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }

    q.prepare( "UPDATE incdtcat "
               "SET incdtcat_name=:incdtcat_name, "
	       "    incdtcat_order=:incdtcat_order, "
	       "    incdtcat_descrip=:incdtcat_descrip, "
               "    incdtcat_ediprofile_id=:incdtcat_ediprofile_id "
               "WHERE (incdtcat_id=:incdtcat_id);" );
  }

  q.bindValue(":incdtcat_id", _incdtcatId);
  q.bindValue(":incdtcat_name", _name->text());
  q.bindValue(":incdtcat_order", _order->value());
  q.bindValue(":incdtcat_descrip", _descrip->toPlainText());
  if (_ediprofile->id() != -1)
    q.bindValue(":incdtcat_ediprofile_id", _ediprofile->id());
  q.exec();
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  done(_incdtcatId);
}

void incidentCategory::populate()
{
  q.prepare( "SELECT *,COALESCE(incdtcat_ediprofile_id,-1) AS ediprofile "
             "FROM incdtcat "
             "WHERE (incdtcat_id=:incdtcat_id);" );
  q.bindValue(":incdtcat_id", _incdtcatId);
  q.exec();
  if (q.first())
  {
    _name->setText(q.value("incdtcat_name").toString());
    _order->setValue(q.value("incdtcat_order").toInt());
    _descrip->setText(q.value("incdtcat_descrip").toString());
    _ediprofile->setId(q.value("ediprofile").toInt());
  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}
