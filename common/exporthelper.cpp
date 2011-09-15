/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "exporthelper.h"

#include <QDir>
#include <QDomDocument>
#include <QFileInfo>
#include <QMessageBox>
#include <QProcess>
#include <QScriptEngine>
#include <QScriptValue>
#include <QSqlError>
#include <QSqlRecord>
#include <QTemporaryFile>
#include <QTextCursor>
#include <QTextDocument>

#include "metasql.h"
#include "mqlutil.h"
#include "xsqlquery.h"

#define DEBUG false

bool ExportHelper::exportHTML(const int qryheadid, ParameterList &params, QString &filename, QString &errmsg)
{
  if (DEBUG)
    qDebug("ExportHelper::exportHTML(%d, %d params, %s, errmsg) entered",
           qryheadid, params.size(), qPrintable(filename));
  bool returnVal = false;

  XSqlQuery setq;
  setq.prepare("SELECT * FROM qryhead WHERE qryhead_id=:id;");
  setq.bindValue(":id", qryheadid);
  setq.exec();
  if (setq.first())
  {
    if (filename.isEmpty())
    {
      QFileInfo fileinfo(setq.value("qryhead_name").toString());
      filename = fileinfo.absoluteFilePath();
    }

    QFile *exportfile = new QFile(filename);
    if (! exportfile->open(QIODevice::ReadWrite | QIODevice::Truncate | QIODevice::Text))
      errmsg = tr("Could not open %1: %2.")
                                      .arg(filename, exportfile->errorString());
    else
    {
      if (exportfile->write(generateHTML(qryheadid, params, errmsg).toUtf8()) < 0)
        errmsg = tr("Error writing to %1: %2")
                                      .arg(filename, exportfile->errorString());
      exportfile->close();
    }
  }
  else if (setq.lastError().type() != QSqlError::NoError)
    errmsg = setq.lastError().text();
  else
    errmsg = tr("<p>Cannot export data because the query set with "
                "id %1 was not found.").arg(qryheadid);

  if (DEBUG)
    qDebug("ExportHelper::exportHTML returning %d, filename %s, and errmsg %s",
           returnVal, qPrintable(filename), qPrintable(errmsg));

  return returnVal;
}

/** \brief Export the results of a query set to an XML file.

  Run all of the queries in the given Query Set in the given order and write
  the results to an XML file. The XML file is constructed very simply:
  \verbatim
  <tablename>
    <column1name>columnvalue</column1name>
    <column2name>columnvalue</column2name>
    ...
  </tablename>
  \endverbatim

  If the caller passes in an XSLT map id, the simple XML will be processed
  using the export XSLT.

  \param qryheadid   The internal ID of the query set (qryhead record) to run.
  \param params      A list of parameters and values to use when building SQL
                     statements from MetaSQL statements.
  \param[in,out] filename The name of the file to create. If passed in empty,
                          a file named after the query set will be created
                          in the current directory (context-dependent) and
                          this filename will be passed back out.
  \param[out]    errmsg An message describing why the processing failed if there
                        was a problem.
  \param xsltmapid   An optional parameter. If this is set, it should be
                     the internal ID of an xsltmap record. The xsltmap_export
                     field of this record and the XSLTDefaultDir will be used
                     to find the XSLT script to run on the generated XML.
  */
