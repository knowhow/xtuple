/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "openReturnAuthorizations.h"

#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include <openreports.h>
#include <metasql.h>

#include "mqlutil.h"
#include "returnAuthorization.h"
#include "openReturnAuthorizations.h"
#include "printRaForm.h"

openReturnAuthorizations::openReturnAuthorizations(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_ra, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*)));
  connect(_close, SIGNAL(clicked()), this, SLOT(close()));
  connect(_edit, SIGNAL(clicked()), this, SLOT(sEdit()));
  connect(_view, SIGNAL(clicked()), this, SLOT(sView()));
  connect(_ra, SIGNAL(valid(bool)), _view, SLOT(setEnabled(bool)));
  connect(_new, SIGNAL(clicked()), this, SLOT(sNew()));
  connect(_delete, SIGNAL(clicked()), this, SLOT(sDelete()));
  connect(_expired, SIGNAL(clicked()), this, SLOT(sFillList()));
  connect(_unauthorized, SIGNAL(clicked()), this, SLOT(sFillList()));

  _ra->addColumn(tr("Return #"),         _orderColumn, Qt::AlignLeft,   true,  "rahead_number"   );
  _ra->addColumn(tr("Cust. #"),          _orderColumn, Qt::AlignLeft,   true,  "custnumber"   );
  _ra->addColumn(tr("Customer"),         -1,           Qt::AlignLeft,   true,  "rahead_billtoname"   );
  _ra->addColumn(tr("Disposition"),      _itemColumn,  Qt::AlignLeft,   true,  "disposition"   );
  _ra->addColumn(tr("Created"),          _dateColumn,  Qt::AlignCenter, true,  "rahead_authdate" );
  _ra->addColumn(tr("Expires"),          -1,           Qt::AlignCenter, true,  "rahead_expiredate" );
  
  if (_privileges->check("MaintainReturns"))
  {
    connect(_ra, SIGNAL(valid(bool)), _edit, SLOT(setEnabled(bool)));
    connect(_ra, SIGNAL(valid(bool)), _delete, SLOT(setEnabled(bool)));
    connect(_ra, SIGNAL(itemSelected(int)), _edit, SLOT(animateClick()));
  }
  else
  {
    _new->setEnabled(FALSE);
    connect(_ra, SIGNAL(itemSelected(int)), _view, SLOT(animateClick()));
  }

  connect(omfgThis, SIGNAL(returnAuthorizationsUpdated()), this, SLOT(sFillList()));
}

openReturnAuthorizations::~openReturnAuthorizations()
{
    // no need to delete child widgets, Qt does it all for us
}

void openReturnAuthorizations::languageChange()
{
    retranslateUi(this);
}

enum SetResponse openReturnAuthorizations::set(const ParameterList& pParams)
{
  XWidget::set(pParams);
  QVariant param;
  bool	   valid;
  
  param = pParams.value("run", &valid);
  if (valid)
  {
    connect(_warehouse, SIGNAL(updated()), this, SLOT(sFillList()));
    sFillList();
  }

  return NoError;
}

void openReturnAuthorizations::setParams(ParameterList &params)
{
  if (_preferences->boolean("selectedSites") || _warehouse->isSelected())
    params.append("selectedSites");
  _warehouse->appendValue(params);
  if(_expired->isChecked())
    params.append("showExpired");
  if(_unauthorized->isChecked())
    params.append("showUnauthorized");
  params.append("undefined", tr("Undefined"));
  params.append("credit", tr("Credit"));
  params.append("return", tr("Return"));
  params.append("replace", tr("Replace"));
  params.append("service", tr("Service"));
  params.append("substitute", tr("Substitute"));
}

void openReturnAuthorizations::sPrint()
{
  ParameterList params;
  setParams(params);

  orReport report("ListOpenReturnAuthorizations", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void openReturnAuthorizations::sNew()
{
  ParameterList params;
  params.append("mode", "new");

  returnAuthorization *newdlg = new returnAuthorization();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void openReturnAuthorizations::sEdit()
{
  if (!checkSitePrivs(_ra->id()))
    return;
    
  ParameterList params;
  params.append("mode", "edit");
  params.append("rahead_id", _ra->id());

  returnAuthorization *newdlg = new returnAuthorization();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void openReturnAuthorizations::sView()
{  
  if (!checkSitePrivs(_ra->id()))
    return;
    
  ParameterList params;
  params.append("mode", "view");
  params.append("rahead_id", _ra->id());

  returnAuthorization *newdlg = new returnAuthorization();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void openReturnAuthorizations::sDelete()
{
  if (!checkSitePrivs(_ra->id()))
    return;
    
  if ( QMessageBox::warning( this, tr("Delete Return Authorization?"),
                             tr("Are you sure that you want to completely delete the selected Return Authorization?"),
                             tr("&Yes"), tr("&No"), QString::null, 0, 1 ) == 0 )
  {
	q.prepare("DELETE FROM rahead WHERE (rahead_id=:rahead_id);");
    q.bindValue(":rahead_id", _ra->id());
    q.exec();
    if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    }
  }
  omfgThis->sReturnAuthorizationsUpdated();
}

void openReturnAuthorizations::sPopulateMenu(QMenu *pMenu)
{
  QAction *menuItem;

  menuItem = pMenu->addAction(tr("Edit..."), this, SLOT(sEdit()));
  menuItem->setEnabled(_privileges->check("MaintainReturns"));

  menuItem = pMenu->addAction(tr("View..."), this, SLOT(sView()));

  menuItem = pMenu->addAction(tr("Delete..."), this, SLOT(sDelete()));
  menuItem->setEnabled(_privileges->check("MaintainReturns"));

  pMenu->addSeparator();

  menuItem = pMenu->addAction(tr("Print Return Authorization Form..."), this, SLOT(sPrintForms())); 

}


void openReturnAuthorizations::sFillList()
{
  MetaSQLQuery mql = mqlLoad("returnAuthorizations", "detail");
  ParameterList params;
  setParams(params);
  q = mql.toQuery(params);
  _ra->populate(q);
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
  _ra->setDragString("raheadid=");
}

void openReturnAuthorizations::sPrintForms()
{
  if (!checkSitePrivs(_ra->id()))
    return;
    
  ParameterList params;
  params.append("rahead_id", _ra->id());

  printRaForm newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

bool openReturnAuthorizations::checkSitePrivs(int orderid)
{
  if (_preferences->boolean("selectedSites"))
  {
    XSqlQuery check;
    check.prepare("SELECT checkRASitePrivs(:raheadid) AS result;");
    check.bindValue(":raheadid", orderid);
    check.exec();
    if (check.first())
    {
      if (!check.value("result").toBool())
      {
        QMessageBox::critical(this, tr("Access Denied"),
                              tr("You may not view or edit this Return Authorization as it references "
                                 "a Site for which you have not been granted privileges.")) ;
        return false;
      }
    }
  }
  return true;
}
