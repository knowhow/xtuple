/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef __XTREEWIDGET_H__
#define __XTREEWIDGET_H__

#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QVariant>
#include <QVector>
#include <QTimer>
#include <QHeaderView> //#13251

#include "widgets.h"
#include "guiclientinterface.h"
#include "xt.h"

//  Table Column Widths
#define _itemColumn     100
#define _whsColumn      35
#define _userColumn     80
#define _dateColumn     80
#define _timeDateColumn 160
#define _timeColumn     80
#define _qtyColumn      80
#define _priceColumn    60
#define _moneyColumn    60
#define _bigMoneyColumn 100
#define _costColumn     70
#define _prcntColumn    55
#define _transColumn    35
#define _uomColumn      45
#define _orderColumn    60
#define _statusColumn   45
#define _seqColumn      40
#define _ynColumn       45
#define _docTypeColumn  80
#define _currencyColumn 80

// TODO: would these be better placed in an enum in xt.h?
#define ROWROLE_INDENT        0
#define ROWROLE_HIDDEN        1
#define ROWROLE_DELETED       2
// make sure ROWROLE_COUNT = last ROWROLE + 1
#define ROWROLE_COUNT         3

#include "xsqlquery.h"

class QAction;
class QMenu;
class QScriptEngine;
class XTreeWidget;
class XTreeWidgetProgress;

void  setupXTreeWidgetItem(QScriptEngine *engine);
void  setupXTreeWidget(QScriptEngine *engine);

class XTUPLEWIDGETS_EXPORT XTreeWidgetItem : public QObject, public QTreeWidgetItem
{
  Q_OBJECT

  friend class XTreeWidget;

  public:
    XTreeWidgetItem(XTreeWidgetItem *, int, QVariant = QVariant(),
                    QVariant = QVariant(), QVariant = QVariant(),
                    QVariant = QVariant(), QVariant = QVariant(),
                    QVariant = QVariant(), QVariant = QVariant(),
                    QVariant = QVariant(), QVariant = QVariant(),
                    QVariant = QVariant(), QVariant = QVariant() );
    XTreeWidgetItem(XTreeWidgetItem *, int, int, QVariant = QVariant(),
                    QVariant = QVariant(), QVariant = QVariant(),
                    QVariant = QVariant(), QVariant = QVariant(),
                    QVariant = QVariant(), QVariant = QVariant(),
                    QVariant = QVariant(), QVariant = QVariant(),
                    QVariant = QVariant(), QVariant = QVariant() );
    XTreeWidgetItem(XTreeWidget *, int, QVariant = QVariant(),
                    QVariant = QVariant(), QVariant = QVariant(),
                    QVariant = QVariant(), QVariant = QVariant(),
                    QVariant = QVariant(), QVariant = QVariant(),
                    QVariant = QVariant(), QVariant = QVariant(),
                    QVariant = QVariant(), QVariant = QVariant() );
    XTreeWidgetItem(XTreeWidget *, int, int, QVariant = QVariant(),
                    QVariant = QVariant(), QVariant = QVariant(),
                    QVariant = QVariant(), QVariant = QVariant(),
                    QVariant = QVariant(), QVariant = QVariant(),
                    QVariant = QVariant(), QVariant = QVariant(),
                    QVariant = QVariant(), QVariant = QVariant() );
    XTreeWidgetItem(XTreeWidget *, XTreeWidgetItem *, int, QVariant = QVariant(),
                    QVariant = QVariant(), QVariant = QVariant(),
                    QVariant = QVariant(), QVariant = QVariant(),
                    QVariant = QVariant(), QVariant = QVariant(),
                    QVariant = QVariant(), QVariant = QVariant(),
                    QVariant = QVariant(), QVariant = QVariant() );
    XTreeWidgetItem(XTreeWidget *, XTreeWidgetItem *, int, int, QVariant = QVariant(),
                    QVariant = QVariant(), QVariant = QVariant(),
                    QVariant = QVariant(), QVariant = QVariant(),
                    QVariant = QVariant(), QVariant = QVariant(),
                    QVariant = QVariant(), QVariant = QVariant(),
                    QVariant = QVariant(), QVariant = QVariant() );
    XTreeWidgetItem(XTreeWidgetItem *, XTreeWidgetItem *, int, QVariant = QVariant(),
                    QVariant = QVariant(), QVariant = QVariant(),
                    QVariant = QVariant(), QVariant = QVariant(),
                    QVariant = QVariant(), QVariant = QVariant(),
                    QVariant = QVariant(), QVariant = QVariant(),
                    QVariant = QVariant(), QVariant = QVariant() );
    XTreeWidgetItem(XTreeWidgetItem *, XTreeWidgetItem *, int, int, QVariant = QVariant(),
                    QVariant = QVariant(), QVariant = QVariant(),
                    QVariant = QVariant(), QVariant = QVariant(),
                    QVariant = QVariant(), QVariant = QVariant(),
                    QVariant = QVariant(), QVariant = QVariant(),
                    QVariant = QVariant(), QVariant = QVariant() );

