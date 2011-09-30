/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "uiforms.h"

#include <QBuffer>
#include <QMessageBox>
#include <QSqlError>

#include <mqlutil.h>
#include <parameter.h>

#include "errorReporter.h"
#include "guiclient.h"
#include "uiform.h"
#include "xmainwindow.h"
#include "xuiloader.h"

#define DEBUG false

uiforms::uiforms(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  connect(_byPackage, SIGNAL(toggled(bool)), this, SLOT(sFillList()));
  connect(_delete,        SIGNAL(clicked()), this, SLOT(sDelete()));
  connect(_edit,          SIGNAL(clicked()), this, SLOT(sEdit()));
  connect(_new,           SIGNAL(clicked()), this, SLOT(sNew()));
  connect(_test,          SIGNAL(clicked()), this, SLOT(sTest()));
  connect(_uiform,       SIGNAL(newId(int)), this, SLOT(sHandleButtons()));

  _uiform->addColumn(tr("Name"),   _itemColumn, Qt::AlignLeft,  true, "uiform_name");
  _uiform->addColumn(tr("Description"),     -1, Qt::AlignLeft,  true, "uiform_notes");
  _uiform->addColumn(tr("Grade"),    _ynColumn, Qt::AlignCenter,true, "uiform_order");
  _uiform->addColumn(tr("Enabled"),  _ynColumn, Qt::AlignCenter,true, "uiform_enabled");
  _uiform->addColumn(tr("Package"),_itemColumn, Qt::AlignLeft,  true, "pkgname");

  sFillList();
}

uiforms::~uiforms()
{
  // no need to delete child widgets, Qt does it all for us
}

void uiforms::languageChange()
{
  retranslateUi(this);
}

void uiforms::sNew()
{
  ParameterList params;
  params.append("mode", "new");

  uiform *newdlg = new uiform(this, "");
  newdlg->set(params);

  omfgThis->handleNewWindow(newdlg);
  connect(newdlg, SIGNAL(destroyed(QObject*)), this, SLOT(sFillList()));
}

void uiforms::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("uiform_id", _uiform->id());

  uiform *newdlg = new uiform(this, "");
  newdlg->set(params);

  omfgThis->handleNewWindow(newdlg);
  connect(newdlg, SIGNAL(destroyed(QObject*)), this, SLOT(sFillList()));
}

void uiforms::sDelete()
{
  if (QMessageBox::question(this,  tr("Really delete?"),
                            tr("Are you sure you want to delete this screen?"),
                            QMessageBox::Yes | QMessageBox::No,
                            QMessageBox::No) == QMessageBox::No)
    return;

  XSqlQuery delq;
  delq.prepare("DELETE FROM uiform WHERE (uiform_id=:uiform_id);" );
  delq.bindValue(":uiform_id", _uiform->id());
  delq.exec();
  if (ErrorReporter::error(QtCriticalMsg, this, tr("Deleting Screen"),
                           delq, __FILE__, __LINE__))
    return;

  sFillList();
}

bool uiforms::setParams(ParameterList &params)
{
  if (_byPackage->isChecked())
    params.append("byPackage");

  return true;
}

void uiforms::sFillList()
{
  QString errmsg;
  bool    ok;
  MetaSQLQuery  getm = MQLUtil::mqlLoad("uiforms", "detail", errmsg, &ok);
  if (! ok)
  {
    ErrorReporter::error(QtCriticalMsg, this, tr("Getting Screens"),
                         errmsg, __FILE__, __LINE__);
    return;
  }

  ParameterList getp;
  if (! setParams(getp))
    return;

  XSqlQuery getq = getm.toQuery(getp);
  _uiform->populate(getq);
  if (ErrorReporter::error(QtCriticalMsg, this, tr("Getting Screens"),
                           getq, __FILE__, __LINE__))
    return;
}

void uiforms::sTest()
{
  XSqlQuery getq;
  getq.prepare("SELECT *"
               "  FROM uiform "
               " WHERE(uiform_id=:uiform_id);");
  getq.bindValue(":uiform_id", _uiform->id());
  getq.exec();
  if (getq.first())
    ; // everything's OK
  else if (ErrorReporter::error(QtCriticalMsg, this, tr("Getting Form"),
                                getq, __FILE__, __LINE__))
    return;
  else
    return;

  XMainWindow * wnd = new XMainWindow();
  wnd->setObjectName(getq.value("uiform_name").toString());

  XUiLoader loader;
  QByteArray ba = getq.value("uiform_source").toString().toUtf8();
  if (DEBUG)
    qDebug("about to load a uiFile with %s", ba.constData());
  QBuffer uiFile(&ba);
  if(!uiFile.open(QIODevice::ReadOnly))
  {
    QMessageBox::critical(this, tr("Could not load file"),
        tr("There was an error loading the UI Form from the database."));
    return;
  }
  QWidget *ui = loader.load(&uiFile);
  if (ui)
  {
    QSize size = ui->size();
    wnd->setWindowTitle(ui->windowTitle());
    wnd->setCentralWidget(ui);
    omfgThis->handleNewWindow(wnd);
    wnd->resize(size);
  }
  else
    QMessageBox::critical(this, tr("Could not load file"),
                          tr("<p>Could not interpret the UI Form data "
                             "as a UI definition."));
  uiFile.close();
}

void uiforms::sHandleButtons()
{
  if (_uiform->id() < 0)
  {
    _delete->setEnabled(false);
    _edit->setEnabled(false);
    _test->setEnabled(false);
  }
}
