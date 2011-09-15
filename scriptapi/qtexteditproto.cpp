/*
 *This file is part of the xTuple ERP: PostBooks Edition, a free and
 *open source Enterprise Resource Planning software suite,
 *Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 *It is licensed to you under the Common Public Attribution License
 *version 1.0, the full text of which (including xTuple-specific Exhibits)
 *is available at www.xtuple.com/CPAL.  By using this software, you agree
 *to be bound by its terms.
 */

#include "qtexteditproto.h"

#include <QString>

#define DEBUG false

void setupQTextEditProto(QScriptEngine *engine)
{
  QScriptValue proto = engine->newQObject(new QTextEditProto(engine));
  engine->setDefaultPrototype(qMetaTypeId<QTextEdit*>(), proto);
  //engine->setDefaultPrototype(qMetaTypeId<QTextEdit>(),  proto);

  QScriptValue constructor = engine->newFunction(constructQTextEdit,
                                                 proto);
  engine->globalObject().setProperty("QTextEdit",  constructor);
}

QScriptValue constructQTextEdit(QScriptContext *context,
                                QScriptEngine  *engine)
{
  QTextEdit *obj = 0;
  switch (context->argumentCount())
  {
    case 0: obj = new QTextEdit(); break;
    case 1: obj = new QTextEdit(qobject_cast<QWidget*>(context->argument(0).toQObject())); break;
    case 2:
    default:
            obj = new QTextEdit(context->argument(0).toString(),
                                qobject_cast<QWidget*>(context->argument(0).toQObject()));
            break;
  }

  return engine->toScriptValue(obj);
}

QTextEditProto::QTextEditProto(QObject *parent)
    : QObject(parent)
{
}

int QTextEditProto::alignment() const
{
  QTextEdit *item = qscriptvalue_cast<QTextEdit*>(thisObject());
  if (item)
    return item->alignment();
  return 0;
}

QString QTextEditProto::anchorAt(const QPoint &pos) const
{
  QTextEdit *item = qscriptvalue_cast<QTextEdit*>(thisObject());
  if (item)
    return item->anchorAt(pos);
  return QString();
}

bool QTextEditProto::canPaste() const
{
  QTextEdit *item = qscriptvalue_cast<QTextEdit*>(thisObject());
  if (item)
    return item->canPaste();
  return false;
}

QMenu *QTextEditProto::createStandardContextMenu()
{
  QTextEdit *item = qscriptvalue_cast<QTextEdit*>(thisObject());
  if (item)
    return item->createStandardContextMenu();
  return 0;
}

QMenu *QTextEditProto::createStandardContextMenu(const QPoint &position)
{
  QTextEdit *item = qscriptvalue_cast<QTextEdit*>(thisObject());
  if (item)
    return item->createStandardContextMenu(position);
  return 0;
}

QTextCharFormat QTextEditProto::currentCharFormat() const
{
  QTextEdit *item = qscriptvalue_cast<QTextEdit*>(thisObject());
  if (item)
    return item->currentCharFormat();
  return QTextCharFormat();
}

QFont QTextEditProto::currentFont() const
{
  QTextEdit *item = qscriptvalue_cast<QTextEdit*>(thisObject());
  if (item)
    return item->currentFont();
  return QFont();
}

QTextCursor QTextEditProto::cursorForPosition(const QPoint &pos) const
{
  QTextEdit *item = qscriptvalue_cast<QTextEdit*>(thisObject());
  if (item)
    return item->cursorForPosition(pos);
  return QTextCursor();
}

QRect QTextEditProto::cursorRect(const QTextCursor &cursor) const
{
  QTextEdit *item = qscriptvalue_cast<QTextEdit*>(thisObject());
  if (item)
    return item->cursorRect(cursor);
  return QRect();
}

QRect QTextEditProto::cursorRect() const
{
  QTextEdit *item = qscriptvalue_cast<QTextEdit*>(thisObject());
  if (item)
    return item->cursorRect();
  return QRect();
}

QTextDocument *QTextEditProto::document() const
{
  QTextEdit *item = qscriptvalue_cast<QTextEdit*>(thisObject());
  if (DEBUG) qDebug("QTextEditProto::document() item = %p", item);
  if (item)
  {
    if (DEBUG) qDebug("QTextEditProto::document() item->document() = %p", item->document());
    return item->document();
  }
  return 0;
}

