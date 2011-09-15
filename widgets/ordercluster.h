/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef _orderCluster_h

#define _orderCluster_h

#include "parameter.h"
#include "virtualCluster.h"

class XTUPLEWIDGETS_EXPORT OrderLineEdit : public VirtualClusterLineEdit
{
  Q_OBJECT
  Q_FLAGS (OrderStatuses)
  Q_FLAGS (OrderTypes)
  Q_PROPERTY(OrderStatuses allowedStatuses READ allowedStatuses WRITE setAllowedStatuses)
  Q_PROPERTY(OrderTypes    allowedTypes    READ allowedTypes    WRITE setAllowedTypes)

  public:
    OrderLineEdit(QWidget*, const char* = 0);
    ~OrderLineEdit();

    enum OrderStatus
    {
      AnyStatus = 0x00,
      Unposted  = 0x01, Open = 0x02, Closed = 0x04
    };
    Q_DECLARE_FLAGS(OrderStatuses, OrderStatus)

    enum OrderType
    {
      AnyType = 0x00,
      Purchase = 0x01, Return = 0x02, Sales = 0x04, Transfer = 0x08
    };
    Q_DECLARE_FLAGS(OrderTypes, OrderType)

    Q_INVOKABLE virtual OrderStatuses allowedStatuses()        const;
    Q_INVOKABLE virtual OrderTypes    allowedTypes()        const;
    Q_INVOKABLE virtual void          clear();
    Q_INVOKABLE virtual QString          from()                const;
    Q_INVOKABLE virtual bool          isClosed()                const;
    Q_INVOKABLE virtual bool          isOpen()                const;
    Q_INVOKABLE virtual bool          isPO()                const;
    Q_INVOKABLE virtual bool          isRA()                const;
    Q_INVOKABLE virtual bool          isSO()                const;
    Q_INVOKABLE virtual bool          isTO()                const;
    Q_INVOKABLE virtual bool          isUnposted()                const;
    Q_INVOKABLE virtual bool      fromSitePrivsEnforced() const {return _fromPrivs; }
    Q_INVOKABLE virtual bool      toSitePrivsEnforced()   const {return _toPrivs; }
    Q_INVOKABLE virtual void      setExtraClause(const QString & p) {VirtualClusterLineEdit::setExtraClause(p);}
    Q_INVOKABLE virtual void          setExtraClause(const QString &, const QString &);
    Q_INVOKABLE virtual void          setExtraClause(const OrderTypes, const QString &);
    Q_INVOKABLE virtual void      setFromSitePrivsEnforced(const bool p);
    Q_INVOKABLE virtual void      setToSitePrivsEnforced(const bool p);
    Q_INVOKABLE virtual OrderStatus          status();
    Q_INVOKABLE virtual QString          to()                        const;
    Q_INVOKABLE virtual QString          type();
    Q_INVOKABLE virtual QString       fromPrivsClause() {return _fromPrivsClause;}
    Q_INVOKABLE virtual QString       toPrivsClause()   {return _toPrivsClause;}

    Q_INVOKABLE virtual bool lockSelected() {return _lock; }
    Q_INVOKABLE virtual void setLockSelected(bool lock) { _lock = lock; }

  public slots:
    virtual void          setAllowedStatuses(const OrderStatuses);
    virtual void          setAllowedType(const QString &);
    virtual void          setAllowedTypes(const OrderTypes);
    virtual void          setCustId(int);
    virtual void          setId(const int, const QString & = QString::null);
    virtual void          sList();
    virtual void          sSearch();

  signals:
    void newId(const int, const QString &);
    void numberChanged(const QString &, const QString &);

  protected:
    OrderStatuses        _allowedStatuses;
    OrderTypes                _allowedTypes;
    QString                _from;
    QString                _to;

    virtual QString        buildExtraClause();
    virtual void        silentSetId(const int);
    virtual void        unlock();


  protected slots:
    virtual void        sNewId(const int);
    virtual void        sParse();
    virtual VirtualList*    listFactory();
    virtual VirtualSearch*  searchFactory();


