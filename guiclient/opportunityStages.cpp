/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "opportunityStages.h"

#include <QVariant>
#include <QMessageBox>
//#include <QStatusBar>

#include <parameter.h>
#include <openreports.h>
#include "opportunityStage.h"
#include "guiclient.h"

/*
 *  Constructs a opportunityStages as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
opportunityStages::opportunityStages(QWidget* parent, const char* name, Qt::WFlags fl)
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
  
  
  _oplist->addColumn(tr("Name"),        _itemColumn, Qt::AlignLeft, true, "opstage_name" );
  _oplist->addColumn(tr("Description"), -1,          Qt::AlignLeft, true, "opstage_descrip" );

  if (_privileges->check("MaintainOpportunityStages"))
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
opportunityStages::~opportunityStages()
{
  // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void opportunityStages::languageChange()
{
  retranslateUi(this);
}

void opportunityStages::sDelete()
{
  q.prepare( "SELECT ophead_id "
             "FROM ophead "
             "WHERE (ophead_opstage_id=:opstage_id);" );
  q.bindValue(":opstage_id", _oplist->id());
  q.exec();
  if (q.first())
  {
    QMessageBox::critical( this, tr("Cannot Delete Opportunity Stage"),
                           tr( "The selected Opportunity Stage cannot be deleted as there are one or more Opportunities currently assigned to it.\n"
                               "You must reassign these Opportunities before you may delete the selected Opportunity Stage." ) );
    return;
  }

  q.prepare( "DELETE FROM opstage "
             "WHERE (opstage_id=:opstage_id);" );
  q.bindValue(":opstage_id", _oplist->id());
  q.exec();

  sFillList();
}

void opportunityStages::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("opstage_id", _oplist->id());

  opportunityStage newdlg(this, "", TRUE);
  newdlg.set(params);
  
  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void opportunityStages::sView()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("opstage_id", _oplist->id());

  opportunityStage newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void opportunityStages::sFillList()
{
  _oplist->populate( "SELECT opstage_id, opstage_name, opstage_descrip "
                     "  FROM opstage "
                     " ORDER BY opstage_name;" );
}

void opportunityStages::sNew()
{
  ParameterList params;
  params.append("mode", "new");

  opportunityStage newdlg(this, "", TRUE);
  newdlg.set(params);
  
  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void opportunityStages::sPrint()
{
  orReport report("OpportunityStageMasterList");
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}


