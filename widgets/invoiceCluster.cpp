/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "invoiceCluster.h"

InvoiceCluster::InvoiceCluster(QWidget* pParent, const char* pName) :
    VirtualCluster(pParent, pName)
{
    addNumberWidget(new InvoiceClusterLineEdit(this, pName));
}

InvoiceClusterLineEdit::InvoiceClusterLineEdit(QWidget* pParent, const char* pName) :
    VirtualClusterLineEdit(pParent, "invchead", "invchead_id", "invchead_invcnumber", "invchead_billto_name", 0, 0, pName)
{
  setTitles(tr("Invoice"), tr("Invoices"));
  setUiName("invoice");
  setEditPriv("EditAROpenItems");
  setViewPriv("ViewAROpenItems");
  setExtraClause("(checkInvoiceSitePrivs(invchead_id))");
}
