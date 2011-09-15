/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "qfontproto.h"

#include <QString>

#define DEBUG false

void setupQFontProto(QScriptEngine *engine)
{
  QScriptValue proto = engine->newQObject(new QFontProto(engine));
  engine->setDefaultPrototype(qMetaTypeId<QFont*>(), proto);
  engine->setDefaultPrototype(qMetaTypeId<QFont>(),  proto);

  QScriptValue constructor = engine->newFunction(constructQFont,
                                                 proto);
  engine->globalObject().setProperty("QFont", constructor);
}

QScriptValue constructQFont(QScriptContext *context,
                            QScriptEngine  *engine)
{
  QFont *obj = 0;

  if (DEBUG)
  {
    qDebug("constructQFont() entered");
    for (int i = 0; i < context->argumentCount(); i++)
      qDebug("context->argument(%d) = %s", i,
             qPrintable(context->argument(i).toString()));
  }

  if (context->argumentCount() >= 4)
    obj = new QFont(context->argument(0).toString(),
                    context->argument(1).toInt32(),
                    context->argument(2).toInt32(),
                    context->argument(3).toBool());
  else if (context->argumentCount() == 3)
    obj = new QFont(context->argument(0).toString(),
                    context->argument(1).toInt32(),
                    context->argument(2).toInt32());
  else if (context->argumentCount() == 2 &&
           qscriptvalue_cast<QPaintDevice*>(context->argument(1)))
    obj = new QFont(qscriptvalue_cast<QFont>(context->argument(0)),
                    qscriptvalue_cast<QPaintDevice*>(context->argument(1)));
  else if (context->argumentCount() == 2)
    obj = new QFont(context->argument(0).toString(),
                    context->argument(1).toInt32());
  else if (context->argumentCount() == 1 &&
          context->argument(0).isString())
  {
    if (DEBUG) qDebug("calling new QFont(QString)");
    obj = new QFont(context->argument(0).toString());
  }
  else if (context->argumentCount() == 1)
    obj = new QFont(qscriptvalue_cast<QFont>(context->argument(0)));
  else
    obj = new QFont();

  return engine->toScriptValue(obj);
}

QFontProto::QFontProto(QObject *parent)
    : QObject(parent)
{
}

bool QFontProto::bold() const
{
  QFont *item = qscriptvalue_cast<QFont*>(thisObject());
  if (item)
    return item->bold();
  return false;
}

int QFontProto::capitalization() const
{
  QFont *item = qscriptvalue_cast<QFont*>(thisObject());
  if (item)
    return item->capitalization();
  return 0;
}

QString QFontProto::defaultFamily() const
{
  QFont *item = qscriptvalue_cast<QFont*>(thisObject());
  if (item)
    return item->defaultFamily();
  return QString();
}

bool QFontProto::exactMatch() const
{
  QFont *item = qscriptvalue_cast<QFont*>(thisObject());
  if (item)
    return item->exactMatch();
  return false;
}

QString QFontProto::family() const
{
  QFont *item = qscriptvalue_cast<QFont*>(thisObject());
  if (item)
    return item->family();
  return QString();
}

bool QFontProto::fixedPitch() const
{
  QFont *item = qscriptvalue_cast<QFont*>(thisObject());
  if (item)
    return item->fixedPitch();
  return false;
}

/*
FT_Face QFontProto::freetypeFace() const
{
  QFont *item = qscriptvalue_cast<QFont*>(thisObject());
  if (item)
    return item->freetypeFace();
  return FT_Face();
}
*/

bool QFontProto::fromString(const QString &descrip)
{
  QFont *item = qscriptvalue_cast<QFont*>(thisObject());
  if (item)
    return item->fromString(descrip);
  return false;
}

/*
HFONT QFontProto::handle() const
{
  QFont *item = qscriptvalue_cast<QFont*>(thisObject());
  if (item)
    return item->handle();
  return HFONT();
}
*/

bool QFontProto::isCopyOf(const QFont &f) const
{
  QFont *item = qscriptvalue_cast<QFont*>(thisObject());
  if (item)
    return item->isCopyOf(f);
  return false;
}

