/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */


#ifndef __XTSETTINGS_H__
#define __XTSETTINGS_H__

#include <QString>
#include <QVariant>

QVariant xtsettingsValue(const QString & key, const QVariant & defaultValue = QVariant());
void xtsettingsSetValue(const QString & key, const QVariant & value);

#endif

