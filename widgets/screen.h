/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef SCREEN_H
#define SCREEN_H

#include "widgets.h"
#include "xdatawidgetmapper.h"
#include "xsqltablemodel.h"
#include <xsqlquery.h>

#include <QWidget>
#include <QSqlDatabase>
#include <QSqlIndex>
#include <QList>

class XTUPLEWIDGETS_EXPORT Screen : public QWidget
{
	Q_OBJECT
		
	Q_ENUMS(Modes)
	Q_ENUMS(SearchTypes)
        Q_ENUMS(Disposition)
	
	Q_PROPERTY (QString		schemaName            READ schemaName		WRITE setSchemaName)
	Q_PROPERTY (QString 		tableName             READ tableName 		WRITE setTableName)
        Q_PROPERTY (int                 primaryKeyColumns     READ primaryKeyColumns    WRITE setPrimaryKeyColumns)
	Q_PROPERTY (Modes		mode                  READ mode                 WRITE setMode)              
        Q_PROPERTY (bool                lockRecords           READ lockRecords          WRITE setLockRecords);

	public:
		Screen(QWidget * = 0);
                ~Screen();
		
                bool throwScriptException(const QString &message);
		enum Modes { New, Edit, View };
                enum Disposition { NoChanges, Save, Cancel };
		Modes mode();

                Q_INVOKABLE bool submit();
                Q_INVOKABLE bool locked()                { return _locked;}
                Q_INVOKABLE bool lockRecords()           { return _lock;}
                Q_INVOKABLE Disposition check();
                Q_INVOKABLE XDataWidgetMapper *mapper()  { return _mapper;}
                Q_INVOKABLE XSqlTableModel    *model()   { return _model;}
                
                void showEvent ( QShowEvent * event );
       	
	public slots:
                bool cancel();
                bool isDirty();
                bool tryLock();
                int  currentIndex();
                int  primaryKeyColumns()    const { return _keyColumns;       }
                QString filter()            const { return _model->filter();  }
                QString schemaName()        const { return _schemaName;       }
                QString sortColumn()        const { return _sortColumn;       }
                QString tableName()         const { return _tableName;        }
                void clear();
                void deleteCurrent();
                void insert();
                void loadAll();
                void newMappedWidget(QWidget *widget);
                void removeCurrent();
                void removeRows(int row, int count);
                void revert();
                void revertAll();
                void revertRow(int row);
		void toNext();
		void toPrevious();
		void save();
		void search(QString criteria);
		void select();
                void setCurrentIndex(int index);
                void setFilter(QString filter)          { _model->setFilter(filter);      }
                void setLockRecords(bool lock);
                void setModel(XSqlTableModel *model);
		void setMode(Modes p);
                void setPrimaryKeyColumns(int count)    { _keyColumns = count;           }
		void setSchemaName(QString schema);
		void setSortColumn(QString p);
		void setTableName(QString table);
		void setTable(QString schema, QString table);
		void setDataWidgetMapper(XSqlTableModel *model);
                void setWidgetsEnabled(bool enabled);
                void unlock();
	
	signals:
                void currentIndexChanged(int index);
                void lockGranted(bool);
		void newDataWidgetMapper(XDataWidgetMapper *mapper);
                void newModel(XSqlTableModel *model);
		void saved();
                void revertedAll();
                void reverted(int index);

	private:
                bool                    _shown;
		enum  Modes		_mode;
                int                     _keyColumns;
                bool                    _lock;
                bool                    _locked;
                qint64                  _key;
		QString			_schemaName;
                QString                 _sortColumn;
		QString			_tableName;
		XDataWidgetMapper*	_mapper;
                XSqlTableModel*		_model;
};

#endif // SCREEN_H
