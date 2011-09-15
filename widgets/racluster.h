/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef _raCluster_h

#define _raCluster_h

#include "virtualCluster.h"

class XTUPLEWIDGETS_EXPORT RaLineEdit : public VirtualClusterLineEdit
{
  Q_OBJECT

  public:
    RaLineEdit(QWidget*, const char* = 0);

    enum RaStatus
    {
      AnyStatus = 0x00, Open = 0x01, Closed = 0x02
    };
    Q_DECLARE_FLAGS(RaStatuses, RaStatus)

    virtual RaStatuses	allowedStatuses()	const;
    virtual void	setAllowedStatuses(const RaStatuses);
    virtual bool	isClosed()		const;
    virtual bool	isOpen()		const;
    virtual RaStatus	status()		const;

  signals:
    void numberChanged(const QString &);

  public slots:
    virtual void setId(const int pId);

  protected:

    RaStatuses	_statuses;
};

class XTUPLEWIDGETS_EXPORT RaCluster : public VirtualCluster
{
  Q_OBJECT

  public:
    RaCluster(QWidget*, const char* = 0);

    virtual RaLineEdit::RaStatuses allowedStatuses()	const;
    virtual void	setAllowedStatuses(const RaLineEdit::RaStatuses p);
    virtual bool		   isClosed()		const;
    virtual bool		   isOpen()		const;
    virtual RaLineEdit::RaStatus   status()		const;

  signals:
    void numberChanged(const QString &);

};

class XTUPLEWIDGETS_EXPORT RaList : public VirtualList
{
  Q_OBJECT

  public:
    RaList(QWidget*, Qt::WindowFlags = 0);
};

#endif
