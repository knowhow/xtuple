/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef itemCluster_h
#define itemCluster_h

#include "virtualCluster.h"
#include "qstringlist.h"
#include <parameter.h>

#include <QItemDelegate>
#include <QStyleOptionViewItem>

#include "virtualCluster.h"

class ParameterList;

class QPushButton;
class QLabel;
class QDragEnterEvent;
class QDropEvent;
class QMouseEvent;
class ItemLineEditDelegate;
class itemList;
class itemSearch;

class XTUPLEWIDGETS_EXPORT itemList : public VirtualList
{
    Q_OBJECT

public:
    itemList(QWidget*, Qt::WindowFlags = 0);

    QCheckBox* _showInactive;
    QCheckBox* _showMake;
    QCheckBox* _showBuy;

public slots:
    virtual void set(const ParameterList & pParams);
    virtual void reject();
    virtual void sSearch( const QString & pTarget );
    virtual void sFillList();

private:
    int _itemid;
    unsigned int _itemType;
    bool _useQuery;
    QString _sql;
    QStringList _extraClauses;
};

class XTUPLEWIDGETS_EXPORT itemSearch : public VirtualSearch
{
    Q_OBJECT

public:
    itemSearch(QWidget*, Qt::WindowFlags = 0);

    XCheckBox* _searchDescrip2;
    XCheckBox* _searchUpc;
    XCheckBox* _showInactive;

public slots:
    void set(const ParameterList & pParams);
    void sFillList();

private:
    int _itemid;
    unsigned int _itemType;
    bool _useQuery;
    QString _sql;
    QStringList _extraClauses;
};

class XTUPLEWIDGETS_EXPORT ItemLineEdit : public VirtualClusterLineEdit
{
  Q_OBJECT
  Q_PROPERTY(unsigned int type           READ type          WRITE setType       DESIGNABLE false)

friend class ItemCluster;
friend class ItemLineEditDelegate;

  public:
    ItemLineEdit(QWidget *, const char * = 0);

    enum Type {
      cUndefined           = 0x00,

      // Specific Item Types
      cPurchased           = 0x00000001,
      cManufactured        = 0x00000002,
      cPhantom             = 0x00000004,
      cBreeder             = 0x00000008,
      cCoProduct           = 0x00000010,
      cByProduct           = 0x00000020,
      cReference           = 0x00000040,
      cCosting             = 0x00000080,
      cTooling             = 0x00000100,
      cOutsideProcess      = 0x00000200,
      cPlanning            = 0x00000400,
      cKit                 = 0x00001000,

      // The first 16 bits are reserved for individual item types and we
      // have this mask defined here for convenience.
      cAllItemTypes_Mask   = 0x0000FFFF,

      // Planning Systems
      // TODO
      // Planning Type moved to Itemsite, don't think this is used
       cPlanningMRP         = 0x00100000,
       cPlanningMPS         = 0x00200000,
       cPlanningNone        = 0x00400000,

       cPlanningAny         = cPlanningMRP | cPlanningMPS | cPlanningNone,

      // Misc. Options
      cItemActive          = 0x04000000,
      cSold                = 0x08000000,

      // Lot/Serial and Location Controlled
      cLocationControlled  = 0x10000000,
      cLotSerialControlled = 0x20000000,
      cDefaultLocation     = 0x40000000,

      // Active ItemSite
      cActive         = 0x80000000,
      
      // Groups of Item Types
      cGeneralManufactured = cManufactured | cBreeder,
      cGeneralPurchased    = cPurchased | cOutsideProcess | cTooling,
      cGeneralComponents   = cManufactured | cPhantom | cCoProduct | cPurchased | cOutsideProcess | cReference | cTooling,
      cGeneralInventory    = cAllItemTypes_Mask ^ cReference,
      cKitComponents       = cSold | (cAllItemTypes_Mask ^ cKit)
    };

    Q_INVOKABLE inline unsigned int type() const                   { return _type;                        }
    Q_INVOKABLE inline void setType(unsigned int pType)            { _type = pType; _defaultType = pType; } 
    Q_INVOKABLE inline void setDefaultType(unsigned int pType)     { _defaultType = pType; } 
    Q_INVOKABLE inline void setQuery(const QString &pSql) { _sql = pSql; _useQuery = TRUE; }
    Q_INVOKABLE inline void setValidationQuery(const QString &pSql) { _validationSql = pSql; _useValidationQuery = TRUE; }
    Q_INVOKABLE inline int queryUsed() const              { return _useQuery;              }
    Q_INVOKABLE inline int validationQueryUsed() const    { return _useValidationQuery;    }

    Q_INVOKABLE void addExtraClause(const QString &);
    Q_INVOKABLE inline QStringList getExtraClauseList() const { return _extraClauses; }
    Q_INVOKABLE inline void clearExtraClauseList() { _extraClauses.clear(); }

    Q_INVOKABLE ItemLineEditDelegate* itemDelegate() { return _delegate;}
    Q_INVOKABLE QString itemNumber();
    Q_INVOKABLE QString uom();
    Q_INVOKABLE QString upc();
    Q_INVOKABLE QString itemType();
    Q_INVOKABLE bool    isConfigured();

