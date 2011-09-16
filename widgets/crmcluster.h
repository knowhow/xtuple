/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef _crmcluster_h

#define _crmcluster_h

#include "virtualCluster.h"

class XTUPLEWIDGETS_EXPORT CrmClusterLineEdit : public VirtualClusterLineEdit
{
    Q_OBJECT

    public:
      CrmClusterLineEdit(QWidget*, const char*, const char*, const char*,
                         const char*, const char*, const char*, const char*,
                         const char* = 0, const char* = 0, const char* = 0);

      Q_INVOKABLE inline virtual QString editOwnPriv() const { return _editOwnPriv; }
      Q_INVOKABLE inline virtual QString viewOwnPriv() const { return _viewOwnPriv; }

    public slots:
      virtual void sOpen();
      virtual void setEditOwnPriv(const QString& priv);
      virtual void setViewOwnPriv(const QString& priv);

    protected slots:
      virtual void sUpdateMenu();
      virtual void setTableAndColumnNames(const char* pTabName,
                                          const char* pIdColumn,
                                          const char* pNumberColumn,
                                          const char* pNameColumn,
                                          const char* pDescripColumn,
                                          const char* pActiveColumn,
                                          const char* pOwnerColumn = 0,
                                          const char* pAssignToColumn = 0);
      virtual void silentSetId(const int);

    protected:
      QString _editOwnPriv;
      QString _viewOwnPriv;
      QString _owner;
      QString _assignto;
      QString _ownerColName;
      QString _assigntoColName;
      QString _setQuery;
      bool _hasOwner;
      bool _hasAssignto;

};

#endif
