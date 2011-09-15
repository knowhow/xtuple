/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "checkForUpdates.h"

#include <QFile>
#include <QSqlError>
#include <QUrl>
#include <QWebView>
#include <QXmlQuery>

#include "guiclient.h"
#include "version.h"

#define DEBUG false

checkForUpdates::checkForUpdates(QWidget* parent, Qt::WFlags fl)
    : XWidget(parent, fl)
{
  setupUi(this);

  QUrl updatequrl("http://updates.xtuple.com/checkForUpdates.php");
  updatequrl.addQueryItem("xtuple", _Version);

  XSqlQuery versions;
  versions.exec("SELECT version() AS pgver,"
                "       fetchMetricText('ServerVersion') AS dbver,"
                "       fetchMetricText('Application') AS dbprod;");
  if(versions.first())
  {
    updatequrl.addQueryItem("postgres", versions.value("pgver").toString());
    updatequrl.addQueryItem(versions.value("dbprod").toString(),
                            versions.value("dbver").toString());
  }
  else if (versions.lastError().type() != QSqlError::NoError)
    systemError(this, versions.lastError().text(), __FILE__, __LINE__);
  // no return - let's learn as much as we can

#ifdef Q_WS_MACX
  QString helpdir = QApplication::applicationDirPath() +
                    "/../Resources/helpXTupleGUIClient";
#else
  QString helpdir = QApplication::applicationDirPath() + "helpXTupleGUIClient";
#endif
  QFile helpfile(helpdir + "/index.html");
  QString helpver;
  if (helpfile.exists())
  {
    if (helpfile.open(QIODevice::ReadOnly))
    {
      QXmlQuery helpverq;
      if (helpverq.setFocus(&helpfile))
      {
        helpverq.setQuery("/descendant::p[attribute::class='releaseinfo']/text()/string()");
        if (helpverq.isValid())
        {
          QStringList helpverlist;
          if (helpverq.evaluateTo(&helpverlist) && ! helpverlist.isEmpty())
            helpver = helpverlist.at(0);
          else if (DEBUG)
            qDebug("Could not find the releaseinfo in %s",
                     qPrintable(helpfile.fileName()));
        }
        else if (DEBUG)
          qDebug("The helpver query is not valid for some reason");
      }
      else if (DEBUG)
        qDebug("Could not set focus on the help file %s",
                 qPrintable(helpfile.fileName()));
    }
    else if (DEBUG)
      qDebug("Found the help file %s but could not open it: %s",
               qPrintable(helpfile.fileName()),
               qPrintable(helpfile.errorString()));
  }
  else if (DEBUG)
    qDebug("The help file %s does not exist", qPrintable(helpfile.fileName()));
  updatequrl.addQueryItem("xTupleHelp", helpver);

  QStringList context;
  context << "xTuple" << "openrpt" << "reports"; // duplicated from main.cpp!

  versions.exec("SELECT pkghead_name, pkghead_version"
                "  FROM pkghead"
                " WHERE packageisenabled(pkghead_id);");
  while(versions.next())
  {
    updatequrl.addQueryItem(versions.value("pkghead_name").toString(),
                            versions.value("pkghead_version").toString());
    context << versions.value("pkghead_name").toString();
  }
  if (versions.lastError().type() != QSqlError::NoError)
    systemError(this, versions.lastError().text(), __FILE__, __LINE__);
  // no return - let's learn as much as we can

  // get all of the locales on the system
  versions.exec("SELECT DISTINCT lang_abbr2, country_abbr"
                "  FROM locale"
                "  JOIN lang ON (locale_lang_id=lang_id)"
                "  LEFT OUTER JOIN country ON (locale_country_id=country_id)"
                " ORDER BY lang_abbr2, country_abbr;");
  QStringList locale;
  while(versions.next())
  {
    QString tmp = versions.value("lang_abbr2").toString();
    if (! versions.value("country_abbr").isNull())
      tmp += "_" + versions.value("country_abbr").toString();
    locale.append(tmp);
  }
  if (! locale.isEmpty())
    updatequrl.addQueryItem("locales", locale.join(","));
  else if(versions.lastError().type() != QSqlError::NoError)
    systemError(this, versions.lastError().text(), __FILE__, __LINE__);

  // maybe i want to update just _my_ translation file
  versions.exec("SELECT lang_abbr2, country_abbr"
                "  FROM locale"
                "  JOIN usr ON (locale_id=usr_locale_id)"
                "  JOIN lang ON (locale_lang_id=lang_id)"
                "  LEFT OUTER JOIN country ON (locale_country_id=country_id)"
                " WHERE (usr_username=CURRENT_USER);");
  if (versions.first())
  {
    QString tmp = versions.value("lang_abbr2").toString();
    if (! versions.value("country_abbr").isNull())
      tmp += "_" + versions.value("country_abbr").toString();
    updatequrl.addQueryItem("mylocale", tmp);
  }
  else if(versions.lastError().type() != QSqlError::NoError)
    systemError(this, versions.lastError().text(), __FILE__, __LINE__);

  QString qmparam = "qm.%1.%2";
  for (int l = 0; l < locale.length(); l++)
  {
    for (int c = 0; c < context.length(); c++)
    {
      QString version("");
      QString tf = translationFile(locale.at(l), context.at(c), version);
      updatequrl.addQueryItem(qmparam.arg(context.at(c), locale.at(l)), version);
    }
  }

  if (DEBUG)
    qDebug("checkForUpdates::checkForUpdates() sending %s",
           updatequrl.toEncoded().data());

  _view->load(updatequrl);
}

checkForUpdates::~checkForUpdates()
{
  // no need to delete child widgets, Qt does it all for us
}

void checkForUpdates::languageChange()
{
  retranslateUi(this);
}

