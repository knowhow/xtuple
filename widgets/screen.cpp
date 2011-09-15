/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "screen.h"

#include <QMessageBox>
#include <QSqlError>
#include <QSqlField>
#include <QScriptContext>
#include <QScriptEngine>
#include <QSqlDatabase>

#include <openreports.h>
#include <qmd5.h>

Screen::Screen(QWidget *parent) : 
  QWidget(parent)
{
  _keyColumns=1;
  _lock=false;
  _locked=false;
  _mode=Edit;
  _shown=false;
  _key=0;
  
  _model = new XSqlTableModel;
  _mapper = new XDataWidgetMapper;
    
  connect(_mapper, SIGNAL(currentIndexChanged(int)), this, SIGNAL(currentIndexChanged(int)));
}

Screen::~Screen()
{ 
  unlock();
}

Screen::Disposition Screen::check()
{
  if (isDirty())
  {
    if (QMessageBox::question(this, tr("Unsaved work"),
                                   tr("You have made changes that have not been saved.  Would you like an opportunity to save your work?"),
                                   QMessageBox::Yes | QMessageBox::Default,
                                   QMessageBox::No ) == QMessageBox::Yes)
      return Save;
    else
      return Cancel;
  }
  else if (_mode == New)
    return Cancel;
  return NoChanges;
}

/* When the widget is first shown, set up table mappings if they exist*/
void Screen::showEvent(QShowEvent *event)
{
  if(!_shown)
  {
    _shown = true;
    setTable(_schemaName, _tableName);
  }

  QWidget::showEvent(event);
}

Screen::Modes Screen::mode()
{
  return _mode;
}

bool Screen::cancel()
{
  switch (check())
  {
    case Save:
      return false;
    case Cancel:
      revertRow(currentIndex());
      return true;
    default: //No Change
      return true;
  }
}

bool Screen::isDirty()
{
  if (_mode==New) //If New and nothing has been changed yet on new row, it's not really dirty
  {
    for (int i = 0; _mapper->model() && i < _mapper->model()->columnCount(); i++)
      if (_mapper->mappedWidgetAt(i))
      {
        QString def = _mapper->mappedWidgetAt(i)->property(_mapper->mappedDefaultName(_mapper->mappedWidgetAt(i))).toString();
        QString cur = _mapper->mappedWidgetAt(i)->property(_mapper->mappedPropertyName(_mapper->mappedWidgetAt(i))).toString();
        if (!def.isEmpty() || !cur.isEmpty())
          if (def != cur)
            return true; 
      }
  }
  else if(_mode == View)
  {
    return false;
  }
  else  //Use usual technique to determine if edits occured
  {
    for (int c = 0; c < _model->columnCount(); c++)
      if (_model->isDirty(_model->index(currentIndex(),c)))
        return true;
  }
  return false;
}

bool Screen::submit()
{
  bool isSaved;
  disconnect(_mapper, SIGNAL(currentIndexChanged(int)), this, SIGNAL(currentIndexChanged(int)));
  int i=_mapper->currentIndex();
  _mapper->submit();  // Make sure changes committed even if user hasn't tabbed off widget
  isSaved=_model->submitAll(); // Apply to the database
  _mapper->setCurrentIndex(i);
  if (!isSaved)
  {
    if(!throwScriptException(_model->lastError().databaseText()))
        QMessageBox::critical(this, tr("Error Saving %1").arg(_tableName),
                          tr("Error saving %1 at %2::%3\n\n%4")
                          .arg(_tableName).arg(__FILE__).arg(__LINE__)
                          .arg(_model->lastError().databaseText()));
  }
  connect(_mapper, SIGNAL(currentIndexChanged(int)), this, SIGNAL(currentIndexChanged(int)));
  return isSaved;
}

bool Screen::throwScriptException(const QString &message)
{ 
   QObject *ancestor = this;
   QScriptEngine *engine = 0;
   for ( ; ancestor; ancestor = ancestor->parent())
   {
     engine = ancestor->findChild<QScriptEngine*>();
     if (engine)
       break;
   } 
   if (engine)
   {
      QScriptContext *ctx = engine->currentContext();
      ctx->throwError(message);
      return true;
   }
   return false;
}

/* Build a uinque key, then attempt to lock on postgres */
bool Screen::tryLock()
{
  if (locked())
    unlock();
    
  if (!lockRecords())
    return false;
  
  if (_model && _mode != View)
  {
    QStringList keys;
    QSqlIndex idx=_model->primaryKey();

    // Build a unique key string by concatenating primary key table name and column values
    keys.insert(0,idx.name());
    for (int i = 0; i < idx.count(); ++i)
      keys.insert(i+1,_model->data(_model->index(currentIndex(),i)).toString());
      
    // Convert to a uinque 128 bit number
    QString qkey = QMd5(keys.join(";"));
    
    // Have to get this number <= 64 bits.
    qkey.resize(15); 
    bool ok;
    _key = qkey.toLongLong(&ok,16);
    if (!ok)
      qDebug("Conversion to LongLong failed");  
      
    // Split 64 bit key in two integers by shifting 32 bits to the right
    int upper = _key >> 32;
    int lower = _key;
    
    XSqlQuery lockQuery;
    lockQuery.prepare("SELECT tryLock(:upper,:lower) AS result;");
    lockQuery.bindValue(":upper", upper);
    lockQuery.bindValue(":lower", lower);
    lockQuery.exec();
    if (lockQuery.first())
      _locked = lockQuery.value("result").toBool();
      
    setWidgetsEnabled(_locked && _mode != View); 
    
    if (!_locked)
    {
      _key = 0;
      QMessageBox::information(this, tr("Record locked"),
        tr("This record is currently in use by another user.  It will be opened in view mode."));
    }
  }

  emit lockGranted(_locked);
  return _locked;
}

