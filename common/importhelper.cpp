/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "importhelper.h"

#include <QApplication>
#include <QDate>
#include <QDateTime>
#include <QDirIterator>
#include <QFile>
#include <QMessageBox>
#include <QPluginLoader>
#include <QProcess>
#include <QScriptEngine>
#include <QScriptValue>
#include <QSqlError>
#include <QTemporaryFile>
#include <QVariant>

#include <xsqlquery.h>

#include "exporthelper.h"

#define MAXCSVFIRSTLINE     2048
#define DEFAULT_SAVE_DIR    "done"
#define DEFAULT_ERR_DIR     "error"
#define DEFAULT_SAVE_SUFFIX ".done"
#define DEFAULT_ERR_SUFFIX  ".err"

#define DEBUG false

static QString getUniqueFileName(QString poriginalname)
{
  QString newname = poriginalname;

  if (QFile::exists(newname))
    newname = newname + QDate::currentDate().toString(".yyyy.MM.dd");
  if (QFile::exists(newname))
    newname = newname + QDateTime::currentDateTime().toString(".hh.mm");
  if (QFile::exists(newname))
    newname = newname + QDateTime::currentDateTime().toString(".ss");

  for (int i = 0; QFile::exists(newname) ; i++)
    newname = newname + "." + QString::number(i);

  if (DEBUG)
    qDebug("getUniqueFileName returning %s", qPrintable(newname));

  return newname;
}

CSVImpPluginInterface *ImportHelper::_csvimpplugin = 0;

CSVImpPluginInterface *ImportHelper::getCSVImpPlugin(QObject *parent)
{
  if (! _csvimpplugin)
  {
    if (! parent)
      parent = QApplication::instance();

    foreach (QPluginLoader *loader, parent->findChildren<QPluginLoader*>())
    {
      QObject *plugin = loader->instance();
      if (plugin)
      {
        _csvimpplugin = qobject_cast<CSVImpPluginInterface*>(plugin);
        if (_csvimpplugin)
        {
          XSqlQuery defq;
          defq.prepare("SELECT fetchMetricText(:datadir) AS datadir,"
                       "       fetchMetricText(:atlasdir) AS atlasdir;");
#if defined Q_WS_MACX
          defq.bindValue(":datadir",  "XMLDefaultDirMac");
          defq.bindValue(":atlasdir", "CSVAtlasDefaultDirMac");
#elif defined Q_WS_WIN
          defq.bindValue(":datadir",  "XMLDefaultDirWindows");
          defq.bindValue(":atlasdir", "CSVAtlasDefaultDirWindows");
#elif defined Q_WS_X11
          defq.bindValue(":datadir",  "XMLDefaultDirLinux");
          defq.bindValue(":atlasdir", "CSVAtlasDefaultDirLinux");
#endif
          defq.exec();
          if (defq.first())
          {
            _csvimpplugin->setCSVDir(defq.value("datadir").toString());
            _csvimpplugin->setAtlasDir(defq.value("atlasdir").toString());
          }
          else if (defq.lastError().type() != QSqlError::NoError)
            qWarning("%s", qPrintable(defq.lastError().text()));

          break; // out of foreach
        }
      }
    }
  }

  if (DEBUG)
    qDebug("ImportHelper::getCSVImpPlugin(%p) returning %p",
           parent, _csvimpplugin);
  return _csvimpplugin;
}

/** \brief Obey the import file configuration and remove or rename the file
           that has just been imported.

    \param[in]  pfilename The name of the file that has just been imported.
    \param[in]  success   An indication of whether the import succeeded or not.
                          This controls whether the import file pfilename will
                          be handled using the configuration for successful
                          imports or failed imports.
    \param[out] errmsg    Any error message generated during file handling.
    \param[in]  saveToErrorFile If this is not an empty string, the string is
                          saved to an error file using the configuration for
                          handling error files.
    \return true if the file was handled successfully, false if there was an
                 error moving or deleting the file.
  */
