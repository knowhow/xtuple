/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

/*
 *     Header containing the prototypes of the MD5 Hash
 * code, based on RFC1321.
 */

#ifndef __QMD5_H__
#define __QMD5_H__

#include <QString>

QString QMd5(const QString &);
QString QMd5(const QByteArray &);

#endif

