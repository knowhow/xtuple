/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "xtsettings.h"

#include <QSettings>

QVariant xtsettingsValue(const QString & key, const QVariant & defaultValue)
{
  QSettings settings(QSettings::UserScope, "xTuple.com", "xTuple");
  QString key2 = key;
  if(key.startsWith("/xTuple/"))
    key2 = key2.replace(0, 8, QString("/OpenMFG/"));
  if(settings.contains(key))
    return settings.value(key, defaultValue);
  else
  {
    QSettings oldsettings(QSettings::UserScope, "OpenMFG.com", "OpenMFG");
    if(oldsettings.contains(key2))
    {
      QVariant val = oldsettings.value(key2, defaultValue);
      xtsettingsSetValue(key, val);
      return val;
    }
  }
  return defaultValue;
}

void xtsettingsSetValue(const QString & key, const QVariant & value)
{
  QSettings settings(QSettings::UserScope, "xTuple.com", "xTuple");
  settings.setValue(key, value);
}

