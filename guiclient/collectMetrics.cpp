#include <QDebug>
#include <QSqlQuery>
#include <QVariant>
#include <QString>
#include <QStringList>
#include <QMap>
#include <QDomDocument>
#include <QDomElement>
#include <QDomText>
#include <QFile>
#include <QTextStream>
#include <QProcess>
#include <QApplication>
#include <QDir>

#if defined Q_WS_WIN
#include <windows.h>
#include <shlobj.h>
#endif

#include "guiclient.h"
#include "version.h"

void collectMetrics()
{
  QSqlQuery qry;
  QString result;
  QStringList list;
  QMap<QString,QString> values;
  // Version client
  values.insert("client_version", _Version);

  // Edition
  values.insert("edition", _metrics->value("Application"));

  // custom?
  // TODO: I don't know how we can tell this

  // Users
  result = "";
  qry.exec("SELECT count(*) FROM pg_stat_activity WHERE datname=current_database();");
  if(qry.first())
    result = qry.value(0).toString();
  values.insert("users", result);

  // clients
  // TODO: Don't know how to get this

  // packages loaded
  qry.exec("SELECT pkghead_name FROM pkghead;");
  list.clear();
  while(qry.next())
    list.append(qry.value(0).toString());
  values.insert("packages", list.join(","));

  // patches
  values.insert("patches", _metrics->value("ServerPatchVersion"));

  // shared memory
  result = "";
  qry.exec("SHOW shared_buffers");
  if(qry.first())
    result = qry.value(0).toString();
  values.insert("shared_buffers", result);

  // db version
  result = "";
  qry.exec("SHOW server_version");
  if(qry.first())
    result = qry.value(0).toString();
  values.insert("database_version", result);

  // db name
  result = "";
  qry.exec("SELECT current_database();");
  if(qry.first())
    result = qry.value(0).toString();
  values.insert("database_name", result);

  // db port
  result = "";
  qry.exec("SHOW port");
  if(qry.first())
    result = qry.value(0).toString();
  values.insert("database_port", result);

  // ssl
  result = "";
  qry.exec("SHOW ssl");
  if(qry.first())
    result = qry.value(0).toString();
  values.insert("ssl", result);

  // enhanced auth
  result = "";
  qry.exec("SELECT usrpref_value "
           "  FROM usrpref "
           " WHERE ( (usrpref_name = 'UseEnhancedAuthentication') "
           "   AND (usrpref_username=getEffectiveXtUser()) ); ");
  if(qry.first())
    result = qry.value(0).toString();
  values.insert("enhanced_auth", result);

  // Create XML document
  QDomDocument doc;
  QDomElement root = doc.createElement("properties");
  doc.appendChild(root);

  QMapIterator<QString, QString> i(values);
  while (i.hasNext())
  {
    i.next();

    QDomElement elem = doc.createElement("property");
    root.appendChild(elem);

    QDomElement name = doc.createElement("name");
    elem.appendChild(name);
  
    QDomText t1 = doc.createTextNode(i.key());
    name.appendChild(t1);

    QDomElement value = doc.createElement("value");
    elem.appendChild(value);
  
    QDomText t2 = doc.createTextNode(i.value());
    value.appendChild(t2);
  }

  QString ns_agent_config_path;
  QString ns_agent_binary_path;
  QString ns_agent_shared_config;

  QString path = QApplication::applicationDirPath();
  QFile f(path+"/network.cfg");
  if(!f.open(QIODevice::ReadOnly | QIODevice::Text))
  {
    return; // We don't need to do anything if we can't open the file
  }
  QTextStream in(&f);
  QString line = in.readLine(); 
  while(!line.isNull())
  {
    int p = line.indexOf("=");
    if(p >= 0)
    {
      QString key, val;
      key = line.section("=", 0, 0);
      val = line.section("=", 1);
      if(key == "ns_agent_config_path")
        ns_agent_config_path = val;
      else if(key == "ns_agent_binary_path")
        ns_agent_binary_path = val;
      else if(key == "ns_agent_shared_config")
        ns_agent_shared_config = val;
    }
    line = in.readLine();
  }
  f.close();

  
  QString appdata = "";
#if defined Q_WS_WIN
  TCHAR szPath[MAX_PATH];
  if(SUCCEEDED(SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, 0, szPath))) 
  {
#ifdef UNICODE
    appdata = QString::fromUtf16((ushort*)szPath);
#else
    appdata = QString::fromLocal8Bit(szPath);
#endif
  }
#endif
  ns_agent_config_path.replace("%APPDATA%", appdata);
  ns_agent_binary_path.replace("%APPDATA%", appdata);
  ns_agent_shared_config.replace("%APPDATA%", appdata);

  QString home = QDir::toNativeSeparators(QDir::homePath());
  ns_agent_config_path.replace("$HOME", home);
  ns_agent_binary_path.replace("$HOME", home);
  ns_agent_shared_config.replace("$HOME", home);
  ns_agent_config_path.replace("~", home);
  ns_agent_binary_path.replace("~", home);
  ns_agent_shared_config.replace("~", home);
  
  if(!ns_agent_config_path.startsWith("/") && ns_agent_config_path.at(1) != ':')
    ns_agent_config_path.prepend(QCoreApplication::applicationDirPath() + "/");
  if(!ns_agent_binary_path.startsWith("/") && ns_agent_binary_path.at(1) != ':')
    ns_agent_binary_path.prepend(QCoreApplication::applicationDirPath() + "/");
  if(!ns_agent_shared_config.startsWith("/") && ns_agent_shared_config.at(1) != ':')
    ns_agent_shared_config.prepend(QCoreApplication::applicationDirPath() + "/");

  QDir conf(ns_agent_config_path);
  if(!conf.exists())
  {
    conf.makeAbsolute();
    conf.mkpath(conf.path());
    QFile cp(ns_agent_shared_config);
    if(!cp.copy(ns_agent_config_path + "/agent.conf"))
    {
      qDebug("Error copying agent config.");
    }
  }

  // write the xml to a file
  QFile file(ns_agent_config_path + "/xtuple-ns-metrics.xml");
  if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
  {
    qDebug("whoops"); // TODO: should do something better here or nothing at all
    return;
  }

  QTextStream out(&file);
  out << doc.toString();
  file.close();


  QString exe = ns_agent_binary_path + "/agent";
#if defined Q_WS_X11
  exe.append(".bin");
#endif
#if defined Q_WS_WIN
  exe.append(".exe");
#endif
  QProcess * proc = new QProcess();

  QStringList arguments;
/*
// Since we are passing arguments we can't use open on mac
#if defined Q_WS_MACX
  arguments << exe;
  exe = "open";
#endif
*/

  // TODO: add additional arguments here
  arguments << "--run";
  arguments << "--enable-gui";
  arguments << "--pid-file";
  arguments << ns_agent_config_path + "/agent.pid";
  arguments << "--log-file";
  arguments << ns_agent_config_path + "/agent.log";
  arguments << "--config-file";
  arguments << ns_agent_config_path + "/agent.conf";

  proc->start(exe, arguments);
}
