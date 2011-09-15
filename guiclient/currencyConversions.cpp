/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "currencyConversions.h"

#include <QAction>
#include <QMenu>
#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include <openreports.h>
#include <metasql.h>
#include "mqlutil.h"

#include "currencyConversion.h"
#include "currency.h"
#include "datecluster.h"
#include "xcombobox.h"

currencyConversions::currencyConversions(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  connect(_close, SIGNAL(clicked()), this, SLOT(sClose()));
  connect(_conversionRates, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*)));
  connect(_delete, SIGNAL(clicked()), this, SLOT(sDelete()));
  connect(_edit, SIGNAL(clicked()), this, SLOT(sEdit()));
  connect(_new, SIGNAL(clicked()), this, SLOT(sNew()));
  connect(_newCurrency, SIGNAL(clicked()), this, SLOT(sNewCurrency()));
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_view, SIGNAL(clicked()), this, SLOT(sView()));
  connect(_query, SIGNAL(clicked()), this, SLOT(sFillList()));

  setBaseCurrency();
  _conversionRates->addColumn(tr("Currency"),_currencyColumn, Qt::AlignLeft, true, "f_curr");
  _conversionRates->addColumn(tr("Exchange Rate"),        -1, Qt::AlignRight,true, "rate");
  _conversionRates->addColumn(tr("Effective"),   _dateColumn, Qt::AlignCenter,true, "curr_effective");
  _conversionRates->addColumn(tr("Expires"),     _dateColumn, Qt::AlignCenter,true, "curr_expires");
  
  _newCurrency->setEnabled(_privileges->check("CreateNewCurrency"));
  
  bool maintainPriv = _privileges->check("MaintainCurrencyRates");
  _new->setEnabled(maintainPriv);
  if (maintainPriv)
  {
    connect(_conversionRates, SIGNAL(valid(bool)), _edit, SLOT(setEnabled(bool)));
    connect(_conversionRates, SIGNAL(itemSelected(int)), _edit, SLOT(animateClick()));
    connect(_conversionRates, SIGNAL(valid(bool)), _delete, SLOT(setEnabled(bool)));
  }
  else
  {
    connect(_conversionRates, SIGNAL(itemSelected(int)), _view, SLOT(animateClick()));
  }
  
  connect(_conversionRates, SIGNAL(valid(bool)), _view, SLOT(setEnabled(bool)));

  _dateCluster->setStartNull(tr("Earliest"), omfgThis->startOfTime(), TRUE);
  _dateCluster->setEndNull(tr("Latest"), omfgThis->endOfTime(), TRUE);
  
  _queryParameters->setType(ParameterGroup::CurrencyNotBase);

  switch (_metrics->value("CurrencyExchangeSense").toInt())
  {
    case 0:
        _exchSenseLit->setText(tr("Base x Exchange Rate = Foreign"));
        break;
    case 1:
        _exchSenseLit->setText(tr("Foreign x Exchange Rate = Base"));
        break;
    case -1:
    default:
        QMessageBox::warning(this, tr("No Exchange Rate Direction Defined"),
                               tr("<p>No selection has yet been made for "
                                  "whether exchange rates convert from "
                                  "the base currency to foreign currencies "
                                  "or from foreign to base. Go to "
                                  "System | Configure Modules | Configure "
                                  "G/L and make your selection."));
        close();
  }

  sFillList();
}

currencyConversions::~currencyConversions()
{
  // no need to delete child widgets, Qt does it all for us
}

void currencyConversions::languageChange()
{
  retranslateUi(this);
}

void currencyConversions::sNew()
{
    if (omfgThis->singleCurrency())
      QMessageBox::warning(this, tr("No Foreign Currencies Defined"),
                                 tr("<p>There is only a base currency defined. "
                                    "You must add more currencies before you "
                                    "can create an exchange rate. Click the "
                                    "NEW CURRENCY button to add another "
                                    "currency."));
    else
    {
	ParameterList params;
	params.append("mode", "new");
	if (_queryParameters->isSelected())
	    params.append("curr_id", _queryParameters->id());
	
	currencyConversion newdlg(this, "", TRUE);
	newdlg.set(params);
	newdlg.exec();
	sFillList();
    }
}

void currencyConversions::sEdit()
{
    ParameterList params;
    params.append("mode", "edit");
    params.append("curr_rate_id", _conversionRates->id());
    
    currencyConversion newdlg(this, "", TRUE);
    newdlg.set(params);
    newdlg.exec();
    sFillList();
}

void currencyConversions::sView()
{
    ParameterList params;
    params.append("mode", "view");
    params.append("curr_rate_id", _conversionRates->id());
    
    currencyConversion newdlg(this, "", TRUE);
    newdlg.set(params);
    newdlg.exec();
}

void currencyConversions::sPrint()
{
    ParameterList params;
    if (! setParams(params))
      return;
    
    orReport report("CurrencyConversionList", params);
    if (report.isValid())
	report.print();
    else
	report.reportError(this);
}

void currencyConversions::sClose()
{
    close();
}

void currencyConversions::sDelete()
{
    q.prepare("DELETE FROM curr_rate "
	      "WHERE curr_rate_id = :curr_rate_id");
    q.bindValue(":curr_rate_id", _conversionRates->id());
    q.exec();
    if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
    sFillList();
}

void currencyConversions::sNewCurrency()
{
    ParameterList params;
    params.append("mode", "new");
    
    currency newdlg(this, "", TRUE);
    newdlg.set(params);
    newdlg.exec();
    setBaseCurrency();
    _queryParameters->repopulateSelected();
}

bool currencyConversions::setParams(ParameterList &params)
{
  if (_metrics->value("CurrencyExchangeSense").toInt() == 1)
    params.append("invert");
  _queryParameters->appendValue(params);
  _dateCluster->appendValue(params);
  
  return true;
}

void currencyConversions::sFillList()
{
  MetaSQLQuery mql = mqlLoad("currencyConversions", "detail");
  ParameterList params;
  if (! setParams(params))
    return;
  q = mql.toQuery(params);
  _conversionRates->populate(q);
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void currencyConversions::sPopulateMenu( QMenu* pMenu)
{
  QAction *menuItem;
  
  menuItem = pMenu->addAction(tr("Edit..."), this, SLOT(sEdit()));
  menuItem->setEnabled(_privileges->check("MaintainCurrencyRates"));

  menuItem = pMenu->addAction(tr("View..."), this, SLOT(sView()));

  menuItem = pMenu->addAction(tr("Delete..."), this, SLOT(sDelete()));
  menuItem->setEnabled(_privileges->check("MaintainCurrencyRates"));
}

void currencyConversions::setBaseCurrency()
{
    q.prepare("SELECT currConcat(curr_abbr, curr_symbol) AS baseCurrency "
	      "FROM curr_symbol "
	      "WHERE curr_base = TRUE");
    q.exec();
    if (q.first())
    {
	_baseCurrency->setText(q.value("baseCurrency").toString());
    }
    else if (q.lastError().type() != QSqlError::NoError)
    {
	QMessageBox::critical(this, tr("A System Error occurred at %1::%2.")
			      .arg(__FILE__)
			      .arg(__LINE__),
			      q.lastError().databaseText());
    }
}
