/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef __XTEXTEDIT_H__
#define __XTEXTEDIT_H__

#include <QMenu>
#include <QTextEdit>
#include <QTextCharFormat>
#include <QSyntaxHighlighter>

#include "widgets.h"
#include "xdatawidgetmapper.h"
#include "guiclientinterface.h"

class XTextEditHighlighter;

class XTUPLEWIDGETS_EXPORT XTextEdit : public QTextEdit
{
  Q_OBJECT
  Q_PROPERTY(QString fieldName   READ fieldName   WRITE setFieldName)
  Q_PROPERTY(QString defaultText  READ defaultText WRITE setDefaultText)
  Q_PROPERTY(bool    spellEnable  READ spellEnabled WRITE setSpellEnable)

  public:
    XTextEdit(QWidget * = 0);
    ~XTextEdit();
    
    virtual QString defaultText() const { return _default; };
    virtual QString fieldName()   const { return _fieldName; };

    static GuiClientInterface *_guiClientInterface;

    Q_INVOKABLE bool spellEnabled() const { return _spellStatus; }

  public slots:
    virtual void setDataWidgetMap(XDataWidgetMapper* m);
    virtual void setDefaultText(QString p)  { _default = p; };
    virtual void setFieldName(QString p) { _fieldName = p; };
    virtual void updateMapperData();
    virtual void setSpellEnable(bool p)  { _spellStatus = p; }

  private slots:
    void contextMenuEvent(QContextMenuEvent *event);
    void sCorrectWord();
    void sAddWord();    
    void sIgnoreWord();

 protected:
    XDataWidgetMapper *_mapper;

  private:
    QString _default;
    QString _fieldName;
    enum { _MaxWords = 5 };
    QPoint _lastPos;
    XTextEditHighlighter *_highlighter;
    bool _spellStatus;
};

class XTextEditHighlighter : public QSyntaxHighlighter
{
    Q_OBJECT

public:
    static GuiClientInterface *_guiClientInterface;
    XTextEditHighlighter(QObject *parent);
    XTextEditHighlighter(QTextDocument *document);
    XTextEditHighlighter(QTextEdit *editor);
    ~XTextEditHighlighter();

protected:
    virtual void highlightBlock(const QString &text);

private:
    struct HighlightingRule
     {
        QRegExp _pattern;
        QTextCharFormat _format;
    };
    QVector<HighlightingRule> _highlightingRules;

    QRegExp _commentStartExpression;
    QRegExp _commentEndExpression;

    QTextCharFormat _keywordFormat;
    QTextCharFormat _classFormat;
    QTextCharFormat _singleLineCommentFormat;
    QTextCharFormat _multiLineCommentFormat;
    QTextCharFormat _quotationFormat;
    QTextCharFormat _functionFormat;
    QTextCharFormat _spellCheckFormat;

}; // SPELLHIGHLIGHTER_H

#endif
