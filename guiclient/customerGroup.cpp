/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "customerGroup.h"

#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include "crmacctcluster.h"
#include "customerGroup.h"

customerGroup::customerGroup(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_new, SIGNAL(clicked()), this, SLOT(sNew()));
  connect(_delete, SIGNAL(clicked()), this, SLOT(sDelete()));
  connect(_save, SIGNAL(clicked()), this, SLOT(sSave()));
  connect(_close, SIGNAL(clicked()), this, SLOT(sClose()));
  connect(_name, SIGNAL(lostFocus()), this, SLOT(sCheck()));

  _custgrpitem->addColumn(tr("Number"), _itemColumn, Qt::AlignLeft, true, "cust_number");
  _custgrpitem->addColumn(tr("Name"),   -1,          Qt::AlignLeft, true, "cust_name");
}

customerGroup::~customerGroup()
{
  // no need to delete child widgets, Qt does it all for us
}

void customerGroup::languageChange()
{
  retranslateUi(this);
}

enum SetResponse customerGroup::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("custgrp_id", &valid);
  if (valid)
  {
    _custgrpid = param.toInt();
    populate();
  }

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      _mode = cNew;

      q.exec("SELECT NEXTVAL('custgrp_custgrp_id_seq') AS _custgrp_id;");
      if (q.first())
        _custgrpid = q.value("_custgrp_id").toInt();
      else if (q.lastError().type() != QSqlError::NoError)
      {
	systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
	return UndefinedError;
      }

      connect(_custgrpitem, SIGNAL(valid(bool)), _delete, SLOT(setEnabled(bool)));

      _name->setFocus();
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;

      sFillList();

      connect(_custgrpitem, SIGNAL(valid(bool)), _delete, SLOT(setEnabled(bool)));

      _name->setFocus();
    }
    else if (param.toString() == "view")
    {
      _mode = cView;

      _name->setEnabled(FALSE);
      _descrip->setEnabled(FALSE);
      _new->setEnabled(FALSE);
      _close->setText(tr("&Close"));
      _save->hide();

      sFillList();

      _close->setFocus();
    }
  }

  return NoError;
}

void customerGroup::sCheck()
{
  _name->setText(_name->text().trimmed());
  if ((_mode == cNew) && (_name->text().length()))
  {
    q.prepare( "SELECT custgrp_id "
               "FROM custgrp "
               "WHERE (UPPER(custgrp_name)=UPPER(:custgrp_name));" );
    q.bindValue(":custgrp_name", _name->text());
    q.exec();
    if (q.first())
    {
      _custgrpid = q.value("custgrp_id").toInt();
      _mode = cEdit;
      populate();

      _name->setEnabled(FALSE);
    }
    else if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }
}

void customerGroup::sClose()
{
  if (_mode == cNew)
  {
    q.prepare( "DELETE FROM custgrpitem "
               "WHERE (custgrpitem_custgrp_id=:custgrp_id);"
               "DELETE FROM custgrp "
               "WHERE (custgrp_id=:custgrp_id);" );
    q.bindValue(":custgrp_id", _custgrpid);
    q.exec();
    if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    }
  }

  reject();
}

void customerGroup::sSave()
{
  if(_name->text().trimmed().isEmpty())
  {
    QMessageBox::warning(this, tr("Cannot Save Customer Group"),
      tr("You cannot have an empty name."));
    _name->setFocus();
    return;
  }

  q.prepare("SELECT custgrp_id"
            "  FROM custgrp"
            " WHERE((custgrp_name=:custgrp_name)"
            "   AND (custgrp_id != :custgrp_id))");
  q.bindValue(":custgrp_id", _custgrpid);
  q.bindValue(":custgrp_name", _name->text());
  q.exec();
  if(q.first())
  {
    QMessageBox::warning(this, tr("Cannot Save Customer Group"),
      tr("You cannot have a duplicate name."));
    _name->setFocus();
    return;
  }

  if (_mode == cNew)
    q.prepare( "INSERT INTO custgrp "
               "(custgrp_id, custgrp_name, custgrp_descrip) "
               "VALUES "
               "(:custgrp_id, :custgrp_name, :custgrp_descrip);" );
  else if (_mode == cEdit)
    q.prepare( "UPDATE custgrp "
               "SET custgrp_name=:custgrp_name, custgrp_descrip=:custgrp_descrip "
               "WHERE (custgrp_id=:custgrp_id);" );

  q.bindValue(":custgrp_id", _custgrpid);
  q.bindValue(":custgrp_name", _name->text());
  q.bindValue(":custgrp_descrip", _descrip->text().trimmed());
  q.exec();
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  done(_custgrpid);
}

void customerGroup::sDelete()
{
  q.prepare( "DELETE FROM custgrpitem "
             "WHERE (custgrpitem_id=:custgrpitem_id);" );
  q.bindValue(":custgrpitem_id", _custgrpitem->id());
  q.exec();
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  sFillList();
}

void customerGroup::sNew()
{
  ParameterList params;

  CRMAcctList *newdlg = new CRMAcctList(this);
  newdlg->setSubtype(CRMAcctLineEdit::Cust);

  int custid;
  if ((custid = newdlg->exec()) != XDialog::Rejected)
  {
    q.prepare( "SELECT custgrpitem_id "
               "FROM custgrpitem "
               "WHERE ( (custgrpitem_custgrp_id=:custgrpitem_custgrp_id)"
               " AND (custgrpitem_cust_id=:custgrpitem_cust_id) );" );
    q.bindValue(":custgrpitem_custgrp_id", _custgrpid);
    q.bindValue(":custgrpitem_cust_id", custid);
    q.exec();
    if (q.first())
      return;
    else if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }

    q.prepare( "INSERT INTO custgrpitem "
               "(custgrpitem_custgrp_id, custgrpitem_cust_id) "
               "VALUES "
               "(:custgrpitem_custgrp_id, :custgrpitem_cust_id);" );
    q.bindValue(":custgrpitem_custgrp_id", _custgrpid);
    q.bindValue(":custgrpitem_cust_id", custid);
    q.exec();
    if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }

    sFillList();
  }
}

void customerGroup::sFillList()
{
  q.prepare( "SELECT custgrpitem_id, cust_number, cust_name "
             "FROM custgrpitem, cust "
             "WHERE ( (custgrpitem_cust_id=cust_id) "
             " AND (custgrpitem_custgrp_id=:custgrp_id) ) "
             "ORDER BY cust_number;" );
  q.bindValue(":custgrp_id", _custgrpid);
  q.exec();
  _custgrpitem->populate(q);
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void customerGroup::populate()
{
  q.prepare( "SELECT custgrp_name, custgrp_descrip "
             "FROM custgrp "
             "WHERE (custgrp_id=:custgrp_id);" );
  q.bindValue(":custgrp_id", _custgrpid);
  q.exec();
  if (q.first())
  {
    _name->setText(q.value("custgrp_name").toString());
    _descrip->setText(q.value("custgrp_descrip").toString());
  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}
