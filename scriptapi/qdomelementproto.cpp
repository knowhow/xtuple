/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "qdomelementproto.h"

#include <QDomAttr>
#include <QDomElement>
#include <QDomNamedNodeMap>
#include <QDomNodeList>

#include "qdomnodeproto.h"

void setupQDomElementProto(QScriptEngine *engine)
{
  QScriptValue proto = engine->newQObject(new QDomElementProto(engine));
  engine->setDefaultPrototype(qMetaTypeId<QDomElement*>(), proto);
  engine->setDefaultPrototype(qMetaTypeId<QDomElement>(),  proto);

  QScriptValue constructor = engine->newFunction(constructQDomElement,
                                                 proto);
  engine->globalObject().setProperty("QDomElement",  constructor);
}

QScriptValue constructQDomElement(QScriptContext * /*context*/,
                                    QScriptEngine  *engine)
{
  QDomElement *obj = 0;
  // if (context->argumentCount() > 0)
  // else
    obj = new QDomElement();
  return engine->toScriptValue(obj);
}

QDomElementProto::QDomElementProto(QObject *parent)
    : QObject(parent)
{
}

QString QDomElementProto:: attribute(const QString& name, const QString& defValue) const
{
  QDomElement *item = qscriptvalue_cast<QDomElement*>(thisObject());
  if (item)
    return item->attribute(name, defValue);
  return QString();
}

QString QDomElementProto:: attributeNS(const QString nsURI, const QString& localName, const QString& defValue) const
{
  QDomElement *item = qscriptvalue_cast<QDomElement*>(thisObject());
  if (item)
    return item->attributeNS(nsURI, localName, defValue);
  return QString();
}

QDomAttr QDomElementProto:: attributeNode(const QString& name)
{
  QDomElement *item = qscriptvalue_cast<QDomElement*>(thisObject());
  if (item)
    return item->attributeNode(name);
  return QDomAttr();
}

QDomAttr QDomElementProto:: attributeNodeNS(const QString& nsURI, const QString& localName)
{
  QDomElement *item = qscriptvalue_cast<QDomElement*>(thisObject());
  if (item)
    return item->attributeNodeNS(nsURI, localName);
  return QDomAttr();
}

QDomNamedNodeMap QDomElementProto:: attributes() const
{
  QDomElement *item = qscriptvalue_cast<QDomElement*>(thisObject());
  if (item)
    return item->attributes();
  return QDomNamedNodeMap();
}

QDomNodeList QDomElementProto:: elementsByTagName(const QString& tagname) const
{
  QDomElement *item = qscriptvalue_cast<QDomElement*>(thisObject());
  if (item)
    return item->elementsByTagName(tagname);
  return QDomNodeList();
}

QDomNodeList QDomElementProto:: elementsByTagNameNS(const QString& nsURI, const QString& localName) const
{
  QDomElement *item = qscriptvalue_cast<QDomElement*>(thisObject());
  if (item)
    return item->elementsByTagNameNS(nsURI, localName);
  return QDomNodeList();
}

bool QDomElementProto:: hasAttribute(const QString& name) const
{
  QDomElement *item = qscriptvalue_cast<QDomElement*>(thisObject());
  if (item)
    return item->hasAttribute(name);
  return false;
}

bool QDomElementProto:: hasAttributeNS(const QString& nsURI, const QString& localName) const
{
  QDomElement *item = qscriptvalue_cast<QDomElement*>(thisObject());
  if (item)
    return item->hasAttributeNS(nsURI, localName);
  return false;
}

int QDomElementProto:: nodeType() const
{
  QDomElement *item = qscriptvalue_cast<QDomElement*>(thisObject());
  if (item)
    return item->nodeType();
  return 0;
}

void QDomElementProto:: removeAttribute(const QString& name)
{
  QDomElement *item = qscriptvalue_cast<QDomElement*>(thisObject());
  if (item)
    item->removeAttribute(name);
}

void QDomElementProto:: removeAttributeNS(const QString& nsURI, const QString& localName)
{
  QDomElement *item = qscriptvalue_cast<QDomElement*>(thisObject());
  if (item)
    item->removeAttributeNS(nsURI, localName);
}

QDomAttr QDomElementProto:: removeAttributeNode(const QDomAttr& oldAttr)
{
  QDomElement *item = qscriptvalue_cast<QDomElement*>(thisObject());
  if (item)
    return item->removeAttributeNode(oldAttr);
  return QDomAttr();
}

