/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "taxZones.h"

#include <QSqlError>
#include <QVariant>
#include <QMessageBox>
//#include <QStatusBar>
#include <parameter.h>
#include <openreports.h>
#include "taxZone.h"
#include "storedProcErrorLookup.h"

/*
 *  Constructs a taxZones as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
taxZones::taxZones(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

//  (void)statusBar();

  // signals and slots connections
  connect(_new, SIGNAL(clicked()), this, SLOT(sNew()));
  connect(_edit, SIGNAL(clicked()), this, SLOT(sEdit()));
  connect(_close, SIGNAL(clicked()), this, SLOT(close()));
  connect(_delete, SIGNAL(clicked()), this, SLOT(sDelete()));
  connect(_view, SIGNAL(clicked()), this, SLOT(sView()));
  connect(_taxZone, SIGNAL(valid(bool)), _view, SLOT(setEnabled(bool)));

//  statusBar()->hide();
  
  _taxZone->addColumn(tr("Area"),        70, Qt::AlignLeft,  true,  "taxzone_code" );
  _taxZone->addColumn(tr("Description"), -1, Qt::AlignLeft,  true,  "taxzone_descrip" );

  if (_privileges->check("MaintainTaxZones"))
  {
    connect(_taxZone, SIGNAL(valid(bool)), _edit, SLOT(setEnabled(bool)));
    connect(_taxZone, SIGNAL(valid(bool)), _delete, SLOT(setEnabled(bool)));
    connect(_taxZone, SIGNAL(itemSelected(int)), _edit, SLOT(animateClick()));
  }
  else
  {
    connect(_taxZone, SIGNAL(itemSelected(int)), _view, SLOT(animateClick()));

    _new->setEnabled(FALSE);
  }

  sFillList(-1);
}

/*
 *  Destroys the object and frees any allocated resources
 */
taxZones::~taxZones()
{
  // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void taxZones::languageChange()
{
  retranslateUi(this);
}

void taxZones::sDelete()
{
  q.prepare("SELECT deletetaxzone(:tax_zone_id) AS result;");
  q.bindValue(":tax_zone_id", _taxZone->id());
  q.exec();

  if (q.first())
  {
    int returnVal = q.value("result").toInt();
    if (returnVal < 0)
    {
      systemError(this, storedProcErrorLookup("deleteTaxZone", returnVal), __FILE__, __LINE__);
      return;
    }
	sFillList(-1);
  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void taxZones::sNew()
{
  ParameterList params;
  params.append("mode", "new");

  taxZone newdlg(this, "", TRUE);
  newdlg.set(params);
  
  int result = newdlg.exec();
  if (result != XDialog::Rejected)
    sFillList(result);
}

void taxZones::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("taxzone_id", _taxZone->id());

  taxZone newdlg(this, "", TRUE);
  newdlg.set(params);
  
  int result = newdlg.exec();
  if (result != XDialog::Rejected)
    sFillList(result);
}

void taxZones::sView()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("taxzone_id", _taxZone->id());

  taxZone newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void taxZones::sFillList(int pId)
{
  _taxZone->populate( "SELECT taxzone_id, taxzone_code, taxzone_descrip "
                      "FROM taxzone "
                      "ORDER BY taxzone_code;", pId );
}

