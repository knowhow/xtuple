/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef _quoteCluster_h

#define _quoteCluster_h

#include "virtualCluster.h"

class QuoteList : public VirtualList
{
  Q_OBJECT

  public:
    QuoteList(QWidget *pParent, Qt::WindowFlags flags);
};

class QuoteSearch : public VirtualSearch
{
  Q_OBJECT

  public:
    QuoteSearch(QWidget *pParent, Qt::WindowFlags flags = 0);

};

class QuoteLineEdit : public VirtualClusterLineEdit
{
    Q_OBJECT

    public:
      QuoteLineEdit(QWidget*, const char* = 0);

      Q_INVOKABLE virtual bool forCustomer();
      Q_INVOKABLE virtual bool forProspect();
      Q_INVOKABLE virtual int  recipId();

    public slots:
      virtual void sSearch();

    protected slots:
        virtual void clear();
        virtual void sParse();

    protected:
      virtual void silentSetId(const int pId);

      int       _recip_id;
      QString   _recip_type;
};

class XTUPLEWIDGETS_EXPORT QuoteCluster : public VirtualCluster
{
    Q_OBJECT

    public:
      QuoteCluster(QWidget*, const char* = 0);

      Q_INVOKABLE bool forCustomer();
      Q_INVOKABLE bool forProspect();
      Q_INVOKABLE int  recipId();

};

#endif
