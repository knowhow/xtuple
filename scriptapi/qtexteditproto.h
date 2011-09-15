/*
 *This file is part of the xTuple ERP: PostBooks Edition, a free and
 *open source Enterprise Resource Planning software suite,
 *Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 *It is licensed to you under the Common Public Attribution License
 *version 1.0, the full text of which (including xTuple-specific Exhibits)
 *is available at www.xtuple.com/CPAL.  By using this software, you agree
 *to be bound by its terms.
 */

#ifndef __QTEXTEDITPROTO_H__
#define __QTEXTEDITPROTO_H__

#include <QFont>
#include <QList>
#include <QMenu>
#include <QObject>
#include <QColor>
#include <QPoint>
#include <QRect>
#include <QString>
#include <QTextCharFormat>
#include <QTextCursor>
#include <QTextDocument>
#include <QTextEdit>
#include <QtScript>

class QString;

Q_DECLARE_METATYPE(QTextEdit*)
//Q_DECLARE_METATYPE(QTextEdit)

void setupQTextEditProto(QScriptEngine *engine);
QScriptValue constructQTextEdit(QScriptContext *context, QScriptEngine *engine);

class QTextEditProto : public QObject, public QScriptable
{
  Q_OBJECT

  public:
    QTextEditProto(QObject *parent);

    Q_INVOKABLE int             alignment() const;
    Q_INVOKABLE QString         anchorAt(const QPoint &pos) const;
    Q_INVOKABLE bool            canPaste() const;
    Q_INVOKABLE QMenu           *createStandardContextMenu();
    Q_INVOKABLE QMenu           *createStandardContextMenu(const QPoint &position);
    Q_INVOKABLE QTextCharFormat currentCharFormat() const;
    Q_INVOKABLE QFont           currentFont() const;
    Q_INVOKABLE QTextCursor     cursorForPosition(const QPoint &pos) const;
    Q_INVOKABLE QRect           cursorRect(const QTextCursor &cursor) const;
    Q_INVOKABLE QRect           cursorRect() const;
    Q_INVOKABLE QTextDocument   *document() const;
    Q_INVOKABLE void            ensureCursorVisible();
    //          QList<ExtraSelection>   extraSelections() const;
    Q_INVOKABLE bool            find(const QString &exp, int options = 0);
    Q_INVOKABLE QString         fontFamily() const;
    Q_INVOKABLE bool            fontItalic() const;
    Q_INVOKABLE qreal           fontPointSize() const;
    Q_INVOKABLE bool            fontUnderline() const;
    Q_INVOKABLE int             fontWeight() const;
    Q_INVOKABLE QVariant        loadResource(int type, const QUrl &name);
    Q_INVOKABLE void            mergeCurrentCharFormat(const QTextCharFormat &modifier);
    Q_INVOKABLE void            moveCursor(int operation, int mode = QTextCursor::MoveAnchor);
    Q_INVOKABLE void            print(QPrinter *printer) const;
    Q_INVOKABLE void            setCurrentCharFormat(const QTextCharFormat &format);
    Q_INVOKABLE void            setDocument(QTextDocument *document);
    //          void            setExtraSelections(const QList<ExtraSelection> &selections);
    Q_INVOKABLE void            setTextCursor(const QTextCursor &cursor);
    Q_INVOKABLE QColor          textBackgroundColor() const;
    Q_INVOKABLE QColor          textColor() const;
    Q_INVOKABLE QTextCursor     textCursor() const;

    Q_INVOKABLE QString         toString() const;

};

#endif
