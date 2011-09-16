/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

/*! \mainpage xTuple ERP Programmer Reference Index Page

  \section intro_sec Introduction

  \brief xTuple ERP is an application written in C++ using the %Qt cross-platform application and %user interface framework.

  This document provides reference information for people who want to
  either understand the core application code or develop %scripts
  and %packages to extend the xTuple ERP core.

  To navigate this documentation you should know the following:
  <ul>
    <li>Most xTuple classes are derived from %Qt classes. Much of the
        behavior that is available to you is provided by the %Qt
        classes.
    </li>
    <li>Most classes with names starting with the letter 'Q'
        are %Qt classes while the others are xTuple classes.
        The documentation of %Qt classes comes directly from %Qt and
        so has a different visual style than the xTuple documentation.
    </li>
    <li>Most windows and widgets implemented by xTuple use
	multiple inheritance.  An application window class might
	inherit properties and behavior from QMainWindow (probably
        indirectly through XMainWindow) and its collection of
        widgets from a related class in the Ui namespace.
        Look for the following xTuple conventions:
        <ul>
          <li>Widget names typically begin with an underscore ('_').
              If a widget name does not begin with this character then
              it's probably not important for manipulation (e.g. it's a
              layout or spacer).
          </li>
	  <li>Widget names that end with 'Lit' are usually given
	      to the labels for widgets that do something interesting.
	      For example, if you see a QLabel named '_currencyLit',
	      then there is probably another widget named '_currency'
	      that you might want to manipulate. If you hide a
	      widget then you should probably hide the corresponding
	      label, too.
          </li>
        </ul>
    </li>
    <li>Because of these inheritance relationships, you should use
        the 'List of all members' link frequently. The page documenting
        an xTuple class only shows the slots, signals, methods, and
        properties unique to or overridden by that class. The inherited
        features are only displayed if you look up the inheritance hierarchy
        either directly or through the 'List of all members.'
    </li>
    <li>If you are writing %scripts, you can use any slots, signals,
        and properties displayed on these pages. You can also use
        any methods marked Q_INVOKABLE. Methods which are <i>not</i>
        Q_INVOKABLE are not available for scripting.
    </li>
    <li>The documentation for xTuple classes is sparse at the moment.
	The addition of details is a work in progress.  If you would
	like to help with this effort, please send email to
        <a href="mailto:devdocs@xtuple.com?subject=xTuple%20ERP%20Programmer%20Reference%20contributions">devdocs@xtuple.com</a>.
	You can attach %file diffs with %comment changes to the email
	for us to review and hopefully merge into the core.

        See the
        <a href="http://www.stack.nl/~dimitri/doxygen/commands.html">Special Commands</a>
        chapter in the
        <a href="http://www.stack.nl/~dimitri/doxygen/manual.html">Doxygen Manual</a>
        for information on how to tag the documentation.
    </li>
    <li>The application code is continually changing as we improve
	the product. Classes and object and method names <i>will</i>
	change over time. Each page in this documentation is marked
	with the xTuple ERP version for which it applies. The fact
	that it is documented is not meant to imply that it will
	remain unchanged in future versions.
    </li>
  </ul>
*/

#include <stdlib.h>

#include <QApplication>
#include <QFile>
#include <QImage>
#include <QMessageBox>
#include <QPixmap>
#include <QSplashScreen>
#include <QSqlDatabase>
#include <QSqlError>
#include <QTranslator>
#include <QHttp>
#include <QUrl>

#include <dbtools.h>
#include <parameter.h>
#include <widgets.h>
#include "xtupleplugin.h"
#include "xtupleproductkey.h"

#include "login2.h"

#include "guiclient.h"
#include "version.h"
#include "metrics.h"
#include "metricsenc.h"
#include "scripttoolbox.h"
#include "xmainwindow.h"

#include "sysLocale.h"

#include "splashconst.h"
#include "xtsettings.h"
#include "welcomeStub.h"

