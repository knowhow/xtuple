/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "standardJournalGroup.h"

#include <QVariant>
#include <QMessageBox>
#include "standardJournalGroupItem.h"

standardJournalGroup::standardJournalGroup(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
  : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  _stdjrnlgrpid = -1;

  // signals and slots connections
  connect(_new, SIGNAL(clicked()), this, SLOT(sNew()));
  connect(_delete, SIGNAL(clicked()), this, SLOT(sDelete()));
  connect(_save, SIGNAL(clicked()), this, SLOT(sSave()));
  connect(_close, SIGNAL(clicked()), this, SLOT(sClose()));
  connect(_name, SIGNAL(lostFocus()), this, SLOT(sCheck()));
  connect(_edit, SIGNAL(clicked()), this, SLOT(sEdit()));
  connect(_view, SIGNAL(clicked()), this, SLOT(sView()));
  connect(_showExpired, SIGNAL(toggled(bool)), this, SLOT(sFillList()));
  connect(_showFuture, SIGNAL(toggled(bool)), this, SLOT(sFillList()));
  connect(_stdjrnlgrpitem, SIGNAL(valid(bool)), _view, SLOT(setEnabled(bool)));

  setAcceptDrops(TRUE);

  _stdjrnlgrpitem->addColumn(tr("Name"),        _itemColumn,  Qt::AlignLeft,   true,  "stdjrnl_name"   );
  _stdjrnlgrpitem->addColumn(tr("Description"), -1,           Qt::AlignLeft,   true,  "stdjrnl_descrip"   );
  _stdjrnlgrpitem->addColumn(tr("To Apply"),    _dateColumn,  Qt::AlignRight,  true,  "toapply"  );
  _stdjrnlgrpitem->addColumn(tr("Applied"),     _dateColumn,  Qt::AlignRight,  true,  "stdjrnlgrpitem_applied"  );
  _stdjrnlgrpitem->addColumn(tr("Effective"),   _dateColumn,  Qt::AlignCenter, true,  "stdjrnlgrpitem_effective" );
  _stdjrnlgrpitem->addColumn(tr("Expires"),     _dateColumn,  Qt::AlignCenter, true,  "stdjrnlgrpitem_expires" );
}

standardJournalGroup::~standardJournalGroup()
{
  // no need to delete child widgets, Qt does it all for us
}

void standardJournalGroup::languageChange()
{
  retranslateUi(this);
}

enum SetResponse standardJournalGroup::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("stdjrnlgrp_id", &valid);
  if (valid)
  {
    _stdjrnlgrpid = param.toInt();
    populate();
  }

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      _mode = cNew;

      q.exec("SELECT NEXTVAL('stdjrnlgrp_stdjrnlgrp_id_seq') AS _stdjrnlgrp_id;");
      if (q.first())
        _stdjrnlgrpid = q.value("_stdjrnlgrp_id").toInt();
//  ToDo

      connect(_stdjrnlgrpitem, SIGNAL(valid(bool)), _edit, SLOT(setEnabled(bool)));
      connect(_stdjrnlgrpitem, SIGNAL(valid(bool)), _delete, SLOT(setEnabled(bool)));
      connect(_stdjrnlgrpitem, SIGNAL(itemSelected(int)), _edit, SLOT(animateClick()));

      _name->setFocus();
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;

      connect(_stdjrnlgrpitem, SIGNAL(valid(bool)), _edit, SLOT(setEnabled(bool)));
      connect(_stdjrnlgrpitem, SIGNAL(valid(bool)), _delete, SLOT(setEnabled(bool)));
      connect(_stdjrnlgrpitem, SIGNAL(itemSelected(int)), _edit, SLOT(animateClick()));

      _save->setFocus();
    }
    else if (param.toString() == "view")
    {
      _mode = cView;

      _name->setEnabled(FALSE);
      _descrip->setEnabled(FALSE);
      _close->setText(tr("&Close"));
      _save->hide();

      _new->setEnabled(FALSE);
      connect(_stdjrnlgrpitem, SIGNAL(itemSelected(int)), _view, SLOT(animateClick()));

      _close->setFocus();
    }
  }

  return NoError;
}

void standardJournalGroup::sCheck()
{
  _name->setText(_name->text().trimmed());
  if ((_mode == cNew) && (_name->text().length() != 0))
  {
    q.prepare( "SELECT stdjrnlgrp_id"
               "  FROM stdjrnlgrp"
               " WHERE((UPPER(stdjrnlgrp_name)=UPPER(:stdjrnlgrp_name))"
               "   AND (stdjrnlgrp_id != :stdjrnlgrp_id));" );
    q.bindValue(":stdjrnlgrp_name", _name->text());
    q.bindValue(":stdjrnlgrp_id", _stdjrnlgrpid);
    q.exec();
    if (q.first())
    {
      _stdjrnlgrpid = q.value("stdjrnlgrp_id").toInt();
      _mode = cEdit;
      populate();

      _name->setEnabled(FALSE);
    }
  }
}

void standardJournalGroup::sClose()
{
  if (_mode == cNew)
  {
    q.prepare( "DELETE FROM stdjrnlgrpitem "
               "WHERE (stdjrnlgrpitem_stdjrnlgrp_id=:stdjrnlgrp_id);" 
               "DELETE FROM stdjrnlgrp "
               "WHERE (stdjrnlgrp_id=:stdjrnlgrp_id);" );
    q.bindValue(":stdjrnlgrp_id", _stdjrnlgrpid);
    q.exec();
  }

  reject();
}

