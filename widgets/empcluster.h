/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef _empCluster_h

#define _empCluster_h

#include "virtualCluster.h"

class EmpInfo : public VirtualInfo
{
    Q_OBJECT

    public:
      EmpInfo(QWidget*, Qt::WindowFlags = 0);
};

class EmpList : public VirtualList
{
    Q_OBJECT

    public:
      EmpList(QWidget*, Qt::WindowFlags = 0);
};

class EmpSearch : public VirtualSearch
{
    Q_OBJECT

    public:
      EmpSearch(QWidget*, Qt::WindowFlags = 0);
};


class XTUPLEWIDGETS_EXPORT EmpClusterLineEdit : public VirtualClusterLineEdit
{
    Q_OBJECT

    public:
        EmpClusterLineEdit(QWidget*, const char* = 0);

        static int idFromList(QWidget* = 0); // TODO: put in VirtualClusterLineEdit?

    protected:
        virtual VirtualInfo   *infoFactory();
        virtual VirtualList   *listFactory();
        virtual VirtualSearch *searchFactory();
};

class XTUPLEWIDGETS_EXPORT EmpCluster : public VirtualCluster
{
    Q_OBJECT

    public:
        EmpCluster(QWidget*, const char* = 0);
};
#endif
