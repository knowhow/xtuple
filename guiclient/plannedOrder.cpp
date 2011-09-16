/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "plannedOrder.h"

#include <QMessageBox>
#include <QSqlError>
#include <QValidator>
#include <QVariant>

plannedOrder::plannedOrder(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_create, SIGNAL(clicked()), this, SLOT(sCreate()));
  connect(_warehouse, SIGNAL(newID(int)), this, SLOT(sHandleItemsite(int)));
  connect(_dueDate, SIGNAL(newDate(const QDate &)), this, SLOT(sUpdateStartDate()));
  connect(_leadTime, SIGNAL(valueChanged(int)), this, SLOT(sUpdateStartDate()));
  connect(_close, SIGNAL(clicked()), this, SLOT(sClose()));

  _item->setType(ItemLineEdit::cGeneralPurchased | ItemLineEdit::cGeneralManufactured |
                 ItemLineEdit::cActive);
  _qty->setValidator(omfgThis->qtyVal());

  //If not multi-warehouse hide whs control
  if (!_metrics->boolean("MultiWhs"))
  {
    _warehouseLit->hide();
    _warehouse->hide();
  }
}

plannedOrder::~plannedOrder()
{
    // no need to delete child widgets, Qt does it all for us
}

void plannedOrder::languageChange()
{
    retranslateUi(this);
}

enum SetResponse plannedOrder::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("itemsite_id", &valid);
  if (valid)
  {
    _captive = TRUE;

    _item->setItemsiteid(param.toInt());
    _item->setReadOnly(TRUE);
    _warehouse->setEnabled(FALSE);

    _qty->setFocus();
  }
  else
  {
    _captive = FALSE;

    _item->setFocus();
  }

  param = pParams.value("planord_id", &valid);
  if (valid)
  {
    _captive = TRUE;

    _planordid = param.toInt();
  }

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      _mode = cNew;

      populateFoNumber();
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;
      populate();
      _close->setFocus();
    }
    else if (param.toString() == "view")
    {
      _mode = cView;

      _number->setEnabled(FALSE);
      _item->setReadOnly(TRUE);
      _warehouse->setEnabled(FALSE);
      _typeGroup->setEnabled(FALSE);
      _qty->setEnabled(FALSE);
      _startDate->setEnabled(FALSE);
      _dueDate->setEnabled(FALSE);
      _leadTimeLit->hide();
      _leadTime->hide();
      _daysLit->hide();

      q.prepare( "SELECT fo_itemsite_id,"
                 "       formatFoNumber(fo_id) AS fonumber,"
                 "       fo_qtyord,"
                 "       fo_startdate, fo_duedate "
                 "FROM fo "
                 "WHERE (fo_id=:fo_id);" );
      q.bindValue(":fo_id", _planordid);
      q.exec();
      if (q.first())
      {
        _number->setText(q.value("fonumber").toString());
        _qty->setDouble(q.value("fo_qtyord").toDouble());
        _startDate->setDate(q.value("fo_startdate").toDate());
        _dueDate->setDate(q.value("fo_duedate").toDate());
        _item->setItemsiteid(q.value("fo_itemsite_id").toInt());
      }
    }
  }

  return NoError;
}

void plannedOrder::sClose()
{
  if (_mode == cNew)
  {
    q.prepare("SELECT releasePlanNumber(:orderNumber);");
    q.bindValue(":orderNumber", _number->text().toInt());
    q.exec();
  }

  reject();
}

