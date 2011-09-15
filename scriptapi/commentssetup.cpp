/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "comments.h"
#include <QtScript>

void setupComments(QScriptEngine *engine)
{
  QScriptValue widget = engine->newObject();

  widget.setProperty("Uninitialized",	 QScriptValue(engine, Comments::Uninitialized),    QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Address",	         QScriptValue(engine, Comments::Address),	   QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("BBOMHead",	 QScriptValue(engine, Comments::BBOMHead),	   QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("BBOMItem",	 QScriptValue(engine, Comments::BBOMItem),	   QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("BOMHead",	         QScriptValue(engine, Comments::BOMHead),	   QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("BOMItem",	         QScriptValue(engine, Comments::BOMItem),	   QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("BOOHead",	         QScriptValue(engine, Comments::BOOHead),	   QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("BOOItem",	         QScriptValue(engine, Comments::BOOItem),	   QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("CRMAccount",	 QScriptValue(engine, Comments::CRMAccount),	   QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Contact",	         QScriptValue(engine, Comments::Contact),	   QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Customer",	 QScriptValue(engine, Comments::Customer),	   QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Employee",	 QScriptValue(engine, Comments::Employee),	   QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Incident",	 QScriptValue(engine, Comments::Incident),	   QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Item",	         QScriptValue(engine, Comments::Item),	           QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("ItemSite",	 QScriptValue(engine, Comments::ItemSite),	   QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("ItemSource",	 QScriptValue(engine, Comments::ItemSource),	   QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Location",	 QScriptValue(engine, Comments::Location),	   QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("LotSerial",	 QScriptValue(engine, Comments::LotSerial),	   QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Opportunity",	 QScriptValue(engine, Comments::Opportunity),      QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Project",	         QScriptValue(engine, Comments::Project),	   QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("PurchaseOrder",	 QScriptValue(engine, Comments::PurchaseOrder),    QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("PurchaseOrderItem",QScriptValue(engine, Comments::PurchaseOrderItem),QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("ReturnAuth",	 QScriptValue(engine, Comments::ReturnAuth),	   QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("ReturnAuthItem",	 QScriptValue(engine, Comments::ReturnAuthItem),   QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Quote",	         QScriptValue(engine, Comments::Quote),	           QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("QuoteItem",	 QScriptValue(engine, Comments::QuoteItem),	   QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("SalesOrder",	 QScriptValue(engine, Comments::SalesOrder),	   QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("SalesOrderItem",	 QScriptValue(engine, Comments::SalesOrderItem),   QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("TransferOrder",	 QScriptValue(engine, Comments::TransferOrder),    QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("TransferOrderItem",QScriptValue(engine, Comments::TransferOrderItem),QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Vendor",	         QScriptValue(engine, Comments::Vendor),	   QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Warehouse",	 QScriptValue(engine, Comments::Warehouse),	   QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("WorkOrder",	 QScriptValue(engine, Comments::WorkOrder),	   QScriptValue::ReadOnly | QScriptValue::Undeletable);

  engine->globalObject().setProperty("Comments", widget, QScriptValue::ReadOnly | QScriptValue::Undeletable);
}
