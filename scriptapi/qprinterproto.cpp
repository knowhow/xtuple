/*
 *This file is part of the xTuple ERP: PostBooks Edition, a free and
 *open source Enterprise Resource Planning software suite,
 *Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 *It is licensed to you under the Common Public Attribution License
 *version 1.0, the full text of which(including xTuple-specific Exhibits)
 *is available at www.xtuple.com/CPAL.  By using this software, you agree
 *to be bound by its terms.
 */

#include "qprinterproto.h"

#include <QString>

void setupQPrinterProto(QScriptEngine *engine)
{
  QScriptValue proto = engine->newQObject(new QPrinterProto(engine));
  engine->setDefaultPrototype(qMetaTypeId<QPrinter*>(), proto);
  //engine->setDefaultPrototype(qMetaTypeId<QPrinter>(),  proto);

  QScriptValue constructor = engine->newFunction(constructQPrinter,
                                                 proto);
  engine->globalObject().setProperty("QPrinter",  constructor);

  // enum QPrinter::ColorMode
  constructor.setProperty("Color",    QScriptValue(engine, QPrinter::Color),    QScriptValue::ReadOnly | QScriptValue::Undeletable);
  constructor.setProperty("GrayScale",QScriptValue(engine, QPrinter::GrayScale),QScriptValue::ReadOnly | QScriptValue::Undeletable);

  // enum QPrinter::DuplexMode
  constructor.setProperty("DuplexNone",     QScriptValue(engine, QPrinter::DuplexNone),     QScriptValue::ReadOnly | QScriptValue::Undeletable);
  constructor.setProperty("DuplexAuto",     QScriptValue(engine, QPrinter::DuplexAuto),     QScriptValue::ReadOnly | QScriptValue::Undeletable);
  constructor.setProperty("DuplexLongSide", QScriptValue(engine, QPrinter::DuplexLongSide), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  constructor.setProperty("DuplexShortSide",QScriptValue(engine, QPrinter::DuplexShortSide),QScriptValue::ReadOnly | QScriptValue::Undeletable);

  // enum QPrinter::Orientation
  constructor.setProperty("Portrait", QScriptValue(engine, QPrinter::Portrait), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  constructor.setProperty("Landscape",QScriptValue(engine, QPrinter::Landscape),QScriptValue::ReadOnly | QScriptValue::Undeletable);

  // enum QPrinter::OutputFormat
  constructor.setProperty("NativeFormat",    QScriptValue(engine, QPrinter::NativeFormat),    QScriptValue::ReadOnly | QScriptValue::Undeletable);
  constructor.setProperty("PdfFormat",       QScriptValue(engine, QPrinter::PdfFormat),       QScriptValue::ReadOnly | QScriptValue::Undeletable);
  constructor.setProperty("PostScriptFormat",QScriptValue(engine, QPrinter::PostScriptFormat),QScriptValue::ReadOnly | QScriptValue::Undeletable);

  // enum QPrinter::PageOrder
  constructor.setProperty("FirstPageFirst",QScriptValue(engine, QPrinter::FirstPageFirst),QScriptValue::ReadOnly | QScriptValue::Undeletable);
  constructor.setProperty("LastPageFirst", QScriptValue(engine, QPrinter::LastPageFirst), QScriptValue::ReadOnly | QScriptValue::Undeletable);

  // enum QPrinter::PaperSize
  constructor.setProperty("A0",       QScriptValue(engine, QPrinter::A0),       QScriptValue::ReadOnly | QScriptValue::Undeletable);
  constructor.setProperty("A1",       QScriptValue(engine, QPrinter::A1),       QScriptValue::ReadOnly | QScriptValue::Undeletable);
  constructor.setProperty("A2",       QScriptValue(engine, QPrinter::A2),       QScriptValue::ReadOnly | QScriptValue::Undeletable);
  constructor.setProperty("A3",       QScriptValue(engine, QPrinter::A3),       QScriptValue::ReadOnly | QScriptValue::Undeletable);
  constructor.setProperty("A4",       QScriptValue(engine, QPrinter::A4),       QScriptValue::ReadOnly | QScriptValue::Undeletable);
  constructor.setProperty("A5",       QScriptValue(engine, QPrinter::A5),       QScriptValue::ReadOnly | QScriptValue::Undeletable);
  constructor.setProperty("A6",       QScriptValue(engine, QPrinter::A6),       QScriptValue::ReadOnly | QScriptValue::Undeletable);
  constructor.setProperty("A7",       QScriptValue(engine, QPrinter::A7),       QScriptValue::ReadOnly | QScriptValue::Undeletable);
  constructor.setProperty("A8",       QScriptValue(engine, QPrinter::A8),       QScriptValue::ReadOnly | QScriptValue::Undeletable);
  constructor.setProperty("A9",       QScriptValue(engine, QPrinter::A9),       QScriptValue::ReadOnly | QScriptValue::Undeletable);
  constructor.setProperty("B0",       QScriptValue(engine, QPrinter::B0),       QScriptValue::ReadOnly | QScriptValue::Undeletable);
  constructor.setProperty("B1",       QScriptValue(engine, QPrinter::B1),       QScriptValue::ReadOnly | QScriptValue::Undeletable);
  constructor.setProperty("B10",      QScriptValue(engine, QPrinter::B10),      QScriptValue::ReadOnly | QScriptValue::Undeletable);
  constructor.setProperty("B2",       QScriptValue(engine, QPrinter::B2),       QScriptValue::ReadOnly | QScriptValue::Undeletable);
  constructor.setProperty("B3",       QScriptValue(engine, QPrinter::B3),       QScriptValue::ReadOnly | QScriptValue::Undeletable);
  constructor.setProperty("B4",       QScriptValue(engine, QPrinter::B4),       QScriptValue::ReadOnly | QScriptValue::Undeletable);
  constructor.setProperty("B5",       QScriptValue(engine, QPrinter::B5),       QScriptValue::ReadOnly | QScriptValue::Undeletable);
  constructor.setProperty("B6",       QScriptValue(engine, QPrinter::B6),       QScriptValue::ReadOnly | QScriptValue::Undeletable);
  constructor.setProperty("B7",       QScriptValue(engine, QPrinter::B7),       QScriptValue::ReadOnly | QScriptValue::Undeletable);
  constructor.setProperty("B8",       QScriptValue(engine, QPrinter::B8),       QScriptValue::ReadOnly | QScriptValue::Undeletable);
  constructor.setProperty("B9",       QScriptValue(engine, QPrinter::B9),       QScriptValue::ReadOnly | QScriptValue::Undeletable);
  constructor.setProperty("C5E",      QScriptValue(engine, QPrinter::C5E),      QScriptValue::ReadOnly | QScriptValue::Undeletable);
  constructor.setProperty("Comm10E",  QScriptValue(engine, QPrinter::Comm10E),  QScriptValue::ReadOnly | QScriptValue::Undeletable);
  constructor.setProperty("DLE",      QScriptValue(engine, QPrinter::DLE),      QScriptValue::ReadOnly | QScriptValue::Undeletable);
  constructor.setProperty("Executive",QScriptValue(engine, QPrinter::Executive),QScriptValue::ReadOnly | QScriptValue::Undeletable);
  constructor.setProperty("Folio",    QScriptValue(engine, QPrinter::Folio),    QScriptValue::ReadOnly | QScriptValue::Undeletable);
  constructor.setProperty("Ledger",   QScriptValue(engine, QPrinter::Ledger),   QScriptValue::ReadOnly | QScriptValue::Undeletable);
  constructor.setProperty("Legal",    QScriptValue(engine, QPrinter::Legal),    QScriptValue::ReadOnly | QScriptValue::Undeletable);
  constructor.setProperty("Letter",   QScriptValue(engine, QPrinter::Letter),   QScriptValue::ReadOnly | QScriptValue::Undeletable);
  constructor.setProperty("Tabloid",  QScriptValue(engine, QPrinter::Tabloid),  QScriptValue::ReadOnly | QScriptValue::Undeletable);
  constructor.setProperty("Custom",   QScriptValue(engine, QPrinter::Custom),   QScriptValue::ReadOnly | QScriptValue::Undeletable);

  // enum QPrinter::PaperSource
  constructor.setProperty("Auto",          QScriptValue(engine, QPrinter::Auto),          QScriptValue::ReadOnly | QScriptValue::Undeletable);
  constructor.setProperty("Cassette",      QScriptValue(engine, QPrinter::Cassette),      QScriptValue::ReadOnly | QScriptValue::Undeletable);
  constructor.setProperty("Envelope",      QScriptValue(engine, QPrinter::Envelope),      QScriptValue::ReadOnly | QScriptValue::Undeletable);
  constructor.setProperty("EnvelopeManual",QScriptValue(engine, QPrinter::EnvelopeManual),QScriptValue::ReadOnly | QScriptValue::Undeletable);
  constructor.setProperty("FormSource",    QScriptValue(engine, QPrinter::FormSource),    QScriptValue::ReadOnly | QScriptValue::Undeletable);
  constructor.setProperty("LargeCapacity", QScriptValue(engine, QPrinter::LargeCapacity), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  constructor.setProperty("LargeFormat",   QScriptValue(engine, QPrinter::LargeFormat),   QScriptValue::ReadOnly | QScriptValue::Undeletable);
  constructor.setProperty("Lower",         QScriptValue(engine, QPrinter::Lower),         QScriptValue::ReadOnly | QScriptValue::Undeletable);
  constructor.setProperty("MaxPageSource", QScriptValue(engine, QPrinter::MaxPageSource), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  constructor.setProperty("Middle",        QScriptValue(engine, QPrinter::Middle),        QScriptValue::ReadOnly | QScriptValue::Undeletable);
  constructor.setProperty("Manual",        QScriptValue(engine, QPrinter::Manual),        QScriptValue::ReadOnly | QScriptValue::Undeletable);
  constructor.setProperty("OnlyOne",       QScriptValue(engine, QPrinter::OnlyOne),       QScriptValue::ReadOnly | QScriptValue::Undeletable);
  constructor.setProperty("Tractor",       QScriptValue(engine, QPrinter::Tractor),       QScriptValue::ReadOnly | QScriptValue::Undeletable);
  constructor.setProperty("SmallFormat",   QScriptValue(engine, QPrinter::SmallFormat),   QScriptValue::ReadOnly | QScriptValue::Undeletable);

  // enum QPrinter::PrintRange
  constructor.setProperty("AllPages",  QScriptValue(engine, QPrinter::AllPages),  QScriptValue::ReadOnly | QScriptValue::Undeletable);
  constructor.setProperty("Selection", QScriptValue(engine, QPrinter::Selection), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  constructor.setProperty("PageRange", QScriptValue(engine, QPrinter::PageRange), QScriptValue::ReadOnly | QScriptValue::Undeletable);

  // enum QPrinter::PrinterMode
  constructor.setProperty("ScreenResolution", QScriptValue(engine, QPrinter::ScreenResolution), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  constructor.setProperty("PrinterResolution",QScriptValue(engine, QPrinter::PrinterResolution),QScriptValue::ReadOnly | QScriptValue::Undeletable);
  constructor.setProperty("HighResolution",   QScriptValue(engine, QPrinter::HighResolution),   QScriptValue::ReadOnly | QScriptValue::Undeletable);

  // enum QPrinter::PrinterState
  constructor.setProperty("Idle",   QScriptValue(engine, QPrinter::Idle),   QScriptValue::ReadOnly | QScriptValue::Undeletable);
  constructor.setProperty("Active", QScriptValue(engine, QPrinter::Active), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  constructor.setProperty("Aborted",QScriptValue(engine, QPrinter::Aborted),QScriptValue::ReadOnly | QScriptValue::Undeletable);
  constructor.setProperty("Error",  QScriptValue(engine, QPrinter::Error),  QScriptValue::ReadOnly | QScriptValue::Undeletable);

  // enum QPrinter::Unit
  constructor.setProperty("Millimeter",QScriptValue(engine, QPrinter::Millimeter),QScriptValue::ReadOnly | QScriptValue::Undeletable);
  constructor.setProperty("Point",     QScriptValue(engine, QPrinter::Point),     QScriptValue::ReadOnly | QScriptValue::Undeletable);
  constructor.setProperty("Inch",      QScriptValue(engine, QPrinter::Inch),      QScriptValue::ReadOnly | QScriptValue::Undeletable);
  constructor.setProperty("Pica",      QScriptValue(engine, QPrinter::Pica),      QScriptValue::ReadOnly | QScriptValue::Undeletable);
  constructor.setProperty("Didot",     QScriptValue(engine, QPrinter::Didot),     QScriptValue::ReadOnly | QScriptValue::Undeletable);
  constructor.setProperty("Cicero",    QScriptValue(engine, QPrinter::Cicero),    QScriptValue::ReadOnly | QScriptValue::Undeletable);

}

QScriptValue constructQPrinter(QScriptContext *context, QScriptEngine *engine)
{
  QPrinter *obj = 0;
  if (context->argumentCount() == 0)
    obj = new QPrinter();
  else if (context->argumentCount() == 1 && context->argument(0).isNumber())
    obj = new QPrinter((QPrinter::PrinterMode)context->argument(0).toInt32());
  else
    context->throwError("Scripting does not yet support the "
                        "QPrinter(QPrinterInfo, PrinterMode) constructor");

  return engine->toScriptValue(obj);
}

QPrinterProto::QPrinterProto(QObject *parent)
    : QObject(parent)
{
}

bool QPrinterProto::abort()
{
  QPrinter *item = qscriptvalue_cast<QPrinter*>(thisObject());
  if (item)
    return item->abort();
  return false;
}

bool QPrinterProto::collateCopies() const
{
  QPrinter *item = qscriptvalue_cast<QPrinter*>(thisObject());
  if (item)
    return item->collateCopies();
  return false;
}

QPrinter::ColorMode QPrinterProto::colorMode() const
{
  QPrinter *item = qscriptvalue_cast<QPrinter*>(thisObject());
  if (item)
    return item->colorMode();
  return QPrinter::Color;
}

QString QPrinterProto::creator() const
{
  QPrinter *item = qscriptvalue_cast<QPrinter*>(thisObject());
  if (item)
    return item->creator();
  return QString();
}

int QPrinterProto::depth() const
{
  QPrinter *item = qscriptvalue_cast<QPrinter*>(thisObject());
  if (item)
    return item->depth();
  return 0;
}

QString QPrinterProto::docName() const
{
  QPrinter *item = qscriptvalue_cast<QPrinter*>(thisObject());
  if (item)
    return item->docName();
  return QString();
}

bool QPrinterProto::doubleSidedPrinting() const
{
  QPrinter *item = qscriptvalue_cast<QPrinter*>(thisObject());
  if (item)
    return item->doubleSidedPrinting();
  return false;
}

QPrinter::DuplexMode QPrinterProto::duplex() const
{
  QPrinter *item = qscriptvalue_cast<QPrinter*>(thisObject());
  if (item)
    return item->duplex();
  return QPrinter::DuplexNone;
}

bool QPrinterProto::fontEmbeddingEnabled() const
{
  QPrinter *item = qscriptvalue_cast<QPrinter*>(thisObject());
  if (item)
    return item->fontEmbeddingEnabled();
  return false;
}

int QPrinterProto::fromPage() const
{
  QPrinter *item = qscriptvalue_cast<QPrinter*>(thisObject());
  if (item)
    return item->fromPage();
  return 0;
}

bool QPrinterProto::fullPage() const
{
  QPrinter *item = qscriptvalue_cast<QPrinter*>(thisObject());
  if (item)
    return item->fullPage();
  return false;
}

/*
HDC QPrinterProto::getDC() const
{
  QPrinter *item = qscriptvalue_cast<QPrinter*>(thisObject());
  if (item)
    return item->getDC();
  return ????;
}
*/

void QPrinterProto::getPageMargins(qreal *left, qreal *top, qreal *right, qreal *bottom, QPrinter::Unit unit) const
{
  QPrinter *item = qscriptvalue_cast<QPrinter*>(thisObject());
  if (item)
    item->getPageMargins(left, top, right, bottom, unit);
}

int QPrinterProto::height() const
{
  QPrinter *item = qscriptvalue_cast<QPrinter*>(thisObject());
  if (item)
    return item->height();
  return 0;
}

int QPrinterProto::heightMM() const
{
  QPrinter *item = qscriptvalue_cast<QPrinter*>(thisObject());
  if (item)
    return item->heightMM();
  return 0;
}

bool QPrinterProto::isValid() const
{
  QPrinter *item = qscriptvalue_cast<QPrinter*>(thisObject());
  if (item)
    return item->isValid();
  return false;
}

int QPrinterProto::logicalDpiX() const
{
  QPrinter *item = qscriptvalue_cast<QPrinter*>(thisObject());
  if (item)
    return item->logicalDpiX();
  return 0;
}

int QPrinterProto::logicalDpiY() const
{
  QPrinter *item = qscriptvalue_cast<QPrinter*>(thisObject());
  if (item)
    return item->logicalDpiY();
  return 0;
}

bool QPrinterProto::newPage()
{
  QPrinter *item = qscriptvalue_cast<QPrinter*>(thisObject());
  if (item)
    return item->newPage();
  return false;
}

int QPrinterProto::numColors() const
{
  QPrinter *item = qscriptvalue_cast<QPrinter*>(thisObject());
  if (item)
    return item->numColors();
  return 0;
}

int QPrinterProto::numCopies() const
{
  QPrinter *item = qscriptvalue_cast<QPrinter*>(thisObject());
  if (item)
    return item->numCopies();
  return 0;
}

QPrinter::Orientation QPrinterProto::orientation() const
{
  QPrinter *item = qscriptvalue_cast<QPrinter*>(thisObject());
  if (item)
    return item->orientation();
  return QPrinter::Portrait;
}

QString QPrinterProto::outputFileName() const
{
  QPrinter *item = qscriptvalue_cast<QPrinter*>(thisObject());
  if (item)
    return item->outputFileName();
  return QString();
}

QPrinter::OutputFormat QPrinterProto::outputFormat() const
{
  QPrinter *item = qscriptvalue_cast<QPrinter*>(thisObject());
  if (item)
    return item->outputFormat();
  return QPrinter::NativeFormat;
}

QPrinter::PageOrder QPrinterProto::pageOrder() const
{
  QPrinter *item = qscriptvalue_cast<QPrinter*>(thisObject());
  if (item)
    return item->pageOrder();
  return QPrinter::FirstPageFirst;
}

QRect QPrinterProto::pageRect() const
{
  QPrinter *item = qscriptvalue_cast<QPrinter*>(thisObject());
  if (item)
    return item->pageRect();
  return QRect();
}

QRectF QPrinterProto::pageRect(QPrinter::Unit unit) const
{
  QPrinter *item = qscriptvalue_cast<QPrinter*>(thisObject());
  if (item)
    return item->pageRect(unit);
  return QRectF();
}

QPaintEngine *QPrinterProto::paintEngine() const
{
  QPrinter *item = qscriptvalue_cast<QPrinter*>(thisObject());
  if (item)
    return item->paintEngine();
  return 0;
}

bool QPrinterProto::paintingActive() const
{
  QPrinter *item = qscriptvalue_cast<QPrinter*>(thisObject());
  if (item)
    return item->paintingActive();
  return false;
}

QRect QPrinterProto::paperRect() const
{
  QPrinter *item = qscriptvalue_cast<QPrinter*>(thisObject());
  if (item)
    return item->paperRect();
  return QRect();
}

QRectF QPrinterProto::paperRect(QPrinter::Unit unit) const
{
  QPrinter *item = qscriptvalue_cast<QPrinter*>(thisObject());
  if (item)
    return item->paperRect(unit);
  return QRectF();
}

QPrinter::PaperSize QPrinterProto::paperSize() const
{
  QPrinter *item = qscriptvalue_cast<QPrinter*>(thisObject());
  if (item)
    return item->paperSize();
  return QPrinter::Letter;
}

QSizeF QPrinterProto::paperSize(QPrinter::Unit unit) const
{
  QPrinter *item = qscriptvalue_cast<QPrinter*>(thisObject());
  if (item)
    return item->paperSize(unit);
  return QSizeF();
}

QPrinter::PaperSource QPrinterProto::paperSource() const
{
  QPrinter *item = qscriptvalue_cast<QPrinter*>(thisObject());
  if (item)
    return item->paperSource();
  return QPrinter::Auto;
}

int QPrinterProto::physicalDpiX() const
{
  QPrinter *item = qscriptvalue_cast<QPrinter*>(thisObject());
  if (item)
    return item->physicalDpiX();
  return 0;
}

int QPrinterProto::physicalDpiY() const
{
  QPrinter *item = qscriptvalue_cast<QPrinter*>(thisObject());
  if (item)
    return item->physicalDpiY();
  return 0;
}

QPrintEngine *QPrinterProto::printEngine() const
{
  QPrinter *item = qscriptvalue_cast<QPrinter*>(thisObject());
  if (item)
    return item->printEngine();
  return 0;
}

QString QPrinterProto::printProgram() const
{
  QPrinter *item = qscriptvalue_cast<QPrinter*>(thisObject());
  if (item)
    return item->printProgram();
  return QString();
}

QPrinter::PrintRange QPrinterProto::printRange() const
{
  QPrinter *item = qscriptvalue_cast<QPrinter*>(thisObject());
  if (item)
    return item->printRange();
  return QPrinter::AllPages;
}

QString QPrinterProto::printerName() const
{
  QPrinter *item = qscriptvalue_cast<QPrinter*>(thisObject());
  if (item)
    return item->printerName();
  return QString();
}

#ifndef Q_WS_WIN
QString QPrinterProto::printerSelectionOption() const
{
  QPrinter *item = qscriptvalue_cast<QPrinter*>(thisObject());
  if (item)
    return item->printerSelectionOption();
  return QString();
}
#endif

QPrinter::PrinterState QPrinterProto::printerState() const
{
  QPrinter *item = qscriptvalue_cast<QPrinter*>(thisObject());
  if (item)
    return item->printerState();
  return QPrinter::Idle;
}

/*
void QPrinterProto::releaseDC(HDC hdc) const
{
  QPrinter *item = qscriptvalue_cast<QPrinter*>(thisObject());
  if (item)
    item->releaseDC(hdc);
}
*/

int QPrinterProto::resolution() const
{
  QPrinter *item = qscriptvalue_cast<QPrinter*>(thisObject());
  if (item)
    return item->resolution();
  return 0;
}

void QPrinterProto::setCollateCopies(bool collate)
{
  QPrinter *item = qscriptvalue_cast<QPrinter*>(thisObject());
  if (item)
    item->setCollateCopies(collate);
}

void QPrinterProto::setColorMode(QPrinter::ColorMode newColorMode)
{
  QPrinter *item = qscriptvalue_cast<QPrinter*>(thisObject());
  if (item)
    item->setColorMode(newColorMode);
}

void QPrinterProto::setCreator(const QString & creator)
{
  QPrinter *item = qscriptvalue_cast<QPrinter*>(thisObject());
  if (item)
    item->setCreator(creator);
}

void QPrinterProto::setDocName(const QString & name)
{
  QPrinter *item = qscriptvalue_cast<QPrinter*>(thisObject());
  if (item)
    item->setDocName(name);
}

void QPrinterProto::setDoubleSidedPrinting(bool doubleSided)
{
  QPrinter *item = qscriptvalue_cast<QPrinter*>(thisObject());
  if (item)
    item->setDoubleSidedPrinting(doubleSided);
}

void QPrinterProto::setDuplex(QPrinter::DuplexMode duplex)
{
  QPrinter *item = qscriptvalue_cast<QPrinter*>(thisObject());
  if (item)
    item->setDuplex(duplex);
}

void QPrinterProto::setFontEmbeddingEnabled(bool enable)
{
  QPrinter *item = qscriptvalue_cast<QPrinter*>(thisObject());
  if (item)
    item->setFontEmbeddingEnabled(enable);
}

void QPrinterProto::setFromTo(int from, int to)
{
  QPrinter *item = qscriptvalue_cast<QPrinter*>(thisObject());
  if (item)
    item->setFromTo(from, to);
}

void QPrinterProto::setFullPage(bool fp)
{
  QPrinter *item = qscriptvalue_cast<QPrinter*>(thisObject());
  if (item)
    item->setFullPage(fp);
}

void QPrinterProto::setNumCopies(int numCopies)
{
  QPrinter *item = qscriptvalue_cast<QPrinter*>(thisObject());
  if (item)
    item->setNumCopies(numCopies);
}

void QPrinterProto::setOrientation(QPrinter::Orientation orientation)
{
  QPrinter *item = qscriptvalue_cast<QPrinter*>(thisObject());
  if (item)
    item->setOrientation(orientation);
}

void QPrinterProto::setOutputFileName(const QString & fileName)
{
  QPrinter *item = qscriptvalue_cast<QPrinter*>(thisObject());
  if (item)
    item->setOutputFileName(fileName);
}

void QPrinterProto::setOutputFormat(QPrinter::OutputFormat format)
{
  QPrinter *item = qscriptvalue_cast<QPrinter*>(thisObject());
  if (item)
    item->setOutputFormat(format);
}

void QPrinterProto::setPageMargins(qreal left, qreal top, qreal right, qreal bottom, QPrinter::Unit unit)
{
  QPrinter *item = qscriptvalue_cast<QPrinter*>(thisObject());
  if (item)
    item->setPageMargins(left, top, right, bottom, unit);
}

void QPrinterProto::setPageOrder(QPrinter::PageOrder pageOrder)
{
  QPrinter *item = qscriptvalue_cast<QPrinter*>(thisObject());
  if (item)
    item->setPageOrder(pageOrder);
}

void QPrinterProto::setPaperSize(QPrinter::PaperSize newPaperSize)
{
  QPrinter *item = qscriptvalue_cast<QPrinter*>(thisObject());
  if (item)
    item->setPaperSize(newPaperSize);
}

void QPrinterProto::setPaperSize(const QSizeF & paperSize, QPrinter::Unit unit)
{
  QPrinter *item = qscriptvalue_cast<QPrinter*>(thisObject());
  if (item)
    item->setPaperSize(paperSize, unit);
}

void QPrinterProto::setPaperSource(QPrinter::PaperSource source)
{
  QPrinter *item = qscriptvalue_cast<QPrinter*>(thisObject());
  if (item)
    item->setPaperSource(source);
}

void QPrinterProto::setPrintProgram(const QString & printProg)
{
  QPrinter *item = qscriptvalue_cast<QPrinter*>(thisObject());
  if (item)
    item->setPrintProgram(printProg);
}

void QPrinterProto::setPrintRange(QPrinter::PrintRange range)
{
  QPrinter *item = qscriptvalue_cast<QPrinter*>(thisObject());
  if (item)
    item->setPrintRange(range);
}

void QPrinterProto::setPrinterName(const QString & name)
{
  QPrinter *item = qscriptvalue_cast<QPrinter*>(thisObject());
  if (item)
    item->setPrinterName(name);
}

#ifndef Q_WS_WIN
void QPrinterProto::setPrinterSelectionOption(const QString & option)
{
  QPrinter *item = qscriptvalue_cast<QPrinter*>(thisObject());
  if (item)
    item->setPrinterSelectionOption(option);
}
#endif

void QPrinterProto::setResolution(int dpi)
{
  QPrinter *item = qscriptvalue_cast<QPrinter*>(thisObject());
  if (item)
    item->setResolution(dpi);
}

#ifdef Q_WS_WIN
void QPrinterProto::setWinPageSize(int pageSize)
{
  QPrinter *item = qscriptvalue_cast<QPrinter*>(thisObject());
  if (item)
    item->setWinPageSize(pageSize);
}

QList<QPrinter::PaperSource> QPrinterProto::supportedPaperSources() const
{
  QPrinter *item = qscriptvalue_cast<QPrinter*>(thisObject());
  if (item)
    return item->supportedPaperSources();
  return QList<QPrinter::PaperSource>();
}
#endif

QList<int> QPrinterProto::supportedResolutions() const
{
  QPrinter *item = qscriptvalue_cast<QPrinter*>(thisObject());
  if (item)
    return item->supportedResolutions();
  return QList<int>();
}

int QPrinterProto::toPage() const
{
  QPrinter *item = qscriptvalue_cast<QPrinter*>(thisObject());
  if (item)
    return item->toPage();
  return 0;
}

int QPrinterProto::width() const
{
  QPrinter *item = qscriptvalue_cast<QPrinter*>(thisObject());
  if (item)
    return item->width();
  return 0;
}

int QPrinterProto::widthMM() const
{
  QPrinter *item = qscriptvalue_cast<QPrinter*>(thisObject());
  if (item)
    return item->widthMM();
  return 0;
}

#ifdef Q_WS_WIN
int QPrinterProto::winPageSize() const
{
  QPrinter *item = qscriptvalue_cast<QPrinter*>(thisObject());
  if (item)
    return item->winPageSize();
  return 0;
}
#endif

/*
QString QPrinterProto::toString() const
{
  QPrinter *item = qscriptvalue_cast<QPrinter*>(thisObject());
  if (item)
    return QString("QPrinter()");
  return QString("QPrinter(unknown)");
}
*/
