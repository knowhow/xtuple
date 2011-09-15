/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "shippingZones.h"

#include <QVariant>
#include <QMessageBox>
//#include <QStatusBar>
#include <parameter.h>
#include <openreports.h>
#include "shippingZone.h"
#include "guiclient.h"

/*
 *  Constructs a shippingZones as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
shippingZones::shippingZones(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
    setupUi(this);

//    (void)statusBar();

    // signals and slots connections
    connect(_new, SIGNAL(clicked()), this, SLOT(sNew()));
    connect(_edit, SIGNAL(clicked()), this, SLOT(sEdit()));
    connect(_view, SIGNAL(clicked()), this, SLOT(sView()));
    connect(_delete, SIGNAL(clicked()), this, SLOT(sDelete()));
    connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
    connect(_shipzone, SIGNAL(populateMenu(QMenu *, QTreeWidgetItem *, int)), this, SLOT(sPopulateMenu(QMenu*)));
    connect(_close, SIGNAL(clicked()), this, SLOT(close()));
    connect(_shipzone, SIGNAL(valid(bool)), _view, SLOT(setEnabled(bool)));
    init();
}

/*
 *  Destroys the object and frees any allocated resources
 */
shippingZones::~shippingZones()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void shippingZones::languageChange()
{
    retranslateUi(this);
}

//Added by qt3to4:
#include <QMenu>

void shippingZones::init()
{
//  statusBar()->hide();
  
  _shipzone->addColumn(tr("Name"),        70, Qt::AlignLeft,   true,  "shipzone_name" );
  _shipzone->addColumn(tr("Description"), -1, Qt::AlignLeft,   true,  "shipzone_descrip" );

  if (_privileges->check("MaintainShippingZones"))
  {
    connect(_shipzone, SIGNAL(valid(bool)), _edit, SLOT(setEnabled(bool)));
    connect(_shipzone, SIGNAL(valid(bool)), _delete, SLOT(setEnabled(bool)));
    connect(_shipzone, SIGNAL(itemSelected(int)), _edit, SLOT(animateClick()));
  }
  else
  {
    _new->setEnabled(FALSE);
    connect(_shipzone, SIGNAL(itemSelected(int)), _view, SLOT(animateClick()));
  }

  sFillList();
}

void shippingZones::sDelete()
{
  q.prepare( "SELECT shipto_id "
             "FROM shipto "
             "WHERE (shipto_shipzone_id=:shipzone_id);" );
  q.bindValue(":shipzone_id", _shipzone->id());
  q.exec();
  if (q.first())
  {
    QMessageBox::critical( this, tr("Cannot Delete Shipping Zone"),
                           tr( "The selected Shipping Zone cannot be deleted as there are one or more Ship-Tos assigned to it.\n"
                               "You must reassign these Ship-Tos to a different Shipping Zone before you may delete the selected Shipping Zone." ) );
    return;
  }

  q.prepare( "DELETE FROM shipzone "
             "WHERE (shipzone_id=:shipzone_id);" );
  q.bindValue(":shipzone_id", _shipzone->id());
  q.exec();

  sFillList();
}

void shippingZones::sNew()
{
  ParameterList params;
  params.append("mode", "new");

  shippingZone newdlg(this, "", TRUE);
  newdlg.set(params);
  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void shippingZones::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("shipzone_id", _shipzone->id());

  shippingZone newdlg(this, "", TRUE);
  newdlg.set(params);
  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void shippingZones::sView()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("shipzone_id", _shipzone->id());

  shippingZone newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void shippingZones::sFillList()
{
  _shipzone->populate( "SELECT shipzone_id, shipzone_name, shipzone_descrip "
                       "FROM shipzone "
                       "ORDER BY shipzone_name" );
}

void shippingZones::sPopulateMenu( QMenu * )
{

}

void shippingZones::sPrint()
{
  orReport report("ShippingZonesMasterList");
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

