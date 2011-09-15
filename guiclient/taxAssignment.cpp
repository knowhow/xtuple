/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "taxAssignment.h"

#include <QCloseEvent>
#include <QSqlError>
#include <QMessageBox>

#include "metasql.h"
#include "parameter.h"

taxAssignment::taxAssignment(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
  : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  _taxCodeOption->addColumn(tr("Tax Code"),	 -1,  Qt::AlignLeft,   true,  "taxcode_option"  );
  _taxCodeSelected->addColumn(tr("Tax Code"),	 -1,  Qt::AlignLeft,   true,  "taxcode_selected"  );

  connect(_taxCodeOption, SIGNAL(itemSelected(int)), this, SLOT(sAdd()));
  connect(_taxCodeSelected, SIGNAL(itemSelected(int)), this, SLOT(sRevoke()));

  connect(_taxCodeOption,	SIGNAL(valid(bool)),	_add, SLOT(setEnabled(bool)));
  connect(_taxCodeSelected,	SIGNAL(valid(bool)),	_revoke, SLOT(setEnabled(bool)));

  connect(_add, SIGNAL(clicked()), this, SLOT(sAdd()));
  connect(_revoke, SIGNAL(clicked()), this, SLOT(sRevoke()));

  connect(_taxZone,	SIGNAL(newID(int)),	 this, SLOT(sPopulateTaxCode()));
  connect(_taxType,	SIGNAL(newID(int)),	 this, SLOT(sPopulateTaxCode()));

  _mode		= cNew;
  _taxassId	= -1;
}

taxAssignment::~taxAssignment()
{
  // no need to delete child widgets, Qt does it all for us
}

void taxAssignment::languageChange()
{
  retranslateUi(this);
}

enum SetResponse taxAssignment::set(const ParameterList& pParams)
{
  XDialog::set(pParams);
  QVariant	param;
  bool		valid;

  param = pParams.value("taxzone_id", &valid);
  if (valid)
    _taxzoneId = param.toInt();

  param = pParams.value("taxtype_id", &valid);
  if (valid)
    _taxtypeId = param.toInt();

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      _mode = cNew;
      _taxZone->setFocus();
	  sPopulate();
      sPopulateTaxCode();
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;
	  sPopulate();
      sPopulateTaxCode();
    }
    else if (param.toString() == "view")
    {
      _mode = cView;
	  sPopulate();
      sPopulateTaxCode();

      _taxZone->setEnabled(false);
      _taxType->setEnabled(false);
	  _add->setEnabled(false);
	  _revoke->setEnabled(false);
      _taxCodeOption->setEnabled(false);
	  _taxCodeSelected->setEnabled(false);
      _close->setFocus();
    }
  }

  return NoError;
}

void taxAssignment::sPopulate()
{
  _taxZone->setId(_taxzoneId);
  _taxType->setId(_taxtypeId);
}

