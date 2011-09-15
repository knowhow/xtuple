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
// TODO: make a base class for the similarities of this and poitemTableModel

#include "toitemTableModel.h"

#include <QMessageBox>
#include <QSqlError>
#include <QSqlField>
#include <QSqlRecord>
#include <QString>

#include "guiclient.h"
#include "currcluster.h"

ToitemTableModel::ToitemTableModel(QObject * parent, QSqlDatabase db) :
  QSqlRelationalTableModel(parent, db)
{
  setTable("toitem");

  // select statement on which everything else is based
  _selectStatement =
      QString("SELECT tohead_number, "
	     "       item_number,"
	     "       prj_number,"
	     "       CURRENT_DATE AS earliestdate, "
	     "       toitem.* "
	     "FROM tohead"
	     "     JOIN toitem ON (toitem_tohead_id=tohead_id)"
	     "     LEFT OUTER JOIN item ON (toitem_item_id=item_id)"
	     "     LEFT OUTER JOIN prj ON (toitem_prj_id=prj_id)"
	     );

  setEditStrategy(QSqlTableModel::OnManualSubmit); // OnRow?
  setSort(TOITEM_LINENUMBER_COL, Qt::AscendingOrder);

  // insert only those columns not directly part of the toitem table
  insertColumns(0, 4);

  setHeaderData(TOITEM_LINENUMBER_COL,	Qt::Horizontal, tr("#"));
  setHeaderData(ITEM_NUMBER_COL,	Qt::Horizontal, tr("Item"));
  setHeaderData(TOITEM_QTY_ORDERED_COL,	Qt::Horizontal, tr("Qty."));
  setHeaderData(TOITEM_STDCOST_COL,	Qt::Horizontal, tr("Std. Cost"));
  setHeaderData(TOITEM_FREIGHT_COL,	Qt::Horizontal, tr("Freight"));
  setHeaderData(TOITEM_DUEDATE_COL,	Qt::Horizontal, tr("Due Date"));
  setHeaderData(PRJ_NUMBER_COL,		Qt::Horizontal, tr("Project #"));

  _toheadid	= -1;
  _toitemid	= -1;
  findHeadData();
  _dirty = false;

  select();

  connect(this, SIGNAL(dataChanged(QModelIndex, QModelIndex)), this, SLOT(markDirty(QModelIndex, QModelIndex)));
}

