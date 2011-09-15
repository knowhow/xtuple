/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef format_h
#define format_h

#include <QColor>
#include <QLocale>
#include <QString>

int             decimalPlaces(QString);
QString         formatNumber(double, int);
QString         formatMoney(double, int = -1, int = 0);
QString         formatCost(double, int = -1);
QString         formatExtPrice(double, int = -1);
QString         formatWeight(double);
QString         formatQty(double);
QString         formatQtyPer(double);
QString         formatSalesPrice(double, int = -1);
QString         formatPurchPrice(double, int = -1);
QString         formatUOMRatio(double);
QString         formatPercent(double);
QColor          namedColor(QString);

inline QString  formatDate(const QDate &pDate)
{
  return QLocale().toString(pDate, QLocale::ShortFormat);
}

#endif
