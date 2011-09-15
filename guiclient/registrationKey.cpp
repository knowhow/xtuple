/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "registrationKey.h"

#include <QMessageBox>

#include "xtupleproductkey.h"

registrationKey::registrationKey(QWidget* parent, const char * name, Qt::WFlags fl)
  : XAbstractConfigure(parent, fl)
{
  setupUi(this);

  if (name)
    setObjectName(name);

  connect(_key, SIGNAL(editingFinished()), this, SLOT(sCheck()));

  _key->setText(_metrics->value("RegistrationKey"));
  sCheck();
}

void registrationKey::languageChange()
{
  retranslateUi(this);
}

bool registrationKey::sSave()
{
  XTupleProductKey pk(_key->text());
  if(pk.valid())
    _metrics->set("RegistrationKey", _key->text());
  else
  {
    QMessageBox::critical(this, tr("Invalid Registration Key"),
                          tr("<p>The Registration Key you have entered "
                             "does not appear to be valid."));
    return false;
  }
  return true;
}

void registrationKey::sCheck()
{
  XTupleProductKey pk(_key->text());
  if(pk.valid())
  {
    _customer->setText(pk.customerId());
    _expirationDate->setText(pk.expiration().toString());
    if(pk.users() == 0)
      _users->setText(tr("Unlimited"));
    else
      _users->setText(QString("%1").arg(pk.users()));
  }
  else
  {
    _customer->setText(tr("Unknown"));
    _expirationDate->setText(tr("Unknown"));
    _users->setText(tr("Unknown"));
  }
}

