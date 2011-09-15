/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "contactMerge.h"
#include "contact.h"

#include <QMessageBox>
#include <QSqlError>

#include <metasql.h>
#include "mqlutil.h"

contactMerge::contactMerge(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  connect(_close,		  SIGNAL(clicked()), this, SLOT(close()));
  connect(_mode,                  SIGNAL(currentIndexChanged(int)), this, SLOT(sHandleMode()));
  connect(_process,	          SIGNAL(clicked()), this, SLOT(sProcess()));
  connect(_query,	          SIGNAL(clicked()), this, SLOT(sFillList()));
  connect(_cntct,    SIGNAL(populateMenu(QMenu *, QTreeWidgetItem *)), this, SLOT(sPopulateCntctMenu(QMenu *)));
  connect(_srccntct, SIGNAL(populateMenu(QMenu *, QTreeWidgetItem *, int)), this, SLOT(sPopulateSrcMenu(QMenu *,QTreeWidgetItem *, int)));
  connect(_cntct,    SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)), this, SLOT(sCntctEdit()));
  connect(_srccntct, SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)), this, SLOT(sSrcCntctEdit()));
  connect(_target,                SIGNAL(valid(bool)), this, SLOT(sHandleProcess()));

  _cntct->addColumn(tr("Contact#"),       100, Qt::AlignLeft, true,  "cntct_number");
  _cntct->addColumn(tr("Active"),          50, Qt::AlignLeft, true,  "cntct_active");
  _cntct->addColumn(tr("Acct.#"),         100, Qt::AlignLeft, false, "crmacct_number");
  _cntct->addColumn(tr("Acct. Name"),     100, Qt::AlignLeft, false, "crmacct_name");
  _cntct->addColumn(tr("Hnrfc"),           50, Qt::AlignLeft, false, "cntct_honorific");
  _cntct->addColumn(tr("First"),           80, Qt::AlignLeft, true,  "cntct_first_name");
  _cntct->addColumn(tr("Middle"),          50, Qt::AlignLeft, false, "cntct_middle");
  _cntct->addColumn(tr("Last"),            -1, Qt::AlignLeft, true,  "cntct_last_name");
  _cntct->addColumn(tr("Suffix"),          80, Qt::AlignLeft, false, "cntct_suffix");
  _cntct->addColumn(tr("Initials"),        80, Qt::AlignLeft, false, "cntct_initials");
  _cntct->addColumn(tr("Phone"),          100, Qt::AlignLeft, true,  "cntct_phone");
  _cntct->addColumn(tr("Alt. Phone"),     100, Qt::AlignLeft, false, "cntct_phone2");
  _cntct->addColumn(tr("Fax"),            100, Qt::AlignLeft, true,  "cntct_fax");
  _cntct->addColumn(tr("Email"),          100, Qt::AlignLeft, true,  "cntct_email");
  _cntct->addColumn(tr("Web"),            100, Qt::AlignLeft, false, "cntct_webaddr");
  _cntct->addColumn(tr("Title"),          100, Qt::AlignLeft, true,  "cntct_title");
  _cntct->addColumn(tr("Owner"),           80, Qt::AlignLeft, true,  "cntct_owner_username");
  _cntct->addColumn(tr("Notes"),          100, Qt::AlignLeft, false, "cntct_notes");
  _cntct->addColumn(tr("Address1"),       100, Qt::AlignLeft, false, "addr_line1");
  _cntct->addColumn(tr("Address2"),       100, Qt::AlignLeft, false, "addr_line2");
  _cntct->addColumn(tr("Address3"),       100, Qt::AlignLeft, false, "addr_line3");
  _cntct->addColumn(tr("City"),           100, Qt::AlignLeft, false, "addr_city");
  _cntct->addColumn(tr("State"),           50, Qt::AlignLeft, false, "addr_state");
  _cntct->addColumn(tr("Postal"),          80, Qt::AlignLeft, false, "addr_postalcode");
  _cntct->addColumn(tr("Country"),        100, Qt::AlignLeft, false, "addr_country");

  _srccntct->addColumn(tr("Contact#"),       100, Qt::AlignLeft, true,  "cntct_number");
  _srccntct->addColumn(tr("Active"),          50, Qt::AlignLeft, true,  "cntct_active");
  _srccntct->addColumn(tr("Acct.#"),         100, Qt::AlignLeft, false, "crmacct_number");
  _srccntct->addColumn(tr("Acct. Name"),     100, Qt::AlignLeft, false, "crmacct_name");
  _srccntct->addColumn(tr("Hnrfc"),           50, Qt::AlignLeft, false, "cntct_honorific");
  _srccntct->addColumn(tr("First"),           80, Qt::AlignLeft, true,  "cntct_first_name");
  _srccntct->addColumn(tr("Middle"),          50, Qt::AlignLeft, false, "cntct_middle");
  _srccntct->addColumn(tr("Last"),            -1, Qt::AlignLeft, true,  "cntct_last_name");
  _srccntct->addColumn(tr("Suffix"),          80, Qt::AlignLeft, false, "cntct_suffix");
  _srccntct->addColumn(tr("Initials"),        80, Qt::AlignLeft, false, "cntct_initials");
  _srccntct->addColumn(tr("Phone"),          100, Qt::AlignLeft, true,  "cntct_phone");
  _srccntct->addColumn(tr("Alt. Phone"),     100, Qt::AlignLeft, false, "cntct_phone2");
  _srccntct->addColumn(tr("Fax"),            100, Qt::AlignLeft, true,  "cntct_fax");
  _srccntct->addColumn(tr("Email"),          100, Qt::AlignLeft, true,  "cntct_email");
  _srccntct->addColumn(tr("Web"),            100, Qt::AlignLeft, false, "cntct_webaddr");
  _srccntct->addColumn(tr("Title"),          100, Qt::AlignLeft, true,  "cntct_title");
  _srccntct->addColumn(tr("Owner"),           80, Qt::AlignLeft, true,  "cntct_owner_username");
  _srccntct->addColumn(tr("Notes"),          100, Qt::AlignLeft, false, "cntct_notes");
  _srccntct->addColumn(tr("Address1"),       100, Qt::AlignLeft, false, "addr_line1");
  _srccntct->addColumn(tr("Address2"),       100, Qt::AlignLeft, false, "addr_line2");
  _srccntct->addColumn(tr("Address3"),       100, Qt::AlignLeft, false, "addr_line3");
  _srccntct->addColumn(tr("City"),           100, Qt::AlignLeft, false, "addr_city");
  _srccntct->addColumn(tr("State"),           50, Qt::AlignLeft, false, "addr_state");
  _srccntct->addColumn(tr("Postal"),          80, Qt::AlignLeft, false, "addr_postalcode");
  _srccntct->addColumn(tr("Country"),        100, Qt::AlignLeft, false, "addr_country");

  _target->setAccountVisible(true);
  _target->setOwnerVisible(true);
  _target->setActiveVisible(false);
  _target->setInitialsVisible(false);
  
  sPopulateSources();
  sPopulateTarget();
}

