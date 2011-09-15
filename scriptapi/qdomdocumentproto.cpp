/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "qdomdocumentproto.h"

#include <QDomAttr>
#include <QDomCDATASection>
#include <QDomComment>
#include <QDomDocument>
#include <QDomDocumentFragment>
#include <QDomDocumentType>
#include <QDomElement>
#include <QDomEntityReference>
#include <QDomImplementation>
#include <QDomNode>
#include <QDomNodeList>
#include <QDomProcessingInstruction>
#include <QDomText>

void setupQDomDocumentProto(QScriptEngine *engine)
{
  QScriptValue proto = engine->newQObject(new QDomDocumentProto(engine));
  engine->setDefaultPrototype(qMetaTypeId<QDomDocument*>(), proto);
  engine->setDefaultPrototype(qMetaTypeId<QDomDocument>(),  proto);

  QScriptValue constructor = engine->newFunction(constructQDomDocument,
                                                 proto);
  engine->globalObject().setProperty("QDomDocument",  constructor);
}

QScriptValue constructQDomDocument(QScriptContext * /*context*/,
                                    QScriptEngine  *engine)
{
  QDomDocument *obj = 0;
  // if (context->argumentCount() > 0)
  // else
    obj = new QDomDocument();
  return engine->toScriptValue(obj);
}

QDomDocumentProto::QDomDocumentProto(QObject *parent)
    : QObject(parent)
{
}

QDomAttr QDomDocumentProto::createAttribute(const QString& name)
{
  QDomDocument *item = qscriptvalue_cast<QDomDocument*>(thisObject());
  if (item)
    return item->createAttribute(name);
  return QDomAttr();
}

QDomAttr QDomDocumentProto::createAttributeNS(const QString& nsURI, const QString& qName)
{
  QDomDocument *item = qscriptvalue_cast<QDomDocument*>(thisObject());
  if (item)
    return item->createAttributeNS(nsURI, qName);
  return QDomAttr();
}

QDomCDATASection QDomDocumentProto::createCDATASection(const QString& data)
{
  QDomDocument *item = qscriptvalue_cast<QDomDocument*>(thisObject());
  if (item)
    return item->createCDATASection(data);
  return QDomCDATASection();
}

QDomComment QDomDocumentProto::createComment(const QString& data)
{
  QDomDocument *item = qscriptvalue_cast<QDomDocument*>(thisObject());
  if (item)
    return item->createComment(data);
  return QDomComment();
}

QDomDocumentFragment QDomDocumentProto::createDocumentFragment()
{
  QDomDocument *item = qscriptvalue_cast<QDomDocument*>(thisObject());
  if (item)
    return item->createDocumentFragment();
  return QDomDocumentFragment();
}

QDomElement QDomDocumentProto::createElement(const QString& tagName)
{
  QDomDocument *item = qscriptvalue_cast<QDomDocument*>(thisObject());
  if (item)
    return item->createElement(tagName);
  return QDomElement();
}

QDomElement QDomDocumentProto::createElementNS(const QString& nsURI, const QString& qName)
{
  QDomDocument *item = qscriptvalue_cast<QDomDocument*>(thisObject());
  if (item)
    return item->createElementNS(nsURI, qName);
  return QDomElement();
}

QDomEntityReference QDomDocumentProto::createEntityReference(const QString& name)
{
  QDomDocument *item = qscriptvalue_cast<QDomDocument*>(thisObject());
  if (item)
    return item->createEntityReference(name);
  return QDomEntityReference();
}

QDomProcessingInstruction QDomDocumentProto::createProcessingInstruction(const QString& target, const QString& data)
{
  QDomDocument *item = qscriptvalue_cast<QDomDocument*>(thisObject());
  if (item)
    return item->createProcessingInstruction(target, data);
  return QDomProcessingInstruction();
}

QDomText QDomDocumentProto::createTextNode(const QString& data)
{
  QDomDocument *item = qscriptvalue_cast<QDomDocument*>(thisObject());
  if (item)
    return item->createTextNode(data);
  return QDomText();
}

QDomDocumentType QDomDocumentProto::doctype() const
{
  QDomDocument *item = qscriptvalue_cast<QDomDocument*>(thisObject());
  if (item)
    return item->doctype();
  return QDomDocumentType();
}

QDomElement QDomDocumentProto::documentElement() const
{
  QDomDocument *item = qscriptvalue_cast<QDomDocument*>(thisObject());
  if (item)
    return item->documentElement();
  return QDomElement();
}

