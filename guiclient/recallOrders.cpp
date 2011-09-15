/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "recallOrders.h"

#include <QMessageBox>
#include <QSqlError>
//#include <QStatusBar>
#include <QVariant>

#include <metasql.h>
#include <parameter.h>
#include "storedProcErrorLookup.h"

recallOrders::recallOrders(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  connect(_recall,	   SIGNAL(clicked()),	  this, SLOT(sRecall()));
  connect(_showInvoiced, SIGNAL(toggled(bool)), this, SLOT(sFillList()));
  connect(omfgThis, SIGNAL(invoicesUpdated(int, bool)), this, SLOT(sFillList()));

  _showInvoiced->setEnabled(_privileges->check("RecallInvoicedShipment"));
  
  _ship->addColumn(tr("Ship Date"),	_dateColumn,  Qt::AlignCenter, true, "shiphead_shipdate" );
  _ship->addColumn(tr("Order #"),	_orderColumn, Qt::AlignLeft  , true, "number");
  _ship->addColumn(tr("Shipment #"),    _orderColumn, Qt::AlignLeft  , true, "shiphead_number");
  _ship->addColumn(tr("Customer"),      -1,           Qt::AlignLeft  , true, "cohead_billtoname" );
  _ship->addColumn(tr("Invoiced"),	_ynColumn,    Qt::AlignCenter, true, "shipitem_invoiced" );

  sFillList();
}

recallOrders::~recallOrders()
{
  // no need to delete child widgets, Qt does it all for us
}

void recallOrders::languageChange()
{
  retranslateUi(this);
}

void recallOrders::sRecall()
{
  if (!checkSitePrivs(_ship->id()))
    return;

  if (_ship->altId() != -1)
  {    
    int answer = QMessageBox::question(this, tr("Purge Invoice?"),
                            tr("<p>There is an unposted Invoice associated "
                               "this Shipment.  This Invoice will be purged "
                               "as part of the recall process. <p> "
                               "OK to continue? "),
                              QMessageBox::Yes | QMessageBox::Default,
                              QMessageBox::No);
    if (answer == QMessageBox::No)
      return;
  }

  q.prepare("SELECT recallShipment(:shiphead_id) AS result;");
  q.bindValue(":shiphead_id", _ship->id());
  q.exec();
  if (q.first())
  {
    int result = q.value("result").toInt();
    if (result < 0)
    {
      systemError(this, storedProcErrorLookup("recallShipment", result),
		  __FILE__, __LINE__);
      return;
    }
    sFillList();
  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void recallOrders::sFillList()
{
  ParameterList params;

  if (_showInvoiced->isChecked())
    params.append("showInvoiced", true);
  if (_metrics->boolean("MultiWhs"))
    params.append("MultiWhs");

  QString sql = "SELECT DISTINCT shiphead_id, COALESCE(invchead_id, -1) AS invchead_id, shiphead_shipdate, "
                "       cohead_number AS number, shiphead_number, cohead_billtoname, "
                "       shipitem_invoiced "
                "FROM shiphead JOIN shipitem ON (shipitem_shiphead_id=shiphead_id)"
                "              JOIN coitem ON (coitem_id=shipitem_orderitem_id)"
                "              JOIN cohead ON (cohead_id=coitem_cohead_id)"
                "              JOIN itemsite ON (itemsite_id=coitem_itemsite_id)"
                "              JOIN site() ON (warehous_id=itemsite_warehous_id)"
                "              LEFT OUTER JOIN invcitem ON (invcitem_id=shipitem_invcitem_id)"
                "              LEFT OUTER JOIN invchead ON (invchead_id=invcitem_invchead_id) "
                "WHERE ( (shiphead_shipped)"
                "  AND   (shiphead_order_type='SO')"
                "<? if exists(\"showInvoiced\") ?>"
                "  AND   (NOT shipitem_invoiced OR invchead_posted=FALSE)"
                "<? else ?>"
                "  AND   (NOT shipitem_invoiced) "
                "<? endif ?>"
                " ) "
                "<? if exists(\"MultiWhs\") ?>"
                "UNION "
                "SELECT DISTINCT shiphead_id, -1 AS invchead_id, shiphead_shipdate, "
                "       tohead_number AS number, shiphead_number, '' AS cohead_billtoname, "
                "       false AS shipitem_invoiced "
                "FROM shiphead, tohead, toitem "
                "WHERE ((toitem_tohead_id=tohead_id)"
                "  AND  (shiphead_order_id=tohead_id)"
                "  AND  (shiphead_shipped)"
                "  AND  (shiphead_order_type='TO')) "
                "<? endif ?>"
                "ORDER BY shiphead_shipdate DESC, number;" ;
  MetaSQLQuery mql(sql);
  XSqlQuery r = mql.toQuery(params);
  _ship->clear();
  _ship->populate(r, true);
  if (r.lastError().type() != QSqlError::NoError)
  {
    systemError(this, r.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

bool recallOrders::checkSitePrivs(int orderid)
{
  if (_preferences->boolean("selectedSites"))
  {
    XSqlQuery check;
    check.prepare("SELECT checkShipmentSitePrivs(:shipheadid) AS result;");
    check.bindValue(":shipheadid", orderid);
    check.exec();
    if (check.first())
    {
      if (!check.value("result").toBool())
      {
        QMessageBox::critical(this, tr("Access Denied"),
                                       tr("You may not recall this Shipment as it references "
                                       "a Site for which you have not been granted privileges.")) ;
        return false;
      }
    }
  }
  return true;
}