bool ExportHelper::exportXML(const int qryheadid, ParameterList &params, QString &filename, QString &errmsg, const int xsltmapid)
{
  if (DEBUG)
    qDebug("ExportHelper::exportXML(%d, %d params, %s, errmsg, %d) entered",
           qryheadid, params.size(), qPrintable(filename), xsltmapid);
  bool returnVal = false;

  XSqlQuery setq;
  setq.prepare("SELECT * FROM qryhead WHERE qryhead_id=:id;");
  setq.bindValue(":id", qryheadid);
  setq.exec();
  if (setq.first())
  {
    if (filename.isEmpty())
    {
      QFileInfo fileinfo(setq.value("qryhead_name").toString());
      filename = fileinfo.absoluteFilePath();
    }

    QFile exportfile(filename);
    if (! exportfile.open(QIODevice::ReadWrite | QIODevice::Truncate | QIODevice::Text))
      errmsg = tr("Could not open %1 (%2).").arg(filename,exportfile.error());
    else
    {
      exportfile.write(generateXML(qryheadid, params, errmsg, xsltmapid).toUtf8());
      exportfile.close();
    }
  }
  else if (setq.lastError().type() != QSqlError::NoError)
    errmsg = setq.lastError().text();
  else
    errmsg = tr("<p>Cannot export data because the query set with "
                "id %1 was not found.").arg(qryheadid);

  if (DEBUG)
    qDebug("ExportHelper::exportXML returning %d, filename %s, and errmsg %s",
           returnVal, qPrintable(filename), qPrintable(errmsg));

  returnVal = errmsg.isEmpty();
  return returnVal;
}

QString ExportHelper::generateDelimited(const int qryheadid, ParameterList &params, QString &errmsg)
{
  if (DEBUG)
    qDebug("ExportHelper::exportDelimited(%d, %d params, errmsgd) entered",
           qryheadid, params.size());

  QStringList result;

  XSqlQuery itemq;
  itemq.prepare("SELECT *"
                "  FROM qryitem"
                " WHERE qryitem_qryhead_id=:id"
                " ORDER BY qryitem_order;");
  itemq.bindValue(":id", qryheadid);
  itemq.exec();
  while (itemq.next())
  {
    QString qtext;
    if (itemq.value("qryitem_src").toString() == "REL")
    {
      QString schemaName = itemq.value("qryitem_group").toString();
      qtext = "SELECT * FROM " +
              (schemaName.isEmpty() ? QString("") : schemaName + QString(".")) +
              itemq.value("qryitem_detail").toString();
    }
    else if (itemq.value("qryitem_src").toString() == "MQL")
    {
      QString tmpmsg;
      bool valid;
      qtext = MQLUtil::mqlLoad(itemq.value("qryitem_group").toString(),
                               itemq.value("qryitem_detail").toString(),
                               tmpmsg, &valid);
      if (! valid)
        errmsg = tmpmsg;
    }
    else if (itemq.value("qryitem_src").toString() == "CUSTOM")
      qtext = itemq.value("qryitem_detail").toString();

    if (! qtext.isEmpty())
    {
      QString oneresult = generateDelimited(qtext, params, errmsg);
      if (! oneresult.isEmpty())
        result.append(oneresult);
    }
  }
  if (itemq.lastError().type() != QSqlError::NoError)
    errmsg = itemq.lastError().text();

  return result.join("\n");
}

QString ExportHelper::generateDelimited(QString qtext, ParameterList &params, QString &errmsg)
{
  if (DEBUG)
    qDebug("ExportHelper::generateDelimited(%s..., %d params, errmsg) entered",
           qPrintable(qtext.left(80)), params.size());
  if (qtext.isEmpty())
    return QString::null;

  if (DEBUG)
  {
    QStringList plist;
    for (int i = 0; i < params.size(); i++)
      plist.append("\t" + params.name(i) + ":\t" + params.value(i).toString());
    qDebug("generateDelimited parameters:\n%s", qPrintable(plist.join("\n")));
  }

  bool valid;
  QString delim = params.value("delim", &valid).toString();
  if (! valid)
    delim = ",";
  if (DEBUG)
    qDebug("generateDelimited(qtest, params, errmsg) delim = %s, valid = %d",
           qPrintable(delim), valid);

  QVariant includeheaderVar = params.value("includeHeaderLine", &valid);
  bool includeheader = (valid ? includeheaderVar.toBool() : false);
  if (DEBUG)
    qDebug("generateDelimited(qtest, params, errmsg) includeheader = %d, valid = %d",
           includeheader, valid);

  QStringList line;
  MetaSQLQuery mql(qtext);
  XSqlQuery qry = mql.toQuery(params);
  if (qry.first())
  {
    QStringList field;
    int cols = qry.record().count();
    if (includeheader)
    {
      for (int p = 0; p < cols; p++)
        field.append(qry.record().fieldName(p));
      line.append(field.join(delim));
    }

    QString tmp;
    do {
      field.clear();
      for (int p = 0; p < cols; p++)
      {
        tmp = qry.value(p).toString();
        if (tmp.contains(delim))
        {
          tmp.replace("\"", "\"\"");
          tmp = "\"" + tmp + "\"";
        }
        field.append(tmp);
      }
      line.append(field.join(delim));
    } while (qry.next());
  }
  if (qry.lastError().type() != QSqlError::NoError)
    errmsg = qry.lastError().text();

  return line.join("\n");
}

