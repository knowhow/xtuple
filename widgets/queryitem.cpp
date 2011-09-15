/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "queryitem.h"

#include <QMessageBox>
#include <QSqlError>

#include <metasql.h>

QueryItem::QueryItem(QWidget *parent, Qt::WindowFlags fl)
  : QWidget(parent, fl)
{
  setupUi(this);

  // SelectMQL is not part of the widgets library so insert it manually
  QGridLayout *mqlLayout = new QGridLayout(_qryStack->widget(_qryStack->indexOf(_qryMQLPage)));
  _selectmql = new SelectMQL(this); //, Qt::Widget);
  _selectmql->setObjectName("_selectmql");
  mqlLayout->addWidget(_selectmql, 0, 0);

  // MQLEdit is not part of the widgets library so insert it manually
  QGridLayout *customLayout = new QGridLayout(_qryStack->widget(_qryStack->indexOf(_qryCustomPage)));
  _mqledit = new MQLEdit(this); //, Qt::Widget);
  _mqledit->setObjectName("_mqledit");
  _mqledit->forceTestMode(! _x_privileges ||
                          ! _x_privileges->check("ExecuteMetaSQL"));
  customLayout->addWidget(_mqledit, 0, 0);

  connect(_mqledit->_text,SIGNAL(textChanged()), this, SLOT(sHandleButtons()));
  connect(_qryName,   SIGNAL(editingFinished()), this, SLOT(sHandleButtons()));
  connect(_qryRelation,      SIGNAL(newID(int)), this, SLOT(sHandleButtons()));
  connect(_qrySchema,        SIGNAL(newID(int)), this, SLOT(sUpdateRelations()));
  connect(_qrySrc,           SIGNAL(newID(int)), this, SLOT(sHandleButtons()));
  connect(_save,              SIGNAL(clicked()), this, SLOT(sSave()));
  connect(_selectmql,SIGNAL(selectedQuery(int)), this, SLOT(sHandleButtons()));

  _qrySrc->append(0, tr("Entire Table or View"), "REL");
  _qrySrc->append(1, tr("Pre-defined MetaSQL"),  "MQL");
  _qrySrc->append(2, tr("Custom Query"),         "CUSTOM");

  _qrySchema->populate("SELECT pg_namespace.oid, nspname, nspname,"
                         "       CASE nspname WHEN 'public' THEN '  '"
                         "                    WHEN 'api'    THEN ' '"
                         "                    ELSE '' END || nspname AS orderby"
                         "  FROM pg_namespace "
                         "  JOIN pkghead ON (nspname IN (pkghead_name, 'api',"
                         "                               'public'))"
                         " ORDER BY orderby;");

  clear();

  sUpdateRelations();
  sHandleButtons();
}

QueryItem::~QueryItem()
{
  // no need to delete child widgets, Qt does it all for us
}

int QueryItem::id() const
{
  return _qryitemid;
}

int QueryItem::parentId() const
{
  return _qryheadid;
}

void QueryItem::clear()
{
  _qryitemid = -1;
  _qryheadid = -1;
  _qryName->setText("");
  _qryOrder->setValue(0);
  _qryNotes->setPlainText("");
  _qrySrc->setId(-1);
  _qrySchema->setId(-1);
  _qryRelation->setId(-1);
  _selectmql->setId(-1);
  _mqledit->clear();
}

void QueryItem::setId(int p)
{
  XSqlQuery itemq;
  itemq.prepare("SELECT *"
                "  FROM qryitem"
                " WHERE (qryitem_id=:id);");
  itemq.bindValue(":id", p);
  itemq.exec();
  if (itemq.first())
  {
    _qryitemid = itemq.value("qryitem_id").toInt();
    _qryheadid = itemq.value("qryitem_qryhead_id").toInt();
    _qryName->setText(itemq.value("qryitem_name").toString());
    _qryOrder->setValue(itemq.value("qryitem_order").toInt());
    _qryNotes->setPlainText(itemq.value("qryitem_notes").toString());
    _qrySrc->setCode(itemq.value("qryitem_src").toString());

    switch (_qrySrc->id())
    {
      case 0: _qrySchema->setCode(itemq.value("qryitem_group").toString());
              _qryRelation->setCode(itemq.value("qryitem_detail").toString());
              _qryStack->setCurrentWidget(_qryRelationPage);
              break;
      case 1: {
              XSqlQuery mqlq;
              mqlq.prepare("SELECT metasql_id"
                           "  FROM metasql"
                           " WHERE ((metasql_group=:group)"
                           "    AND (metasql_name=:name))"
                           " ORDER BY metasql_grade DESC"
                           " LIMIT 1;");
              mqlq.bindValue(":group", itemq.value("qryitem_group"));
              mqlq.bindValue(":name",  itemq.value("qryitem_detail"));
              mqlq.exec();
              if (mqlq.first())
              {
                _selectmql->setId(mqlq.value("metasql_id").toInt());
                _qryStack->setCurrentWidget(_qryMQLPage);
              }
              else if (mqlq.lastError().type() != QSqlError::NoError)
              {
                QMessageBox::warning(this, tr("Database Error"),
                                     mqlq.lastError().text());
                return;
              }
              break;
              }
      case 2: _mqledit->_text->setPlainText(itemq.value("qryitem_detail").toString());
              _qryStack->setCurrentWidget(_qryCustomPage);
              break;
      default: QMessageBox::warning(this, tr("Unknown Query Source"),
                                    tr("<p>Could not populate this Query Item "
                                       "properly because the query source %1 "
                                       "is not recognized.")
                                    .arg(itemq.value("qryitem_src").toString()));
              _qryStack->setCurrentIndex(0);
               break;
    }
  }
  else if (itemq.lastError().type() != QSqlError::NoError)
  {
    QMessageBox::warning(this, tr("Database Error"), itemq.lastError().text());
    return;
  }
}

