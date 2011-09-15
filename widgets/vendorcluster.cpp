/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "vendorcluster.h"
#include "crmacctcluster.h"

VendorLineEdit::VendorLineEdit(QWidget *pParent, const char *pName) :
  VirtualClusterLineEdit(pParent, "vendinfo", "vend_id", "vend_number",
                         "vend_name", 0, 0, pName, "vend_active")
{
  setTitles(tr("Vendor"), tr("Vendors"));
  setUiName("vendor");
  setEditPriv("MaintainVendors");
  setViewPriv("ViewVendors");
  setNewPriv("MaintainVendors");

  _query = "SELECT vend_id AS id, vend_number AS number, vend_name AS name, vendtype_code AS type,"
           "       addr.*,"
           "       formatAddr(addr_line1, addr_line2, addr_line3, '', '') AS street,"
           "       cntct.*, vend_active as active "
           "    FROM vendtype JOIN vendinfo"
           "           ON (vend_vendtype_id=vendtype_id) LEFT OUTER JOIN"
           "         cntct ON (vend_cntct1_id=cntct_id) LEFT OUTER JOIN"
           "         addr ON (vend_addr_id=addr_id)"
           " WHERE true ";
}

VirtualList *VendorLineEdit::listFactory()
{
  return new CRMAcctList(this);
}

VirtualSearch *VendorLineEdit::searchFactory()
{
  return new CRMAcctSearch(this);
}

VendorCluster::VendorCluster(QWidget *pParent, const char *pName) :
  VirtualCluster(pParent, pName)
{
  addNumberWidget(new VendorLineEdit(this, pName));
  setNameVisible(true);
}
