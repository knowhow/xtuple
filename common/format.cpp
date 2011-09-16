/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include <string.h>

#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include "format.h"
#include "xsqlquery.h"

#define MONEYSCALE            2
#define COSTEXTRASCALE        2
#define WEIGHTSCALE           2
#define QTYSCALE              2
#define QTYPERSCALE           6
#define SALESPRICEEXTRASCALE  2
#define PURCHPRICEEXTRASCALE  2
#define UOMRATIOSCALE         6
#define PERCENTSCALE          2

#define DEBUG                 false

static QColor error("red");
static QColor warning("orange");
static QColor emphasis("blue");
static QColor altemphasis("green");
static QColor expired("red");
static QColor future("blue");

static int    costscale        = MONEYSCALE + COSTEXTRASCALE;
static int    currvalscale     = MONEYSCALE;
static int    extpricescale    = MONEYSCALE;
static int    percentscale     = PERCENTSCALE;
static int    purchpricescale  = MONEYSCALE + PURCHPRICEEXTRASCALE;
static int    qtyscale         = QTYSCALE;
static int    qtyperscale      = QTYPERSCALE;
static int    salespricescale  = MONEYSCALE + SALESPRICEEXTRASCALE;
static int    uomratioscale    = UOMRATIOSCALE;
static int    weightscale      = WEIGHTSCALE;

static bool   loadedLocales    = false;

static bool   loadLocale()
{
  if (!loadedLocales)
  {
    XSqlQuery localeq("SELECT * "
                      "FROM usr, locale LEFT OUTER JOIN"
                      "     lang ON (locale_lang_id=lang_id) LEFT OUTER JOIN"
                      "     country ON (locale_country_id=country_id) "
                      "WHERE ( (usr_username=getEffectiveXtUser())"
                      " AND (usr_locale_id=locale_id) );" );
    if (localeq.first())
    {
      if (!localeq.value("locale_error_color").toString().isEmpty())
        error = QColor(localeq.value("locale_error_color").toString());
      if (!localeq.value("locale_warning_color").toString().isEmpty())
        warning = QColor(localeq.value("locale_warning_color").toString());
      if (!localeq.value("locale_emphasis_color").toString().isEmpty())
        emphasis = QColor(localeq.value("locale_emphasis_color").toString());
      if (!localeq.value("locale_altemphasis_color").toString().isEmpty())
        altemphasis = QColor(localeq.value("locale_altemphasis_color").toString());
      if (!localeq.value("locale_expired_color").toString().isEmpty())
        expired = QColor(localeq.value("locale_expired_color").toString());
      if (!localeq.value("locale_future_color").toString().isEmpty())
        future = QColor(localeq.value("locale_future_color").toString());

      if (!localeq.value("locale_cost_scale").toString().isEmpty())
        costscale = localeq.value("locale_cost_scale").toInt();
      if (!localeq.value("locale_curr_scale").toString().isEmpty())
        currvalscale = localeq.value("locale_curr_scale").toInt();
      if (!localeq.value("locale_extprice_scale").toString().isEmpty())
        extpricescale = localeq.value("locale_extprice_scale").toInt();
      if (!localeq.value("locale_percent_scale").toString().isEmpty())
        percentscale = localeq.value("locale_percent_scale").toInt();
      if (!localeq.value("locale_purchprice_scale").toString().isEmpty())
        purchpricescale = localeq.value("locale_purchprice_scale").toInt();
      if (!localeq.value("locale_qty_scale").toString().isEmpty())
        qtyscale = localeq.value("locale_qty_scale").toInt();
      if (!localeq.value("locale_qtyper_scale").toString().isEmpty())
        qtyperscale = localeq.value("locale_qtyper_scale").toInt();
      if (!localeq.value("locale_salesprice_scale").toString().isEmpty())
        salespricescale = localeq.value("locale_salesprice_scale").toInt();
      if (!localeq.value("locale_uomratio_scale").toString().isEmpty())
        uomratioscale = localeq.value("locale_uomratio_scale").toInt();

      // TODO: add locale_percent_scale
      // TODO: add locale_weight_scale
    }
    else if (localeq.lastError().type() != QSqlError::NoError)
    {
      QMessageBox::critical(0,
                            QObject::tr("A System Error Occurred at %1::%2.")
                            .arg(__FILE__).arg(__LINE__),
                            localeq.lastError().databaseText());
      return false;
    }
    loadedLocales = true;
  }
  return true;
}

