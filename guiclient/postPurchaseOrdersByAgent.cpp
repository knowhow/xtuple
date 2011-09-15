/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "postPurchaseOrdersByAgent.h"

#include <QVariant>
#include <QMessageBox>

postPurchaseOrdersByAgent::postPurchaseOrdersByAgent(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_release, SIGNAL(clicked()), this, SLOT(sRelease()));

  _agent->setText(omfgThis->username());
}

postPurchaseOrdersByAgent::~postPurchaseOrdersByAgent()
{
  // no need to delete child widgets, Qt does it all for us
}

void postPurchaseOrdersByAgent::languageChange()
{
  retranslateUi(this);
}

void postPurchaseOrdersByAgent::sRelease()
{
  static bool run = false;
  if (run)
    return;
  run = true;

  QString sql("SELECT COUNT(releasePurchaseOrder(pohead_id)) AS result "
              "  FROM ( SELECT pohead_id, pohead_agent_username "
              "           FROM pohead, poitem "
              "          WHERE ( (poitem_pohead_id=pohead_id)"
              "            AND   (poitem_status='U') ) ) AS data "
              " WHERE ( (checkPOSitePrivs(pohead_id))");
  if(_selectedAgent->isChecked())
    sql +=    "   AND   (pohead_agent_username=:username)";
  sql += ")";
  q.prepare(sql);
  q.bindValue(":username", _agent->currentText());
  q.exec();
  if (q.first())
  {
    QMessageBox::information( this, tr("Purchase Orders Released"),
                              tr("%1 Purchase Orders have been released.")
                              .arg(q.value("result").toString()) );

    omfgThis->sPurchaseOrdersUpdated(-1, TRUE);
  }

  accept();
  run = false;
}