bool ImportHelper::handleFilePostImport(const QString &pfilename, bool success, QString &errmsg, const QString &saveToErrorFile)
{
  if (DEBUG)
    qDebug("handleFilePostImport(%s, %d, errmsg, %s)",
           qPrintable(pfilename), success, qPrintable(saveToErrorFile));

  bool returnValue = false;

  QString xmldir;
  QString destdir;
  QString suffix;
  QString filetreatment;
  QString errfiledir;
  QString errfilesuffix;
  QString errtreatment;
  XSqlQuery q;

  q.prepare("SELECT fetchMetricText(:xmldir)               AS xmldir,"
            "       fetchMetricText('XMLSuccessDir')       AS successdir,"
            "       fetchMetricText('XMLSuccessSuffix')    AS successsuffix,"
            "       fetchMetricText('XMLSuccessTreatment') AS successtreatment,"
            "       fetchMetricText('ImportFailureDir')       AS failuredir,"
            "       fetchMetricText('ImportFailureSuffix')    AS failuresuffix,"
            "       fetchMetricText('ImportFailureTreatment') AS failuretreatment;");
#if defined Q_WS_MACX
  q.bindValue(":xmldir",  "XMLDefaultDirMac");
#elif defined Q_WS_WIN
  q.bindValue(":xmldir",  "XMLDefaultDirWindows");
#elif defined Q_WS_X11
  q.bindValue(":xmldir",  "XMLDefaultDirLinux");
#endif
  q.exec();
  if (q.first())
  {
    xmldir = q.value("xmldir").toString();
    if (success)
    {
      destdir       = q.value("successdir").toString();
      suffix        = q.value("successsuffix").toString();
      filetreatment = q.value("successtreatment").toString();
    }
    else
    {
      destdir       = q.value("failuredir").toString();
      suffix        = q.value("failuresuffix").toString();
      filetreatment = q.value("failuretreatment").toString();
    }
    errfiledir      = q.value("failuredir").toString();
    errfilesuffix   = q.value("failuresuffix").toString();
    errtreatment    = q.value("failuretreatment").toString();
  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    errmsg = q.lastError().text();
    return false;
  }
  else
  {
    errmsg = tr("Could not find the metrics for handling import results.");
    return false;
  }

  if (xmldir.isEmpty())
    xmldir = ".";

  if (destdir.isEmpty())
    destdir = success ? DEFAULT_SAVE_DIR : DEFAULT_ERR_DIR;
  if (suffix.isEmpty())
    suffix = success ? DEFAULT_SAVE_SUFFIX : DEFAULT_ERR_SUFFIX;

  if (errfiledir.isEmpty())
    errfiledir = DEFAULT_ERR_DIR;
  if (errfilesuffix.isEmpty())
    errfilesuffix = DEFAULT_ERR_SUFFIX;

  QFile file(pfilename);

  // handle error file first. if it fails, don't delete the primary import file
  if (! saveToErrorFile.isEmpty() &&
      errtreatment != "Delete" && errtreatment != "None")
  {
    if (errtreatment == "Move")
      errfilesuffix  = "";
    else if (errtreatment == "Rename")
      errfiledir = ".";
    /*
    else if (errtreatment == "Delete")
      ; // should never reach here
    else if (errtreatment == "None")
      ; // should never reach here
    */

    if (QDir::isRelativePath(errfiledir))
      errfiledir = xmldir + QDir::separator() + errfiledir;

    QString errname = errfiledir + QDir::separator() +
                      QFileInfo(file).fileName() + errfilesuffix;
    errname = getUniqueFileName(errname);

    if (DEBUG)
      qDebug("handleFilePostImport about to try saving err file %s",
             qPrintable(errname));

    QFile *errorfile = new QFile(errname);
    if (! errorfile->open(QIODevice::ReadWrite | QIODevice::Truncate | QIODevice::Text) ||
        ! errorfile->write(saveToErrorFile.toUtf8()) >= saveToErrorFile.length())
    {
      // don't delete the original import file if we couldn't save the error file
      errmsg += (errmsg.isEmpty() ? "" : "\n") + 
               tr("<p>Could not write error file %1 after processing %2 (%3). %4")
                        .arg(errname, pfilename, errorfile->errorString(),
                             (filetreatment == "Delete" ?
                              tr("Trying to save backup copy.") : QString()));
      if (filetreatment == "Delete")
      {
        filetreatment == "Rename";
        suffix = DEFAULT_ERR_SUFFIX;
      }
    }
    errorfile->close();
  }

  if (filetreatment == "Delete")
  {
    if (file.remove())
      returnValue = true;
    else
      errmsg = tr("Could not remove %1 after successful processing (%2).")
                        .arg(pfilename, file.error());
  }
  else if (filetreatment == "Rename")
  {
    QString newname = pfilename + suffix;
    newname = getUniqueFileName(newname);
    if (file.rename(newname))
      returnValue = true;
    else
      errmsg = tr("Could not rename %1 to %2 after successful processing (%3).")
                        .arg(pfilename, newname).arg(file.error());
  }
  else if (filetreatment == "Move")
  {
    if (QDir::isRelativePath(destdir))
      destdir = xmldir + QDir::separator() + destdir;

    QDir donedir(destdir);
    if (! donedir.exists())
      donedir.mkpath(destdir);

    QString newname = destdir + QDir::separator() + QFileInfo(file).fileName(); 
    newname = getUniqueFileName(newname);
    if (file.rename(newname))
      returnValue = true;
    else
      errmsg = tr("<p>Could not move %1 to %2 after successful processing (%3).")
                        .arg(pfilename, newname).arg(file.error());
  }

  else if (filetreatment == "None")
    returnValue = true;

  else
    errmsg = tr("<p>Don't know what to do %1 after import "
                "so leaving it where it was.").arg(pfilename);

  if (DEBUG)
    qDebug("ImportHelper::handleFilePostImport returning %d", returnValue);

  return returnValue;
}

