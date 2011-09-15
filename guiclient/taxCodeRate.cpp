/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "taxCodeRate.h"

#include <QMessageBox>
#include <QSqlError>

taxCodeRate::taxCodeRate(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_save, SIGNAL(clicked()), this, SLOT(sSave()));
  
  _dates->setStartNull(tr("Always"), omfgThis->startOfTime(), TRUE);
  _dates->setEndNull(tr("Never"), omfgThis->endOfTime(), TRUE);
  
  _percent->setValidator(omfgThis->negPercentVal());
}

taxCodeRate::~taxCodeRate()
{
  // no need to delete child widgets, Qt does it all for us
}

void taxCodeRate::languageChange()
{
  retranslateUi(this);
}

enum SetResponse taxCodeRate::set( const ParameterList & pParams )
{
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("taxrate_id", &valid);
  if (valid)
    _taxrateid = param.toInt();
  
  param = pParams.value("tax_id", &valid);
  if (valid)
    _taxId = param.toInt(); 

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      _mode = cNew;
	  q.prepare(" (SELECT taxrate_id "
                " FROM taxrate "
                " WHERE taxrate_tax_id = :taxrate_tax_id); ");
	  q.bindValue(":taxrate_tax_id", _taxId);
	  q.exec();
	  if(q.first())
      {
	    XSqlQuery maxdate;
		maxdate.prepare(" (SELECT (MAX(taxrate_expires) + 1) AS max_expires"
                        " FROM taxrate "
                        " WHERE taxrate_tax_id = :taxrate_tax_id) ");
	    maxdate.bindValue(":taxrate_tax_id", _taxId);
	    maxdate.exec();
	    if(maxdate.first())
		{
		  _dates->setStartDate(maxdate.value("max_expires").toDate());
		}
		else if (maxdate.lastError().type() != QSqlError::NoError)
        {
	      systemError(this, maxdate.lastError().databaseText(), __FILE__, __LINE__);
          return UndefinedError;
        }
	  }
	  _dates->setFocus();
      
      q.exec("SELECT NEXTVAL('taxrate_taxrate_id_seq') AS taxrate_id");
      if (q.first())
        _taxrateid = q.value("taxrate_id").toInt();
      else if (q.lastError().type() != QSqlError::NoError)
      {
	    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
		return UndefinedError;
      }
	}
    else if (param.toString() == "edit")
    {
      _mode = cEdit;
	  sPopulate();
      
      _save->setFocus();
    }
    else if (param.toString() == "view")
    {
      _mode = cView;
	  sPopulate();

      _dates->setEnabled(FALSE);
	  _percent->setEnabled(FALSE);
	  _flat->setEnabled(FALSE);
      _close->setText(tr("&Close"));
      _save->hide();
      
      _close->setFocus();
    }
  }
  
  return NoError;
}

void taxCodeRate::sSave()
{ 
  // Check if start date is greater than end date
  if( _dates->startDate() > _dates->endDate()) 
  {
	QMessageBox::critical(this, tr("Incorrect Date Entry"),
		tr("The start date should be earlier than the end date.") );
	_dates->setFocus();
	return;
  }
  // Check for overlapping dates
  q.prepare("SELECT taxrate_id,taxrate_tax_id,taxrate_effective,taxrate_expires "
            "  FROM taxrate "
            " WHERE (taxrate_id != :taxrate_id) "
			"   AND (taxrate_tax_id = :taxrate_tax_id) "
			"   AND (taxrate_effective < :taxrate_expires) "
			"   AND (taxrate_expires > :taxrate_effective ) ");
  q.bindValue(":taxrate_id", _taxrateid);
  q.bindValue(":taxrate_tax_id", _taxId);
  q.bindValue(":taxrate_effective", _dates->startDate());
  q.bindValue(":taxrate_expires", _dates->endDate());
  q.exec();
  if(q.first())
  {
    QMessageBox::critical(this, tr("Invalid Date Range"),
      tr("A Tax Rate already exists within the specified Date Range.") );
    _dates->setFocus();
    return;
  }
  
  // Save the values in the database
  if (cNew == _mode) 
  {
    q.prepare("INSERT INTO taxrate (taxrate_id, "
	      "    taxrate_tax_id, taxrate_percent, "
	      "    taxrate_curr_id, taxrate_amount, "
		  "    taxrate_effective, taxrate_expires) "
	      "    VALUES (:taxrate_id, :taxrate_tax_id, "
	      "    :taxrate_percent, :taxrate_curr_id, "
	      "    :taxrate_amount, :taxrate_effective, "
		  "    :taxrate_expires);");
  }
  else 
  {
    q.prepare("UPDATE taxrate SET "
	      "    taxrate_tax_id=:taxrate_tax_id, "
	      "    taxrate_percent=:taxrate_percent, "
	      "    taxrate_curr_id=:taxrate_curr_id, "
		  "    taxrate_amount=:taxrate_amount, "
		  "    taxrate_effective=:taxrate_effective, "
		  "	   taxrate_expires=:taxrate_expires "
		  "WHERE (taxrate_id=:taxrate_id);");
  }
  q.bindValue(":taxrate_id", _taxrateid);
  q.bindValue(":taxrate_tax_id", _taxId);

  if(_percent->isValid())
    q.bindValue(":taxrate_percent", (_percent->toDouble() / 100));
  else
    q.bindValue(":taxrate_percent", 0.0);

  q.bindValue(":taxrate_curr_id", _flat->id());
  if(_flat->isEmpty())
    q.bindValue(":taxrate_amount", 0.0);
  else
    q.bindValue(":taxrate_amount", _flat->localValue());
  q.bindValue(":taxrate_effective", _dates->startDate());
  q.bindValue(":taxrate_expires", _dates->endDate()); 

  q.exec();
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  done (_taxrateid);
}

void taxCodeRate::sPopulate()
{
  q.prepare("SELECT * FROM taxrate WHERE (taxrate_id=:taxrate_id);");
  q.bindValue(":taxrate_id", _taxrateid);
  q.exec();
  if (q.first())
  { 
    _taxrateid	= q.value("taxrate_id").toInt();
	_dates->setStartDate(q.value("taxrate_effective").toDate()); 
	_dates->setEndDate(q.value("taxrate_expires").toDate());
	_percent->setText(q.value("taxrate_percent").toDouble() * 100);
	_flat->setId(q.value("taxrate_curr_id").toInt());
	_flat->setLocalValue(q.value("taxrate_amount").toDouble());
  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}
