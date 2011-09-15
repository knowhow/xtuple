/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "packingListBatch.h"

#include <QAction>
#include <QMenu>
#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include <metasql.h>
#include <openreports.h>

#include "mqlutil.h"
#include "printPackingList.h"
#include "salesOrder.h"
#include "salesOrderList.h"
#include "storedProcErrorLookup.h"
#include "transferOrder.h"
#include "transferOrderList.h"

packingListBatch::packingListBatch(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  connect(_add,		     SIGNAL(clicked()), this, SLOT(sAddSO()));
  connect(_addTO,	     SIGNAL(clicked()), this, SLOT(sAddTO()));
  connect(_autoUpdate,	 SIGNAL(toggled(bool)), this, SLOT(sHandleAutoUpdate(bool)));
  connect(_delete,	     SIGNAL(clicked()), this, SLOT(sDelete()));
  connect(_deletePrinted,    SIGNAL(clicked()), this, SLOT(sClearPrinted()));
  connect(_pack, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*)));
  connect(_printBatch,       SIGNAL(clicked()), this, SLOT(sPrintBatch()));
  connect(_printEditList,    SIGNAL(clicked()), this, SLOT(sPrintEditList()));
  connect(_printPackingList, SIGNAL(clicked()), this, SLOT(sPrintPackingList()));

  setAcceptDrops(TRUE);

  _pack->addColumn(tr("Order #"),       80,           Qt::AlignCenter, true, "order_number" );
  _pack->addColumn(tr("Type"),		40,           Qt::AlignCenter, true, "pack_head_type" );
  _pack->addColumn(tr("Shipment #"),    80,           Qt::AlignCenter, true, "shipment_number" );
  _pack->addColumn(tr("Customer #"),    _itemColumn,  Qt::AlignLeft,   true, "number"   );
  _pack->addColumn(tr("Customer Name"), -1,           Qt::AlignLeft,   true, "name"   );
  _pack->addColumn(tr("Ship Via"),      80,           Qt::AlignLeft,   true, "shipvia");
  _pack->addColumn(tr("Hold Type"),     _dateColumn,  Qt::AlignCenter, true, "f_holdtype" );
  _pack->addColumn(tr("Printed"),       _dateColumn,  Qt::AlignCenter, true, "pack_printed" );

  if (_privileges->check("MaintainPackingListBatch"))
  {
    _add->setEnabled(TRUE);
    _addTO->setEnabled(TRUE);
    _deletePrinted->setEnabled(TRUE);
    connect(_pack, SIGNAL(valid(bool)), _delete, SLOT(setEnabled(bool)));
  }

  if (_privileges->check("PrintPackingLists"))
  {
    _printBatch->setEnabled(TRUE);
    connect(_pack, SIGNAL(valid(bool)), _printPackingList, SLOT(setEnabled(bool)));
  }

  _addTO->setVisible(_metrics->boolean("MultiWhs"));

  sFillList();
  sHandleAutoUpdate(_autoUpdate->isChecked());
}

packingListBatch::~packingListBatch()
{
    // no need to delete child widgets, Qt does it all for us
}

void packingListBatch::languageChange()
{
    retranslateUi(this);
}

