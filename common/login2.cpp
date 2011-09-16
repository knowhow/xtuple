/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "login2.h"

#include <QAction>
#include <QVariant>
#include <QMenu>
#include <QMessageBox>
#include <QCursor>
#include <QSqlDatabase>
#include <QSqlError>
#include <QApplication>
#include <QSplashScreen>
#include <QStringList>
#include <QCheckBox>
#include <QDesktopServices>
#include <QUrl>
#include <QPushButton>

#include "xtsettings.h"
#include "dbtools.h"
#include "xsqlquery.h"
#include "login2Options.h"
#include "login2.h"
#include "qmd5.h"
#include "storedProcErrorLookup.h"

#include "splashconst.h"

login2::login2(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : QDialog(parent, modal ? (fl | Qt::Dialog) : fl)
{
  setObjectName(name);
  Q_INIT_RESOURCE(xTupleCommon);
  setupUi(this);

  _company->hide();
  _companyLit->hide();

  _options = _buttonBox->addButton(tr("Options..."), QDialogButtonBox::ActionRole);
  _recent = _buttonBox->addButton(tr("Recent"), QDialogButtonBox::ActionRole);
  _buttonBox->button(QDialogButtonBox::Ok)->setText(tr("Login"));

  connect(_buttonBox, SIGNAL(accepted()), this, SLOT(sLogin()));
  connect(_options, SIGNAL(clicked()), this, SLOT(sOptions()));
  connect(_cloudLink, SIGNAL(linkActivated(QString)), this, SLOT(cloudLink(QString)));
  connect(_otherOption, SIGNAL(toggled(bool)), _options, SLOT(setEnabled(bool)));
  connect(_otherOption, SIGNAL(toggled(bool)), _recent, SLOT(setEnabled(bool)));

  _splash = 0;

  _captive = false; _nonxTupleDB = false;
  _multipleConnections = false;
  _evalDatabaseURL = "pgsql://demo.xtuple.com:5434/%1";
  _cloudDatabaseURL= "pgsql://cloud.xtuple.com:5432/%1";

  _password->setEchoMode(QLineEdit::Password);

  updateRecentOptionsActions();
  _databaseURL = xtsettingsValue("/xTuple/_databaseURL", "pgsql://:5432/").toString();
  _enhancedAuth = xtsettingsValue("/xTuple/_enhancedAuthentication", false).toBool();
  _requireSSL = xtsettingsValue("/xTuple/_requireSSL", false).toBool();
  if(xtsettingsValue("/xTuple/_demoOption", false).toBool())
    _demoOption->setChecked(true);
  else if(xtsettingsValue("/xTuple/_cloudOption", false).toBool())
    _cloudOption->setChecked(true);
  _company->setText(xtsettingsValue("/xTuple/cloud_company", "").toString());

  adjustSize();
}

login2::~login2()
{
  // no need to delete child widgets, Qt does it all for us
}

void login2::languageChange()
{
  retranslateUi(this);
}

int login2::set(const ParameterList &pParams) { return set(pParams, 0); }

int login2::set(const ParameterList &pParams, QSplashScreen *pSplash)
{
  _splash = pSplash;
  
  QVariant param;
  bool     valid;

  param = pParams.value("username", &valid);
  if (valid)
  {
    _username->setText(param.toString());
    _password->setFocus();
    _captive = TRUE;
  }
  else
  {
    _username->setFocus();
    _captive = FALSE;
  }

  param = pParams.value("password", &valid);
  if (valid)
  {
    _password->setText(param.toString());
    _captive = TRUE;
  }

  param = pParams.value("copyright", &valid);
  if (valid)
    _copyrightLit->setText(param.toString());

  param = pParams.value("version", &valid);
  if (valid)
    _versionLit->setText(tr("Version ") + param.toString());

  param = pParams.value("build", &valid);
  if (valid)
    _build->setText(param.toString());

  param = pParams.value("evaluation", &valid);
  if (valid)
    _demoOption->setChecked(TRUE);

  param = pParams.value("cloud", &valid);
  if (valid)
    _cloudOption->setChecked(true);

  param = pParams.value("company", &valid);
  if (valid)
    _company->setText(param.toString());

  param = pParams.value("name", &valid);
  if (valid)
    _nameLit->setText(param.toString());

  param = pParams.value("databaseURL", &valid);
  if (valid)
    _databaseURL = param.toString();

  populateDatabaseInfo();

  param = pParams.value("nonxTupleDB", &valid);
  if (valid)
    _nonxTupleDB = true;

  param = pParams.value("multipleConnections", &valid);
  if (valid)
    _multipleConnections = true;

  param = pParams.value("enhancedAuth", &valid);
  if (valid)
    _enhancedAuth = param.toBool();

  param = pParams.value("requireSSL", &valid);
  if (valid)
    _requireSSL = param.toBool();

  if(pParams.inList("login"))
    sLogin();

  return 0;
}

void login2::sLogin()
{
  QSqlDatabase db;

  QString databaseURL;
  databaseURL = _databaseURL;
  if (_demoOption->isChecked())
    databaseURL = _evalDatabaseURL.arg(_username->text().trimmed());
  else if(_cloudOption->isChecked())
    databaseURL = _cloudDatabaseURL.arg(_company->text().trimmed());

  QString protocol;
  QString hostName;
  QString dbName;
  QString port;
  parseDatabaseURL(databaseURL, protocol, hostName, dbName, port);

  if (_splash)
  {
    _splash->show();
    _splash->raise();
    _splash->showMessage(tr("Initializing the Database Connector"), SplashTextAlignment, SplashTextColor);
    qApp->processEvents();
  }

  // Open the Database Driver
  if (_multipleConnections)
    db = QSqlDatabase::addDatabase("QPSQL7", dbName);
  else
    db = QSqlDatabase::addDatabase("QPSQL7");
  if (!db.isValid())
  {
    if (_splash)
      _splash->hide();
    
    QMessageBox::warning( this, tr("No Database Driver"),
                          tr("<p>A connection could not be established with "
                             "the specified Database as the Proper Database "
                             "Drivers have not been installed. Contact your "
                             "Systems Administator."));
    
    return;
  }

  if(hostName.isEmpty() || dbName.isEmpty())
  {
    if (_splash)
      _splash->hide();
    
    QMessageBox::warning(this, tr("Incomplete Connection Options"),
                         tr("<p>One or more connection options are missing. "
                            "Please check that you have specified the host "
                            "name, database name, and any other required "
                            "options.") );

    return;
  }

  db.setDatabaseName(dbName);
  db.setHostName(hostName);
  db.setPort(port.toInt());

  _cUsername = _username->text().trimmed();
  _cPassword = _password->text().trimmed();
  _cCompany  = _company->text().trimmed();
  if(_cloudOption->isChecked())
  {
    if(_cCompany.isEmpty())
    {
      QMessageBox::warning(this, tr("Incomplete Connection Options"),
        tr("<p>You must specify the Company name to connect to the cloud."));
      return;
    }
    _cUsername = _cUsername + "_" + _cCompany;
  }

  db.setUserName(_cUsername);
  if(_demoOption->isChecked())
  {
    QString passwd = QMd5(QString(_cPassword + "private" + _cUsername)); 
    db.setPassword(passwd);
  }
  else if(_cloudOption->isChecked())
  {
    QString passwd = QMd5(QString(_cPassword + "cloudkey" + _cUsername)); 
    db.setPassword(passwd);
  }
  else
  {
    if(_enhancedAuth)
    {
      QString passwd = QMd5(QString(_cPassword + "xTuple" + _cUsername));
      db.setPassword(passwd);
    }
    else
      db.setPassword(_cPassword);

    if(_requireSSL)
      db.setConnectOptions("requiressl=1");
  }

  setCursor(QCursor(Qt::WaitCursor));

  if (_splash)
  {
    _splash->showMessage(tr("Connecting to the Database"), SplashTextAlignment, SplashTextColor);
    qApp->processEvents();
  }
  
  //  Try to connect to the Database
  bool result = db.open();
  if(!result && _enhancedAuth)
  {
    QString altpasswd = QMd5(QString(_cPassword + "OpenMFG" + _cUsername));
    db.setPassword(altpasswd);
    result = db.open();
    if(result)
    {
      altpasswd = QMd5(QString(_cPassword + "xTuple" + _cUsername));
      XSqlQuery chgpass(QString("ALTER USER %1 WITH PASSWORD '%2'").arg(_cUsername).arg(altpasswd));
    }
  }

  if (!result)
  {
    if(_requireSSL)
      db.setConnectOptions();

    if (_splash)
      _splash->hide();
    
    setCursor(QCursor(Qt::ArrowCursor));

    QMessageBox::critical(this, tr("Cannot Connect to xTuple ERP Server"),
                          tr("<p>Sorry, can't connect to the specified xTuple ERP server. "
                             "<p>This may be due to a problem with your user name, password, or server connection information. "
                             "<p>Below is more detail on the connection problem: "
                             "<p>%1" )
                            .arg(db.lastError().databaseText().replace('\n', "<br>") ));
    if (!_captive)
    {
      _username->setText("");
      _username->setFocus();
    }
    else
      _password->setFocus();

    _password->setText("");
    return;
  }

  xtsettingsSetValue("/xTuple/_demoOption", (bool)_demoOption->isChecked());
  xtsettingsSetValue("/xTuple/_cloudOption", (bool)_cloudOption->isChecked());
  xtsettingsSetValue("/xTuple/cloud_company", _company->text());

  if (_splash)
  {
    _splash->showMessage(tr("Logging into the Database"), SplashTextAlignment, SplashTextColor);
    qApp->processEvents();
  }
  
  if(!_nonxTupleDB)
  {
    XSqlQuery login( "SELECT login() AS result,"
                     "       getEffectiveXtUser() AS user;" );
    setCursor(QCursor(Qt::ArrowCursor));
    if (login.first())
    {
      int result = login.value("result").toInt();
      if (result < 0)
      {
        if (_splash)
          _splash->hide();
        QMessageBox::critical(this, tr("Error Logging In"),
                              storedProcErrorLookup("login", result));
        return;
      }
      _user = login.value("user").toString();
      _databaseURL = databaseURL;
      updateRecentOptions();
      accept();
    }
    else if (login.lastError().type() != QSqlError::NoError)
    {
      if (_splash)
        _splash->hide();
      QMessageBox::critical(this, tr("System Error"),
                            tr("A System Error occurred at %1::%2:\n%3")
                              .arg(__FILE__).arg(__LINE__)
                              .arg(login.lastError().databaseText()));
    }
    else
    {
      if (_splash)
        _splash->hide();
      
      QMessageBox::critical(this, tr("System Error"),
                            tr("<p>An unknown error occurred at %1::%2. You may"
                               " not log in to the specified xTuple ERP Server "
                               "at this time.")
                              .arg(__FILE__).arg(__LINE__));
    }
  }
  else
  {
    setCursor(QCursor(Qt::ArrowCursor));
    _databaseURL = databaseURL;
    updateRecentOptions();
    accept();
  }
}

void login2::sOptions()
{
  ParameterList params;
  params.append("databaseURL", _databaseURL);

  if (_multipleConnections)
    params.append("dontSaveSettings");

  if(_enhancedAuth)
    params.append("useEnhancedAuthentication");

  if(_requireSSL)
    params.append("requireSSL");

  login2Options newdlg(this, "", TRUE);
  newdlg.set(params);
  if (newdlg.exec() != QDialog::Rejected)
  {
    updateRecentOptions();
    _databaseURL = newdlg._databaseURL;
    _enhancedAuth = newdlg._enhancedAuth->isChecked();
    _requireSSL = newdlg._requireSSL->isChecked();
    populateDatabaseInfo();
    updateRecentOptions();
    updateRecentOptionsActions();
    _username->setFocus();
  }
}

void login2::populateDatabaseInfo()
{
  QString protocol;
  QString hostName;
  QString dbName;
  QString port;

  parseDatabaseURL(_databaseURL, protocol, hostName, dbName, port);
  _server->setText(hostName);
  _database->setText(dbName);
}

QString login2::username()
{
  return _cUsername;
}

QString login2::password()
{
  return _cPassword;
}

QString login2::company()
{
  return _cCompany;
}

bool login2::useCloud() const
{
  return _cloudOption->isChecked();
}

void login2::setLogo(const QImage & img)
{
  if(img.isNull())
    _logo->setPixmap(QPixmap(":/login/images/splashXTuple.png"));
  else
    _logo->setPixmap(QPixmap::fromImage(img));
}

void login2::setEnhancedAuth(bool on)
{
  _enhancedAuth = on;
}

void login2::setRequireSSL(bool on)
{
  _requireSSL = on;
}

void login2::updateRecentOptions()
{
  if (_demoOption->isChecked() || _cloudOption->isChecked())
    return;

    
  QStringList list = xtsettingsValue("/xTuple/_recentOptionsList").toStringList();
  list.removeAll(_databaseURL);
  list.prepend(_databaseURL);
      
  xtsettingsSetValue("/xTuple/_recentOptionsList", list);
  xtsettingsSetValue("/xTuple/_databaseURL", _databaseURL);
}

void login2::updateRecentOptionsActions()
{ 
  QMenu * recentMenu = new QMenu;
  QStringList list = xtsettingsValue("/xTuple/_recentOptionsList").toStringList();
  if (list.size())
  {
    list.takeFirst();
    int size = list.size();
    if (size > 5)
      size = 5;
  
    if (size)
    {
      _recent->setEnabled(true);
      QAction *act;
      for (int i = 0; i < size; ++i) 
      {
        act = new QAction(list.value(i).remove("psql://"),this);
        connect(act, SIGNAL(triggered()), this, SLOT(selectRecentOptions()));
        recentMenu->addAction(act);
      }
  
      recentMenu->addSeparator();

      act = new QAction(tr("Clear &Menu"), this);
      act->setObjectName(QLatin1String("__xt_action_clear_menu_"));
      connect(act, SIGNAL(triggered()), this, SLOT(clearRecentOptions()));
      recentMenu->addAction(act);
    }
    else
      _recent->setEnabled(false);
  }
  else
    _recent->setEnabled(false);
  
  _recent->setMenu(recentMenu);
}

void login2::selectRecentOptions()
{
  if (const QAction *action = qobject_cast<const QAction *>(sender())) 
  {
    _databaseURL = action->iconText().prepend("psql://");
    populateDatabaseInfo();
    updateRecentOptions();
    updateRecentOptionsActions();
  }
}

void login2::clearRecentOptions()
{
    QStringList list;
    xtsettingsSetValue("/xTuple/_recentOptionsList", list);
    updateRecentOptionsActions();
}

void login2::cloudLink(const QString & /*link*/)
{
  QDesktopServices::openUrl(QUrl("http://www.xtuple.com/cloud"));
}
