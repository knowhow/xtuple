/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "plannerCodes.h"

#include <QVariant>
#include <QMessageBox>
//#include <QStatusBar>

#include <parameter.h>
#include <openreports.h>
#include "plannerCode.h"
#include "guiclient.h"

/*
 *  Constructs a plannerCodes as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
plannerCodes::plannerCodes(QWidget* parent, const char* name, Qt::WFlags fl)
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
  
  
  _plancode->addColumn(tr("Code"),        _itemColumn, Qt::AlignLeft, true, "plancode_code" );
  _plancode->addColumn(tr("Description"), -1,          Qt::AlignLeft, true, "plancode_name" );

  if (_privileges->check("MaintainPlannerCodes"))
  {
    connect(_plancode, SIGNAL(valid(bool)), _edit, SLOT(setEnabled(bool)));
    connect(_plancode, SIGNAL(valid(bool)), _delete, SLOT(setEnabled(bool)));
    connect(_plancode, SIGNAL(itemSelected(int)), _edit, SLOT(animateClick()));
  }
  else
  {
    connect(_plancode, SIGNAL(itemSelected(int)), _view, SLOT(animateClick()));
    _new->setEnabled(FALSE);
  }

   sFillList();
}

/*
 *  Destroys the object and frees any allocated resources
 */
plannerCodes::~plannerCodes()
{
  // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void plannerCodes::languageChange()
{
  retranslateUi(this);
}

void plannerCodes::sDelete()
{
  q.prepare( "SELECT itemsite_id "
             "FROM itemsite "
             "WHERE (itemsite_plancode_id=:plancode_id);" );
  q.bindValue(":plancode_id", _plancode->id());
  q.exec();
  if (q.first())
  {
    QMessageBox::critical( this, tr("Cannot Delete Planner Code"),
                           tr( "The selected Planner Code cannot be deleted as there are one or more Item Sites currently assigned to it.\n"
                               "You must reassign these Item Sites before you may delete the selected Planner Code." ) );
    return;
  }

  q.prepare( "DELETE FROM plancode "
             "WHERE (plancode_id=:plancode_id);" );
  q.bindValue(":plancode_id", _plancode->id());
  q.exec();

  sFillList();
}

void plannerCodes::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("plancode_id", _plancode->id());

  plannerCode newdlg(this, "", TRUE);
  newdlg.set(params);
  
  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void plannerCodes::sView()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("plancode_id", _plancode->id());

  plannerCode newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void plannerCodes::sFillList()
{
  _plancode->populate( "SELECT plancode_id, plancode_code, plancode_name "
	                      "FROM plancode "
	                      "ORDER BY plancode_code;" );
}

void plannerCodes::sNew()
{
  ParameterList params;
  params.append("mode", "new");

  plannerCode newdlg(this, "", TRUE);
  newdlg.set(params);
  
  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void plannerCodes::sPrint()
{
  orReport report("PlannerCodeMasterList");
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}