void standardJournalGroup::sSave()
{
  if (_name->text().trimmed().length() == 0)
  {
    QMessageBox::warning( this, tr("Cannot Save Standard Journal Group"),
                          tr("You must enter a Name for this Standard Journal Group before you may save it.") );
    _name->setFocus();
    return;
  }

  q.prepare( "SELECT stdjrnlgrp_id"
             "  FROM stdjrnlgrp"
             " WHERE((UPPER(stdjrnlgrp_name)=UPPER(:stdjrnlgrp_name))"
             "   AND (stdjrnlgrp_id != :stdjrnlgrp_id));" );
  q.bindValue(":stdjrnlgrp_name", _name->text());
  q.bindValue(":stdjrnlgrp_id", _stdjrnlgrpid);
  q.exec();
  if (q.first())
  {
    QMessageBox::warning(this, tr("Cannot Save Standard Journal Group"),
                         tr("The Name you have entered for this Standard Journal Group is already in use. "
                            "Please enter in a different Name for this Standard Journal Group."));
    _name->setFocus();
    return;
  }

  if (_mode == cNew)
    q.prepare( "INSERT INTO stdjrnlgrp "
               "(stdjrnlgrp_id, stdjrnlgrp_name, stdjrnlgrp_descrip) "
               "VALUES "
               "(:stdjrnlgrp_id, :stdjrnlgrp_name, :stdjrnlgrp_descrip);" );
  else
    q.prepare( "UPDATE stdjrnlgrp "
               "SET stdjrnlgrp_name=:stdjrnlgrp_name, stdjrnlgrp_descrip=:stdjrnlgrp_descrip "
               "WHERE (stdjrnlgrp_id=:stdjrnlgrp_id);" );

  q.bindValue(":stdjrnlgrp_id", _stdjrnlgrpid);
  q.bindValue(":stdjrnlgrp_name", _name->text());
  q.bindValue(":stdjrnlgrp_descrip", _descrip->text().trimmed());
  q.exec();

  done(_stdjrnlgrpid);
}

void standardJournalGroup::sDelete()
{
  q.prepare( "DELETE FROM stdjrnlgrpitem "
             "WHERE (stdjrnlgrpitem_id=:stdjrnlgrpitem_id);" );
  q.bindValue(":stdjrnlgrpitem_id", _stdjrnlgrpitem->id());
  q.exec();

  sFillList();
}

void standardJournalGroup::sNew()
{
  ParameterList params;
  params.append("mode", "new");
  params.append("stdjrnlgrp_id", _stdjrnlgrpid);

  standardJournalGroupItem newdlg(this, "", TRUE);
  newdlg.set(params);
  
  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void standardJournalGroup::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("stdjrnlgrpitem_id", _stdjrnlgrpitem->id());

  standardJournalGroupItem newdlg(this, "", TRUE);
  newdlg.set(params);
  
  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void standardJournalGroup::sView()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("stdjrnlgrpitem_id", _stdjrnlgrpitem->id());

  standardJournalGroupItem newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void standardJournalGroup::sFillList()
{
  QString sql( "SELECT stdjrnlgrpitem_id, stdjrnl_name, stdjrnl_descrip,"
               "       CASE WHEN (stdjrnlgrpitem_toapply=-1) THEN :always"
               "            ELSE TEXT(stdjrnlgrpitem_toapply)"
               "       END AS toapply,"
               "       stdjrnlgrpitem_applied,"
               "       stdjrnlgrpitem_effective, stdjrnlgrpitem_expires,"
               "       CASE WHEN (COALESCE(stdjrnlgrpitem_effective, startOfTime())=startOfTime()) THEN 'Always' END AS stdjrnlgrpitem_effective_qtdisplayrole,"
               "       CASE WHEN (COALESCE(stdjrnlgrpitem_expires, endOfTime())=endOfTime()) THEN 'Never' END AS stdjrnlgrpitem_expires_qtdisplayrole "
               "FROM stdjrnlgrpitem, stdjrnl "
               "WHERE ((stdjrnlgrpitem_stdjrnl_id=stdjrnl_id)" );

  if (!_showExpired->isChecked())
    sql += " AND (stdjrnlgrpitem_expires > CURRENT_DATE)"
           " AND ( (stdjrnlgrpitem_applied < stdjrnlgrpitem_toapply) OR (stdjrnlgrpitem_toapply = -1) )";

  if (!_showFuture->isChecked())
    sql += " AND (stdjrnlgrpitem_effective <= CURRENT_DATE)";


  sql += " AND (stdjrnlgrpitem_stdjrnlgrp_id=:stdjrnlgrp_id) ) "
         "ORDER BY stdjrnl_name;";

  q.prepare(sql);
  q.bindValue(":always", tr("Always"));
  q.bindValue(":stdjrnlgrp_id", _stdjrnlgrpid);
  q.exec();
  _stdjrnlgrpitem->populate(q);
}

void standardJournalGroup::populate()
{
  q.prepare( "SELECT stdjrnlgrp_name, stdjrnlgrp_descrip "
             "FROM stdjrnlgrp "
             "WHERE (stdjrnlgrp_id=:stdjrnlgrp_id);" );
  q.bindValue(":stdjrnlgrp_id", _stdjrnlgrpid);
  q.exec();
  if (q.first())
  {
    _name->setText(q.value("stdjrnlgrp_name").toString());
    _descrip->setText(q.value("stdjrnlgrp_descrip").toString());

    sFillList();
  }
}

