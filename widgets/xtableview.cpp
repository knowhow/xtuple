/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "xtableview.h"
#include "xsqltablemodel.h"

#include <QAbstractItemDelegate>
#include <QBuffer>
#include <QHeaderView>
#include <QMessageBox>
#include <QModelIndex>
#include <QPalette>
#include <QScriptEngine>
#include <QSqlDatabase>
#include <QSqlDriver>
#include <QSqlError>
#include <QSqlField>
#include <QSqlIndex>
#include <QSqlQuery>
#include <QSqlRelationalTableModel>
#include <QStack>
#include <QTreeWidgetItem>

#include "format.h"
#include "xtsettings.h"
#include "xitemdelegate.h"

#define DEBUG true

XTableView::XTableView(QWidget *parent) : 
  QTableView(parent)
{
  _keyColumns        = 1;
  _resizingInProcess = false;
  _settingsLoaded    = false;
  _forgetful         = false;

  _menu = new QMenu(this);
  _menu->setObjectName("_menu");

  setContextMenuPolicy(Qt::CustomContextMenu);
  verticalHeader()->setDefaultSectionSize(20);
  horizontalHeader()->setStretchLastSection(false);
  horizontalHeader()->setClickable(true);
  horizontalHeader()->setContextMenuPolicy(Qt::CustomContextMenu); 

  setAlternatingRowColors(true);
  QPalette muted = palette();
  muted.setColor(QPalette::AlternateBase, QColor(0xEE, 0xEE, 0xEE));
  setPalette(muted);

  _mapper = new XDataWidgetMapper(this);
  _model = new XSqlTableModel(this);
  _model->setEditStrategy(QSqlTableModel::OnManualSubmit);
  
  connect(this, SIGNAL(customContextMenuRequested(const QPoint &)), SLOT(sShowMenu(const QPoint &)));
  connect(horizontalHeader(), SIGNAL(customContextMenuRequested(const QPoint &)),
                    SLOT(sShowHeaderMenu(const QPoint &)));
  connect(horizontalHeader(), SIGNAL(sectionResized(int, int, int)),
          this,     SLOT(sColumnSizeChanged(int, int, int)));
  connect(_model, SIGNAL(dataChanged(QModelIndex, QModelIndex)), this, SLOT(handleDataChanged(const QModelIndex, const QModelIndex)));
  _windowName = window()->objectName();
}

XTableView::~XTableView()
{
  if(_x_preferences)
  {
    // window()->objectName() isn't available here, at least for scripted windows!
    QString settingsPrefix = _windowName + "/" + objectName();

    xtsettingsSetValue(settingsPrefix + "/isForgetful", _forgetful);
    QString savedString;
    if(!_forgetful)
    {
      savedString = "";
      for(int i = 0; i < horizontalHeader()->count(); i++)
      {
        int w = -1;
        ColumnProps *cp = _columnByName.value(columnNameFromLogicalIndex(i), 0);
        if (cp)
        {
          if (!cp->stretchy)
            w = cp->savedWidth;
          _columnByName.remove(cp->columnName);
          delete cp;
        }
        if (w >= 0 && ! horizontalHeader()->isSectionHidden(i))
          savedString.append(QString::number(i) + "," +
                             QString::number(horizontalHeader()->sectionSize(i)) + "|");
      }
      xtsettingsSetValue(settingsPrefix + "/columnWidths", savedString);
    }
    savedString = "";
    for(int i = 0; i < horizontalHeader()->count(); i++)
    {
      savedString.append(QString::number(i) + "," + (horizontalHeader()->isSectionHidden(i)?"off":"on") + "|");
    }
    if(!savedString.isEmpty())
      _x_preferences->set(settingsPrefix + "/columnsShown", savedString);
    else
      _x_preferences->remove(settingsPrefix + "/columnsShown");
  }
}

int XTableView::columnIndex(const QString column)
{
  for (int colnum = 0; colnum < horizontalHeader()->count(); colnum++) {
    if (column == QTableView::model()->headerData(colnum, Qt::Horizontal,
                                                  Qt::UserRole).toString()) {
      return colnum;
    }
  }
  
  return -1;
}

QString XTableView::columnNameFromLogicalIndex(const int logicalIndex) const
{
  for (QMap<QString, ColumnProps*>::const_iterator it = _columnByName.constBegin();
       it != _columnByName.constEnd(); it++)
  {
    if (it.value()->logicalIndex == logicalIndex)
      return it.value()->columnName;
  }
  return QString();
}