    Q_INVOKABLE virtual void    setText(int, const QVariant&);
    Q_INVOKABLE virtual QString text(int p) const { return QTreeWidgetItem::text(p); }
    Q_INVOKABLE virtual QString text(const QString&) const;
    Q_INVOKABLE inline void     setTextColor(int column, const QColor &color) { QTreeWidgetItem::setTextColor(column, color); }
    Q_INVOKABLE void                    setTextColor(const QColor&);

    Q_INVOKABLE inline int              id() const        { return _id;    }
    Q_INVOKABLE inline int              altId() const     { return _altId; }
    Q_INVOKABLE inline void             setId(int pId)    { _id = pId;     }
    Q_INVOKABLE inline void             setAltId(int pId) { _altId = pId;  }

    Q_INVOKABLE inline QVariant         data(int colidx,    int role) const { return QTreeWidgetItem::data(colidx, role); }
    Q_INVOKABLE inline void             setData(int colidx, int role, const QVariant &val) { QTreeWidgetItem::setData(colidx, role, val); }
    Q_INVOKABLE virtual QVariant        rawValue(const QString colname);
    Q_INVOKABLE virtual int             id(const QString);

    virtual bool operator               <(const XTreeWidgetItem &other) const;
    virtual bool operator               ==(const XTreeWidgetItem &other) const;

    Q_INVOKABLE inline XTreeWidgetItem  *child(int idx) const
    {
      QTreeWidgetItem *item = QTreeWidgetItem::child(idx);
      return ((XTreeWidgetItem *)item);
    }

    virtual QString toString() const;

  protected:
    virtual double totalForItem(const int, const int) const;

  private:
    void constructor( int, int, QVariant, QVariant, QVariant,
                      QVariant, QVariant, QVariant, QVariant,
                      QVariant, QVariant, QVariant, QVariant );

    int _id;
    int _altId;
};

Q_DECLARE_METATYPE(XTreeWidgetItem *)
// Q_DECLARE_METATYPE(XTreeWidgetItem)

class XTreeWidgetPopulateParams;

class XTUPLEWIDGETS_EXPORT XTreeWidget : public QTreeWidget
{
  Q_OBJECT Q_PROPERTY(QString dragString READ dragString WRITE setDragString)
  Q_PROPERTY( QString altDragString READ altDragString WRITE setAltDragString)
  Q_PROPERTY( bool populateLinear READ populateLinear WRITE setPopulateLinear)

  Q_ENUMS(PopulateStyle)

  public :
    enum PopulateStyle { Replace, Append };
    XTreeWidget(QWidget *);
    ~XTreeWidget();

    Q_INVOKABLE void  populate(XSqlQuery, bool = FALSE, PopulateStyle = Replace);
    Q_INVOKABLE void  populate(XSqlQuery, int, bool = FALSE, PopulateStyle = Replace);
    void    populate(const QString&, bool = FALSE);
    void    populate(const QString&, int, bool = FALSE);

    QString dragString() const;
    void    setDragString(QString);
    QString altDragString() const;
    void    setAltDragString(QString);
    bool    populateLinear();
    void    setPopulateLinear(bool alwaysLinear = true);

    Q_INVOKABLE int   altId() const;
    Q_INVOKABLE int   id()    const;
    Q_INVOKABLE int   id(const QString)     const;
    Q_INVOKABLE void  setId(int pId, bool pClear = true);
    Q_INVOKABLE void  setId(int pId,int pAltId, bool pClear = true);

    Q_INVOKABLE virtual int column(const QString) const;
    Q_INVOKABLE virtual XTreeWidgetItem *currentItem()         const;
    Q_INVOKABLE virtual void            setColumnCount(int columns);
    Q_INVOKABLE virtual void            setColumnLocked(int, bool);
    Q_INVOKABLE virtual void            setColumnVisible(int, bool);
    Q_INVOKABLE virtual void            sortItems(int column, Qt::SortOrder order);
    Q_INVOKABLE virtual XTreeWidgetItem *topLevelItem(int idx) const;

    Q_INVOKABLE XTreeWidgetItem         *findXTreeWidgetItemWithId(const XTreeWidget *ptree, const int pid);
    Q_INVOKABLE XTreeWidgetItem         *findXTreeWidgetItemWithId(const XTreeWidgetItem *ptreeitem, const int pid);

    Q_INVOKABLE QString toTxt() const;
    Q_INVOKABLE QString toCsv() const;
    Q_INVOKABLE QString toHtml() const;

