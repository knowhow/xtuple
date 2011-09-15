/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "postPurchaseOrder.h"

#include <QVariant>

postPurchaseOrder::postPurchaseOrder(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_close,   SIGNAL(clicked()), this,     SLOT(reject()));
  connect(_po,    SIGNAL(valid(bool)), _release, SLOT(setEnabled(bool)));
  connect(_release, SIGNAL(clicked()), this,     SLOT(sRelease()));

  _captive = false;
}

postPurchaseOrder::~postPurchaseOrder()
{
  // no need to delete child widgets, Qt does it all for us
}

void postPurchaseOrder::languageChange()
{
  retranslateUi(this);
}

enum SetResponse postPurchaseOrder::set(ParameterList &pParams)
{
  XDialog::set(pParams);
  _captive = true;

  QVariant param;
  bool     valid;

  param = pParams.value("pohead_id", &valid);
  if (valid)
  {
    _po->setId(param.toInt());
    _release->setFocus();
  }

  return NoError;
}

void postPurchaseOrder::sRelease()
{
  q.prepare("SELECT releasePurchaseOrder(:pohead_id) AS result;");
  q.bindValue(":pohead_id", _po->id());
  q.exec();

  omfgThis->sPurchaseOrdersUpdated(_po->id(), true);

  if (_captive)
    accept();
  else
  {
    _close->setText(tr("&Close"));

    _po->setId(-1);
    _po->setFocus();
  }
}
