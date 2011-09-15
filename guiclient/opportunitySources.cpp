/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "opportunitySources.h"

#include <QVariant>
#include <QMessageBox>
//#include <QStatusBar>

#include <parameter.h>
#include <openreports.h>
#include "opportunitySource.h"
#include "guiclient.h"

/*
 *  Constructs a opportunitySources as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
opportunitySources::opportunitySources(QWidget* parent, const char* name, Qt::WFlags fl)
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
  
  
  _oplist->addColumn(tr("Name"),        _itemColumn, Qt::AlignLeft, true, "opsource_name" );
  _oplist->addColumn(tr("Description"), -1,          Qt::AlignLeft, true, "opsource_descrip" );

  if (_privileges->check("MaintainOpportunitySources"))
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
opportunitySources::~opportunitySources()
{
  // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void opportunitySources::languageChange()
{
  retranslateUi(this);
}

void opportunitySources::sDelete()
{
  q.prepare( "SELECT ophead_id "
             "FROM ophead "
             "WHERE (ophead_opsource_id=:opsource_id);" );
  q.bindValue(":opsource_id", _oplist->id());
  q.exec();
  if (q.first())
  {
    QMessageBox::critical( this, tr("Cannot Delete Opportunity Source"),
                           tr( "The selected Opportunity Source cannot be deleted as there are one or more Opportunities currently assigned to it.\n"
                               "You must reassign these Opportunities before you may delete the selected Opportunity Source." ) );
    return;
  }

  q.prepare( "DELETE FROM opsource "
             "WHERE (opsource_id=:opsource_id);" );
  q.bindValue(":opsource_id", _oplist->id());
  q.exec();

  sFillList();
}

void opportunitySources::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("opsource_id", _oplist->id());

  opportunitySource newdlg(this, "", TRUE);
  newdlg.set(params);
  
  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void opportunitySources::sView()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("opsource_id", _oplist->id());

  opportunitySource newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void opportunitySources::sFillList()
{
  _oplist->populate( "SELECT opsource_id, opsource_name, opsource_descrip "
                     "  FROM opsource "
                     " ORDER BY opsource_name;" );
}

void opportunitySources::sNew()
{
  ParameterList params;
  params.append("mode", "new");

  opportunitySource newdlg(this, "", TRUE);
  newdlg.set(params);
  
  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void opportunitySources::sPrint()
{
  orReport report("OpportunitySourceMasterList");
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}


