/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "sales.h"

#include <QVariant>
#include <QMessageBox>
#include <parameter.h>
#include "sale.h"
#include "guiclient.h"

sales::sales(QWidget* parent, const char* name, Qt::WFlags fl)
  : XWidget(parent, name, fl)
{
  setupUi(this);


  // signals and slots connections
  connect(_sale, SIGNAL(valid(bool)), _delete, SLOT(setEnabled(bool)));
  connect(_new, SIGNAL(clicked()), this, SLOT(sNew()));
  connect(_edit, SIGNAL(clicked()), this, SLOT(sEdit()));
  connect(_delete, SIGNAL(clicked()), this, SLOT(sDelete()));
  connect(_sale, SIGNAL(itemSelected(int)), _edit, SLOT(animateClick()));
  connect(_sale, SIGNAL(valid(bool)), _edit, SLOT(setEnabled(bool)));
  connect(_close, SIGNAL(clicked()), this, SLOT(close()));

  _sale->addColumn(tr("Name"),     _itemColumn, Qt::AlignLeft,   true, "sale_name"  );
  _sale->addColumn(tr("Schedule"), -1         , Qt::AlignLeft,   true, "ipshead_name"  );
  _sale->addColumn(tr("Start"),    _dateColumn, Qt::AlignCenter, true, "sale_startdate" );
  _sale->addColumn(tr("End"),      _dateColumn, Qt::AlignCenter, true, "sale_enddate" );

  sFillList();
}

sales::~sales()
{
  // no need to delete child widgets, Qt does it all for us
}

void sales::languageChange()
{
  retranslateUi(this);
}

void sales::sNew()
{
  ParameterList params;
  params.append("mode", "new");

  sale newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != QDialog::Rejected)
    sFillList();
}

void sales::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("sale_id", _sale->id());

  sale newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != QDialog::Rejected)
    sFillList();
}

void sales::sDelete()
{
  q.prepare( "DELETE FROM sale "
             "WHERE (sale_id=:sale_id);" );
  q.bindValue(":sale_id", _sale->id());
  q.exec();

  sFillList();
}

void sales::sFillList()
{
  QString sql( "SELECT sale_id, sale_name, ipshead_name,"
               "       sale_startdate, sale_enddate "
               "FROM sale, ipshead "
               "WHERE (sale_ipshead_id=ipshead_id) "
               "ORDER BY sale_name;" );

  _sale->populate(sql);
}


