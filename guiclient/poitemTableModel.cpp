/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

// TODO: drag/drop support?

#include "poitemTableModel.h"

#include <QMessageBox>
#include <QSqlError>
#include <QSqlField>
#include <QSqlRecord>
#include <QString>

#include "guiclient.h"
#include "currcluster.h"

PoitemTableModel::PoitemTableModel(QObject * parent, QSqlDatabase db) :
  QSqlRelationalTableModel(parent, db)
{
  setTable("poitem");

  // select statement on which everything else is based
  _selectStatement =
      QString("SELECT pohead_number, "
	     "       item_id, item_number,"
	     "       warehous_id, warehous_code,"
	     "       poitem_unitprice * poitem_qty_ordered AS extprice,"
	     "       prj_number,"
	     "       expcat_code,"
	     "       0 AS itemsrc_minordqty, 0 AS itemsrc_multordqty, "
	     "       1 AS itemsrc_invvendoruomratio,"
	     "       CURRENT_DATE AS earliestdate, "
	     "       poitem.* "
	     "FROM pohead"
	     "     JOIN poitem ON (poitem_pohead_id=pohead_id)"
	     "     LEFT OUTER JOIN itemsite ON (poitem_itemsite_id=itemsite_id)"
	     "     LEFT OUTER JOIN item ON (itemsite_item_id=item_id)"
	     "     LEFT OUTER JOIN whsinfo ON (itemsite_warehous_id=warehous_id)"
	     "     LEFT OUTER JOIN prj ON (poitem_prj_id=prj_id)"
	     "     LEFT OUTER JOIN expcat ON (poitem_expcat_id=expcat_id)"
	     );

  setEditStrategy(QSqlTableModel::OnManualSubmit); // OnRow?
  setSort(POITEM_LINENUMBER_COL, Qt::AscendingOrder);

  // insert only those columns not directly part of the poitem table
  insertColumns(0, 7);

  setHeaderData(POITEM_LINENUMBER_COL,	Qt::Horizontal, tr("#"));
  setHeaderData(ITEM_NUMBER_COL,	Qt::Horizontal, tr("Item"));
  setHeaderData(WAREHOUS_CODE_COL,	Qt::Horizontal, tr("Supplying Site"));
  setHeaderData(POITEM_VEND_ITEM_NUMBER_COL, Qt::Horizontal, tr("Vend Item #"));
  setHeaderData(POITEM_VEND_ITEM_DESCRIP_COL,Qt::Horizontal, tr("Vend Description"));
  setHeaderData(POITEM_VEND_UOM_COL,	Qt::Horizontal, tr("Vend UOM"));
  setHeaderData(POITEM_QTY_ORDERED_COL,	Qt::Horizontal, tr("Qty."));
  setHeaderData(POITEM_UNITPRICE_COL,	Qt::Horizontal, tr("Unit Price"));
  setHeaderData(EXTPRICE_COL,		Qt::Horizontal, tr("Ext. Price"));
  setHeaderData(POITEM_FREIGHT_COL,	Qt::Horizontal, tr("Freight"));
  setHeaderData(POITEM_DUEDATE_COL,	Qt::Horizontal, tr("Due Date"));
  setHeaderData(PRJ_NUMBER_COL,		Qt::Horizontal, tr("Project #"));
  setHeaderData(EXPCAT_CODE_COL,	Qt::Horizontal, tr("Expense Cat."));

  _poheadid	= -1;
  _poitemid	= -1;
  findHeadData();
  _dirty = false;

  select();

  connect(this, SIGNAL(dataChanged(QModelIndex, QModelIndex)), this, SLOT(markDirty(QModelIndex, QModelIndex)));
}

