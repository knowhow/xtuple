/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "alarms.h"
#include <QtScript>

void setupAlarms(QScriptEngine *engine)
{
  QScriptValue widget = engine->newObject();

  widget.setProperty("Uninitialized",	 QScriptValue(engine, Alarms::Uninitialized),	 QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Address",	         QScriptValue(engine, Alarms::Address),	         QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("BBOMHead",	 QScriptValue(engine, Alarms::BBOMHead),	 QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("BBOMItem",	 QScriptValue(engine, Alarms::BBOMItem),	 QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("BOMHead",	         QScriptValue(engine, Alarms::BOMHead),	         QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("BOMItem",	         QScriptValue(engine, Alarms::BOMItem),	         QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("BOOHead",	         QScriptValue(engine, Alarms::BOOHead),	         QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("BOOItem",	         QScriptValue(engine, Alarms::BOOItem),	         QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("CRMAccount",	 QScriptValue(engine, Alarms::CRMAccount),	 QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Contact",	         QScriptValue(engine, Alarms::Contact),	         QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Customer",	 QScriptValue(engine, Alarms::Customer),	 QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Employee",	 QScriptValue(engine, Alarms::Employee),	 QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Incident",	 QScriptValue(engine, Alarms::Incident),	 QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Item",	         QScriptValue(engine, Alarms::Item),	         QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("ItemSite",	 QScriptValue(engine, Alarms::ItemSite),	 QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("ItemSource",	 QScriptValue(engine, Alarms::ItemSource),	 QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Location",	 QScriptValue(engine, Alarms::Location),	 QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("LotSerial",	 QScriptValue(engine, Alarms::LotSerial),	 QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Opportunity",	 QScriptValue(engine, Alarms::Opportunity),	 QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Project",	         QScriptValue(engine, Alarms::Project),	         QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("PurchaseOrder",	 QScriptValue(engine, Alarms::PurchaseOrder),	 QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("PurchaseOrderItem",QScriptValue(engine, Alarms::PurchaseOrderItem),QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("ReturnAuth",	 QScriptValue(engine, Alarms::ReturnAuth),	 QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("ReturnAuthItem",	 QScriptValue(engine, Alarms::ReturnAuthItem),   QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Quote",	         QScriptValue(engine, Alarms::Quote),	         QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("QuoteItem",	 QScriptValue(engine, Alarms::QuoteItem),	 QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("SalesOrder",	 QScriptValue(engine, Alarms::SalesOrder),	 QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("SalesOrderItem",	 QScriptValue(engine, Alarms::SalesOrderItem),   QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("TodoItem",	 QScriptValue(engine, Alarms::TodoItem),	 QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("TransferOrder",	 QScriptValue(engine, Alarms::TransferOrder),	 QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("TransferOrderItem",QScriptValue(engine, Alarms::TransferOrderItem),QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Vendor",	         QScriptValue(engine, Alarms::Vendor),	         QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Warehouse",	 QScriptValue(engine, Alarms::Warehouse),	 QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("WorkOrder",	 QScriptValue(engine, Alarms::WorkOrder),	 QScriptValue::ReadOnly | QScriptValue::Undeletable);

  engine->globalObject().setProperty("Alarms", widget, QScriptValue::ReadOnly | QScriptValue::Undeletable);
}
