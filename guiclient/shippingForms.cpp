/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "shippingForms.h"

#include <QVariant>
#include <QMessageBox>
#include <parameter.h>
#include "shippingForm.h"

shippingForms::shippingForms(QWidget* parent, const char* name, Qt::WFlags fl)
  : XWidget(parent, name, fl)
{
  setupUi(this);


  // signals and slots connections
  connect(_new, SIGNAL(clicked()), this, SLOT(sNew()));
  connect(_edit, SIGNAL(clicked()), this, SLOT(sEdit()));
  connect(_view, SIGNAL(clicked()), this, SLOT(sView()));
  connect(_delete, SIGNAL(clicked()), this, SLOT(sDelete()));
  connect(_close, SIGNAL(clicked()), this, SLOT(close()));
  connect(_bolformat, SIGNAL(valid(bool)), _view, SLOT(setEnabled(bool)));

  _bolformat->addColumn(tr("Form Name"), -1, Qt::AlignLeft, true, "shipform_name");

  if (_privileges->check("MaintainShippingForms"))
  {
    connect(_bolformat, SIGNAL(valid(bool)), _edit, SLOT(setEnabled(bool)));
    connect(_bolformat, SIGNAL(valid(bool)), _delete, SLOT(setEnabled(bool)));
    connect(_bolformat, SIGNAL(itemSelected(int)), _edit, SLOT(animateClick()));
  }
  else
  {
    _new->setEnabled(FALSE);
    connect(_bolformat, SIGNAL(itemSelected(int)), _view, SLOT(animateClick()));
  }

  sFillList();
}

shippingForms::~shippingForms()
{
  // no need to delete child widgets, Qt does it all for us
}

void shippingForms::languageChange()
{
  retranslateUi(this);
}

void shippingForms::sNew()
{
  ParameterList params;
  params.append("mode", "new");

  shippingForm newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void shippingForms::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("shipform_id", _bolformat->id());

  shippingForm newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void shippingForms::sView()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("shipform_id", _bolformat->id());

  shippingForm newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void shippingForms::sDelete()
{
  q.prepare( "SELECT cust_id "
             "FROM cust "
             "WHERE (cust_shipform_id=:shipform_id);" );
  q.bindValue(":shipform_id", _bolformat->id());
  q.exec();
  if (q.first())
  {
    QMessageBox::critical( this, tr("Cannot Delete Shipping Form"),
                           tr( "The selected Shipping Form cannot be deleted as there are one or more Customers assigned to use it.\n"
                               "You must reassign these Customers before you may delete the selected Shipping Form." ) );
    return;
  }

  q.prepare( "SELECT shipto_id "
             "FROM shipto "
             "WHERE (shipto_shipform_id=:shipform_id);" );
  q.bindValue(":shipform_id", _bolformat->id());
  q.exec();
  if (q.first())
  {
    QMessageBox::critical( this, tr("Cannot Delete Shipping Form"),
                           tr( "The selected Shipping Form cannot be deleted as there are one or more Ship-Tos assigned to use it.\n"
                               "You must reassign these Ship-Tos before you may delete the selected Shipping Form." ) );
    return;
  }

  q.prepare( "DELETE FROM shipform "
             "WHERE (shipform_id=:shipform_id);" );
  q.bindValue(":shipform_id", _bolformat->id());
  q.exec();

  sFillList();
}

void shippingForms::sFillList()
{
  _bolformat->populate( "SELECT shipform_id, shipform_name "
                        "FROM shipform "
                        "ORDER BY shipform_name" );
}