QString ExportHelper::generateHTML(const int qryheadid, ParameterList &params, QString &errmsg)
{
  if (DEBUG)
    qDebug("ExportHelper::generateHTML(%d, %d params, errmsg) entered",
           qryheadid, params.size());
  QTextDocument doc(0);
  QTextCursor   cursor(&doc);

  XSqlQuery itemq;
  itemq.prepare("SELECT * FROM qryitem WHERE qryitem_qryhead_id=:id ORDER BY qryitem_order;");
  itemq.bindValue(":id", qryheadid);
  itemq.exec();
  while (itemq.next())
  {
    QString qtext;
    if (itemq.value("qryitem_src").toString() == "REL")
    {
      QString schemaName = itemq.value("qryitem_group").toString();
      qtext = "SELECT * FROM " +
              (schemaName.isEmpty() ? QString("") : schemaName + QString(".")) +
              itemq.value("qryitem_detail").toString();
    }
    else if (itemq.value("qryitem_src").toString() == "MQL")
    {
      QString tmpmsg;
      bool valid;
      qtext = MQLUtil::mqlLoad(itemq.value("qryitem_group").toString(),
                               itemq.value("qryitem_detail").toString(),
                               tmpmsg, &valid);
      if (! valid)
        errmsg = tmpmsg;
    }
    else if (itemq.value("qryitem_src").toString() == "CUSTOM")
      qtext = itemq.value("qryitem_detail").toString();

    if (! qtext.isEmpty())
      cursor.insertHtml(generateHTML(qtext, params, errmsg));
  }
  if (itemq.lastError().type() != QSqlError::NoError)
    errmsg = itemq.lastError().text();

  return doc.toHtml();
}

QString ExportHelper::generateHTML(QString qtext, ParameterList &params, QString &errmsg)
{
  if (DEBUG)
    qDebug("ExportHelper::generateHTML(%s..., %d params, errmsg) entered",
           qPrintable(qtext.left(80)), params.size());
  if (qtext.isEmpty())
    return QString::null;

  QTextDocument    doc(0);
  QTextCursor      cursor(&doc);
  QTextTableFormat tablefmt;

  bool valid;
  QVariant includeheaderVar = params.value("includeHeaderLine", &valid);
  bool includeheader = (valid ? includeheaderVar.toBool() : false);
  if (DEBUG)
    qDebug("generateHTML(qtest, params, errmsg) includeheader = %d, valid = %d",
           includeheader, valid);

  MetaSQLQuery mql(qtext);
  XSqlQuery qry = mql.toQuery(params);
  if (qry.first())
  {
    int cols = qry.record().count();
    int expected = qry.size();
    if (includeheader)
      expected++;

    // presize the table
    cursor.insertTable((expected < 0 ? 1 : expected), cols, tablefmt);
    if (includeheader)
    {
      tablefmt.setHeaderRowCount(1);
      for (int p = 0; p < cols; p++)
      {
        cursor.insertText(qry.record().fieldName(p));
        cursor.movePosition(QTextCursor::NextCell);
      }
    }

    do {
      for (int i = 0; i < cols; i++)
      {
        cursor.insertText(qry.value(i).toString());
        cursor.movePosition(QTextCursor::NextCell);
      }
    } while (qry.next());
  }
  if (qry.lastError().type() != QSqlError::NoError)
    errmsg = qry.lastError().text();

  return doc.toHtml();
}

