/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "taxZone.h"

#include <QVariant>
#include <QMessageBox>
#include <QSqlError>
#include <QCloseEvent>

#include "storedProcErrorLookup.h"

/*
 *  Constructs a taxZone as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
taxZone::taxZone(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  // signals and slots connections
  connect(_save, SIGNAL(clicked()), this, SLOT(sSave()));
  connect(_taxZone, SIGNAL(lostFocus()), this, SLOT(sCheck()));
}

/*
 *  Destroys the object and frees any allocated resources
 */
taxZone::~taxZone()
{
  // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void taxZone::languageChange()
{
  retranslateUi(this);
}

enum SetResponse taxZone::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("taxzone_id", &valid);
  if (valid)
  {
    _taxzoneid = param.toInt();
    populate();
  }

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
		_mode = cNew;
      
		_taxZone->setFocus();
    }
	
    else if (param.toString() == "edit")
    {
      _mode = cEdit;

	  _save->setFocus();
    }
    else if (param.toString() == "view")
    {
      _mode = cView;

      _taxZone->setEnabled(FALSE);
      _description->setEnabled(FALSE);
      _close->setText(tr("&Close"));
      _save->hide();

      _close->setFocus();
    }
  }

  return NoError;
}

void taxZone::sCheck()
{
  _taxZone->setText(_taxZone->text().trimmed());
  if ( (_mode == cNew) && (_taxZone->text().length()) )
  {
	q.prepare( "SELECT taxzone_id "
               "FROM taxzone "
               "WHERE (UPPER(taxzone_code)=UPPER(:taxzone_code));" );
    q.bindValue(":taxzone_code", _taxZone->text());
    q.exec();
    if (q.first())
    {
      _taxzoneid = q.value("taxzone_id").toInt();
      _mode = cEdit;
      populate();

      _taxZone->setEnabled(FALSE);
    }
  }
}

void taxZone::sSave()
{
  if (_taxZone->text().length() == 0)
  {
      QMessageBox::warning( this, tr("Cannot Save Tax Zone"),
                            tr("You must enter a valid Code.") );
      return;
  }
  
  if (_mode == cEdit)
  {
    q.prepare( "SELECT taxzone_id "
               "FROM taxzone "
               "WHERE ( (taxzone_id<>:taxzone_id)"
               " AND (UPPER(taxzone_code)=UPPER(:taxzone_code)) );");
    q.bindValue(":taxzone_id", _taxzoneid);
  }
  else
  {
    q.prepare( "SELECT taxzone_id "
               "FROM taxzone "
               "WHERE (taxzone_code=:taxzone_code);");
  }
  q.bindValue(":taxzone_code", _taxZone->text().trimmed());
  q.exec();
  if (q.first())
  {
    QMessageBox::critical( this, tr("Cannot Create Tax Zone"),
			   tr( "A Tax Zone with the entered code already exists."
			       "You may not create a Tax Zone with this code." ) );
    _taxZone->setFocus();
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
    q.prepare( "UPDATE taxzone "
               "SET taxzone_code=:taxzone_code,"
               "    taxzone_descrip=:taxzone_descrip "
               "WHERE (taxzone_id=:taxzone_id);" );
  }
  else if (_mode == cNew)
  {
    q.exec("SELECT NEXTVAL('taxzone_taxzone_id_seq') AS taxzone_id;");
    if (q.first())
      _taxzoneid = q.value("taxzone_id").toInt();
    else if (q.lastError().type() != QSqlError::NoError)
    {
      rollback.exec();
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }

    q.prepare( "INSERT INTO taxzone "
               "(taxzone_id, taxzone_code, taxzone_descrip)" 
               "VALUES "
               "(:taxzone_id, :taxzone_code, :taxzone_descrip);" ); 
  }
  q.bindValue(":taxzone_id", _taxzoneid);
  q.bindValue(":taxzone_code", _taxZone->text().trimmed());
  q.bindValue(":taxzone_descrip", _description->text());
  q.exec();
  if (q.lastError().type() != QSqlError::NoError)
  {
    rollback.exec();
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  q.exec("COMMIT;");

  done(_taxzoneid);
}

void taxZone::populate()
{
  q.prepare( "SELECT taxzone_code, taxzone_descrip "
             "FROM taxzone " 
             "WHERE (taxzone_id=:taxzone_id);" );
  q.bindValue(":taxzone_id", _taxzoneid);
  q.exec();
  if (q.first())
  {
    _taxZone->setText(q.value("taxzone_code").toString());
    _description->setText(q.value("taxzone_descrip").toString());
	
  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void taxZone::done( int r )
{
  XDialog::done( r );
  close();
}
