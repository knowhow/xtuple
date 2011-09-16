/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "xtextedit.h"
#include <QTextCursor>
#include <QContextMenuEvent>
#include <QColor>

GuiClientInterface* XTextEditHighlighter::_guiClientInterface = 0;
GuiClientInterface* XTextEdit::_guiClientInterface = 0;

XTextEdit::XTextEdit(QWidget *pParent) :
  QTextEdit(pParent)
{
  _spellStatus = true;
  _mapper = 0;  
  _highlighter = 0;
  _highlighter = new XTextEditHighlighter(this);
}

XTextEdit::~XTextEdit()
{   
}

void XTextEdit::setDataWidgetMap(XDataWidgetMapper* m)
{
  disconnect(this, SIGNAL(lostFocus()), this, SLOT(updateMapperData()));
  if (acceptRichText())
    m->addMapping(this, _fieldName, "html", "defaultText");
  else
    m->addMapping(this, _fieldName, "plainText", "defaultText");
  _mapper = m;
  connect(this, SIGNAL(textChanged()), this, SLOT(updateMapperData()));
}

void XTextEdit::updateMapperData()
{
  if (_mapper->model() &&
      _mapper->model()->data(_mapper->model()->index(_mapper->currentIndex(),
                     _mapper->mappedSection(this))).toString() != toPlainText())
      _mapper->model()->setData(_mapper->model()->index(_mapper->currentIndex(),
                                                        _mapper->mappedSection(this)),
                                toPlainText()); 
}

void XTextEdit::contextMenuEvent(QContextMenuEvent *event)
{
    QMenu *menu = createStandardContextMenu();

    bool enableSpellPref = false;
    if(_x_preferences)
       enableSpellPref = (_x_preferences->value("SpellCheck")=="t");

    if(_guiClientInterface && _guiClientInterface->hunspell_ready()
       && enableSpellPref && _spellStatus
       && this->isEnabled() && !this->isReadOnly())
    {       
       _lastPos=event->pos();
       QTextCursor cursor = cursorForPosition(_lastPos);
       QString textBlock = cursor.block().text();
       int pos = cursor.columnNumber();
       int end = textBlock.indexOf(QRegExp("\\W+"),pos);
       int begin = textBlock.left(pos).lastIndexOf(QRegExp("\\W+"),pos);
       textBlock = textBlock.mid(begin+1,end-begin-1).trimmed();
       QStringList wordList = _guiClientInterface->hunspell_suggest(textBlock);
       if (_guiClientInterface->hunspell_check(textBlock) < 1)
       {
         menu->addSeparator();
         QAction *act;
         act = menu->addAction(tr("Add Word"), this, SLOT(sAddWord()));
         act = menu->addAction(tr("Ignore Word"), this, SLOT(sIgnoreWord()));       
         if(!wordList.isEmpty())
         {
            menu->addSeparator();
            for (int menuActionCount = 0;menuActionCount < qMin(int(_MaxWords),wordList.size());
            ++menuActionCount)
            {
                 act = menu->addAction(wordList.at(menuActionCount).trimmed(),
                       this, SLOT(sCorrectWord()));
            }
         }
         menu->addSeparator();
       }
     }
     menu->exec(event->globalPos());
     delete menu;
}

void XTextEdit::sCorrectWord()
{
   QAction *action = qobject_cast<QAction *>(sender());
   if (action)
   {
      QString replacement = action->text();
      QTextCursor cursor = cursorForPosition(_lastPos);
      cursor.select(QTextCursor::WordUnderCursor);
      cursor.deleteChar();
      cursor.insertText(replacement);
      _highlighter->rehighlight();
   }
}

void XTextEdit::sAddWord()
{
    QTextCursor cursor = cursorForPosition(_lastPos);
    QString textBlock = cursor.block().text();
    int pos = cursor.columnNumber();
    int end = textBlock.indexOf(QRegExp("\\W+"),pos);
    int begin = textBlock.left(pos).lastIndexOf(QRegExp("\\W+"),pos);
    textBlock = textBlock.mid(begin+1,end-begin-1);
    _guiClientInterface->hunspell_add(textBlock);
    _highlighter->rehighlight();
}


void XTextEdit::sIgnoreWord()
{
    QTextCursor cursor = cursorForPosition(_lastPos);
    QString textBlock = cursor.block().text();
    int pos = cursor.columnNumber();
    int end = textBlock.indexOf(QRegExp("\\W+"),pos);
    int begin = textBlock.left(pos).lastIndexOf(QRegExp("\\W+"),pos);
    textBlock = textBlock.mid(begin+1,end-begin-1);
    _guiClientInterface->hunspell_ignore(textBlock);
    _highlighter->rehighlight();
}


XTextEditHighlighter::XTextEditHighlighter(QObject *parent)
  : QSyntaxHighlighter(parent)
{
    HighlightingRule rule;
    _spellCheckFormat.setUnderlineColor(QColor(Qt::red));
    _spellCheckFormat.setUnderlineStyle(QTextCharFormat::SpellCheckUnderline);
}

XTextEditHighlighter::XTextEditHighlighter(QTextDocument *document)
  : QSyntaxHighlighter(document)
{
    HighlightingRule rule;
    _spellCheckFormat.setUnderlineColor(QColor(Qt::red));
    _spellCheckFormat.setUnderlineStyle(QTextCharFormat::SpellCheckUnderline);
}

XTextEditHighlighter::XTextEditHighlighter(QTextEdit *editor)
  : QSyntaxHighlighter(editor)
{
    HighlightingRule rule;
    _spellCheckFormat.setUnderlineColor(QColor(Qt::red));
    _spellCheckFormat.setUnderlineStyle(QTextCharFormat::SpellCheckUnderline);
}

XTextEditHighlighter::~XTextEditHighlighter()
{
}

void XTextEditHighlighter::highlightBlock(const QString &text)
{
    XTextEdit* textEdit = qobject_cast<XTextEdit *>(this->parent());

    bool enableSpellPref = false;
    if(_x_preferences)
       enableSpellPref = (_x_preferences->value("SpellCheck")=="t");

    if(_guiClientInterface && _guiClientInterface->hunspell_ready()
       && enableSpellPref && textEdit->spellEnabled()
       && textEdit->isEnabled() && !textEdit->isReadOnly())
    {
      QString widgetText = text.simplified();
      if (!widgetText.isEmpty())
      {
         QStringList widgetWords = widgetText.split(QRegExp("([^\\w,^\\\\]|(?=\\\\))+"),
                                                       QString::SkipEmptyParts);
         foreach(QString word, widgetWords)
         {            
            if (word.length() > 1 && !word.startsWith('\\'))
            {
                if (_guiClientInterface->hunspell_check(word) < 1)
                {
                   int wordCount = text.count(QRegExp("\\b" + word + "\\b"));
                   //loop through all repeated words in Line Edit
                   int wordStartPos=-1;
                   for (int repeatCount=0;repeatCount < wordCount; ++repeatCount)
                   {
                     wordStartPos = text.indexOf(QRegExp("\\b" + word + "\\b"),wordStartPos+1);
                     if (wordStartPos>=0)
                         setFormat(wordStartPos, word.length(), _spellCheckFormat);
                   }
                }
             }
          }
       }
    }   
}
