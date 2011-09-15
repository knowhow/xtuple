/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "taxClass.h"

#include <QVariant>
#include <QMessageBox>
#include <QSqlError>
#include <QCloseEvent>

#include "storedProcErrorLookup.h"

/*
 *  Constructs a taxClass as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
taxClass::taxClass(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  // signals and slots connections
  connect(_save, SIGNAL(clicked()), this, SLOT(sSave()));
  connect(_taxClass, SIGNAL(lostFocus()), this, SLOT(sCheck()));
}

/*
 *  Destroys the object and frees any allocated resources
 */
taxClass::~taxClass()
{
  // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void taxClass::languageChange()
{
  retranslateUi(this);
}

enum SetResponse taxClass::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("taxclass_id", &valid);
  if (valid)
  {
    _taxclassid = param.toInt();
    populate();
  }

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
		_mode = cNew;
      
		_taxClass->setFocus();
    }
	
    else if (param.toString() == "edit")
    {
      _mode = cEdit;

	  _save->setFocus();
    }
    else if (param.toString() == "view")
    {
      _mode = cView;

      _taxClass->setEnabled(FALSE);
      _description->setEnabled(FALSE);
	  _seq->setEnabled(FALSE);
      _close->setText(tr("&Close"));
      _save->hide();

      _close->setFocus();
    }
  }

  return NoError;
}

void taxClass::sCheck()
{
  _taxClass->setText(_taxClass->text().trimmed());
  if ( (_mode == cNew) && (_taxClass->text().length()) )
  {
	q.prepare( "SELECT taxclass_id "
               "FROM taxclass "
               "WHERE (UPPER(taxclass_code) = UPPER(:taxclass_code));" );
    q.bindValue(":taxclass_code", _taxClass->text());
    q.exec();
    if (q.first())
    {
      _taxclassid = q.value("taxclass_id").toInt();
      _mode = cEdit;
      populate();

      _taxClass->setEnabled(FALSE);
    }
  }
}

void taxClass::sSave()
{
  if (_taxClass->text().length() == 0)
  {
      QMessageBox::warning( this, tr("Cannot Save Tax Class"),
                            tr("You must enter a valid Code.") );
      return;
  }
  
  if (_mode == cEdit)
  {
    q.prepare( "SELECT taxclass_id "
               "FROM taxclass "
               "WHERE ( (taxclass_id <> :taxclass_id)"
               " AND (UPPER(taxclass_code) = UPPER(:taxclass_code)) );");
    q.bindValue(":taxclass_id", _taxclassid);
  }
  else
  {
    q.prepare( "SELECT taxclass_id "
               "FROM taxclass "
               "WHERE (taxclass_code = :taxclass_code);");
  }
  q.bindValue(":taxclass_code", _taxClass->text().trimmed());
  q.exec();
  if (q.first())
  {
    QMessageBox::critical( this, tr("Cannot Create Tax Class"),
			   tr( "A Tax Class with the entered code already exists."
			       "You may not create a Tax Class with this code." ) );
    _taxClass->setFocus();
    return;
  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  XSqlQuery rollback;
  rollback.prepare("ROLLBACK;");

  q.exec("BEGIN;");

  if (_mode == cEdit)
  {
    q.prepare( "UPDATE taxclass "
               "SET taxclass_code=:taxclass_code,"
               "    taxclass_descrip=:taxclass_descrip, "
			   "    taxclass_sequence=:taxclass_sequence "
               "WHERE (taxclass_id=:taxclass_id);" );
  }
  else if (_mode == cNew)
  {
    q.exec("SELECT NEXTVAL('taxclass_taxclass_id_seq') AS taxclass_id;");
    if (q.first())
      _taxclassid = q.value("taxclass_id").toInt();
    else if (q.lastError().type() != QSqlError::NoError)
    {
      rollback.exec();
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }

    q.prepare( "INSERT INTO taxclass "
               "(taxclass_id, taxclass_code, taxclass_descrip, taxclass_sequence)" 
               "VALUES "
               "(:taxclass_id, :taxclass_code, :taxclass_descrip, :taxclass_sequence);" ); 
  }
  q.bindValue(":taxclass_id", _taxclassid);
  q.bindValue(":taxclass_code", _taxClass->text().trimmed());
  q.bindValue(":taxclass_descrip", _description->text());
  q.bindValue(":taxclass_sequence", _seq->value());
  q.exec();
  if (q.lastError().type() != QSqlError::NoError)
  {
    rollback.exec();
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  q.exec("COMMIT;");

  done(_taxclassid);
}

void taxClass::populate()
{
  q.prepare( "SELECT taxclass_code, taxclass_descrip, taxclass_sequence "
             "FROM taxclass "
             "WHERE (taxclass_id=:taxclass_id);" );
  q.bindValue(":taxclass_id", _taxclassid);
  q.exec();
  if (q.first())
  {
    _taxClass->setText(q.value("taxclass_code").toString());
    _description->setText(q.value("taxclass_descrip").toString());
	_seq->setValue(q.value("taxclass_sequence").toInt());
  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void taxClass::done( int r )
{
  XDialog::done( r );
  close();
}
