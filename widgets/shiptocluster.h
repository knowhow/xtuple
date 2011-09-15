/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef shiptoCluster_h
#define shiptoCluster_h

#include "virtualCluster.h"

#include "widgets.h"

class QLabel;
class QPushButton;

class ShiptoCluster;

class XTUPLEWIDGETS_EXPORT shipToList : public VirtualList
{
    Q_OBJECT

public:
    shipToList(QWidget*, Qt::WindowFlags = 0);
    void set( const ParameterList & pParams );

private:
    QLabel* _custNumber;
    QLabel* _custName;
    QLabel* _custAddr;

};

class XTUPLEWIDGETS_EXPORT shipToSearch : public VirtualSearch
{
    Q_OBJECT

public:
    shipToSearch(QWidget*, Qt::WindowFlags = 0);
    void set( ParameterList & pParams );

private:
    int _custid;
};

class XTUPLEWIDGETS_EXPORT ShiptoEdit : public VirtualClusterLineEdit
{
  Q_OBJECT

  public:
    ShiptoEdit(QWidget *, const char * = 0);

    inline int custid()  { return _custid; };

  signals:
    void nameChanged(const QString &);
    void address1Changed(const QString &);
    void requestList();
    void newCustid(int);
    void disableList(bool);

  public slots:
    void setId(int);
    void setCustid(int);
    void sNew();
    void sList();
    void sSearch();

  protected slots:
    shipToList* listFactory();
    shipToSearch* searchFactory();

  private:
    int  _custid;
};


class XTUPLEWIDGETS_EXPORT ShiptoCluster : public VirtualCluster
{
  Q_OBJECT

  public:
    ShiptoCluster(QWidget *, const char * = 0);

    inline int custid()   { return static_cast<ShiptoEdit*>(_number)->custid(); }

  signals:
    void newCustid(int);

  public slots:
    void setCustid(int);
};

#endif

