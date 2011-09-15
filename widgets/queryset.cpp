/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "queryset.h"

#include <QMessageBox>
#include <QSqlError>
#include <QtScript>

#include <metasql.h>

#include "queryitem.h"

#define DEBUG false

QuerySet::QuerySet(QWidget *parent, Qt::WindowFlags fl)
  : QWidget(parent, fl)
{
  setupUi(this);

  // create child first so we can connect signals but not if we're in Designer
  if (_x_preferences)
  {
    _itemdlg = new QueryItem(0, Qt::Dialog);
    if (_itemdlg)
    {
      _itemdlg->setWindowModality(Qt::WindowModal);
      connect(_itemdlg, SIGNAL(closed(bool)), this, SLOT(sFillList()));
    }
  }
  else
    _itemdlg = 0;

  connect(_qryDelete,  SIGNAL(clicked()),         this, SLOT(sDelete()));
  connect(_qryEdit,    SIGNAL(clicked()),         this, SLOT(sEdit()));
  connect(_qryList,    SIGNAL(itemSelected(int)), this, SLOT(sEdit()));
  connect(_qryNew,     SIGNAL(clicked()),         this, SLOT(sNew()));
  connect(_save,       SIGNAL(clicked()),         this, SLOT(sSave()));
  connect(_setName,    SIGNAL(editingFinished()), this, SLOT(sHandleButtons()));

  _qryList->addColumn(tr("Order"),        _seqColumn, Qt::AlignRight, true, "qryitem_order");
  _qryList->addColumn(tr("Name"),        _itemColumn, Qt::AlignLeft,  true, "qryitem_name");
  _qryList->addColumn(tr("Source"),       _seqColumn, Qt::AlignLeft,  true, "qryitem_src");
  _qryList->addColumn(tr("Schema/Group"),_itemColumn, Qt::AlignLeft,  true, "qryitem_group");
  _qryList->addColumn(tr("Table/Name"),           -1, Qt::AlignLeft,  true, "qryitem_detail");

  sHandleButtons();

  _qryheadid = -1;

}

QuerySet::~QuerySet()
{
  // no need to delete child widgets, Qt does it all for us
  // but _itemdlg isn't really a child
  if (_itemdlg)
    _itemdlg->deleteLater();
}

int QuerySet::id() const
{
  return _qryheadid;
}

void QuerySet::setId(int p)
{
  XSqlQuery headq;
  headq.prepare("SELECT *"
                "  FROM qryhead"
                " WHERE (qryhead_id=:id);");
  headq.bindValue(":id", p);
  headq.exec();
  if (headq.first())
  {
    _qryheadid = headq.value("qryhead_id").toInt();
    _setName->setText(headq.value("qryhead_name").toString());
    _setDescrip->setText(headq.value("qryhead_descrip").toString());
    _setNotes->setPlainText(headq.value("qryhead_notes").toString());
    sFillList();
  }
  else
  {
    _qryheadid = -1;
    _setName->setText("");
    _setDescrip->setText("");
    _qryList->clear();

    if (headq.lastError().type() != QSqlError::NoError)
    {
      QMessageBox::warning(this, tr("Database Error"),
                           headq.lastError().text());
      return;
    }
  }
}

void QuerySet::languageChange()
{
  retranslateUi(this);
}

void QuerySet::sDelete()
{
  if (_qryList->id() > 0 &&
      QMessageBox::question(this, tr("Delete Query Item?"),
                            tr("<p>Are you sure you want to delete this Query "
                               "Item from the Query Set?"),
                            QMessageBox::Yes | QMessageBox::No,
                            QMessageBox::No) == QMessageBox::Yes)
  {
    XSqlQuery delq;
    delq.prepare("DELETE FROM qryitem WHERE qryitem_id=:id;");
    delq.bindValue(":id", _qryList->id());
    delq.exec();
    if (delq.lastError().type() != QSqlError::NoError)
    {
      QMessageBox::warning(this, tr("Database Error"), delq.lastError().text());
      return;
    }
  }
}

void QuerySet::sEdit()
{
  if (_itemdlg)
  {
    _itemdlg->setParentId(_qryheadid);
    _itemdlg->setId(_qryList->id());
    _itemdlg->show();
  }
}