contactMerge::~contactMerge()
{
    // no need to delete child widgets, Qt does it all for us
}

void contactMerge::languageChange()
{
    retranslateUi(this);
}

void contactMerge::sAdd()
{
  sSelect(false);
}

void contactMerge::sCntctEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("cntct_id", _cntct->id());

  contact newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
  sFillList();
}

void contactMerge::sCntctView()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("cntct_id", _cntct->id());

  contact newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void contactMerge::sCntctDelete()
{
  QString question = tr("The delete action cannot be undone. "
                        "Are you sure you want to proceed?");
  if (QMessageBox::question(this, tr("Delete Contact Merg?"), question,
                QMessageBox::Yes,
                QMessageBox::No | QMessageBox::Default) == QMessageBox::No)
    return;

  MetaSQLQuery mql = mqlLoad("contactmerge", "delete");

  ParameterList params;
  params.append("cntct_id", _cntct->id());
  q = mql.toQuery(params);
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
  sFillList();
}

void contactMerge::sCntctDoubleClicked()
{
  if ((_cntct->altId() == 0) || (_cntct->altId() == 4)) // cNone or cError
  {
    sSelect(false);
  }
  else if ((_cntct->altId() == 1) || (_cntct->altId() == 2)) // cTarget or cSource
  {
    sDeselect(_cntct->id());
  }
}