#include <QtPlugin>
Q_IMPORT_PLUGIN(xTuplePlugin)

QString __password;

#define DEBUG false

extern void xTupleMessageOutput(QtMsgType type, const char *msg);

int main(int argc, char *argv[])
{
  Q_INIT_RESOURCE(guiclient);

  QString username;
  QString databaseURL;
  QString passwd;
  QString company;
  bool    haveUsername    = FALSE;
  bool    haveDatabaseURL = FALSE;
  bool    loggedIn        = FALSE;
  bool    haveEnhancedAuth= false;
  bool    _enhancedAuth   = false;
  bool    haveRequireSSL  = false;
  bool    _requireSSL     = false;
  bool    havePasswd      = false;
  bool    cloudOption     = false;
  bool    haveCloud       = false;
  bool    forceWelcomeStub= false;

  qInstallMsgHandler(xTupleMessageOutput);
  QApplication app(argc, argv);
  app.setOrganizationDomain("xTuple.com");
  app.setOrganizationName("xTuple");
  app.setApplicationName("xTuple");
  app.setApplicationVersion(_Version);

#if QT_VERSION >= 0x040400
  // This is the correct place for this call but on versions less
  // than 4.4 it causes a crash for an unknown reason so it is
  // called later on earlier versions.
  QCoreApplication::addLibraryPath(QString("."));
#endif

#ifndef Q_WS_MACX
  QApplication::setWindowIcon(QIcon(":/images/icon32x32.png"));
#endif

  app.processEvents();

  if (argc > 1)
  {
    for (int intCounter = 1; intCounter < argc; intCounter++)
    {
      QString argument(argv[intCounter]);

      if (argument.contains("-databaseURL=", Qt::CaseInsensitive))
      {
        haveDatabaseURL = TRUE;
        databaseURL = argument.right(argument.length() - 13);
      }
      else if (argument.contains("-username=", Qt::CaseInsensitive))
      {
        haveUsername = TRUE;
        username = argument.right(argument.length() - 10);
      }
      else if (argument.contains("-passwd=", Qt::CaseInsensitive))
      {
        havePasswd = TRUE;
        passwd     = argument.right(argument.length() - 8);
      }
      else if (argument.contains("-noAuth", Qt::CaseInsensitive))
      {
        haveUsername = TRUE;
        havePasswd   = TRUE;
      } 
      else if (argument.contains("-enhancedAuth", Qt::CaseInsensitive))
      {
        haveEnhancedAuth = true;
        _enhancedAuth = true;
        if(argument.contains("=no", Qt::CaseInsensitive) || argument.contains("=false", Qt::CaseInsensitive))
          _enhancedAuth = false;
      }
      else if (argument.contains("-requireSSL", Qt::CaseInsensitive))
      {
        haveRequireSSL = true;
        _requireSSL = true;
        if(argument.contains("=no", Qt::CaseInsensitive) || argument.contains("=false", Qt::CaseInsensitive))
          _requireSSL = false;
      }
      else if (argument.contains("-cloud", Qt::CaseInsensitive))
      {
        haveCloud = true;
        cloudOption = true;
        if(argument.contains("=no", Qt::CaseInsensitive) || argument.contains("=false", Qt::CaseInsensitive))
          cloudOption = false;
      }
      else if (argument.contains("-forceWelcomeStub", Qt::CaseInsensitive))
        forceWelcomeStub = true;
      else if (argument.contains("-company=", Qt::CaseInsensitive))
      {
        company = argument.right(argument.length() - 9);
      }
    }
  }

  // Try and load a default translation file and install it
  // otherwise if we are non-english inform the user that translation are available
  bool checkLanguage = false;
  QLocale sysl = QLocale::system();
  QTranslator defaultTranslator(&app);
  if (defaultTranslator.load(translationFile(sysl.name().toLower(), "default")))
    app.installTranslator(&defaultTranslator);
  else if(!xtsettingsValue("LanguageCheckIgnore", false).toBool() && sysl.language() != QLocale::C && sysl.language() != QLocale::English)
    checkLanguage = translationFile(sysl.name().toLower(), "xTuple").isNull();
  if (forceWelcomeStub || checkLanguage)
  {
    QTranslator * translator = new QTranslator(&app);
    if (translator->load(translationFile(sysl.name().toLower(), "welcome/wmsg")))
      app.installTranslator(translator);

    welcomeStub wsdlg;
    wsdlg.checkBox->setChecked(xtsettingsValue("LanguageCheckIgnore", false).toBool());
    wsdlg.exec();
    xtsettingsSetValue("LanguageCheckIgnore", wsdlg.checkBox->isChecked());
  }

  _splash = new QSplashScreen();
  _splash->setPixmap(QPixmap(":/images/splashEmpty.png"));

  _evaluation = FALSE;

  if (!loggedIn)
  {
    ParameterList params;
    params.append("copyright", _Copyright);
    params.append("version", _Version);
    params.append("build", QString("%1 %2").arg(__DATE__).arg(__TIME__));

    if (haveUsername)
      params.append("username", username);

    if (havePasswd)
      params.append("password", passwd);

    if (haveDatabaseURL)
      params.append("databaseURL", databaseURL);

    if (haveEnhancedAuth)
      params.append("enhancedAuth", _enhancedAuth);
    
    if (haveRequireSSL)
      params.append("requireSSL", _requireSSL);

    if (_evaluation)
      params.append("evaluation");

    if ( (haveDatabaseURL) && (haveUsername) && (havePasswd) )
      params.append("login");

    if (haveCloud)
      params.append("cloud", cloudOption);

    if (!company.isEmpty())
      params.append("company", company);

    login2 newdlg(0, "", TRUE);
    newdlg.set(params, _splash);

    if(newdlg.result() != QDialog::Accepted)
    {
      if (newdlg.exec() == QDialog::Rejected)
        return -1;
      else
      {
        databaseURL = newdlg._databaseURL;
        username = newdlg.username();
        __password = newdlg.password();
        company = newdlg.company();
        cloudOption = newdlg.useCloud();
      }
    }
  }

  XSqlQuery metric;
  metric.exec("SELECT metric_value"
           "  FROM metric"
           " WHERE (metric_name = 'Application')" );
  if(!metric.first() || (metric.value("metric_value").toString() == "Standard"))
  {
    // check if the xtmfg package is installed
    metric.exec("SELECT pkghead_name FROM pkghead WHERE pkghead_name='xtmfg'");
    if(metric.first())
    {
      metric.exec("SELECT count(*) FROM pkghead WHERE pkghead_name IN ('xtprjaccnt','asset','assetdepn')");
      if(metric.first() && metric.value(0).toInt() == 3)
      {
        _splash->setPixmap(QPixmap(":/images/splashEnterprise.png"));
        _Name = _Name.arg("Enterprise");
      }
      else
      {
        _splash->setPixmap(QPixmap(":/images/splashMfgEdition.png"));
        _Name = _Name.arg("Manufacturing");
      }
    }
    else
    {
      _splash->setPixmap(QPixmap(":/images/splashStdEdition.png"));
      _Name = _Name.arg("Standard");
    }

    _splash->showMessage(QObject::tr("Checking License Key"), SplashTextAlignment, SplashTextColor);
    qApp->processEvents();
    metric.exec("SELECT count(*) AS registered, (SELECT count(*) FROM pg_stat_activity WHERE datname=current_database()) AS total"
                "  FROM pg_stat_activity, pg_locks"
                " WHERE((database=datid)"
                "   AND (classid=datid)"
                "   AND (objsubid=2)"
                "   AND (procpid = pg_backend_pid()));");
    int cnt = 50000;
    int tot = 50000;
    if(metric.first())
    {
      cnt = metric.value("registered").toInt();
      tot = metric.value("total").toInt();
    }
    metric.exec("SELECT packageIsEnabled('drupaluserinfo') AS result;");
    bool xtweb = false;
    if(metric.first())
      xtweb = metric.value("result").toBool();
    metric.exec("SELECT metric_value"
                "  FROM metric"
                " WHERE(metric_name = 'ForceLicenseLimit');");
    bool forceLimit = false;
    bool forced = false;
    if(metric.first())
      forceLimit = metric.value("metric_value").toBool();
    metric.exec("SELECT metric_value"
                "  FROM metric"
                " WHERE(metric_name = 'RegistrationKey');");
    bool checkPass = true;
    bool checkLock = false;
    QString checkPassReason;
    QString rkey = "";
    if(metric.first())
      rkey = metric.value("metric_value").toString();
    XTupleProductKey pkey(rkey);
    if(pkey.valid() && (pkey.version() == 1 || pkey.version() == 2))
    {
      if(pkey.expiration() < QDate::currentDate())
      {
        checkPass = false;
        checkPassReason = QObject::tr("<p>Your license has expired.");
        if(!pkey.perpetual())
        {
          int daysTo = pkey.expiration().daysTo(QDate::currentDate());
          if(daysTo > 30)
          {
            checkLock = true;
            checkPassReason = QObject::tr("<p>Your xTuple license expired over 30 days ago, and this software will no longer function. Please contact xTuple immediately to reinstate your software.");
          }
          else
            checkPassReason = QObject::tr("<p>Attention:  Your xTuple license has expired, and in %1 days this software will cease to function.  Please make arrangements for immediate payment").arg(30 - daysTo);
        }
      }
      else if(pkey.users() != 0 && (pkey.users() < cnt || (!xtweb && (pkey.users() * 2 < tot))))
      {
        checkPass = false;
        checkPassReason = QObject::tr("<p>You have exceeded the number of allowed concurrent users for your license.");
        checkLock = forced = forceLimit;
      }
      else
      {
        int daysTo = QDate::currentDate().daysTo(pkey.expiration());
        if(!pkey.perpetual() && daysTo <= 15)
        {
          checkPass = false;
          checkPassReason = QObject::tr("<p>Please note: your xTuple license will expire in %1 days.  You should already have received your renewal invoice; please contact xTuple at your earliest convenience.").arg(daysTo);
        }
      }
    }
    else
    {
      checkPass = false;
      checkPassReason = QObject::tr("<p>The Registration key installed for this system does not appear to be valid.");
    }
    if(!checkPass)
    {
      _splash->hide();
      if(checkLock)
      {
        QMessageBox::critical(0, QObject::tr("Registration Key"), checkPassReason);
        if(!forced)
          return 0;
      }
      else
      {
        if(QMessageBox::critical(0, QObject::tr("Registration Key"), QObject::tr("%1\n<p>Would you like to continue anyway?").arg(checkPassReason), QMessageBox::Yes | QMessageBox::No, QMessageBox::No) == QMessageBox::No)
          return 0;
      }

      if(forced)
        checkPassReason.append(" FORCED!");

      metric.exec("SELECT current_database() AS db,"
                  "       fetchMetricText('DatabaseName') AS dbname,"
                  "       fetchMetricText('remitto_name') AS name;");
      QString db = "";
      QString dbname = "";
      QString name = "";
      if(metric.first())
      {
        db = metric.value("db").toString();
        dbname = metric.value("dbname").toString();
        name = metric.value("name").toString();
      }

      QHttp *http = new QHttp();
      
      QUrl url;
      url.setPath("/api/regviolation.php");
      url.addQueryItem("key", QUrl::toPercentEncoding(rkey));
      url.addQueryItem("error", QUrl::toPercentEncoding(checkPassReason));
      url.addQueryItem("name", QUrl::toPercentEncoding(name));
      url.addQueryItem("dbname", QUrl::toPercentEncoding(dbname));
      url.addQueryItem("db", QUrl::toPercentEncoding(db));
      url.addQueryItem("cnt", QString::number(cnt));
      url.addQueryItem("tot", QString::number(tot));
      url.addQueryItem("ver", _Version);

      http->setHost("www.xtuple.org");
      http->get(url.toString());

      if(forced)
        return 0;

      _splash->show();
    }
  }
  else
  {
    metric.exec("SELECT pkghead_name FROM pkghead WHERE pkghead_name='xtprjaccnt'");
    if(metric.first())
    {
      _splash->setPixmap(QPixmap(":/images/splashProject.png"));
      _Name = _Name.arg("Project");
    }
    else
    {
      _splash->setPixmap(QPixmap(":/images/splashPostBooks.png"));
      _Name = _Name.arg("PostBooks");
    }
  }

  metric.exec("SELECT metric_value"
           "  FROM metric"
           " WHERE (metric_name = 'ServerVersion')" );
  if(!metric.first() || (metric.value("metric_value").toString() != _dbVersion))
  {
    bool disallowMismatch = false;
    metric.exec("SELECT metric_value FROM metric WHERE(metric_name='DisallowMismatchClientVersion')");
    if(metric.first() && (metric.value("metric_value").toString() == "t"))
      disallowMismatch = true;
    
    _splash->hide();
    int result;
    if(disallowMismatch)
      result = QMessageBox::warning( 0, QObject::tr("Version Mismatch"),
        QObject::tr("<p>The version of the database you are connecting to is "
                    "not the version this client was designed to work against. "
                    "This client was designed to work against the database "
                    "version %1. The system has been configured to disallow "
                    "access in this case.<p>Please contact your systems "
                    "administrator.").arg(_dbVersion),
                 QMessageBox::Ok | QMessageBox::Escape | QMessageBox::Default );
    else
      result = QMessageBox::warning( 0, QObject::tr("Version Mismatch"),
        QObject::tr("<p>The version of the database you are connecting to is "
                    "not the version this client was designed to work against. "
                    "This client was designed to work against the database "
                    "version %1. If you continue some or all functionality may "
                    "not work properly or at all. You may also cause other "
                    "problems on the database.<p>Do you want to continue "
                    "anyway?").arg(_dbVersion),
                 QMessageBox::Yes,
                 QMessageBox::No | QMessageBox::Escape | QMessageBox::Default );
    if(result != QMessageBox::Yes)
      return 0;
    _splash->show();
  }

  _splash->showMessage(QObject::tr("Loading Database Metrics"), SplashTextAlignment, SplashTextColor);
  qApp->processEvents();
  _metrics = new Metrics();

  _splash->showMessage(QObject::tr("Loading User Preferences"), SplashTextAlignment, SplashTextColor);
  qApp->processEvents();
  _preferences = new Preferences(username);

  _splash->showMessage(QObject::tr("Loading User Privileges"), SplashTextAlignment, SplashTextColor);
  qApp->processEvents();
  _privileges = new Privileges();

  // Load the translator and set the locale from the User's preferences
  _splash->showMessage(QObject::tr("Loading Translation Dictionary"), SplashTextAlignment, SplashTextColor);
  qApp->processEvents();
  XSqlQuery langq("SELECT * "
                  "FROM usr, locale LEFT OUTER JOIN"
                  "     lang ON (locale_lang_id=lang_id) LEFT OUTER JOIN"
                  "     country ON (locale_country_id=country_id) "
                  "WHERE ( (usr_username=getEffectiveXtUser())"
                  " AND (usr_locale_id=locale_id) );" );
  if (langq.first())
  {
    QStringList files;
    if (!langq.value("locale_lang_file").toString().isEmpty())
      files << langq.value("locale_lang_file").toString();

    QString langext;
    if (!langq.value("lang_abbr2").toString().isEmpty() && 
        !langq.value("country_abbr").toString().isEmpty())
    {
      langext = langq.value("lang_abbr2").toString() + "_" +
                langq.value("country_abbr").toString().toLower();
    }
    else if (!langq.value("lang_abbr2").toString().isEmpty())
    {
      langext = langq.value("lang_abbr2").toString();
    }

    if(!langext.isEmpty())
    {
      files << "qt";
      files << "xTuple";
      files << "openrpt";
      files << "reports";

      XSqlQuery pkglist("SELECT pkghead_name"
                        "  FROM pkghead"
                        " WHERE packageIsEnabled(pkghead_name);");
      while(pkglist.next())
        files << pkglist.value("pkghead_name").toString();
    }

    if (files.size() > 0)
    {
      QStringList notfound;
      QTranslator *translator = new QTranslator(&app);
      for (QStringList::Iterator fit = files.begin(); fit != files.end(); ++fit)
      {
        if (DEBUG)
          qDebug("looking for %s", (*fit).toAscii().data());
        if (translator->load(translationFile(langext, *fit)))
        {
          app.installTranslator(translator);
          qDebug("installed %s", (*fit).toAscii().data());
          translator = new QTranslator(&app);
        }
        else
        {
          notfound << *fit;
        }
      }

      if (! notfound.isEmpty() &&
          !_preferences->boolean("IngoreMissingTranslationFiles"))
        QMessageBox::warning( 0, QObject::tr("Cannot Load Dictionary"),
                              QObject::tr("<p>The Translation Dictionaries %1 "
                                          "cannot be loaded. Reverting "
                                          "to the default dictionary." )
                                       .arg(notfound.join(QObject::tr(", "))));
    }

    /* set the locale to langabbr_countryabbr, langabbr, {lang# country#}, or
       lang#, depending on what information is available
     */
    QString langAbbr = langq.value("lang_abbr2").toString();
    QString cntryAbbr = langq.value("country_abbr").toString().toUpper();
    if(cntryAbbr == "UK")
      cntryAbbr = "GB";
    if (! langAbbr.isEmpty() &&
        ! cntryAbbr.isEmpty())
      QLocale::setDefault(QLocale(langAbbr + "_" + cntryAbbr));
    else if (! langAbbr.isEmpty())
      QLocale::setDefault(QLocale(langq.value("lang_abbr2").toString()));
    else if (langq.value("lang_qt_number").toInt() &&
             langq.value("country_qt_number").toInt())
      QLocale::setDefault(
          QLocale(QLocale::Language(langq.value("lang_qt_number").toInt()),
                  QLocale::Country(langq.value("country_qt_number").toInt())));
    else
      QLocale::setDefault(QLocale::system());

    qDebug("Locale set to language %s and country %s",
           QLocale().languageToString(QLocale().language()).toAscii().data(),
           QLocale().countryToString(QLocale().country()).toAscii().data());

  }
  else if (langq.lastError().type() != QSqlError::NoError)
  {
    systemError(0, langq.lastError().databaseText(), __FILE__, __LINE__);
  }

  qApp->processEvents();
  QString key;

  // TODO: Add code to check a few locations - Hopefully done

  QString keypath;
  QString keyname;
  QString keytogether;
  
#ifdef Q_WS_WIN
  keypath = _metrics->value("CCWinEncKey");
#elif defined Q_WS_MACX
  keypath = _metrics->value("CCMacEncKey");
#elif defined Q_WS_X11
  keypath = _metrics->value("CCLinEncKey");
#endif
  
  if (keypath.isEmpty())
    keypath = app.applicationDirPath();

  if (! keypath.endsWith(QDir::separator()))
    keypath += QDir::separator();

  keyname = _metrics->value("CCEncKeyName");
  if (keyname.isEmpty())
  {
    keyname = "xTuple.key";
    keytogether = keypath + keyname;
    QFile kn(keytogether);
    if(!kn.exists())
      keyname = "OpenMFG.key";
  }
  
  keytogether = keypath + keyname;
  
  // qDebug("keytogether: %s", keytogether.toAscii().data());
  QFile keyFile(keytogether);

  if(keyFile.exists())
  {
    if(keyFile.open(QIODevice::ReadOnly))
    {
      key = keyFile.readLine(1024);
      // strip off any newline characters
      key = key.trimmed();
    }
  }

  omfgThis = 0;
  omfgThis = new GUIClient(databaseURL, username);
  omfgThis->_key = key;
  omfgThis->_company = company;
  omfgThis->_useCloud = cloudOption;

// qDebug("Encryption Key: %s", key.toAscii().data() );
  
  if (key.length() > 0) {
	_splash->showMessage(QObject::tr("Loading Database Encryption Metrics"), SplashTextAlignment, SplashTextColor);
	qApp->processEvents();
	_metricsenc = new Metricsenc(key);
  }
  
  initializePlugin(_preferences, _metrics, _privileges, omfgThis->username(), omfgThis->workspace());

// START code for updating the locale settings if they haven't been already
  XSqlQuery lc;
  lc.exec("SELECT count(*) FROM metric WHERE metric_name='AutoUpdateLocaleHasRun';");
  lc.first();
  if(lc.value(0).toInt() == 0)
  {
    lc.exec("INSERT INTO metric (metric_name, metric_value) values('AutoUpdateLocaleHasRun', 't');");
    lc.exec("SELECT locale_id from locale;");
    while(lc.next())
    {
      ParameterList params;
      params.append("mode","edit");
      params.append("locale_id", lc.value(0));
      sysLocale lcdlg;
      lcdlg.set(params);
      lcdlg.sSave();
    }
  }
// END code for updating locale settings

  QObject::connect(&app, SIGNAL(aboutToQuit()), &app, SLOT(closeAllWindows()));
  if (omfgThis->_singleWindow.isEmpty())
  {
    omfgThis->setAttribute(Qt::WA_DeleteOnClose);
    omfgThis->show();
  }
  // keep this synchronized with GUIClient and user.ui.h
  else if (omfgThis->_singleWindow == "woTimeClock")
  {
    ScriptToolbox sb(0);
    QWidget* newdlg = sb.openWindow("woTimeClock");
    if(newdlg)
    {
      XMainWindow *mw = qobject_cast<XMainWindow*>(newdlg);
      if(mw)
      {
        ParameterList params;
        params.append("captive");
        mw->set(params);
      }
      newdlg->setAttribute(Qt::WA_DeleteOnClose);
      QObject::connect(omfgThis, SIGNAL(destroyed(QObject*)), &app, SLOT(quit()));
      newdlg->show();
    }
    else
    {
      qDebug("Failed to initialize woTimeClock window.");
      return -2;
    }
  }

  if(!omfgThis->singleCurrency() &&
     _metrics->value("GLCompanySize").toInt() == 0)
  {
    // Check for the gain/loss and discrep accounts
    q.prepare("SELECT COALESCE((SELECT TRUE"
              "                   FROM accnt, metric"
              "                  WHERE ((CAST(accnt_id AS text)=metric_value)"
              "                    AND  (metric_name='CurrencyGainLossAccount'))), FALSE)"
              "   AND COALESCE((SELECT TRUE"
              "                   FROM accnt, metric"
              "                  WHERE ((CAST(accnt_id AS text)=metric_value)"
              "                    AND  (metric_name='GLSeriesDiscrepancyAccount'))), FALSE) AS result; ");
    q.exec();
    if(q.first() && q.value("result").toBool() != true)
      QMessageBox::warning( omfgThis, QObject::tr("Additional Configuration Required"),
        QObject::tr("<p>Your system is configured to use multiple Currencies, "
                    "but the Currency Gain/Loss Account and/or the G/L Series "
                    "Discrepancy Account does not appear to be configured "
                    "correctly. You should define these Accounts in 'System | "
                    "Configure Modules | Configure G/L...' before posting any "
                    "transactions in the system.") );
  }

  app.exec();

//  Clean up
  delete _metrics;
  delete _preferences;
  delete _privileges;
  if (0 != _metricsenc)
    delete _metricsenc;

  return 0;
}
