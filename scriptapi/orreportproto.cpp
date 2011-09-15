/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "orreportproto.h"

#include <QDomDocument>
#include <QFont>
#include <QPainter>
#include <QPrinter>
#include <QSqlDatabase>
#include <QStringList>

#include "parameterlistsetup.h"
#include "qprinterproto.h"
#include "qsqldatabaseproto.h"

#define DEBUG false

/* TODO: make this work even though orReport isn't a QObject
QScriptValue orReporttoScriptValue(QScriptEngine *engine, orReport* const &item)
{
  return engine->newQObject(item);
}

void orReportfromScriptValue(const QScriptValue &obj, orReport* &item)
{
  item = qobject_cast<orReport*>(obj.toQObject());
}
*/

QScriptValue scriptBeginMultiPrint(QScriptContext *context,
                                   QScriptEngine  * /*engine*/)
{
  bool result = false;
  if (context->argumentCount() > 1)
  {
    QPrinter *printer = qscriptvalue_cast<QPrinter*>(context->argument(0).toObject());
    bool userCanceled = false;
    result  = orReport::beginMultiPrint(printer, userCanceled);
    if (DEBUG)
      qDebug("beginMultiPrint(%p, %d) returned %d",
             printer, userCanceled, result);
    if (context->argumentCount() > 1)
    {
      // TODO: figure out how to set userCanceled in the script
    }
  }
  else
  {
    QPrinter *printer = qscriptvalue_cast<QPrinter*>(context->argument(0).toObject());
    result  = orReport::beginMultiPrint(printer);
  }

  return result;
}

QScriptValue scriptEndMultiPrint(QScriptContext *context,
                                 QScriptEngine  * /*engine*/)
{
  bool result = false;
  if (context->argumentCount() > 0)
  {
    QPrinter *printer = qscriptvalue_cast<QPrinter*>(context->argument(0).toObject());
    result = orReport::endMultiPrint(printer);
    if (DEBUG)
      qDebug("endMultiPrint(%p) returned %d", printer, result);
  }

  return result;
}

void setupOrReportProto(QScriptEngine *engine)
{
//qScriptRegisterMetaType(engine, orReporttoScriptValue, orReportfromScriptValue);

  QScriptValue proto = engine->newQObject(new orReportProto(engine));
  engine->setDefaultPrototype(qMetaTypeId<orReport*>(), proto);
  engine->setDefaultPrototype(qMetaTypeId<orReport>(),  proto);

  QScriptValue constructor = engine->newFunction(constructOrReport, proto);
  engine->globalObject().setProperty("orReport",  constructor);

  constructor.setProperty("beginMultiPrint", engine->newFunction(scriptBeginMultiPrint));
  constructor.setProperty("endMultiPrint",   engine->newFunction(scriptEndMultiPrint));
}

QScriptValue constructOrReport(QScriptContext *context,
                               QScriptEngine  *engine)
{
  orReport *obj = 0;
  if (context->argumentCount() == 0)
    obj = new orReport();

  else if (context->argumentCount() == 1 &&
           context->argument(0).isString())
    obj = new orReport(context->argument(0).toString());
  else if (context->argumentCount() == 1)
    obj = new orReport(qscriptvalue_cast<QSqlDatabase>(context->argument(0)));

  /* TODO: figure out how to differentiate between types of arg 1
           in the meanwhile, ParameterList is used more frequently so
           expose that constructor
  else if (context->argumentCount() == 2 &&
           context->argument(1) is a QSqlDatabase)
    obj = new orReport(context->argument(0).toString(),
                       qscriptvalue_cast<QSqlDatabase>(context->argument(1)));
   */
  else if (context->argumentCount() == 2 /* &&
           context->argument(1) is a ParameterList */)
    obj = new orReport(context->argument(0).toString(),
                       qscriptvalue_cast<ParameterList>(context->argument(1).toObject()));

  else if (context->argumentCount() == 3)
    obj = new orReport(context->argument(0).toString(),
                       qscriptvalue_cast<ParameterList>(context->argument(1)),
                       qscriptvalue_cast<QSqlDatabase>(context->argument(2)));

  return engine->toScriptValue(obj);
}

orReportProto::orReportProto(QObject *parent)
  : QObject(parent)
{
}

int orReportProto::backgroundAlignment()
{
  orReport *item = qscriptvalue_cast<orReport*>(thisObject());
  if (item)
    return item->backgroundAlignment();
  return 0;
}

QImage orReportProto::backgroundImage()
{
  orReport *item = qscriptvalue_cast<orReport*>(thisObject());
  if (item)
    return item->backgroundImage();
  return QImage();
}

unsigned char orReportProto::backgroundOpacity()
{
  orReport *item = qscriptvalue_cast<orReport*>(thisObject());
  if (item)
    return item->backgroundOpacity();
  return 0;
}

QRect orReportProto::backgroundRect()
{
  orReport *item = qscriptvalue_cast<orReport*>(thisObject());
  if (item)
    return item->backgroundRect();
  return QRect();
}

bool orReportProto::backgroundScale()
{
  orReport *item = qscriptvalue_cast<orReport*>(thisObject());
  if (item)
    return item->backgroundScale();
  return false;
}

Qt::AspectRatioMode orReportProto::backgroundScaleMode()
{
  orReport *item = qscriptvalue_cast<orReport*>(thisObject());
  if (item)
    return item->backgroundScaleMode();
  return Qt::IgnoreAspectRatio;
}

