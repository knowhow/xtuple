/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "taxType.h"

#include <QVariant>
#include <QMessageBox>

taxType::taxType(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
  : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  // signals and slots connections
  connect(_save, SIGNAL(clicked()), this, SLOT(sSave()));
  connect(_close, SIGNAL(clicked()), this, SLOT(reject()));
  connect(_name, SIGNAL(lostFocus()), this, SLOT(sCheck()));
}

taxType::~taxType()
{
  // no need to delete child widgets, Qt does it all for us
}

void taxType::languageChange()
{
  retranslateUi(this);
}

enum SetResponse taxType::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("taxtype_id", &valid);
  if (valid)
  {
    _taxtypeid = param.toInt();
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

      _save->setFocus();
    }
    else if (param.toString() == "view")
    {
      _mode = cView;

      _name->setEnabled(FALSE);
      _description->setEnabled(FALSE);
      _close->setText(tr("&Close"));
      _save->hide();

      _close->setFocus();
    }
  }

  return NoError;
}

void taxType::sCheck()
{
  _name->setText(_name->text().trimmed());
  if ( (_mode == cNew) && (_name->text().length()) )
  {
    q.prepare( "SELECT taxtype_id "
               "FROM taxtype "
               "WHERE (UPPER(taxtype_name)=UPPER(:taxtype_name));" );
    q.bindValue(":taxtype_name", _name->text());
    q.exec();
    if (q.first())
    {
      _taxtypeid = q.value("taxtype_id").toInt();
      _mode = cEdit;
      populate();

      _name->setEnabled(FALSE);
    }
  }
}

void taxType::sSave()
{
  if (_name->text().trimmed().isEmpty())
  {
    QMessageBox::critical(this, tr("Missing Name"),
			  tr("<p>You must name this Tax Type before saving it."));
    _name->setFocus();
    return;
  }

  if (_mode == cEdit)
  {
    q.prepare( "SELECT taxtype_id "
               "FROM taxtype "
               "WHERE ( (taxtype_id<>:taxtype_id)"
               " AND (UPPER(taxtype_name)=UPPER(:taxtype_name)) );");
    q.bindValue(":taxtype_id", _taxtypeid);
    q.bindValue(":taxtype_name", _name->text().trimmed());
    q.exec();
    if (q.first())
    {
      QMessageBox::critical( this, tr("Cannot Create Tax Type"),
                             tr( "A Tax Type with the entered name already exists."
                                 "You may not create a Tax Type with this name." ) );
      _name->setFocus();
      return;
    }

    q.prepare( "UPDATE taxtype "
               "SET taxtype_name=:taxtype_name,"
               "    taxtype_descrip=:taxtype_descrip "
               "WHERE (taxtype_id=:taxtype_id);" );
    q.bindValue(":taxtype_id", _taxtypeid);
    q.bindValue(":taxtype_name", _name->text().trimmed());
    q.bindValue(":taxtype_descrip", _description->text());
    q.exec();
  }
  else if (_mode == cNew)
  {
    q.prepare( "SELECT taxtype_id "
               "FROM taxtype "
               "WHERE (UPPER(taxtype_name)=UPPER(:taxtype_name));");
    q.bindValue(":taxtype_name", _name->text().trimmed());
    q.exec();
    if (q.first())
    {
      QMessageBox::critical( this, tr("Cannot Create Tax Type"),
                             tr( "A Tax Type with the entered name already exists.\n"
                                 "You may not create a Tax Type with this name." ) );
      _name->setFocus();
      return;
    }

    q.exec("SELECT NEXTVAL('taxtype_taxtype_id_seq') AS taxtype_id;");
    if (q.first())
      _taxtypeid = q.value("taxtype_id").toInt();
    else
    {
      systemError(this, tr("A System Error occurred at %1::%2.")
                        .arg(__FILE__)
                        .arg(__LINE__) );
      return;
    }

    q.prepare( "INSERT INTO taxtype "
               "( taxtype_id, taxtype_name, taxtype_descrip ) "
               "VALUES "
               "( :taxtype_id, :taxtype_name, :taxtype_descrip );" );
    q.bindValue(":taxtype_id", _taxtypeid);
    q.bindValue(":taxtype_name", _name->text().trimmed());
    q.bindValue(":taxtype_descrip", _description->text());
    q.exec();
  }

  done(_taxtypeid);
}

void taxType::populate()
{
  q.prepare( "SELECT taxtype_name, taxtype_descrip, taxtype_sys "
             "FROM taxtype "
             "WHERE (taxtype_id=:taxtype_id);" );
  q.bindValue(":taxtype_id", _taxtypeid);
  q.exec();
  if (q.first())
  {
    _name->setText(q.value("taxtype_name").toString());
    if(q.value("taxtype_sys").toBool())
      _name->setEnabled(false);
    _description->setText(q.value("taxtype_descrip").toString());
  }
}

