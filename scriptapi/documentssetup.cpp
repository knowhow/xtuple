/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "documents.h"
#include <QtScript>

void setupDocuments(QScriptEngine *engine)
{
  QScriptValue widget = engine->newObject();

  widget.setProperty("Uninitialized",	 QScriptValue(engine, Documents::Uninitialized),    QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Address",	         QScriptValue(engine, Documents::Address),	    QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("BBOMHead",	 QScriptValue(engine, Documents::BBOMHead),	    QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("BBOMItem",	 QScriptValue(engine, Documents::BBOMItem),	    QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("BOMHead",	         QScriptValue(engine, Documents::BOMHead),	    QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("BOMItem",	         QScriptValue(engine, Documents::BOMItem),	    QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("BOOHead",	         QScriptValue(engine, Documents::BOOHead),	    QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("BOOItem",	         QScriptValue(engine, Documents::BOOItem),	    QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("CRMAccount",	 QScriptValue(engine, Documents::CRMAccount),	    QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Contact",	         QScriptValue(engine, Documents::Contact),	    QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Customer",	 QScriptValue(engine, Documents::Customer),	    QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Employee",	 QScriptValue(engine, Documents::Employee),	    QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Incident",	 QScriptValue(engine, Documents::Incident),	    QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Item",	         QScriptValue(engine, Documents::Item),	            QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("ItemSite",	 QScriptValue(engine, Documents::ItemSite),	    QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("ItemSource",	 QScriptValue(engine, Documents::ItemSource),	    QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Location",	 QScriptValue(engine, Documents::Location),	    QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("LotSerial",	 QScriptValue(engine, Documents::LotSerial),	    QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Opportunity",	 QScriptValue(engine, Documents::Opportunity),      QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Project",	         QScriptValue(engine, Documents::Project),	    QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("PurchaseOrder",	 QScriptValue(engine, Documents::PurchaseOrder),    QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("PurchaseOrderItem",QScriptValue(engine, Documents::PurchaseOrderItem),QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("ReturnAuth",	 QScriptValue(engine, Documents::ReturnAuth),	    QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("ReturnAuthItem",	 QScriptValue(engine, Documents::ReturnAuthItem),   QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Quote",	         QScriptValue(engine, Documents::Quote),	    QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("QuoteItem",	 QScriptValue(engine, Documents::QuoteItem),	    QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("SalesOrder",	 QScriptValue(engine, Documents::SalesOrder),	    QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("SalesOrderItem",	 QScriptValue(engine, Documents::SalesOrderItem),   QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("TransferOrder",	 QScriptValue(engine, Documents::TransferOrder),    QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("TransferOrderItem",QScriptValue(engine, Documents::TransferOrderItem),QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Vendor",	         QScriptValue(engine, Documents::Vendor),	    QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Warehouse",	 QScriptValue(engine, Documents::Warehouse),	    QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("WorkOrder",	 QScriptValue(engine, Documents::WorkOrder),	    QScriptValue::ReadOnly | QScriptValue::Undeletable);

  engine->globalObject().setProperty("Documents", widget, QScriptValue::ReadOnly | QScriptValue::Undeletable);
}
