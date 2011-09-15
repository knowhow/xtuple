/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "packages.h"

#include <QAction>
#include <QMenu>
#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include <dbtools.h>
#include <openreports.h>

#include "package.h"
#include "storedProcErrorLookup.h"

packages::packages(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  connect(_autoUpdate, SIGNAL(toggled(bool)), this, SLOT(sHandleAutoUpdate(bool)));
  connect(_close,   SIGNAL(clicked()), this, SLOT(close()));
  connect(_delete,  SIGNAL(clicked()), this, SLOT(sDelete()));
  connect(_disable, SIGNAL(clicked()), this, SLOT(sDisable()));
  connect(_edit,    SIGNAL(clicked()), this, SLOT(sEdit()));
  connect(_enable,  SIGNAL(clicked()), this, SLOT(sEnable()));
  connect(_load,    SIGNAL(clicked()), this, SLOT(sLoad()));
  connect(_new,     SIGNAL(clicked()), this, SLOT(sNew()));
  connect(_package, SIGNAL(populateMenu(QMenu *, QTreeWidgetItem *, int)), this, SLOT(sPopulateMenu(QMenu*)));
  connect(_print,   SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_view,    SIGNAL(clicked()), this, SLOT(sView()));

  _package->addColumn(tr("Name"),    _itemColumn, Qt::AlignLeft, true, "pkghead_name");
  _package->addColumn(tr("Description"),      -1, Qt::AlignLeft, true, "pkghead_descrip");
  _package->addColumn(tr("Version"), _itemColumn, Qt::AlignRight,true, "pkghead_version");
  _package->addColumn(tr("Enabled"),   _ynColumn, Qt::AlignCenter,true, "enabled");

  _load->setEnabled(package::userHasPriv(cNew));
  // It would be nice if we could create (8225) and export (8224) packages
  //_new->setEnabled(package::userHasPriv(cNew));
  _new->setVisible(false);
  _edit->setEnabled(package::userHasPriv(cEdit));

  if (package::userHasPriv(cEdit))
  {
    disconnect(_package, SIGNAL(itemSelected(int)), _view, SLOT(animateClick()));
    connect(_package,      SIGNAL(valid(bool)), _edit,  SLOT(setEnabled(bool)));
    connect(_package,SIGNAL(itemSelected(int)), _edit,  SLOT(animateClick()));
  }
  else
    connect(_package, SIGNAL(itemSelected(int)), _view, SLOT(animateClick()));

  if (package::userHasPriv(cNew))
  {
    connect(_package, SIGNAL(valid(bool)), this, SLOT(sHandleButtons(bool)));
  }

  sHandleAutoUpdate(_autoUpdate->isChecked());
}

packages::~packages()
{
  // no need to delete child widgets, Qt does it all for us
}

void packages::languageChange()
{
  retranslateUi(this);
}