void packingListBatch::sPrintBatch()
{
  QPrinter printer(QPrinter::HighResolution);
  bool     setupPrinter = TRUE;

  XSqlQuery updateq;
  updateq.prepare("UPDATE pack "
		  "SET pack_printed=TRUE "
		  "WHERE (pack_id=:packid);" );

  ParameterList params;
  if (_metrics->boolean("MultiWhs"))
    params.append("MultiWhs");
  MetaSQLQuery mql = mqlLoad("packingListBatch", "print");
  q = mql.toQuery(params);
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  bool userCanceled = false;
  if (orReport::beginMultiPrint(&printer, userCanceled) == false)
  {
    if(!userCanceled)
      systemError(this, tr("Could not initialize printing system for multiple reports."));
    return;
  }
  while (q.next())
  {
    int osmiscid = q.value("pack_shiphead_id").toInt();

    // set sohead_id, tohead_id, and cosmisc_id for customer and 3rd-party use
    ParameterList params;
    params.append("head_id",   q.value("pack_head_id").toInt());
    params.append("head_type", q.value("pack_head_type").toString());
    if (q.value("pack_head_type").toString() == "SO")
      params.append("sohead_id", q.value("pack_head_id").toInt());
    else if (q.value("pack_head_type").toString() == "TO")
      params.append("tohead_id", q.value("pack_head_id").toInt());
    if (osmiscid > 0)
    {
      params.append("cosmisc_id", osmiscid);
      params.append("shiphead_id",  osmiscid);
    }
    if (_metrics->boolean("MultiWhs"))
      params.append("MultiWhs");

    orReport report(q.value(osmiscid > 0 ? "packform" : "pickform").toString(), params);
    if (report.isValid() && report.print(&printer, setupPrinter))
    {
        setupPrinter = FALSE;
	updateq.bindValue(":packid", q.value("pack_id").toInt());
	updateq.exec();
	if (updateq.lastError().type() != QSqlError::NoError)
	{
	  systemError(this, updateq.lastError().databaseText(), __FILE__, __LINE__);
	  orReport::endMultiPrint(&printer);
	  return;
	}
    }
    else
    {
      report.reportError(this);
      orReport::endMultiPrint(&printer);
      return;
    }
  }
  orReport::endMultiPrint(&printer);

  if (setupPrinter)
  {
    QMessageBox::warning(this, tr("Nothing to Print"),
			 tr("<p>All of the Packing Lists appear to have been "
			    "printed already."));
  }
  else
  {
    sFillList();
  }
}

void packingListBatch::setParams(ParameterList & params)
{
  if (_metrics->boolean("MultiWhs"))
    params.append("MultiWhs");
  params.append("none",		tr("None"));
  params.append("credit",	tr("Credit"));
  params.append("ship",		tr("Ship"));
  params.append("pack",		tr("Pack"));
  params.append("return",	tr("Return"));
  params.append("other",	tr("Other"));
}

