/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "itemPricingSchedule.h"

#include <QVariant>
#include <QMessageBox>
#include <QSqlError>
#include "itemPricingScheduleItem.h"

/*
 *  Constructs a itemPricingSchedule as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
itemPricingSchedule::itemPricingSchedule(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);


  // signals and slots connections
  connect(_close, SIGNAL(clicked()), this, SLOT(reject()));
  connect(_save, SIGNAL(clicked()), this, SLOT(sSave()));
  connect(_new, SIGNAL(clicked()), this, SLOT(sNew()));
  connect(_edit, SIGNAL(clicked()), this, SLOT(sEdit()));
  connect(_delete, SIGNAL(clicked()), this, SLOT(sDelete()));
  connect(_currency, SIGNAL(newID(int)), this, SLOT(sFillList()));

  _dates->setStartNull(tr("Always"), omfgThis->startOfTime(), TRUE);
  _dates->setStartCaption(tr("Effective"));
  _dates->setEndNull(tr("Never"), omfgThis->endOfTime(), TRUE);
  _dates->setEndCaption(tr("Expires"));

  _ipsitem->addColumn(tr("Type"),            _ynColumn,    Qt::AlignLeft,   true,  "type"  );
  _ipsitem->addColumn(tr("Item/Prod. Cat."), _itemColumn,  Qt::AlignLeft,   true,  "number"  );
  _ipsitem->addColumn(tr("Description"),     -1,           Qt::AlignLeft,   true,  "descrip"  );
  _ipsitem->addColumn(tr("UOM"),             _uomColumn,   Qt::AlignCenter, true,  "qtyuom");
  _ipsitem->addColumn(tr("Qty. Break"),      _qtyColumn,   Qt::AlignRight,  true,  "qtybreak" );
  _ipsitem->addColumn(tr("UOM"),             _uomColumn,   Qt::AlignCenter, true,  "priceuom");
  _ipsitem->addColumn(tr("Price/Discount"),  _priceColumn, Qt::AlignRight,  true,  "price" );
  _ipsitem->addColumn(tr("Fixed Discount"),  _priceColumn, Qt::AlignRight,  true,  "fixedAmtDiscount" );

  _currency->setType(XComboBox::Currencies);
  _currency->setLabel(_currencyLit);
  _updated = QDate::currentDate();
  
  q.exec("BEGIN;");
  _rejectedMsg = tr("The application has encountered an error and must "
                    "stop editing this Pricing Schedule.\n%1");
}

/*
 *  Destroys the object and frees any allocated resources
 */
itemPricingSchedule::~itemPricingSchedule()
{
  // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void itemPricingSchedule::languageChange()
{
  retranslateUi(this);
}

enum SetResponse itemPricingSchedule::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      _mode = cNew;

      _name->setFocus();
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;

      _save->setFocus();
    }
    else if (param.toString() == "copy")
    {
      _mode = cCopy;

      _name->setFocus();
    }
    else if (param.toString() == "view")
    {
      _mode = cView;

      _name->setEnabled(FALSE);
      _descrip->setEnabled(FALSE);
      _dates->setEnabled(FALSE);
      _currency->setEnabled(FALSE);
      _new->setEnabled(FALSE);
      _close->setText(tr("&Close"));
      _save->hide();

      _close->setFocus();
    }
  }

  param = pParams.value("ipshead_id", &valid);
  if (valid)
  {
    _ipsheadid = param.toInt();
    populate();
  }

  if ( (_mode == cNew) || (_mode == cEdit) || (_mode == cCopy) )
  {
    connect(_ipsitem, SIGNAL(valid(bool)), _edit, SLOT(setEnabled(bool)));
    connect(_ipsitem, SIGNAL(valid(bool)), _delete, SLOT(setEnabled(bool)));
    connect(_ipsitem, SIGNAL(itemSelected(int)), _edit, SLOT(animateClick()));
  }

  if ( (_mode == cNew) )
  {
    q.exec("SELECT NEXTVAL('ipshead_ipshead_id_seq') AS ipshead_id;");
    if (q.first())
      _ipsheadid = q.value("ipshead_id").toInt();
    else if (q.lastError().type() != QSqlError::NoError)
    {
	systemError(this, _rejectedMsg.arg(q.lastError().databaseText()),
                  __FILE__, __LINE__);
        reject();
        return UndefinedError;
    }
  }

  return NoError;
}