bool ImportHelper::importCSV(const QString &pFileName, QString &errmsg)
{
  errmsg = QString::null;

  QFile file(pFileName);
  if (! file.open(QIODevice::ReadOnly))
  {
    errmsg = tr("Could not open %1: %2").arg(pFileName, file.errorString());
    return false;
  }

  QString firstline(file.readLine(MAXCSVFIRSTLINE));
  file.close();
  if (firstline.isEmpty())
  {
    errmsg = tr("Could not read the first line from %1").arg(pFileName);
    return false;
  }

  XSqlQuery mapq;
  mapq.prepare("SELECT atlasmap_name, atlasmap_atlas,"
               "       atlasmap_map, atlasmap_headerline,"
               "       CASE WHEN (:filename  ~ atlasmap_filter) THEN 0"
               "            WHEN (:firstline ~ atlasmap_filter) THEN 1"
               "       END AS seq"
               "  FROM atlasmap"
               " WHERE ((:filename ~ atlasmap_filter AND atlasmap_filtertype='filename')"
               "     OR (:firstline ~ atlasmap_filter AND atlasmap_filtertype='firstline'))"
               " ORDER BY seq LIMIT 1;");
  mapq.bindValue(":filename",  pFileName);
  mapq.bindValue(":firstline", firstline);
  mapq.exec();
  if (mapq.first())
  {
    QString atlasfile = mapq.value("atlasmap_atlas").toString();
    QString map       = mapq.value("atlasmap_map").toString();

    CSVImpPluginInterface *csvplugin = getCSVImpPlugin();
    if (csvplugin)
    {
      if (! csvplugin->openCSV(pFileName))
        errmsg = tr("Could not open CSV File %1").arg(pFileName);
      else if (! csvplugin->openAtlas(atlasfile))
        errmsg = tr("Could not open Atlas %1").arg(atlasfile);
      else if (! csvplugin->setAtlasMap(map))
        errmsg = tr("Could not set Map to %1").arg(map);
      else if (! csvplugin->setFirstLineHeader(mapq.value("atlasmap_headerline").toBool()))
        errmsg = tr("Could not set first line status");
      else if (! csvplugin->importCSV())
        errmsg = tr("Could not import the CSV data from %1").arg(pFileName);

      errmsg += (errmsg.isEmpty() ? "" : "\n") + csvplugin->lastError();
    }
  }
  else if (mapq.lastError().type() != QSqlError::NoError)
    errmsg = mapq.lastError().text();
  else
    errmsg = tr("Could not find a Map or Atlas for %1").arg(pFileName);

  if (! handleFilePostImport(pFileName, errmsg.isEmpty(), errmsg))
    return false;

  return errmsg.isEmpty();
}

