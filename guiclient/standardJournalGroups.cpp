/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "standardJournalGroups.h"

#include <QVariant>
#include <QMessageBox>
#include <QMenu>
#include <openreports.h>
#include <parameter.h>
#include "postStandardJournalGroup.h"
#include "standardJournalGroup.h"

standardJournalGroups::standardJournalGroups(QWidget* parent, const char* name, Qt::WFlags fl)
  : XWidget(parent, name, fl)
{
  setupUi(this);

  // signals and slots connections
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_new, SIGNAL(clicked()), this, SLOT(sNew()));
  connect(_edit, SIGNAL(clicked()), this, SLOT(sEdit()));
  connect(_delete, SIGNAL(clicked()), this, SLOT(sDelete()));
  connect(_stdjrnlgrp, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*)));
  connect(_close, SIGNAL(clicked()), this, SLOT(close()));
  connect(_view, SIGNAL(clicked()), this, SLOT(sView()));
  connect(_stdjrnlgrp, SIGNAL(valid(bool)), _view, SLOT(setEnabled(bool)));
  connect(_post, SIGNAL(clicked()), this, SLOT(sPost()));

  if (_privileges->check("MaintainStandardJournalGroups"))
  {
    connect(_stdjrnlgrp, SIGNAL(valid(bool)), _edit, SLOT(setEnabled(bool)));
    connect(_stdjrnlgrp, SIGNAL(valid(bool)), _delete, SLOT(setEnabled(bool)));
    connect(_stdjrnlgrp, SIGNAL(itemSelected(int)), _edit, SLOT(animateClick()));
  }
  else
    _new->setEnabled(FALSE);

  if (_privileges->check("PostStandardJournalGroups"))
    connect(_stdjrnlgrp, SIGNAL(valid(bool)), _post, SLOT(setEnabled(bool)));

  _stdjrnlgrp->addColumn(tr("Name"),        _itemColumn, Qt::AlignLeft,   true,  "stdjrnlgrp_name" );
  _stdjrnlgrp->addColumn(tr("Description"), -1,          Qt::AlignLeft,   true,  "stdjrnlgrp_descrip" );

  sFillList();
}

standardJournalGroups::~standardJournalGroups()
{
  // no need to delete child widgets, Qt does it all for us
}

void standardJournalGroups::languageChange()
{
  retranslateUi(this);
}

void standardJournalGroups::sNew()
{
  ParameterList params;
  params.append("mode", "new");

  standardJournalGroup newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void standardJournalGroups::sPost()
{
  ParameterList params;
  params.append("stdjrnlgrp_id", _stdjrnlgrp->id());

  postStandardJournalGroup newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void standardJournalGroups::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("stdjrnlgrp_id", _stdjrnlgrp->id());

  standardJournalGroup newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void standardJournalGroups::sView()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("stdjrnlgrp_id", _stdjrnlgrp->id());

  standardJournalGroup newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void standardJournalGroups::sDelete()
{
  q.prepare("SELECT deleteStandardJournalGroup(:stdjrnlgrp_id);");
  q.bindValue(":stdjrnlgrp_id", _stdjrnlgrp->id());
  q.exec();

  sFillList();
}

void standardJournalGroups::sFillList()
{
  _stdjrnlgrp->populate( "SELECT stdjrnlgrp_id, stdjrnlgrp_name, stdjrnlgrp_descrip "
                         "FROM stdjrnlgrp "
                         "ORDER BY stdjrnlgrp_name;" );
}

void standardJournalGroups::sPopulateMenu(QMenu *)
{

}

void standardJournalGroups::sPrint()
{
  orReport report("StandardJournalGroupMasterList");
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}