void QDomElementProto:: setAttribute(const QString& name, const QString& value)
{
  QDomElement *item = qscriptvalue_cast<QDomElement*>(thisObject());
  if (item)
    item->setAttribute(name, value);
}

void QDomElementProto:: setAttribute(const QString& name, double value)
{
  QDomElement *item = qscriptvalue_cast<QDomElement*>(thisObject());
  if (item)
    item->setAttribute(name, value);
}

void QDomElementProto:: setAttribute(const QString& name, float value)
{
  QDomElement *item = qscriptvalue_cast<QDomElement*>(thisObject());
  if (item)
    item->setAttribute(name, value);
}

void QDomElementProto:: setAttribute(const QString& name, int value)
{
  QDomElement *item = qscriptvalue_cast<QDomElement*>(thisObject());
  if (item)
    item->setAttribute(name, value);
}

void QDomElementProto:: setAttribute(const QString& name, qlonglong value)
{
  QDomElement *item = qscriptvalue_cast<QDomElement*>(thisObject());
  if (item)
    item->setAttribute(name, value);
}

void QDomElementProto:: setAttribute(const QString& name, qulonglong value)
{
  QDomElement *item = qscriptvalue_cast<QDomElement*>(thisObject());
  if (item)
    item->setAttribute(name, value);
}

void QDomElementProto:: setAttribute(const QString& name, uint value)
{
  QDomElement *item = qscriptvalue_cast<QDomElement*>(thisObject());
  if (item)
    item->setAttribute(name, value);
}

void QDomElementProto:: setAttributeNS(const QString nsURI, const QString& qName, const QString& value)
{
  QDomElement *item = qscriptvalue_cast<QDomElement*>(thisObject());
  if (item)
    item->setAttributeNS(nsURI, qName, value);
}

void QDomElementProto:: setAttributeNS(const QString nsURI, const QString& qName, double value)
{
  QDomElement *item = qscriptvalue_cast<QDomElement*>(thisObject());
  if (item)
    item->setAttributeNS(nsURI, qName, value);
}

void QDomElementProto:: setAttributeNS(const QString nsURI, const QString& qName, int value)
{
  QDomElement *item = qscriptvalue_cast<QDomElement*>(thisObject());
  if (item)
    item->setAttributeNS(nsURI, qName, value);
}

void QDomElementProto:: setAttributeNS(const QString nsURI, const QString& qName, qlonglong value)
{
  QDomElement *item = qscriptvalue_cast<QDomElement*>(thisObject());
  if (item)
    item->setAttributeNS(nsURI, qName, value);
}

void QDomElementProto:: setAttributeNS(const QString nsURI, const QString& qName, qulonglong value)
{
  QDomElement *item = qscriptvalue_cast<QDomElement*>(thisObject());
  if (item)
    item->setAttributeNS(nsURI, qName, value);
}

void QDomElementProto:: setAttributeNS(const QString nsURI, const QString& qName, uint value)
{
  QDomElement *item = qscriptvalue_cast<QDomElement*>(thisObject());
  if (item)
    item->setAttributeNS(nsURI, qName, value);
}

QDomAttr QDomElementProto:: setAttributeNode(const QDomAttr& newAttr)
{
  QDomElement *item = qscriptvalue_cast<QDomElement*>(thisObject());
  if (item)
    return item->setAttributeNode(newAttr);
  return QDomAttr();
}

QDomAttr QDomElementProto:: setAttributeNodeNS(const QDomAttr& newAttr)
{
  QDomElement *item = qscriptvalue_cast<QDomElement*>(thisObject());
  if (item)
    return item->setAttributeNodeNS(newAttr);
  return QDomAttr();
}

void QDomElementProto:: setTagName(const QString& name)
{
  QDomElement *item = qscriptvalue_cast<QDomElement*>(thisObject());
  if (item)
    item->setTagName(name);
}

QString QDomElementProto:: tagName() const
{
  QDomElement *item = qscriptvalue_cast<QDomElement*>(thisObject());
  if (item)
    return item->tagName();
  return QString();
}

QString QDomElementProto:: text() const
{
  QDomElement *item = qscriptvalue_cast<QDomElement*>(thisObject());
  if (item)
    return item->text();
  return QString();
}

QString QDomElementProto::toString() const
{
  QDomElement *item = qscriptvalue_cast<QDomElement*>(thisObject());
  if (item)
    return QString("[QDomElement(tagname %1)]").arg(item->tagName());
  return QString("[QDomElement(unknown)]");
}