QString ExportHelper::generateXML(const int qryheadid, ParameterList &params, QString &errmsg, int xsltmapid)
{
  if (DEBUG)
    qDebug("ExportHelper::generateXML(%d, %d params, errmsg, %d) entered",
           qryheadid, params.size(), xsltmapid);
  if (DEBUG)
  {
    QStringList plist;
    for (int i = 0; i < params.size(); i++)
      plist.append("\t" + params.name(i) + ":\t" + params.value(i).toString());
    qDebug("generateXML parameters:\n%s", qPrintable(plist.join("\n")));
  }

  QDomDocument xmldoc("xtupleimport");
  QDomElement rootelem = xmldoc.createElement("xtupleimport");
  xmldoc.appendChild(rootelem);

  XSqlQuery itemq;
  QString tableElemName;
  QString schemaName;
  itemq.prepare("SELECT * FROM qryitem WHERE qryitem_qryhead_id=:id ORDER BY qryitem_order;");
  itemq.bindValue(":id", qryheadid);
  itemq.exec();
  while (itemq.next())
  {
    QString qtext;
    tableElemName = itemq.value("qryitem_name").toString();
    if (itemq.value("qryitem_src").toString() == "REL")
    {
      schemaName = itemq.value("qryitem_group").toString();
      qtext = "SELECT * FROM " +
              (schemaName.isEmpty() ? QString("") : schemaName + QString(".")) +
              itemq.value("qryitem_detail").toString();
    }
    else if (itemq.value("qryitem_src").toString() == "MQL")
    {
      QString tmpmsg;
      bool valid;
      qtext = MQLUtil::mqlLoad(itemq.value("qryitem_group").toString(),
                               itemq.value("qryitem_detail").toString(),
                               tmpmsg, &valid);
      if (! valid)
        errmsg = tmpmsg;
    }
    else if (itemq.value("qryitem_src").toString() == "CUSTOM")
      qtext = itemq.value("qryitem_detail").toString();

    if (! qtext.isEmpty())
    {
      MetaSQLQuery mql(qtext);
      XSqlQuery qry = mql.toQuery(params);
      if (qry.first())
      {
        do {
          QDomElement tableElem = xmldoc.createElement(tableElemName);

          if (DEBUG)
            qDebug("exportXML starting %s", qPrintable(tableElemName));
          if (! schemaName.isEmpty())
            tableElem.setAttribute("schema", schemaName);
          for (int i = 0; i < qry.record().count(); i++)
          {
            QDomElement fieldElem = xmldoc.createElement(qry.record().fieldName(i));
            if (qry.record().value(i).isNull())
              fieldElem.appendChild(xmldoc.createTextNode("[NULL]"));
            else
              fieldElem.appendChild(xmldoc.createTextNode(qry.record().value(i).toString()));
            tableElem.appendChild(fieldElem);
          }
          rootelem.appendChild(tableElem);
        } while (qry.next());
      }
      if (qry.lastError().type() != QSqlError::NoError)
        errmsg = qry.lastError().text();
    }
  }
  if (itemq.lastError().type() != QSqlError::NoError)
    errmsg = itemq.lastError().text();

  if (xsltmapid < 0)
    return xmldoc.toString();
  else
    return XSLTConvertString(xmldoc.toString(), xsltmapid, errmsg);
}