QDomElement QDomDocumentProto::elementById(const QString& elementId)
{
  QDomDocument *item = qscriptvalue_cast<QDomDocument*>(thisObject());
  if (item)
    return item->elementById(elementId);
  return QDomElement();
}

QDomNodeList QDomDocumentProto::elementsByTagName(const QString& tagname) const
{
  QDomDocument *item = qscriptvalue_cast<QDomDocument*>(thisObject());
  if (item)
    return item->elementsByTagName(tagname);
  return QDomNodeList();
}

QDomNodeList QDomDocumentProto::elementsByTagNameNS(const QString& nsURI, const QString& localName)
{
  QDomDocument *item = qscriptvalue_cast<QDomDocument*>(thisObject());
  if (item)
    return item->elementsByTagNameNS(nsURI, localName);
  return QDomNodeList();
}

QDomImplementation QDomDocumentProto::implementation() const
{
  QDomDocument *item = qscriptvalue_cast<QDomDocument*>(thisObject());
  if (item)
    return item->implementation();
  return QDomImplementation();
}

QDomNode QDomDocumentProto::importNode(const QDomNode& importedNode, bool deep)
{
  QDomDocument *item = qscriptvalue_cast<QDomDocument*>(thisObject());
  if (item)
    return item->importNode(importedNode, deep);
  return QDomNode();
}

int QDomDocumentProto::nodeType() const
{
  QDomDocument *item = qscriptvalue_cast<QDomDocument*>(thisObject());
  if (item)
    return item->nodeType();
  return 0;
}

bool QDomDocumentProto::setContent(QIODevice* dev, QString *errorMsg, int *errorLine, int *errorColumn)
{
  QDomDocument *item = qscriptvalue_cast<QDomDocument*>(thisObject());
  if (item)
    return item->setContent(dev, errorMsg, errorLine, errorColumn );
  return false;
}

bool QDomDocumentProto::setContent(QIODevice* dev, bool namespaceProcessing, QString *errorMsg, int *errorLine, int *errorColumn)
{
  QDomDocument *item = qscriptvalue_cast<QDomDocument*>(thisObject());
  if (item)
    return item->setContent(dev, namespaceProcessing, errorMsg, errorLine, errorColumn);
  return false;
}

bool QDomDocumentProto::setContent(QXmlInputSource *source, QXmlReader *reader, QString *errorMsg, int *errorLine, int *errorColumn)
{
  QDomDocument *item = qscriptvalue_cast<QDomDocument*>(thisObject());
  if (item)
    return item->setContent(source, reader, errorMsg, errorLine, errorColumn);
  return false;
}

bool QDomDocumentProto::setContent(const QByteArray& text, QString *errorMsg, int *errorLine, int *errorColumn)
{
  QDomDocument *item = qscriptvalue_cast<QDomDocument*>(thisObject());
  if (item)
    return item->setContent(text, errorMsg, errorLine, errorColumn);
  return false;
}

bool QDomDocumentProto::setContent(const QByteArray& text, bool namespaceProcessing, QString *errorMsg, int *errorLine, int *errorColumn )
{
  QDomDocument *item = qscriptvalue_cast<QDomDocument*>(thisObject());
  if (item)
    return item->setContent(text, namespaceProcessing, errorMsg, errorLine, errorColumn);
  return false;
}

bool QDomDocumentProto::setContent(const QString& text, QString *errorMsg, int *errorLine, int *errorColumn )
{
  QDomDocument *item = qscriptvalue_cast<QDomDocument*>(thisObject());
  if (item)
    return item->setContent(text, errorMsg, errorLine, errorColumn);
  return false;
}

bool QDomDocumentProto::setContent(const QString& text, bool namespaceProcessing, QString *errorMsg, int *errorLine, int *errorColumn )
{
  QDomDocument *item = qscriptvalue_cast<QDomDocument*>(thisObject());
  if (item)
    return item->setContent(text, namespaceProcessing, errorMsg, errorLine, errorColumn);
  return false;
}

QByteArray QDomDocumentProto::toByteArray(int arg) const
{
  QDomDocument *item = qscriptvalue_cast<QDomDocument*>(thisObject());
  if (item)
    return item->toByteArray(arg);
  return QByteArray();
}

QString QDomDocumentProto::toString(int arg) const
{
  QDomDocument *item = qscriptvalue_cast<QDomDocument*>(thisObject());
  if (item)
    return item->toString(arg);
  return QString();
}

// now for the inherited domnode methods
QDomNode QDomDocumentProto::appendChild(const QDomNode& newChild)
{
  QDomDocument *item = qscriptvalue_cast<QDomDocument*>(thisObject());
  if (item)
    return item->appendChild(newChild);
  return QDomDocument();
}