void taxAssignment::sPopulateTaxCode()
{
  ParameterList params;
  params.append("taxzone_id", _taxZone->id());
  params.append("taxtype_id", _taxType->id());

  QString sqlTaxOption("SELECT tax_id, tax_code AS taxcode_option "
              "FROM tax "
			  "WHERE tax_id NOT IN ("
			    "SELECT taxass_tax_id FROM taxass "
				"<? if exists(\"taxzone_id\") ?>"
	            "WHERE (COALESCE(taxass_taxzone_id, -1) = <? value(\"taxzone_id\") ?>) "
				"<? else ?>"
	            "WHERE (taxass_taxzone_id IS NULL) "
				"<? endif ?>"
				"<? if exists(\"taxtype_id\") ?>"
	            "AND (COALESCE(taxass_taxtype_id, -1) = <? value(\"taxtype_id\") ?>) "
				"<? else ?>"
                "AND (taxass_taxtype_id IS NULL) "
	            "<? endif ?>"
			  ") "
			  "AND COALESCE(tax_basis_tax_id, -1) < 0;");

  MetaSQLQuery mqlTaxOption(sqlTaxOption);
  q = mqlTaxOption.toQuery(params);

  _taxCodeOption->clear();
  _taxCodeOption->populate(q);
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  QString sqlTaxSelected("SELECT tax_id, tax_code AS taxcode_selected "
              "FROM tax "
			  "WHERE tax_id IN ("
			    "SELECT taxass_tax_id FROM taxass "
				"<? if exists(\"taxzone_id\") ?>"
	            "WHERE (COALESCE(taxass_taxzone_id, -1) = <? value(\"taxzone_id\") ?>) "
				"<? else ?>"
	            "WHERE (taxass_taxzone_id IS NULL) "
				"<? endif ?>"
				"<? if exists(\"taxtype_id\") ?>"
	            "AND (COALESCE(taxass_taxtype_id, -1) = <? value(\"taxtype_id\") ?>) "
				"<? else ?>"
                "AND (taxass_taxtype_id IS NULL) "
	            "<? endif ?>"
			  ") "
			  "AND COALESCE(tax_basis_tax_id, -1) < 0;");

  MetaSQLQuery mqlTaxSelected(sqlTaxSelected);
  q = mqlTaxSelected.toQuery(params);

  _taxCodeSelected->clear();
  _taxCodeSelected->populate(q);
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void taxAssignment::sAdd()
{
  q.prepare("SELECT * FROM getSubTax(:tax_id, 1);");
  q.bindValue(":tax_id", _taxCodeOption->id());
  q.exec();
  if(q.first())
  {
	q.prepare("SELECT DISTINCT tax_id "
	          "FROM tax JOIN taxass ON (tax_id = taxass_tax_id) "
			  "LEFT OUTER JOIN taxclass ON (tax_taxclass_id = taxclass_id) "
			  "WHERE COALESCE(taxass_taxzone_id, -1) = :taxass_taxzone_id "
			  "AND COALESCE(taxass_taxtype_id, -1) = :taxass_taxtype_id "
			  "AND COALESCE(taxclass_sequence, 0) IN "
			  "(SELECT COALESCE(taxclass_sequence, 0) "
			  "FROM taxclass RIGHT OUTER JOIN tax "
			  "ON (taxclass_id = tax_taxclass_id) "
			  "AND tax_id = :tax_id);");
    q.bindValue(":tax_id", _taxCodeOption->id());
	if(_taxZone->isValid())
	  q.bindValue(":taxass_taxzone_id", _taxZone->id());
	else
	  q.bindValue(":taxass_taxzone_id", -1);
    if(_taxType->isValid())
      q.bindValue(":taxass_taxtype_id", _taxType->id());
	else
	  q.bindValue(":taxass_taxtype_id", -1);
    q.exec();
    if(q.first())
	{
	  QMessageBox::critical(this, tr("Incorrect Tax Code"), tr("Tax codes with same group sequence as this "
	  "tax code are already assigned to this Tax Zone / Tax Type pair. \nYou first "
	  "need to Revoke those Tax Codes."));
	  return;
	}
  }
  else
  {
	q.prepare("SELECT DISTINCT A.tax_id, A.tax_code, A.tax_taxclass_id "
	          "FROM tax A JOIN taxass ON (A.tax_id = taxass_tax_id) "
			  "LEFT OUTER JOIN taxclass ON (A.tax_taxclass_id = taxclass_id), tax B "
			  "WHERE A.tax_id = B.tax_basis_tax_id "
			  "AND COALESCE(A.tax_basis_tax_id, 0) = 0 "
			  "AND COALESCE(taxass_taxzone_id, -1) = :taxass_taxzone_id "
			  "AND COALESCE(taxass_taxtype_id, -1) = :taxass_taxtype_id "
			  "AND COALESCE(taxclass_sequence, 0) IN "
			  "(SELECT COALESCE(taxclass_sequence, 0) "
			  "FROM taxclass RIGHT OUTER JOIN tax "
			  "ON (taxclass_id = tax_taxclass_id) "
			  "AND tax_id = :tax_id);");
    q.bindValue(":tax_id", _taxCodeOption->id());
	if(_taxZone->isValid())
	  q.bindValue(":taxass_taxzone_id", _taxZone->id());
    else
	  q.bindValue(":taxass_taxzone_id", -1);
    if(_taxType->isValid())
      q.bindValue(":taxass_taxtype_id", _taxType->id());
	else
	  q.bindValue(":taxass_taxtype_id", -1);
    q.exec();
    if(q.first())
	{
	  QMessageBox::critical(this, tr("Incorrect Tax Code"), tr("Tax codes with same group sequence as this "
	  "tax code and which have subordinate taxes are already assigned to this Tax Zone / Tax Type pair. \nYou first "
	  "need to Revoke those Tax Codes."));
	  return;
	}
  }
	

  XSqlQuery rollback;
  rollback.prepare("ROLLBACK;");

  q.exec("BEGIN;");
  q.exec("SELECT NEXTVAL('taxass_taxass_id_seq') AS _taxass_id;");
  if (q.first())
    _taxassId = q.value("_taxass_id").toInt();
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
  
  q.prepare("INSERT INTO taxass(taxass_id, taxass_taxzone_id, "
         "taxass_taxtype_id, taxass_tax_id) "
         "VALUES (:taxass_id, :taxass_taxzone_id, "
		 ":taxass_taxtype_id, :taxass_tax_id);");
  q.bindValue(":taxass_id", _taxassId);
  if(_taxZone->isValid())
	q.bindValue(":taxass_taxzone_id", _taxZone->id());
  if(_taxType->isValid())
    q.bindValue(":taxass_taxtype_id", _taxType->id());
  q.bindValue(":taxass_tax_id", _taxCodeOption->id());
  q.exec();
  if (q.lastError().type() != QSqlError::NoError)
  {
    rollback.exec();
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
	return;
  }
  q.exec("COMMIT;");
  sPopulateTaxCode();
}

void taxAssignment::sRevoke()
{
  q.prepare("DELETE FROM taxass "
            "WHERE ((taxass_tax_id = :taxass_tax_id) "
			"AND (COALESCE(taxass_taxzone_id, -1) = :taxass_taxzone_id) "
			"AND (COALESCE(taxass_taxtype_id, -1) = :taxass_taxtype_id));");
  q.bindValue(":taxass_tax_id", _taxCodeSelected->id());
  q.bindValue(":taxass_taxzone_id", _taxZone->id());
  q.bindValue(":taxass_taxtype_id", _taxType->id());
  q.exec();
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
	return;
  }
  sPopulateTaxCode();
}