void packingListBatch::sPrintEditList()
{
  ParameterList params;
  setParams(params);
  orReport report("PackingListBatchEditList", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void packingListBatch::sClearPrinted()
{
  q.exec( "DELETE FROM pack "
          "WHERE ( (pack_printed)"
		  "  AND   (pack_head_type='SO')"
		  "  AND   (checkSOSitePrivs(pack_head_id)) );" );
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  q.exec( "DELETE FROM pack "
          "WHERE ( (pack_printed)"
		  "  AND   (pack_head_type='TO') );" );
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  sFillList();
}

void packingListBatch::sPopulateMenu(QMenu *pMenu)
{
  QAction *menuItem;

  menuItem = pMenu->addAction(tr("View Sales Order..."), this, SLOT(sViewSalesOrder()));
  menuItem->setEnabled(_pack->currentItem()->rawValue("pack_head_type") == "SO" &&
                       (_privileges->check("MaintainSalesOrders") ||
                        _privileges->check("ViewSalesOrders")));
    

  menuItem = pMenu->addAction(tr("View Transfer Order..."), this, SLOT(sViewTransferOrder()));
  menuItem->setEnabled(_pack->currentItem()->rawValue("pack_head_type") == "TO" &&
                       (_privileges->check("MaintainTransferOrders") ||
                        _privileges->check("ViewTransferOrders")));
}

void packingListBatch::sViewSalesOrder()
{
  salesOrder::viewSalesOrder(_pack->id());
}

void packingListBatch::sViewTransferOrder()
{
  transferOrder::viewTransferOrder(_pack->id());
}

void packingListBatch::sAddSO()
{
  ParameterList params;
  params.append("soType", cSoOpen);
  
  salesOrderList newdlg(this, "", TRUE);
  newdlg.set(params);

  int soid;
  if ((soid = newdlg.exec()) != QDialog::Rejected)
  {
    q.prepare("SELECT addToPackingListBatch('SO', :sohead_id) AS result;");
    q.bindValue(":sohead_id", soid);
    q.exec();
    if (q.first())
      sFillList();
    else if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }
}

void packingListBatch::sAddTO()
{
  ParameterList params;
  params.append("toType", cToOpen);
  
  transferOrderList newdlg(this, "", TRUE);
  newdlg.set(params);

  int toid;
  if ((toid = newdlg.exec()) != QSqlError::NoError)
  {
    q.prepare("SELECT addToPackingListBatch('TO', :tohead_id) AS result;");
    q.bindValue(":tohead_id", toid);
    q.exec();
    if (q.first())
      sFillList();
    else if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }
}

void packingListBatch::sDelete()
{
  QString sql( "DELETE FROM pack "
	       "WHERE ((pack_head_id=<? value(\"head_id\") ?>)"
	       "  AND  (pack_head_type=<? value(\"head_type\") ?>)"
	       " <? if exists(\"shiphead_id\") ?>"
	       "  AND  (pack_shiphead_id=<? value(\"shiphead_id\") ?>)"
	       " <? else ?>"
	       "  AND  (pack_shiphead_id IS NULL)"
	       "<? endif ?>"
	       ");" );

  ParameterList params;
  params.append("head_id",   _pack->id());
  params.append("head_type", _pack->currentItem()->rawValue("pack_head_type"));
  if (_pack->altId() > 0)
    params.append("shiphead_id", _pack->altId());

  MetaSQLQuery mql(sql);
  q = mql.toQuery(params);
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  sFillList();
}

void packingListBatch::sPrintPackingList()
{
  if (_pack->altId() == -1)
  {
    QMessageBox::critical(this, tr("Shipment Number Required"),
			  tr("<p>Packing Lists may only be printed for "
			     "existing Shipments and there is no Shipment for "
			     "this Order. Issue Stock To Shipping to "
			     "create a Shipment."));
    return;
  }

  ParameterList params;
  if (_pack->currentItem()->rawValue("pack_head_type") == "TO")
  {
    params.append("head_id",     _pack->id());
    params.append("shiphead_id", _pack->altId());
    params.append("head_type",   _pack->currentItem()->rawValue("pack_head_type"));
    params.append("print");
  }
  else
  {
    params.append("sohead_id",   _pack->id());
    params.append("cosmisc_id",  _pack->altId());
    params.append("shiphead_id", _pack->altId());
  }

  printPackingList newdlg(this, "", TRUE);
  if (newdlg.set(params) == NoError_Print ||
      newdlg.exec() != XDialog::Rejected)
  {
    QString sql( "UPDATE pack "
		 "SET pack_printed=TRUE "
		 "WHERE ((pack_head_id=<? value(\"head_id\") ?>)"
	       "  AND  (pack_head_type=<? value(\"head_type\") ?>)"
		 " <? if exists(\"shiphead_id\") ?>"
		 "  AND  (pack_shiphead_id=<? value(\"shiphead_id\") ?>)"
		 " <? else ?>"
		 "  AND  (pack_shiphead_id IS NULL)"
		 "<? endif ?>"
		 ");" );

    ParameterList params;
    params.append("head_id",   _pack->id());
    params.append("head_type", _pack->currentItem()->rawValue("pack_head_type"));
    if (_pack->altId() > 0)
      params.append("shiphead_id", _pack->altId());

    MetaSQLQuery mql(sql);
    q = mql.toQuery(params);
    if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }

    sFillList();
  }
}

void packingListBatch::sFillList()
{
  ParameterList params;
  setParams(params);
  MetaSQLQuery mql = mqlLoad("packingListBatch", "detail");
  q = mql.toQuery(params);
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
  _pack->populate(q, true);
}

void packingListBatch::sHandleAutoUpdate(bool pAutoUpdate)
{
  if (pAutoUpdate)
    connect(omfgThis, SIGNAL(tick()), this, SLOT(sFillList()));
  else
    disconnect(omfgThis, SIGNAL(tick()), this, SLOT(sFillList()));
}