QString ExportHelper::generateXML(QString qtext, QString tableElemName, ParameterList &params, QString &errmsg, int xsltmapid)
{
  if (DEBUG)
    qDebug("ExportHelper::generateXML(%s..., %s, %d params, errmsg, %d) entered",
           qPrintable(qtext.left(80)), qPrintable(tableElemName),
           params.size(), xsltmapid);
  if (DEBUG)
  {
    QStringList plist;
    for (int i = 0; i < params.size(); i++)
      plist.append("\t" + params.name(i) + ":\t" + params.value(i).toString());
    qDebug("generateXML parameters:\n%s", qPrintable(plist.join("\n")));
  }

  QDomDocument xmldoc("xtupleimport");
  QDomElement rootelem = xmldoc.createElement("xtupleimport");
  xmldoc.appendChild(rootelem);

  if (! qtext.isEmpty())
  {
    MetaSQLQuery mql(qtext);
    XSqlQuery qry = mql.toQuery(params);
    if (qry.first())
    {
      do {
        QDomElement tableElem = xmldoc.createElement(tableElemName);

        if (DEBUG)
          qDebug("generateXML starting %s", qPrintable(tableElemName));
        for (int i = 0; i < qry.record().count(); i++)
        {
          QDomElement fieldElem = xmldoc.createElement(qry.record().fieldName(i));
          if (qry.record().value(i).isNull())
            fieldElem.appendChild(xmldoc.createTextNode("[NULL]"));
          else
            fieldElem.appendChild(xmldoc.createTextNode(qry.record().value(i).toString()));
          tableElem.appendChild(fieldElem);
        }
        rootelem.appendChild(tableElem);
      } while (qry.next());
    }
    if (qry.lastError().type() != QSqlError::NoError)
      errmsg = qry.lastError().text();
  }

  if (xsltmapid < 0)
    return xmldoc.toString();
  else
    return XSLTConvertString(xmldoc.toString(), xsltmapid, errmsg);
}

bool ExportHelper::XSLTConvertFile(QString inputfilename, QString outputfilename, int xsltmapid, QString &errmsg)
{
  if (DEBUG)
    qDebug("ExportHelper::XSLTConvertFile(%s, %s, %d, errmsg) entered",
           qPrintable(inputfilename), qPrintable(outputfilename), xsltmapid);
  bool returnVal = false;

  XSqlQuery xsltq;
  xsltq.prepare("SELECT xsltmap_export"
                "  FROM xsltmap"
                " WHERE xsltmap_id=:id;");

  xsltq.bindValue(":id", xsltmapid);
  xsltq.exec();
  if (xsltq.first())
    return XSLTConvertFile(inputfilename, outputfilename,
                           xsltq.value("xsltmap_export").toString(), errmsg);
  else  if (xsltq.lastError().type() != QSqlError::NoError)
    errmsg = xsltq.lastError().text();
  else
    errmsg = tr("Could not find XSLT mapping with internal id %1.")
               .arg(xsltmapid);

  return returnVal;
}

bool ExportHelper::XSLTConvertFile(QString inputfilename, QString outputfilename, QString xsltfilename, QString &errmsg)
{
  QString xsltdir;
  QString xsltcmd;

  XSqlQuery q;
  q.prepare("SELECT fetchMetricText(:xsltdir) AS dir,"
            "       fetchMetricText(:xsltcmd) AS cmd;");
#if defined Q_WS_MACX
  q.bindValue(":xsltdir", "XSLTDefaultDirMac");
  q.bindValue(":xsltcmd", "XSLTProcessorMac");
#elif defined Q_WS_WIN
  q.bindValue(":xsltdir", "XSLTDefaultDirWindows");
  q.bindValue(":xsltcmd", "XSLTProcessorWindows");
#elif defined Q_WS_X11
  q.bindValue(":xsltdir", "XSLTDefaultDirLinux");
  q.bindValue(":xsltcmd", "XSLTProcessorLinux");
#endif
  q.exec();
  if (q.first())
  {
    xsltdir = q.value("dir").toString();
    xsltcmd = q.value("cmd").toString();
  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    errmsg = q.lastError().text();
    return false;
  }
  else
  {
    errmsg = tr("Could not find the XSLT directory and command metrics.");
    return false;
  }

  QStringList args = xsltcmd.split(" ", QString::SkipEmptyParts);
  QString command = args[0];
  args.removeFirst();
  args.replaceInStrings("%f", inputfilename);

  if (QFile::exists(xsltfilename))
    args.replaceInStrings("%x", xsltfilename);
  else if (QFile::exists(xsltdir + QDir::separator() + xsltfilename))
    args.replaceInStrings("%x", xsltdir + QDir::separator() + xsltfilename);
  else
  {
    errmsg = tr("Cannot find the XSLT file as either %1 or %2")
                .arg(xsltfilename, xsltdir + QDir::separator() + xsltfilename);
    return false;
  }

  QProcess xslt;
  xslt.setStandardOutputFile(outputfilename);
  xslt.start(command, args);
  QString commandline = command + " " + args.join(" ");
  errmsg = "";
  /* TODO: make the file-processing asynchronous
     this will keep the UI snappy and handle spurious errors
     like the occasional waitForFinished failure if the processing
     runs faster than expected.
   */
  if (! xslt.waitForStarted())
    errmsg = tr("Error starting XSLT Processing: %1\n%2")
                      .arg(commandline)
                      .arg(QString(xslt.readAllStandardError()));
  if (! xslt.waitForFinished())
    errmsg = tr("The XSLT Processor encountered an error: %1\n%2")
                      .arg(commandline)
                      .arg(QString(xslt.readAllStandardError()));
  if (xslt.exitStatus() !=  QProcess::NormalExit)
    errmsg = tr("The XSLT Processor did not exit normally: %1\n%2")
                      .arg(commandline)
                      .arg(QString(xslt.readAllStandardError()));
  if (xslt.exitCode() != 0)
    errmsg = tr("The XSLT Processor returned an error code: %1\nreturned %2\n%3")
                      .arg(commandline)
                      .arg(xslt.exitCode())
                      .arg(QString(xslt.readAllStandardError()));

  return errmsg.isEmpty();
}

