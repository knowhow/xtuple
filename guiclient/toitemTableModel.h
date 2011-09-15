/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef TOITEMTABLEMODEL_H
#define TOITEMTABLEMODEL_H

#include <QDate>
#include <QHash>
#include <QObject>
#include <QPair>
#include <QSqlError>
#include <QSqlRelationalTableModel>
#include <QString>

/*
 Represent as a Table Model either the entire TOITEM table, the TOITEMS for
 a single POHEAD_ID, or a single TOITEM row.

 This should probably be written as a an XAbstractTableModel that includes
 xTuple-generic concepts and PoitemTableModel as a subclass of that abstract
 class but we don't yet know what all of those abstractions should be.

 */

// these #defines have to match the order of fields in the select statement
#define TOHEAD_NUMBER			 0
#define ITEM_NUMBER_COL			 1
#define PRJ_NUMBER_COL			 2
#define EARLIESTDATE_COL		 3
#define TOITEM_ID_COL			 4
#define TOITEM_TOHEAD_ID_COL		 5
#define TOITEM_LINENUMBER_COL		 6
#define TOITEM_ITEM_ID_COL		 7
#define TOITEM_STATUS_COL		 8
#define TOITEM_DUEDATE_COL		 9
#define TOITEM_SCHEDSHIPDATE_COL	10
#define TOITEM_SCHEDRECVDATE_COL	11
#define TOITEM_QTY_ORDERED_COL		12
#define TOITEM_QTY_SHIPPED_COL		13
#define TOITEM_QTY_RECEIVED_COL		14
#define TOITEM_UOM_COL			15
#define TOITEM_STDCOST_COL		16
#define TOITEM_PRJ_ID_COL		17
#define TOITEM_FREIGHT_COL		18
#define TOITEM_FREIGHT_CURR_ID_COL	19
#define TOITEM_NOTES_COL                20
#define TOITEM_CLOSEDATE_COL		21
#define TOITEM_CLOSE_USERNAME_COL	22
#define TOITEM_LASTUPDATED_COL		23
#define TOITEM_CREATED_COL		24
#define TOITEM_CREATOR_COL		25
#define TOITEM_FREIGHT_RECEIVED_COL	26

class ToitemTableModel : public QSqlRelationalTableModel
{
  Q_OBJECT

  public:
    ToitemTableModel(QObject * parent = 0, QSqlDatabase = QSqlDatabase());

    virtual Qt::ItemFlags flags (const QModelIndex &) const;
    inline virtual int	  currId()    const { return _toheadcurrid; };
    inline virtual int	  headId()    const { return _toheadid; };
    inline virtual int	  itemId()    const { return _toitemid; };
    virtual bool	  isDirty()   const;
    virtual bool	  removeRow(int, const QModelIndex & = QModelIndex());
    inline virtual QDate  shipDate()  const { return _toshipdate; };
    inline virtual int    srcWhsId()  const { return _toheadsrcwhsid; };
    inline virtual QDate  transDate() const { return _toheaddate; };

  public slots:
    virtual bool	select();
    virtual void	setCurrId(const int pId = -1);
    virtual void	setHeadId(const int pId = -1);
    virtual void	setItemId(const int pId = -1);
    virtual void	setShipDate(const QDate);
    virtual void	setSrcWhsId(const int pId = -1);
    virtual void	setTransDate(const QDate);
    virtual bool	submitAll();

  protected:
    virtual bool	insertRowIntoTable(const QSqlRecord&);
    virtual QString	selectStatement() const;
    virtual bool	updateRowInTable(int, const QSqlRecord&);
    virtual bool	validRow(QSqlRecord&);

    QString		_selectStatement;

  protected slots:
    virtual void	markDirty(QModelIndex, QModelIndex);

  private:
    void	findHeadData();
    bool	_dirty;
    int		_toheadcurrid;
    QDate	_toheaddate;
    int		_toheadid;
    int		_toitemid;
    QDate	_toshipdate;
    int		_toheadsrcwhsid;
    QString	_tostatus;
};

#endif // TOITEMTABLEMODEL_H
