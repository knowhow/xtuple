/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef DSPSTANDARDJOURNALHISTORY_H
#define DSPSTANDARDJOURNALHISTORY_H

#include "display.h"

#include "ui_dspStandardJournalHistory.h"

class dspStandardJournalHistory : public display, public Ui::dspStandardJournalHistory
{
    Q_OBJECT

public:
    dspStandardJournalHistory(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = Qt::Window);

    virtual bool setParams(ParameterList&);

public slots:
    virtual void sPopulateMenu(QMenu *, QTreeWidgetItem*, int);

protected slots:
    virtual void languageChange();

    virtual void sReverse();
    virtual void sDelete();


};

#endif // DSPSTANDARDJOURNALHISTORY_H
