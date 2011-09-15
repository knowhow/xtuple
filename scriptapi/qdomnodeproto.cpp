/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "qdomnodeproto.h"

#include <QDomAttr>
#include <QDomCDATASection>
#include <QDomCharacterData>
#include <QDomComment>
#include <QDomDocument>
#include <QDomDocumentFragment>
#include <QDomDocumentType>
#include <QDomElement>
#include <QDomEntity>
#include <QDomEntityReference>
#include <QDomImplementation>
#include <QDomNamedNodeMap>
#include <QDomNode>
#include <QDomNodeList>
#include <QDomNotation>
#include <QDomProcessingInstruction>
#include <QDomText>

void setupQDomNodeProto(QScriptEngine *engine)
{
  QScriptValue proto = engine->newQObject(new QDomNodeProto(engine));
  engine->setDefaultPrototype(qMetaTypeId<QDomNode*>(), proto);
  engine->setDefaultPrototype(qMetaTypeId<QDomNode>(), proto);

  QScriptValue constructor = engine->newFunction(constructQDomNode,
                                                 proto);
  engine->globalObject().setProperty("QDomNode",  constructor);
}

QScriptValue constructQDomNode(QScriptContext * /*context*/,
                                    QScriptEngine  *engine)
{
  QDomNode *obj = 0;
  // if (context->argumentCount() > 0)
  // else
    obj = new QDomNode();
  return engine->toScriptValue(obj);
}

QDomNodeProto::QDomNodeProto(QObject *parent)
    : QObject(parent)
{
}

QDomNode QDomNodeProto:: appendChild(const QDomNode& newChild)
{
  QDomNode *item = qscriptvalue_cast<QDomNode*>(thisObject());
  if (item)
    return item->appendChild(newChild);
  return QDomNode();
}

QDomNamedNodeMap QDomNodeProto:: attributes() const
{
  QDomNode *item = qscriptvalue_cast<QDomNode*>(thisObject());
  if (item)
    return item->attributes();
  return QDomNamedNodeMap();
}

QDomNodeList QDomNodeProto:: childNodes() const
{
  QDomNode *item = qscriptvalue_cast<QDomNode*>(thisObject());
  if (item)
    return item->childNodes();
  return QDomNodeList();
}

void QDomNodeProto:: clear()
{
  QDomNode *item = qscriptvalue_cast<QDomNode*>(thisObject());
  if (item)
    item->clear();
}

QDomNode QDomNodeProto:: cloneNode(bool deep) const
{
  QDomNode *item = qscriptvalue_cast<QDomNode*>(thisObject());
  if (item)
    return item->cloneNode(deep);
  return QDomNode();
}

int QDomNodeProto:: columnNumber() const
{
  QDomNode *item = qscriptvalue_cast<QDomNode*>(thisObject());
  if (item)
    return item->columnNumber();
  return 0;
}

QDomNode QDomNodeProto:: firstChild() const
{
  QDomNode *item = qscriptvalue_cast<QDomNode*>(thisObject());
  if (item)
    return item->firstChild();
  return QDomNode();
}

QDomElement QDomNodeProto:: firstChildElement(const QString &tagName) const
{
  QDomNode *item = qscriptvalue_cast<QDomNode*>(thisObject());
  if (item)
    return item->firstChildElement(tagName);
  return QDomElement();
}

bool QDomNodeProto:: hasAttributes() const
{
  QDomNode *item = qscriptvalue_cast<QDomNode*>(thisObject());
  if (item)
    return item->hasAttributes();
  return false;
}

bool QDomNodeProto:: hasChildNodes() const
{
  QDomNode *item = qscriptvalue_cast<QDomNode*>(thisObject());
  if (item)
    return item->hasChildNodes();
  return false;
}

QDomNode QDomNodeProto:: insertAfter(const QDomNode& newChild, const QDomNode& refChild)
{
  QDomNode *item = qscriptvalue_cast<QDomNode*>(thisObject());
  if (item)
    return item->insertAfter(newChild, refChild);
  return QDomNode();
}

