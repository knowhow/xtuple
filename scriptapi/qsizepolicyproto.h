/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef __QSIZEPOLICYPROTO_H__
#define __QSIZEPOLICYPROTO_H__

#include <QSizePolicy>
#include <QObject>
#include <QtScript>

Q_DECLARE_METATYPE(enum QSizePolicy::ControlType)
Q_DECLARE_METATYPE(enum QSizePolicy::Policy)
Q_DECLARE_METATYPE(enum QSizePolicy::PolicyFlag)

void setupQSizePolicy(QScriptEngine *engine);

#endif
