/*
 *This file is part of the xTuple ERP: PostBooks Edition, a free and
 *open source Enterprise Resource Planning software suite,
 *Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 *It is licensed to you under the Common Public Attribution License
 *version 1.0, the full text of which(including xTuple-specific Exhibits)
 *is available at www.xtuple.com/CPAL.  By using this software, you agree
 *to be bound by its terms.
 */

#ifndef __QPRINTERPROTO_H__
#define __QPRINTERPROTO_H__

#include <QPrinter>
#include <QtScript>

class QString;

Q_DECLARE_METATYPE(QPrinter*)
//Q_DECLARE_METATYPE(QPrinter)

Q_DECLARE_METATYPE(enum QPrinter::ColorMode)
Q_DECLARE_METATYPE(enum QPrinter::DuplexMode)
Q_DECLARE_METATYPE(enum QPrinter::Orientation)
Q_DECLARE_METATYPE(enum QPrinter::OutputFormat)
Q_DECLARE_METATYPE(enum QPrinter::PageOrder)
Q_DECLARE_METATYPE(/*enum*/ QPrinter::PaperSize)
Q_DECLARE_METATYPE(enum QPrinter::PaperSource)
Q_DECLARE_METATYPE(enum QPrinter::PrintRange)
Q_DECLARE_METATYPE(enum QPrinter::PrinterMode)
Q_DECLARE_METATYPE(enum QPrinter::PrinterState)
Q_DECLARE_METATYPE(enum QPrinter::Unit)

void setupQPrinterProto(QScriptEngine *engine);
QScriptValue constructQPrinter(QScriptContext *context, QScriptEngine *engine);

class QPrinterProto : public QObject, public QScriptable
{
  Q_OBJECT

  public:
    QPrinterProto(QObject *parent);


    Q_INVOKABLE bool            abort();
    Q_INVOKABLE bool            collateCopies() const;
    Q_INVOKABLE QPrinter::ColorMode       colorMode() const;
    Q_INVOKABLE QString         creator() const;
    Q_INVOKABLE int             depth() const;
    Q_INVOKABLE QString         docName() const;
    Q_INVOKABLE bool            doubleSidedPrinting() const;
    Q_INVOKABLE QPrinter::DuplexMode      duplex() const;
    Q_INVOKABLE bool            fontEmbeddingEnabled() const;
    Q_INVOKABLE int             fromPage() const;
    Q_INVOKABLE bool            fullPage() const;
 // Q_INVOKABLE virtual HDC     getDC() const; // what's an HDC?
    Q_INVOKABLE void            getPageMargins(qreal *left, qreal *top, qreal *right, qreal *bottom, QPrinter::Unit unit) const;
    Q_INVOKABLE int             height() const;
    Q_INVOKABLE int             heightMM() const;
    Q_INVOKABLE bool            isValid() const;
    Q_INVOKABLE int             logicalDpiX() const;
    Q_INVOKABLE int             logicalDpiY() const;
    Q_INVOKABLE bool            newPage();
    Q_INVOKABLE int             numColors() const;
    Q_INVOKABLE int             numCopies() const;
    Q_INVOKABLE QPrinter::Orientation     orientation() const;
    Q_INVOKABLE QString         outputFileName() const;
    Q_INVOKABLE QPrinter::OutputFormat    outputFormat() const;
    Q_INVOKABLE QPrinter::PageOrder       pageOrder() const;
    Q_INVOKABLE QRect           pageRect() const;
    Q_INVOKABLE QRectF          pageRect(QPrinter::Unit unit) const;
    Q_INVOKABLE virtual QPaintEngine *paintEngine() const;
    Q_INVOKABLE bool            paintingActive() const;
    Q_INVOKABLE QRect           paperRect() const;
    Q_INVOKABLE QRectF          paperRect(QPrinter::Unit unit) const;
    Q_INVOKABLE QPrinter::PaperSize       paperSize() const;
    Q_INVOKABLE QSizeF          paperSize(QPrinter::Unit unit) const;
    Q_INVOKABLE QPrinter::PaperSource     paperSource() const;
    Q_INVOKABLE int             physicalDpiX() const;
    Q_INVOKABLE int             physicalDpiY() const;
    Q_INVOKABLE QPrintEngine   *printEngine() const;
    Q_INVOKABLE QString         printProgram() const;
    Q_INVOKABLE QPrinter::PrintRange      printRange() const;
    Q_INVOKABLE QString         printerName() const;
#ifndef Q_WS_WIN
    Q_INVOKABLE QString         printerSelectionOption() const;
#endif
    Q_INVOKABLE QPrinter::PrinterState    printerState() const;
 // Q_INVOKABLE virtual void    releaseDC(HDC hdc) const;
    Q_INVOKABLE int             resolution() const;
    Q_INVOKABLE void            setCollateCopies(bool collate);
    Q_INVOKABLE void            setColorMode(QPrinter::ColorMode newColorMode);
    Q_INVOKABLE void            setCreator(const QString & creator);
    Q_INVOKABLE void            setDocName(const QString & name);
    Q_INVOKABLE void            setDoubleSidedPrinting(bool doubleSided);
    Q_INVOKABLE void            setDuplex(QPrinter::DuplexMode duplex);
    Q_INVOKABLE void            setFontEmbeddingEnabled(bool enable);
    Q_INVOKABLE void            setFromTo(int from, int to);
    Q_INVOKABLE void            setFullPage(bool fp);
    Q_INVOKABLE void            setNumCopies(int numCopies);
    Q_INVOKABLE void            setOrientation(QPrinter::Orientation orientation);
    Q_INVOKABLE void            setOutputFileName(const QString & fileName);
    Q_INVOKABLE void            setOutputFormat(QPrinter::OutputFormat format);
    Q_INVOKABLE void            setPageMargins(qreal left, qreal top, qreal right, qreal bottom, QPrinter::Unit unit);
    Q_INVOKABLE void            setPageOrder(QPrinter::PageOrder pageOrder);
    Q_INVOKABLE void            setPaperSize(QPrinter::PaperSize newPaperSize);
    Q_INVOKABLE void            setPaperSize(const QSizeF & paperSize, QPrinter::Unit unit);
    Q_INVOKABLE void            setPaperSource(QPrinter::PaperSource source);
    Q_INVOKABLE void            setPrintProgram(const QString & printProg);
    Q_INVOKABLE void            setPrintRange(QPrinter::PrintRange range);
    Q_INVOKABLE void            setPrinterName(const QString & name);
#ifndef Q_WS_WIN
    Q_INVOKABLE void            setPrinterSelectionOption(const QString & option);
#endif
    Q_INVOKABLE void            setResolution(int dpi);
#ifdef Q_WS_WIN
    Q_INVOKABLE void            setWinPageSize(int pageSize);
    Q_INVOKABLE QList<QPrinter::PaperSource> supportedPaperSources() const;
#endif
    Q_INVOKABLE QList<int>      supportedResolutions() const;
    Q_INVOKABLE int             toPage() const;
    Q_INVOKABLE int             width() const;
    Q_INVOKABLE int             widthMM() const;
#ifdef Q_WS_WIN
    Q_INVOKABLE int             winPageSize() const;
#endif

};

#endif
