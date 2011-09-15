/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef _shiftCluster_h
#define _shiftCluster_h

#include "virtualCluster.h"

class XTUPLEWIDGETS_EXPORT ShiftClusterLineEdit : public VirtualClusterLineEdit
{
    Q_OBJECT

    public:
        ShiftClusterLineEdit(QWidget*, const char* = 0);
};

class XTUPLEWIDGETS_EXPORT ShiftCluster : public VirtualCluster
{
    Q_OBJECT

    public:
        ShiftCluster(QWidget*, const char* = 0);
};

#endif
