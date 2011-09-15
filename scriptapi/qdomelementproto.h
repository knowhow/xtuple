/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef __QDOMELEMENTPROTO_H__
#define __QDOMELEMENTPROTO_H__

#include <QDomAttr>
#include <QDomElement>
#include <QDomNamedNodeMap>
#include <QDomNode>
#include <QDomNodeList>
#include <QObject>
#include <QtScript>

class QDomCDATASection;
class QDomCharacterData;
class QDomComment;
class QDomDocument;
class QDomDocumentFragment;
class QDomDocumentType;
class QDomEntity;
class QDomEntityReference;
class QDomImplementation;
class QDomNotation;
class QDomProcessingInstruction;
class QDomText;

Q_DECLARE_METATYPE(QDomElement*)
Q_DECLARE_METATYPE(QDomElement)

void setupQDomElementProto(QScriptEngine *engine);
QScriptValue constructQDomElement(QScriptContext *context, QScriptEngine *engine);

class QDomElementProto : public QObject, public QScriptable
{
  Q_OBJECT

  public:
    QDomElementProto(QObject *parent);

    Q_INVOKABLE QString  attribute(const QString& name, const QString& defValue = QString() ) const;
    Q_INVOKABLE QString  attributeNS(const QString nsURI, const QString& localName, const QString& defValue = QString()) const;
    Q_INVOKABLE QDomAttr attributeNode(const QString& name);
    Q_INVOKABLE QDomAttr attributeNodeNS(const QString& nsURI, const QString& localName);
    Q_INVOKABLE QDomNamedNodeMap attributes() const;
    Q_INVOKABLE QDomNodeList     elementsByTagName(const QString& tagname) const;
    Q_INVOKABLE QDomNodeList     elementsByTagNameNS(const QString& nsURI, const QString& localName) const;
    Q_INVOKABLE bool     hasAttribute(const QString& name) const;
    Q_INVOKABLE bool     hasAttributeNS(const QString& nsURI, const QString& localName) const;
    Q_INVOKABLE int      nodeType() const;
    Q_INVOKABLE void     removeAttribute(const QString& name);
    Q_INVOKABLE void     removeAttributeNS(const QString& nsURI, const QString& localName);
    Q_INVOKABLE QDomAttr removeAttributeNode(const QDomAttr& oldAttr);
    Q_INVOKABLE void     setAttribute(const QString& name, const QString& value);
    Q_INVOKABLE void     setAttribute(const QString& name, double value);
    Q_INVOKABLE void     setAttribute(const QString& name, float value);
    Q_INVOKABLE void     setAttribute(const QString& name, int value);
    Q_INVOKABLE void     setAttribute(const QString& name, qlonglong value);
    Q_INVOKABLE void     setAttribute(const QString& name, qulonglong value);
    Q_INVOKABLE void     setAttribute(const QString& name, uint value);
    Q_INVOKABLE void     setAttributeNS(const QString nsURI, const QString& qName, const QString& value);
    Q_INVOKABLE void     setAttributeNS(const QString nsURI, const QString& qName, double value);
    Q_INVOKABLE void     setAttributeNS(const QString nsURI, const QString& qName, int value);
    Q_INVOKABLE void     setAttributeNS(const QString nsURI, const QString& qName, qlonglong value);
    Q_INVOKABLE void     setAttributeNS(const QString nsURI, const QString& qName, qulonglong value);
    Q_INVOKABLE void     setAttributeNS(const QString nsURI, const QString& qName, uint value);
    Q_INVOKABLE QDomAttr setAttributeNode(const QDomAttr& newAttr);
    Q_INVOKABLE QDomAttr setAttributeNodeNS(const QDomAttr& newAttr);
    Q_INVOKABLE void     setTagName(const QString& name);
    Q_INVOKABLE QString  tagName()  const;
    Q_INVOKABLE QString  text()     const;
    Q_INVOKABLE QString  toString() const;