QDomNode QDomElementProto::appendChild(const QDomNode& newChild)
{
  QDomNode *item = qscriptvalue_cast<QDomElement*>(thisObject());
  if (item)
    return item->appendChild(newChild);
  return QDomNode();
}

QDomNodeList QDomElementProto::childNodes() const
{
  QDomNode *item = qscriptvalue_cast<QDomElement*>(thisObject());
  if (item)
    return item->childNodes();
  return QDomNodeList();
}

void QDomElementProto::clear()
{
  QDomNode *item = qscriptvalue_cast<QDomElement*>(thisObject());
  if (item)
    item->clear();
}

QDomNode QDomElementProto::cloneNode(bool deep) const
{
  QDomNode *item = qscriptvalue_cast<QDomElement*>(thisObject());
  if (item)
    return item->cloneNode(deep);
  return QDomNode();
}

int QDomElementProto::columnNumber() const
{
  QDomNode *item = qscriptvalue_cast<QDomElement*>(thisObject());
  if (item)
    return item->columnNumber();
  return 0;
}

QDomNode QDomElementProto::firstChild() const
{
  QDomNode *item = qscriptvalue_cast<QDomElement*>(thisObject());
  if (item)
    return item->firstChild();
  return QDomNode();
}

QDomElement QDomElementProto::firstChildElement(const QString &tagName) const
{
  QDomNode *item = qscriptvalue_cast<QDomElement*>(thisObject());
  if (item)
    return item->firstChildElement(tagName);
  return QDomElement();
}

bool QDomElementProto::hasAttributes() const
{
  QDomNode *item = qscriptvalue_cast<QDomElement*>(thisObject());
  if (item)
    return item->hasAttributes();
  return false;
}

bool QDomElementProto::hasChildNodes() const
{
  QDomNode *item = qscriptvalue_cast<QDomElement*>(thisObject());
  if (item)
    return item->hasChildNodes();
  return false;
}

QDomNode QDomElementProto::insertAfter(const QDomNode& newChild, const QDomNode& refChild)
{
  QDomNode *item = qscriptvalue_cast<QDomElement*>(thisObject());
  if (item)
    return item->insertAfter(newChild, refChild);
  return QDomNode();
}

QDomNode QDomElementProto::insertBefore(const QDomNode& newChild, const QDomNode& refChild)
{
  QDomNode *item = qscriptvalue_cast<QDomElement*>(thisObject());
  if (item)
    return item->insertBefore(newChild, refChild);
  return QDomNode();
}

bool QDomElementProto::isAttr() const
{
  QDomNode *item = qscriptvalue_cast<QDomElement*>(thisObject());
  if (item)
    return item->isAttr();
  return false;
}

bool QDomElementProto::isCDATASection() const
{
  QDomNode *item = qscriptvalue_cast<QDomElement*>(thisObject());
  if (item)
    return item->isCDATASection();
  return false;
}

bool QDomElementProto::isCharacterData() const
{
  QDomNode *item = qscriptvalue_cast<QDomElement*>(thisObject());
  if (item)
    return item->isCharacterData();
  return false;
}

bool QDomElementProto::isComment() const
{
  QDomNode *item = qscriptvalue_cast<QDomElement*>(thisObject());
  if (item)
    return item->isComment();
  return false;
}

bool QDomElementProto::isDocument() const
{
  QDomNode *item = qscriptvalue_cast<QDomElement*>(thisObject());
  if (item)
    return item->isDocument();
  return false;
}

bool QDomElementProto::isDocumentFragment() const
{
  QDomNode *item = qscriptvalue_cast<QDomElement*>(thisObject());
  if (item)
    return item->isDocumentFragment();
  return false;
}

bool QDomElementProto::isDocumentType() const
{
  QDomNode *item = qscriptvalue_cast<QDomElement*>(thisObject());
  if (item)
    return item->isDocumentType();
  return false;
}

bool QDomElementProto::isElement() const
{
  QDomNode *item = qscriptvalue_cast<QDomElement*>(thisObject());
  if (item)
    return item->isElement();
  return false;
}

bool QDomElementProto::isEntity() const
{
  QDomNode *item = qscriptvalue_cast<QDomElement*>(thisObject());
  if (item)
    return item->isEntity();
  return false;
}