void QuerySet::sFillList()
{
  XSqlQuery itemq;
  itemq.prepare("SELECT qryitem_id, qryitem_name, qryitem_order, qryitem_src,"
                "       qryitem_group,"
                "       REGEXP_REPLACE(qryitem_detail, E'\\n.*', '...')"
                "         AS qryitem_detail,"
                "       qryitem_detail AS qryitem_detail_qttooltiprole"
                "  FROM qryitem"
                " WHERE (qryitem_qryhead_id=:id)"
                " ORDER BY qryitem_order;");
  itemq.bindValue(":id",  _qryheadid);
  itemq.exec();
  _qryList->populate(itemq);
  if (itemq.lastError().type() != QSqlError::NoError)
  {
    QMessageBox::warning(this, tr("Database Error"),
                         itemq.lastError().text());
    return;
  }
}

void QuerySet::sHandleButtons()
{
  _save->setEnabled(! _setName->text().isEmpty());
}

void QuerySet::sNew()
{
  if (_qryheadid < 0)
  {
    if (QMessageBox::question(this, tr("Save First?"),
                              tr("<p>The Query Set has not yet been saved to "
                                 "the database. You must save it before adding "
                                 "queries.</p><p>Would you like to save before "
                                 "continuing?</p>"),
                              QMessageBox::Yes | QMessageBox::No,
                              QMessageBox::Yes) == QMessageBox::No ||
        ! sSave(false))
      return;
  }

  if (_itemdlg)
  {
    _itemdlg->setParentId(_qryheadid);
    _itemdlg->setId(-1);
    _itemdlg->show();
  }
}

bool QuerySet::sSave(bool done)
{
  XSqlQuery saveq;
  if (_qryheadid < 0)
    saveq.prepare("INSERT INTO qryhead ("
                  "    qryhead_name,  qryhead_descrip,  qryhead_notes"
                  ") VALUES ("
                  "   :qryhead_name, :qryhead_descrip, :qryhead_notes"
                  ") RETURNING qryhead_id;");
  else
  {
    saveq.prepare("UPDATE qryhead SET"
                  "    qryhead_id=:qryhead_id,"
                  "    qryhead_name=:qryhead_name,"
                  "    qryhead_descrip=:qryhead_descrip,"
                  "    qryhead_notes=:qryhead_notes"
                  " WHERE qryhead_id=:qryhead_id"
                  " RETURNING qryhead_id;");
    saveq.bindValue(":qryhead_id", _qryheadid);
  }
  
  saveq.bindValue(":qryhead_name",    _setName->text());
  saveq.bindValue(":qryhead_descrip", _setDescrip->text());
  saveq.bindValue(":qryhead_notes",   _setNotes->toPlainText());
  saveq.exec();
  if (saveq.first())
    _qryheadid = saveq.value("qryhead_id").toInt();
  else if (saveq.lastError().type() != QSqlError::NoError)
  {
    QMessageBox::warning(this, tr("Database Error"), saveq.lastError().text());
    return false;
  }

  emit saved(_qryheadid);
  if (done)
  {
    close();
    emit closed(true);
  }

  return true;
}

// script exposure ////////////////////////////////////////////////////////////

void QuerySetfromScriptValue(const QScriptValue &obj, QuerySet* &item)
{
  item = qobject_cast<QuerySet*>(obj.toQObject());
}

QScriptValue QuerySettoScriptValue(QScriptEngine *engine, QuerySet* const &item)
{
  return engine->newQObject(item);
}

QScriptValue constructQuerySet(QScriptContext *context,
                               QScriptEngine  *engine)
{
  QuerySet *obj = 0;
  if (context->argumentCount() == 1 &&
      (context->argument(0).toInt32() == 0 ||
       qscriptvalue_cast<QWidget*>(context->argument(0))))
  {
    if (DEBUG) qDebug("constructQuerySet got a QWidget*");
    obj = new QuerySet(qscriptvalue_cast<QWidget*>(context->argument(0)));
  }
  else if (context->argumentCount() >= 2 &&
           (context->argument(0).toInt32() == 0 ||
            qscriptvalue_cast<QWidget*>(context->argument(0))))
  {
    if (DEBUG) qDebug("constructQuerySet got a QWidget* and an int");
    obj = new QuerySet(qscriptvalue_cast<QWidget*>(context->argument(0)),
                       (Qt::WindowFlags)(context->argument(1).toInt32()));
  }

  return engine->toScriptValue(obj);
}

void setupQuerySet(QScriptEngine *engine)
{
  qScriptRegisterMetaType(engine, QuerySettoScriptValue, QuerySetfromScriptValue);

  QScriptValue widget = engine->newFunction(constructQuerySet);

  engine->globalObject().setProperty("QuerySet", widget, QScriptValue::ReadOnly | QScriptValue::Undeletable);
}
