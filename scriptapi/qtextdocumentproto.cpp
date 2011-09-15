/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "qtextdocumentproto.h"

#include <QString>

void setupQTextDocumentProto(QScriptEngine *engine)
{
  QScriptValue proto = engine->newQObject(new QTextDocumentProto(engine));
  engine->setDefaultPrototype(qMetaTypeId<QTextDocument*>(), proto);
  // engine->setDefaultPrototype(qMetaTypeId<QTextDocument>(),  proto);

  QScriptValue constructor = engine->newFunction(constructQTextDocument,
                                                 proto);
  engine->globalObject().setProperty("QTextDocument",  constructor);
}

QScriptValue constructQTextDocument(QScriptContext *context,
                                    QScriptEngine  *engine)
{
  QTextDocument *obj = 0;
  switch (context->argumentCount())
  {
    case 0: obj = new QTextDocument(); break;
    case 1: obj = new QTextDocument(context->argument(0).toQObject()); break;
    case 2:
    default:
            obj = new QTextDocument(context->argument(0).toString(),
                                    context->argument(0).toQObject()); break;
  }
  return engine->toScriptValue(obj);
}

QTextDocumentProto::QTextDocumentProto(QObject *parent)
    : QObject(parent)
{
}

void QTextDocumentProto::addResource(int type, const QUrl &name, const QVariant &resource)
{
  QTextDocument *item = qscriptvalue_cast<QTextDocument*>(thisObject());
  if (item)
    item->addResource(type, name, resource);
}

void QTextDocumentProto::adjustSize()
{
  QTextDocument *item = qscriptvalue_cast<QTextDocument*>(thisObject());
  if (item)
    item->adjustSize();
}

QVector<QTextFormat> QTextDocumentProto::allFormats() const
{
  QTextDocument *item = qscriptvalue_cast<QTextDocument*>(thisObject());
  if (item)
    return item->allFormats();
  return QVector<QTextFormat>();
}

QTextBlock QTextDocumentProto::begin() const
{
  QTextDocument *item = qscriptvalue_cast<QTextDocument*>(thisObject());
  if (item)
    return item->begin();
  return QTextBlock();
}

QChar QTextDocumentProto::characterAt(int pos) const
{
  QTextDocument *item = qscriptvalue_cast<QTextDocument*>(thisObject());
  if (item)
    return item->characterAt(pos);
  return QChar();
}

int QTextDocumentProto::characterCount() const
{
  QTextDocument *item = qscriptvalue_cast<QTextDocument*>(thisObject());
  if (item)
    return item->characterCount();
  return 0;
}

void QTextDocumentProto::clear()
{
  QTextDocument *item = qscriptvalue_cast<QTextDocument*>(thisObject());
  if (item)
    item->clear();
}

QTextDocument *QTextDocumentProto::clone(QObject *parent) const
{
  QTextDocument *item = qscriptvalue_cast<QTextDocument*>(thisObject());
  if (item)
    return item->clone(parent);
  return 0;
}

QAbstractTextDocumentLayout *QTextDocumentProto::documentLayout() const
{
  QTextDocument *item = qscriptvalue_cast<QTextDocument*>(thisObject());
  if (item)
    return item->documentLayout();
  return 0;
}

void QTextDocumentProto::drawContents(QPainter *p, const QRectF &rect)
{
  QTextDocument *item = qscriptvalue_cast<QTextDocument*>(thisObject());
  if (item)
    item->drawContents(p, rect);
}

QTextBlock QTextDocumentProto::end() const
{
  QTextDocument *item = qscriptvalue_cast<QTextDocument*>(thisObject());
  if (item)
    return item->end();
  return QTextBlock();
}

QTextCursor QTextDocumentProto::find(const QString &subString, const QTextCursor &cursor, int options) const
{
  QTextDocument *item = qscriptvalue_cast<QTextDocument*>(thisObject());
  if (item)
    return item->find(subString, cursor, (QTextDocument::FindFlags)options);
  return QTextCursor();
}

QTextCursor QTextDocumentProto::find(const QRegExp &expr, const QTextCursor &cursor, int options) const
{
  QTextDocument *item = qscriptvalue_cast<QTextDocument*>(thisObject());
  if (item)
    return item->find(expr, cursor, (QTextDocument::FindFlags)options);
  return QTextCursor();
}

QTextCursor QTextDocumentProto::find(const QString &subString, int position, int options) const
{
  QTextDocument *item = qscriptvalue_cast<QTextDocument*>(thisObject());
  if (item)
    return item->find(subString, position, (QTextDocument::FindFlags)options);
  return QTextCursor();
}

QTextCursor QTextDocumentProto::find(const QRegExp &expr, int position, int options) const
{
  QTextDocument *item = qscriptvalue_cast<QTextDocument*>(thisObject());
  if (item)
    return item->find(expr, position, (QTextDocument::FindFlags)options);
  return QTextCursor();
}

QTextBlock QTextDocumentProto::findBlock(int pos) const
{
  QTextDocument *item = qscriptvalue_cast<QTextDocument*>(thisObject());
  if (item)
    return item->findBlock(pos);
  return QTextBlock();
}

QTextBlock QTextDocumentProto::findBlockByLineNumber(int lineNumber) const
{
  QTextDocument *item = qscriptvalue_cast<QTextDocument*>(thisObject());
  if (item)
    return item->findBlockByLineNumber(lineNumber);
  return QTextBlock();
}

QTextBlock QTextDocumentProto::findBlockByNumber(int blockNumber) const
{
  QTextDocument *item = qscriptvalue_cast<QTextDocument*>(thisObject());
  if (item)
    return item->findBlockByNumber(blockNumber);
  return QTextBlock();
}