bool itemPricingSchedule::sSave(bool p)
{
  if (_name->text().trimmed().isEmpty())
  {
    QMessageBox::critical( this, tr("Enter Name"),
                           tr("You must enter a Name for this Pricing Schedule.") );
    _name->setFocus();
    return false;
  }

  if (!_dates->startDate().isValid())
  {
    QMessageBox::critical( this, tr("Enter Effective Date"),
                           tr("You must enter an effective date for this Pricing Schedule.") );
    _dates->setFocus();
    return false;
  }

  if (!_dates->endDate().isValid())
  {
    QMessageBox::critical( this, tr("Enter Expiration Date"),
                           tr("You must enter an expiration date for this Pricing Schedule.") );
    _dates->setFocus();
    return false;
  }

  if (_dates->endDate() < _dates->startDate())
  {
    QMessageBox::critical( this, tr("Invalid Expiration Date"),
                           tr("The expiration date cannot be earlier than the effective date.") );
    _dates->setFocus();
    return false;
  }

  q.prepare("SELECT ipshead_id"
            "  FROM ipshead"
            " WHERE ((ipshead_name=:ipshead_name)"
            "   AND  (ipshead_id != :ipshead_id))");
  q.bindValue(":ipshead_id", _ipsheadid);
  q.bindValue(":ipshead_name", _name->text());
  q.exec();
  if(q.first())
  {
    QMessageBox::warning(this, tr("Pricing Schedule Already Exists"),
                      tr("A Pricing Schedule with the entered Name already exists."));
    return false;
  }

  if (_mode == cNew) 
    q.prepare( "INSERT INTO ipshead "
               "( ipshead_id, ipshead_name, ipshead_descrip,"
               "  ipshead_effective, ipshead_expires, "
	       "  ipshead_curr_id, ipshead_updated ) "
               "VALUES "
               "( :ipshead_id, :ipshead_name, :ipshead_descrip,"
               "  :ipshead_effective, :ipshead_expires, "
	       "  :ipshead_curr_id, CURRENT_DATE );" );
  else if ( (_mode == cEdit) || (_mode == cCopy) )
    q.prepare( "UPDATE ipshead "
               "SET ipshead_name=:ipshead_name, ipshead_descrip=:ipshead_descrip,"
               "    ipshead_effective=:ipshead_effective, ipshead_expires=:ipshead_expires, "
	       "    ipshead_curr_id=:ipshead_curr_id, "
	       "    ipshead_updated=CURRENT_DATE "
               "WHERE (ipshead_id=:ipshead_id);" );

  q.bindValue(":ipshead_id", _ipsheadid);
  q.bindValue(":ipshead_name", _name->text());
  q.bindValue(":ipshead_descrip", _descrip->text());
  q.bindValue(":ipshead_effective", _dates->startDate());
  q.bindValue(":ipshead_expires", _dates->endDate());
  q.bindValue(":ipshead_curr_id", _currency->id());
  q.exec();
  if (q.lastError().type() != QSqlError::NoError)
  {
	systemError(this, _rejectedMsg.arg(q.lastError().databaseText()),
                  __FILE__, __LINE__);
        reject();
  }

  _mode = cEdit;
  
  if (p)
  {
    q.exec("COMMIT;");
    done(_ipsheadid);
  }
  return true;
}

void itemPricingSchedule::sSave()
{
  sSave(true) ;  
}

void itemPricingSchedule::sCheck()
{
}

