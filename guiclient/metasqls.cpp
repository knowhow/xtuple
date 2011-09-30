/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "metasqls.h"

#include <QMenu>
#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include <openreports.h>
#include <metasql.h>
#include <mqlutil.h>

#include "errorReporter.h"
#include "mqledit.h"
#include "storedProcErrorLookup.h"

bool metasqls::userHasPriv(const int priv)
{
  return (priv == cView && _privileges->check("ViewMetaSQL")) ||
         ((priv == cEdit || priv == cNew) && _privileges->check("MaintainMetaSQL"));
}

metasqls::metasqls(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  connect(_byPackage, SIGNAL(toggled(bool)), this, SLOT(sFillList()));
  connect(_delete,        SIGNAL(clicked()), this, SLOT(sDelete()));
  connect(_edit,          SIGNAL(clicked()), this, SLOT(sEdit()));
  connect(_list,  SIGNAL(itemSelected(int)), this, SLOT(sEdit()));
  connect(_list,   SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*)),
                                             this, SLOT(sPopulateMenu(QMenu*)));
  connect(_list,        SIGNAL(valid(bool)), this, SLOT(sHandleButtons()));
  connect(_new,           SIGNAL(clicked()), this, SLOT(sNew()));
  connect(_print,         SIGNAL(clicked()), this, SLOT(sPrint()));

  _list->addColumn(tr("Group"),  _itemColumn, Qt::AlignLeft, true, "metasql_group");
  _list->addColumn(tr("Name"),   _itemColumn, Qt::AlignLeft, true, "metasql_name");
  _list->addColumn(tr("Grade"),   _seqColumn, Qt::AlignRight,false,"metasql_grade");
  _list->addColumn(tr("Notes"),           -1, Qt::AlignLeft, true, "metasql_notes");
  _list->addColumn(tr("Package"),_itemColumn, Qt::AlignLeft, false,"pkgname");

  if (! userHasPriv(cEdit) && userHasPriv(cView))
  {
    _new->setEnabled(false);
    _edit->setText(tr("View"));
  }

  sFillList();
}

metasqls::~metasqls()
{
  // no need to delete child widgets, Qt does it all for us
}

void metasqls::languageChange()
{
  retranslateUi(this);
}

void metasqls::sNew()
{
  MQLEdit *newdlg = new MQLEdit(0);
  omfgThis->handleNewWindow(newdlg);
  newdlg->forceTestMode(! _privileges->check("ExecuteMetaSQL"));
  connect(newdlg, SIGNAL(destroyed()), this, SLOT(sFillList()));
}

void metasqls::sDelete()
{
  if (QMessageBox::question(this, tr("Delete MetaSQL?"),
                            tr("Are you sure you want to delete this "
                               "MetaSQL statement?"),
                            QMessageBox::Yes | QMessageBox::No,
                            QMessageBox::No) == QMessageBox::No)
    return;

  // replace deleteMetaSQL with triggers then remove storedprocerrorlookup call
  XSqlQuery delq;
  delq.prepare("SELECT deleteMetaSQL(:metasql_id) AS returnVal;");
  delq.bindValue(":metasql_id", _list->id());
  delq.exec();
  if (delq.first())
  {
    int returnVal = delq.value("returnVal").toInt();
    if (returnVal < 0)
    {
      ErrorReporter::error(QtCriticalMsg, this,
                           tr("Deleting MetaSQL Statement"),
                           storedProcErrorLookup("deleteMetaSQL", returnVal),
                           __FILE__, __LINE__);
      return;
    }
  }
  else if (ErrorReporter::error(QtCriticalMsg, this,
                                tr("Deleting MetaSQL Statement"),
                                delq, __FILE__, __LINE__))
    return;

  sFillList();
}

void metasqls::sEdit()
{
#ifdef Q_WS_MAC
  if (_preferences->value("InterfaceWindowOption") == "Workspace")
  {
    QMessageBox::critical( this, tr("Interface Option is Invalid"),
                          tr("<p>The embedded MetaSQL editor "
                             "is only available when user preferences "
                             "are set to show windows as free-floating.") );
    return;
  }
#endif

  MQLEdit *newdlg = new MQLEdit(0);
  newdlg->fileDatabaseOpen(_list->id());
  newdlg->setReadOnly(! userHasPriv(cEdit) && userHasPriv(cView));
  newdlg->forceTestMode(! _privileges->check("ExecuteMetaSQL"));
  omfgThis->handleNewWindow(newdlg);
  connect(newdlg, SIGNAL(destroyed()), this, SLOT(sFillList()));
}

bool metasqls::setParams(ParameterList &params)
{
  if (_byPackage->isChecked())
    params.append("byPackage");

  return true;
}

void metasqls::sFillList()
{
  QString errmsg;
  bool    ok;
  MetaSQLQuery  getm = MQLUtil::mqlLoad("metasqls", "detail", errmsg, &ok);
  if (! ok)
  {
    ErrorReporter::error(QtCriticalMsg, this, tr("Getting MetaSQL Statements"),
                         errmsg, __FILE__, __LINE__);
    return;
  }

  ParameterList getp;
  if (! setParams(getp))
    return;

  XSqlQuery getq = getm.toQuery(getp);
  _list->populate(getq);
  if (ErrorReporter::error(QtCriticalMsg, this,
                           tr("Getting MetaSQL Statements"),
                           getq, __FILE__, __LINE__))
    return;
}

void metasqls::sPopulateMenu(QMenu *pMenu)
{
  QAction *menuAction;

  menuAction = pMenu->addAction(_privileges->check("MaintainMetaSQL") ?
                                            tr("Edit...") : tr("View..."),
                                this, SLOT(sEdit()));
  menuAction->setEnabled(_privileges->check("MaintainMetaSQL") ||
                         _privileges->check("ViewMetaSQL"));

  menuAction = pMenu->addAction(tr("Print"), this, SLOT(sPrintMetaSQL()));
  menuAction->setEnabled(_privileges->check("MaintainMetaSQL") ||
                         _privileges->check("ViewMetaSQL"));

  menuAction = pMenu->addAction(tr("Delete"), this, SLOT(sDelete()));
  menuAction->setEnabled(_privileges->check("MaintainMetaSQL"));
}

void metasqls::sPrint()
{
  ParameterList params;
  if (! setParams(params))
    return;

  orReport report("MetaSQLMasterList", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void metasqls::sPrintMetaSQL()
{
  MQLEdit newdlg(this);
  newdlg.setReadOnly(! userHasPriv(cEdit) && userHasPriv(cView));
  newdlg.fileDatabaseOpen(_list->id());
  newdlg.filePrint();
}

void metasqls::sHandleButtons()
{
  _delete->setEnabled(_list->id() >= 0 &&
                      _privileges->check("MaintainMetaSQL"));
  _edit->setEnabled(_list->id() >= 0 &&
                    (_privileges->check("MaintainMetaSQL") ||
                         _privileges->check("ViewMetaSQL")));
}