  public slots:
    void sHandleCompleter();
    void sInfo();
    void sList();
    void sSearch();
    void sSearch(ParameterList params);
    void sAlias();

    void silentSetId(const int);
    void setId(const int);
    void setItemNumber(const QString& pNumber); // Legacy support
    void setItemsiteid(int);
    void setNumber(const QString& pNumber) { setItemNumber(pNumber); }
    void sParse();

  signals:
    void privateIdChanged(int);
    void newId(int);
    void aliasChanged(const QString &);
    void uomChanged(const QString &);
    void descrip1Changed(const QString &);
    void descrip2Changed(const QString &);
    void typeChanged(const QString &);
    void upcChanged(const QString &);
    void configured(bool);
    void warehouseIdChanged(int);
    void valid(bool);

  protected slots:
    itemList* listFactory();
    itemSearch* searchFactory();
    void sUpdateMenu();

  private:
    void constructor();

    ItemLineEditDelegate *_delegate;
    QString _sql;
    QString _validationSql;
    QString _itemNumber;
    QString _uom;
    QString _upc;
    QString _itemType;
    QStringList _extraClauses;
    unsigned int _type;
    unsigned int _defaultType;
    bool    _configured;
    bool    _useQuery;
    bool    _useValidationQuery;
};

class ItemLineEditDelegate : public QItemDelegate
{
  Q_OBJECT

  public:
    ItemLineEditDelegate(QObject *parent = 0);
    ~ItemLineEditDelegate();

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    void     setEditorData(QWidget *editor, const QModelIndex &index) const;
    void     setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const;

  private:
    ItemLineEdit *_template;
};

class XTUPLEWIDGETS_EXPORT ItemCluster : public VirtualCluster
{
  Q_OBJECT
  Q_PROPERTY(unsigned int type      READ type           WRITE setType            DESIGNABLE false)

  friend class ContactList;
  friend class ContactSearch;

  public:
    ItemCluster(QWidget*, const char* = 0);

    Q_INVOKABLE void setReadOnly(const bool);
    void setEnabled(const bool);
    void setDisabled(bool);

    Q_INVOKABLE inline void    setType(unsigned int pType)    { static_cast<ItemLineEdit*>(_number)->setType(pType);
                                                                static_cast<ItemLineEdit*>(_number)->setDefaultType(pType); }
    Q_INVOKABLE inline unsigned int type() const              { return static_cast<ItemLineEdit*>(_number)->type();                 }
    Q_INVOKABLE inline void setDefaultType(unsigned int pType){ static_cast<ItemLineEdit*>(_number)->setDefaultType(pType); }
    Q_INVOKABLE inline void setQuery(const QString &pSql)     { static_cast<ItemLineEdit*>(_number)->setQuery(pSql); }
    Q_INVOKABLE inline void setValidationQuery(const QString &pSql) { static_cast<ItemLineEdit*>(_number)->setValidationQuery(pSql);      }
    Q_INVOKABLE QString itemNumber() const                 { return static_cast<ItemLineEdit*>(_number)->itemNumber();           }
    Q_INVOKABLE QString itemType() const                   { return static_cast<ItemLineEdit*>(_number)->itemType();             }
    Q_INVOKABLE bool isConfigured() const                  { return static_cast<ItemLineEdit*>(_number)->isConfigured();         }
    Q_INVOKABLE QString  uom() const                       { return static_cast<ItemLineEdit*>(_number)->uom();                  }
    Q_INVOKABLE QString  upc() const                       { return static_cast<ItemLineEdit*>(_number)->upc();                  }

    Q_INVOKABLE inline void addExtraClause(const QString & pClause) { static_cast<ItemLineEdit*>(_number)->addExtraClause(pClause);     }
    Q_INVOKABLE inline QStringList getExtraClauseList() const       { return static_cast<ItemLineEdit*>(_number)->getExtraClauseList(); }
    Q_INVOKABLE inline void clearExtraClauseList()                  { static_cast<ItemLineEdit*>(_number)->clearExtraClauseList();      }
    Q_INVOKABLE ItemLineEdit *itemLineEdit() { return static_cast<ItemLineEdit*>(_number); }

    void setOrientation(Qt::Orientation orientation);

  public slots:
    QItemDelegate *itemDelegate() { return static_cast<ItemLineEdit*>(_number)->itemDelegate(); }
    void setDescriptionVisible(const bool p);
    void setId(const int pId);
    void setItemNumber(QString);
    void setItemsiteid(int);

  signals:
    void privateIdChanged(int);
    void aliasChanged(const QString &);
    void uomChanged(const QString &);
    void descrip1Changed(const QString &);
    void descrip2Changed(const QString &);
    void warehouseIdChanged(int);
    void typeChanged(const QString &);
    void upcChanged(const QString &);
    void configured(bool);

  protected:
    void addNumberWidget(ItemLineEdit* pNumberWidget);

  private:
    QLabel       *_uom;
    QLabel       *_descrip2;
    QString       _default;
};

#endif