bool orReportProto::doParamsSatisfy()
{
  orReport *item = qscriptvalue_cast<orReport*>(thisObject());
  if (item)
    return item->doParamsSatisfy();
  return false;
}

bool orReportProto::doesReportExist()
{
  orReport *item = qscriptvalue_cast<orReport*>(thisObject());
  if (item)
    return item->doesReportExist();
  return false;
}

bool orReportProto::exportToPDF(const QString& fileName)
{
  orReport *item = qscriptvalue_cast<orReport*>(thisObject());
  if (item)
    return item->exportToPDF(fileName);
  return false;
}

ParameterList orReportProto::getParamList()
{
  orReport *item = qscriptvalue_cast<orReport*>(thisObject());
  if (item)
    return item->getParamList();
  return ParameterList();
}

bool orReportProto::isValid()
{
  orReport *item = qscriptvalue_cast<orReport*>(thisObject());
  if (item)
    return item->isValid();
  return false;
}

bool orReportProto::print(QPrinter *prtThis, bool boolSetupPrinter, bool showPreview)
{
  orReport *item = qscriptvalue_cast<orReport*>(thisObject());
  if (item)
    return item->print(prtThis, boolSetupPrinter, showPreview);
  return false;
}

bool orReportProto::render(QPainter *painter, QPrinter *printer)
{
  orReport *item = qscriptvalue_cast<orReport*>(thisObject());
  if (item)
    return item->render(painter, printer);
  return false;
}

int orReportProto::reportError(QWidget *widget)
{
  orReport *item = qscriptvalue_cast<orReport*>(thisObject());
  if (item)
    return item->reportError(widget);
  return 0;
}

bool orReportProto::satisfyParams(QWidget *widget)
{
  orReport *item = qscriptvalue_cast<orReport*>(thisObject());
  if (item)
    return item->satisfyParams(widget);
  return false;
}

bool orReportProto::setDom(const QDomDocument &docPReport)
{
  orReport *item = qscriptvalue_cast<orReport*>(thisObject());
  if (item)
    return item->setDom(docPReport);
  return false;
}

void orReportProto::setBackgroundAlignment(int alignment)
{
  orReport *item = qscriptvalue_cast<orReport*>(thisObject());
  if (item)
    item->setBackgroundAlignment(alignment);
}

void orReportProto::setBackgroundImage(const QImage &image)
{
  orReport *item = qscriptvalue_cast<orReport*>(thisObject());
  if (item)
    item->setBackgroundImage(image);
}

void orReportProto::setBackgroundOpacity(unsigned char opacity)
{
  orReport *item = qscriptvalue_cast<orReport*>(thisObject());
  if (item)
    item->setBackgroundOpacity(opacity);
}

void orReportProto::setBackgroundRect(const QRect &rect)
{
  orReport *item = qscriptvalue_cast<orReport*>(thisObject());
  if (item)
    item->setBackgroundRect(rect);
}

void orReportProto::setBackgroundRect(int x, int y, int w, int h) 
{
  orReport *item = qscriptvalue_cast<orReport*>(thisObject());
  if (item)
    item->setBackgroundRect(x, y, w, h);
}

void orReportProto::setBackgroundScale(bool scale)
{
  orReport *item = qscriptvalue_cast<orReport*>(thisObject());
  if (item)
    item->setBackgroundScale(scale);
}

void orReportProto::setBackgroundScaleMode(Qt::AspectRatioMode mode)
{
  orReport *item = qscriptvalue_cast<orReport*>(thisObject());
  if (item)
    item->setBackgroundScaleMode(mode);
}

void orReportProto::setDatabase(QSqlDatabase db)
{
  orReport *item = qscriptvalue_cast<orReport*>(thisObject());
  if (item)
    item->setDatabase(db);
}

void orReportProto::setParamList(const ParameterList &params)
{
  orReport *item = qscriptvalue_cast<orReport*>(thisObject());
  if (item)
    item->setParamList(params);
}

void orReportProto::setParamList(const QStringList &list)
{
  orReport *item = qscriptvalue_cast<orReport*>(thisObject());
  if (item)
    item->setParamList(list);
}

void orReportProto::setWatermarkFont(const QFont &font)
{
  orReport *item = qscriptvalue_cast<orReport*>(thisObject());
  if (item)
    item->setWatermarkFont(font);
}

void orReportProto::setWatermarkOpacity(unsigned char opacity)
{
  orReport *item = qscriptvalue_cast<orReport*>(thisObject());
  if (item)
    item->setWatermarkOpacity(opacity);
}

void orReportProto::setWatermarkText(const QString &text)
{
  orReport *item = qscriptvalue_cast<orReport*>(thisObject());
  if (item)
    item->setWatermarkText(text);
}

QFont orReportProto::watermarkFont()
{
  orReport *item = qscriptvalue_cast<orReport*>(thisObject());
  if (item)
    return item->watermarkFont();
  return QFont();
}

unsigned char orReportProto::watermarkOpacity()
{
  orReport *item = qscriptvalue_cast<orReport*>(thisObject());
  if (item)
    return item->watermarkOpacity();
  return 0;
}

QString orReportProto::watermarkText()
{
  orReport *item = qscriptvalue_cast<orReport*>(thisObject());
  if (item)
    return item->watermarkText();
  return QString();
}

/*
QString orReportProto::toString()     const
{
  orReport *item = qscriptvalue_cast<orReport*>(thisObject());
  if (item)
    return item->toString();
  return QString();
}
*/
