/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "taxCache.h"

#include <QString>

void taxCache::clear()
{
  for (unsigned h = Pct; h <= Amount; h++)
    for (unsigned i = A; i <= C; i++)
      for (unsigned j = Line; j <= Adj; j++)
	cache[h][i][j] = 0.0;

  for (unsigned i = Tax; i <= Type; i++)
    for (unsigned j = Line; j <= Adj; j++)
      ids[i][j] = 0;
}

taxCache::taxCache()
{
  clear();
}

taxCache::taxCache(taxCache& p)
{
  for (unsigned h = Pct; h <= Amount; h++)
    for (unsigned i = A; i <= C; i++)
      for (unsigned j = Line; j <= Adj; j++)
	cache[h][i][j] = p.cache[h][i][j];

  for (unsigned i = Tax; i <= Type; i++)
    for (unsigned j = Line; j <= Adj; j++)
      ids[i][j] = p.ids[i][j];
}

taxCache::~taxCache()
{
}

double	taxCache::adj() const
{
  return adj(0) + adj(1) + adj(2);
}

double	taxCache::adj(unsigned p) const
{
  return cache[Amount][p][Adj];
}

int	taxCache::adjId() const
{
  return ids[Tax][Adj];
}

int	taxCache::adjType() const
{
  return ids[Type][Adj];
}

double	taxCache::adjPct() const
{
  return adjPct(0) + adjPct(1) + adjPct(2);
}

double	taxCache::adjPct(unsigned p) const
{
  return cache[Pct][p][Adj];
}


double	taxCache::freight() const
{
  return freight(0) + freight(1) + freight(2);
}

double	taxCache::freight(unsigned p) const
{
  return cache[Amount][p][Freight];
}

int	taxCache::freightId() const
{
  return ids[Tax][Freight];
}

int	taxCache::freightType() const
{
  return ids[Type][Freight];
}

double	taxCache::freightPct() const
{
  return freightPct(0) + freightPct(1) + freightPct(2);
}

double taxCache::freightPct(unsigned p) const
{
  return cache[Pct][p][Freight];
}


double	taxCache::line() const
{
  return line(0) + line(1) + line(2);
}

double	taxCache::line(unsigned p) const
{
  return cache[Amount][p][Line];
}

double	taxCache::linePct() const
{
  return linePct(0) + linePct(1) + linePct(2);
}

double	taxCache::linePct(unsigned p) const
{
  return cache[Pct][p][Line];
}


void	taxCache::setAdj(const double a, const double b, const double c)
{
  cache[Amount][A][Adj] = a;
  cache[Amount][B][Adj] = b;
  cache[Amount][C][Adj] = c;
}

void	taxCache::setAdjPct(const double a, const double b, const double c)
{
  cache[Pct][A][Adj] = a;
  cache[Pct][B][Adj] = b;
  cache[Pct][C][Adj] = c;
}

void	taxCache::setAdjId(const int p)
{
  ids[Tax][Adj] = p;
}

void	taxCache::setAdjType(const int p)
{
  ids[Type][Adj] = p;
}

void	taxCache::setFreight(const double a, const double b, const double c)
{
  cache[Amount][A][Freight] = a;
  cache[Amount][B][Freight] = b;
  cache[Amount][C][Freight] = c;
}

void	taxCache::setFreightPct(const double a, const double b, const double c)
{
  cache[Pct][A][Freight] = a;
  cache[Pct][B][Freight] = b;
  cache[Pct][C][Freight] = c;
}

void	taxCache::setFreightId(const int p)
{
  ids[Tax][Freight] = p;
}

void	taxCache::setFreightType(const int p)
{
  ids[Type][Freight] = p;
}

void	taxCache::setLine(const double a, const double b, const double c)
{
  cache[Amount][A][Line] = a;
  cache[Amount][B][Line] = b;
  cache[Amount][C][Line] = c;
}

void	taxCache::setLinePct(const double a, const double b, const double c)
{
  cache[Pct][A][Line] = a;
  cache[Pct][B][Line] = b;
  cache[Pct][C][Line] = c;
}


double	taxCache::total() const
{
  return line() + freight() + adj();
}

double	taxCache::total(unsigned p) const
{
  return line(p) + freight(p) + adj(p);
}

QString	taxCache::toString() const
{
  QString result = "";
  QString id[]   =	{ "Tax", "Type" };
  QString info[] =	{ "Pct", "Amount" };
  QString rate[] =	{ "A", "B", "C" };
  QString part[] =	{ "Line", "Freight", "Adj" };

  for (unsigned h = Pct; h <= Amount; h++)
  {
    for (unsigned i = A; i <= C; i++)
    {
      result += info[h] + " " + rate[i] + ":";
      for (unsigned j = Line; j <= Adj; j++)
	result += "\t" + part[j] + " = " + QString::number(cache[h][i][j]);
      result += "\n";
    }
    result += "\n";
  }

  for (unsigned i = Tax; i <= Type; i++)
  {
    result += id[i] + ":";
    for (unsigned j = Line; j <= Adj; j++)
      result += "\t" + part[j] + " = " + QString::number(ids[i][j]);
    result += "\n";
  }

  return result;
}
