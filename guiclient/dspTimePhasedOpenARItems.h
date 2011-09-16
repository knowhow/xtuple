/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef DSPTIMEPHASEDOPENARITEMS_H
#define DSPTIMEPHASEDOPENARITEMS_H

#include "guiclient.h"
#include "display.h"
#include <parameter.h>

#include "ui_dspTimePhasedOpenARItems.h"

class dspTimePhasedOpenARItems : public display, public Ui::dspTimePhasedOpenARItems
{
    Q_OBJECT

public:
    dspTimePhasedOpenARItems(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = Qt::Window);
    ~dspTimePhasedOpenARItems();

    virtual bool setParams(ParameterList &);

public slots:
    virtual void sViewOpenItems();
    virtual void sPrintStatement();
    virtual void sPopulateMenu( QMenu *, QTreeWidgetItem *, int ); 
    virtual void sFillList();
    virtual void sFillStd();
    virtual void sFillCustom();
    virtual void sToggleCustom();

private:
    int _column;
    QList<DatePair> _columnDates;
};

#endif // DSPTIMEPHASEDOPENARITEMS_H
