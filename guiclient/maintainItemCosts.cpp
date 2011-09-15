/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "maintainItemCosts.h"

#include <QAction>
#include <QMenu>
#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include "dspItemCostDetail.h"
#include "itemCost.h"

maintainItemCosts::maintainItemCosts(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
    setupUi(this);

    connect(_item, SIGNAL(newId(int)), this, SLOT(sFillList()));
    connect(_new, SIGNAL(clicked()), this, SLOT(sNew()));
    connect(_edit, SIGNAL(clicked()), this, SLOT(sEdit()));
    connect(_delete, SIGNAL(clicked()), this, SLOT(sDelete()));
    connect(_close, SIGNAL(clicked()), this, SLOT(close()));
    connect(_itemcost, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*,QTreeWidgetItem*)));
    connect(_itemcost, SIGNAL(itemSelected(int)), _edit, SLOT(animateClick()));
    connect(_itemcost, SIGNAL(itemSelectionChanged()), this, SLOT(sSelectionChanged()));
    
    _itemcost->addColumn(tr("Element"),     -1,           Qt::AlignLeft,   true, "costelem_type");
    _itemcost->addColumn(tr("Lower"),       _costColumn,  Qt::AlignCenter, true, "itemcost_lowlevel" );
    _itemcost->addColumn(tr("Std. Cost"),   _costColumn,  Qt::AlignRight,  true, "itemcost_stdcost"  );
    _itemcost->addColumn(tr("Currency"),    _currencyColumn, Qt::AlignLeft,true, "baseCurr" );
    _itemcost->addColumn(tr("Posted"),      _dateColumn,  Qt::AlignCenter, true, "itemcost_posted" );
    _itemcost->addColumn(tr("Act. Cost"),   _costColumn,  Qt::AlignRight,  true, "itemcost_actcost"  );
    _itemcost->addColumn(tr("Currency"),    _currencyColumn, Qt::AlignLeft,true, "costCurr" );
    _itemcost->addColumn(tr("Updated"),     _dateColumn,  Qt::AlignCenter, true, "itemcost_updated" );

    if (omfgThis->singleCurrency())
    {
	_itemcost->hideColumn(3);
	_itemcost->hideColumn(6);
    }

    if (_privileges->check("CreateCosts"))
      connect(_item, SIGNAL(valid(bool)), _new, SLOT(setEnabled(bool)));
}

maintainItemCosts::~maintainItemCosts()
{
    // no need to delete child widgets, Qt does it all for us
}

void maintainItemCosts::languageChange()
{
    retranslateUi(this);
}

enum SetResponse maintainItemCosts::set(const ParameterList &pParams)
{
  XWidget::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("item_id", &valid);
  if (valid)
  {
    _item->setId(param.toInt());
    _item->setReadOnly(TRUE);
  }

  return NoError;
}

void maintainItemCosts::sPopulateMenu(QMenu *pMenu, QTreeWidgetItem *pSelected)
{
  if (((XTreeWidgetItem *)pSelected)->id() == -1)
    return;

  QAction *menuItem;

  if (pSelected->text(1) == "No")
  {
    if (pSelected->text(0) == "Direct Labor")
    {
      menuItem = pMenu->addAction(tr("Update Actual Cost..."), this, SLOT(sUpdateDirectLabor()));
      menuItem->setEnabled(_privileges->check("UpdateActualCosts"));
    }

    else if (pSelected->text(0) == "Overhead")
    {
      menuItem = pMenu->addAction(tr("Update Actual Cost..."), this, SLOT(sUpdateOverhead()));
      menuItem->setEnabled(_privileges->check("UpdateActualCosts"));
    }

    else if (pSelected->text(0) == "Machine Overhead")
    {
      menuItem = pMenu->addAction(tr("Update Actual Cost..."), this, SLOT(sUpdateMachineOverhead()));
      menuItem->setEnabled(_privileges->check("UpdateActualCosts"));
    }
  }
  else
  {
    pMenu->addAction(tr("View Costing Detail..."), this, SLOT(sViewDetail()));
    pMenu->addSeparator();

    menuItem = pMenu->addAction(tr("Update Actual Cost..."), this, SLOT(sUpdateDetail()));
    menuItem->setEnabled(_privileges->check("UpdateActualCosts"));
  }

  if (((XTreeWidgetItem *)pSelected)->altId() == 0)
  {
    menuItem = pMenu->addAction(tr("Delete Cost..."), this, SLOT(sDelete()));
    menuItem->setEnabled(_privileges->check("DeleteCosts"));
  }

  menuItem = pMenu->addAction(tr("Post Actual Cost to Standard..."), this, SLOT(sPost()));
  menuItem->setEnabled(_privileges->check("PostActualCosts"));

  pMenu->addSeparator();

  menuItem = pMenu->addAction(tr("Edit Actual Cost..."), this, SLOT(sEnterActualCost()));
  menuItem->setEnabled(_privileges->check("EnterActualCosts"));

  menuItem = pMenu->addAction(tr("New Actual Cost..."), this, SLOT(sCreateUserCost()));
  menuItem->setEnabled(_privileges->check("CreateCosts"));
}