void PoitemTableModel::findHeadData()
{
  _poheadcurrid = CurrDisplay::baseId();
  _poheaddate	= QDate();
  _postatus	= "U";	// safest default
  _vendid	= -1;
  _vendrestrictpurch = false;

  XSqlQuery poheadq;
  XSqlQuery vendq;
  if (_poheadid > 0)
  {
    poheadq.prepare("SELECT pohead_id, pohead_curr_id, pohead_orderdate,"
		    "       pohead_status "
		    "FROM pohead "
		    "WHERE (pohead_id=:pohead_id);");
    poheadq.bindValue(":pohead_id", _poheadid);
    vendq.prepare("SELECT vend_id, vend_restrictpurch "
		  "FROM vend, pohead "
		  "WHERE ((vend_id=pohead_vend_id)"
		  "  AND  (pohead_id=:id));");
    vendq.bindValue(":id", _poheadid);
  }
  else if (_poheadid > 0)
  {
    poheadq.prepare("SELECT pohead_id, pohead_curr_id, pohead_orderdate,"
		    "       pohead_status "
		    "FROM pohead "
		    "WHERE pohead_id IN (SELECT poitem_pohead_id "
		    "                    FROM poitem WHERE poitem_id=:poitem_id);");
    poheadq.bindValue(":poitem_id", _poitemid);
    vendq.prepare("SELECT vend_id, vend_restrictpurch "
		  "FROM vend, pohead, poitem "
		  "WHERE ((vend_id=pohead_vend_id)"
		  "  AND  (pohead_id=poitem_pohead_id)"
		  "  AND  (poitem_id=:id));");
    vendq.bindValue(":id", _poitemid);
  }
  else
    return;

  poheadq.exec();
  if (poheadq.first())
  {
    _poheadcurrid = poheadq.value("pohead_curr_id").toInt();
    _poheaddate   = poheadq.value("pohead_orderdate").toDate();
    _postatus	  = poheadq.value("pohead_status").toString();
  }
  else if (poheadq.lastError().type() != QSqlError::NoError)
  {
    systemError(0, poheadq.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  vendq.exec();
  if (vendq.first())
  {
    _vendid		= vendq.value("vend_id").toInt();
    _vendrestrictpurch	= vendq.value("vend_restrictpurch").toBool();
  }
  else if (vendq.lastError().type() != QSqlError::NoError)
  {
    systemError(0, vendq.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void PoitemTableModel::setHeadId(const int pId)
{
  setFilter(QString("poitem_pohead_id=%1").arg(pId));
  _poheadid = pId;
  _poitemid = -1;

  findHeadData();
}

void PoitemTableModel::setItemId(const int pId)
{
  setFilter(QString("poitem_id=%1").arg(pId));
  _poheadid = -1;
  _poitemid = pId;

  findHeadData();
}

void PoitemTableModel::setTransDate(const QDate pDate)
{
  _poheaddate = pDate;
}

void PoitemTableModel::setCurrId(const int pId)
{
  _poheadcurrid = pId;
}

QString PoitemTableModel::selectStatement() const
{
  QString strFilter = filter();
  if(strFilter.isEmpty())
    strFilter = "poitem_pohead_id=-1";
  return _selectStatement +
	  " WHERE " + strFilter +
	  " ORDER BY poitem_linenumber"
	  ;
}

bool PoitemTableModel::select()
{
  bool returnVal = QSqlRelationalTableModel::select();
  if (returnVal)
  {
    insertRow(rowCount());
    _dirty = false;
    connect(this, SIGNAL(dataChanged(QModelIndex, QModelIndex)), this, SLOT(markDirty(QModelIndex, QModelIndex)));
  }
  return returnVal;
}

bool PoitemTableModel::submitAll()
{
  XSqlQuery begin("BEGIN;");
  bool returnVal = QSqlRelationalTableModel::submitAll();
  if (returnVal)
  {
    _dirty = false;
    XSqlQuery commit("COMMIT;");
  }
  else
  {
    XSqlQuery rollback("ROLLBACK;");
    systemError(0, lastError().databaseText(), __FILE__, __LINE__);
  }

  select();

  return returnVal;
}

Qt::ItemFlags PoitemTableModel::flags(const QModelIndex& index) const
{
  Qt::ItemFlags flags = QSqlRelationalTableModel::flags(index);
  if (index.column() == POITEM_VEND_UOM_COL || index.column() == EXTPRICE_COL)
    flags &= ~(Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsEnabled);

  return flags;
}

bool PoitemTableModel::isDirty() const
{
  return _dirty;
}

bool PoitemTableModel::removeRow(int row, const QModelIndex & parent)
{
  return QSqlRelationalTableModel::removeRow(row, parent);
}

/*
    Make sure the row is internally consistent. If so then fix it so the
    parent class has a valid row to insert (some of the SELECTed columns
    shouldn't be directly modified in the db 'cause they're not part of the
    model's current table).
*/
bool PoitemTableModel::validRow(QSqlRecord& record)
{
  QString errormsg;
  QString warningmsg;

  // TODO: what is a better way to decide if this is an inventory item or not?
  bool inventoryItem = ! record.value("item_number").toString().isEmpty();

  if (! inventoryItem &&
      record.value("poitem_expcat_id").toInt() < 0)
    errormsg = tr("<p>You must specify an Expense Category for this "
		  "non-Inventory Item before you may save it.");

  else if (inventoryItem &&
	   record.value("item_number").toString().isEmpty())
    errormsg = tr("<p>You must select an Item Number before you may save.");

  else if (inventoryItem &&
	   record.value("warehous_id").toInt() <= 0)
    errormsg = tr("<p>You must select a Supplying Site before you may save.");

  else if (record.value("poitem_qty_ordered").toDouble() <= 0)
    errormsg = tr("<p>You must enter a quantity before you may save this "
		  "Purchase Order Item.");

  else if (record.value("itemsrc_minordqty").toDouble() > 0 &&
	   record.value("poitem_qty_ordered").toDouble() < record.value("itemsrc_minordqty").toDouble())
    warningmsg = tr("<p>The quantity that you are ordering is below the "
		    "Minimum Order Quantity for this Item Source.  You may "
		    "continue but this Vendor may not honor pricing or "
		    "delivery quotations. ");

  else if (record.value("itemsrc_multordqty").toDouble() > 0 &&
	   record.value("poitem_qty_ordered").toInt() % (int)(record.value("itemsrc_multordqty").toDouble()))
    warningmsg = tr("<p>The quantity that you are ordering does not fall "
		    "within the Order Multiple for this Item Source.  You may "
		    "continue but this Vendor may not honor pricing or "
		    "delivery quotations.");

  else if (! record.value("poitem_duedate").toDate().isValid())
    errormsg = tr("<p>You must enter a due date.");

  else if (record.value("earliestDate").toDate().isValid() &&
	   record.value("poitem_duedate").toDate() < record.value("earliestDate").toDate())
    warningmsg = tr("<p>The Due Date that you are requesting does not fall "
		    "within the Lead Time Days for this Item Source.  You may "
		    "continue but this Vendor may not honor pricing or "
		    "delivery quotations or may not be able to deliver by the "
		    "requested Due Date.");

  else if (record.value("poitem_pohead_id").toInt() <= 0 &&
	   _poheadid <= 0)
    errormsg = tr("<p>There is no Purchase Order header yet. "
	     "Try entering a Vendor if you are using the Purchase Order "
	     "window.");

  else if (inventoryItem &&
	   record.value("item_id").toInt() > 0 &&
	   record.value("warehous_id").toInt() > 0)
  {
    XSqlQuery isq;
    isq.prepare("SELECT itemsite_id, item_id "
		"FROM itemsite, item "
		"WHERE ((itemsite_item_id=item_id)"
		"  AND  (itemsite_warehous_id=:whs_id)"
		"  AND  (item_id=:item_id));");
    isq.bindValue(":whs_id", record.value("warehous_id").toInt());
    isq.bindValue(":item_id", record.value("item_id").toInt());
    isq.exec();
    if (isq.first())
    {
      int itemsiteid = isq.value("itemsite_id").toInt();
      if (itemsiteid != record.value("poitem_itemsite_id").toInt())
	record.setValue("poitem_itemsite_id", itemsiteid);
    }
    else if (isq.lastError().type() != QSqlError::NoError)
      errormsg = isq.lastError().databaseText();
    else
      errormsg = tr("<p>There is no Item Site for this Site (%1) and "
	       "Item Number (%2).")
	       .arg(record.value("warehous_code").toInt())
	       .arg(record.value("item_number").toString());
  }

  int index = record.indexOf("poitem_pohead_id");
  if (index < 0)
  {
    QSqlField field("poitem_pohead_id", QVariant::Int);
    field.setValue(_poheadid);
    record.append(field);
  }
  else
    record.setValue(index, _poheadid);

  XSqlQuery ln;
  /*
  ln.prepare("SELECT MAX(poitem_linenumber) + 1 AS newln "
	     "FROM poitem "
	     "WHERE (poitem_pohead_id=:pohead_id);");
             */
  // get the smallest available line number
  ln.prepare("SELECT MIN(sequence_value) AS newln "
             "FROM sequence "
             "WHERE sequence_value NOT IN (SELECT poitem_linenumber "
             "                             FROM poitem"
             "                             WHERE (poitem_pohead_id=:pohead_id));");
  ln.bindValue(":pohead_id", _poheadid);
  if (record.indexOf("poitem_linenumber") < 0)
  {
    ln.exec();
    if (ln.first())
    {
      QSqlField field("poitem_linenumber", QVariant::Int);
      field.setValue(ln.value("newln"));
      record.append(field);
    }
    else if (ln.lastError().type() != QSqlError::NoError)
    {
      errormsg = ln.lastError().databaseText();
    }
  }
  else if (record.value("poitem_linenumber").toInt() <= 0)
  {
    ln.exec();
    if (ln.first())
      record.setValue("poitem_linenumber", ln.value("newln"));
    else if (ln.lastError().type() != QSqlError::NoError)
    {
      errormsg = ln.lastError().databaseText();
    }
  }

  if (record.value("poitem_id").isNull())
  {
    XSqlQuery idq("SELECT NEXTVAL('poitem_poitem_id_seq') AS poitem_id;");
    if (idq.first())
      record.setValue("poitem_id", idq.value("poitem_id"));
    else
    {
      errormsg = idq.lastError().databaseText();
    }
  }

  if (_postatus.isEmpty())
    findHeadData();

  index = record.indexOf("poitem_status");
  if (index < 0)
  {
    QSqlField field("poitem_status", QVariant::String);
    field.setValue(_postatus);
    record.append(field);
  }
  else if (record.field(index).value().toString().isEmpty())
    record.setValue(index, _postatus);

  if (record.field("poitem_invvenduomratio").value().isNull() &&
      record.field("itemsrc_invvendoruomratio").value().isNull())
    record.setValue("poitem_invvenduomratio", 1);
  else if (record.field("poitem_invvenduomratio").value().isNull())
    record.setValue("poitem_invvenduomratio", record.value("itemsrc_invvendoruomratio"));

  if (! errormsg.isEmpty())
  {
    setLastError(QSqlError(QString("PoitemTableModel::validRow() error"),
			   errormsg, QSqlError::UnknownError));
    return false;
  }
  else if (! warningmsg.isEmpty())
  {
    if (QMessageBox::question(0, tr("Are you sure you want to continue?"),
		    warningmsg + tr("<p>Do you wish to Save this Order?"),
		    QMessageBox::Yes,
		    QMessageBox::No | QMessageBox::Default) == QMessageBox::No)
    return false;
  }

  record.remove(record.indexOf("earliestdate"));
  record.remove(record.indexOf("itemsrc_invvendoruomratio"));
  record.remove(record.indexOf("itemsrc_multordqty"));
  record.remove(record.indexOf("itemsrc_minordqty"));
  record.remove(record.indexOf("expcat_code"));
  record.remove(record.indexOf("prj_number"));
  record.remove(record.indexOf("extprice"));
  record.remove(record.indexOf("warehous_code"));
  record.remove(record.indexOf("warehous_id"));
  record.remove(record.indexOf("item_number"));
  record.remove(record.indexOf("item_id"));
  record.remove(record.indexOf("pohead_number"));

  return true;
}

bool PoitemTableModel::insertRowIntoTable(const QSqlRecord& record)
{
  if (record.isEmpty())
    return true;

  bool isNull = true;
  for (int i = 0; i < record.count(); i++)
  {
    if (i == record.indexOf("poitem_pohead_id") ||
	(record.value(i).toString().isEmpty() &&
	  (i == record.indexOf("poitem_status") ||
	   i == record.indexOf("poitem_vend_item_descrip") ||
	   i == record.indexOf("poitem_vend_uom") ||
	   i == record.indexOf("poitem_vend_item_number") ||
	   i == record.indexOf("poitem_comments") )))
      continue;
    isNull &= record.isNull(i);
  }
  if (isNull)
    return true;

  QSqlRecord newRecord(record);
  if (! validRow(newRecord))
    return false;

  return QSqlRelationalTableModel::insertRowIntoTable(newRecord);
}

bool PoitemTableModel::updateRowInTable(int row, const QSqlRecord& record)
{
  // touch everything so we can distinguish unchanged fields from NULL/0 as new val
  for (int i = 0; i < columnCount(); i++)
    setData(index(row, i), data(index(row, i)));

  QSqlRecord newRecord(record);
  if (! validRow(newRecord))
    return false;

  return QSqlRelationalTableModel::updateRowInTable(row, newRecord);
}

void PoitemTableModel::markDirty(QModelIndex, QModelIndex)
{
  _dirty = true;

  disconnect(this, SIGNAL(dataChanged(QModelIndex, QModelIndex)), this, SLOT(markDirty(QModelIndex, QModelIndex)));
}
