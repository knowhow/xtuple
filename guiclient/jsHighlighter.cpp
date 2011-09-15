/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "jsHighlighter.h"

#include <QColor>

#include "format.h"

static QStringList _keyword;
static QStringList _extension;

void JSHighlighter::init()
{
  _commentColor   = namedColor("altemphasis");
  _errorColor     = namedColor("error");
  _extensionColor = namedColor("warning");
  _keywordColor   = namedColor("emphasis");
  _literalColor   = namedColor("future");

  if (_keyword.isEmpty())
    _keyword  << "break"     << "case"    << "catch"          << "continue"
              << "default"   << "delete"  << "do"             << "else"
              << "false"     << "finally" << "for"            << "function"
              << "if"        << "in"      << "instanceof"     << "new"
              << "null"      << "return"  << "switch"         << "this"
              << "throw"     << "true"    << "try"            << "typeof"
              << "var"       << "void"    << "while"          << "with"
              ;

  if (_extension.isEmpty())
    _extension  << "abstract" << "boolean"   << "byte"         << "char"
                << "class"    << "const"     << "debugger"     << "double"
                << "enum"     << "export"    << "extends"      << "final"
                << "float"    << "goto"      << "implements"   << "import"
                << "int"      << "interface" << "long"         << "native"
                << "package"  << "private"   << "protected"    << "public"
                << "short"    << "static"    << "super"        << "synchronized"
                << "throws"   << "transient" << "volatile"
                ;
  
  _kwtest.setPattern("^(" + _keyword.join("|") + ")((?=\\W)|$)");
  _extest.setPattern("^(" + _extension.join("|") + ")((?=\\W)|$)");
  _numerictest.setPattern("^(0[xX][0-9a-fA-F]+)|(-?[0-9]+(\\.[0-9]+)?)");
  _wordtest.setPattern("^\\w+");
  _quotetest.setPattern("^\"[^\"]*\"|'[^']*'");
  _regexptest.setPattern("/[^/]*/[igm]?");

}

JSHighlighter::JSHighlighter(QObject *parent)
  : QSyntaxHighlighter(parent)
{
  init();
}

JSHighlighter::JSHighlighter(QTextDocument *document)
  : QSyntaxHighlighter(document)
{
  init();
}

JSHighlighter::JSHighlighter(QTextEdit *editor)
  : QSyntaxHighlighter(editor)
{
  init();
}

JSHighlighter::~JSHighlighter()
{
}

void JSHighlighter::highlightBlock(const QString &text)
{
  int state = previousBlockState();
  int start = 0;

  for (int i = 0; i < text.length(); i++)
  {
    if (state == InsideCStyleComment)
    {
      if (text.mid(i, 2) == "*/")
      {
        state = NormalState;
        setFormat(start, i - start + 2, _commentColor);
      }
    }
    else if (state == InsideString)
    {
      // TODO: if i == 0 then error color until next "
      if (text.mid(i, 1) == "\"")
      {
        state = NormalState;
        setFormat(start, i - start + 1, _literalColor);
        start = i;
      }
    }
    else if (text.mid(i, 2) == "//")
    {
      setFormat(i, text.length() - i, _commentColor);
      break;
    }
    else if (text.mid(i, 2) == "/*")
    {
      start = i;
      state = InsideCStyleComment;
    }
    else if (_quotetest.indexIn(text.mid(i)) == 0)
    {
      setFormat(i, _quotetest.matchedLength(), _literalColor);
      i += _quotetest.matchedLength();
    }
    else if (text.mid(i, 1) == "\"")
    {
      start = i;
      state = InsideString;
    }
    else if (_kwtest.indexIn(text.mid(i)) == 0)
    {
      setFormat(i, _kwtest.matchedLength(), _keywordColor);
      i += _kwtest.matchedLength();
    }
    else if (_extest.indexIn(text.mid(i)) == 0)
    {
      setFormat(i, _extest.matchedLength(), _extensionColor);
      i += _extest.matchedLength();
    }
    else if (_numerictest.indexIn(text.mid(i)) == 0)
    {
      setFormat(i, _numerictest.matchedLength(), _literalColor);
      i += _numerictest.matchedLength();
    }
    else if (_regexptest.indexIn(text.mid(i)) == 0)
    {
      setFormat(i, _regexptest.matchedLength(), _literalColor);
      i += _regexptest.matchedLength();
    }
    else if (_wordtest.indexIn(text.mid(i)) == 0)        // skip non-keywords
      i += _wordtest.matchedLength();
  }

  if (state == InsideCStyleComment)
    setFormat(start, text.length() - start, _commentColor);
  else if (state == InsideString)
    setFormat(start, text.length() - start, _errorColor);

  setCurrentBlockState(state);
}
