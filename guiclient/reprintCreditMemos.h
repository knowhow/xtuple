/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef REPRINTCREDITMEMOS_H
#define REPRINTCREDITMEMOS_H

#include "guiclient.h"
#include "xdialog.h"
#include <parameter.h>
#include "ui_reprintCreditMemos.h"

class reprintCreditMemos : public XDialog, public Ui::reprintCreditMemos
{
    Q_OBJECT

public:
    reprintCreditMemos(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WFlags fl = 0);
    ~reprintCreditMemos();

public slots:
    virtual void sPrint();
    virtual void sHandleCopies( int pValue );
    virtual void sEditWatermark();

signals:
            void finishedPrinting(int);

protected slots:
    virtual void languageChange();

};

#endif // REPRINTCREDITMEMOS_H
