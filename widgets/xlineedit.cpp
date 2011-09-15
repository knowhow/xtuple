/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "xlineedit.h"

#include <QFocusEvent>
#include <QKeyEvent>
#include <QLocale>
#include <QMouseEvent>
#include <QValidator>

#include "format.h"

#define DEBUG false

XLineEdit::XLineEdit(QWidget *parent, const char *name) :
  QLineEdit(parent)
{
  if (! name && objectName().isEmpty())
    setObjectName("XLineEdit");
  setAcceptDrops(FALSE);

#ifdef Q_WS_MAC
  QFont f = font();
  f.setPointSize(f.pointSize() - 2);
  setFont(f);
#endif

  _parsed = true;
  _valid = false;
  _isNull = true;

  _id = -1;
  connect(this, SIGNAL(editingFinished()), this, SLOT(sParse()));

  _listAct = new QAction(tr("List..."), this);
  _listAct->setShortcut(QKeySequence(tr("Ctrl+Shift+L")));
  _listAct->setShortcutContext(Qt::WidgetShortcut);
  _listAct->setToolTip(tr("List all records"));
  connect(_listAct, SIGNAL(triggered()), this, SIGNAL(requestList()));
  addAction(_listAct);

  _searchAct = new QAction(tr("Search..."), this);
  _searchAct->setShortcut(QKeySequence(tr("Ctrl+Shift+Space")));
  _searchAct->setShortcutContext(Qt::WidgetShortcut);
  _searchAct->setToolTip(tr("Search on specific criteria"));
  connect(_searchAct, SIGNAL(triggered()), this, SIGNAL(requestSearch()));
  addAction(_searchAct);

  _aliasAct = new QAction(tr("Alias..."), this);
  _aliasAct->setShortcut(QKeySequence(tr("Ctrl+Shift+A")));
  _aliasAct->setShortcutContext(Qt::WidgetShortcut);
  _aliasAct->setToolTip(tr("List of alias records"));
  connect(_aliasAct, SIGNAL(triggered()), this, SIGNAL(requestAlias()));
  addAction(_aliasAct);

  _mapper = new XDataWidgetMapper(this);
}

bool XLineEdit::isValid()
{
  sParse();
  return _valid;
}

int XLineEdit::id()
{
  sParse();
  return _id;
}

void XLineEdit::sParse()
{
  if (validator() && validator()->inherits("QDoubleValidator"))
  {
    QRegExp zeroRegex(QString("^[0") + QLocale().groupSeparator() + "]*" +
                      QLocale().decimalPoint() + "0*$");
    if (! text().isEmpty() && toDouble() == 0 && ! text().contains(zeroRegex))
    {
      _valid = false;
      setText("");
    }
    else _valid = true;
  }
  if (DEBUG)
    qDebug("%s::sParse() _valid = %d",
           objectName().isEmpty() ? "XLineEdit" : qPrintable(objectName()),
           _valid);
}

double XLineEdit::toDouble(bool *pIsValid)
{
  QString strippedText = text().remove(QLocale().groupSeparator());
  double result = QLocale().toDouble(strippedText, pIsValid);
  _valid = pIsValid;
  if (DEBUG)
    qDebug("%s::toDouble() returning %f (%d) for %s (%s)",
           objectName().isEmpty() ? "XLineEdit" : qPrintable(objectName()),
           result, _valid, qPrintable(text()), qPrintable(strippedText));
  return result;
}

void XLineEdit::setDataWidgetMap(XDataWidgetMapper* m)
{
  _mapper=m;
  m->addMapping(this, _fieldName, QByteArray("text"), QByteArray("defaultText"));
  connect(this, SIGNAL(editingFinished()), this, SLOT(setData()));
}

void XLineEdit::setText(const QVariant &pVariant)
{
  if (pVariant.type() == QVariant::Double)
  {
    const QValidator *v = validator();
    int prec = 0;

    if (v && v->inherits("QDoubleValidator"))
      prec = ((QDoubleValidator*)v)->decimals();
    else if (v && v->inherits("QIntValidator"))
      prec = 0;

    QLineEdit::setText(formatNumber(pVariant.toDouble(), prec));
  }
  else
    QLineEdit::setText(pVariant.toString());
  sHandleNullStr();
}


void XLineEdit::setDouble(const double pDouble, const int pPrec)
{
  const QValidator *v = validator();
  int prec = pPrec;

  if (pPrec < 0 && v && v->inherits("QDoubleValidator"))
    prec = ((QDoubleValidator*)v)->decimals();

  QLineEdit::setText(formatNumber(pDouble, prec));
}

void XLineEdit::mouseDoubleClickEvent(QMouseEvent *event)
{
  emit doubleClicked();

  QLineEdit::mouseDoubleClickEvent(event);
}

void XLineEdit::mousePressEvent(QMouseEvent *event)
{
  emit clicked();

  QLineEdit::mousePressEvent(event);
}

void XLineEdit::keyPressEvent(QKeyEvent *event)
{
  _parsed = false;
  QLineEdit::keyPressEvent(event);
}

void XLineEdit::setData()
{
  setData(text());
}

void XLineEdit::setData(const QString &text)
{
  if (_mapper->model())
    _mapper->model()->setData(_mapper->model()->index(_mapper->currentIndex(),_mapper->mappedSection(this)), text);
}

void XLineEdit::setNullStr(const QString &text)
{
  if (_nullStr == text )
    return;

  _nullStr = text;
  sHandleNullStr();
  if (!_nullStr.isEmpty())
    connect(this, SIGNAL(textChanged(QString)), this, SLOT(sHandleNullStr()));
}

void XLineEdit::sHandleNullStr()
{
  if (_nullStr.isEmpty())
    return;

  QString sheet = styleSheet();
  QString nullStyle = " QLineEdit{ color: Grey; "
                      "            font: italic; "
                      "            font-size: 12px}";

  if (!hasFocus() &&
      text().isEmpty())
  {
    setText(_nullStr);
    sheet.append(nullStyle);
    _isNull = true;
  }
  else if (hasFocus() &&
           sheet.contains(nullStyle))
  {
    clear();
    sheet.remove(nullStyle);
  }
  else
  {
    _isNull = false;
    sheet.remove(nullStyle);
  }

  setStyleSheet(sheet);
}

void XLineEdit::focusInEvent(QFocusEvent * event)
{
  if (!_nullStr.isEmpty())
  {
    disconnect(this, SIGNAL(textChanged(QString)), this, SLOT(sHandleNullStr()));
    sHandleNullStr();
  }
  QLineEdit::focusInEvent(event);
}

void XLineEdit::focusOutEvent(QFocusEvent * event)
{
  if (!_nullStr.isEmpty())
  {
    connect(this, SIGNAL(textChanged(QString)), this, SLOT(sHandleNullStr()));
    sHandleNullStr();
  }
  QLineEdit::focusOutEvent(event);
}

bool XLineEdit::isNull()
{
  if (_nullStr.isEmpty() || hasFocus())
    return text().isEmpty();
  return _isNull;
}

