/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef _projectCluster_h

#define _projectCluster_h

#include "virtualCluster.h"

class XTUPLEWIDGETS_EXPORT ProjectLineEdit : public VirtualClusterLineEdit
{
    Q_OBJECT

    Q_ENUMS(ProjectType)

    Q_PROPERTY(ProjectType projectType READ type WRITE setType )
    
    public:
      enum ProjectType
      {
        Undefined,
        SalesOrder,
        WorkOrder,
        PurchaseOrder,
      };

      enum ProjectStatus
      {
        AnyStatus = 0x00,
        Concept  = 0x01, InProcess = 0x02, Complete = 0x04
      };
      Q_DECLARE_FLAGS(ProjectStatuses, ProjectStatus)
       
      ProjectLineEdit(QWidget*, const char* = 0);
      ProjectLineEdit(enum ProjectType pPrjType, QWidget *pParent, const char *pName);

      Q_INVOKABLE virtual enum ProjectType type() const { return _type;     }
      Q_INVOKABLE virtual ProjectStatuses allowedStatuses() const { return _allowedStatuses; }
      
    public slots:
      void setExtraClause(const QString &pExt);
      void setAllowedStatuses(const ProjectStatuses);
      void setType(enum ProjectType ptype);

    protected:
      void buildExtraClause();

      QString          _prjExtraClause;
      ProjectStatuses  _allowedStatuses;
      
    private:
      enum ProjectType _type;

};
Q_DECLARE_OPERATORS_FOR_FLAGS(ProjectLineEdit::ProjectStatuses)

class XTUPLEWIDGETS_EXPORT ProjectCluster : public VirtualCluster
{
    Q_OBJECT

    Q_ENUMS(ProjectLineEdit::ProjectType)

    Q_PROPERTY(ProjectLineEdit::ProjectType projectType READ type WRITE setType )
    
    public:
      ProjectCluster(QWidget*, const char* = 0);

      Q_INVOKABLE virtual ProjectLineEdit::ProjectStatuses allowedStatuses() const;
      Q_INVOKABLE enum ProjectLineEdit::ProjectType type();
      
    public slots:
      virtual void setAllowedStatuses(const ProjectLineEdit::ProjectStatuses);
      virtual void setAllowedStatuses(const int);
      virtual void setType(enum ProjectLineEdit::ProjectType ptype);

};

#endif
