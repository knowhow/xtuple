/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "uninvoicedShipments.h"

#include <QAction>
#include <QMenu>
#include <QMessageBox>
#include <QVariant>

#include <parameter.h>
#include <openreports.h>
#include "selectOrderForBilling.h"

uninvoicedShipments::uninvoicedShipments(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_close, SIGNAL(clicked()), this, SLOT(close()));
  connect(_showUnselected, SIGNAL(toggled(bool)), this, SLOT(sFillList()));
  connect(_coship, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*)));
  connect(_warehouse, SIGNAL(updated()), this, SLOT(sFillList()));

  _coship->setRootIsDecorated(TRUE);
  _coship->addColumn(tr("Order/Line #"),           _itemColumn, Qt::AlignRight,  true,  "orderline" );
  _coship->addColumn(tr("Cust./Item Number"),      _itemColumn, Qt::AlignLeft,   true,  "custitem"  );
  _coship->addColumn(tr("Cust. Name/Description"), -1,          Qt::AlignLeft,   true,  "custdescrip"  );
  _coship->addColumn(tr("UOM"),                    _uomColumn,  Qt::AlignLeft,   true,  "uom_name"  );
  _coship->addColumn(tr("Shipped"),                _qtyColumn,  Qt::AlignRight,  true,  "shipped" );
  _coship->addColumn(tr("Selected"),               _qtyColumn,  Qt::AlignRight,  true,  "selected" );
  
  connect(omfgThis, SIGNAL(billingSelectionUpdated(int, int)), this, SLOT(sFillList()));

  sFillList();
}

uninvoicedShipments::~uninvoicedShipments()
{
  // no need to delete child widgets, Qt does it all for us
}

void uninvoicedShipments::languageChange()
{
  retranslateUi(this);
}

void uninvoicedShipments::sPrint()
{
  ParameterList params;
  _warehouse->appendValue(params);
  if (_showUnselected->isChecked())
    params.append("showUnselected");
	
  orReport report("UninvoicedShipments", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void uninvoicedShipments::sPopulateMenu(QMenu *menu)
{
  QAction *menuItem;

  menuItem = menu->addAction(tr("Select This Order for Billing..."), this, SLOT(sSelectForBilling()));
  menuItem->setEnabled(_privileges->check("SelectBilling"));
}

void uninvoicedShipments::sSelectForBilling()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("sohead_id", _coship->id());

  selectOrderForBilling *newdlg = new selectOrderForBilling();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void uninvoicedShipments::sFillList()
{
  QString sql( "SELECT cohead_id, coship_id, cohead_number, coitem_linenumber,"
               "       cust_number, cust_name,"
               "       item_number, description,"
               "       uom_name, shipped, selected,"
               "       CASE WHEN (level=0) THEN cohead_number"
               "            ELSE CAST(coitem_linenumber AS TEXT)"
               "       END AS orderline,"
               "       CASE WHEN (level=0) THEN cust_number"
               "            ELSE item_number"
               "       END AS custitem,"
               "       CASE WHEN (level=0) THEN cust_name"
               "            ELSE description"
               "       END AS custdescrip,"
               "       'qty' AS shipped_xtnumericrole,"
               "       'qty' AS selected_xtnumericrole,"
               "       level AS xtindentrole "
               "FROM ( " );
  sql +=       "SELECT cohead_id, -1 AS coship_id, cohead_number, -1 AS coitem_linenumber, 0 AS level,"
               "       cust_number, cust_name,"
               "       '' AS item_number, '' AS description,"
               "       '' AS uom_name,"
               "       COALESCE(SUM(coship_qty), 0) AS shipped,"
               "       COALESCE(SUM(cobill_qty), 0) AS selected "
               "FROM cohead, cust, itemsite, item, warehous, coship, cosmisc, uom,"
               "     coitem LEFT OUTER JOIN"
               "      ( cobill JOIN cobmisc"
               "        ON ( (cobill_cobmisc_id=cobmisc_id) AND (NOT cobmisc_posted) ) )"
               "     ON (cobill_coitem_id=coitem_id) "
               "WHERE ( (cohead_cust_id=cust_id)"
               " AND (coship_coitem_id=coitem_id)"
               " AND (coship_cosmisc_id=cosmisc_id)"
               " AND (coitem_cohead_id=cohead_id)"
               " AND (coitem_itemsite_id=itemsite_id)"
               " AND (coitem_qty_uom_id=uom_id)"
               " AND (itemsite_item_id=item_id)"
               " AND (itemsite_warehous_id=warehous_id)"
               " AND (cosmisc_shipped)"
               " AND (NOT coship_invoiced)";

  if (_warehouse->isSelected())
    sql += " AND (itemsite_warehous_id=:warehous_id)";

  sql += ") "
         "GROUP BY cohead_id, cohead_number, cust_number, cust_name ";

  sql +=       "UNION "
               "SELECT cohead_id, coship_id, cohead_number, coitem_linenumber, 1 AS level,"
               "       cust_number, cust_name,"
               "       item_number, (item_descrip1 || ' ' || item_descrip2) AS description,"
               "       uom_name,"
               "       COALESCE(SUM(coship_qty), 0) AS shipped,"
               "       COALESCE(SUM(cobill_qty), 0) AS selected "
               "FROM cohead, cust, itemsite, item, warehous, coship, cosmisc, uom,"
               "     coitem LEFT OUTER JOIN"
               "      ( cobill JOIN cobmisc"
               "        ON ( (cobill_cobmisc_id=cobmisc_id) AND (NOT cobmisc_posted) ) )"
               "     ON (cobill_coitem_id=coitem_id) "
               "WHERE ( (cohead_cust_id=cust_id)"
               " AND (coship_coitem_id=coitem_id)"
               " AND (coship_cosmisc_id=cosmisc_id)"
               " AND (coitem_cohead_id=cohead_id)"
               " AND (coitem_itemsite_id=itemsite_id)"
               " AND (coitem_qty_uom_id=uom_id)"
               " AND (itemsite_item_id=item_id)"
               " AND (itemsite_warehous_id=warehous_id)"
               " AND (cosmisc_shipped)"
               " AND (NOT coship_invoiced)";

  if (_warehouse->isSelected())
    sql += " AND (itemsite_warehous_id=:warehous_id)";

  sql += ") "
         "GROUP BY cohead_number, cust_number, cust_name,"
         "         coitem_id, coitem_linenumber, item_number,"
         "         item_descrip1, item_descrip2, cohead_id, coship_id, uom_name "
         
         "   ) AS data ";

  if (_showUnselected->isChecked())
    sql += " WHERE (selected = 0) ";

  sql += "ORDER BY cohead_number DESC, level, coitem_linenumber DESC;";

  q.prepare(sql);
  _warehouse->bindValue(q);
  q.exec();
  _coship->populate(q, true);
}
