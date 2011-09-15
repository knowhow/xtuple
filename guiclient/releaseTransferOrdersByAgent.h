/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef RELEASETRANSFERORDERSBYAGENT_H
#define RELEASETRANSFERORDERSBYAGENT_H

#include "guiclient.h"
#include "xdialog.h"
#include "ui_releaseTransferOrdersByAgent.h"

class releaseTransferOrdersByAgent : public XDialog, public Ui::releaseTransferOrdersByAgent
{
    Q_OBJECT

public:
    releaseTransferOrdersByAgent(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WFlags fl = 0);
    ~releaseTransferOrdersByAgent();

public slots:
    virtual void sRelease();

protected slots:
    virtual void languageChange();

};

#endif // RELEASETRANSFERORDERSBYAGENT_H