QDomNamedNodeMap QDomDocumentProto::attributes() const
{
  QDomDocument *item = qscriptvalue_cast<QDomDocument*>(thisObject());
  if (item)
    return item->attributes();
  return QDomNamedNodeMap();
}

QDomNodeList QDomDocumentProto::childNodes() const
{
  QDomDocument *item = qscriptvalue_cast<QDomDocument*>(thisObject());
  if (item)
    return item->childNodes();
  return QDomNodeList();
}

void QDomDocumentProto::clear()
{
  QDomDocument *item = qscriptvalue_cast<QDomDocument*>(thisObject());
  if (item)
    item->clear();
}

QDomNode QDomDocumentProto::cloneNode(bool deep) const
{
  QDomDocument *item = qscriptvalue_cast<QDomDocument*>(thisObject());
  if (item)
    return item->cloneNode(deep);
  return QDomNode();
}

int QDomDocumentProto::columnNumber() const
{
  QDomDocument *item = qscriptvalue_cast<QDomDocument*>(thisObject());
  if (item)
    return item->columnNumber();
  return 0;
}

QDomNode QDomDocumentProto::firstChild() const
{
  QDomDocument *item = qscriptvalue_cast<QDomDocument*>(thisObject());
  if (item)
    return item->firstChild();
  return QDomNode();
}

QDomElement QDomDocumentProto::firstChildElement(const QString &tagName) const
{
  QDomDocument *item = qscriptvalue_cast<QDomDocument*>(thisObject());
  if (item)
    return item->firstChildElement(tagName);
  return QDomElement();
}

bool QDomDocumentProto::hasAttributes() const
{
  QDomDocument *item = qscriptvalue_cast<QDomDocument*>(thisObject());
  if (item)
    return item->hasAttributes();
  return false;
}

bool QDomDocumentProto::hasChildNodes() const
{
  QDomDocument *item = qscriptvalue_cast<QDomDocument*>(thisObject());
  if (item)
    return item->hasChildNodes();
  return false;
}

QDomNode QDomDocumentProto::insertAfter(const QDomNode& newChild, const QDomNode& refChild)
{
  QDomDocument *item = qscriptvalue_cast<QDomDocument*>(thisObject());
  if (item)
    return item->insertAfter(newChild, refChild);
  return QDomNode();
}

QDomNode QDomDocumentProto::insertBefore(const QDomNode& newChild, const QDomNode& refChild)
{
  QDomDocument *item = qscriptvalue_cast<QDomDocument*>(thisObject());
  if (item)
    return item->insertBefore(newChild, refChild);
  return QDomNode();
}

bool QDomDocumentProto::isAttr() const
{
  QDomDocument *item = qscriptvalue_cast<QDomDocument*>(thisObject());
  if (item)
    return item->isAttr();
  return false;
}

bool QDomDocumentProto::isCDATASection() const
{
  QDomDocument *item = qscriptvalue_cast<QDomDocument*>(thisObject());
  if (item)
    return item->isCDATASection();
  return false;
}

bool QDomDocumentProto::isCharacterData() const
{
  QDomDocument *item = qscriptvalue_cast<QDomDocument*>(thisObject());
  if (item)
    return item->isCharacterData();
  return false;
}

bool QDomDocumentProto::isComment() const
{
  QDomDocument *item = qscriptvalue_cast<QDomDocument*>(thisObject());
  if (item)
    return item->isComment();
  return false;
}

bool QDomDocumentProto::isDocument() const
{
  QDomDocument *item = qscriptvalue_cast<QDomDocument*>(thisObject());
  if (item)
    return item->isDocument();
  return false;
}

bool QDomDocumentProto::isDocumentFragment() const
{
  QDomDocument *item = qscriptvalue_cast<QDomDocument*>(thisObject());
  if (item)
    return item->isDocumentFragment();
  return false;
}

bool QDomDocumentProto::isDocumentType() const
{
  QDomDocument *item = qscriptvalue_cast<QDomDocument*>(thisObject());
  if (item)
    return item->isDocumentType();
  return false;
}

bool QDomDocumentProto::isElement() const
{
  QDomDocument *item = qscriptvalue_cast<QDomDocument*>(thisObject());
  if (item)
    return item->isElement();
  return false;
}

bool QDomDocumentProto::isEntity() const
{
  QDomDocument *item = qscriptvalue_cast<QDomDocument*>(thisObject());
  if (item)
    return item->isEntity();
  return false;
}

bool QDomDocumentProto::isEntityReference() const
{
  QDomDocument *item = qscriptvalue_cast<QDomDocument*>(thisObject());
  if (item)
    return item->isEntityReference();
  return false;
}