QString ExportHelper::XSLTConvertString(QString input, int xsltmapid, QString &errmsg)
{
  if (DEBUG)
    qDebug("ExportHelper::XSLTConvertString(%s..., %d, errmsg) entered",
           qPrintable(input.left(200)), xsltmapid);
  QString returnVal;

  XSqlQuery xsltq;
  xsltq.prepare("SELECT xsltmap_name, xsltmap_export"
                "  FROM xsltmap"
                " WHERE xsltmap_id=:id;");

  xsltq.bindValue(":id", xsltmapid);
  xsltq.exec();
  if (xsltq.first())
  {
    /* tempfile handling is messy because windows doesn't handle them as you
       might expect.
       TODO: find a simpler way
     */
    QString xsltmap = xsltq.value("xsltmap_name").toString();
    QTemporaryFile *inputfile = new QTemporaryFile(QDir::tempPath()
                                                   + QDir::separator()
                                                   + xsltmap
                                                   + "Input.XXXXXX.xml");
    inputfile->setAutoRemove(false);
    if (! inputfile->open())
      errmsg = tr("Could not open temporary input file (%1).")
                  .arg(inputfile->error());
    else
    {
      QString inputfileName = inputfile->fileName();
      inputfile->write(input.toUtf8());
      inputfile->close();
      delete inputfile;
      inputfile = 0;

      QTemporaryFile *outputfile = new QTemporaryFile(QDir::tempPath()
                                                      + QDir::separator()
                                                      + xsltmap
                                                      + "Output.XXXXXX.xml");
      outputfile->setAutoRemove(false);
      if (! outputfile->open())
        errmsg = tr("Could not open temporary output file (%1).")
                  .arg(outputfile->error());
      else
      {
        QString outputfileName = outputfile->fileName();

        if (DEBUG)
          qDebug("ExportHelper::XSLTConvertString writing from %s to %s",
                 qPrintable(inputfileName), qPrintable(outputfileName));

        if (XSLTConvertFile(inputfileName, outputfileName,
                            xsltq.value("xsltmap_export").toString(), errmsg))
          returnVal = outputfile->readAll();

        outputfile->close();
        delete outputfile;
        outputfile = 0;

        if (errmsg.isEmpty())
        {
          QFile::remove(outputfileName);
          QFile::remove(inputfileName);
        }
      }
    }
  }
  else  if (xsltq.lastError().type() != QSqlError::NoError)
    errmsg = xsltq.lastError().text();
  else
    errmsg = tr("Could not find XSLT mapping with internal id %1.")
               .arg(xsltmapid);

  if (! errmsg.isEmpty())
    qWarning("%s", qPrintable(errmsg));
  return returnVal;
}

// scripting exposure //////////////////////////////////////////////////////////

