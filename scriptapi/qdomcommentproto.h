/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef __QDOMCOMMENTPROTO_H__
#define __QDOMCOMMENTPROTO_H__

#include <QDomComment>
#include <QObject>
#include <QtScript>

class QDomAttr;
class QDomCDATASection;
class QDomCharacterData;
class QDomDocument;
class QDomDocumentFragment;
class QDomDocumentType;
class QDomElement;
class QDomEntity;
class QDomEntityReference;
class QDomImplementation;
class QDomNamedNodeMap;
class QDomNode;
class QDomNodeList;
class QDomNotation;
class QDomProcessingInstruction;
class QDomText;

Q_DECLARE_METATYPE(QDomComment*)
Q_DECLARE_METATYPE(QDomComment)

void setupQDomCommentProto(QScriptEngine *engine);
QScriptValue constructQDomComment(QScriptContext *context, QScriptEngine *engine);

class QDomCommentProto : public QObject, public QScriptable
{
  Q_OBJECT

  public:
    QDomCommentProto(QObject *parent);

    Q_INVOKABLE int     nodeType() const;
    Q_INVOKABLE QString toString() const;
};

#endif