void plannedOrder::sCreate()
{
  if (!(_item->isValid()))
  {
    QMessageBox::information( this, tr("No Item Number Selected"),
                              tr("You must enter or select a valid Item number before creating this Planned Order")  );
    return;
  }

  if (!_qty->text().length())
  {
    QMessageBox::information( this, tr("Invalid Quantity Ordered"),
                              tr( "You have entered an invalid Qty. Ordered.\n"
                                  "Please correct before creating this Planned Order"  ) );
    _qty->setFocus();
    return;
  }

  if (!_dueDate->isValid())
  {
    QMessageBox::information( this, tr("Invalid Due Date Entered"),
                              tr( "You have entered an invalid Due Date.\n"
                                  "Please correct before creating this Planned Order"  ) );
    _dueDate->setFocus();
    return;
  }

  q.prepare( "SELECT itemsite_id "
             "FROM itemsite "
             "WHERE ( (itemsite_item_id=:item_id)"
             " AND (itemsite_warehous_id=:warehous_id) );" );
  q.bindValue(":item_id", _item->id());
  q.bindValue(":warehous_id", _warehouse->id());
  q.exec();
  if (!q.first())
  {
    QMessageBox::information( this, tr("Invalid Item Site"),
                              tr("The Item and Site entered is and invalid Item Site combination.")  );
    return;
  }

  int itemsiteid = q.value("itemsite_id").toInt();
  int _supplyItemsiteId = -1;
  if (_toButton->isChecked())
  {
    q.prepare("SELECT itemsite_id "
              "FROM itemsite "
              "WHERE ( (itemsite_item_id=:item_id)"
              "  AND   (itemsite_warehous_id=:warehous_id) ); ");
    q.bindValue(":item_id", _item->id());
    q.bindValue(":warehous_id", _fromWarehouse->id());
    q.exec();
    if (q.first())
    {
      if (q.value("itemsite_id").toInt() == itemsiteid)
      { 
        QMessageBox::warning( this, tr("Cannot Save Planned Order"),
          tr("The Supplied From Site must be different from the Transfer To Site.") );
        return;
      }
      else
        _supplyItemsiteId = q.value("itemsite_id").toInt();
    }
    else
    { 
      QMessageBox::warning( this, tr("Cannot Save Planned"),
        tr("Cannot find Supplied From Item Site.") );
      return;
    }
  }

  int foid = 0;

  if(cEdit == _mode)
  {
    q.prepare( "UPDATE planord "
               "SET planord_number=:planord_number, "
               "    planord_type=:planord_type, "
               "    planord_itemsite_id=:planord_itemsite_id, "
               "    planord_supply_itemsite_id=:planord_supply_itemsite_id, "
               "    planord_comments=:planord_comments, "
               "    planord_qty=:planord_qty, "
               "    planord_duedate=:planord_dueDate, "
               "    planord_startdate=(DATE(:planord_dueDate) - :planord_leadTime) "
               "WHERE (planord_id=:planord_id);" );
    q.bindValue(":planord_number", _number->text().toInt());
    q.bindValue(":planord_itemsite_id", itemsiteid);
    if (_poButton->isChecked())
      q.bindValue(":planord_type", "P");
    else if (_woButton->isChecked())
      q.bindValue(":planord_type", "W");
    else if (_toButton->isChecked())
    {
      q.bindValue(":planord_type", "T");
      q.bindValue(":planord_supply_itemsite_id", _supplyItemsiteId);
    }
    q.bindValue(":planord_qty", _qty->toDouble());
    q.bindValue(":planord_dueDate", _dueDate->date());
    q.bindValue(":planord_leadTime", _leadTime->value());
    q.bindValue(":planord_comments", _notes->toPlainText());
    q.bindValue(":planord_id", _planordid);
    q.exec();
    if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }

    q.prepare( "SELECT explodePlannedOrder( :planord_id, true) AS result;" );
    q.bindValue(":planord_id", _planordid);
    q.exec();
    if (q.first())
    {
      double result = q.value("result").toDouble();
      if (result < 0.0)
      {
        systemError(this, tr("ExplodePlannedOrder returned %, indicating an "
                             "error occurred.").arg(result),
                    __FILE__, __LINE__);
        return;
      }
    }
    else if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }
  else
  {
    q.prepare( "SELECT createPlannedOrder( :orderNumber, :itemsite_id, :qty, "
               "                           (DATE(:dueDate) - :leadTime), :dueDate, "
               "                           :type, :supply_itemsite_id, :notes) AS result;" );
    q.bindValue(":orderNumber", _number->text().toInt());
    q.bindValue(":itemsite_id", itemsiteid);
    q.bindValue(":qty", _qty->toDouble());
    q.bindValue(":dueDate", _dueDate->date());
    q.bindValue(":leadTime", _leadTime->value());
    q.bindValue(":notes",    _notes->toPlainText());
    if (_poButton->isChecked())
      q.bindValue(":type", "P");
    else if (_woButton->isChecked())
      q.bindValue(":type", "W");
    else if (_toButton->isChecked())
    {
      q.bindValue(":type", "T");
      q.bindValue(":supply_itemsite_id", _supplyItemsiteId);
    }
  
    q.exec();
    if (!q.first())
    {
      systemError( this, tr("A System Error occurred at %1::%2.")
                         .arg(__FILE__)
                         .arg(__LINE__) );
      return;
    }

    foid = XDialog::Rejected;
    switch (q.value("result").toInt())
    {
      case -1:
        QMessageBox::critical( this, tr("Planned Order not Exploded"),
                               tr( "The Planned Order was created but not Exploded as there is no valid Bill of Materials for the selected Item.\n"
                                   "You must create a valid Bill of Materials before you may explode this Planned Order." ));
        break;
  
      case -2:
        QMessageBox::critical( this, tr("Planned Order not Exploded"),
                               tr( "The Planned Order was created but not Exploded as Component Items defined in the Bill of Materials\n"
                                   "for the selected Planned Order Item do not exist in the selected Planned Order Site.\n"
                                   "You must create Item Sites for these Component Items before you may explode this Planned Order." ));
        break;

      default:
        foid = q.value("result").toInt();
        break;
    }
  }

  if (_captive)
    done(foid);
  else
  {
    populateFoNumber();
    _item->setId(-1);
    _typeGroup->setEnabled(FALSE);
    _qty->clear();
    _dueDate->setNull();
    _leadTime->setValue(0);
    _startDate->setNull();
    _notes->clear();
    _close->setText(tr("&Close"));

    _item->setFocus();
  }
}

