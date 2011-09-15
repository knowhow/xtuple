/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef __QBASE64ENCODE_H__
#define __QBASE64ENCODE_H__

#include <QString>

class QIODevice;

QString    QBase64Encode(QIODevice &);
QByteArray QBase64Decode(const QString &);

#endif

