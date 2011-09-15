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

#include <parameter.h>
#include "xuiloader.h"
#include "uiform.h"
#include "guiclient.h"
#include "xmainwindow.h"

#define DEBUG false

uiforms::uiforms(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  connect(_new, SIGNAL(clicked()), this, SLOT(sNew()));
  connect(_edit, SIGNAL(clicked()), this, SLOT(sEdit()));
  connect(_delete, SIGNAL(clicked()), this, SLOT(sDelete()));
  connect(_test, SIGNAL(clicked()), this, SLOT(sTest()));
  
  connect(_uiform, SIGNAL(newId(int)), this, SLOT(sHandleButtons()));

  _uiform->addColumn(tr("Name"),   _itemColumn, Qt::AlignLeft,  true, "uiform_name");
  _uiform->addColumn(tr("Description"),     -1, Qt::AlignLeft,  true, "uiform_notes");
  _uiform->addColumn(tr("Grade"),    _ynColumn, Qt::AlignCenter,true, "uiform_order");
  _uiform->addColumn(tr("Enabled"),  _ynColumn, Qt::AlignCenter,true, "uiform_enabled");
  _uiform->addColumn(tr("Package"),_itemColumn, Qt::AlignLeft,  true, "nspname");

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

  q.prepare( "DELETE FROM uiform "
             "WHERE (uiform_id=:uiform_id);" );
  q.bindValue(":uiform_id", _uiform->id());
  q.exec();
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  sFillList();
}

void uiforms::sFillList()
{
  XSqlQuery r;
  r.exec(" SELECT uiform_id, uiform_name, uiform_notes, uiform_order, uiform_enabled,"
         "        CASE WHEN nspname='public' THEN ''"
         "             ELSE nspname END AS nspname,"
         "        CASE WHEN (pkghead_id IS NULL) THEN 0"
         "             ELSE 1 END AS xtindentrole"
         " FROM uiform, pg_class, pg_namespace"
         "   LEFT OUTER JOIN pkghead ON (nspname=pkghead_name) "
         " WHERE ((uiform.tableoid=pg_class.oid)"
         "   AND  (relnamespace=pg_namespace.oid))"
         " UNION "
         " SELECT -1, pkghead_name, pkghead_descrip, NULL, NULL, pkghead_name, 0"
         " FROM uiform, pg_class, pg_namespace"
         "   RIGHT OUTER JOIN pkghead ON (nspname=pkghead_name) "
         " WHERE ((uiform.tableoid=pg_class.oid)"
         "   AND  (relnamespace=pg_namespace.oid))"
         " ORDER BY nspname, xtindentrole, uiform_name, uiform_order, uiform_id;" );
  _uiform->populate(r);
  if (r.lastError().type() != QSqlError::NoError)
  {
    systemError(this, r.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void uiforms::sTest()
{
  q.prepare("SELECT *"
            "  FROM uiform "
            " WHERE(uiform_id=:uiform_id);");
  q.bindValue(":uiform_id", _uiform->id());
  q.exec();
  if (q.first())
    ; // everything's OK
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
  else
    return;
  
  XMainWindow * wnd = new XMainWindow();
  wnd->setObjectName(q.value("uiform_name").toString());

  XUiLoader loader;
  QByteArray ba = q.value("uiform_source").toString().toUtf8();
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