int Screen::currentIndex()
{
  return _mapper->currentIndex();
}

void Screen::deleteCurrent()
{
  if (!locked())
  {
    removeCurrent();
    save();
  }
  else
    QMessageBox::information(this, tr("Record locked"),
        tr("This record is currently in use by another user.  It may not be edited or deleted at this time."));
}

void Screen::clear()
{
  unlock();
  _mapper->clear();
}

void Screen::insert()
{
  unlock();
  if (_mapper->model())
  {
    _mapper->model()->insertRows(_model->rowCount(),1);
    _mapper->toLast();
    _mapper->clear();
  }
}

void Screen::loadAll()
{
  _model->loadAll();
  if (_model->rowCount())
  {
    _mapper->toFirst();
    tryLock();
  }
}

void Screen::newMappedWidget(QWidget *widget)
{
  widget->setEnabled(_mode != View);
}

void Screen::removeCurrent()
{
  unlock();
  removeRows(_mapper->currentIndex(),1);
}

void Screen::removeRows(int row, int count)
{
  _mapper->model()->removeRows(row, count);
}

void Screen::revert()
{
  _mapper->revert();
  emit reverted(currentIndex());
}

void Screen::revertAll()
{
  _model->revertAll();
  emit revertedAll();
}

void Screen::revertRow(int row)
{
  _model->revertRow(row);
}

void Screen::save()
{ 
  if (locked())
    return;
  
  if (submit())
  {
    emit saved();
    if (_mode==New)
      insert();
  }
}

void Screen::search(QString criteria)
{
  QString filter;
  if (criteria.trimmed().length())
  {
    QStringList parts;
    for (int i = 0; i < _model->columnCount(); i++)
    { 
      if (_model->headerData(i,Qt::Horizontal,Qt::DisplayRole).toString().length())
        parts.append("(CAST(" +
                     _model->headerData(i,Qt::Horizontal,Qt::DisplayRole).toString() +
                     " AS TEXT) ~* '" + criteria + "')");
    }
    filter=parts.join(" OR ");
  }
  setFilter(filter);
  select();
}

void Screen::select()
{
  _model->select();
  if (_model->rowCount())
  {
    _mapper->toFirst();
    tryLock();
  }
}

void Screen::setLockRecords(bool lock)
{
  if (primaryKeyColumns())
    _lock = lock;
  else
    _lock = false;
}

void Screen::setMode(Modes p)
{
  if (_mode == p)
    return;
    
  unlock();
  
  _mode = p;
  if (_mode == New)
    insert();
    
  if (_mode == Edit)
    tryLock();
  else
    setWidgetsEnabled(_mode != View);
}

void Screen::setModel(XSqlTableModel *model)
{
  _model=model;
  _model->setKeys(_keyColumns);
  _model->setEditStrategy(QSqlTableModel::OnManualSubmit);
  setDataWidgetMapper(_model);
  emit newModel(_model);
}

void Screen::setSortColumn(QString p)
{
  _sortColumn = p;
}

void Screen::setCurrentIndex(int index)
{
  _mode=Screen::Edit;
  _mapper->setCurrentIndex(index);
  if (lockRecords())
    tryLock();
}

void Screen::toNext()
{
  if (_mapper->currentIndex() < _model->rowCount()-1)
  {
    _mapper->toNext();
    if (lockRecords())
      tryLock();
  }
}

void Screen::toPrevious()
{
  if (_mapper->currentIndex() > 0)
  {
    _mapper->toPrevious(); 
    if (lockRecords())
      tryLock();
  }
}

void Screen::setSchemaName(QString schema)
{ 
  _schemaName = schema;  
}

void Screen::setTableName(QString table)
{
  _tableName = table;
}

/* Pass in a schema name and a table name.  If schema name is blank, it will be ignored.
     If there is a table name to set, data widget mappings will be set as well. */
void Screen::setTable(QString schema, QString table)
{
  if (table.length())
  {
    QString tablename="";
    if (schema.length())
      tablename=schema + ".";
    tablename+=table;
    if (_model->tableName() != tablename)
    {
      _model->setTable(tablename,_keyColumns);
      _model->setEditStrategy(QSqlTableModel::OnManualSubmit);
      setDataWidgetMapper(_model);
      emit newModel(_model);
    }
  }
}

void Screen::setDataWidgetMapper(XSqlTableModel *model)
{
  _mapper->setModel(model);
  emit newDataWidgetMapper(_mapper);
}

void Screen::setWidgetsEnabled(bool enabled)
{
  for (int i = 0; _mapper->model() && i < _mapper->model()->columnCount(); i++)
    if (_mapper->mappedWidgetAt(i))
      _mapper->mappedWidgetAt(i)->setEnabled(enabled);
}

void Screen::unlock()
{
  if (!locked())
    return;
    
  if (_key)
  {    
    int upper = _key >> 32;
    int lower = _key;
    
    XSqlQuery unlock;
    unlock.prepare("SELECT pg_advisory_unlock(:upper, :lower);");
    unlock.bindValue(":upper", upper);
    unlock.bindValue(":lower", lower);
    unlock.exec();
    _key = 0;
  }
  _locked = false;
  setWidgetsEnabled(_mode != View); 
  emit lockGranted(false);
}