void contactMerge::sDeselect(int id)
{
  MetaSQLQuery mql = mqlLoad("contactmerge", "deselect");

  ParameterList params;
  params.append("cntct_id", id);
  q = mql.toQuery(params);
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
  sFillList();
}

void contactMerge::sDeselectCntct()
{
  sDeselect(_cntct->id());
}

void contactMerge::sDeselectSource()
{
  sDeselect(_srccntct->id());
}

void contactMerge::sFillList()
{

  if ((_mode->currentIndex() == 0) || (_mode->currentIndex() == 2)) // cMerge or cMergePurge
  {
    ParameterList params;
    params.append("searchText", _search->text());
    params.append("searchContactName", QVariant(_searchContact->isChecked()));
    params.append("searchPhone", QVariant(_searchPhone->isChecked()));
    params.append("searchEmail", QVariant(_searchEmail->isChecked()));
    params.append("searchNumber", QVariant(_searchNumber->isChecked()));
    params.append("searchName", QVariant(_searchName->isChecked()));
    params.append("showInactive", QVariant(_showInactive->isChecked()));
    params.append("ignoreBlanks", QVariant(!_blanks->isChecked()));
    params.append("IndentedDups", QVariant(_showGroup->isChecked()));
    params.append("CheckHnfc", QVariant(_showGroup->isChecked() && _checkHonorific->isChecked()));
    params.append("CheckFirst", QVariant(_showGroup->isChecked() && _checkFirst->isChecked()));
    params.append("CheckMiddle", QVariant(_showGroup->isChecked() && _checkMiddle->isChecked()));
    params.append("CheckLast", QVariant(_showGroup->isChecked() && _checkLast->isChecked()));
    params.append("CheckSuffix", QVariant(_showGroup->isChecked() && _checkSuffix->isChecked()));
    params.append("CheckPhone", QVariant(_showGroup->isChecked() && _checkPhone->isChecked()));
    params.append("CheckEmail", QVariant(_showGroup->isChecked() && _checkEmail->isChecked()));
    MetaSQLQuery mql = mqlLoad("contactmerge", "search");
    XSqlQuery qry = mql.toQuery(params);
    if (qry.lastError().type() != QSqlError::NoError)
    {
      systemError(this, qry.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
    _cntct->populate(qry, true);
  }
  else
  {
    ParameterList params;
    MetaSQLQuery mql = mqlLoad("contactmerge", "merged");
    XSqlQuery qry = mql.toQuery(params);
    if (qry.lastError().type() != QSqlError::NoError)
    {
      systemError(this, qry.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
    _cntct->populate(qry, true);
  }

  _cntct->expandAll();
  sPopulateSources();
  sPopulateTarget();
}

void contactMerge::sHandleMode()
{
  bool canSearch = ((_mode->currentIndex() == 0) || (_mode->currentIndex() == 2)); // cMerge or cMergePurge
  _searchLit->setVisible(canSearch);
  _search->setVisible(canSearch);
  _showGroup->setVisible(canSearch);
  _searchGroup->setVisible(canSearch);
  sHandleProcess();
  _process->setText(_mode->currentText());
  _tab->setTabEnabled(_tab->indexOf(_selTab), canSearch);
  if (canSearch)
    _cntct->clear();
  else
    sFillList();
}

void contactMerge::sHandleProcess()
{
  bool valid = (_target->isValid());
  _process->setEnabled((valid &&
                       (_mode->currentIndex() == 0 || //cMerge
                        _mode->currentIndex() == 2))  || //cMergePurge
                       (_mode->currentIndex() == 3 || //cMerged
                        _mode->currentIndex() == 1)); //cPurge
}

void contactMerge::sPopulateCntctMenu(QMenu *pMenu)
{
  if (_cntct->id() == -1)
    return;

  QAction *menuItem;

  if (_cntct->altId() == 0 || // cNone
      _cntct->altId() == 4)  // cError
  {
    menuItem = pMenu->addAction(tr("Set as Source..."), this, SLOT(sAdd()));
  }

  if (_cntct->altId() != 1) // cTarget
  {
    menuItem = pMenu->addAction(tr("Set as Target..."), this, SLOT(sSetTarget()));
  }

  if (_cntct->altId() == 1 || // cTarget
      _cntct->altId() == 2)  // cSource
  {
    menuItem = pMenu->addAction(tr("Deselect..."), this, SLOT(sDeselectCntct()));
  }

  if (_cntct->altId() == 3) // cMerged
  {
    menuItem = pMenu->addAction(tr("Restore..."), this, SLOT(sRestore()));
    menuItem = pMenu->addAction(tr("Purge..."), this, SLOT(sPurge()));
  }

  pMenu->addSeparator();

  menuItem = pMenu->addAction(tr("Edit..."), this, SLOT(sCntctEdit()));
  menuItem->setEnabled(_privileges->check("MaintainContacts"));

  menuItem = pMenu->addAction(tr("View..."), this, SLOT(sCntctView()));
  menuItem->setEnabled(_privileges->check("MaintainContacts") || _privileges->check("ViewContacts"));

  if (_cntct->altId() != 3) // cMerged
  {
    // Check to see if this contact is used, if not add delete action
    ParameterList params;
    params.append("cntct_id", _cntct->id());
    MetaSQLQuery mql = mqlLoad("contactmerge", "contactused");
    q = mql.toQuery(params);
    if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
    if (q.first() && !q.value("used").toBool())
    {
        menuItem = pMenu->addAction(tr("Delete"), this, SLOT(sCntctDelete()));
        menuItem->setEnabled(_privileges->check("MaintainContacts"));
    }
  }
}

void contactMerge::sPopulateSrcMenu(QMenu *pMenu, QTreeWidgetItem *pItem, int pCol)
{
  QAction *menuItem;
  QString col = "";
  QString menuStr;

  if (pCol == 2 || pCol == 3)
    col = tr("CRM Account");
  else if (pCol == 4)
    col = tr("Honorific");
  else if (pCol == 5)
    col = tr("First Name");
  else if (pCol == 6)
    col = tr("Middle Initial");
  else if (pCol == 7)
    col = tr("Last Name");
  else if (pCol == 8)
    col = tr("Suffix");
  else if (pCol == 9)
    col = tr("Initials");
  else if (pCol == 10)
    col = tr("Phone");
  else if (pCol == 11)
    col = tr("Alt. Phone");
  else if (pCol == 12)
    col = tr("Fax");
  else if (pCol == 13)
    col = tr("Email");
  else if (pCol == 14)
    col = tr("Web Address");
  else if (pCol == 15)
    col = tr("Title");
  else if (pCol == 16)
    col = tr("Owner");
  else if (pCol == 17)
    col = tr("Notes");
  else if (pCol >= 18)
    col = tr("Address");

  menuStr = tr("Merge ") + col + tr(" to target");
  _selectCol = pCol;

  menuItem = pMenu->addAction(tr("Deselect"), this, SLOT(sDeselectSource()));

  if (col.length() > 0)
    menuItem = pMenu->addAction(menuStr, this, SLOT(sSelectCol()));

  pMenu->addSeparator();

  menuItem = pMenu->addAction(tr("Edit..."), this, SLOT(sCntctEdit()));
  menuItem->setEnabled(_privileges->check("MaintainContacts"));

  menuItem = pMenu->addAction(tr("View..."), this, SLOT(sCntctView()));
  menuItem->setEnabled(_privileges->check("MaintainContacts") || _privileges->check("ViewContacts"));
}

void contactMerge::sPopulateSources()
{
  ParameterList params;
  params.append("target", QVariant(false));

  MetaSQLQuery mql = mqlLoad("contactmerge", "populate");
  q = mql.toQuery(params);
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
  _srccntct->populate(q);
}

void contactMerge::sPopulateTarget()
{
  QString grpTitle = tr("Target Contact");
  ParameterList params;
  params.append("target", QVariant(true));

  MetaSQLQuery mql = mqlLoad("contactmerge", "populate");
  q = mql.toQuery(params);
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  if (q.first())
  {
    _target->setId(q.value("cntct_id").toInt());
    _targetGroup->setTitle(grpTitle + " (#" + q.value("cntct_number").toString() + ")");
  }
  else
  {
    _target->setId(-1);
    _targetGroup->setTitle(grpTitle);
  }

  sHandleProcess();
}

void contactMerge::sProcess()
{
  ParameterList params;
  QString qry;

  if ((_mode->currentIndex() == 0) || (_mode->currentIndex() == 2)) // cMerge or cMergePurge
  {
    if (_mode->currentIndex() == 2 && !purgeConfirm()) // cMergePurge
      return;
    qry = "merge";
    if  (_mode->currentIndex() == 2)
      params.append("purge", true);
    else
      params.append("purge", false);
  }
  else if (_mode->currentIndex() == 1) // cPurge
  {
    if (!purgeConfirm())
      return;
    qry = "purge";
  }
  else
  {
    qry = "restore";
  }

  MetaSQLQuery mql = mqlLoad("contactmerge", qry);
  q = mql.toQuery(params);
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
  sFillList();
}

void contactMerge::sPurge()
{
  if (!purgeConfirm())
    return;

  MetaSQLQuery mql = mqlLoad("contactmerge", "purge");

  ParameterList params;
  params.append("cntct_id", _cntct->id());
  q = mql.toQuery(params);
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
  sFillList();
}

bool contactMerge::purgeConfirm()
{
  QString question = tr("The purge action cannot be undone. "
                        "Are you sure you want to proceed?");
  if (QMessageBox::question(this, tr("Purge Contact Merge?"), question,
                QMessageBox::Yes,
                QMessageBox::No | QMessageBox::Default) == QMessageBox::No)
    return false;

  return true;
}

void contactMerge::sRestore()
{
  MetaSQLQuery mql = mqlLoad("contactmerge", "restore");

  ParameterList params;
  params.append("cntct_id", _cntct->id());
  q = mql.toQuery(params);
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
  sFillList();
}

void contactMerge::sSelect(bool target)
{
  MetaSQLQuery mql = mqlLoad("contactmerge", "select");

  ParameterList params;
  params.append("cntct_id", _cntct->id());
  params.append("target", QVariant(target));
  q = mql.toQuery(params);
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
  sFillList();
}

void contactMerge::sSelectCol()
{
  MetaSQLQuery mql = mqlLoad("contactmerge", "selectcol");

  ParameterList params;
  params.append("cntct_id", _srccntct->id());
  params.append("col_number", _selectCol);
  q = mql.toQuery(params);
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
  sPopulateSources();
}

void contactMerge::sSetTarget()
{
  sSelect(true);
}

void contactMerge::sSrcCntctEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("cntct_id", _srccntct->id());

  contact newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
  sFillList();
}

void contactMerge::sSrcCntctView()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("cntct_id", _srccntct->id());

  contact newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
  sFillList();
}


