/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "shipVias.h"

#include <QVariant>
#include <QMessageBox>
#include <parameter.h>
#include "shipVia.h"
#include "guiclient.h"

shipVias::shipVias(QWidget* parent, const char* name, Qt::WFlags fl)
  : XWidget(parent, name, fl)
{
  setupUi(this);


  // signals and slots connections
  connect(_close, SIGNAL(clicked()), this, SLOT(close()));
  connect(_new, SIGNAL(clicked()), this, SLOT(sNew()));
  connect(_edit, SIGNAL(clicked()), this, SLOT(sEdit()));
  connect(_delete, SIGNAL(clicked()), this, SLOT(sDelete()));

  if (_privileges->check("MaintainShipVias"))
  {
    connect(_shipvia, SIGNAL(valid(bool)), _edit, SLOT(setEnabled(bool)));
    connect(_shipvia, SIGNAL(valid(bool)), _delete, SLOT(setEnabled(bool)));
    connect(_shipvia, SIGNAL(itemSelected(int)), _edit, SLOT(animateClick()));
  }
  else
    _new->setEnabled(FALSE);

  _shipvia->addColumn(tr("Code"),        _itemColumn, Qt::AlignLeft,   true,  "shipvia_code" );
  _shipvia->addColumn(tr("Description"), -1,          Qt::AlignLeft,   true,  "shipvia_descrip" ); 
  
  sFillList();
}

shipVias::~shipVias()
{
  // no need to delete child widgets, Qt does it all for us
}

void shipVias::languageChange()
{
  retranslateUi(this);
}

void shipVias::sNew()
{
  ParameterList params;
  params.append("mode", "new");

  shipVia newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void shipVias::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("shipvia_id", _shipvia->id());

  shipVia newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void shipVias::sView()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("shipvia_id", _shipvia->id());

  shipVia newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void shipVias::sDelete()
{
  q.prepare( "DELETE FROM shipvia "
             "WHERE (shipvia_id=:shipvia_id);" );
  q.bindValue(":shipvia_id", _shipvia->id());
  q.exec();

  sFillList();
}

void shipVias::sFillList()
{
  _shipvia->populate( "SELECT shipvia_id, shipvia_code, shipvia_descrip "
                      "FROM shipvia "
                      "ORDER BY shipvia_code;" );
}

