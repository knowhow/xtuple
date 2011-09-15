/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "postCountSlips.h"

#include <QVariant>
#include <QMessageBox>

postCountSlips::postCountSlips(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
  : XDialog(parent, name, modal, fl)
{
  setupUi(this);


  // signals and slots connections
  connect(_close, SIGNAL(clicked()), this, SLOT(reject()));
  connect(_post, SIGNAL(clicked()), this, SLOT(sPost()));
}

postCountSlips::~postCountSlips()
{
  // no need to delete child widgets, Qt does it all for us
}

void postCountSlips::languageChange()
{
  retranslateUi(this);
}

void postCountSlips::sPost()
{
  QString sql( "SELECT postCountSlip(cntslip_id) "
               "FROM cntslip, invcnt, itemsite "
               "WHERE ((NOT cntslip_posted)"
               " AND (cntslip_cnttag_id=invcnt_id)"
               " AND (invcnt_itemsite_id=itemsite_id)" );

  if (_warehouse->isSelected())
    sql += " AND (itemsite_warehous_id=:warehous_id)";

  sql += ");";

  q.prepare(sql);
  _warehouse->bindValue(q);
  q.exec();

  if(!q.first())
    QMessageBox::information(this, tr("No Count Slips Posted"),
      tr("No count slips were posted! Either there were no count slips available to\n"
         "be posted or an error occurred trying to post the count slips.") );
  accept();
}
