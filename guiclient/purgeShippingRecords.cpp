/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "purgeShippingRecords.h"

#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include "storedProcErrorLookup.h"

purgeShippingRecords::purgeShippingRecords(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_purge, SIGNAL(clicked()), this, SLOT(sPurge()));
}

purgeShippingRecords::~purgeShippingRecords()
{
  // no need to delete child widgets, Qt does it all for us
}

void purgeShippingRecords::languageChange()
{
  retranslateUi(this);
}

void purgeShippingRecords::sPurge()
{
  if (QMessageBox::question(this, tr("Purge Shipping Records"),
                            tr("<p>You may not re-print or view Bill of Lading "
			       "information for a shipment after you delete "
			       "its Shipping Records.<p>Are you sure you want "
			       "to delete the selected Shipping Records?." ),
			    QMessageBox::Yes,
			    QMessageBox::No | QMessageBox::Default) == QMessageBox::Yes)
  {
    q.prepare("SELECT purgeShipments(:cutOffDate) AS result;");
    q.bindValue(":cutOffDate", _cutOffDate->date());
    q.exec();
    if (q.first())
    {
      int result = q.value("result").toInt();
      if (result < 0)
      {
	systemError(this, storedProcErrorLookup("purgeShipments", result), __FILE__, __LINE__);
	return;
      }
    }
    else if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }

  accept();
}