    // now for the inherited stuff from QDomNode
    Q_INVOKABLE QDomNode         appendChild(const QDomNode& newChild);
    Q_INVOKABLE QDomNodeList childNodes() const;
    Q_INVOKABLE void         clear();
    Q_INVOKABLE QDomNode     cloneNode(bool deep = true) const;
    Q_INVOKABLE int          columnNumber() const;
    Q_INVOKABLE QDomNode     firstChild() const;
    Q_INVOKABLE QDomElement  firstChildElement(const QString &tagName = QString()) const;
    Q_INVOKABLE bool         hasAttributes() const;
    Q_INVOKABLE bool         hasChildNodes() const;
    Q_INVOKABLE QDomNode     insertAfter(const QDomNode& newChild, const QDomNode& refChild);
    Q_INVOKABLE QDomNode     insertBefore(const QDomNode& newChild, const QDomNode& refChild);
    Q_INVOKABLE bool         isAttr() const;
    Q_INVOKABLE bool         isCDATASection() const;
    Q_INVOKABLE bool         isCharacterData() const;
    Q_INVOKABLE bool         isComment() const;
    Q_INVOKABLE bool         isDocument() const;
    Q_INVOKABLE bool         isDocumentFragment() const;
    Q_INVOKABLE bool         isDocumentType() const;
    Q_INVOKABLE bool         isElement() const;
    Q_INVOKABLE bool         isEntity() const;
    Q_INVOKABLE bool         isEntityReference() const;
    Q_INVOKABLE bool         isNotation() const;
    Q_INVOKABLE bool         isNull() const;
    Q_INVOKABLE bool         isProcessingInstruction() const;
    Q_INVOKABLE bool         isSupported(const QString& feature, const QString& version) const;
    Q_INVOKABLE bool         isText() const;
    Q_INVOKABLE QDomNode     lastChild() const;
    Q_INVOKABLE QDomElement  lastChildElement(const QString &tagName = QString()) const;
    Q_INVOKABLE int          lineNumber() const;
    Q_INVOKABLE QString      localName() const;
    Q_INVOKABLE QDomNode     namedItem(const QString& name) const;
    Q_INVOKABLE QString      namespaceURI() const;
    Q_INVOKABLE QDomNode     nextSibling() const;
    Q_INVOKABLE QDomElement  nextSiblingElement(const QString &taName = QString()) const;
    Q_INVOKABLE QString      nodeName()        const;
    Q_INVOKABLE QString      nodeValue()       const;
    Q_INVOKABLE void         normalize();
    Q_INVOKABLE QDomDocument ownerDocument()   const;
    Q_INVOKABLE QDomNode     parentNode()      const;
    Q_INVOKABLE QString      prefix()          const;
    Q_INVOKABLE QDomNode     previousSibling() const;
    Q_INVOKABLE QDomElement  previousSiblingElement(const QString &tagName = QString()) const;
    Q_INVOKABLE QDomNode     removeChild(const QDomNode& oldChild);
    Q_INVOKABLE QDomNode     replaceChild(const QDomNode& newChild, const QDomNode& oldChild);
    Q_INVOKABLE void         save(QTextStream&, int)      const;
    Q_INVOKABLE void         save(QTextStream&, int, int) const;
    Q_INVOKABLE void         setNodeValue(const QString&);
    Q_INVOKABLE void         setPrefix(const QString& pre);
    Q_INVOKABLE QDomAttr     toAttr()                               const;
    Q_INVOKABLE QDomCDATASection          toCDATASection()          const;
    Q_INVOKABLE QDomCharacterData         toCharacterData()         const;
    Q_INVOKABLE QDomComment               toComment()               const;
    Q_INVOKABLE QDomDocument              toDocument()              const;
    Q_INVOKABLE QDomDocumentFragment      toDocumentFragment()      const;
    Q_INVOKABLE QDomDocumentType          toDocumentType()          const;
    Q_INVOKABLE QDomElement               toElement()               const;
    Q_INVOKABLE QDomEntity                toEntity()                const;
    Q_INVOKABLE QDomEntityReference       toEntityReference()       const;
    Q_INVOKABLE QDomNotation              toNotation()              const;
    Q_INVOKABLE QDomProcessingInstruction toProcessingInstruction() const;
    Q_INVOKABLE QDomText                  toText()                  const;

};

#endif
