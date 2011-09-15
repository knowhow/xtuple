/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "releaseTransferOrdersByAgent.h"

#include <QVariant>
#include <QMessageBox>

releaseTransferOrdersByAgent::releaseTransferOrdersByAgent(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_release, SIGNAL(clicked()), this, SLOT(sRelease()));

  _agent->setText(omfgThis->username());
}

releaseTransferOrdersByAgent::~releaseTransferOrdersByAgent()
{
  // no need to delete child widgets, Qt does it all for us
}

void releaseTransferOrdersByAgent::languageChange()
{
  retranslateUi(this);
}

void releaseTransferOrdersByAgent::sRelease()
{
  static bool run = false;
  if (run)
    return;
  run = true;

  QString sql("SELECT COUNT(releaseTransferOrder(tohead_id)) AS result "
              "  FROM ( SELECT DISTINCT tohead_id, tohead_agent_username "
              "           FROM tohead, toitem "
              "          WHERE ( (toitem_tohead_id=tohead_id)"
              "            AND   (toitem_status='U') ) ) AS data "
// TODO: site priv checking for TO's?
//              " WHERE ( (checkTOSitePrivs(tohead_id))");
              " WHERE ( TRUE");
  if(_selectedAgent->isChecked())
    sql +=    "   AND   (tohead_agent_username=:username)";
  sql += ")";
  q.prepare(sql);
  q.bindValue(":username", _agent->currentText());
  q.exec();
  if (q.first())
  {
    QMessageBox::information( this, tr("Transfer Orders Released"),
                              tr("%1 Transfer Orders have been released.")
                              .arg(q.value("result").toString()) );

    omfgThis->sTransferOrdersUpdated(-1);
  }

  accept();
  run = false;
}
