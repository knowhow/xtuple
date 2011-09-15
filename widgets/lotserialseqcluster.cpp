/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "lotserialseqcluster.h"

LotserialseqCluster::LotserialseqCluster(QWidget* pParent, const char* pName) :
    VirtualCluster(pParent, pName)
{
    addNumberWidget(new LotserialseqClusterLineEdit(this, pName));
}

LotserialseqClusterLineEdit::LotserialseqClusterLineEdit(QWidget* pParent, const char* pName) :
    VirtualClusterLineEdit(pParent, "lsseq", "lsseq_id", "lsseq_number", 0, "lsseq_descrip", 0, pName)
{
    setTitles(tr("Lot/Serial Sequence"), tr("Lot/Serial Sequences"));
    setUiName("lotSerialSequence");
    setEditPriv("MaintainLotSerialSequences");
    setViewPriv("ViewLotSerialSequences");
}