    // just for scripting exposure:
    Q_INVOKABLE inline void addTopLevelItem(XTreeWidgetItem *item) {        QTreeWidget::addTopLevelItem(item); }
    Q_INVOKABLE void        addTopLevelItems(const QList<XTreeWidgetItem *> &items);
    Q_INVOKABLE inline void closePersistentEditor(XTreeWidgetItem *item, int column = 0) {        QTreeWidget::closePersistentEditor(item, column); }
    Q_INVOKABLE inline int  columnCount() const   { return QTreeWidget::columnCount(); }
    Q_INVOKABLE inline int  currentColumn() const { return QTreeWidget::currentColumn(); }
    Q_INVOKABLE inline void editItem(XTreeWidgetItem *item, int column = 0) {        QTreeWidget::editItem(item, column); }
    Q_INVOKABLE QList<XTreeWidgetItem *>  findItems(const QString &text, Qt::MatchFlags flags, int column = 0, int role = 0) const;
    Q_INVOKABLE inline QTreeWidgetItem    *headerItem() const { return QTreeWidget::headerItem(); }
    Q_INVOKABLE inline int                indexOfTopLevelItem(XTreeWidgetItem *item) const { return QTreeWidget::indexOfTopLevelItem(item); }
    Q_INVOKABLE inline void               insertTopLevelItem(int index, XTreeWidgetItem *item) {        QTreeWidget::insertTopLevelItem(index, item); }
    Q_INVOKABLE void                      insertTopLevelItems(int index, const QList<XTreeWidgetItem *> &items);
    Q_INVOKABLE XTreeWidgetItem           *invisibleRootItem() const;
    Q_INVOKABLE inline bool               isFirstItemColumnSpanned(const XTreeWidgetItem *item) const { return QTreeWidget::isFirstItemColumnSpanned(item); }
    Q_INVOKABLE XTreeWidgetItem           *itemAbove(const XTreeWidgetItem *item) const;
    Q_INVOKABLE inline QTreeWidgetItem    *itemAt(const QPoint &p) const  { return QTreeWidget::itemAt(p); }
    Q_INVOKABLE inline QTreeWidgetItem    *itemAt(int x, int y) const     { return QTreeWidget::itemAt(x, y); }
    Q_INVOKABLE inline QTreeWidgetItem    *itemBelow(const XTreeWidgetItem *item) const         { return QTreeWidget::itemBelow(item); }
    Q_INVOKABLE inline QWidget            *itemWidget(XTreeWidgetItem *item, int column) const  { return QTreeWidget::itemWidget(item, column); }
    Q_INVOKABLE inline void               openPersistentEditor(XTreeWidgetItem *item, int column = 0)       {        QTreeWidget::openPersistentEditor(item, column); }
    Q_INVOKABLE QVariant                  rawValue(const QString colname) const;
    Q_INVOKABLE inline void               removeItemWidget(XTreeWidgetItem *item, int column)               {        QTreeWidget::removeItemWidget(item, column); }
    Q_INVOKABLE QList<XTreeWidgetItem *>  selectedItems() const;
    Q_INVOKABLE inline void               setCurrentItem(XTreeWidgetItem *item)                             {        QTreeWidget::setCurrentItem(item); }
    Q_INVOKABLE inline void               setCurrentItem(XTreeWidgetItem *item, int column)                 {        QTreeWidget::setCurrentItem(item, column); }
    Q_INVOKABLE inline void               setCurrentItem(XTreeWidgetItem *item, int column, QItemSelectionModel::SelectionFlags command) { QTreeWidget::setCurrentItem(item, column, command); }
    Q_INVOKABLE inline void               setFirstItemColumnSpanned(const XTreeWidgetItem *item, bool span) {        QTreeWidget::setFirstItemColumnSpanned(item, span); }
    Q_INVOKABLE inline void               setHeaderItem(XTreeWidgetItem *item)                              {        QTreeWidget::setHeaderItem(item); }
    Q_INVOKABLE inline void               setHeaderLabel(const QString &label)                              {        QTreeWidget::setHeaderLabel(label); }
    Q_INVOKABLE inline void               setHeaderLabels(const QStringList &labels)                        {        QTreeWidget::setHeaderLabels(labels); }
    Q_INVOKABLE inline void               setItemWidget(XTreeWidgetItem *item, int column, QWidget *widget) {        QTreeWidget::setItemWidget(item, column, widget); }
    Q_INVOKABLE inline int                sortColumn() const { return QTreeWidget::sortColumn(); }
    Q_INVOKABLE inline QTreeWidgetItem    *takeTopLevelItem(int index)                                      { return QTreeWidget::takeTopLevelItem(index); }
    Q_INVOKABLE inline int                topLevelItemCount() const { return QTreeWidget::topLevelItemCount(); }
    Q_INVOKABLE inline QRect              visualItemRect(const XTreeWidgetItem *item) const                 { return QTreeWidget::visualItemRect(item); }
	Q_INVOKABLE inline void               moveColumn(int from, int to)                                      { header()->moveSection(from, to); } //#13251
    // end of scripting exposure