bool QDomDocumentProto::isNotation() const
{
  QDomDocument *item = qscriptvalue_cast<QDomDocument*>(thisObject());
  if (item)
    return item->isNotation();
  return false;
}

bool QDomDocumentProto::isNull() const
{
  QDomDocument *item = qscriptvalue_cast<QDomDocument*>(thisObject());
  if (item)
    return item->isNull();
  return false;
}

bool QDomDocumentProto::isProcessingInstruction() const
{
  QDomDocument *item = qscriptvalue_cast<QDomDocument*>(thisObject());
  if (item)
    return item->isProcessingInstruction();
  return false;
}

bool QDomDocumentProto::isSupported(const QString& feature, const QString& version) const
{
  QDomDocument *item = qscriptvalue_cast<QDomDocument*>(thisObject());
  if (item)
    return item->isSupported(feature, version);
  return false;
}

bool QDomDocumentProto::isText() const
{
  QDomDocument *item = qscriptvalue_cast<QDomDocument*>(thisObject());
  if (item)
    return item->isText();
  return false;
}

QDomNode QDomDocumentProto::lastChild() const
{
  QDomDocument *item = qscriptvalue_cast<QDomDocument*>(thisObject());
  if (item)
    return item->lastChild();
  return QDomNode();
}

QDomElement QDomDocumentProto::lastChildElement(const QString &tagName) const
{
  QDomDocument *item = qscriptvalue_cast<QDomDocument*>(thisObject());
  if (item)
    return item->lastChildElement(tagName);
  return QDomElement();
}

int QDomDocumentProto::lineNumber() const
{
  QDomDocument *item = qscriptvalue_cast<QDomDocument*>(thisObject());
  if (item)
    return item->lineNumber();
  return 0;
}

QString QDomDocumentProto::localName() const
{
  QDomDocument *item = qscriptvalue_cast<QDomDocument*>(thisObject());
  if (item)
    return item->localName();
  return QString();
}

QDomNode QDomDocumentProto::namedItem(const QString& name) const
{
  QDomDocument *item = qscriptvalue_cast<QDomDocument*>(thisObject());
  if (item)
    return item->namedItem(name);
  return QDomNode();
}

QString QDomDocumentProto::namespaceURI() const
{
  QDomDocument *item = qscriptvalue_cast<QDomDocument*>(thisObject());
  if (item)
    return item->namespaceURI();
  return QString();
}

QDomNode QDomDocumentProto::nextSibling() const
{
  QDomDocument *item = qscriptvalue_cast<QDomDocument*>(thisObject());
  if (item)
    return item->nextSibling();
  return QDomNode();
}

QDomElement QDomDocumentProto::nextSiblingElement(const QString &taName) const
{
  QDomDocument *item = qscriptvalue_cast<QDomDocument*>(thisObject());
  if (item)
    return item->nextSiblingElement(taName);
  return QDomElement();
}

QString QDomDocumentProto::nodeName() const
{
  QDomDocument *item = qscriptvalue_cast<QDomDocument*>(thisObject());
  if (item)
    return item->nodeName();
  return QString();
}

QString QDomDocumentProto::nodeValue() const
{
  QDomDocument *item = qscriptvalue_cast<QDomDocument*>(thisObject());
  if (item)
    return item->nodeValue();
  return QString();
}

void QDomDocumentProto::normalize()
{
  QDomDocument *item = qscriptvalue_cast<QDomDocument*>(thisObject());
  if (item)
    item->normalize();
}

QDomDocument QDomDocumentProto::ownerDocument() const
{
  QDomDocument *item = qscriptvalue_cast<QDomDocument*>(thisObject());
  if (item)
    return item->ownerDocument();
  return QDomDocument();
}

QDomNode QDomDocumentProto::parentNode() const
{
  QDomDocument *item = qscriptvalue_cast<QDomDocument*>(thisObject());
  if (item)
    return item->parentNode();
  return QDomNode();
}

QString QDomDocumentProto::prefix() const
{
  QDomDocument *item = qscriptvalue_cast<QDomDocument*>(thisObject());
  if (item)
    return item->prefix();
  return QString();
}

QDomNode QDomDocumentProto::previousSibling() const
{
  QDomDocument *item = qscriptvalue_cast<QDomDocument*>(thisObject());
  if (item)
    return item->previousSibling();
  return QDomNode();
}

QDomElement QDomDocumentProto::previousSiblingElement(const QString &tagName) const
{
  QDomDocument *item = qscriptvalue_cast<QDomDocument*>(thisObject());
  if (item)
    return item->previousSiblingElement(tagName);
  return QDomElement();
}

