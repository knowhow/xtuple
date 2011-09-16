/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */


#include "metrics.h"
#include <QVariant>
#include "xsqlquery.h"

Parameters::Parameters(QObject * parent)
  : QObject(parent)
{
  _dirty = FALSE;
}

void Parameters::load()
{
  _values.clear();

  XSqlQuery q;
  q.prepare(_readSql);
  q.bindValue(":username", _username);
  q.exec();
  while (q.next())
    _values[q.value("key").toString()] = q.value("value").toString();

  _dirty = FALSE;
}

QString Parameters::value(const char *pName)
{
  return value(QString(pName));
}

QString Parameters::value(const QString &pName)
{
  MetricMap::iterator it = _values.find(pName);
  if (it == _values.end())
    return QString::null;
  else
    return it.value();
}

bool Parameters::boolean(const char *pName)
{
  return boolean(QString(pName));
}

bool Parameters::boolean(const QString &pName)
{
  MetricMap::iterator it = _values.find(pName);
  if (it == _values.end())
    return FALSE;
  else if (it.value() == "t")
    return TRUE;

  return FALSE;
}

void Parameters::set(const char *pName, bool pValue)
{
  set(pName, QString(pValue ? "t" : "f"));
}

void Parameters::set(const QString &pName, bool pValue)
{
  set(pName, QString(pValue ? "t" : "f"));
}

void Parameters::set(const char *pName, int pValue)
{
  set(QString(pName), pValue);
}

void Parameters::set(const QString &pName, int pValue)
{
  set(QString(pName), QString::number(pValue));
}

void Parameters::set(const char *pName, const QString &pValue)
{
  set(QString(pName), pValue);
}

void Parameters::set(const QString &pName, const QString &pValue)
{
  MetricMap::iterator it = _values.find(pName);
  if (it != _values.end())
  {
    if (it.value() == pValue)
      return;
    else
      it.value() = pValue;
  }
  else
    _values[pName] = pValue;

  _set(pName, pValue);
}

void Parameters::_set(const QString &pName, QVariant pValue)
{
  XSqlQuery q;
  q.prepare(_setSql);
  q.bindValue(":username", _username);
  q.bindValue(":name", pName);
  q.bindValue(":value", pValue);
  q.exec();

  _dirty = TRUE;
}

QString Parameters::parent(const QString &pValue)
{
  for (MetricMap::iterator it = _values.begin(); it != _values.end(); it++)
    if (it.value() == pValue)
      return it.key();

  return QString::null;
}


Metrics::Metrics()
{
  _readSql = "SELECT metric_name AS key, metric_value AS value FROM metric;";
  _setSql  = "SELECT setMetric(:name, :value);";

  load();
}


Preferences::Preferences(const QString &pUsername)
{
  _readSql  = "SELECT usrpref_name AS key, usrpref_value AS value "
              "FROM usrpref "
              "WHERE (usrpref_username=:username);";
  _setSql   = "SELECT setUserPreference(:username, :name, :value);";
  _username = pUsername;

  load();
}

void Preferences::remove(const QString &pPrefName)
{
  XSqlQuery q;
  q.prepare("SELECT deleteUserPreference(:prefname);");
  q.bindValue(":prefname", pPrefName);
  q.exec();

  _dirty = TRUE;
}


Privileges::Privileges()
{
  _readSql = "SELECT priv_name AS key, TEXT('t') AS value "
             "  FROM usrpriv, priv "
             " WHERE((usrpriv_priv_id=priv_id)"
             "   AND (usrpriv_username=getEffectiveXtUser())) "
             " UNION "
             "SELECT priv_name AS key, TEXT('t') AS value "
             "  FROM priv, grppriv, usrgrp"
             " WHERE((usrgrp_grp_id=grppriv_grp_id)"
             "   AND (grppriv_priv_id=priv_id)"
             "   AND (usrgrp_username=getEffectiveXtUser()));";

  load();
}

bool Privileges::check(const QString &pName)
{
  MetricMap::iterator it = _values.find(pName);
  if (it == _values.end())
    return FALSE;
  else
    return TRUE;
}