void itemPricingSchedule::sNew()
{
  if(!sSave(false))
    return;
  ParameterList params;
  params.append("mode", "new");
  params.append("ipshead_id", _ipsheadid);
  params.append("curr_id", _currency->id());
  params.append("updated", _updated);

  itemPricingScheduleItem newdlg(this, "", TRUE);
  newdlg.set(params);

  int result;
  if ((result = newdlg.exec()) != XDialog::Rejected)
  {
    if (result == -1)
      reject();
    else
      sFillList(result);
  }
}

void itemPricingSchedule::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("curr_id", _currency->id());
  params.append("updated", _updated);

  if(_ipsitem->altId() == 1)
    params.append("ipsitem_id", _ipsitem->id());
  else if(_ipsitem->altId() == 2)
    params.append("ipsprodcat_id", _ipsitem->id());
  else if(_ipsitem->altId() == 3)
    params.append("ipsfreight_id", _ipsitem->id());
  else
    return;
    // ToDo - tell the user why we're not showing the pricing sched?

  itemPricingScheduleItem newdlg(this, "", TRUE);
  newdlg.set(params);

  int result;
  if ((result = newdlg.exec()) != XDialog::Rejected)
  {
    if (result == -1)
      reject();
    else
      sFillList(result);
  }
}

void itemPricingSchedule::sDelete()
{
  if(_ipsitem->altId() == 1)
    q.prepare( "DELETE FROM ipsitem "
               "WHERE (ipsitem_id=:ipsitem_id);" );
  else if(_ipsitem->altId() == 2)
    q.prepare( "DELETE FROM ipsprodcat "
               "WHERE (ipsprodcat_id=:ipsitem_id);" );
  else if(_ipsitem->altId() == 3)
    q.prepare( "DELETE FROM ipsfreight "
               "WHERE (ipsfreight_id=:ipsitem_id);" );
  else
    return;
  q.bindValue(":ipsitem_id", _ipsitem->id());
  q.exec();
  if (q.lastError().type() != QSqlError::NoError)
  {
	systemError(this, _rejectedMsg.arg(q.lastError().databaseText()),
                  __FILE__, __LINE__);
        reject();
  }

  sFillList();
}

void itemPricingSchedule::sFillList()
{
  sFillList(-1);
}