bool QDomElementProto::isEntityReference() const
{
  QDomNode *item = qscriptvalue_cast<QDomElement*>(thisObject());
  if (item)
    return item->isEntityReference();
  return false;
}

bool QDomElementProto::isNotation() const
{
  QDomNode *item = qscriptvalue_cast<QDomElement*>(thisObject());
  if (item)
    return item->isNotation();
  return false;
}

bool QDomElementProto::isNull() const
{
  QDomNode *item = qscriptvalue_cast<QDomElement*>(thisObject());
  if (item)
    return item->isNull();
  return false;
}

bool QDomElementProto::isProcessingInstruction() const
{
  QDomNode *item = qscriptvalue_cast<QDomElement*>(thisObject());
  if (item)
    return item->isProcessingInstruction();
  return false;
}

bool QDomElementProto::isSupported(const QString& feature, const QString& version) const
{
  QDomNode *item = qscriptvalue_cast<QDomElement*>(thisObject());
  if (item)
    return item->isSupported(feature, version);
  return false;
}

bool QDomElementProto::isText() const
{
  QDomNode *item = qscriptvalue_cast<QDomElement*>(thisObject());
  if (item)
    return item->isText();
  return false;
}

QDomNode QDomElementProto::lastChild() const
{
  QDomNode *item = qscriptvalue_cast<QDomElement*>(thisObject());
  if (item)
    return item->lastChild();
  return QDomNode();
}

QDomElement QDomElementProto::lastChildElement(const QString &tagName) const
{
  QDomNode *item = qscriptvalue_cast<QDomElement*>(thisObject());
  if (item)
    return item->lastChildElement(tagName);
  return QDomElement();
}

int QDomElementProto::lineNumber() const
{
  QDomNode *item = qscriptvalue_cast<QDomElement*>(thisObject());
  if (item)
    return item->lineNumber();
  return 0;
}

QString QDomElementProto::localName() const
{
  QDomNode *item = qscriptvalue_cast<QDomElement*>(thisObject());
  if (item)
    return item->localName();
  return QString();
}

QDomNode QDomElementProto::namedItem(const QString& name) const
{
  QDomNode *item = qscriptvalue_cast<QDomElement*>(thisObject());
  if (item)
    return item->namedItem(name);
  return QDomNode();
}

QString QDomElementProto::namespaceURI() const
{
  QDomNode *item = qscriptvalue_cast<QDomElement*>(thisObject());
  if (item)
    return item->namespaceURI();
  return QString();
}

QDomNode QDomElementProto::nextSibling() const
{
  QDomNode *item = qscriptvalue_cast<QDomElement*>(thisObject());
  if (item)
    return item->nextSibling();
  return QDomNode();
}

QDomElement QDomElementProto::nextSiblingElement(const QString &taName) const
{
  QDomNode *item = qscriptvalue_cast<QDomElement*>(thisObject());
  if (item)
    return item->nextSiblingElement(taName);
  return QDomElement();
}

QString QDomElementProto::nodeName() const
{
  QDomNode *item = qscriptvalue_cast<QDomElement*>(thisObject());
  if (item)
    return item->nodeName();
  return QString();
}

QString QDomElementProto::nodeValue() const
{
  QDomNode *item = qscriptvalue_cast<QDomElement*>(thisObject());
  if (item)
    return item->nodeValue();
  return QString();
}

void QDomElementProto::normalize()
{
  QDomNode *item = qscriptvalue_cast<QDomElement*>(thisObject());
  if (item)
    item->normalize();
}

QDomDocument QDomElementProto::ownerDocument() const
{
  QDomNode *item = qscriptvalue_cast<QDomElement*>(thisObject());
  if (item)
    return item->ownerDocument();
  return QDomDocument();
}

QDomNode QDomElementProto::parentNode() const
{
  QDomNode *item = qscriptvalue_cast<QDomElement*>(thisObject());
  if (item)
    return item->parentNode();
  return QDomNode();
}

QString QDomElementProto::prefix() const
{
  QDomNode *item = qscriptvalue_cast<QDomElement*>(thisObject());
  if (item)
    return item->prefix();
  return QString();
}

QDomNode QDomElementProto::previousSibling() const
{
  QDomNode *item = qscriptvalue_cast<QDomElement*>(thisObject());
  if (item)
    return item->previousSibling();
  return QDomNode();
}

QDomElement QDomElementProto::previousSiblingElement(const QString &tagName) const
{
  QDomNode *item = qscriptvalue_cast<QDomElement*>(thisObject());
  if (item)
    return item->previousSiblingElement(tagName);
  return QDomElement();
}

