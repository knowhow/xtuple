/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef _opportunityCluster_h

#define _opportunityCluster_h

#include "crmcluster.h"

class XTUPLEWIDGETS_EXPORT OpportunityClusterLineEdit : public CrmClusterLineEdit
{
    Q_OBJECT

    public:
      OpportunityClusterLineEdit(QWidget*, const char* = 0);
};

class XTUPLEWIDGETS_EXPORT OpportunityCluster : public VirtualCluster
{
    Q_OBJECT

    public:
      OpportunityCluster(QWidget*, const char* = 0);
};

#endif
