/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */


#ifndef __EXPORTHELPER_H__
#define __EXPORTHELPER_H__

#include <QDomNode>
#include <QFile>
#include <QObject>
#include <QString>

#include <parameter.h>

class QScriptEngine;

class ExportHelper : public QObject
{
  Q_OBJECT

  public:
    static bool exportHTML(const int qryheadid, ParameterList &params, QString &filename, QString &errmsg);
    static bool exportXML(const int qryheadid, ParameterList &params, QString &filename, QString &errmsg, const int xsltmapid = -1);
    static QString generateDelimited(const int qryheadid, ParameterList &params, QString &errmsg);
    static QString generateDelimited(QString qtext, ParameterList &params, QString &errmsg);
    static QString generateHTML(const int qryheadid, ParameterList &params, QString &errmsg);
    static QString generateHTML(QString qtext, ParameterList &params, QString &errmsg);
    static QString generateXML(const int qryheadid, ParameterList &params, QString &errmsg, int xsltmapid = -1);
    static QString generateXML(QString qtext, QString tableElemName, ParameterList &params, QString &errmsg, int xsltmapid = -1);
    static bool    XSLTConvertFile(QString inputfilename, QString outputfilename, QString xsltfilename, QString &errmsg);
    static bool    XSLTConvertFile(QString inputfilename, QString outputfilename, int xsltmapid, QString &errmsg);
    static QString XSLTConvertString(QString input, int xsltmapid, QString &errmsg);
};

void setupExportHelper(QScriptEngine *engine);

#endif