  private:
    bool        _fromPrivs;
    bool        _toPrivs;
    bool        _lock;
    QString     _toPrivsClause;
    QString     _fromPrivsClause;
    QString        _allClause;
    QString        _poClause;
    QString        _raClause;
    QString        _soClause;
    QString        _toClause;
    QString        _statusClause;
    QString        _typeClause;
    int         _custid;
};
Q_DECLARE_OPERATORS_FOR_FLAGS(OrderLineEdit::OrderStatuses)
Q_DECLARE_OPERATORS_FOR_FLAGS(OrderLineEdit::OrderTypes)

class XTUPLEWIDGETS_EXPORT OrderCluster : public VirtualCluster
{
  Q_OBJECT

  Q_PROPERTY(OrderLineEdit::OrderStatuses allowedStatuses READ allowedStatuses WRITE setAllowedStatuses)
  Q_PROPERTY(OrderLineEdit::OrderTypes    allowedTypes    READ allowedTypes    WRITE setAllowedTypes)
  Q_PROPERTY(bool    nameVisible    READ nameVisible    WRITE setNameVisible)
  Q_PROPERTY(bool    descriptionVisible    READ descriptionVisible    WRITE setDescriptionVisible)

  public:
    OrderCluster(QWidget*, const char* = 0);

    Q_INVOKABLE virtual OrderLineEdit::OrderStatuses allowedStatuses() const;
    Q_INVOKABLE virtual OrderLineEdit::OrderTypes    allowedTypes()    const;
    Q_INVOKABLE virtual QString from()                  const;
    Q_INVOKABLE virtual bool    isClosed()              const;
    Q_INVOKABLE virtual bool    isOpen()                const;
    Q_INVOKABLE virtual bool    isPO()                  const;
    Q_INVOKABLE virtual bool    isRA()                  const;
    Q_INVOKABLE virtual bool    isSO()                  const;
    Q_INVOKABLE virtual bool    isTO()                  const;
    Q_INVOKABLE virtual bool    isUnposted()            const;
    Q_INVOKABLE virtual bool    fromSitePrivsEnforced() const;
    Q_INVOKABLE virtual bool    toSitePrivsEnforced()   const;
    Q_INVOKABLE virtual void    setExtraClause(const QString &, const QString &);
    Q_INVOKABLE virtual void    setExtraClause(const OrderLineEdit::OrderTypes,
                                               const QString &);
    Q_INVOKABLE virtual OrderLineEdit::OrderStatus status()            const;
    Q_INVOKABLE virtual QString                    to()                const;
    Q_INVOKABLE virtual QString                    type()              const;

    Q_INVOKABLE virtual bool lockSelected();
    Q_INVOKABLE virtual void setLockSelected(bool lock);

    virtual bool        nameVisible();
    virtual void        setNameVisible(const bool p);

    virtual bool        descriptionVisible();
    virtual void        setDescriptionVisible(const bool p);

  public slots:
    virtual void        setAllowedStatuses(const OrderLineEdit::OrderStatuses);
    virtual void        setAllowedStatuses(const int);
    virtual void        setAllowedType(const QString &);
    virtual void        setAllowedTypes(const OrderLineEdit::OrderTypes);
    virtual void        setAllowedTypes(const int);
    virtual void        setCustId(int);
    virtual void        setId(const int, const QString& = QString::null);
    virtual void        setFromSitePrivsEnforced(const bool p);
    virtual void        setToSitePrivsEnforced(const bool p);
    virtual void        sRefresh();

  signals:
    void newId(const int, const QString &);
    void numberChanged(const QString &, const QString &);

  protected:
    QLabel        *_fromLit;
    QLabel        *_from;
    QLabel        *_toLit;
    QLabel        *_to;
    bool           _nameVisible;
    bool           _descripVisible;
    
};

class XTUPLEWIDGETS_EXPORT OrderList : public VirtualList
{
  Q_OBJECT

  public:
    OrderList(QWidget*, Qt::WindowFlags = 0);

    QString type() const;

  public slots:
    virtual void sClose();

  protected:
    QList<XTreeWidgetItem*> selectedAtDone;

  protected slots:
    virtual void done(int);
};

class XTUPLEWIDGETS_EXPORT OrderSearch : public VirtualSearch
{
  Q_OBJECT

  public:
    OrderSearch(QWidget*, Qt::WindowFlags = 0);

    QString     type() const;

  public slots:
    virtual void sClose();

  protected:
   QList<XTreeWidgetItem*> selectedAtDone;

  protected slots:
    virtual void done(int);
};

#endif
