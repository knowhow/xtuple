/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef DSPTODOBYUSERANDINCIDENT_H
#define DSPTODOBYUSERANDINCIDENT_H

#include "display.h"

#include "ui_dspTodoByUserAndIncident.h"

class dspTodoByUserAndIncident : public display, public Ui::dspTodoByUserAndIncident
{
    Q_OBJECT

public:
    dspTodoByUserAndIncident(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = Qt::Window);

    virtual bool setParams(ParameterList&);

public slots:
    virtual void sEditIncident();
    virtual void sEditTodoItem();
    virtual void sPopulateMenu(QMenu*, QTreeWidgetItem*, int);
    virtual void sViewIncident();
    virtual void sViewTodoItem();

protected slots:
    virtual void languageChange();

};

#endif // DSPTODOBYUSERANDINCIDENT_H
