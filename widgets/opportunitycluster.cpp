/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "opportunitycluster.h"

OpportunityCluster::OpportunityCluster(QWidget* pParent, const char* pName) :
    VirtualCluster(pParent, pName)
{
    addNumberWidget(new OpportunityClusterLineEdit(this, pName));
}

OpportunityClusterLineEdit::OpportunityClusterLineEdit(QWidget* pParent, const char* pName) :
    CrmClusterLineEdit(pParent, "ophead()", "ophead_id", "ophead_name", 0, "ophead_notes", "ophead_owner_username", "ophead_username", 0, pName)
{
    setTitles(tr("Opportunity"), tr("Opportunities"));
    setUiName("opportunity");
    setEditPriv("MaintainAllOpportunities");
    setNewPriv("MaintainAllOpportunities");
    setViewPriv("ViewAllOpportunities");
    setEditOwnPriv("MaintainPersonalOpportunities");
    setViewOwnPriv("ViewPersonalOpportunities");
}
