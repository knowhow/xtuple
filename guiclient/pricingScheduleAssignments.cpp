/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "pricingScheduleAssignments.h"

#include <QVariant>
#include <QMessageBox>
//#include <QStatusBar>
#include <openreports.h>
#include <parameter.h>
#include "pricingScheduleAssignment.h"

/*
 *  Constructs a pricingScheduleAssignments as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
pricingScheduleAssignments::pricingScheduleAssignments(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

//  (void)statusBar();

  // signals and slots connections
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_new, SIGNAL(clicked()), this, SLOT(sNew()));
  connect(_edit, SIGNAL(clicked()), this, SLOT(sEdit()));
  connect(_delete, SIGNAL(clicked()), this, SLOT(sDelete()));
  connect(_close, SIGNAL(clicked()), this, SLOT(close()));
  connect(_view, SIGNAL(clicked()), this, SLOT(sView()));
  connect(_ipsass, SIGNAL(valid(bool)), _view, SLOT(setEnabled(bool)));

//  statusBar()->hide();
  
  _ipsass->addColumn(tr("Ship-To"),          _itemColumn, Qt::AlignCenter, true, "shiptonum" );
  _ipsass->addColumn(tr("Customer #"),       _itemColumn, Qt::AlignCenter, true, "custnumber" );
  _ipsass->addColumn(tr("Cust. Name"),       150,         Qt::AlignCenter, true, "custname" );
  _ipsass->addColumn(tr("Cust. Type"),       _itemColumn, Qt::AlignCenter, true, "custtype" );
  _ipsass->addColumn(tr("Pricing Schedule"), -1,          Qt::AlignCenter, true, "ipshead_name" );

  if (_privileges->check("AssignPricingSchedules"))
  {
    connect(_ipsass, SIGNAL(valid(bool)), _edit, SLOT(setEnabled(bool)));
    connect(_ipsass, SIGNAL(valid(bool)), _delete, SLOT(setEnabled(bool)));
    connect(_ipsass, SIGNAL(itemSelected(int)), _edit, SLOT(animateClick()));
  }
  else
  {
    _new->setEnabled(FALSE);
    connect(_ipsass, SIGNAL(itemSelected(int)), _view, SLOT(animateClick()));
  }

  sFillList();
}

/*
 *  Destroys the object and frees any allocated resources
 */
pricingScheduleAssignments::~pricingScheduleAssignments()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void pricingScheduleAssignments::languageChange()
{
    retranslateUi(this);
}

void pricingScheduleAssignments::sPrint()
{
  orReport report("PricingScheduleAssignments");
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void pricingScheduleAssignments::sNew()
{
  ParameterList params;
  params.append("mode", "new");

  pricingScheduleAssignment newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void pricingScheduleAssignments::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("ipsass_id", _ipsass->id());

  pricingScheduleAssignment newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void pricingScheduleAssignments::sView()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("ipsass_id", _ipsass->id());

  pricingScheduleAssignment newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void pricingScheduleAssignments::sDelete()
{
  q.prepare( "DELETE FROM ipsass "
             "WHERE (ipsass_id=:ipsass_id);" );
  q.bindValue(":ipsass_id", _ipsass->id());
  q.exec();

  sFillList();
}

void pricingScheduleAssignments::sFillList()
{
  _ipsass->populate( "SELECT ipsass_id,"
                     "       CASE WHEN (ipsass_shipto_id != -1) THEN (SELECT shipto_num FROM shipto WHERE (shipto_id=ipsass_shipto_id))"
                     "            WHEN (COALESCE(LENGTH(ipsass_shipto_pattern), 0) > 0) THEN ipsass_shipto_pattern"
                     "            ELSE TEXT('ANY')"
                     "       END AS shiptonum,"
                     "       CASE WHEN (ipsass_shipto_id != -1) THEN (SELECT cust_number FROM shipto, cust WHERE ((shipto_cust_id=cust_id) AND (shipto_id=ipsass_shipto_id))) "
                     "            WHEN (ipsass_cust_id=-1) THEN TEXT('Any')"
                     "            ELSE (SELECT cust_number FROM cust WHERE (cust_id=ipsass_cust_id))"
                     "       END AS custnumber,"
                     "       CASE WHEN (ipsass_shipto_id != -1) THEN (SELECT cust_name FROM shipto, cust WHERE ((shipto_cust_id=cust_id) AND (shipto_id=ipsass_shipto_id))) "
                     "            WHEN (ipsass_cust_id=-1) THEN ''"
                     "            ELSE (SELECT cust_name FROM cust WHERE (cust_id=ipsass_cust_id))"
                     "       END AS custname,"
                     "       CASE WHEN (ipsass_cust_id != -1) THEN TEXT('N/A')"
                     "            WHEN (ipsass_shipto_id != -1) THEN TEXT('N/A')"
                     "            WHEN (COALESCE(LENGTH(ipsass_shipto_pattern),0) > 0) THEN TEXT('N/A')"
                     "            WHEN (ipsass_custtype_id=-1) THEN ipsass_custtype_pattern"
                     "            ELSE (SELECT custtype_code FROM custtype WHERE (custtype_id=ipsass_custtype_id))"
                     "       END AS custtype,"
                     "       ipshead_name "
                     "FROM ipsass, ipshead "
                     "WHERE (ipshead_id=ipsass_ipshead_id) "
                     "ORDER BY custname, custtype;" );
}