QDomNode QDomDocumentProto::removeChild(const QDomNode& oldChild)
{
  QDomDocument *item = qscriptvalue_cast<QDomDocument*>(thisObject());
  if (item)
    return item->removeChild(oldChild);
  return QDomNode();
}

QDomNode QDomDocumentProto::replaceChild(const QDomNode& newChild, const QDomNode& oldChild)
{
  QDomDocument *item = qscriptvalue_cast<QDomDocument*>(thisObject());
  if (item)
    return item->replaceChild(newChild, oldChild);
  return QDomNode();
}

void QDomDocumentProto::save(QTextStream& stream, int intarg) const
{
  QDomDocument *item = qscriptvalue_cast<QDomDocument*>(thisObject());
  if (item)
    item->save(stream, intarg);
}

void QDomDocumentProto::save(QTextStream& stream, int intarg, int policy) const
{
  QDomDocument *item = qscriptvalue_cast<QDomDocument*>(thisObject());
  if (item)
    item->save(stream, intarg, (QDomNode::EncodingPolicy)policy);
}

void QDomDocumentProto::setNodeValue(const QString&value)
{
  QDomDocument *item = qscriptvalue_cast<QDomDocument*>(thisObject());
  if (item)
    item->setNodeValue(value);
}

void QDomDocumentProto::setPrefix(const QString& pre)
{
  QDomDocument *item = qscriptvalue_cast<QDomDocument*>(thisObject());
  if (item)
    item->setPrefix(pre);
}

QDomAttr QDomDocumentProto::toAttr() const
{
  QDomDocument *item = qscriptvalue_cast<QDomDocument*>(thisObject());
  if (item)
    return item->toAttr();
  return QDomAttr();
}

QDomCDATASection QDomDocumentProto::toCDATASection() const
{
  QDomDocument *item = qscriptvalue_cast<QDomDocument*>(thisObject());
  if (item)
    return item->toCDATASection();
  return QDomCDATASection();
}

QDomCharacterData QDomDocumentProto::toCharacterData() const
{
  QDomDocument *item = qscriptvalue_cast<QDomDocument*>(thisObject());
  if (item)
    return item->toCharacterData();
  return QDomCharacterData();
}

QDomComment QDomDocumentProto::toComment() const
{
  QDomDocument *item = qscriptvalue_cast<QDomDocument*>(thisObject());
  if (item)
    return item->toComment();
  return QDomComment();
}

QDomDocument QDomDocumentProto::toDocument() const
{
  QDomDocument *item = qscriptvalue_cast<QDomDocument*>(thisObject());
  if (item)
    return item->toDocument();
  return QDomDocument();
}

QDomDocumentFragment QDomDocumentProto::toDocumentFragment() const
{
  QDomDocument *item = qscriptvalue_cast<QDomDocument*>(thisObject());
  if (item)
    return item->toDocumentFragment();
  return QDomDocumentFragment();
}

QDomDocumentType QDomDocumentProto::toDocumentType() const
{
  QDomDocument *item = qscriptvalue_cast<QDomDocument*>(thisObject());
  if (item)
    return item->toDocumentType();
  return QDomDocumentType();
}

QDomElement QDomDocumentProto::toElement() const
{
  QDomDocument *item = qscriptvalue_cast<QDomDocument*>(thisObject());
  if (item)
    return item->toElement();
  return QDomElement();
}

QDomEntity QDomDocumentProto::toEntity() const
{
  QDomDocument *item = qscriptvalue_cast<QDomDocument*>(thisObject());
  if (item)
    return item->toEntity();
  return QDomEntity();
}

QDomEntityReference QDomDocumentProto::toEntityReference() const
{
  QDomDocument *item = qscriptvalue_cast<QDomDocument*>(thisObject());
  if (item)
    return item->toEntityReference();
  return QDomEntityReference();
}

QDomNotation QDomDocumentProto::toNotation() const
{
  QDomDocument *item = qscriptvalue_cast<QDomDocument*>(thisObject());
  if (item)
    return item->toNotation();
  return QDomNotation();
}

QDomProcessingInstruction QDomDocumentProto::toProcessingInstruction() const
{
  QDomDocument *item = qscriptvalue_cast<QDomDocument*>(thisObject());
  if (item)
    return item->toProcessingInstruction();
  return QDomProcessingInstruction();
}

QDomText QDomDocumentProto::toText() const
{
  QDomDocument *item = qscriptvalue_cast<QDomDocument*>(thisObject());
  if (item)
    return item->toText();
  return QDomText();
}