Q_DECLARE_METATYPE(ParameterList)

static QScriptValue exportHTML(QScriptContext *context,
                               QScriptEngine  * /*engine*/)
{
  if (context->argumentCount() < 1)
    context->throwError(QScriptContext::UnknownError,
                        "not enough args passed to exportHTML");

  int           qryheadid = context->argument(0).toInt32();
  ParameterList params;
  QString       filename;
  QString       errmsg;

  if (context->argumentCount() >= 2)
    params = qscriptvalue_cast<ParameterList>(context->argument(1));
  if (context->argumentCount() >= 3)
    filename = context->argument(2).toString();
  if (context->argumentCount() >= 4)
    errmsg = context->argument(3).toString();

  if (DEBUG)
    qDebug("exportHTML(%d, %d params, %s, %s) called with %d args",
           qryheadid, params.size(), qPrintable(filename),
           qPrintable(errmsg), context->argumentCount());

  bool result = ExportHelper::exportHTML(qryheadid, params, filename, errmsg);
  // TODO: how to we pass back filename and errmsg output parameters?

  return QScriptValue(result);
}

static QScriptValue exportXML(QScriptContext *context,
                              QScriptEngine  * /*engine*/)
{
  if (context->argumentCount() < 1)
    context->throwError(QScriptContext::UnknownError,
                        "not enough args passed to exportXML");

  int           qryheadid = context->argument(0).toInt32();
  ParameterList params;
  QString       filename;
  QString       errmsg;
  int           xsltmapid = -1;

  if (context->argumentCount() >= 2)
    params = qscriptvalue_cast<ParameterList>(context->argument(1));
  if (context->argumentCount() >= 3)
    filename = context->argument(2).toString();
  if (context->argumentCount() >= 4)
    errmsg = context->argument(3).toString();
  if (context->argumentCount() >= 5)
    xsltmapid = context->argument(4).toInt32();

  if (DEBUG)
    qDebug("exportXML(%d, %d params, %s, %s, %d) called with %d args",
           qryheadid, params.size(), qPrintable(filename),
           qPrintable(errmsg), xsltmapid, context->argumentCount());

  bool result = ExportHelper::exportXML(qryheadid, params, filename,
                                        errmsg, xsltmapid);
  // TODO: how to we pass back filename and errmsg output parameters?

  return QScriptValue(result);
}

static QScriptValue generateDelimited(QScriptContext *context,
                                      QScriptEngine  * /*engine*/)
{
  QString result = QString::null;
  QString errmsg = QString::null;

  if (context->argumentCount() < 2)
    context->throwError(QScriptContext::UnknownError,
                        "not enough args passed to generateHTML");

  if (context->argument(0).isNumber())
  {
    int        qryheadid = context->argument(0).toInt32();
    ParameterList params = qscriptvalue_cast<ParameterList>(context->argument(1));

    result = ExportHelper::generateDelimited(qryheadid, params, errmsg);
  }
  else
  {
    QString         qtext = context->argument(0).toString();
    ParameterList  params = qscriptvalue_cast<ParameterList>(context->argument(1));
    result = ExportHelper::generateDelimited(qtext, params, errmsg);
  }

  // TODO: how to we pass back errmsg output parameter?

  return QScriptValue(result);
}

static QScriptValue generateHTML(QScriptContext *context,
                                 QScriptEngine  * /*engine*/)
{
  QString result = QString::null;
  QString errmsg = QString::null;

  if (context->argumentCount() < 2)
    context->throwError(QScriptContext::UnknownError,
                        "not enough args passed to generateHTML");

  if (context->argument(0).isNumber())
  {
    int        qryheadid = context->argument(0).toInt32();
    ParameterList params = qscriptvalue_cast<ParameterList>(context->argument(1));

    result = ExportHelper::generateHTML(qryheadid, params, errmsg);
  }
  else
  {
    QString         qtext = context->argument(0).toString();
    ParameterList  params = qscriptvalue_cast<ParameterList>(context->argument(1));
    result = ExportHelper::generateHTML(qtext, params, errmsg);
  }

  // TODO: how to we pass back errmsg output parameter?

  return QScriptValue(result);
}