bool ImportHelper::importXML(const QString &pFileName, QString &errmsg, QString &warnmsg)
{
  if (DEBUG)
    qDebug("ImportHelper::importXML(%s, errmsg)", qPrintable(pFileName));

  QString xmldir;
  QString xsltdir;
  QString xsltcmd;
  QStringList errors;
  QStringList warnings;
  bool        saveErrorXML = false;

  XSqlQuery q;
  q.prepare("SELECT fetchMetricText(:xmldir)  AS xmldir,"
            "       fetchMetricText(:xsltdir) AS xsltdir,"
            "       fetchMetricText(:xsltcmd) AS xsltcmd,"
            "       fetchMetricBool('ImportXMLCreateErrorFile') AS createerr;");
#if defined Q_WS_MACX
  q.bindValue(":xmldir",  "XMLDefaultDirMac");
  q.bindValue(":xsltdir", "XSLTDefaultDirMac");
  q.bindValue(":xsltcmd", "XSLTProcessorMac");
#elif defined Q_WS_WIN
  q.bindValue(":xmldir",  "XMLDefaultDirWindows");
  q.bindValue(":xsltdir", "XSLTDefaultDirWindows");
  q.bindValue(":xsltcmd", "XSLTProcessorWindows");
#elif defined Q_WS_X11
  q.bindValue(":xmldir",  "XMLDefaultDirLinux");
  q.bindValue(":xsltdir", "XSLTDefaultDirLinux");
  q.bindValue(":xsltcmd", "XSLTProcessorLinux");
#endif
  q.exec();
  if (q.first())
  {
    xmldir  = q.value("xmldir").toString();
    xsltdir = q.value("xsltdir").toString();
    xsltcmd = q.value("xsltcmd").toString();
    saveErrorXML = q.value("createerr").toBool();
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

  if (xmldir.isEmpty())
    xmldir = ".";

  QDomDocument doc(pFileName);
  if (!openDomDocument(pFileName, doc, errmsg))
    return false;

  QString doctype = doc.doctype().name();
  if (DEBUG) qDebug("initial doctype = %s", qPrintable(doctype));
  if (doctype.isEmpty())
  {
    doctype = doc.documentElement().tagName();
    if (DEBUG) qDebug("changed doctype to %s", qPrintable(doctype));
  }

  QString tmpfileName;
  if (doctype != "xtupleimport")
  {
    QString xsltfile;
    XSqlQuery q;
    q.prepare("SELECT xsltmap_import FROM xsltmap "
              "WHERE ((xsltmap_doctype=:doctype OR xsltmap_doctype='')"
              "   AND (xsltmap_system=:system   OR xsltmap_system=''));");
    q.bindValue(":doctype", doctype);
    q.bindValue(":system",  doc.doctype().systemId());
    q.exec();
    if (q.first())
      xsltfile = q.value("xsltmap_import").toString();
    else if (q.lastError().type() != QSqlError::NoError)
    {
      errmsg = q.lastError().databaseText();
      return false;
    }
    else
    {
      errmsg = tr("<p>Could not find a map for doctype '%1' and system id '%2'"
                  ". Write an XSLT stylesheet to convert this to valid xtuple "
                  "import XML and add it to the Map of XSLT Import Filters.")
                    .arg(doctype, doc.doctype().systemId());
      return false;
    }

    tmpfileName = xmldir + QDir::separator() + doctype + "TOxtupleimport";

    if (! ExportHelper::XSLTConvertFile(pFileName, tmpfileName,
                                        q.value("xsltmap_import").toString(),
                                        errmsg))
      return false;

    if (! openDomDocument(tmpfileName, doc, errmsg))
      return false;
  }

  /* xtupleimport format is very straightforward:
      top level element is xtupleimport
        second level elements are all table/view names (default to api schema)
          third level elements are all column names
     and there are no text nodes until third level

     wrap the import of an entire file in a single transaction so
     we can reimport files which have failures. however, if a
     view-level element has the ignore attribute set to true then
     rollback just that view-level element if it generates an error.
  */

  // the silent attribute provides the user the option to turn off 
  // the interactive message for the view-level element

  QDomDocument errorDoc;
  QDomElement errorRoot = errorDoc.appendChild(errorDoc.createElement("xtupleimport")).toElement();

  q.exec("BEGIN;");
  if (q.lastError().type() != QSqlError::NoError)
  {
    errmsg = q.lastError().databaseText();
    return false;
  }

  XSqlQuery rollback;
  rollback.prepare("ROLLBACK;");

  QRegExp apos("\\\\*'");

  for (QDomElement viewElem = doc.documentElement().firstChildElement();
       ! viewElem.isNull();
       viewElem = viewElem.nextSiblingElement())
  {
    QStringList columnNameList;
    QStringList columnValueList;

    bool ignoreErr = (viewElem.attribute("ignore", "false").isEmpty() ||
                      viewElem.attribute("ignore", "false") == "true");

    bool silent = (viewElem.attribute("silent", "false").isEmpty() ||
                   viewElem.attribute("silent", "false") == "true");

    QString mode = viewElem.attribute("mode", "insert");
    QStringList keyList;
    if (! viewElem.attribute("key").isEmpty())
      keyList = viewElem.attribute("key").split(QRegExp(",\\s*"));

    QString viewName = viewElem.tagName();
    if (viewName.indexOf(".") > 0)
      ; // viewName contains . so accept that it's schema-qualified
    else if (! viewElem.attribute("schema").isEmpty())
      viewName = viewElem.attribute("schema") + "." + viewName;
    else // backwards compatibility - must be in the api schema
      viewName = "api." + viewName;

    QString savepointName = viewName;
    savepointName.remove(".");
    XSqlQuery rollbacktosavepoint;
    if (ignoreErr || saveErrorXML)
    {
      q.exec("SAVEPOINT " + savepointName + ";");
      rollbacktosavepoint.prepare("ROLLBACK TO SAVEPOINT " + savepointName + ";");
    }
    bool haveSavepoint = (ignoreErr || saveErrorXML);

    if (mode.isEmpty())
      mode = "insert";
    else if (mode == "update" && keyList.isEmpty())
    {
      if (! viewElem.namedItem(viewName + "_number").isNull())
        keyList.append(viewName + "_number");
      else if (! viewElem.namedItem("order_number").isNull())
        keyList.append("order_number");
      else
      {
        if (haveSavepoint)
          rollbacktosavepoint.exec();
        if (ignoreErr || saveErrorXML)
        {
          warnings.append(tr("Cannot process %1 element without a key attribute"));
          if (saveErrorXML)
            errorRoot.appendChild(errorDoc.importNode(viewElem, true));
        }
        else
          errors.append(tr("Cannot process %1 element without a key attribute"));
        continue;       // back to top of viewElem for loop
      }
      if (! viewElem.namedItem("line_number").isNull())
        keyList.append("line_number");
    }

    for (QDomElement columnElem = viewElem.firstChildElement();
         ! columnElem.isNull();
         columnElem = columnElem.nextSiblingElement())
    {
      QString value = columnElem.attribute("value").isEmpty() ?
                              columnElem.text() : columnElem.attribute("value");
      if (DEBUG)
        qDebug("%s before transformation: /%s/",
               qPrintable(columnElem.tagName()), qPrintable(value));

      columnNameList.append(columnElem.tagName());

      if (value.trimmed() == "[NULL]")
        columnValueList.append("NULL");
      else if (value.trimmed().startsWith("SELECT"))
        columnValueList.append("(" + value.trimmed() + ")");
      else if (columnElem.attribute("quote") == "false")
        columnValueList.append(value);
      else
        columnValueList.append("'" + value.replace(apos, "''") + "'");

      if (DEBUG)
        qDebug("%s after transformation: /%s/",
               qPrintable(columnElem.tagName()), qPrintable(value));
    }

    QString sql;
    if (mode == "update")
    {
      QStringList whereList;
      for (int i = 0; i < keyList.size(); i++)
        whereList.append("(" + keyList[i] + "=" +
                         columnValueList[columnNameList.indexOf(keyList[i])] + ")");

      for (int i = 0; i < columnNameList.size(); i++)
        columnNameList[i].append("=" + columnValueList[i]);

      sql = "UPDATE " + viewName + " SET " +
            columnNameList.join(", ") +
            " WHERE (" + whereList.join(" AND ") + ");";
    }
    else if (mode == "insert")
      sql = "INSERT INTO " + viewName + " (" +
            columnNameList.join(", ") +
            " ) SELECT " +
            columnValueList.join(", ") + ";" ;
    else
    {
      if (haveSavepoint)
        rollbacktosavepoint.exec();
      if (! ignoreErr)
        errors.append(tr("Could not process %1: invalid mode %2")
                      .arg(viewElem.tagName(), mode));
      continue;       // back to top of viewElem for loop
    }

    if (DEBUG) qDebug("About to run this: %s", qPrintable(sql));
    q.exec(sql);
    if (q.lastError().type() != QSqlError::NoError)
    {
      if (haveSavepoint)
        rollbacktosavepoint.exec();
      if (ignoreErr)
      {
        if (! silent)
          warnings.append(tr("Ignored error while importing %1:\n%2")
                              .arg(viewElem.tagName(), q.lastError().text()));
      }
      else if (saveErrorXML)
      {
        warnings.append(tr("Error processing %1. Saving to retry later:\t%2")
                              .arg(viewElem.tagName(), q.lastError().text()));
        QDomNode nodecopy = errorDoc.importNode(viewElem, true);
        nodecopy.appendChild(errorDoc.createComment(q.lastError().text()));
        errorRoot.appendChild(nodecopy);
      }
      else
      {
        errors.append(tr("Error importing %1: %2")
                      .arg(pFileName, q.lastError().databaseText()));
        continue;       // back to top of viewElem for loop
      }
    }
    else if (haveSavepoint)
      q.exec("RELEASE SAVEPOINT " + savepointName + ";");
  }

  q.exec("COMMIT;");
  if (q.lastError().type() != QSqlError::NoError)
  {
    rollback.exec();
    errmsg = q.lastError().databaseText();
    return false;
  }

  if (! tmpfileName.isEmpty())
    QFile::remove(tmpfileName);

  if (warnings.size() > 0)
    warnmsg = warnings.join("\n");

  QString fileerrmsg;
  if (! handleFilePostImport(pFileName,
                             errors.size() == 0,
                             fileerrmsg,
                             errorRoot.hasChildNodes() ? errorDoc.toString()
                                                       : QString()))
  {
    errors.append(fileerrmsg);
    return false;
  }

  errmsg = errors.join(tr("\n"));

  return errors.size() == 0;
}

bool ImportHelper::openDomDocument(const QString &pFileName, QDomDocument &pDoc, QString &errmsg)
{
  QFile file(pFileName);
  if (!file.open(QIODevice::ReadOnly))
  {
    errmsg = tr("<p>Could not open file %1 (error %2)")
                      .arg(pFileName, file.error());
    return false;
  }

  QString errMsg;
  int errLine;
  int errColumn;
  if (!pDoc.setContent(&file, false, &errMsg, &errLine, &errColumn))
  {
    file.close();
    errmsg = tr("Problem reading %1, line %2 column %3:<br>%4")
                      .arg(pFileName, errLine).arg(errColumn).arg(errMsg);
    return false;
  }

  file.close();

  return true;
}

// scripting exposure //////////////////////////////////////////////////////////

Q_DECLARE_METATYPE(QDomDocument)

static QScriptValue openDomDocument(QScriptContext *context,
                                    QScriptEngine  * /*engine*/)
{
  if (context->argumentCount() < 2)
    context->throwError(QScriptContext::UnknownError,
                        "not enough args passed to openDomDocument");

  QDomDocument domdoc = qscriptvalue_cast<QDomDocument>(context->argument(1));
  QString errmsg;

  bool result = ImportHelper::openDomDocument(context->argument(0).toString(),
                                              domdoc, errmsg);
  // TODO: how to we pass back errmsg output parameter?

  return QScriptValue(result);
}

static QScriptValue importXML(QScriptContext *context,
                              QScriptEngine  * /*engine*/)
{
  if (context->argumentCount() < 1)
    context->throwError(QScriptContext::UnknownError,
                        "not enough args passed to importXML");
  QString errmsg;
  QString warnmsg;
  bool result = ImportHelper::importXML(context->argument(0).toString(),
                                        errmsg, warnmsg);

  // TODO: how to we pass back errmsg and warnmsg output parameter?

  return QScriptValue(result);
}

void setupImportHelper(QScriptEngine *engine)
{
  QScriptValue obj = engine->newObject();
  obj.setProperty("importXML", engine->newFunction(importXML),      QScriptValue::ReadOnly | QScriptValue::Undeletable);
  obj.setProperty("openDomDocument", engine->newFunction(openDomDocument),QScriptValue::ReadOnly | QScriptValue::Undeletable);

  engine->globalObject().setProperty("ImportHelper", obj, QScriptValue::ReadOnly | QScriptValue::Undeletable);
}