void itemPricingSchedule::sFillList(int pIpsitemid)
{
  q.prepare( "SELECT ipsitem_id AS id, 1 AS altid, :item AS type, item_number AS number,"
             "       (item_descrip1 || ' ' || item_descrip2) AS descrip,"
             "       qty.uom_name AS qtyuom, ipsitem_qtybreak AS qtybreak,"
             "       price.uom_name AS priceuom, ipsitem_price AS price,"
			 "       0.00 AS fixedAmtDiscount,"
             "       'qty' AS qtybreak_xtnumericrole,"
             "       'salesprice' AS price_xtnumericrole "
             "  FROM ipsitem, item, uom AS qty, uom AS price "
             " WHERE ( (ipsitem_item_id=item_id)"
             "   AND   (ipsitem_qty_uom_id=qty.uom_id)"
             "   AND   (ipsitem_price_uom_id=price.uom_id)"
             "   AND   (ipsitem_ipshead_id=:ipshead_id) )"
             " UNION "
             "SELECT ipsprodcat_id AS id, 2 AS altid, :prodcat AS type, prodcat_code AS number,"
             "       prodcat_descrip AS descrip,"
             "       '' AS qtyuom, ipsprodcat_qtybreak AS qtybreak,"
             "       '' AS priceuom, ipsprodcat_discntprcnt AS price,"
			 "       ipsprodcat_fixedamtdiscount AS fixedAmtDiscount,"
             "       'qty' AS qtybreak_xtnumericrole,"
             "       'percent' AS price_xtnumericrole "
             "  FROM ipsprodcat, prodcat"
             " WHERE ( (ipsprodcat_prodcat_id=prodcat_id)"
             "   AND   (ipsprodcat_ipshead_id=:ipshead_id) )"
             " UNION "
             "SELECT ipsfreight_id AS id, 3 AS altid, :freight AS type,"
             "       CASE WHEN (ipsfreight_type='F') THEN :flatrate"
             "            ELSE :peruom"
             "       END AS number,"
             "       ('From ' || COALESCE(warehous_code, 'All Sites') || ' To ' || COALESCE(shipzone_name, 'All Shipping Zones')) AS descrip,"
             "       CASE WHEN (ipsfreight_type='P') THEN uom_name END AS qtyuom,"
             "       CASE WHEN (ipsfreight_type='P') THEN ipsfreight_qtybreak END AS qtybreak,"
             "       uom_name AS priceuom, ipsfreight_price AS price,"
			 "       0.00 AS fixedAmtDiscount,"
             "       'qty' AS qtybreak_xtnumericrole,"
             "       'curr' AS price_xtnumericrole "
             "  FROM ipsfreight LEFT OUTER JOIN uom ON (uom_item_weight)"
             "                  LEFT OUTER JOIN whsinfo ON (warehous_id=ipsfreight_warehous_id)"
             "                  LEFT OUTER JOIN shipzone ON (shipzone_id=ipsfreight_shipzone_id)"
             " WHERE ( (ipsfreight_ipshead_id=:ipshead_id) )"
             " ORDER BY altid, number, qtybreak;" );
  q.bindValue(":ipshead_id", _ipsheadid);
  q.bindValue(":item", tr("Item"));
  q.bindValue(":prodcat", tr("Prod. Cat."));
  q.bindValue(":freight", tr("Freight"));
  q.bindValue(":flatrate", tr("Flat Rate"));
  q.bindValue(":peruom", tr("Price Per UOM"));
  q.exec();
  if (q.lastError().type() != QSqlError::NoError)
  {
	systemError(this, _rejectedMsg.arg(q.lastError().databaseText()),
                  __FILE__, __LINE__);
        reject();
  }

  if (pIpsitemid == -1)
    _ipsitem->populate(q, true);
  else
    _ipsitem->populate(q, pIpsitemid, true);

  _currency->setEnabled(_ipsitem->topLevelItemCount() <= 0);
}

void itemPricingSchedule::populate()
{
  XSqlQuery pop;
  pop.prepare( "SELECT ipshead_name, ipshead_descrip,"
             "       ipshead_effective, ipshead_expires, "
	     "       ipshead_curr_id, ipshead_updated "
             "FROM ipshead "
             "WHERE (ipshead_id=:ipshead_id);" );
  pop.bindValue(":ipshead_id", _ipsheadid);
  pop.exec();
  if (pop.first())
  {
    _name->setText(pop.value("ipshead_name").toString());
    _descrip->setText(pop.value("ipshead_descrip").toString());
    _dates->setStartDate(pop.value("ipshead_effective").toDate());
    _dates->setEndDate(pop.value("ipshead_expires").toDate());
    _currency->setId(pop.value("ipshead_curr_id").toInt());
    _currency->setEnabled(FALSE);
    QDate tmpDate = pop.value("ipshead_updated").toDate();
    if (tmpDate.isValid() && ! tmpDate.isNull())
	_updated = tmpDate;

    sFillList(-1);
  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
	systemError(this, _rejectedMsg.arg(q.lastError().databaseText()),
                  __FILE__, __LINE__);
		  reject();
  }
}

void itemPricingSchedule::reject()
{
  q.exec("ROLLBACK;");
  if(_mode == cCopy) 
  {
    q.prepare( "DELETE FROM ipsitem "
               "WHERE (ipsitem_ipshead_id=:ipshead_id); "
               "DELETE FROM ipsprodcat "
               "WHERE (ipsprodcat_ipshead_id=:ipshead_id); "
               "DELETE FROM ipsfreight "
               "WHERE (ipsfreight_ipshead_id=:ipshead_id); "
               "DELETE FROM ipshead "
               "WHERE (ipshead_id=:ipshead_id);" );
    q.bindValue(":ipshead_id", _ipsheadid);
    q.exec();
  }
  
  XDialog::reject();
}