QDomNode QDomNodeProto:: insertBefore(const QDomNode& newChild, const QDomNode& refChild)
{
  QDomNode *item = qscriptvalue_cast<QDomNode*>(thisObject());
  if (item)
    return item->insertBefore(newChild, refChild);
  return QDomNode();
}

bool QDomNodeProto:: isAttr() const
{
  QDomNode *item = qscriptvalue_cast<QDomNode*>(thisObject());
  if (item)
    return item->isAttr();
  return false;
}

bool QDomNodeProto:: isCDATASection() const
{
  QDomNode *item = qscriptvalue_cast<QDomNode*>(thisObject());
  if (item)
    return item->isCDATASection();
  return false;
}

bool QDomNodeProto:: isCharacterData() const
{
  QDomNode *item = qscriptvalue_cast<QDomNode*>(thisObject());
  if (item)
    return item->isCharacterData();
  return false;
}

bool QDomNodeProto:: isComment() const
{
  QDomNode *item = qscriptvalue_cast<QDomNode*>(thisObject());
  if (item)
    return item->isComment();
  return false;
}

bool QDomNodeProto:: isDocument() const
{
  QDomNode *item = qscriptvalue_cast<QDomNode*>(thisObject());
  if (item)
    return item->isDocument();
  return false;
}

bool QDomNodeProto:: isDocumentFragment() const
{
  QDomNode *item = qscriptvalue_cast<QDomNode*>(thisObject());
  if (item)
    return item->isDocumentFragment();
  return false;
}

bool QDomNodeProto:: isDocumentType() const
{
  QDomNode *item = qscriptvalue_cast<QDomNode*>(thisObject());
  if (item)
    return item->isDocumentType();
  return false;
}

bool QDomNodeProto:: isElement() const
{
  QDomNode *item = qscriptvalue_cast<QDomNode*>(thisObject());
  if (item)
    return item->isElement();
  return false;
}

bool QDomNodeProto:: isEntity() const
{
  QDomNode *item = qscriptvalue_cast<QDomNode*>(thisObject());
  if (item)
    return item->isEntity();
  return false;
}

bool QDomNodeProto:: isEntityReference() const
{
  QDomNode *item = qscriptvalue_cast<QDomNode*>(thisObject());
  if (item)
    return item->isEntityReference();
  return false;
}

bool QDomNodeProto:: isNotation() const
{
  QDomNode *item = qscriptvalue_cast<QDomNode*>(thisObject());
  if (item)
    return item->isNotation();
  return false;
}

bool QDomNodeProto:: isNull() const
{
  QDomNode *item = qscriptvalue_cast<QDomNode*>(thisObject());
  if (item)
    return item->isNull();
  return false;
}

bool QDomNodeProto:: isProcessingInstruction() const
{
  QDomNode *item = qscriptvalue_cast<QDomNode*>(thisObject());
  if (item)
    return item->isProcessingInstruction();
  return false;
}

bool QDomNodeProto:: isSupported(const QString& feature, const QString& version) const
{
  QDomNode *item = qscriptvalue_cast<QDomNode*>(thisObject());
  if (item)
    return item->isSupported(feature, version);
  return false;
}

bool QDomNodeProto:: isText() const
{
  QDomNode *item = qscriptvalue_cast<QDomNode*>(thisObject());
  if (item)
    return item->isText();
  return false;
}

QDomNode QDomNodeProto:: lastChild() const
{
  QDomNode *item = qscriptvalue_cast<QDomNode*>(thisObject());
  if (item)
    return item->lastChild();
  return QDomNode();
}

QDomElement QDomNodeProto:: lastChildElement(const QString &tagName) const
{
  QDomNode *item = qscriptvalue_cast<QDomNode*>(thisObject());
  if (item)
    return item->lastChildElement(tagName);
  return QDomElement();
}

int QDomNodeProto:: lineNumber() const
{
  QDomNode *item = qscriptvalue_cast<QDomNode*>(thisObject());
  if (item)
    return item->lineNumber();
  return 0;
}

