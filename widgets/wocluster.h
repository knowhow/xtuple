/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef woCluster_h
#define woCluster_h

#include "widgets.h"

#include <QDate>

#include <xsqlquery.h>
#include "virtualCluster.h"
#include "ui_womatlcluster.h"

class QLabel;
class QPushButton;
class WoCluster;
class XComboBox;
class WarehouseGroup;

//  Possible Work Order Status
#define cWoOpen     WoLineEdit::Open
#define cWoExploded WoLineEdit::Exploded
#define cWoIssued   WoLineEdit::Issued
#define cWoReleased WoLineEdit::Released
#define cWoClosed   WoLineEdit::Closed

class QScriptEngine;

void setupWoCluster(QScriptEngine *engine);

class XTUPLEWIDGETS_EXPORT woList : public VirtualList
{
    Q_OBJECT

public:
    woList(QWidget*, Qt::WindowFlags = 0);

public slots:
    virtual void set( const ParameterList & pParams );
    virtual void sFillList();

private:
    int _woid;
    int _type;
    WarehouseGroup* _warehouse;

};

class XTUPLEWIDGETS_EXPORT woSearch : public VirtualSearch
{
    Q_OBJECT

public:
    woSearch(QWidget*, Qt::WindowFlags = 0);

public slots:
    void set( ParameterList & pParams );
    void sFillList();

private:
    int _woid;
    int _type;
    WarehouseGroup* _warehouse;
};


class XTUPLEWIDGETS_EXPORT WoLineEdit : public VirtualClusterLineEdit
{
  Q_OBJECT

  Q_ENUMS(WoStatus)

  Q_PROPERTY(int type READ type WRITE setType DESIGNABLE false)

friend class WoCluster;

  public:
    WoLineEdit(QWidget *, const char * = 0);
    WoLineEdit(int, QWidget *, const char * = 0);

    enum WoStatus {
      Open      = 0x01,
      Exploded  = 0x02,
      Issued    = 0x04,
      Released  = 0x08,
      Closed    = 0x10
    };

    Q_INVOKABLE inline int     currentWarehouse() const       { return _currentWarehouseid;  }
    Q_INVOKABLE inline QString method() const                 { return _method;              }
    Q_INVOKABLE void           setType(int pWoType);
    Q_INVOKABLE void           setWarehouse(int pWarehouseid);
    Q_INVOKABLE inline int     type() const                   { return _woType;              }
    Q_INVOKABLE inline int     warehouse() const              { return _warehouseid;         }

  public slots:
    void silentSetId(const int);
    void sList();
    void sSearch();

  private:
    void init();
    void buildExtraClause();

    int     _currentWarehouseid;
    int     _woType;
    int     _warehouseid;
    QChar   _status;
    double  _qtyOrdered;
    double  _qtyReceived;
    QString _method;

  protected:
    woList* listFactory();
    woSearch* searchFactory();

  signals:
    void newId(int);
    void newItemid(int);
    void warehouseChanged(const QString &);
    void itemNumberChanged(const QString &);
    void uomChanged(const QString &);
    void itemDescrip1Changed(const QString &);
    void itemDescrip2Changed(const QString &);
    void startDateChanged(const QDate &);
    void dueDateChanged(const QDate &);
    void qtyOrderedChanged(const double);
    void qtyReceivedChanged(const double);
    void qtyBalanceChanged(const double);
    void statusChanged(const QString &);
    void methodChanged(const QString &);
    void valid(bool);
};

class XTUPLEWIDGETS_EXPORT WoCluster : public QWidget
{
  Q_OBJECT
  Q_PROPERTY(QString label          READ label          WRITE setLabel)
  Q_PROPERTY(QString fieldName      READ fieldName      WRITE setFieldName)
  Q_PROPERTY(QString number         READ woNumber       WRITE setWoNumber       DESIGNABLE false)
  Q_PROPERTY(QString defaultNumber  READ defaultNumber                          DESIGNABLE false)
  Q_PROPERTY(int     type           READ type           WRITE setType           DESIGNABLE false)
  
  public:
    WoCluster(QWidget *, const char * = 0);
    WoCluster(int, QWidget *, const char * = 0);

    QString  defaultNumber()    { return QString();          }
    QString  fieldName()        { return _fieldName;         }
    QString  woNumber() const;

    virtual Qt::Orientation orientation();
    virtual void setOrientation(Qt::Orientation orientation);

    virtual QString label();
    virtual void setLabel(const QString& p);

