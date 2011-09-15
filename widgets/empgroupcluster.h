/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef _empGroupCluster_h
#define _empGroupCluster_h

#include "virtualCluster.h"

class EmpGroupInfo : public VirtualInfo
{
    Q_OBJECT

    public:
      EmpGroupInfo(QWidget*, Qt::WindowFlags = 0);
};

class EmpGroupList : public VirtualList
{
    Q_OBJECT

    public:
      EmpGroupList(QWidget*, Qt::WindowFlags = 0);
};

class EmpGroupSearch : public VirtualSearch
{
    Q_OBJECT

    public:
      EmpGroupSearch(QWidget*, Qt::WindowFlags = 0);
};


class XTUPLEWIDGETS_EXPORT EmpGroupClusterLineEdit : public VirtualClusterLineEdit
{
    Q_OBJECT

    public:
        EmpGroupClusterLineEdit(QWidget*, const char* = 0);

        static int idFromList(QWidget* = 0); // TODO: put in VirtualClusterLineEdit?

    protected:
        virtual VirtualInfo   *infoFactory();
        virtual VirtualList   *listFactory();
        virtual VirtualSearch *searchFactory();
};

class XTUPLEWIDGETS_EXPORT EmpGroupCluster : public VirtualCluster
{
    Q_OBJECT

    public:
        EmpGroupCluster(QWidget*, const char* = 0);
};
#endif