QString XTableView::filter()
{
  return _model->filter();
}

void XTableView::handleDataChanged(const QModelIndex topLeft, const QModelIndex lowerRight)
{
  int col;
  int row;

  // Emit data changed signal
  for (col = topLeft.column(); col <= lowerRight.column(); col++) {
    for (row = topLeft.row(); row <= lowerRight.row(); row++)
      emit dataChanged(row, col);
  }
}

void XTableView::insert()
{ 
  int row=_model->rowCount();
  _model->insertRows(row,1);
  //Set default values for foreign keys
  for (int i = 0; i < _idx.count(); ++i)
    _model->setData(_model->index(row,i),_idx.value(i));
  selectRow(row);
}

QAbstractItemDelegate* XTableView::itemDelegateForColumn(const QString column)
{
  return itemDelegateForColumn(columnIndex(column));
}

void XTableView::populate(int p)
{   
  XSqlTableModel *t=static_cast<XSqlTableModel*>(_mapper->model());
  if (t)
  {
    _idx=t->primaryKey();

    for (int i = 0; i < _idx.count(); ++i) {
      _idx.setValue(i, t->data(t->index(p,i)));
      setColumnHidden(i,true);
    }
    
    QString clause = QString(_model->database().driver()->sqlStatement(QSqlDriver::WhereStatement, t->tableName(),_idx, false)).mid(6);
    _model->setFilter(clause);
    if (!_model->query().isActive()) {
      _model->select();
    }
  }
}

void XTableView::popupMenuActionTriggered(QAction * pAction)
{
  QMap<QString, QVariant> m = pAction->data().toMap();
  QString command = m.value("command").toString();
  if("toggleColumnHidden" == command)
    setColumnVisible(m.value("column").toInt(), pAction->isChecked());
  //else if (some other command to handle)
}

int XTableView::rowCount()
{
  return _model->rowCount();
}

int XTableView::rowCountVisible()
{
  int counter = 0;
  for (int i = 0; i < _model->rowCount(); i++)
  {
    if (!isRowHidden(i))
      counter++;
  }
  return counter;
}

void XTableView::select()
{
  setTable();
  _model->select();
  
  emit valid(FALSE);

}

void XTableView::removeSelected()
{
  QModelIndexList selected=selectedIndexes();

  for (int i = selected.count() -1 ; i > -1; i--) {
    if (selected.value(i).column() == 1) { // Only once per row
      _model->removeRow(selected.value(i).row());
      hideRow(selected.value(i).row());
    }
  }
}

void XTableView::resizeEvent(QResizeEvent * e)
{
  QTableView::resizeEvent(e);

  sColumnSizeChanged(-1, 0, 0);
}

void XTableView::revertAll()
{
  _model->revertAll();
  if (_mapper)
    populate(_mapper->currentIndex());
}

void XTableView::save()
{ 
  if(!_model->submitAll())
  {
    if(!throwScriptException(_model->lastError().databaseText()))
          QMessageBox::critical(this, tr("Error Saving %1").arg(_tableName),
                            tr("Error saving %1 at %2::%3\n\n%4")
                            .arg(_tableName).arg(__FILE__).arg(__LINE__)
                            .arg(_model->lastError().databaseText()));
  }
  else
    emit saved();
}

QVariant XTableView::selectedValue(int column)
{
  QModelIndexList selected=selectedIndexes();
  return value(selected.first().row(),column);
}

void XTableView::selectionChanged(const QItemSelection & selected, const QItemSelection & deselected)
{
  if (!selected.indexes().isEmpty())
  {
    emit rowSelected(selected.indexes().first().row());
    emit valid(true);
  }
  else
    emit valid(FALSE);
  QTableView::selectionChanged(selected, deselected);
}

void XTableView::selectRow(int index)
{
  selectionModel()->select(QItemSelection(model()->index(index,0),model()->index(index,0)),
                                        QItemSelectionModel::ClearAndSelect |
                                        QItemSelectionModel::Rows);
}

/* this is intended to be similar to XTreeWidget::addColumn(...)
   but differs in several important ways:
   - this doesn't actually add columns to the tree view, it only defines how
     they should appear by default if the model contains columns with the given
     colname.
   - as a result it has the opposite effect on visibility - any column not
     explicitly mentioned in a setColumn call /is visible/ in an XTableView
   - the alignment parameter only affects the header
TODO: figure out how to set the alignment of individual elements
 */
 
