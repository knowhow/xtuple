/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */


#include "fixSerial.h"

#include <QApplication>
#include <QCursor>
#include <QSqlError>

#include <metasql.h>

fixSerial::fixSerial(QWidget* parent, const char * name, Qt::WindowFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  connect(_fix,		SIGNAL(clicked()),	this, SLOT(sFix()));
  connect(_fixAll,	SIGNAL(clicked()), 	this, SLOT(sFixAll()));
  connect(_query,	SIGNAL(clicked()),	this, SLOT(sFillList()));
  connect(_serial,	SIGNAL(valid(bool)),	this, SLOT(sHandleSerial()));
  connect(_serial,	SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*)),
			    this, SLOT(sPopulateMenu(QMenu*, QTreeWidgetItem*)));
  connect(_showProblems, SIGNAL(toggled(bool)), this, SLOT(sFillList()));

  _serial->addColumn(tr("Schema Name"),		-1,	Qt::AlignLeft, true, "nspname");
  _serial->addColumn(tr("Table Name"),		-1,	Qt::AlignLeft, true, "relname");
  _serial->addColumn(tr("Column Name"),		-1,	Qt::AlignLeft, true, "attname");
  _serial->addColumn(tr("Sequence Name"),	-1,	Qt::AlignLeft, true, "seq");
  _serial->addColumn(tr("Largest Key Used"),	-1,	Qt::AlignRight, true, "maxval");
  _serial->addColumn(tr("Next Key"),		-1,	Qt::AlignRight, true, "lastvalue");
}

fixSerial::~fixSerial()
{
  // no need to delete child widgets, Qt does it all for us
}

void fixSerial::languageChange()
{
  retranslateUi(this);
}

void fixSerial::sFillList()
{
  _serial->clear();

  QApplication::setOverrideCursor( QCursor(Qt::WaitCursor) );

  QString sql = "SELECT nspname ||'.' ||relname AS tablename, nspname, relname, attname, "
		"	TRIM(quote_literal('\"''') FROM"
		"	  SUBSTRING(pg_catalog.pg_get_expr(d.adbin, d.adrelid)"
		"	  FROM '[' || quote_literal('\"''') || "
		"               '].*[' || quote_literal('\"''') || ' ]')) AS seq"
		"  FROM pg_catalog.pg_attribute a, pg_catalog.pg_class,"
		"       pg_catalog.pg_attrdef d, pg_catalog.pg_namespace   "
		"  WHERE a.attnum > 0"
                "    AND pg_namespace.oid = pg_class.relnamespace"
		"    AND NOT a.attisdropped"
		"    AND a.attnotnull"
		"    AND a.attrelid = pg_class.oid"
		"    AND d.adrelid = a.attrelid"
		"    AND d.adnum = a.attnum"
		"    AND pg_catalog.pg_get_expr(d.adbin, d.adrelid) ~* 'nextval'"
		"    AND a.atthasdef "
		"ORDER BY relname;" ;

  XSqlQuery relq;
  relq.prepare(sql);

  QString maxStr = "SELECT MAX(<? literal(\"attname\" ?>) AS maxval "
		   "FROM <? literal(\"tablename\") ?>;" ;
  XSqlQuery maxq;

  QString seqStr = "SELECT last_value AS currval FROM <? literal(\"seq\") ?>;" ;
  XSqlQuery seqq;

  XTreeWidgetItem *last = 0;
  int rows	= 0;
  int maxval	= 0;
  int currval	= 0;
  int errors	= 0;

  relq.exec();
  while (relq.next())
  {
    ParameterList params;
    params.append("attname",	relq.value("attname").toString());
    params.append("tablename",	relq.value("tablename").toString());
    params.append("seq",	relq.value("seq").toString());

    MetaSQLQuery maxMql = MetaSQLQuery(maxStr);
    maxq = maxMql.toQuery(params);
    if (maxq.first())
      maxval = maxq.value("maxval").toInt();
    else if (maxq.lastError().type() != QSqlError::NoError)
    {
      systemError(this, maxq.lastError().databaseText(), __FILE__, __LINE__);
      continue;
    }

    MetaSQLQuery seqMql = MetaSQLQuery(seqStr);
    seqq = seqMql.toQuery(params);
    if (seqq.first())
      currval = seqq.value("currval").toInt();
    else if (seqq.lastError().type() != QSqlError::NoError)
    {
      systemError(this, seqq.lastError().databaseText(), __FILE__, __LINE__);
      continue;
    }

    rows++;

    if (maxval > currval)
      errors++;

    if ((_showProblems->isChecked() && maxval > currval) ||
	! _showProblems->isChecked())
    {
      last = new XTreeWidgetItem(_serial, last, rows, maxval > currval ? 1 : 0,
				 relq.value("nspname"),
                                 relq.value("relname"),
				 relq.value("attname"),
				 relq.value("seq"),
				 maxval,
				 currval);

      if (maxval > currval)
	last->setTextColor("red");
    }
  }

  QApplication::restoreOverrideCursor();
  if (relq.lastError().type() != QSqlError::NoError)
  {
    systemError(this, relq.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  if (errors > 0)
    _statusLit->setText(QObject::tr("Found %1 tables with mismatched serial values.")
			.arg(errors));
  else
    _statusLit->setText(QObject::tr("No problems found"));

  _fixAll->setEnabled(errors > 0);
}

void fixSerial::sHandleSerial()
{
  _fix->setEnabled(_serial->altId() > 0);
}

bool fixSerial::fixOne(XTreeWidgetItem *pItem)
{
  q.prepare("SELECT SETVAL(:sequence, :value);");
  q.bindValue(":sequence",	pItem->text(3));
  q.bindValue(":value",		pItem->text(4));

  q.exec();
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return false;
  }

  return true;
}

void fixSerial::sFix()
{
  if (fixOne(static_cast<XTreeWidgetItem*>(_serial->currentItem())))
    sFillList();
}

void fixSerial::sFixAll()
{
  QList<XTreeWidgetItem*> all = _serial->findItems("", Qt::MatchContains);
  for (int i = 0; i < all.size(); i++)
  {
    XTreeWidgetItem *currItem = static_cast<XTreeWidgetItem*>(all[i]);
    if (currItem->altId() > 0)
      fixOne(currItem);
  }
  sFillList();
}

void fixSerial::sPopulateMenu(QMenu *pMenu, QTreeWidgetItem *pItem)
{
  QAction *menuItem;

  menuItem = pMenu->addAction(tr("Fix"), this, SLOT(sFix()));
  menuItem->setEnabled((static_cast<XTreeWidgetItem*>(pItem))->altId() > 0 &&
                       _privileges->check("FixSerial"));

  menuItem = pMenu->addAction(tr("Fix All"), this, SLOT(sFixAll()));
  menuItem->setEnabled(_privileges->check("FixSerial"));
}
