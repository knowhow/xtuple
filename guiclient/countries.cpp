/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "countries.h"

#include <QAction>
#include <QMenu>
#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include <parameter.h>

#include "country.h"

countries::countries(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  connect(_new,         SIGNAL(clicked()),	this,	SLOT(sNew()));
  connect(_edit,	SIGNAL(clicked()),	this,	SLOT(sEdit()));
  connect(_close,	SIGNAL(clicked()),	this,	SLOT(close()));
  connect(_delete,	SIGNAL(clicked()),	this,	SLOT(sDelete()));
  connect(_view,	SIGNAL(clicked()),	this,	SLOT(sView()));
  connect(_countries,	SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*)),
                                        this,	SLOT(sPopulateMenu(QMenu*)));

  if (_privileges->check("MaintainCountries"))
  {
    connect(_countries, SIGNAL(valid(bool)), _edit, SLOT(setEnabled(bool)));
    connect(_countries, SIGNAL(valid(bool)), _delete, SLOT(setEnabled(bool)));
    connect(_countries, SIGNAL(itemSelected(int)), _edit, SLOT(animateClick()));
  }
  else
  {
    _new->setEnabled(FALSE);
    connect(_countries, SIGNAL(itemSelected(int)), _view, SLOT(animateClick()));
  }
    
  _countries->addColumn( tr("Abbreviation"),  50, Qt::AlignCenter,true, "country_abbr");
  _countries->addColumn( tr("Name"),          -1, Qt::AlignLeft,  true, "country_name");
  _countries->addColumn( tr("Currency Abbr"),100, Qt::AlignCenter,true, "country_curr_abbr");
  _countries->addColumn( tr("Currency Name"),100, Qt::AlignLeft,  true, "country_curr_name");
  _countries->addColumn( tr("Symbol"),        50, Qt::AlignLeft,  true, "country_curr_symbol");
    
  sFillList();
}

countries::~countries()
{
    // no need to delete child widgets, Qt does it all for us
}

void countries::languageChange()
{
    retranslateUi(this);
}

void countries::sNew()
{
  ParameterList params;
  params.append("mode", "new");
    
  country *newdlg = new country(this, "", TRUE);
  newdlg->set(params);
  newdlg->exec();
  sFillList();
}

void countries::sEdit()
{
  ParameterList params;
  params.append("country_id", _countries->id());
  params.append("mode", "edit");
    
  country *newdlg = new country(this, "", TRUE);
  newdlg->set(params);
  newdlg->exec();
  sFillList();
}

void countries::sView()
{
  ParameterList params;
  params.append("country_id", _countries->id());
  params.append("mode", "view");
    
  country *newdlg = new country(this, "", TRUE);
  newdlg->set(params);
  newdlg->exec();
}

void countries::sDelete()
{
  int answer = QMessageBox::question(this,
				     tr("Confirm Delete"),
				     tr("<p>This is a master table used for "
					"selecting country names for Addresses "
					"and default currency symbols for the "
					"Currency window. Are you sure you "
					"want to delete this country and its "
					"associated information?"),
				      QMessageBox::Yes,
				      QMessageBox::No | QMessageBox::Default);
  if (QMessageBox::Yes == answer)
  {
    q.prepare("DELETE FROM country WHERE country_id = :country_id;");
    q.bindValue(":country_id", _countries->id());
    q.exec();
    if (q.lastError().type() != QSqlError::NoError)
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
  
    sFillList();
  }
}

void countries::sFillList()
{
  q.prepare( "SELECT country_id, country_abbr, country_name,"
             "       country_curr_abbr, country_curr_name, "
             "       country_curr_symbol "
             "FROM country "
             "ORDER BY country_name;" );
  q.exec();
  _countries->populate(q);
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void countries::sPopulateMenu(QMenu* pMenu)
{
    QAction *menuItem;
    
    pMenu->addAction("View...", this, SLOT(sView()));
    
    menuItem = pMenu->addAction("Edit...", this, SLOT(sEdit()));
    menuItem->setEnabled(_privileges->check("MaintainCountries"));
    
    menuItem = pMenu->addAction("Delete...", this, SLOT(sDelete()));
    menuItem->setEnabled(_privileges->check("MaintainCountries"));
}
