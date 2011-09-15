/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef POITEMTABLEMODEL_H
#define POITEMTABLEMODEL_H

#include <QDate>
#include <QHash>
#include <QObject>
#include <QPair>
#include <QSqlError>
#include <QSqlRelationalTableModel>
#include <QString>

/*
 Represent as a Table Model either the entire POITEM table, the POITEMS for
 a single POHEAD_ID, or a single POITEM row.

 This should probably be written as a an XAbstractTableModel that includes
 xTuple-generic concepts and PoitemTableModel as a subclass of that abstract
 class but we don't yet know what all of those abstractions should be.

 */

// these #defines have to match the order of fields in the select statement
#define POHEAD_NUMBER			 0
#define ITEM_ID_COL			 1
#define ITEM_NUMBER_COL			 2
#define WAREHOUS_ID_COL			 3
#define WAREHOUS_CODE_COL		 4
#define EXTPRICE_COL			 5
#define PRJ_NUMBER_COL			 6
#define EXPCAT_CODE_COL			 7
#define ITEMSRC_MINORDQTY_COL		 8
#define ITEMSRC_MULTORDQTY_COL		 9
#define ITEMSRC_INVVENDORUOMRATIO_COL	10
#define EARLIESTDATE_COL		11
#define POITEM_ID_COL			12
#define POITEM_STATUS_COL		13
#define POITEM_POHEAD_ID_COL		14
#define POITEM_LINENUMBER_COL		15
#define POITEM_DUEDATE_COL		16
#define POITEM_WOHEAD_ID		17
#define POITEM_ITEMSITE_ID_COL		18
#define POITEM_VEND_ITEM_DESCRIP_COL	19
#define POITEM_VEND_UOM_COL		20
#define POITEM_INVVENDUOMRATIO_COL	21
#define POITEM_QTY_ORDERED_COL		22
#define POITEM_QTY_RECEIVED_COL		23
#define POITEM_QTY_RETURNED_COL		24
#define POITEM_QTY_VOUCHERED_COL	25
#define POITEM_UNITPRICE_COL		26
#define POITEM_VEND_ITEM_NUMBER_COL	27
#define POITEM_COMMENTS_COL		28
#define POITEM_QTY_TORECEIVE_COL	39
#define POITEM_EXPCAT_ID_COL		30
#define POITEM_ITEMSRC_ID_COL		31
#define POITEM_FREIGHT_COL		32
#define POITEM_FREIGHT_RECEIVED_COL	33
#define POITEM_FREIGHT_VOUCHERED_COL	34
#define POITEM_SOITEM_ID_COL		35
#define POITEM_PRJ_ID_COL		36
#define POITEM_STDCOST_COL		37
#define POITEM_BOM_REV_ID_COL           38
#define POITEIM_BOO_REV_ID_COL          39
#define POITEM_MANUF_NAME_COL           40
#define POITEM_MANUF_ITEM_NUMBER_COL    41
#define POITEM_MANUF_ITEM_DESCRIP_COL   42
#define POITEM_TAXTYPE_ID_COL           43
#define POITEM_TAX_RECOVERABLE_COL      44

class PoitemTableModel : public QSqlRelationalTableModel
{
  Q_OBJECT

  public:
    PoitemTableModel(QObject * parent = 0, QSqlDatabase = QSqlDatabase());

    virtual Qt::ItemFlags flags (const QModelIndex &) const;
    inline virtual int	  currId() const {return _poheadcurrid; };
    inline virtual int	  headId() const { return _poheadid; };
    inline virtual int	  itemId() const { return _poitemid; };
    virtual bool	  isDirty() const;
    virtual bool	  removeRow(int, const QModelIndex & = QModelIndex());
    inline virtual QDate  transDate() const {return _poheaddate; };
    int		_vendid;
    bool	_vendrestrictpurch;

  public slots:
    virtual bool	select();
    virtual void	setCurrId(const int pId = -1);
    virtual void	setHeadId(const int pId = -1);
    virtual void	setItemId(const int pId = -1);
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
    int		_poheadcurrid;
    QDate	_poheaddate;
    int		_poheadid;
    int		_poitemid;
    QString	_postatus;
};

#endif // POITEMTABLEMODEL_H