void XTableView::setColumn(const QString &label, int width, int alignment, bool visible, const QString &colname)
{
  ColumnProps *cp = _columnByName.value(colname, 0);

  if (! cp)
  {
    cp = new ColumnProps();
    cp->columnName = colname;
    cp->fromSettings = false;
    _columnByName.insert(cp->columnName, cp);
  }

  cp->defaultWidth = width;
  cp->locked       = false;
  cp->visible      = visible;
  cp->label        = label;
  cp->alignment    = alignment;

  if (cp->fromSettings)
    cp->stretchy   = (width == -1 || cp->savedWidth == -1);
  else
  {
    cp->savedWidth = -1;
    cp->stretchy   = (width == -1);
  }

  cp->logicalIndex = -1;
  int colnum = -1;
  for (colnum = 0; colnum < horizontalHeader()->count(); colnum++)
  {
    if (colname == QTableView::model()->headerData(colnum, Qt::Horizontal,
                                                  Qt::UserRole).toString())
    {
      cp->logicalIndex = colnum;
      QTableView::model()->setHeaderData(colnum, Qt::Horizontal, colname,
                                        Qt::UserRole);
      QTableView::model()->setHeaderData(colnum, Qt::Horizontal, label,
                                        Qt::DisplayRole);
      QTableView::model()->setHeaderData(colnum, Qt::Horizontal, alignment,
                                        Qt::TextAlignmentRole);
      setColumnTextAlignment(colnum, alignment);
      break;
    }
  }

  horizontalHeader()->setResizeMode(colnum, QHeaderView::Interactive);

  if (! cp->fromSettings)
    setColumnVisible(colnum, visible);

  if (_forgetful || ! cp->fromSettings)
    horizontalHeader()->resizeSection(colnum, width);
}

void XTableView::setColumnEditor(int column, const QString editor)
{
  _model->setColumnRole(column, XSqlTableModel::EditorRole, editor);
}

void XTableView::setColumnEditor(const QString column, const QString editor)
{
  setColumnRole(column, XSqlTableModel::EditorRole, editor);
}

void XTableView::setColumnFormat(const QString column, int format)
{
  setColumnRole(column, XSqlTableModel::FormatRole, QVariant(format));
}

void XTableView::setColumnLocked(const QString &pColname, bool pLocked)
{
  if (_columnByName.value(pColname))
    _columnByName.value(pColname)->locked = pLocked;
}

void XTableView::setColumnLocked(int pColumn, bool pLocked)
{
  ColumnProps *cp = _columnByName.value(columnNameFromLogicalIndex(pColumn), 0);
  if (cp)
    cp->locked = pLocked;
}

void XTableView::setColumnRole(const QString column, int role, const QVariant value)
{
  _model->setColumnRole(columnIndex(column), role, value);
}

void XTableView::setColumnTextAlignment(int column, int alignment)
{
  _model->setColumnRole(column, Qt::TextAlignmentRole, QVariant(alignment));
}

void XTableView::setColumnTextAlignment(const QString column, int alignment)
{
  setColumnRole(column, Qt::TextAlignmentRole, QVariant(alignment));
}

void XTableView::setColumnVisible(int pColumn, bool pVisible)
{
  if(pVisible)
    horizontalHeader()->showSection(pColumn);
  else
    horizontalHeader()->hideSection(pColumn);

  ColumnProps *cp = _columnByName.value(columnNameFromLogicalIndex(pColumn), 0);
  if (cp)
    cp->visible = pVisible;
}

void XTableView::setDataWidgetMap(XDataWidgetMapper* mapper)
{  
  setTable();
  _mapper=mapper; 
}

void XTableView::setItemDelegateForColumn(const QString column, QAbstractItemDelegate * delegate)
{
  setItemDelegateForColumn(columnIndex(column), delegate);
}

void XTableView::setFilter(const QString filter)
{  
  _model->setFilter(filter);
}

void XTableView::setForegroundColor(int row, int col, QString color)
{
  _model->setData(_model->index(row,col), namedColor(color), Qt::ForegroundRole);
}