void maintainItemCosts::sViewDetail()
{
  ParameterList params;
  params.append("itemcost_id", _itemcost->id());
  params.append("run");

  dspItemCostDetail *newdlg = new dspItemCostDetail();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void maintainItemCosts::sUpdateDetail()
{
  QString cost = _itemcost->currentItem()->text(0);

  q.prepare("SELECT updateCost(:item_id, :cost, TRUE, lowerCost(:item_id, :cost));");
  q.bindValue(":item_id", _item->id());
  q.bindValue(":cost", cost);
  q.exec();

  sFillList();
}

void maintainItemCosts::sUpdateDirectLabor()
{
  q.prepare("SELECT updateCost(:item_id, 'Direct Labor', FALSE, directLaborCost(:item_id));");
  q.bindValue(":item_id", _item->id());
  q.exec();

  sFillList();
}

void maintainItemCosts::sUpdateOverhead()
{
  q.prepare("SELECT updateCost(:item_id, 'Overhead', FALSE, overheadCost(:item_id));");
  q.bindValue(":item_id", _item->id());
  q.exec();

  sFillList();
}

void maintainItemCosts::sUpdateMachineOverhead()
{
  q.prepare("SELECT updateCost(:item_id, 'Machine Overhead', FALSE, machineOverheadCost(:item_id));");
  q.bindValue(":item_id", _item->id());
  q.exec();

  sFillList();
}

void maintainItemCosts::sPost()
{
  q.prepare("SELECT postCost(:item_id);");
  q.bindValue(":item_id", _itemcost->id());
  q.exec();
  if (q.lastError().type() != QSqlError::NoError)
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);

  sFillList();
}

void maintainItemCosts::sDelete()
{
  double stdCost = 0.0;
  q.prepare( "SELECT itemcost_stdcost "
	     "FROM itemcost "
	     "WHERE (itemcost_id=:itemcost_id);" );
  q.bindValue(":itemcost_id", _itemcost->id());
  q.exec();
  if (q.first())
    stdCost = q.value("itemcost_stdcost").toDouble();
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  if (stdCost > 0)
  {
    if (QMessageBox::warning(this, tr("Deletion of Costing Elements"),
			     tr("<p>Before a Costing Element is deleted, the "
				"system will set the Actual Cost value to 0. "
				"This helps ensure Inventory is valued "
				"correctly. Once the 0 value Actual Cost is "
				"posted to Standard, the Costing Element will "
				"be removed."),
			     QMessageBox::Ok | QMessageBox::Default,
			     QMessageBox::Cancel) == QMessageBox::Cancel)
      return;
    q.prepare("SELECT updateCost(:itemcost_id, 0);");
  }
  else
  {
    q.prepare( "DELETE FROM itemcost WHERE (itemcost_id=:itemcost_id);" );
  }
  q.bindValue(":itemcost_id", _itemcost->id());
  q.exec();
  if (q.lastError().type() != QSqlError::NoError)
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);

  sFillList();
}

void maintainItemCosts::sEnterActualCost()
{
  ParameterList params;
  params.append("itemcost_id", _itemcost->id());
  params.append("mode", "edit");

  itemCost newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec())
    sFillList();
}

void maintainItemCosts::sCreateUserCost()
{
  ParameterList params;
  params.append("item_id", _item->id());
  params.append("mode", "new");

  itemCost newdlg(this, "", TRUE);
  if (newdlg.set(params) == NoError && newdlg.exec())
    sFillList();
}

