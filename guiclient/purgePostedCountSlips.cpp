/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "purgePostedCountSlips.h"

purgePostedCountSlips::purgePostedCountSlips(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
  : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  // signals and slots connections
  connect(_close, SIGNAL(clicked()), this, SLOT(reject()));
  connect(_purge, SIGNAL(clicked()), this, SLOT(sPurge()));
}

purgePostedCountSlips::~purgePostedCountSlips()
{
  // no need to delete child widgets, Qt does it all for us
}

void purgePostedCountSlips::languageChange()
{
  retranslateUi(this);
}

void purgePostedCountSlips::sPurge()
{
  if (_cutOffDate->isValid())
  {
    q.prepare("SELECT purgePostedCountSlips(:cutOffDate, :warehous_id) AS _result;");
    q.bindValue(":cutOffDate", _cutOffDate->date());
    q.bindValue(":warehous_id", ((_warehouse->isSelected()) ? _warehouse->id() : -1));
    q.exec();

    _cutOffDate->clear();

    _close->setText(tr("&Close"));
  }
}