void plannedOrder::populate()
{
  XSqlQuery planord;
  planord.prepare( "SELECT planord.*, (planord_duedate - planord_startdate) AS leadtime,"
                   "       itemsite_warehous_id AS supplywarehousid "
                   "FROM planord LEFT OUTER JOIN itemsite ON (planord_supply_itemsite_id=itemsite_id) "
                   "WHERE (planord_id=:planord_id);" );
  planord.bindValue(":planord_id", _planordid);
  planord.exec();
  if (planord.first())
  {
    _number->setText(planord.value("planord_number").toString());
    _item->setItemsiteid(planord.value("planord_itemsite_id").toInt());
    _qty->setDouble(planord.value("planord_qty").toDouble());
    _dueDate->setDate(planord.value("planord_duedate").toDate());
    _startDate->setDate(planord.value("planord_startdate").toDate());
    _leadTime->setValue(planord.value("leadtime").toInt());
    _notes->setText(planord.value("planord_comments").toString());
    if (planord.value("planord_type").toString() == "P")
      _poButton->setChecked(TRUE);
    else if (planord.value("planord_type").toString() == "W")
      _woButton->setChecked(TRUE);
    else
    {
      _toButton->setChecked(TRUE);
      _fromWarehouse->setId(planord.value("supplywarehousid").toInt());
    }
  }
  else if (planord.lastError().type() != QSqlError::NoError)
  {
    systemError(this, planord.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void plannedOrder::sUpdateStartDate()
{
  if(_dueDate->isValid())
    _startDate->setDate(_dueDate->date().addDays(_leadTime->value() * -1));
}

void plannedOrder::sHandleItemsite(int pWarehousid)
{
  q.prepare( "SELECT itemsite_leadtime, itemsite_wosupply, itemsite_posupply, item_type "
             "FROM itemsite JOIN item ON (item_id=itemsite_item_id) "
             "WHERE ( (itemsite_item_id=:item_id)"
             " AND (itemsite_warehous_id=:warehous_id) );" );
  q.bindValue(":item_id", _item->id());
  q.bindValue(":warehous_id", pWarehousid);
  q.exec();
  if (!q.first())
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  _leadTime->setValue(q.value("itemsite_leadtime").toInt());
  
  if ( q.value("itemsite_posupply").toBool() &&
      (q.value("item_type").toString() == "P" ||
       q.value("item_type").toString() == "O" ||
       q.value("item_type").toString() == "T" ||
       q.value("item_type").toString() == "M") )
    _poButton->setEnabled(TRUE);
  else
    _poButton->setEnabled(FALSE);
  if ( q.value("itemsite_wosupply").toBool() &&
      (q.value("item_type").toString() == "P" ||
       q.value("item_type").toString() == "T" ||
       q.value("item_type").toString() == "M") )
    _woButton->setEnabled(TRUE);
  else
    _woButton->setEnabled(FALSE);
  if ( q.value("itemsite_wosupply").toBool() && q.value("itemsite_posupply").toBool() && q.value("item_type").toString() == "P" )
  {
    _poButton->setChecked(TRUE);
    _woButton->setChecked(FALSE);
  }
  else if ( q.value("itemsite_wosupply").toBool() )
  {
    _poButton->setChecked(FALSE);
    _woButton->setChecked(TRUE);
  }
  else
  {
    _poButton->setChecked(TRUE);
    _woButton->setChecked(FALSE);
  }

  q.prepare( "SELECT COALESCE(COUNT(*), 0) AS supplysites "
             "FROM itemsite "
             "WHERE ( (itemsite_item_id=:item_id)"
             " AND (itemsite_warehous_id <> :warehous_id) );" );
  q.bindValue(":item_id", _item->id());
  q.bindValue(":warehous_id", pWarehousid);
  q.exec();
  if (!q.first())
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  if (q.value("supplysites").toInt() > 0)
    _toButton->setEnabled(TRUE);
  else
    _toButton->setEnabled(FALSE);

  q.prepare( "SELECT COALESCE(supply.itemsite_id, -1) AS supplyitemsiteid,"
             "       COALESCE(supply.itemsite_warehous_id, -1) AS supplywarehousid "
             "FROM itemsite LEFT OUTER JOIN itemsite supply ON (supply.itemsite_id=itemsite.itemsite_supply_itemsite_id)"
             "WHERE ( (itemsite.itemsite_item_id=:item_id)"
             "  AND   (itemsite.itemsite_warehous_id=:warehous_id) );" );
  q.bindValue(":item_id", _item->id());
  q.bindValue(":warehous_id", pWarehousid);
  q.exec();
  if (!q.first())
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  if (q.value("supplyitemsiteid").toInt() != -1)
  {
    _toButton->setChecked(TRUE);
    _fromWarehouse->setId(q.value("supplywarehousid").toInt());
  }
  else
    _fromWarehouse->setId(pWarehousid);
}

void plannedOrder::populateFoNumber()
{
  q.exec("SELECT fetchPlanNumber() AS foNumber;");
  if (q.first())
    _number->setText(q.value("foNumber").toString());
  else
  {
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
    _number->setText("Error");
    return;
  }
}

