/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "lotserialcluster.h"

#include <QMessageBox>
#include <QSqlError>

#include "xsqlquery.h"

LotserialList::LotserialList(QWidget* pParent, Qt::WindowFlags flags)
  : VirtualList(pParent, flags)
{
  setObjectName("LotserialList");

  _listTab->headerItem()->setText(0, tr("Lot/Serial #"));
  _listTab->headerItem()->setText(1, tr("Item #"));
}

LotserialSearch::LotserialSearch(QWidget* pParent, Qt::WindowFlags flags)
  : VirtualSearch(pParent, flags)
{
  setObjectName("LotserialSearch");

  _listTab->headerItem()->setText(0, tr("Lot/Serial #"));
  _listTab->headerItem()->setText(1, tr("Item #"));
}

LotserialCluster::LotserialCluster(QWidget* pParent, const char* pName) :
    VirtualCluster(pParent, pName)
{
    LotserialLineEdit* lsle=new LotserialLineEdit(this, pName);
    addNumberWidget(lsle);
    
    connect(lsle, SIGNAL(newItemId(int)), this, SIGNAL(newItemId(int)));
}

LotserialLineEdit::LotserialLineEdit(QWidget* pParent, const char* pName) :
    VirtualClusterLineEdit(pParent, "ls", "ls_id", "ls_number", "item_number", "item_id", 0, pName)
{
    _itemid = -1;
    setTitles(tr("Lot/Serial Number"), tr("Lot/Serial Numbers"));
    _query = QString("SELECT ls_id AS id, "
                     "       ls_number AS number, "
                     "       item_id,"
                     "       item_number AS name, "
                     "       ls_notes AS description "
                     "FROM ls "
                     "  JOIN item ON (ls_item_id=item_id) "
                     "WHERE (true) ");
    _strict = true;
}

int LotserialCluster::itemId() const
{
  if (_number && _number->inherits("LotserialLineEdit"))
    return ((LotserialLineEdit*)(_number))->itemId();
  return true;
}

void LotserialCluster::setItemId(const int p)
{
  if (_number && _number->inherits("LotserialLineEdit"))
    ((LotserialLineEdit*)(_number))->setItemId(p);
}

bool LotserialCluster::strict() const
{
  if (_number && _number->inherits("LotserialLineEdit"))
    return ((LotserialLineEdit*)(_number))->strict();
  return true;
}

void LotserialCluster::setStrict(const bool p)
{
  if (_number && _number->inherits("LotserialLineEdit"))
    ((LotserialLineEdit*)(_number))->setStrict(p);
}

void LotserialLineEdit::setItemId(const int itemid)
{
  if (_itemid == itemid)
    return;
  else if (itemid <= 0)
  {
    _itemid = -1;
    _extraClause = "";
  }
  else
  {
    _itemid = itemid;
    _extraClause = QString(" (item_id=%1) ").arg(itemid);
  }

  if (_id != -1)
  {
    XSqlQuery qry;
    qry.prepare("SELECT ls_id "
                "FROM ls "
                "WHERE ((ls_id=:ls_id) "
                " AND (ls_item_id=:item_id)); ");
    qry.bindValue(":ls_id", _id);
    qry.bindValue(":item_id", _itemid);
    qry.exec();
    if (!qry.first())
      VirtualClusterLineEdit::clear();
  }
}

void LotserialLineEdit::clear()
{
  VirtualClusterLineEdit::clear();
}

void LotserialLineEdit::setId(const int lsid)
{
  if (_x_metrics)
  {
    if (_x_metrics->boolean("LotSerialControl"))
    {
      VirtualClusterLineEdit::setId(lsid);
      XSqlQuery qq;
      qq.prepare("SELECT ls_item_id"
                 "  FROM ls"
                 " WHERE(ls_id=:id);");
      qq.bindValue(":id", id());
      qq.exec();
      if(qq.first())
        emit newItemId(qq.value("ls_item_id").toInt());
    }
  }
}

/* copied from virtualCluster.cpp but with one important difference:
   if a not-strict flag is set then warn the user but don't clear the
   lotserial field
 */
void LotserialLineEdit::sParse()
{
  if (_x_metrics)
    if (_x_metrics->boolean("LotSerialControl"))
    {
      if (! _parsed)
      {
          QString stripped = text().trimmed().toUpper();
          if (stripped.length() == 0)
          {
              _parsed = true;
              setId(-1);
          }
          else
          {
              XSqlQuery numQ;
              numQ.prepare(_query + _numClause +
                           (_extraClause.isEmpty() ? "" : " AND " + _extraClause) +
                           QString("ORDER BY %1 LIMIT 1;").arg(_numColName));
              numQ.bindValue(":number", "^" + stripped);
              numQ.exec();
              if (numQ.first())
              {
                  _valid = true;
                  setId(numQ.value("id").toInt());
                  _name = (numQ.value("name").toString());
                  _itemid = (numQ.value("item_id").toInt());
              }
              else if (numQ.lastError().type() != QSqlError::NoError)
              {
                QMessageBox::critical(this, tr("A System Error Occurred at %1::%2.")
                                              .arg(__FILE__)
                                              .arg(__LINE__),
                                      numQ.lastError().databaseText());
                return;
              }
              else if (_strict || _itemid == -1)
                VirtualClusterLineEdit::clear();
              else if (isVisible() &&
                       QMessageBox::question(this, tr("Lot/Serial # Not Found"),
                        (_itemid > 0 ?
                          tr("This Lot/Serial # was not found for this item.") :
                          tr("This Lot/Serial # was not found.")) + 
                        tr(" Are you sure it is correct?"),
                     QMessageBox::Yes | QMessageBox::No,
                     QMessageBox::No) == QMessageBox::No)
              {
                VirtualClusterLineEdit::clear();
                return;
              }
              else
              {
                int lsid=-1;
                numQ.exec("SELECT nextval('ls_ls_id_seq') AS ls_id;");
                if (numQ.first())
                  lsid= numQ.value("ls_id").toInt();
                else if (numQ.lastError().type() != QSqlError::NoError)
                {
                  QMessageBox::critical(this, tr("A System Error Occurred at %1::%2.")
                                                .arg(__FILE__)
                                                .arg(__LINE__),
                                        numQ.lastError().databaseText());
                  return;
                }
                numQ.prepare("INSERT INTO ls (ls_id,ls_item_id,ls_number) "
                             "VALUES (:ls_id,:item_id,:number)");
                numQ.bindValue(":ls_id", lsid);
                numQ.bindValue(":item_id", _itemid);
                numQ.bindValue(":number", stripped);
                numQ.exec();
                if (numQ.lastError().type() != QSqlError::NoError)
                {
                  QMessageBox::critical(this, tr("A System Error Occurred at %1::%2.")
                                                .arg(__FILE__)
                                                .arg(__LINE__),
                                        numQ.lastError().databaseText());
                  return;
                }
                setId(lsid);
              }
          }
      }

      _parsed = TRUE;
      emit parsed();
    }
}

LotserialList* LotserialLineEdit::listFactory()
{
  return new LotserialList(this);
}

LotserialSearch* LotserialLineEdit::searchFactory()
{
  return new LotserialSearch(this);
}

/*
LotserialInfo* LotserialLineEdit::infoFactory()
{
  return new LotserialInfo(this, "LotserialInfo", true);
}
*/