    static GuiClientInterface *_guiClientInterface;

  public slots:
    void  addColumn(const QString&, int, int, bool = true, const QString = QString(), const QString = QString(), const int scale = 0);
    void  clear();
    void  hideColumn(int colnum)  { QTreeWidget::hideColumn(colnum); };
    void  hideColumn(const QString&);
    void  showColumn(int colnum)  { QTreeWidget::showColumn(colnum); };
    void  showColumn(const QString&);
    void  sExport();
    void  sCopyVisibleToClipboard();
    void  sCopyRowToClipboard();
    void  sCopyCellToClipboard();
    void  sSearch(const QString&);

  signals:
    void  valid(bool);
    void  newId(int);
    void  currentItemChanged(XTreeWidgetItem *, XTreeWidgetItem *);
    void  itemActivated(XTreeWidgetItem *item, int column);
    void  itemChanged(XTreeWidgetItem *item, int column);
    void  itemClicked(XTreeWidgetItem *item, int column);
    void  itemCollapsed(XTreeWidgetItem *item);
    void  itemDoubleClicked(XTreeWidgetItem *item, int column);
    void  itemEntered(XTreeWidgetItem *item, int column);
    void  itemExpanded(XTreeWidgetItem *item);
    void  itemPressed(XTreeWidgetItem *item, int column);
    void  itemSelected(int);
    void  populateMenu(QMenu *, QTreeWidgetItem *);
    void  populateMenu(QMenu *, QTreeWidgetItem *, int);
    void  populateMenu(QMenu *, XTreeWidgetItem *);
    void  populateMenu(QMenu *, XTreeWidgetItem *, int);
    void  resorted();
    void  populated();

  protected slots:
    void  sHeaderClicked(int);
    void  populateCalculatedColumns();
    void  sCurrentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous);
    void  sItemActivated(QTreeWidgetItem *item, int column);
    void  sItemChanged(QTreeWidgetItem *item, int column);
    void  sItemClicked(QTreeWidgetItem *item, int column);
    void  sItemCollapsed(QTreeWidgetItem *item);
    void  sItemDoubleClicked(QTreeWidgetItem *item, int column);
    void  sItemEntered(QTreeWidgetItem *item, int column);
    void  sItemExpanded(QTreeWidgetItem *item);
    void  sItemPressed(QTreeWidgetItem *item, int column);
    void  populateWorker();

  protected:
    QPoint        dragStartPosition;

    virtual void  mousePressEvent(QMouseEvent *);
    virtual void  mouseMoveEvent(QMouseEvent *);
    virtual void  resizeEvent(QResizeEvent *);

  private:
    QString _dragString;
    QString _altDragString;
    QMenu   *_menu;
    QMap<int, int>  _defaultColumnWidths;
    QMap<int, int>  _savedColumnWidths;
    QMap<int, bool> _savedVisibleColumns;
    QMap<int, QVariantMap *>     _roles;
    QList<int>      _lockedColumns;
    QVector<int>    _stretch;
    bool          _resizingInProcess;
    bool          _forgetful;
    bool          _forgetfulOrder;
    bool          _savedId;
    bool          _settingsLoaded;
    QString       _settingsName;
    int           _resetWhichWidth;
    int           _scol;
    Qt::SortOrder _sord;
    static void   loadLocale();
    QList<XTreeWidgetPopulateParams> _workingParams;
    QTimer        _workingTimer;
    bool          _alwaysLinear;
    bool          _linear;

    QVector<int>    *_colIdx;
    QVector<int *>  *_colRole;
    int              _fieldCount;
    XTreeWidgetItem *_last;
    int              _rowRole[ROWROLE_COUNT];
    void             cleanupAfterPopulate();
    XTreeWidgetProgress *_progress;
    QList<QMap<int, double> *> *_subtotals;

  private slots:
    void  sSelectionChanged();
    void  sItemSelected();
    void  sItemSelected(QTreeWidgetItem *, int);
    void  sShowMenu();
    void  sShowMenu(const QPoint&);
    void  sShowHeaderMenu(const QPoint&);
    // void sStartDrag(QTreeWidgetItem *, int);
    void  sColumnSizeChanged(int, int, int);
    void  sResetWidth();
    void  sResetAllWidths();
    void  sToggleForgetfulness();
    void  sToggleForgetfulnessOrder();
    void  popupMenuActionTriggered(QAction *);
};

class XTreeWidgetPopulateParams
{
  public:
    XSqlQuery _workingQuery;
    int       _workingIndex;
    bool      _workingUseAlt;
    XTreeWidget::PopulateStyle _workingPopstyle;
};

#endif
