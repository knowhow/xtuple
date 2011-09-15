/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which(including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef __QLAYOUTPROTO_H__
#define __QLAYOUTPROTO_H__

#include <QLayout>
#include <QObject>
#include <QtScript>

class QString;

Q_DECLARE_METATYPE(QLayout*)

void setupQLayoutProto(QScriptEngine *engine);

#endif
