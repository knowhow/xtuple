/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef _lotserialCluster_h
#define _lotserialCluster_h

#include "virtualCluster.h"

class LotserialCluster;

class XTUPLEWIDGETS_EXPORT  LotserialList : public VirtualList
{
  Q_OBJECT

  friend class LotSerialLineEdit;

  public:
    LotserialList(QWidget*, Qt::WindowFlags = 0);

  private:
    LotserialCluster* _parent;
};

class XTUPLEWIDGETS_EXPORT  LotserialSearch : public VirtualSearch
{
  Q_OBJECT

  friend class LotSerialLineEdit;

  public:
    LotserialSearch(QWidget*, Qt::WindowFlags = 0);

  private:
    LotserialCluster* _parent;
};

class XTUPLEWIDGETS_EXPORT LotserialLineEdit : public VirtualClusterLineEdit
{
    Q_OBJECT

    Q_PROPERTY(bool strict	READ	strict	WRITE setStrict)

    friend class LotserialCluster;

    public:
	LotserialLineEdit(QWidget*, const char* = 0);

	inline virtual int	itemId() const		{ return _itemid; };
	inline virtual bool	strict() const		{ return _strict; };

    public slots:
        virtual void setId(const int);
	virtual void clear();
	virtual void sParse();
	virtual void setItemId(const int);
	inline virtual void	setStrict(const bool p)	{ _strict = p; };

    protected slots:
	virtual LotserialList*		listFactory();
	virtual LotserialSearch*	searchFactory();
	// virtual LotserialInfo*	infoFactory();
	
    signals:
        void newItemId(int);
	
    private:
	int	_itemid;
	bool	_strict;
};

class XTUPLEWIDGETS_EXPORT LotserialCluster : public VirtualCluster
{
    Q_OBJECT

    Q_PROPERTY(bool     strict          READ  strict          WRITE setStrict)

    friend class LotserialLineEdit;

    public:
	LotserialCluster(QWidget*, const char* = 0);

	virtual bool	strict()          const;
	virtual int	itemId()          const;

    public slots:
	virtual void	setItemId           (const int);
	virtual void	setStrict           (const bool);
	
    signals:
        void newItemId(int);

};

#endif
