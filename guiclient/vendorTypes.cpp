/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "vendorTypes.h"

#include <QVariant>
#include <QMessageBox>
//#include <QStatusBar>
#include <QMenu>
#include <parameter.h>
#include <openreports.h>
#include "vendorType.h"
#include "guiclient.h"
/*
 *  Constructs a vendorTypes as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
vendorTypes::vendorTypes(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

//  (void)statusBar();

  // signals and slots connections
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_new, SIGNAL(clicked()), this, SLOT(sNew()));
  connect(_edit, SIGNAL(clicked()), this, SLOT(sEdit()));
  connect(_delete, SIGNAL(clicked()), this, SLOT(sDelete()));
  connect(_vendtype, SIGNAL(populateMenu(QMenu *, QTreeWidgetItem *)), this, SLOT(sPopulateMenu(QMenu*)));
  connect(_close, SIGNAL(clicked()), this, SLOT(close()));

//  statusBar()->hide();
  
  if (_privileges->check("MaintainVendorTypes"))
  {
    connect(_vendtype, SIGNAL(valid(bool)), _edit, SLOT(setEnabled(bool)));
    connect(_vendtype, SIGNAL(valid(bool)), _delete, SLOT(setEnabled(bool)));
    connect(_vendtype, SIGNAL(itemSelected(int)), _edit, SLOT(animateClick()));
  }
  else
  {
    _new->setEnabled(FALSE);
  }

  _vendtype->addColumn(tr("Code"),        70, Qt::AlignLeft,   true,  "vendtype_code" );
  _vendtype->addColumn(tr("Description"), -1, Qt::AlignLeft,   true,  "vendtype_descrip" );

  sFillList();
}

/*
 *  Destroys the object and frees any allocated resources
 */
vendorTypes::~vendorTypes()
{
  // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void vendorTypes::languageChange()
{
  retranslateUi(this);
}

void vendorTypes::sFillList()
{
  _vendtype->populate( "SELECT vendtype_id, vendtype_code, vendtype_descrip "
                       "FROM vendtype "
                       "ORDER BY vendtype_code;" );
}

void vendorTypes::sDelete()
{
  q.prepare( "SELECT vend_id "
             "FROM vend "
             "WHERE (vend_vendtype_id=:vendtype_id) "
             "LIMIT 1;" );
  q.bindValue(":vendtype_id", _vendtype->id());
  q.exec();
  if (q.first())
  {
    QMessageBox::critical( this, tr("Cannot Delete Vendor Type"),
                           tr( "The selected Vendor Type cannot be deleted as there are one or more Vendors assigned to it.\n"
                               "You must reassign these Vendors before you may delete the selected Vendor Type." ) );
    return;
  }

  q.prepare( "DELETE FROM vendtype "
             "WHERE (vendtype_id=:vendtype_id);"

             "DELETE FROM apaccnt "
             "WHERE (apaccnt_vendtype_id=:vendtype_id);" );
  q.bindValue(":vendtype_id", _vendtype->id());
  q.exec();
  sFillList();
}

void vendorTypes::sNew()
{
  ParameterList params;
  params.append("mode", "new");

  vendorType newdlg(this, "", TRUE);
  newdlg.set(params);
  
  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void vendorTypes::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("vendtype_id", _vendtype->id());

  vendorType newdlg(this, "", TRUE);
  newdlg.set(params);
  
  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void vendorTypes::sView()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("vendtype_id", _vendtype->id());

  vendorType newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void vendorTypes::sPopulateMenu(QMenu *)
{
}

void vendorTypes::sPrint()
{
  orReport report("VendorTypesMasterList");
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