void maintainItemCosts::sNew()
{
  sCreateUserCost();
}

void maintainItemCosts::sEdit()
{
  sEnterActualCost();
}

void maintainItemCosts::sFillList()
{
  if (_item->isValid())
  {
    double standardCost = 0.0;
    double actualCostBase = 0.0;
    double actualCostLocal = 0.0;

    q.prepare( "SELECT itemcost_id,"
               "       CASE WHEN (costelem_sys) THEN 1"
               "            ELSE 0"
               "       END,"
               "       CASE WHEN itemcost_costelem_id = -1 THEN :error "
	       "	    ELSE costelem_type "
	       "	END AS costelem_type, itemcost_lowlevel,"
               "       itemcost_stdcost, currConcat(baseCurrID()) AS baseCurr, "
	       "       itemcost_posted,"
               "       itemcost_actcost, currConcat(itemcost_curr_id) AS costCurr, "
	       "       itemcost_updated,"
	       "       currToBase(itemcost_curr_id, itemcost_actcost, CURRENT_DATE) AS actcostBase, "
	       "       itemcost_curr_id, "
               "       CASE WHEN (COALESCE(itemcost_posted, startOfTime()) <= startOfTime()) THEN :never"
               "       END AS itemcost_posted_qtdisplayrole,"
               "       'cost' AS itemcost_stdcost_xtnumericrole, "
               "       'cost' AS itemcost_actcost_xtnumericrole "
               "FROM itemcost LEFT OUTER JOIN costelem ON "
               "	(itemcost_costelem_id=costelem_id)"
               "WHERE (itemcost_item_id=:item_id) "
               "ORDER BY itemcost_lowlevel, costelem_type;" );
    q.bindValue(":item_id", _item->id());
    q.bindValue(":error", tr("!ERROR!"));
    q.bindValue(":never", tr("Never"));
    q.exec();
    _itemcost->populate(q, TRUE);

    bool multipleCurrencies = false;
    int firstCurrency = 0;
    bool baseKnown = true;
    if (q.first())
    {
      firstCurrency = q.value("itemcost_curr_id").toInt();
      do
      {
        standardCost += q.value("itemcost_stdcost").toDouble();
	if (q.value("itemcost_actcost").isNull())
	    baseKnown = false;
	else
	    actualCostBase += q.value("actcostBase").toDouble();
        actualCostLocal += q.value("itemcost_actcost").toDouble();
	if (! multipleCurrencies &&
	    q.value("itemcost_curr_id").toInt() != firstCurrency)
	    multipleCurrencies = true;
      }
      while (q.next());
    }

    XSqlQuery convert;
    double actualCost = 0;
    if (multipleCurrencies)
    {
	actualCost = actualCostBase;
	convert.prepare("SELECT currConcat(baseCurrId()) AS baseConcat, "
			"       currConcat(baseCurrId()) AS currConcat;");
    }
    else
    {
	actualCost = actualCostLocal;
	baseKnown = true; // not necessarily but we can trust actualCost
	convert.prepare("SELECT currConcat(baseCurrId()) AS baseConcat, "
			"	currConcat(:firstCurrency) AS currConcat;" );
	convert.bindValue(":firstCurrency", firstCurrency);
    }
    convert.exec();
    if (convert.first())
	new XTreeWidgetItem(_itemcost,
			    _itemcost->topLevelItem(_itemcost->topLevelItemCount() - 1), -1,
			    QVariant(tr("Totals")),
			    "",
			    formatCost(standardCost),
			    convert.value("baseConcat"),
			    "",
			    baseKnown ? formatCost(actualCost) : tr("?????"),
			    convert.value("currConcat"));
    else if (convert.lastError().type() != QSqlError::NoError)
	systemError(this, convert.lastError().databaseText(), __FILE__, __LINE__);

  }
  else
    _itemcost->clear();
}

void maintainItemCosts::sSelectionChanged()
{
  bool yes = (_itemcost->id() != -1);

  if (_privileges->check("EnterActualCosts"))
    _edit->setEnabled(yes);

  if (_privileges->check("DeleteCosts"))
    _delete->setEnabled(yes);
}