void ToitemTableModel::findHeadData()
{
  _toheadcurrid = CurrDisplay::baseId();
  _toheaddate	= QDate();
  _tostatus	= "O";	// safest default
  _toheadsrcwhsid = -1;

  XSqlQuery toheadq;
  if (_toheadid > 0)
  {
    toheadq.prepare("SELECT * "
		    "FROM tohead "
		    "WHERE (tohead_id=:tohead_id);");
    toheadq.bindValue(":tohead_id", _toheadid);
  }
  else if (_toitemid > 0)
  {
    toheadq.prepare("SELECT * "
		    "FROM tohead "
		    "WHERE tohead_id IN (SELECT toitem_tohead_id "
		    "                    FROM toitem WHERE toitem_id=:toitem_id);");
    toheadq.bindValue(":toitem_id", _toitemid);
  }
  else
    return;

  toheadq.exec();
  if (toheadq.first())
  {
    _toheadcurrid = toheadq.value("tohead_freight_curr_id").toInt();
    _toheaddate   = toheadq.value("tohead_orderdate").toDate();
    _tostatus	  = toheadq.value("tohead_status").toString();
    _toheadsrcwhsid=toheadq.value("tohead_src_warehous_id").toInt();
  }
  else if (toheadq.lastError().type() != QSqlError::NoError)
  {
    systemError(0, toheadq.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void ToitemTableModel::setHeadId(const int pId)
{
  setFilter(QString("toitem_tohead_id=%1").arg(pId));
  _toheadid = pId;
  _toitemid = -1;

  findHeadData();
}

void ToitemTableModel::setItemId(const int pId)
{
  setFilter(QString("toitem_id=%1").arg(pId));
  _toheadid = -1;
  _toitemid = pId;

  findHeadData();
}

void ToitemTableModel::setShipDate(const QDate pDate)
{
  _toshipdate = pDate;
}

void ToitemTableModel::setSrcWhsId(const int pId)
{
  _toheadsrcwhsid = pId;
}

void ToitemTableModel::setTransDate(const QDate pDate)
{
  _toheaddate = pDate;
}

void ToitemTableModel::setCurrId(const int pId)
{
  _toheadcurrid = pId;
}

QString ToitemTableModel::selectStatement() const
{
  return _selectStatement + " WHERE " +
	  QString((filter().isEmpty()) ? " (toitem_tohead_id=-1) " : filter()) +
	  " ORDER BY toitem_linenumber"
	  ;
}

bool ToitemTableModel::select()
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

bool ToitemTableModel::submitAll()
{
  XSqlQuery begin("BEGIN;");
  bool returnVal = QSqlRelationalTableModel::submitAll();
  if (returnVal)
  {
    _dirty = false;
    XSqlQuery commit("COMMIT;");
  }
  else
    XSqlQuery rollback("ROLLBACK;");

  return returnVal;
}

Qt::ItemFlags ToitemTableModel::flags(const QModelIndex& index) const
{
  Qt::ItemFlags flags = QSqlRelationalTableModel::flags(index);
  if (index.column() == TOITEM_STDCOST_COL)
    flags &= ~(Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsEnabled);

  return flags;
}

bool ToitemTableModel::isDirty() const
{
  return _dirty;
}

bool ToitemTableModel::removeRow(int row, const QModelIndex & parent)
{
  return QSqlRelationalTableModel::removeRow(row, parent);
}

/*
    Make sure the row is internally consistent. If so then fix it so the
    parent class has a valid row to insert (some of the SELECTed columns
    shouldn't be directly modified in the db 'cause they're not part of the
    model's current table).
*/
bool ToitemTableModel::validRow(QSqlRecord& record)
{
  QString errormsg;
  QString warningmsg;

  if (record.value("item_number").toString().isEmpty())
    errormsg = tr("<p>You must select an Item Number before you may save.");

  else if (record.value("toitem_qty_ordered").toDouble() <= 0)
    errormsg = tr("<p>You must enter a quantity before you may save this "
		  "Purchase Order Item.");

  else if (! record.value("toitem_duedate").toDate().isValid())
    errormsg = tr("<p>You must enter a due date.");

  else if (record.value("toitem_tohead_id").toInt() <= 0 &&
	   _toheadid <= 0)
    errormsg = tr("<p>There is no Transfer Order header yet. "
	     "Try entering ????.");

  int index = record.indexOf("toitem_tohead_id");
  if (index < 0)
  {
    QSqlField field("toitem_tohead_id", QVariant::Int);
    field.setValue(_toheadid);
    record.append(field);
  }
  else
    record.setValue(index, _toheadid);

  XSqlQuery ln;
  ln.prepare("SELECT COUNT(*) + 1 AS newln "
	     "FROM toitem "
	     "WHERE (toitem_tohead_id=:tohead_id);");
  ln.bindValue(":tohead_id", _toheadid);
  if (record.indexOf("toitem_linenumber") < 0)
  {
    ln.exec();
    if (ln.first())
    {
      QSqlField field("toitem_linenumber", QVariant::Int);
      field.setValue(ln.value("newln"));
      record.append(field);
    }
    else if (ln.lastError().type() != QSqlError::NoError)
    {
      errormsg = ln.lastError().databaseText();
    }
  }
  else if (record.value("toitem_linenumber").toInt() <= 0)
  {
    ln.exec();
    if (ln.first())
      record.setValue("toitem_linenumber", ln.value("newln"));
    else if (ln.lastError().type() != QSqlError::NoError)
    {
      errormsg = ln.lastError().databaseText();
    }
  }

  if (record.value("toitem_id").isNull())
  {
    XSqlQuery idq("SELECT NEXTVAL('toitem_toitem_id_seq') AS toitem_id;");
    if (idq.first())
      record.setValue("toitem_id", idq.value("toitem_id"));
    else if (idq.lastError().type() != QSqlError::NoError)
      errormsg = idq.lastError().databaseText();
  }

  if (_tostatus.isEmpty())
    findHeadData();

  index = record.indexOf("toitem_freight_curr_id");
  if (index < 0)
  {
    QSqlField field("toitem_freight_curr_id", QVariant::String);
    field.setValue(_toheadcurrid);
    record.append(field);
  }
  else if (record.field(index).value().toString().isEmpty())
    record.setValue(index, _toheadcurrid);

  index = record.indexOf("toitem_status");
  if (index < 0)
  {
    QSqlField field("toitem_status", QVariant::String);
    field.setValue(_tostatus);
    record.append(field);
  }
  else if (record.field(index).value().toString().isEmpty())
    record.setValue(index, _tostatus);

  index = record.indexOf("toitem_schedshipdate");
  if (index < 0)
  {
    QSqlField field("toitem_schedshipdate", QVariant::Date);
    field.setValue(_toshipdate);
    record.append(field);
  }
  else if (record.field(index).value().isNull())
    record.setValue(index, _toshipdate);

  if (! errormsg.isEmpty())
  {
    setLastError(QSqlError(QString("ToitemTableModel::validRow() error"),
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

  record.remove(record.indexOf("prj_number"));
  record.remove(record.indexOf("item_number"));
  record.remove(record.indexOf("tohead_number"));
  record.remove(record.indexOf("earliestdate"));

  return true;
}

bool ToitemTableModel::insertRowIntoTable(const QSqlRecord& record)
{
  if (record.isEmpty())
    return true;

  bool isNull = true;
  for (int i = 0; i < record.count(); i++)
  {
    if (i == record.indexOf("toitem_tohead_id") ||
	(record.value(i).toString().isEmpty() &&
	  (i == record.indexOf("toitem_status") ||
	   i == record.indexOf("toitem_comments") )))
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

bool ToitemTableModel::updateRowInTable(int row, const QSqlRecord& record)
{
  // touch everything so we can distinguish unchanged fields from NULL/0 as new val
  for (int i = 0; i < columnCount(); i++)
    setData(index(row, i), data(index(row, i)));

  QSqlRecord newRecord(record);
  if (! validRow(newRecord))
    return false;

  return QSqlRelationalTableModel::updateRowInTable(row, newRecord);
}

void ToitemTableModel::markDirty(QModelIndex, QModelIndex)
{
  _dirty = true;

  disconnect(this, SIGNAL(dataChanged(QModelIndex, QModelIndex)), this, SLOT(markDirty(QModelIndex, QModelIndex)));
}
