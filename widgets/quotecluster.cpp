/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include <QMessageBox>
#include <QSqlError>

#include "quotecluster.h"

#include <metasql.h>

QuoteList::QuoteList(QWidget *pParent, Qt::WindowFlags flags)
  : VirtualList(pParent, flags)
{
  setObjectName("QuoteList");

  _listTab->headerItem()->setText(0, tr("Quote Number"));
  _listTab->headerItem()->setText(1, tr("Customer/Prospect"));
}

QuoteSearch::QuoteSearch(QWidget *pParent, Qt::WindowFlags flags)
  : VirtualSearch(pParent, flags)
{
  setObjectName("QuoteSearch");

  _listTab->headerItem()->setText(0, tr("Quote Number"));
  _listTab->headerItem()->setText(1, tr("Customer/Prospect"));

}

QuoteCluster::QuoteCluster(QWidget* pParent, const char* pName) :
    VirtualCluster(pParent, pName)
{
    addNumberWidget(new QuoteLineEdit(this, pName));
}

bool QuoteCluster::forCustomer()
{
  return ((QuoteLineEdit*)_number)->forCustomer();
}

bool QuoteCluster::forProspect()
{
  return ((QuoteLineEdit*)_number)->forProspect();
}

int  QuoteCluster::recipId()
{
  return ((QuoteLineEdit*)_number)->recipId();
}

void QuoteLineEdit::sSearch()
{
  QuoteSearch *newdlg = new QuoteSearch(this);
  if (newdlg)
  {
    int id = newdlg->exec();
    if (id != QDialog::Rejected)
      setId(id);
  }
  else
    QMessageBox::critical(this, tr("A System Error Occurred  at %1::%2.")
                                   .arg(__FILE__).arg(__LINE__),
                          tr("Could not instantiate a Quote Search Dialog"));
}

QuoteLineEdit::QuoteLineEdit(QWidget* pParent, const char* pName) :
    VirtualClusterLineEdit(pParent, "quhead", "quhead_id", "quhead_number",
                           "name", "quhead_billtoname", 0, pName)
{
    setTitles(tr("Quote"), tr("Quotes"));
    _query = QString("SELECT quhead_id AS id, quhead_number AS number,"
                     "       COALESCE(cust_number, prospect_number, '?') AS name,"
                     "       quhead_billtoname AS description,"
                     "       quhead_cust_id AS recip_id,"
                     "       CASE WHEN cust_number IS NOT NULL THEN 'C'"
                     "            WHEN prospect_number IS NOT NULL THEN 'P'"
                     "       END AS recip_type "
                     "FROM quhead"
                     "     LEFT OUTER JOIN custinfo ON (quhead_cust_id=cust_id)"
                     "     LEFT OUTER JOIN prospect ON (quhead_cust_id=prospect_id) "
                     "WHERE TRUE ");
    _recip_id = 0;
    _recip_type = QString::null;
}

void QuoteLineEdit::clear()
{
  VirtualClusterLineEdit::clear();
  _id = -1;
  _recip_id = 0;
  _recip_type = QString::null;
}

void QuoteLineEdit::silentSetId(const int pId)
{
  if (pId == -1 || pId == 0)
    clear();
  else
  {
    XSqlQuery idq;
    idq.prepare(_query + _idClause +
                (_extraClause.isEmpty() || !_strict ? "" : " AND " + _extraClause) +
                QString(";"));
    idq.bindValue(":id", pId);
    idq.exec();
    if (idq.first())
    {
      _id = pId;
      _valid = true;
      setText(idq.value("number").toString());
      _name = idq.value("name").toString();
      _description = idq.value("description").toString();
      _recip_id = idq.value("recip_id").toInt();
      _recip_type = idq.value("recip_type").toString();
    }
    else if (idq.lastError().type() != QSqlError::NoError)
      QMessageBox::critical(this, tr("A System Error Occurred at %1::%2.")
                                      .arg(__FILE__).arg(__LINE__),
                            idq.lastError().databaseText());
  }

  _parsed = true;
  emit parsed();
}

// TODO: can we get _recip_id and _recip_type using the inherited sParse()?
void QuoteLineEdit::sParse()
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
                  (_extraClause.isEmpty() || !_strict ? "" : " AND " + _extraClause) +
                  QString(";"));
      numQ.bindValue(":number", "^" + stripped);
      numQ.exec();
      if (numQ.first())
      {
        _valid       = true;
        setId(numQ.value("id").toInt());
        if (_hasName)
          _name      = numQ.value("name").toString();
        if (_hasDescription)
          _description = numQ.value("description").toString();
        _recip_id    = numQ.value("recip_id").toInt();
        _recip_type  = numQ.value("recip_type").toString();
      }
      else
      {
        setId(-1);
        if (numQ.lastError().type() != QSqlError::NoError)
            QMessageBox::critical(this, tr("A System Error Occurred at %1::%2.")
                                          .arg(__FILE__)
                                          .arg(__LINE__),
                    numQ.lastError().databaseText());
      }
    }
    emit valid(_valid);
    emit parsed();
  }

  _parsed = true;
  sHandleNullStr();
}

bool QuoteLineEdit::forCustomer()
{
  return (_recip_type == "C");
}

bool QuoteLineEdit::forProspect()
{
  return (_recip_type == "P");
}

int  QuoteLineEdit::recipId()
{
  return _recip_id;
}