void XTableView::setModel(XSqlTableModel * model)
{
  QTableView::setModel(model);

  for (int i = 0; i < QTableView::model()->columnCount(); ++i)
  {
    QString h = QTableView::model()->headerData(i, Qt::Horizontal,
                                               Qt::DisplayRole).toString();
    QTableView::model()->setHeaderData(i, Qt::Horizontal, h, Qt::UserRole);

    ColumnProps *cp = _columnByName.value(h, 0);

    if (! cp)
    {
      cp = new ColumnProps;
      cp->columnName   = h;
      cp->logicalIndex = i;
      cp->defaultWidth = -1;
      cp->savedWidth   = -1;
      cp->stretchy     = true;
      cp->locked       = false;
      cp->visible      = true;
      cp->label        = h;
      cp->fromSettings = false;
      cp->alignment    = QTableView::model()->headerData(i, Qt::Horizontal,
                                                Qt::TextAlignmentRole).toInt();

      cp->label.replace(0, 1, cp->label.left(1).toUpper());
      for (int j = 0; (j = cp->label.indexOf("_", j)) >= 0; /* empty */)
      {
        cp->label.replace(j,     1, " ");
        cp->label.replace(j + 1, 1, cp->label.mid(j + 1, 1).toUpper());
      }

      _columnByName.insert(cp->columnName, cp);
    }

    QTableView::model()->setHeaderData(i, Qt::Horizontal, cp->label,
                                      Qt::DisplayRole);
    QTableView::model()->setHeaderData(i, Qt::Horizontal, cp->alignment,
                                      Qt::TextAlignmentRole);
  }

  if (! _settingsLoaded)
  {
    QString settingsPrefix = _windowName + "/" + objectName();
    _settingsLoaded = true;

    _forgetful = xtsettingsValue(settingsPrefix + "/isForgetful").toBool();
    QString savedString;
    QStringList savedParts;
    QString part, key, val;
    bool b1 = false, b2 = false;
    if(!_forgetful)
    {
      savedString = xtsettingsValue(settingsPrefix + "/columnWidths").toString();
      savedParts = savedString.split("|", QString::SkipEmptyParts);
      for(int i = 0; i < savedParts.size(); i++)
      {
        part = savedParts.at(i);
        key = part.left(part.indexOf(","));
        val = part.right(part.length() - part.indexOf(",") - 1);
        b1 = false;
        b2 = false;
        int k = key.toInt(&b1);
        int v = val.toInt(&b2);
        if(b1 && b2)
        {
          ColumnProps *cp = _columnByName.value(columnNameFromLogicalIndex(k), 0);
          if (cp)
          {
            cp->savedWidth = v;
            cp->stretchy   = false;
            cp->fromSettings = true;
          }
          horizontalHeader()->resizeSection(k, v);
        }
      }
    }

    // Load any previously saved column hidden/visible information
    if(_x_preferences)
    {
      savedString = _x_preferences->value(settingsPrefix + "/columnsShown");
      savedParts = savedString.split("|", QString::SkipEmptyParts);
      for(int i = 0; i < savedParts.size(); i++)
      {
        part = savedParts.at(i);
        key = part.left(part.indexOf(","));
        val = part.right(part.length() - part.indexOf(",") - 1);
        int c = key.toInt(&b1);
        if(b1 && (val == "on" || val == "off"))
        {
          ColumnProps *cp = _columnByName.value(columnNameFromLogicalIndex(c), 0);
          if (cp)
          {
            cp->visible = (val == "on");
            cp->fromSettings = true;
          }
          horizontalHeader()->setSectionHidden(c, (val != "on"));
        }
      }
    }
  }
  emit newModel(model);
}

void XTableView::setRowForegroundColor(int row, QString color)
{
  for (int col = 0; col < _model->query().record().count(); col++)
    setForegroundColor(row, col, color);
}

void XTableView::setTable()
{
  if (_model->tableName() == _tableName)
    return;
      
  if (!_tableName.isEmpty())
  {
    QString tablename=_tableName;
    if (!_schemaName.isEmpty())
      tablename = _schemaName + "." + tablename;
    _model->setTable(tablename,_keyColumns);
    
    setModel(_model);
    setItemDelegate(new XItemDelegate(this));
  //  setRelations();
  }
}

void XTableView::setValue(int row, int column, QVariant value)
{
  _model->setData(_model->index(row,column), value);
}

