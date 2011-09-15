/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef TAXCACHE_H
#define TAXCACHE_H

class QString;

class taxCache
{
  public:
    taxCache();
    taxCache(taxCache&);
    virtual ~taxCache();

    virtual void	clear();

    virtual double	adj()		const;
    virtual double	adj(unsigned)	const;
    virtual int		adjId()		const;
    virtual double	adjPct()	const;
    virtual double	adjPct(unsigned) const;
    virtual int		adjType()	const;

    virtual double	freight()	const;
    virtual double	freight(unsigned) const;
    virtual int		freightId()	const;
    virtual double	freightPct()	const;
    virtual double	freightPct(unsigned) const;
    virtual int		freightType()	const;

    virtual double	line()		const;
    virtual double	line(unsigned)	const;
    virtual double	linePct()	const;
    virtual double	linePct(unsigned) const;

    virtual double	total()		const;
    virtual double	total(unsigned)	const;

    virtual void	setAdj(const double, const double = 0, const double = 0);
    virtual void	setAdjId(const int);
    virtual void	setAdjPct(const double, const double = 0, const double = 0);
    virtual void	setAdjType(const int);
    virtual void	setFreight(const double, const double = 0, const double = 0);
    virtual void	setFreightId(const int);
    virtual void	setFreightPct(const double, const double = 0, const double = 0);
    virtual void	setFreightType(const int);
    virtual void	setLine(const double, const double = 0, const double = 0);
    virtual void	setLinePct(const double, const double = 0, const double = 0);

    virtual QString	toString()	const;

  protected:
    enum Id	{ Tax, Type };
    enum Info	{ Pct, Amount };
    enum Rate	{ A, B, C };
    enum Part	{ Line, Freight, Adj };
    double	cache[2][3][4];	// [Info] x [Rate] x [Part]
    int		ids[2][4];	// [Id] x [Part]
};
#endif // TAXCACHE_H
