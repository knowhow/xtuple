/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "opportunityTypes.h"

#include <QVariant>
#include <QMessageBox>
//#include <QStatusBar>

#include <parameter.h>
#include <openreports.h>
#include "opportunityType.h"
#include "guiclient.h"

/*
 *  Constructs a opportunityTypes as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
opportunityTypes::opportunityTypes(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

//  (void)statusBar();

  // signals and slots connections
  connect(_new, SIGNAL(clicked()), this, SLOT(sNew()));
  connect(_edit, SIGNAL(clicked()), this, SLOT(sEdit()));
  connect(_view, SIGNAL(clicked()), this, SLOT(sView()));
  connect(_delete, SIGNAL(clicked()), this, SLOT(sDelete()));
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));

//  statusBar()->hide();
  _print->hide();
  
  
  _oplist->addColumn(tr("Name"),        _itemColumn, Qt::AlignLeft, true, "optype_name" );
  _oplist->addColumn(tr("Description"), -1,          Qt::AlignLeft, true, "optype_descrip" );

  if (_privileges->check("MaintainOpportunityTypes"))
  {
    connect(_oplist, SIGNAL(valid(bool)), _edit, SLOT(setEnabled(bool)));
    connect(_oplist, SIGNAL(valid(bool)), _delete, SLOT(setEnabled(bool)));
    connect(_oplist, SIGNAL(itemSelected(int)), _edit, SLOT(animateClick()));
  }
  else
  {
    connect(_oplist, SIGNAL(itemSelected(int)), _view, SLOT(animateClick()));
    _new->setEnabled(FALSE);
  }

   sFillList();
}

/*
 *  Destroys the object and frees any allocated resources
 */
opportunityTypes::~opportunityTypes()
{
  // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void opportunityTypes::languageChange()
{
  retranslateUi(this);
}

void opportunityTypes::sDelete()
{
  q.prepare( "SELECT ophead_id "
             "FROM ophead "
             "WHERE (ophead_optype_id=:optype_id);" );
  q.bindValue(":optype_id", _oplist->id());
  q.exec();
  if (q.first())
  {
    QMessageBox::critical( this, tr("Cannot Delete Opportunity Type"),
                           tr( "The selected Opportunity Type cannot be deleted as there are one or more Opportunities currently assigned to it.\n"
                               "You must reassign these Opportunities before you may delete the selected Opportunity Type." ) );
    return;
  }

  q.prepare( "DELETE FROM optype "
             "WHERE (optype_id=:optype_id);" );
  q.bindValue(":optype_id", _oplist->id());
  q.exec();

  sFillList();
}

void opportunityTypes::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("optype_id", _oplist->id());

  opportunityType newdlg(this, "", TRUE);
  newdlg.set(params);
  
  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void opportunityTypes::sView()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("optype_id", _oplist->id());

  opportunityType newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void opportunityTypes::sFillList()
{
  _oplist->populate( "SELECT optype_id, optype_name, optype_descrip "
                     "  FROM optype "
                     " ORDER BY optype_name;" );
}

void opportunityTypes::sNew()
{
  ParameterList params;
  params.append("mode", "new");

  opportunityType newdlg(this, "", TRUE);
  newdlg.set(params);
  
  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void opportunityTypes::sPrint()
{
  orReport report("OpportunityTypeMasterList");
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}