void packages::sFillList()
{
  q.prepare( "SELECT *, packageIsEnabled(pkghead_name) AS enabled "
             "FROM pkghead "
             "ORDER BY pkghead_name, pkghead_version DESC;" );
  q.exec();
  _package->populate(q);
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void packages::sDelete()
{
  if (QMessageBox::question(this, tr("Delete Package?"),
                            tr("<p>Are you sure you want to delete the package "
                               "%1?<br>If you answer 'Yes' then you should "
                               "have backed up your database first.")
                            .arg(_package->currentItem()->text(0)),
                            QMessageBox::Yes,
                            QMessageBox::No | QMessageBox::Default) == QMessageBox::No)
    return;

  q.prepare( "SELECT deletePackage(:pkghead_id) AS result;" );
  q.bindValue(":pkghead_id", _package->id());
  q.exec();
  if (q.first())
  {
    int result = q.value("result").toInt();
    if (result < 0)
    {
      systemError(this, storedProcErrorLookup("deletePackage", result),
                  __FILE__, __LINE__);
      return;
    }
  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  sFillList();
}

// TODO: implement the updater as a Qt application plugin and call it directly
// for now, launch it as an external program but try to preset the db connection info
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QProcess>
#include <QSqlDatabase>
void packages::sLoad()
{
  QProcess proc(this);
#ifdef Q_WS_MACX
  QString proc_path = QDir::cleanPath(qApp->applicationDirPath() +
                      "/../../../updater.app/Contents/MacOS/updater");
  QString proc_path2= QDir::cleanPath(qApp->applicationDirPath() +
                      "/../../../../Updater/updater.app/Contents/MacOS/updater");
#elif defined Q_WS_WIN
  QString proc_path = QDir::cleanPath(qApp->applicationDirPath() + "/updater.exe");
  QString proc_path2= QDir::cleanPath(qApp->applicationDirPath() + "/../Updater/updater.exe");
#else
  QString proc_path = QDir::cleanPath(qApp->applicationDirPath() + "/updater");
  QString proc_path2= QDir::cleanPath(qApp->applicationDirPath() + "/../Updater/updater");
#endif
  if (! QFile::exists(proc_path))
    proc_path = proc_path2;
  if (! QFile::exists(proc_path))
  {
#ifdef Q_WS_MACX
    if (QMessageBox::question(this, tr("Could Not Find Updater"),
                              tr("<p>xTuple ERP could not find the Updater "
                                 "application. Would you like to look for it?"),
                              QMessageBox::Yes | QMessageBox::Default,
                              QMessageBox::No) == QMessageBox::No)
      return;
#endif
    proc_path = QFileDialog::getOpenFileName(this,
                                             tr("Find Updater Application"));
    if (proc_path.isEmpty())
      return;
#ifdef Q_WS_MACX
    proc_path += "/Contents/MacOS/updater";
#endif
  }

  QStringList proc_args;
  QSqlDatabase db = QSqlDatabase::database();
  QString dbURL;
  buildDatabaseURL(dbURL, "QPSQL", db.hostName(), db.databaseName(),
                   QString::number(db.port()));
  proc_args << "-databaseURL=" + dbURL;
  if (! db.userName().isEmpty())
    proc_args << "-username=" + db.userName();
  if (! db.password().isEmpty())
    proc_args << "-passwd=" + db.password();

  QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
  proc.start(proc_path, proc_args);
  if (proc.waitForStarted() &&
      proc.waitForFinished(-1) &&
      proc.exitStatus() == QProcess::NormalExit &&
      proc.exitCode() == 0)
  {
    QApplication::restoreOverrideCursor();
    sFillList();
  }
  else
  {
    if (! db.password().isEmpty())
    {
      proc_args.removeLast();
      proc_args << "-passwd=XXXXX";
    }
    QApplication::restoreOverrideCursor();
    systemError(this,
                tr("<p>There was an error running the Updater program: "
                   "<br>%1 %2<br><br><pre>%3</pre>")
		  .arg(proc_path)
                  .arg(proc_args.join(" "))
		  .arg(QString(proc.readAllStandardError())));
    return;
  }

  sFillList();
}

void packages::sNew()
{
  ParameterList params;
  params.append("mode", "new");

  package newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void packages::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("pkghead_id", _package->id());

  package newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void packages::sView()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("pkghead_id", _package->id());

  package newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void packages::sPopulateMenu(QMenu *pMenu)
{
  QAction *menuItem;

  menuItem = pMenu->addAction(tr("View..."), this, SLOT(sView()));
  menuItem->setEnabled(package::userHasPriv(cView));

  menuItem = pMenu->addAction(tr("Delete"), this, SLOT(sDelete()));
  menuItem->setEnabled(package::userHasPriv(cNew));

  menuItem = pMenu->addAction(tr("Enable"), this, SLOT(sEnable()));
  menuItem->setEnabled(package::userHasPriv(cNew));

  menuItem = pMenu->addAction(tr("Disable"), this, SLOT(sDisable()));
  menuItem->setEnabled(package::userHasPriv(cNew));
}

void packages::sPrint()
{
  orReport report("PackageMasterList");
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void packages::sHandleAutoUpdate(const bool pAutoUpdate)
{
  if (pAutoUpdate)
    connect(omfgThis, SIGNAL(tick()), this, SLOT(sFillList()));
  else
    disconnect(omfgThis, SIGNAL(tick()), this, SLOT(sFillList()));
  sFillList();
}

void packages::sEnable()
{
  XSqlQuery eq;
  eq.prepare("SELECT enablePackage(:id) AS result;");
  eq.bindValue(":id", _package->id());
  eq.exec();
  if (eq.first())
  {
    int result = eq.value("result").toInt();
    if (result < 0)
    {
      systemError(this, storedProcErrorLookup("enablePackage", result)
                          .arg(_package->id()),
                  __FILE__, __LINE__);
      return;
    }
  }
  else if (eq.lastError().type() != QSqlError::NoError)
  {
    systemError(this, eq.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
  sFillList();
}

void packages::sDisable()
{
  XSqlQuery dq;
  dq.prepare("SELECT disablePackage(:id) AS result;");
  dq.bindValue(":id", _package->id());
  dq.exec();
  if (dq.first())
  {
    int result = dq.value("result").toInt();
    if (result < 0)
    {
      systemError(this, storedProcErrorLookup("disablePackage", result)
                          .arg(_package->id()),
                  __FILE__, __LINE__);
      return;
    }
  }
  else if (dq.lastError().type() != QSqlError::NoError)
  {
    systemError(this, dq.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
  sFillList();
}

void packages::sHandleButtons(const bool pvalid)
{
  _delete->setEnabled(pvalid);
  _disable->setEnabled(pvalid &&
                       _package->currentItem()->rawValue("enabled").toBool());
  _enable->setEnabled(pvalid &&
                      ! _package->currentItem()->rawValue("enabled").toBool());
}