int decimalPlaces(QString pName)
{
  int returnVal = MONEYSCALE;
  if (!loadedLocales)
    loadLocale();

  QByteArray  tba  = pName.toAscii();
  char        *ptr = tba.data();
  // the following order is based on the relative frequencies of usage
  // for xtnumericrole in the .cpp and .mql files except for the final
  // else, which ranked 7th overall
  if (strcmp(ptr, "qty") == 0)
    returnVal = qtyscale;
  else if (strncmp(ptr, "curr", 4) == 0)
    returnVal = currvalscale;   // TODO: change this to currency-specific value?
  else if (strcmp(ptr, "percent") == 0)
    returnVal = percentscale;
  else if (strcmp(ptr, "cost") == 0)
    returnVal = costscale;
  else if (strcmp(ptr, "qtyper") == 0)
    returnVal = qtyperscale;
  else if (strcmp(ptr, "salesprice") == 0)
    returnVal = salespricescale;
  else if (strcmp(ptr, "purchprice") == 0)
    returnVal = purchpricescale;
  else if (strcmp(ptr, "uomratio") == 0)
    returnVal = uomratioscale;
  else if (strcmp(ptr, "extprice") == 0)
    returnVal = extpricescale;
  else if (strcmp(ptr, "weight") == 0)
    returnVal = weightscale;
  else
  {
    bool ok = false;
    returnVal = pName.toInt(&ok);
    if (!ok)
      returnVal = MONEYSCALE;
  }

  return returnVal;
}

QColor namedColor(QString pName)
{
  (void)loadLocale();

  if (pName == "error")
    return error;
  else if (pName == "warning")
    return warning;
  else if (pName == "emphasis")
    return emphasis;
  else if (pName == "altemphasis")
    return altemphasis;
  else if (pName == "expired")
    return expired;
  else if (pName == "future")
    return future;

  return QColor(pName);
}

QString formatNumber(double value, int decimals)
{
  if (DEBUG)
    qDebug("formatNumber(%f, %d)", value, decimals);
  return QLocale().toString(value, 'f', decimals);
}

/*
  different currencies have different rounding conventions, so we need
  the currency id to find the right rounding rules.
  we need extra decimal places for some data because some monetary values,
  like unit costs, are stored with extra precision.
*/
QString formatMoney(double value, int /* curr_id */, int extraDecimals)
{
  return QLocale().toString(value, 'f', currvalscale + extraDecimals);
}

QString formatCost(double value, int /* curr_id */)
{
  return formatNumber(value, costscale);
}

QString formatExtPrice(double value, int /* curr_id */)
{
  return formatNumber(value, extpricescale);
}

QString formatWeight(double value)
{
  return formatNumber(value, weightscale);
}

QString formatQty(double value)
{
  return formatNumber(value, qtyscale);
}

QString formatQtyPer(double value)
{
  return formatNumber(value, qtyperscale);
}

QString formatSalesPrice(double value, int /* curr_id */)
{
  return formatNumber(value, salespricescale);
}

QString formatPurchPrice(double value, int /* curr_id */)
{
  return formatNumber(value, purchpricescale);
}

QString formatUOMRatio(double value)
{
  return formatNumber(value, uomratioscale);
}

QString formatPercent(double value)
{
  return formatNumber(value * 100.0, percentscale);
}