QTextBlock QTextDocumentProto::firstBlock() const
{
  QTextDocument *item = qscriptvalue_cast<QTextDocument*>(thisObject());
  if (item)
    return item->firstBlock();
  return QTextBlock();
}

qreal QTextDocumentProto::idealWidth() const
{
  QTextDocument *item = qscriptvalue_cast<QTextDocument*>(thisObject());
  if (item)
    return item->idealWidth();
  return qreal();
}

bool QTextDocumentProto::isEmpty() const
{
  QTextDocument *item = qscriptvalue_cast<QTextDocument*>(thisObject());
  if (item)
    return item->isEmpty();
  return false;
}

bool QTextDocumentProto::isRedoAvailable() const
{
  QTextDocument *item = qscriptvalue_cast<QTextDocument*>(thisObject());
  if (item)
    return item->isRedoAvailable();
  return false;
}

bool QTextDocumentProto::isUndoAvailable() const
{
  QTextDocument *item = qscriptvalue_cast<QTextDocument*>(thisObject());
  if (item)
    return item->isUndoAvailable();
  return false;
}

QTextBlock QTextDocumentProto::lastBlock() const
{
  QTextDocument *item = qscriptvalue_cast<QTextDocument*>(thisObject());
  if (item)
    return item->lastBlock();
  return QTextBlock();
}

int QTextDocumentProto::lineCount() const
{
  QTextDocument *item = qscriptvalue_cast<QTextDocument*>(thisObject());
  if (item)
    return item->lineCount();
  return 0;
}

void QTextDocumentProto::markContentsDirty(int position, int length)
{
  QTextDocument *item = qscriptvalue_cast<QTextDocument*>(thisObject());
  if (item)
    item->markContentsDirty(position, length);
}

QString QTextDocumentProto::metaInformation(int info) const
{
  QTextDocument *item = qscriptvalue_cast<QTextDocument*>(thisObject());
  if (item)
    return item->metaInformation((QTextDocument::MetaInformation)info);
  return QString();
}

QTextObject *QTextDocumentProto::object(int objectIndex) const
{
  QTextDocument *item = qscriptvalue_cast<QTextDocument*>(thisObject());
  if (item)
    return item->object(objectIndex);
  return 0;
}

QTextObject *QTextDocumentProto::objectForFormat(const QTextFormat &f) const
{
  QTextDocument *item = qscriptvalue_cast<QTextDocument*>(thisObject());
  if (item)
    return item->objectForFormat(f);
  return 0;
}

int QTextDocumentProto::pageCount() const
{
  QTextDocument *item = qscriptvalue_cast<QTextDocument*>(thisObject());
  if (item)
    return item->pageCount();
  return 0;
}

void QTextDocumentProto::print(QPrinter *printer) const
{
  QTextDocument *item = qscriptvalue_cast<QTextDocument*>(thisObject());
  if (item)
    item->print(printer);
}

void QTextDocumentProto::redo(QTextCursor *cursor)
{
  QTextDocument *item = qscriptvalue_cast<QTextDocument*>(thisObject());
  if (item)
    item->redo(cursor);
}

QVariant QTextDocumentProto::resource(int type, const QUrl &name) const
{
  QTextDocument *item = qscriptvalue_cast<QTextDocument*>(thisObject());
  if (item)
    return item->resource(type, name);
  return QVariant();
}

int QTextDocumentProto::revision() const
{
  QTextDocument *item = qscriptvalue_cast<QTextDocument*>(thisObject());
  if (item)
    return item->revision();
  return 0;
}

QTextFrame *QTextDocumentProto::rootFrame() const
{
  QTextDocument *item = qscriptvalue_cast<QTextDocument*>(thisObject());
  if (item)
    return item->rootFrame();
  return 0;
}

void QTextDocumentProto::setDefaultFont(const QFont &font)
{
  QTextDocument *item = qscriptvalue_cast<QTextDocument*>(thisObject());
  if (item)
    item->setDefaultFont(font);
}

void QTextDocumentProto::setDocumentLayout(QAbstractTextDocumentLayout *layout)
{
  QTextDocument *item = qscriptvalue_cast<QTextDocument*>(thisObject());
  if (item)
    item->setDocumentLayout(layout);
}

void QTextDocumentProto::setHtml(const QString &html)
{
  QTextDocument *item = qscriptvalue_cast<QTextDocument*>(thisObject());
  if (item)
    item->setHtml(html);
}

void QTextDocumentProto::setMetaInformation(int info, const QString &string)
{
  QTextDocument *item = qscriptvalue_cast<QTextDocument*>(thisObject());
  if (item)
    item->setMetaInformation((QTextDocument::MetaInformation)info, string);
}

void QTextDocumentProto::setPlainText(const QString &text)
{
  QTextDocument *item = qscriptvalue_cast<QTextDocument*>(thisObject());
  if (item)
    item->setPlainText(text);
}

QString QTextDocumentProto::toHtml(const QByteArray &encoding) const
{
  QTextDocument *item = qscriptvalue_cast<QTextDocument*>(thisObject());
  if (item)
    return item->toHtml(encoding);
  return QString();
}

QString QTextDocumentProto::toPlainText() const
{
  QTextDocument *item = qscriptvalue_cast<QTextDocument*>(thisObject());
  if (item)
    return item->toPlainText();
  return QString();
}

void QTextDocumentProto::undo(QTextCursor *cursor)
{
  QTextDocument *item = qscriptvalue_cast<QTextDocument*>(thisObject());
  if (item)
    item->undo(cursor);
}

QString QTextDocumentProto::toString()     const
{
  QTextDocument *item = qscriptvalue_cast<QTextDocument*>(thisObject());
  if (item)
    return item->toPlainText();
  return QString();
}
