/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef __METASQLHIGHLIGHTERPROTO_H__
#define __METASQLHIGHLIGHTERPROTO_H__

#include <QObject>
#include <QtScript>

#include "metasqlhighlighter.h"

class QString;

Q_DECLARE_METATYPE(MetaSQLHighlighter*)
//Q_DECLARE_METATYPE(MetaSQLHighlighter)

void setupMetaSQLHighlighterProto(QScriptEngine *engine);
QScriptValue constructMetaSQLHighlighter(QScriptContext *context, QScriptEngine *engine);

class MetaSQLHighlighterProto : public QObject, public QScriptable
{
  Q_OBJECT

  public:
    MetaSQLHighlighterProto(QObject *parent);

    Q_INVOKABLE QString toString() const;
};

#endif