QDomNode QDomElementProto::removeChild(const QDomNode& oldChild)
{
  QDomNode *item = qscriptvalue_cast<QDomElement*>(thisObject());
  if (item)
    return item->removeChild(oldChild);
  return QDomNode();
}

QDomNode QDomElementProto::replaceChild(const QDomNode& newChild, const QDomNode& oldChild)
{
  QDomNode *item = qscriptvalue_cast<QDomElement*>(thisObject());
  if (item)
    return item->replaceChild(newChild, oldChild);
  return QDomNode();
}

void QDomElementProto::save(QTextStream& stream, int intarg) const
{
  QDomNode *item = qscriptvalue_cast<QDomElement*>(thisObject());
  if (item)
    item->save(stream, intarg);
}

void QDomElementProto::save(QTextStream& stream, int intarg, int policy) const
{
  QDomNode *item = qscriptvalue_cast<QDomElement*>(thisObject());
  if (item)
    item->save(stream, intarg, (QDomNode::EncodingPolicy)policy);
}

void QDomElementProto::setNodeValue(const QString&value)
{
  QDomNode *item = qscriptvalue_cast<QDomElement*>(thisObject());
  if (item)
    item->setNodeValue(value);
}

void QDomElementProto::setPrefix(const QString& pre)
{
  QDomNode *item = qscriptvalue_cast<QDomElement*>(thisObject());
  if (item)
    item->setPrefix(pre);
}

QDomAttr QDomElementProto::toAttr() const
{
  QDomNode *item = qscriptvalue_cast<QDomElement*>(thisObject());
  if (item)
    return item->toAttr();
  return QDomAttr();
}

QDomCDATASection QDomElementProto::toCDATASection() const
{
  QDomNode *item = qscriptvalue_cast<QDomElement*>(thisObject());
  if (item)
    return item->toCDATASection();
  return QDomCDATASection();
}

QDomCharacterData QDomElementProto::toCharacterData() const
{
  QDomNode *item = qscriptvalue_cast<QDomElement*>(thisObject());
  if (item)
    return item->toCharacterData();
  return QDomCharacterData();
}

QDomComment QDomElementProto::toComment() const
{
  QDomNode *item = qscriptvalue_cast<QDomElement*>(thisObject());
  if (item)
    return item->toComment();
  return QDomComment();
}

QDomDocument QDomElementProto::toDocument() const
{
  QDomNode *item = qscriptvalue_cast<QDomElement*>(thisObject());
  if (item)
    return item->toDocument();
  return QDomDocument();
}

QDomDocumentFragment QDomElementProto::toDocumentFragment() const
{
  QDomNode *item = qscriptvalue_cast<QDomElement*>(thisObject());
  if (item)
    return item->toDocumentFragment();
  return QDomDocumentFragment();
}

QDomDocumentType QDomElementProto::toDocumentType() const
{
  QDomNode *item = qscriptvalue_cast<QDomElement*>(thisObject());
  if (item)
    return item->toDocumentType();
  return QDomDocumentType();
}

QDomElement QDomElementProto::toElement() const
{
  QDomNode *item = qscriptvalue_cast<QDomElement*>(thisObject());
  if (item)
    return item->toElement();
  return QDomElement();
}

QDomEntity QDomElementProto::toEntity() const
{
  QDomNode *item = qscriptvalue_cast<QDomElement*>(thisObject());
  if (item)
    return item->toEntity();
  return QDomEntity();
}

QDomEntityReference QDomElementProto::toEntityReference() const
{
  QDomNode *item = qscriptvalue_cast<QDomElement*>(thisObject());
  if (item)
    return item->toEntityReference();
  return QDomEntityReference();
}

QDomNotation QDomElementProto::toNotation() const
{
  QDomNode *item = qscriptvalue_cast<QDomElement*>(thisObject());
  if (item)
    return item->toNotation();
  return QDomNotation();
}

QDomProcessingInstruction QDomElementProto::toProcessingInstruction() const
{
  QDomNode *item = qscriptvalue_cast<QDomElement*>(thisObject());
  if (item)
    return item->toProcessingInstruction();
  return QDomProcessingInstruction();
}

QDomText QDomElementProto::toText() const
{
  QDomNode *item = qscriptvalue_cast<QDomElement*>(thisObject());
  if (item)
    return item->toText();
  return QDomText();
}
