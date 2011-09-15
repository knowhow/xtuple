/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */


#include "metricsenc.h"
#include "xsqlquery.h"

Parametersenc::Parametersenc(QObject * parent)
  : QObject(parent)
{
  _dirty = FALSE;
}

void Parametersenc::load()
{
  _values.clear();

  XSqlQuery q;
  q.prepare(_readSql);
//  q.bindValue(":username", _username);
  q.bindValue(":key", _key);
  q.exec();
  while (q.next())
    _values[q.value("key").toString()] = q.value("value").toString();

  _dirty = FALSE;
}

QString Parametersenc::value(const char *pName)
{
  return value(QString(pName));
}

QString Parametersenc::value(const QString &pName)
{
  MetricMap::iterator it = _values.find(pName);
  if (it == _values.end())
    return QString::null;
  else
    return it.value();
}

bool Parametersenc::boolean(const char *pName)
{
  return boolean(QString(pName));
}

bool Parametersenc::boolean(const QString &pName)
{
  MetricMap::iterator it = _values.find(pName);
  if (it == _values.end())
    return FALSE;
  else if (it.value() == "t")
    return TRUE;

  return FALSE;
}

void Parametersenc::set(const char *pName, bool pValue)
{
  set(QString(pName), pValue);
}

void Parametersenc::set(const QString &pName, bool pValue)
{
  MetricMap::iterator it = _values.find(pName);
  if ( (it != _values.end()) && (it.value() == ((pValue) ? "t" : "f")) )
    return;

  _set(pName, ((pValue) ? QString("t") : QString("f")));
}

void Parametersenc::set(const char *pName, int pValue)
{
  set(QString(pName), pValue);
}

void Parametersenc::set(const QString &pName, int pValue)
{
  MetricMap::iterator it = _values.find(pName);
  if ( (it != _values.end()) && (it.value().toInt() == pValue) )
    return;

  _set(pName, pValue);
}

void Parametersenc::set(const char *pName, const QString &pValue)
{
  set(QString(pName), pValue);
}

void Parametersenc::set(const QString &pName, const QString &pValue)
{
  MetricMap::iterator it = _values.find(pName);
  if (it != _values.end())
  {
    if (it.value() == pValue)
      return;
    else
      it.value() =  pValue;
  }
  else
    _values[pName] = pValue;

  _set(pName, pValue);
}

void Parametersenc::_set(const QString &pName, QVariant pValue)
{
  XSqlQuery q;
  q.prepare(_setSql);
//  q.bindValue(":username", _username);
  q.bindValue(":name", pName);
  q.bindValue(":value", pValue);
  q.bindValue(":key", _key);
  q.exec();

  _dirty = TRUE;
}

QString Parametersenc::parent(const QString &pValue)
{
  for (MetricMap::iterator it = _values.begin(); it != _values.end(); it++)
    if (it.value() == pValue)
      return it.key();

  return QString::null;
}


Metricsenc::Metricsenc(const QString &pKey)
{
  _readSql = "SELECT metricenc_name AS key, decrypt(setbytea(metricenc_value), setbytea(:key), 'bf') AS value FROM metricenc;";
  _setSql  = "SELECT setMetricEnc(:name, :value, :key);";
  _key = pKey;

  load();
}



