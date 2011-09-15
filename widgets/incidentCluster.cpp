/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "incidentcluster.h"

IncidentCluster::IncidentCluster(QWidget* pParent, const char* pName) :
    VirtualCluster(pParent, pName)
{
    addNumberWidget(new IncidentClusterLineEdit(this, pName));
}

IncidentClusterLineEdit::IncidentClusterLineEdit(QWidget* pParent, const char* pName) :
    VirtualClusterLineEdit(pParent, "incdt", "incdt_id", "incdt_number::text", "incdt_assigned_username", "incdt_summary", 0, pName)
{
  setTitles(tr("Incident"), tr("Incidents"));
  setUiName("incident");
  setEditPriv("MaintainIncidents");
  setNewPriv("AddIncidents");
  setViewPriv("ViewIncidents");
}