void QTextEditProto::ensureCursorVisible()
{
  QTextEdit *item = qscriptvalue_cast<QTextEdit*>(thisObject());
  if (item)
    item->ensureCursorVisible();
}

bool QTextEditProto::find(const QString &exp, int options)
{
  QTextEdit *item = qscriptvalue_cast<QTextEdit*>(thisObject());
  if (item)
    return item->find(exp, (QTextDocument::FindFlag)options);
  return false;
}

QString QTextEditProto::fontFamily() const
{
  QTextEdit *item = qscriptvalue_cast<QTextEdit*>(thisObject());
  if (item)
    return item->fontFamily();
  return QString();
}

bool QTextEditProto::fontItalic() const
{
  QTextEdit *item = qscriptvalue_cast<QTextEdit*>(thisObject());
  if (item)
    return item->fontItalic();
  return false;
}

qreal QTextEditProto::fontPointSize() const
{
  QTextEdit *item = qscriptvalue_cast<QTextEdit*>(thisObject());
  if (item)
    return item->fontPointSize();
  return qreal();
}

bool QTextEditProto::fontUnderline() const
{
  QTextEdit *item = qscriptvalue_cast<QTextEdit*>(thisObject());
  if (item)
    return item->fontUnderline();
  return false;
}

int QTextEditProto::fontWeight() const
{
  QTextEdit *item = qscriptvalue_cast<QTextEdit*>(thisObject());
  if (item)
    return item->fontWeight();
  return 0;
}

QVariant QTextEditProto::loadResource(int type, const QUrl &name)
{
  QTextEdit *item = qscriptvalue_cast<QTextEdit*>(thisObject());
  if (item)
    return item->loadResource(type, name);
  return QVariant();
}

void QTextEditProto::mergeCurrentCharFormat(const QTextCharFormat &modifier)
{
  QTextEdit *item = qscriptvalue_cast<QTextEdit*>(thisObject());
  if (item)
    item->mergeCurrentCharFormat(modifier);
}

void QTextEditProto::moveCursor(int operation, int mode)
{
  QTextEdit *item = qscriptvalue_cast<QTextEdit*>(thisObject());
  if (item)
    item->moveCursor((QTextCursor::MoveOperation)operation,
                     (QTextCursor::MoveMode)mode);
}

void QTextEditProto::print(QPrinter *printer) const
{
  QTextEdit *item = qscriptvalue_cast<QTextEdit*>(thisObject());
  if (item)
    item->print(printer);
}

void QTextEditProto::setCurrentCharFormat(const QTextCharFormat &format)
{
  QTextEdit *item = qscriptvalue_cast<QTextEdit*>(thisObject());
  if (item)
    item->setCurrentCharFormat(format);
}

void QTextEditProto::setDocument(QTextDocument *document)
{
  QTextEdit *item = qscriptvalue_cast<QTextEdit*>(thisObject());
  if (item)
    item->setDocument(document);
}

void QTextEditProto::setTextCursor(const QTextCursor &cursor)
{
  QTextEdit *item = qscriptvalue_cast<QTextEdit*>(thisObject());
  if (item)
    item->setTextCursor(cursor);
}

QColor QTextEditProto::textBackgroundColor() const
{
  QTextEdit *item = qscriptvalue_cast<QTextEdit*>(thisObject());
  if (item)
    return item->textBackgroundColor();
  return QColor();
}

QColor QTextEditProto::textColor() const
{
  QTextEdit *item = qscriptvalue_cast<QTextEdit*>(thisObject());
  if (item)
    return item->textColor();
  return QColor();
}

QTextCursor QTextEditProto::textCursor() const
{
  QTextEdit *item = qscriptvalue_cast<QTextEdit*>(thisObject());
  if (item)
    return item->textCursor();
  return QTextCursor();
}

QString QTextEditProto::toString() const
{
  QTextEdit *item = qscriptvalue_cast<QTextEdit*>(thisObject());
  if (item)
    return QString("[ QTextEdit starting: %1... ]").arg(item->toPlainText().left(50));
  return QString("QTextEdit(unknown)");
}