    inline void setType(int pWoType)           { _woNumber->setType(pWoType);           }
    inline int  type() const                   { return _woNumber->_woType;              }
    inline void setWarehouse(int pWarehouseid) { _woNumber->setWarehouse(pWarehouseid); }
    Q_INVOKABLE inline int currentWarehouse() const { return _woNumber->currentWarehouse(); }
    Q_INVOKABLE inline int id() const          { return _woNumber->_id;                  }
    Q_INVOKABLE inline bool isValid() const    { return _woNumber->_valid;               }
    Q_INVOKABLE inline char status() const     { return _woNumber->_status.toAscii();    }
    Q_INVOKABLE inline QString method() const     { return _woNumber->method();          }
    Q_INVOKABLE inline double qtyOrdered() const { return _woNumber->_qtyOrdered;        }
    Q_INVOKABLE inline double qtyReceived() const { return _woNumber->_qtyReceived;      }
    Q_INVOKABLE inline double qtyBalance() const
    {
      if (_woNumber->_qtyOrdered <= _woNumber->_qtyReceived)
        return 0;
      else
        return (_woNumber->_qtyOrdered - _woNumber->_qtyReceived);
    }
    Q_INVOKABLE inline int    warehouse() const   { return _woNumber->warehouse();       }

  public slots:
    void setDataWidgetMap(XDataWidgetMapper* m);
    void setFieldName(const QString& name)        { _fieldName = name; }
    void setId(int);
    void setReadOnly(bool);
    void setWoNumber(const QString& number);

  private:
    void constructor();

    WoLineEdit  *_woNumber;
    QLabel      *_woNumberLit;
    QLabel      *_warehouse;
    QLabel      *_itemNumber;
    QLabel      *_itemNumberLit;
    QLabel      *_uom;
    QLabel      *_uomLit;
    QLabel      *_descrip1;
    QLabel      *_descrip2;
    QLabel      *_status;
    QLabel      *_statusLit;
    QLabel      *_method;
    QLabel      *_methodLit;
    QString      _fieldName;
    QHBoxLayout *_line1Layout;
    QHBoxLayout *_line2Layout;
    QHBoxLayout *_itemLayout;
    QVBoxLayout *_mainLayout;
    QHBoxLayout *_statusLayout;
    QHBoxLayout *_uomLayout;
    QHBoxLayout *_warehouseLayout;
    QHBoxLayout *_woLayout;
    Qt::Orientation _orientation;

  signals:
    void newId(int);
    void newItemid(int);
    void startDateChanged(const QDate &);
    void dueDateChanged(const QDate &);
    void qtyOrderedChanged(const double &);
    void qtyReceivedChanged(const double &);
    void qtyBalanceChanged(const double &);

    void valid(bool);
};

class XTUPLEWIDGETS_EXPORT WomatlCluster : public QWidget, public Ui::WomatlCluster
{
  Q_OBJECT
  Q_PROPERTY(QString fieldName      READ fieldName      WRITE setFieldName)

  public:
    enum SourceTypes
    {
      WorkOrder  = 0x01,
      WoMaterial = 0x02,
      Wooper     = 0x04
    };

    enum IssueTypes
    {
      Pull  = 0x01,
      Push  = 0x02,
      Mixed = 0x04
    };

    WomatlCluster(QWidget *, const char * = 0);
    WomatlCluster(WoCluster *, QWidget *, const char * = 0);

    void setReadOnly(bool);

    Q_INVOKABLE int id()             { return _id;       }
    Q_INVOKABLE int woid()           { return _woid;     }
    Q_INVOKABLE bool isValid()       { return _valid;    }
    Q_INVOKABLE double qtyRequired() { return _required; }
    Q_INVOKABLE double qtyIssued()   { return _issued;   }
    inline QString fieldName()  { return _fieldName;}

  signals:
    void newId(int);
    void valid(bool);
    void newQtyRequired(const double);
    void newQtyIssued(const double);
    void newQtyScrappedFromWIP(const double);

  public slots:
    void setDataWidgetMap(XDataWidgetMapper* m);
    void setFieldName(const QString& name)        { _fieldName = name; }
    void setId(int);
    void setType(int);
    void setWooperid(int);
    void setWoid(int);
    void sPopulateInfo(int);
    void sRefresh();

  protected slots:
    virtual void languageChange();

  private:
    void constructor();

    XSqlQuery _womatl;
    int      _id;
    int      _woid;
    int      _source;
    int      _type;
    int      _sourceId;
    int      _sense;
    bool     _valid;
    double   _required;
    double   _issued;
    QString  _fieldName;
};


#endif

