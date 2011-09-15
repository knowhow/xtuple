/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "apAccountAssignments.h"

#include <QVariant>
#include <QMessageBox>

#include <parameter.h>
#include <openreports.h>

#include "apAccountAssignment.h"

apAccountAssignments::apAccountAssignments(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_new, SIGNAL(clicked()), this, SLOT(sNew()));
  connect(_edit, SIGNAL(clicked()), this, SLOT(sEdit()));
  connect(_delete, SIGNAL(clicked()), this, SLOT(sDelete()));
  connect(_view, SIGNAL(clicked()), this, SLOT(sView()));

  _apaccnt->addColumn(tr("Vendor Type"),      -1, Qt::AlignCenter,true, "vendtypecode");
  _apaccnt->addColumn(tr("A/P Account"),     120, Qt::AlignLeft,  true, "apaccnt");
  _apaccnt->addColumn(tr("Prepaid Account"), 120, Qt::AlignLeft,  true, "prepaidaccnt");
  _apaccnt->addColumn(tr("Discount Account"),120, Qt::AlignLeft,  true, "discountaccnt");

  if (_privileges->check("MaintainVendorAccounts"))
  {
    connect(_apaccnt, SIGNAL(valid(bool)), _edit, SLOT(setEnabled(bool)));
    connect(_apaccnt, SIGNAL(valid(bool)), _delete, SLOT(setEnabled(bool)));
    connect(_apaccnt, SIGNAL(itemSelected(int)), _edit, SLOT(animateClick()));
  }
  else
  {
    _new->setEnabled(FALSE);
    connect(_apaccnt, SIGNAL(itemSelected(int)), _view, SLOT(animateClick()));
  }

  sFillList();
}

apAccountAssignments::~apAccountAssignments()
{
  // no need to delete child widgets, Qt does it all for us
}

void apAccountAssignments::languageChange()
{
    retranslateUi(this);
}

void apAccountAssignments::sPrint()
{
  orReport report("APAssignmentsMasterList");
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void apAccountAssignments::sNew()
{
  ParameterList params;
  params.append("mode", "new");

  apAccountAssignment newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void apAccountAssignments::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("apaccnt_id", _apaccnt->id());

  apAccountAssignment newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void apAccountAssignments::sView()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("apaccnt_id", _apaccnt->id());

  apAccountAssignment newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void apAccountAssignments::sDelete()
{
  q.prepare( "DELETE FROM apaccnt "
             "WHERE (apaccnt_id=:apaccnt_id);" );
  q.bindValue(":apaccnt_id", _apaccnt->id());
  q.exec();
  sFillList();
}

void apAccountAssignments::sFillList()
{
  q.prepare( "SELECT apaccnt_id,"
             "       CASE WHEN (apaccnt_vendtype='.*') THEN :all"
             "            WHEN (apaccnt_vendtype<> '') THEN apaccnt_vendtype"
             "            ELSE (SELECT vendtype_code FROM vendtype WHERE (vendtype_id=apaccnt_vendtype_id))"
             "       END AS vendtypecode,"
             "       formatGLAccount(apaccnt_ap_accnt_id) AS apaccnt,"
             "       formatGLAccount(apaccnt_prepaid_accnt_id) AS prepaidaccnt,"
             "       formatGLAccount(apaccnt_discount_accnt_id) AS discountaccnt "
             "FROM apaccnt "
             "ORDER BY vendtypecode;" );
  q.bindValue(":all", tr("All"));
  q.exec();
  _apaccnt->populate(q);
}
