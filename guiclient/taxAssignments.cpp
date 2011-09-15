/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "taxAssignments.h"
#include "taxAssignment.h"

#include <QSqlError>

#include "metasql.h"
#include "parameter.h"

taxAssignments::taxAssignments(QWidget* parent, const char* name, Qt::WFlags fl)
  : XWidget(parent, name, fl)
{
  setupUi(this);

  connect(_delete,	SIGNAL(clicked()),	 this, SLOT(sDelete()));
  connect(_edit,	SIGNAL(clicked()),	 this, SLOT(sEdit()));
  connect(_new,		SIGNAL(clicked()),	 this, SLOT(sNew()));
  connect(_showAll,	SIGNAL(toggled(bool)),	 this, SLOT(sFillList()));
  connect(_showAllTypes,	SIGNAL(toggled(bool)),	 this, SLOT(sFillList()));
  connect(_taxass,	SIGNAL(valid(bool)),	_view, SLOT(setEnabled(bool)));
  connect(_taxZone,	SIGNAL(newID(int)),	 this, SLOT(sFillList()));
  connect(_taxType,	SIGNAL(newID(int)),	 this, SLOT(sFillList()));
  connect(_view,	SIGNAL(clicked()),	 this, SLOT(sView()));

  if (_privileges->check("MaintainTaxAssignments"))
  {
    connect(_taxass,	SIGNAL(valid(bool)),  _delete, SLOT(setEnabled(bool)));
    connect(_taxass,	SIGNAL(valid(bool)),	_edit, SLOT(setEnabled(bool)));
    connect(_taxass,	SIGNAL(itemSelected(int)), _edit, SLOT(animateClick()));
  }
  else
  {
    _new->setEnabled(false);
    connect(_taxass, SIGNAL(itemSelected(int)), _view, SLOT(animateClick()));
  }

  _taxass->addColumn(tr("Tax Zone/Code"),	 -1,  Qt::AlignLeft,   true,  "taxassign_zone_code");
  _taxass->addColumn(tr("Tax Type/Description"),      100,  Qt::AlignLeft,   true,  "taxassign_type_descrip");
  _taxass->addColumn(tr("Tax Class"),      100,  Qt::AlignLeft,   true,  "taxassign_taxclass_code");
  _taxass->addColumn(tr("Group Sequence"),      100,  Qt::AlignLeft,   true,  "taxassign_taxclass_sequence");
  _taxass->setIndentation(10);

  sFillList();
}

taxAssignments::~taxAssignments()
{
  // no need to delete child widgets, Qt does it all for us
}

void taxAssignments::languageChange()
{
  retranslateUi(this);
}

void taxAssignments::sNew()
{
  ParameterList params;
  params.append("mode", "new");
  
  taxAssignment newdlg(this, "", true);
  newdlg.set(params);

  newdlg.exec();
  sFillList();
}

void taxAssignments::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");

  params.append("taxzone_id", _taxass->id());
  params.append("taxtype_id", _taxass->altId());

  taxAssignment newdlg(this, "", true);
  newdlg.set(params);

  newdlg.exec();
  sFillList();
}

void taxAssignments::sView()
{
  ParameterList params;
  params.append("mode", "view");
  
  params.append("taxzone_id", _taxass->id());
  params.append("taxtype_id", _taxass->altId());

  taxAssignment newdlg(this, "", true);
  newdlg.set(params);
  newdlg.exec();
}

void taxAssignments::sDelete()
{
  q.prepare("DELETE FROM taxass WHERE ((COALESCE(taxass_taxzone_id, -1) = :taxzone_id) "
            "AND (COALESCE(taxass_taxtype_id, -1) = :taxtype_id));");
  q.bindValue(":taxzone_id", _taxass->id());
  q.bindValue(":taxtype_id", _taxass->altId());
  q.exec();
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  sFillList();
}

void taxAssignments::sFillList()
{
  ParameterList params;
  
  if (! _showAll->isChecked())
    params.append("taxzone_id", _taxZone->id());
  
  if (! _showAllTypes->isChecked())
    params.append("taxtype_id", _taxType->id());
  

  QString sql("SELECT taxassign_taxzone_id, taxassign_taxtype_id, taxassign_level AS xtindentrole, "
              "COALESCE(taxassign_zone_code, '~Any~') AS taxassign_zone_code, "
		      "COALESCE(taxassign_type_descrip, '~Any~') AS taxassign_type_descrip, "
			  "taxassign_taxclass_code, taxassign_taxclass_sequence, COALESCE(taxassign_taxclass_sequence, -1) AS dummy_seq "
		      "FROM taxassignments(<? value(\"taxzone_id\") ?>, <? value(\"taxtype_id\") ?>) "
			  "ORDER BY taxassign_taxzone_id, taxassign_taxtype_id, dummy_seq, xtindentrole;");
  MetaSQLQuery mql(sql);
  XSqlQuery r = mql.toQuery(params);
  _taxass->populate(r, true);
  if (r.lastError().type() != QSqlError::NoError)
  {
    systemError(this, r.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}