void QueryItem::setParentId(int p)
{
  _qryheadid = p;
}

void QueryItem::languageChange()
{
  retranslateUi(this);
}

void QueryItem::sHandleButtons()
{
  if (_qrySrc->code() == "REL")
    _save->setEnabled(! _qryName->text().isEmpty() &&
                      _qrySchema->id() > 0 &&
                      _qryRelation->id() > 0);

  else if (_qrySrc->code() == "MQL")
    _save->setEnabled(! _qryName->text().isEmpty() && _selectmql->id() > 0);

  else if (_qrySrc->code() == "CUSTOM")
    _save->setEnabled(! _qryName->text().isEmpty() &&
                      ! _mqledit->_text->toPlainText().isEmpty());

  else
    _save->setEnabled(false);
}

void QueryItem::sSave()
{
  XSqlQuery saveq;
  if (_qryitemid < 0)
    saveq.prepare("INSERT INTO qryitem ("
                  "    qryitem_qryhead_id,  qryitem_order,   qryitem_src,"
                  "    qryitem_group,       qryitem_detail,  qryitem_notes,"
                  "    qryitem_name"
                  ") VALUES ("
                  "   :qryitem_qryhead_id, :qryitem_order,  :qryitem_src,"
                  "   :qryitem_group,      :qryitem_detail, :qryitem_notes,"
                  "   :qryitem_name"
                  ") RETURNING qryitem_id;");
  else
  {
    saveq.prepare("UPDATE qryitem SET"
                  "    qryitem_qryhead_id=:qryitem_qryhead_id,"
                  "    qryitem_order=:qryitem_order,"
                  "    qryitem_src=:qryitem_src,"
                  "    qryitem_group=:qryitem_group,"
                  "    qryitem_detail=:qryitem_detail,"
                  "    qryitem_notes=:qryitem_notes,"
                  "    qryitem_name=:qryitem_name"
                  " WHERE qryitem_id=:qryitem_id"
                  " RETURNING qryitem_id;");
    saveq.bindValue(":qryitem_id", _qryitemid);
  }
  saveq.bindValue(":qryitem_qryhead_id", _qryheadid);
  saveq.bindValue(":qryitem_name",       _qryName->text());
  saveq.bindValue(":qryitem_order",      _qryOrder->value());
  saveq.bindValue(":qryitem_notes",      _qryNotes->toPlainText());
  saveq.bindValue(":qryitem_src",        _qrySrc->code());

  switch (_qrySrc->id())
  {
    case 0: saveq.bindValue(":qryitem_group",  _qrySchema->code());
            saveq.bindValue(":qryitem_detail", _qryRelation->code());
            break;
    case 1: saveq.bindValue(":qryitem_group",  _selectmql->group());
            saveq.bindValue(":qryitem_detail", _selectmql->name());
            break;
    case 2: saveq.bindValue(":qryitem_detail", _mqledit->_text->toPlainText());
            break;
    default: QMessageBox::warning(this, tr("Unknown Query Source"),
                                  tr("<p>Could not save this Query Item "
                                     "because the query source %1 "
                                     "is not recognized.")
                                  .arg(_qrySrc->code()));
             return;
  }

  saveq.exec();
  if (saveq.first())
    _qryitemid = saveq.value("qryitem_id").toInt();
  else if (saveq.lastError().type() != QSqlError::NoError)
  {
    QMessageBox::warning(this, tr("Database Error"), saveq.lastError().text());
    return;
  }
  
  close();
  emit saved(_qryitemid);
  emit closed(true);
}

void QueryItem::sUpdateRelations()
{
  if (! _x_preferences)   // don't query when loading in Qt Designer
    return;

  MetaSQLQuery mql("SELECT oid, relname, relname"
                 "  FROM pg_class"
                 " WHERE ((relkind IN ('r', 'v'))"
                 " <? if exists(\"oid\") ?>"
                 "    AND (relnamespace=<? value(\"oid\") ?>)"
                 " <? endif ?>"
                 " )"
                 " ORDER BY relname;");
  ParameterList params;
  if (_qrySchema->id() > 0)
    params.append("oid", _qrySchema->id());

  XSqlQuery popq = mql.toQuery(params);
  _qryRelation->populate(popq);
  if (popq.lastError().type() != QSqlError::NoError)
  {
    QMessageBox::warning(this, tr("Database Error"), popq.lastError().text());
    return;
  }
}

void QueryItem::closeEvent(QCloseEvent *event)
{
  clear();
  QWidget::closeEvent(event);
}