void XTableView::sColumnSizeChanged(int logicalIndex, int /*oldSize*/, int newSize)
{
  ColumnProps *cp = _columnByName.value(columnNameFromLogicalIndex(logicalIndex), 0);
  if (cp)
  {
    cp->savedWidth = newSize;
    if (newSize < 0)
      cp->stretchy = true;
    else if (! _resizingInProcess && cp->stretchy)
      cp->stretchy = false;
  }

  if (_resizingInProcess)
    return;

  _resizingInProcess = true;

  int usedSpace = 0;
  QVector<int> stretch;

  for(int i = 0; i < horizontalHeader()->count(); i++)
  {
    ColumnProps *tmpcp = _columnByName.value(columnNameFromLogicalIndex(i), 0);

    if (horizontalHeader()->isSectionHidden(i))
      continue;
    else if (logicalIndex == i && newSize < 0)
      stretch.append(i);
    else if (logicalIndex == i)
      usedSpace += newSize;
    else if (tmpcp && ! tmpcp->stretchy)
      usedSpace += horizontalHeader()->sectionSize(i);
    else
      stretch.append(i);
  }

  if(stretch.size() > 0)
  {
    int leftover = (viewport()->width() - usedSpace) / stretch.size();

    if(leftover < 50)
      leftover = 50;

    for (int n = 0; n < stretch.size(); n++)
      horizontalHeader()->resizeSection(stretch.at(n), leftover);
  }

  _resizingInProcess = false;
}

void XTableView::sResetAllWidths()
{
  bool autoSections = false;
  for (int i = 0; i < horizontalHeader()->length(); i++)
  {
    ColumnProps *cp = _columnByName.value(columnNameFromLogicalIndex(i), 0);
    int width = cp ? cp->defaultWidth : -1;
    if (width >= 0)
      horizontalHeader()->resizeSection(i, width);
    else
    {
    if (cp)
        cp->stretchy = true;
      autoSections = true;
    }
  }
  if(autoSections)
    sColumnSizeChanged(-1, 0, 0);
}

void XTableView::sResetWidth()
{
  ColumnProps *cp = _columnByName.value(columnNameFromLogicalIndex(_resetWhichWidth), 0);

  int w = cp ? cp->defaultWidth : -1;

  if(w >= 0)
    horizontalHeader()->resizeSection(_resetWhichWidth, w);
  else
    sColumnSizeChanged(-1, 0, 0);
}

void XTableView::sShowMenu(const QPoint &pntThis)
{
  QModelIndex item = indexAt(pntThis);
  if (item.isValid())
  {
    _menu->clear();
    emit populateMenu(_menu, item);

    bool disableExport = FALSE;
    if(_x_preferences)
      disableExport = (_x_preferences->value("DisableExportContents")=="t");
    if(!disableExport)
      _menu->addAction(tr("Export Contents..."),  this, SLOT(sExport()));

    if(! _menu->isEmpty())
      _menu->popup(mapToGlobal(pntThis));
  }
}

void XTableView::sShowHeaderMenu(const QPoint &pntThis)
{
  _menu->clear();

  int logicalIndex = horizontalHeader()->logicalIndexAt(pntThis);
  int currentSize = horizontalHeader()->sectionSize(logicalIndex);
// If we have a default value and the current size is not equal to that default value
// then we want to show the menu items for resetting those values back to default
  ColumnProps *cp = _columnByName.value(columnNameFromLogicalIndex(logicalIndex), 0);
  if (cp && (cp->defaultWidth > 0) && (cp->defaultWidth != currentSize) )
  {
    _resetWhichWidth = logicalIndex;
    _menu->addAction(tr("Reset this Width"), this, SLOT(sResetWidth()));
  }

  _menu->addAction(tr("Reset all Widths"), this, SLOT(sResetAllWidths()));
  _menu->addSeparator();
  if(_forgetful)
    _menu->addAction(tr("Remember Widths"), this, SLOT(sToggleForgetfulness()));
  else
    _menu->addAction(tr("Do Not Remember Widths"), this, SLOT(sToggleForgetfulness()));

  _menu->addSeparator();

  for(int i = 0; i < horizontalHeader()->count(); i++)
  {
    QAction *act = _menu->addAction(QTableView::model()->headerData(i, Qt::Horizontal).toString());
    act->setCheckable(true);
    act->setChecked(! horizontalHeader()->isSectionHidden(i));

    ColumnProps *tmp = _columnByName.value(columnNameFromLogicalIndex(i), 0);
    act->setEnabled(tmp ?  ! tmp->locked : true);

    QMap<QString,QVariant> m;
    m.insert("command", QVariant("toggleColumnHidden"));
    m.insert("column", QVariant(i));
    act->setData(m);
    connect(_menu, SIGNAL(triggered(QAction*)), this, SLOT(popupMenuActionTriggered(QAction*)));
  }

  if(! _menu->isEmpty())
    _menu->popup(mapToGlobal(pntThis));
}

void XTableView::sToggleForgetfulness()
{
  _forgetful = !_forgetful;
}

bool XTableView::throwScriptException(const QString &message)
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

QVariant XTableView::value(int row, int column)
{
  return model()->data(model()->index(row,column));
}

