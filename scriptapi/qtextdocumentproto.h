/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef __QTEXTDOCUMENTPROTO_H__
#define __QTEXTDOCUMENTPROTO_H__

#include <QAbstractTextDocumentLayout>
#include <QChar>
#include <QFont>
#include <QObject>
#include <QTextBlock>
#include <QTextDocument>
#include <QTextFormat>
#include <QUrl>
#include <QVariant>
#include <QVector>
#include <QtScript>

class QString;

Q_DECLARE_METATYPE(QTextDocument*)
//Q_DECLARE_METATYPE(QTextDocument)

void setupQTextDocumentProto(QScriptEngine *engine);
QScriptValue constructQTextDocument(QScriptContext *context, QScriptEngine *engine);

class QTextDocumentProto : public QObject, public QScriptable
{
  Q_OBJECT

  public:
    QTextDocumentProto(QObject *parent);

    Q_INVOKABLE void            addResource(int type, const QUrl &name, const QVariant &resource);
    Q_INVOKABLE void            adjustSize();
    Q_INVOKABLE QVector<QTextFormat>    allFormats() const;
    Q_INVOKABLE QTextBlock      begin() const;
    Q_INVOKABLE QChar           characterAt(int pos) const;
    Q_INVOKABLE int             characterCount() const;
    Q_INVOKABLE void            clear();
    Q_INVOKABLE QTextDocument  *clone(QObject *parent = 0) const;
    Q_INVOKABLE QAbstractTextDocumentLayout *documentLayout() const;
    Q_INVOKABLE void            drawContents(QPainter *p, const QRectF &rect = QRectF());
    Q_INVOKABLE QTextBlock      end() const;
    Q_INVOKABLE QTextCursor     find(const QString &subString, const QTextCursor &cursor, int options = 0) const;
    Q_INVOKABLE QTextCursor     find(const QRegExp &expr, const QTextCursor &cursor, int options = 0) const;
    Q_INVOKABLE QTextCursor     find(const QString &subString, int position = 0, int options = 0) const;
    Q_INVOKABLE QTextCursor     find(const QRegExp &expr, int position = 0, int options = 0) const;
    Q_INVOKABLE QTextBlock      findBlock(int pos) const;
    Q_INVOKABLE QTextBlock      findBlockByLineNumber(int lineNumber) const;
    Q_INVOKABLE QTextBlock      findBlockByNumber(int blockNumber) const;
    Q_INVOKABLE QTextBlock      firstBlock() const;
    Q_INVOKABLE qreal           idealWidth() const;
    Q_INVOKABLE bool            isEmpty() const;
    Q_INVOKABLE bool            isRedoAvailable() const;
    Q_INVOKABLE bool            isUndoAvailable() const;
    Q_INVOKABLE QTextBlock      lastBlock() const;
    Q_INVOKABLE int             lineCount() const;
    Q_INVOKABLE void            markContentsDirty(int position, int length);
    Q_INVOKABLE QString         metaInformation(int info) const;
    Q_INVOKABLE QTextObject    *object(int objectIndex) const;
    Q_INVOKABLE QTextObject    *objectForFormat(const QTextFormat &f) const;
    Q_INVOKABLE int             pageCount() const;
    Q_INVOKABLE void            print(QPrinter *printer) const;
    Q_INVOKABLE void            redo(QTextCursor *cursor);
    Q_INVOKABLE QVariant        resource(int type, const QUrl &name) const;
    Q_INVOKABLE int             revision() const;
    Q_INVOKABLE QTextFrame     *rootFrame() const;
    Q_INVOKABLE void            setDefaultFont(const QFont &font);
    Q_INVOKABLE void            setDocumentLayout(QAbstractTextDocumentLayout *layout);
    Q_INVOKABLE void            setHtml(const QString &html);
    Q_INVOKABLE void            setMetaInformation(int info, const QString &string);
    Q_INVOKABLE void            setPlainText(const QString &text);
    Q_INVOKABLE QString         toHtml(const QByteArray &encoding = QByteArray()) const;
    Q_INVOKABLE QString         toPlainText() const;
    Q_INVOKABLE void            undo(QTextCursor *cursor);

    Q_INVOKABLE QString         toString()     const;
};

#endif