QString QDomNodeProto:: localName() const
{
  QDomNode *item = qscriptvalue_cast<QDomNode*>(thisObject());
  if (item)
    return item->localName();
  return QString();
}

QDomNode QDomNodeProto:: namedItem(const QString& name) const
{
  QDomNode *item = qscriptvalue_cast<QDomNode*>(thisObject());
  if (item)
    return item->namedItem(name);
  return QDomNode();
}

QString QDomNodeProto:: namespaceURI() const
{
  QDomNode *item = qscriptvalue_cast<QDomNode*>(thisObject());
  if (item)
    return item->namespaceURI();
  return QString();
}

QDomNode QDomNodeProto:: nextSibling() const
{
  QDomNode *item = qscriptvalue_cast<QDomNode*>(thisObject());
  if (item)
    return item->nextSibling();
  return QDomNode();
}

QDomElement QDomNodeProto:: nextSiblingElement(const QString &taName) const
{
  QDomNode *item = qscriptvalue_cast<QDomNode*>(thisObject());
  if (item)
    return item->nextSiblingElement(taName);
  return QDomElement();
}

QString QDomNodeProto:: nodeName() const
{
  QDomNode *item = qscriptvalue_cast<QDomNode*>(thisObject());
  if (item)
    return item->nodeName();
  return QString();
}

int QDomNodeProto::nodeType() const
{
  QDomNode *item = qscriptvalue_cast<QDomNode*>(thisObject());
  if (item)
    return item->nodeType();
  return 0;
}

QString QDomNodeProto:: nodeValue() const
{
  QDomNode *item = qscriptvalue_cast<QDomNode*>(thisObject());
  if (item)
    return item->nodeValue();
  return QString();
}

void QDomNodeProto:: normalize()
{
  QDomNode *item = qscriptvalue_cast<QDomNode*>(thisObject());
  if (item)
    item->normalize();
}

QDomDocument QDomNodeProto:: ownerDocument() const
{
  QDomNode *item = qscriptvalue_cast<QDomNode*>(thisObject());
  if (item)
    return item->ownerDocument();
  return QDomDocument();
}

QDomNode QDomNodeProto:: parentNode() const
{
  QDomNode *item = qscriptvalue_cast<QDomNode*>(thisObject());
  if (item)
    return item->parentNode();
  return QDomNode();
}

QString QDomNodeProto:: prefix() const
{
  QDomNode *item = qscriptvalue_cast<QDomNode*>(thisObject());
  if (item)
    return item->prefix();
  return QString();
}

QDomNode QDomNodeProto:: previousSibling() const
{
  QDomNode *item = qscriptvalue_cast<QDomNode*>(thisObject());
  if (item)
    return item->previousSibling();
  return QDomNode();
}

QDomElement QDomNodeProto:: previousSiblingElement(const QString &tagName) const
{
  QDomNode *item = qscriptvalue_cast<QDomNode*>(thisObject());
  if (item)
    return item->previousSiblingElement(tagName);
  return QDomElement();
}

QDomNode QDomNodeProto:: removeChild(const QDomNode& oldChild)
{
  QDomNode *item = qscriptvalue_cast<QDomNode*>(thisObject());
  if (item)
    return item->removeChild(oldChild);
  return QDomNode();
}

QDomNode QDomNodeProto:: replaceChild(const QDomNode& newChild, const QDomNode& oldChild)
{
  QDomNode *item = qscriptvalue_cast<QDomNode*>(thisObject());
  if (item)
    return item->replaceChild(newChild, oldChild);
  return QDomNode();
}

void QDomNodeProto:: save(QTextStream& stream, int intarg) const
{
  QDomNode *item = qscriptvalue_cast<QDomNode*>(thisObject());
  if (item)
    item->save(stream, intarg);
}