static QScriptValue generateXML(QScriptContext *context,
                                QScriptEngine  * /*engine*/)
{
  QString result = QString::null;
  QString errmsg = QString::null;

    
  if (context->argument(0).isNumber() && context->argumentCount() == 3)
  {
    int        qryheadid = context->argument(0).toInt32();
    ParameterList params = qscriptvalue_cast<ParameterList>(context->argument(1));

    result = ExportHelper::generateXML(qryheadid, params, errmsg);
  }
  else if (context->argument(0).isNumber() && context->argumentCount() == 4)
  {
    int        qryheadid = context->argument(0).toInt32();
    ParameterList params = qscriptvalue_cast<ParameterList>(context->argument(1));
    int        xsltmapid = context->argument(3).toInt32();

    result = ExportHelper::generateXML(qryheadid, params, errmsg, xsltmapid);
  }
  else if (context->argumentCount() >= 3)
  {
    QString         qtext = context->argument(0).toString();
    QString tableElemName = context->argument(1).toString();
    ParameterList  params = qscriptvalue_cast<ParameterList>(context->argument(2));
    if (context->argumentCount() >= 5)
    {
      int xsltmapid = context->argument(4).toInt32();
      result = ExportHelper::generateXML(qtext, tableElemName, params, errmsg, xsltmapid);
    }
    else
      result = ExportHelper::generateXML(qtext, tableElemName, params, errmsg);
  }
  else
    context->throwError(QScriptContext::UnknownError,
                        "Don't know which version of generateXML to call");

  // TODO: how to we pass back errmsg output parameter?

  return QScriptValue(result);
}

static QScriptValue XSLTConvertFile(QScriptContext *context,
                                    QScriptEngine  * /*engine*/)
{
  QString inputfilename  = context->argument(0).toString();
  QString outputfilename = context->argument(1).toString();
  QString errmsg;

  bool result = false;

  if (context->argument(2).isNumber())
    result = ExportHelper::XSLTConvertFile(inputfilename, outputfilename,
                                           context->argument(2).toInt32(),
                                           errmsg);
  else
    result = ExportHelper::XSLTConvertFile(inputfilename, outputfilename,
                                           context->argument(2).toString(),
                                           errmsg);


  // TODO: how to we pass back errmsg output parameter?

  return QScriptValue(result);
}

static QScriptValue XSLTConvertString(QScriptContext *context,
                                      QScriptEngine  * /*engine*/)
{
  QString input     = context->argument(0).toString();
  int     xsltmapid = context->argument(1).toInt32();
  QString errmsg;

  QString result = ExportHelper::XSLTConvertString(input, xsltmapid, errmsg);

  // TODO: how to we pass back errmsg output parameter?

  return QScriptValue(result);
}

void setupExportHelper(QScriptEngine *engine)
{
  QScriptValue obj = engine->newObject();
  obj.setProperty("exportHTML", engine->newFunction(exportHTML),    QScriptValue::ReadOnly | QScriptValue::Undeletable);
  obj.setProperty("exportXML", engine->newFunction(exportXML),      QScriptValue::ReadOnly | QScriptValue::Undeletable);
  obj.setProperty("generateDelimited", engine->newFunction(generateDelimited),QScriptValue::ReadOnly | QScriptValue::Undeletable);
  obj.setProperty("generateHTML", engine->newFunction(generateHTML),QScriptValue::ReadOnly | QScriptValue::Undeletable);
  obj.setProperty("generateXML", engine->newFunction(generateXML),  QScriptValue::ReadOnly | QScriptValue::Undeletable);
  obj.setProperty("XSLTConvertFile", engine->newFunction(XSLTConvertFile), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  obj.setProperty("XSLTConvertString", engine->newFunction(XSLTConvertString), QScriptValue::ReadOnly | QScriptValue::Undeletable);

  engine->globalObject().setProperty("ExportHelper", obj, QScriptValue::ReadOnly | QScriptValue::Undeletable);
}