bool QFontProto::italic() const
{
  QFont *item = qscriptvalue_cast<QFont*>(thisObject());
  if (item)
    return item->italic();
  return false;
}

bool QFontProto::kerning() const
{
  QFont *item = qscriptvalue_cast<QFont*>(thisObject());
  if (item)
    return item->kerning();
  return false;
}

QString QFontProto::key() const
{
  QFont *item = qscriptvalue_cast<QFont*>(thisObject());
  if (item)
    return item->key();
  return QString();
}

QString QFontProto::lastResortFamily() const
{
  QFont *item = qscriptvalue_cast<QFont*>(thisObject());
  if (item)
    return item->lastResortFamily();
  return QString();
}

QString QFontProto::lastResortFont() const
{
  QFont *item = qscriptvalue_cast<QFont*>(thisObject());
  if (item)
    return item->lastResortFont();
  return QString();
}

qreal QFontProto::letterSpacing() const
{
  QFont *item = qscriptvalue_cast<QFont*>(thisObject());
  if (item)
    return item->letterSpacing();
  return qreal();
}

int QFontProto::letterSpacingType() const
{
  QFont *item = qscriptvalue_cast<QFont*>(thisObject());
  if (item)
    return item->letterSpacingType();
  return 0;
}

#ifdef Q_WS_MAC
quint32 QFontProto::macFontID() const
{
  QFont *item = qscriptvalue_cast<QFont*>(thisObject());
  if (item)
    return item->macFontID();
  return quint32();
}
#endif

bool QFontProto::overline() const
{
  QFont *item = qscriptvalue_cast<QFont*>(thisObject());
  if (item)
    return item->overline();
  return false;
}

int QFontProto::pixelSize() const
{
  QFont *item = qscriptvalue_cast<QFont*>(thisObject());
  if (item)
    return item->pixelSize();
  return 0;
}

int QFontProto::pointSize() const
{
  QFont *item = qscriptvalue_cast<QFont*>(thisObject());
  if (item)
    return item->pointSize();
  return 0;
}

qreal QFontProto::pointSizeF() const
{
  QFont *item = qscriptvalue_cast<QFont*>(thisObject());
  if (item)
    return item->pointSizeF();
  return 0;
}

bool QFontProto::rawMode() const
{
  QFont *item = qscriptvalue_cast<QFont*>(thisObject());
  if (item)
    return item->rawMode();
  return false;
}

QString QFontProto::rawName() const
{
  QFont *item = qscriptvalue_cast<QFont*>(thisObject());
  if (item)
    return item->rawName();
  return QString();
}

QFont QFontProto::resolve(const QFont &other) const
{
  QFont *item = qscriptvalue_cast<QFont*>(thisObject());
  if (item)
    return item->resolve(other);
  return QFont();
}

void QFontProto::setBold(bool enable)
{
  QFont *item = qscriptvalue_cast<QFont*>(thisObject());
  if (item)
    item->setBold(enable);
}

void QFontProto::setCapitalization(int caps)
{
  QFont *item = qscriptvalue_cast<QFont*>(thisObject());
  if (item)
    item->setCapitalization((QFont::Capitalization)caps);
}

void QFontProto::setFamily(const QString &family)
{
  QFont *item = qscriptvalue_cast<QFont*>(thisObject());
  if (item)
    item->setFamily(family);
}

void QFontProto::setFixedPitch(bool enable)
{
  QFont *item = qscriptvalue_cast<QFont*>(thisObject());
  if (item)
    item->setFixedPitch(enable);
}

void QFontProto::setItalic(bool enable)
{
  QFont *item = qscriptvalue_cast<QFont*>(thisObject());
  if (item)
    item->setItalic(enable);
}

void QFontProto::setKerning(bool enable)
{
  QFont *item = qscriptvalue_cast<QFont*>(thisObject());
  if (item)
    item->setKerning(enable);
}

void QFontProto::setLetterSpacing(int type, qreal spacing)
{
  QFont *item = qscriptvalue_cast<QFont*>(thisObject());
  if (item)
    item->setLetterSpacing((QFont::SpacingType)type, spacing);
}

void QFontProto::setOverline(bool enable)
{
  QFont *item = qscriptvalue_cast<QFont*>(thisObject());
  if (item)
    item->setOverline(enable);
}

