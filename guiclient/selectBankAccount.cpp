/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "selectBankAccount.h"

#include <QVariant>

selectBankAccount::selectBankAccount(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
  : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  // signals and slots connections
  connect(_close, SIGNAL(clicked()), this, SLOT(sClose()));
  connect(_select, SIGNAL(clicked()), this, SLOT(sSelect()));

  _bankaccnt->setAllowNull(TRUE);
}

selectBankAccount::~selectBankAccount()
{
  // no need to delete child widgets, Qt does it all for us
}

void selectBankAccount::languageChange()
{
  retranslateUi(this);
}

enum SetResponse selectBankAccount::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("type", &valid);
  if (valid)
    _type = param.toString();
  else
    _type = "A";

  param = pParams.value("bankaccnt_id", &valid);
  if (valid)
    _bankaccntid = param.toInt();
  else
    _bankaccntid = -1;

  populate();

  return NoError;
}

void selectBankAccount::sSelect()
{
  done(_bankaccnt->id());
}

void selectBankAccount::sClose()
{
  done(-1);
}

void selectBankAccount::populate()
{
  if (_type == "A")
    _bankaccnt->populate( "SELECT bankaccnt_id, (bankaccnt_name || '-' || bankaccnt_descrip) "
                          "FROM bankaccnt "
                          "ORDER BY bankaccnt_name;" );
  else if (_type == "R")
    _bankaccnt->setType(XComboBox::ARBankAccounts);
  else if (_type == "P")
    _bankaccnt->setType(XComboBox::APBankAccounts);

  if (_bankaccntid != -1)
    _bankaccnt->setId(_bankaccntid);
}

