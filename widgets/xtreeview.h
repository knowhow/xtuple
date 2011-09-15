  /*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef XTREEVIEW_H
#define XTREEVIEW_H

#include <QItemDelegate>
#include <QMenu>
#include <QSqlRecord>
#include <QTreeView>

#include "xdatawidgetmapper.h"
#include "xsqltablemodel.h"
#include "widgets.h"

class XTUPLEWIDGETS_EXPORT XTreeView : public QTreeView
{
    Q_OBJECT
    Q_PROPERTY(QString         schemaName            READ schemaName           WRITE setSchemaName       )
    Q_PROPERTY(QString         tableName             READ tableName            WRITE setTableName        )
    Q_PROPERTY(int             primaryKeyCoulmns     READ primaryKeyColumns    WRITE setPrimaryKeyColumns)
    
    public:
      XTreeView(QWidget *parent = 0);
      ~XTreeView();

      Q_INVOKABLE bool    isRowHidden(int row);
                  int     primaryKeyColumns() const { return _keyColumns; }
                  QString schemaName()        const { return _schemaName; }
                  QString tableName()         const { return _tableName;  }

                  bool    throwScriptException(const QString &message);

      Q_INVOKABLE int columnIndex(const QString column);
      Q_INVOKABLE      QString columnNameFromLogicalIndex(const int logicalIndex) const;
      Q_INVOKABLE         void setColumn(const QString &label, int width, int alignment, bool visible, const QString &colname);
      Q_INVOKABLE virtual void setColumnLocked(const QString &pColname, bool pLocked);
      Q_INVOKABLE virtual void setColumnLocked(const int      pColumn, bool pLocked);
      Q_INVOKABLE virtual void setColumnRole(const QString column, int role, QVariant value);
      Q_INVOKABLE virtual void setColumnVisible(int, bool);
      Q_INVOKABLE         void setTable();
      Q_INVOKABLE XDataWidgetMapper *mapper()  { return _mapper;}
      Q_INVOKABLE XSqlTableModel    *model()   { return _model;}
      virtual bool ignoreGeometrySizing() { return _ignoreSizing; }
      virtual void setIgnoreGeometrySizing(bool ignore) { _ignoreSizing = ignore; }
      virtual void setGeometry( int x, int y, int w, int h );
            
    public slots:
      virtual int  currentIndex();
      virtual int  rowCount();
      virtual int  rowCountVisible();
      virtual QString filter();
      virtual QVariant value(int row, int column);
      virtual QVariant selectedValue(int column); 
      virtual void insert();
      virtual void populate(int p);
      virtual void removeSelected();
      virtual void revertAll(); 
      virtual void save();
      virtual void select();
      virtual void selectRow(int index);
      virtual void setColumnEditor(int column, const QString editor);
      virtual void setColumnEditor(const QString column, const QString editor);
      virtual void setColumnFormat(const QString column, int format);
      virtual void setColumnTextAlignment(int column, int alignment);
      virtual void setColumnTextAlignment(const QString column, int alignment);
      virtual void setDataWidgetMap(XDataWidgetMapper* mapper);
      virtual void setFilter(const QString filter);
      virtual void setForegroundColor(int row, int col, QString color);
      virtual void setModel(XSqlTableModel* model=0);
      virtual void setPrimaryKeyColumns(int p)                { _keyColumns = p;            }
      virtual void setRowForegroundColor(int row, QString color);
      virtual void setSchemaName(QString p)                   { _schemaName = p;            }
      virtual void setTableName(QString p)                    { _tableName = p;             }
      virtual void setValue(int row, int column, QVariant value);
      virtual void sShowMenu(const QPoint &);

    signals:
      void  dataChanged(int row, int col);
      void  newModel(XSqlTableModel *model);
      void  rowSelected(int);
      void  valid(bool);
      void  saved();
      void  populateMenu(QMenu *, QModelIndex);
      
    protected:
      virtual void resizeEvent(QResizeEvent*);
      virtual void selectionChanged(const QItemSelection & selected, const QItemSelection & deselected);

    private slots:
      void handleDataChanged(const QModelIndex topLeft, const QModelIndex lowerRight);
      void popupMenuActionTriggered(QAction*);
      void sColumnSizeChanged(int, int, int);
      void sResetAllWidths();
      void sResetWidth();
      void sShowHeaderMenu(const QPoint &);
      void sToggleForgetfulness();

    private:
      bool                 _forgetful;
      bool                 _ignoreSizing;
      QSqlRecord           _idx;
      int                  _keyColumns;
      XDataWidgetMapper   *_mapper;
      QMenu               *_menu;
      XSqlTableModel      *_model;
      int                  _resetWhichWidth;
      bool                 _resizingInProcess;
      QString              _schemaName;
      bool                 _settingsLoaded;
      QString              _tableName;
      QString              _windowName;

      struct ColumnProps { 
        QString columnName;
        int     logicalIndex;
        int     defaultWidth;
        int     savedWidth;
        bool    stretchy;
        bool    locked;
        bool    visible;
        QString label;
        int     alignment;
        bool    fromSettings;
      };
      QMap<QString, ColumnProps*> _columnByName;
};

#endif
