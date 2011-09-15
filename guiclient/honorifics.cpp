/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "honorifics.h"

#include <QAction>
#include <QMenu>
#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include <openreports.h>

#include "honorific.h"

/*
   honorifics is internal name, titles is external name.  names
   differ because 'title' might make more sense to users but we
   need to distinguish between Mr/Dr/Miss/Mrs used to address people
   directly and a job title such as Director of Operations and
   Shop Foreman in case this ever comes up.
 */

honorifics::honorifics(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  connect(_honorifics, SIGNAL(populateMenu(QMenu *, QTreeWidgetItem *, int)), this, SLOT(sPopulateMenu(QMenu*)));
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_new, SIGNAL(clicked()), this, SLOT(sNew()));
  connect(_edit, SIGNAL(clicked()), this, SLOT(sEdit()));
  connect(_delete, SIGNAL(clicked()), this, SLOT(sDelete()));
  connect(_view, SIGNAL(clicked()), this, SLOT(sView()));

  _honorifics->addColumn(tr("Title"), -1, Qt::AlignLeft, true, "hnfc_code");

  if (_privileges->check("MaintainTitles"))
  {
    connect(_honorifics, SIGNAL(valid(bool)), _edit, SLOT(setEnabled(bool)));
    connect(_honorifics, SIGNAL(valid(bool)), _delete, SLOT(setEnabled(bool)));
    connect(_honorifics, SIGNAL(itemSelected(int)), _edit, SLOT(animateClick()));
  }
  else
  {
    _new->setEnabled(FALSE);
    connect(_honorifics, SIGNAL(itemSelected(int)), _view, SLOT(animateClick()));
  }

  sFillList();
}

honorifics::~honorifics()
{
    // no need to delete child widgets, Qt does it all for us
}

void honorifics::languageChange()
{
    retranslateUi(this);
}

void honorifics::sFillList()
{
  q.prepare( "SELECT hnfc_id, hnfc_code "
             "FROM hnfc "
             "ORDER BY hnfc_code;" );
  q.exec();
  _honorifics->populate(q);
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void honorifics::sDelete()
{
  q.prepare( "DELETE FROM hnfc "
             "WHERE (hnfc_id=:hnfc_id);" );
  q.bindValue(":hnfc_id", _honorifics->id());
  q.exec();
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  sFillList();
}

void honorifics::sNew()
{
  ParameterList params;
  params.append("mode", "new");

  honorific newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void honorifics::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("hnfc_id", _honorifics->id());

  honorific newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void honorifics::sView()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("hnfc_id", _honorifics->id());

  honorific newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void honorifics::sPopulateMenu( QMenu *pMenu)
{
  QAction *menuItem;

  menuItem = pMenu->addAction(tr("Edit..."), this, SLOT(sEdit()));
  if (!_privileges->check("MaintainTitles"))
    menuItem->setEnabled(false);

  pMenu->addAction(tr("View..."), this, SLOT(sView()));

  menuItem = pMenu->addAction(tr("Delete"), this, SLOT(sDelete()));
  if (!_privileges->check("MaintainTitles"))
    menuItem->setEnabled(false);
}

void honorifics::sPrint()
{
  orReport report("TitleList");
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}
