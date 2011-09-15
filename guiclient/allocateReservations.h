/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef ALLOCATERESERVATIONS_H
#define ALLOCATERESERVATIONS_H

#include "guiclient.h"
#include "xdialog.h"
#include "ui_allocateReservations.h"

class allocateReservations : public XDialog, public Ui::allocateReservations
{
    Q_OBJECT

public:
    allocateReservations(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WFlags fl = 0);
    ~allocateReservations();

    QPushButton* _submit;

public slots:
    virtual void sAllocate();
    virtual void sSubmit();

protected slots:
    virtual void sCustomerSelected();
    virtual void languageChange();

};

#endif // ALLOCATERESERVATIONS_H
