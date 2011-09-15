/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef __QFONTPROTO_H__
#define __QFONTPROTO_H__

#include <QFont>
#include <QObject>
#include <QtScript>

class QString;

Q_DECLARE_METATYPE(QFont*)
Q_DECLARE_METATYPE(QFont)

// this really belongs in qpaintdeviceproto.h
Q_DECLARE_METATYPE(QPaintDevice*)

void setupQFontProto(QScriptEngine *engine);
QScriptValue constructQFont(QScriptContext *context, QScriptEngine *engine);

class QFontProto : public QObject, public QScriptable
{
  Q_OBJECT

  public:
    QFontProto(QObject *parent);

    Q_INVOKABLE bool    bold()          const;
    Q_INVOKABLE int     capitalization()        const;
    Q_INVOKABLE QString defaultFamily() const;
    Q_INVOKABLE bool    exactMatch()    const;
    Q_INVOKABLE QString family()        const;
    Q_INVOKABLE bool    fixedPitch()    const;
    // Q_INVOKABLE FT_Face freetypeFace()  const;
    Q_INVOKABLE bool    fromString(const QString &descrip);
    // Q_INVOKABLE HFONT   handle()        const;
    Q_INVOKABLE bool    isCopyOf(const QFont &f)        const;
    Q_INVOKABLE bool    italic()        const;
    Q_INVOKABLE bool    kerning()       const;
    Q_INVOKABLE QString key()           const;
    Q_INVOKABLE QString lastResortFamily()      const;
    Q_INVOKABLE QString lastResortFont()        const;
    Q_INVOKABLE qreal   letterSpacing() const;
    Q_INVOKABLE int     letterSpacingType()    const;
#ifdef Q_WS_MAC
    Q_INVOKABLE quint32 macFontID()     const;
#endif
    Q_INVOKABLE bool    overline()      const;
    Q_INVOKABLE int     pixelSize()     const;
    Q_INVOKABLE int     pointSize()     const;
    Q_INVOKABLE qreal   pointSizeF()    const;
    Q_INVOKABLE bool    rawMode()       const;
    Q_INVOKABLE QString rawName()       const;
    Q_INVOKABLE QFont   resolve(const QFont &other)     const;
    Q_INVOKABLE void    setBold(bool enable);
    Q_INVOKABLE void    setCapitalization(int caps);
    Q_INVOKABLE void    setFamily(const QString &family);
    Q_INVOKABLE void    setFixedPitch(bool enable);
    Q_INVOKABLE void    setItalic(bool enable);
    Q_INVOKABLE void    setKerning(bool enable);
    Q_INVOKABLE void    setLetterSpacing(int type, qreal spacing);
    Q_INVOKABLE void    setOverline(bool enable);
    Q_INVOKABLE void    setPixelSize(int pixelSize);
    Q_INVOKABLE void    setPointSize(int pointSize);
    Q_INVOKABLE void    setPointSizeF(qreal pointSize);
    Q_INVOKABLE void    setRawMode(bool enable);
    Q_INVOKABLE void    setRawName(const QString &name);
    Q_INVOKABLE void    setStretch(int factor);
    Q_INVOKABLE void    setStrikeOut(bool enable);
    Q_INVOKABLE void    setStyle(int style);
    Q_INVOKABLE void    setStyleHint(int hint, int strategy = 0);
    Q_INVOKABLE void    setStyleStrategy(int s);
    Q_INVOKABLE void    setUnderline(bool enable);
    Q_INVOKABLE void    setWeight(int weight);
    Q_INVOKABLE void    setWordSpacing(qreal spacing);
    Q_INVOKABLE int     stretch()       const;
    Q_INVOKABLE bool    strikeOut()     const;
    Q_INVOKABLE int     style()         const;
    Q_INVOKABLE int     styleHint()     const;
    Q_INVOKABLE int     styleStrategy() const;
    Q_INVOKABLE QString toString()      const;
    Q_INVOKABLE bool    underline()     const;
    Q_INVOKABLE int     weight()        const;
    Q_INVOKABLE qreal   wordSpacing()   const;

};
#endif