void QFontProto::setPixelSize(int pixelSize)
{
  QFont *item = qscriptvalue_cast<QFont*>(thisObject());
  if (item)
    item->setPixelSize(pixelSize);
}

void QFontProto::setPointSize(int pointSize)
{
  QFont *item = qscriptvalue_cast<QFont*>(thisObject());
  if (item)
    item->setPointSize(pointSize);
}

void QFontProto::setPointSizeF(qreal pointSize)
{
  QFont *item = qscriptvalue_cast<QFont*>(thisObject());
  if (item)
    item->setPointSizeF(pointSize);
}

void QFontProto::setRawMode(bool enable)
{
  QFont *item = qscriptvalue_cast<QFont*>(thisObject());
  if (item)
    item->setRawMode(enable);
}

void QFontProto::setRawName(const QString &name)
{
  QFont *item = qscriptvalue_cast<QFont*>(thisObject());
  if (item)
    item->setRawName(name);
}

void QFontProto::setStretch(int factor)
{
  QFont *item = qscriptvalue_cast<QFont*>(thisObject());
  if (item)
    item->setStretch(factor);
}

void QFontProto::setStrikeOut(bool enable)
{
  QFont *item = qscriptvalue_cast<QFont*>(thisObject());
  if (item)
    item->setStrikeOut(enable);
}

void QFontProto::setStyle(int style)
{
  QFont *item = qscriptvalue_cast<QFont*>(thisObject());
  if (item)
    item->setStyle((QFont::Style)style);
}

void QFontProto::setStyleHint(int hint, int strategy)
{
  QFont *item = qscriptvalue_cast<QFont*>(thisObject());
  if (item)
    item->setStyleHint((QFont::StyleHint)hint, (QFont::StyleStrategy)strategy);
}

void QFontProto::setStyleStrategy(int s)
{
  QFont *item = qscriptvalue_cast<QFont*>(thisObject());
  if (item)
    item->setStyleStrategy((QFont::StyleStrategy)s);
}

void QFontProto::setUnderline(bool enable)
{
  QFont *item = qscriptvalue_cast<QFont*>(thisObject());
  if (item)
    item->setUnderline(enable);
}

void QFontProto::setWeight(int weight)
{
  QFont *item = qscriptvalue_cast<QFont*>(thisObject());
  if (item)
    item->setWeight(weight);
}

void QFontProto::setWordSpacing(qreal spacing)
{
  QFont *item = qscriptvalue_cast<QFont*>(thisObject());
  if (item)
    item->setWordSpacing(spacing);
}

int QFontProto::stretch() const
{
  QFont *item = qscriptvalue_cast<QFont*>(thisObject());
  if (item)
    return item->stretch();
  return 0;
}

bool QFontProto::strikeOut() const
{
  QFont *item = qscriptvalue_cast<QFont*>(thisObject());
  if (item)
    return item->strikeOut();
  return false;
}

int QFontProto::style() const
{
  QFont *item = qscriptvalue_cast<QFont*>(thisObject());
  if (item)
    return item->style();
  return 0;
}

int QFontProto::styleHint() const
{
  QFont *item = qscriptvalue_cast<QFont*>(thisObject());
  if (item)
    return item->styleHint();
  return 0;
}

int QFontProto::styleStrategy() const
{
  QFont *item = qscriptvalue_cast<QFont*>(thisObject());
  if (item)
    return item->styleStrategy();
  return 0;
}

QString QFontProto::toString() const
{
  QFont *item = qscriptvalue_cast<QFont*>(thisObject());
  if (item)
    return item->toString();
  return QString();
}

bool QFontProto::underline() const
{
  QFont *item = qscriptvalue_cast<QFont*>(thisObject());
  if (item)
    return item->underline();
  return false;
}

int QFontProto::weight() const
{
  QFont *item = qscriptvalue_cast<QFont*>(thisObject());
  if (item)
    return item->weight();
  return 0;
}

qreal QFontProto::wordSpacing() const
{
  QFont *item = qscriptvalue_cast<QFont*>(thisObject());
  if (item)
    return item->wordSpacing();
  return qreal();
}

