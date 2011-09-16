/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef glCluster_h
#define glCluster_h

#include "virtualCluster.h"
#include "parameter.h"
#include "projectcluster.h"
#include <QComboBox>

class XTUPLEWIDGETS_EXPORT accountList : public VirtualList
{
    Q_OBJECT

public:
    accountList(QWidget*, Qt::WindowFlags = 0);
    virtual XTreeWidget* xtreewidget();

public slots:
    virtual void set( const ParameterList & pParams );
    virtual void sFillList();

private:
    int          _accntid;
    bool         _showExternal;
    unsigned int _type;

};

class XTUPLEWIDGETS_EXPORT accountSearch : public VirtualSearch
{
    Q_OBJECT

public:
    accountSearch(QWidget*, Qt::WindowFlags = 0);

public slots:
    void set( ParameterList & pParams );
    void sFillList();

protected:
    void showEvent(QShowEvent*);

private:
    int          _accntid;
    bool         _showExternal;
    unsigned int _typeval;
    QComboBox*   _type;
    QLabel*      _typeLit;
    QHBoxLayout* _typeStrLyt;
};

class XTUPLEWIDGETS_EXPORT GLClusterLineEdit : public VirtualClusterLineEdit
{
    Q_OBJECT

    public:
      GLClusterLineEdit(QWidget*, const char* = 0);

      void setType(unsigned int pType);
      unsigned int type()  const       { return _type; }
      bool showExternal()              { return _showExternal; }
      void setShowExternal(bool p);

    public slots:
      void sList();
      void sSearch();
      void setId(const int);

    protected slots:
      accountList* listFactory();
      accountSearch* searchFactory();
      void buildExtraClause();
      void sParse();

    protected:
      QMap<QString, QString> _typeMap;

    private:
      unsigned int _type;
      bool _showExternal;
      QStringList  _types;
};


class XTUPLEWIDGETS_EXPORT GLCluster : public VirtualCluster
{
  Q_OBJECT
    
  public:
    GLCluster(QWidget*, const char* = 0);

    enum Type {
      cUndefined  = 0x00,

      cAsset      = 0x01,
      cLiability  = 0x02,
      cExpense    = 0x04,
      cRevenue    = 0x08,
      cEquity     = 0x10
    };

    Q_INVOKABLE void setType(unsigned int pType) { static_cast<GLClusterLineEdit*>(_number)->setType(pType); }
    Q_INVOKABLE unsigned int type()  const       { return static_cast<GLClusterLineEdit*>(_number)->type(); }
    Q_INVOKABLE bool showExternal()              { return static_cast<GLClusterLineEdit*>(_number)->showExternal(); }
    Q_INVOKABLE void setShowExternal(bool p)     { static_cast<GLClusterLineEdit*>(_number)->setShowExternal(p); }

    Q_INVOKABLE bool projectVisible();
    Q_INVOKABLE bool setProjectVisible(bool p);

  public slots:
    void setId(const int p);

  protected slots:
    void sHandleProjectState(int p);
    void sHandleProjectId();

  private:
    ProjectLineEdit* _project;
    QLabel*          _projectLit;
    bool             _projectVisible;
};

#endif

