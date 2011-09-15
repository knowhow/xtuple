/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef __ORREPORTPROTO_H__
#define __ORREPORTPROTO_H__

#include "openreports.h"
#include "parameter.h"

#include <QImage>
#include <QObject>
#include <QString>
#include <QtScript>

class QDomDocument;
class QFont;
class QPainter;
class QPrinter;
class QSqlDatabase;
class QStringList;

Q_DECLARE_METATYPE(orReport*)
Q_DECLARE_METATYPE(orReport)

void setupOrReportProto(QScriptEngine *engine);
QScriptValue constructOrReport(QScriptContext *context, QScriptEngine *engine);

class orReportProto : public QObject, public QScriptable
{
  Q_OBJECT

  public:
    orReportProto(QObject *parent);

    // these are handled as properties of the global object
    // static bool beginMultiPrint(QPrinter *printer, bool &userCanceled);
    // static bool endMultiPrint(QPrinter *printer);

    Q_INVOKABLE int                 backgroundAlignment();
    Q_INVOKABLE QImage              backgroundImage();
    Q_INVOKABLE unsigned char       backgroundOpacity();
    Q_INVOKABLE QRect               backgroundRect();
    Q_INVOKABLE bool                backgroundScale();
    Q_INVOKABLE Qt::AspectRatioMode backgroundScaleMode();
    Q_INVOKABLE bool                doParamsSatisfy();
    Q_INVOKABLE bool                doesReportExist();
    Q_INVOKABLE bool                exportToPDF(const QString& fileName);
    Q_INVOKABLE ParameterList       getParamList();
    Q_INVOKABLE bool                isValid();
    Q_INVOKABLE bool                print(QPrinter *prtThis = 0, bool boolSetupPrinter = TRUE, bool showPreview = false);
    Q_INVOKABLE bool                render(QPainter *painter, QPrinter *printer = 0);
    Q_INVOKABLE int                 reportError(QWidget *widget);
    Q_INVOKABLE bool                satisfyParams(QWidget *widget);
    Q_INVOKABLE bool                setDom(const QDomDocument &docPReport);
    Q_INVOKABLE void                setBackgroundAlignment(int alignment);
    Q_INVOKABLE void                setBackgroundImage(const QImage &image);
    Q_INVOKABLE void                setBackgroundOpacity(unsigned char opacity);
    Q_INVOKABLE void                setBackgroundRect(const QRect &rect);
    Q_INVOKABLE void                setBackgroundRect(int x, int y, int w, int h); 
    Q_INVOKABLE void                setBackgroundScale(bool scale);
    Q_INVOKABLE void                setBackgroundScaleMode(Qt::AspectRatioMode mode);
    Q_INVOKABLE void                setDatabase(QSqlDatabase db);
    Q_INVOKABLE void                setParamList(const ParameterList &params);
    Q_INVOKABLE void                setParamList(const QStringList &list);
    Q_INVOKABLE void                setWatermarkFont(const QFont &font);
    Q_INVOKABLE void                setWatermarkOpacity(unsigned char opacity);
    Q_INVOKABLE void                setWatermarkText(const QString &text);
    Q_INVOKABLE QFont               watermarkFont();
    Q_INVOKABLE unsigned char       watermarkOpacity();
    Q_INVOKABLE QString             watermarkText();

    //Q_INVOKABLE QString     toString()     const;
};

#endif