void QDomNodeProto:: save(QTextStream& stream, int intarg, int policy) const
{
  QDomNode *item = qscriptvalue_cast<QDomNode*>(thisObject());
  if (item)
    item->save(stream, intarg, (QDomNode::EncodingPolicy)policy);
}

void QDomNodeProto:: setNodeValue(const QString&value)
{
  QDomNode *item = qscriptvalue_cast<QDomNode*>(thisObject());
  if (item)
    item->setNodeValue(value);
}

void QDomNodeProto:: setPrefix(const QString& pre)
{
  QDomNode *item = qscriptvalue_cast<QDomNode*>(thisObject());
  if (item)
    item->setPrefix(pre);
}

QDomAttr QDomNodeProto:: toAttr() const
{
  QDomNode *item = qscriptvalue_cast<QDomNode*>(thisObject());
  if (item)
    return item->toAttr();
  return QDomAttr();
}

QDomCDATASection QDomNodeProto:: toCDATASection() const
{
  QDomNode *item = qscriptvalue_cast<QDomNode*>(thisObject());
  if (item)
    return item->toCDATASection();
  return QDomCDATASection();
}

QDomCharacterData QDomNodeProto:: toCharacterData() const
{
  QDomNode *item = qscriptvalue_cast<QDomNode*>(thisObject());
  if (item)
    return item->toCharacterData();
  return QDomCharacterData();
}

QDomComment QDomNodeProto:: toComment() const
{
  QDomNode *item = qscriptvalue_cast<QDomNode*>(thisObject());
  if (item)
    return item->toComment();
  return QDomComment();
}

QDomDocument QDomNodeProto:: toDocument() const
{
  QDomNode *item = qscriptvalue_cast<QDomNode*>(thisObject());
  if (item)
    return item->toDocument();
  return QDomDocument();
}

QDomDocumentFragment QDomNodeProto:: toDocumentFragment() const
{
  QDomNode *item = qscriptvalue_cast<QDomNode*>(thisObject());
  if (item)
    return item->toDocumentFragment();
  return QDomDocumentFragment();
}

QDomDocumentType QDomNodeProto:: toDocumentType() const
{
  QDomNode *item = qscriptvalue_cast<QDomNode*>(thisObject());
  if (item)
    return item->toDocumentType();
  return QDomDocumentType();
}

QDomElement QDomNodeProto:: toElement() const
{
  QDomNode *item = qscriptvalue_cast<QDomNode*>(thisObject());
  if (item)
    return item->toElement();
  return QDomElement();
}

QDomEntity QDomNodeProto:: toEntity() const
{
  QDomNode *item = qscriptvalue_cast<QDomNode*>(thisObject());
  if (item)
    return item->toEntity();
  return QDomEntity();
}

QDomEntityReference QDomNodeProto:: toEntityReference() const
{
  QDomNode *item = qscriptvalue_cast<QDomNode*>(thisObject());
  if (item)
    return item->toEntityReference();
  return QDomEntityReference();
}

QDomNotation QDomNodeProto:: toNotation() const
{
  QDomNode *item = qscriptvalue_cast<QDomNode*>(thisObject());
  if (item)
    return item->toNotation();
  return QDomNotation();
}

QDomProcessingInstruction QDomNodeProto:: toProcessingInstruction() const
{
  QDomNode *item = qscriptvalue_cast<QDomNode*>(thisObject());
  if (item)
    return item->toProcessingInstruction();
  return QDomProcessingInstruction();
}

QDomText QDomNodeProto:: toText() const
{
  QDomNode *item = qscriptvalue_cast<QDomNode*>(thisObject());
  if (item)
    return item->toText();
  return QDomText();
}

QString QDomNodeProto::toString() const
{
  QDomNode *item = qscriptvalue_cast<QDomNode*>(thisObject());
  if (item)
    return QString("[QDomNode(%1=%2, %3, %4)]")
                    .arg(item->nodeName())
                    .arg(item->nodeValue())
                    .arg(item->nodeType())
                    .arg(item->hasChildNodes() ? "has children" : "leaf node");
  return QString("[QDomNode(unknown)]");
}
