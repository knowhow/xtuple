/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "itemCost.h"

#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include "currcluster.h"
#include "mqlutil.h"
#include "storedProcErrorLookup.h"

static bool _foundCostElems;

itemCost::itemCost(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
  : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_save, SIGNAL(clicked()), this, SLOT(sSave()));

  _captive = false;
  _itemcostid = -1;

  _postCost->setEnabled(_privileges->check("PostActualCosts"));
  _foundCostElems = true; // assume it's true until sPopulateCostelem
}

itemCost::~itemCost()
{
  // no need to delete child widgets, Qt does it all for us
}

void itemCost::languageChange()
{
  retranslateUi(this);
}

enum SetResponse itemCost::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  _captive = true;

  QVariant param;
  bool     valid;

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      _mode = cNew;
      _lowerLevel->hide();
      _lowerLevelLit->hide();
      setWindowTitle("Create Item Cost");
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;
      _costelem->setEnabled(false);
      setWindowTitle("Enter Actual Cost");
    }
    else if (param.toString() == "view")
    {
      _actualCost->setEnabled(false);
      _costelem->setEnabled(false);
      setWindowTitle("View Actual Cost");
    }
  }
      
  param = pParams.value("item_id", &valid);
  if (valid)
  {
    _item->setId(param.toInt());
    _item->setReadOnly(TRUE);
    _actualCost->setFocus();
  }
  else
    _item->setFocus();

  param = pParams.value("itemcost_id", &valid);
  if (valid)
  {
    _itemcostid = param.toInt();
    _item->setReadOnly(TRUE);

    q.prepare( "SELECT item_id, formatBoolYN(itemcost_lowlevel) AS lowlevel,"
               "       itemcost_actcost, itemcost_curr_id, itemcost_updated "
               "FROM item, itemcost "
               "WHERE ( (itemcost_item_id=item_id)"
               " AND    (itemcost_id=:itemcost_id) );" );
    q.bindValue(":itemcost_id", _itemcostid);
    q.exec();
    if (q.first())
    {
      _item->setId(q.value("item_id").toInt());
      _lowerLevel->setText(q.value("lowlevel").toString());
      _actualCost->set(q.value("itemcost_actcost").toDouble(),
		       q.value("itemcost_curr_id").toInt(),
		       QDate::currentDate(),
		       false);
    }
    else if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return UndefinedError;
    }
  }

  sPopulateCostelem();

  return _foundCostElems ? NoError : UndefinedError;
}

void itemCost::sSave()
{
  if (_mode != cNew && _mode != cEdit)
    done(_itemcostid);

  if (_costelem->id() < 0)
  {
    QMessageBox::warning(this, tr("Costing Element Required"),
		 tr("You must select a Costing Element to save this Item Cost."));
    _costelem->setFocus();
    return;
  }

  if (_mode == cNew)
  {
    q.exec("SELECT NEXTVAL('itemcost_itemcost_id_seq') AS itemcost_id;");
    if (q.first())
    {
      _itemcostid = q.value("itemcost_id").toInt();

      q.prepare( "INSERT INTO itemcost "
		 "( itemcost_id, itemcost_item_id,"
		 "  itemcost_costelem_id, itemcost_lowlevel,"
		 "  itemcost_stdcost, itemcost_posted,"
		 "  itemcost_actcost, itemcost_updated, "
		 "  itemcost_curr_id ) "
		 "VALUES "
		 "( :itemcost_id, :itemcost_item_id,"
		 "  :itemcost_costelem_id, FALSE,"
		 "  0, startOfTime(),"
		 "  :itemcost_actcost, CURRENT_DATE, "
		 "  :itemcost_curr_id );" );
      q.bindValue(":itemcost_item_id", _item->id());
      q.bindValue(":itemcost_costelem_id", _costelem->id());
    }
  }
  else if (_mode == cEdit)
  {
      q.prepare( "UPDATE itemcost SET"
		 " itemcost_actcost=:itemcost_actcost,"
		 " itemcost_curr_id=:itemcost_curr_id "
		 "WHERE (itemcost_id=:itemcost_id);");
  }
  q.bindValue(":itemcost_id", _itemcostid);
  q.bindValue(":itemcost_actcost", _actualCost->localValue());
  q.bindValue(":itemcost_curr_id", _actualCost->id());

  if (q.exec() && _postCost->isChecked())
  {
    q.prepare("SELECT postCost(:itemcost_id) AS result;");
    q.bindValue(":itemcost_id", _itemcostid);
    q.exec();
    if (q.first())
    {
      int result = q.value("result").toInt();
      if (result < 0)
      {
        systemError(this, storedProcErrorLookup("postCost", result),
                    __FILE__, __LINE__);
        return;
      }
    }
  }

  if (q.lastError().type() != QSqlError::NoError)  // if EITHER q.exec() failed
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  done(_itemcostid);
}

void itemCost::sPopulateCostelem()
{
  if (_mode == cNew)
  {
    ParameterList params;
    params.append("item_id", _item->id());
    params.append("itemtype", _item->itemType());

    q = mqlLoad("costelem", "unusedbyitem").toQuery(params);
    _costelem->populate(q);
    if (q.size() <= 0)
    {
      QMessageBox::warning(this, tr("No Costing Elements Remaining"),
                           tr("<p>Item %1 already has all available costing "
                              "elements assigned. No new Item Costs can be "
                              "created for it until more costing elements are "
                              "defined.")
                           .arg(_item->itemNumber()));
      _foundCostElems = false;
    }
  }
  else // _mode == cEdit || cView)
  {
    QString sql( "SELECT costelem_id, costelem_type "
		 "FROM costelem, itemcost "
		 "WHERE ((costelem_id=itemcost_costelem_id)"
		 "  AND  (itemcost_id=:itemcost_id));");
    q.prepare(sql);
    q.bindValue(":itemcost_id", _itemcostid);
    q.exec();
    _costelem->populate(q);
  }
}
