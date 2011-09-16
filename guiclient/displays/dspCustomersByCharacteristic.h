/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef DSPCUSTOMERSBYCHARACTERISTIC_H
#define DSPCUSTOMERSBYCHARACTERISTIC_H

#include "display.h"

#include "ui_dspCustomersByCharacteristic.h"

class dspCustomersByCharacteristic : public display, public Ui::dspCustomersByCharacteristic
{
    Q_OBJECT

public:
    dspCustomersByCharacteristic(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = Qt::Window);

    virtual bool setParams(ParameterList&);

public slots:
    virtual void sPopulateMenu(QMenu * pMenu, QTreeWidgetItem * selected, int);
    virtual void sEdit();
    virtual void sEditCharacteristic();
    virtual void sNewCharacteristic();
    virtual void sView();

protected slots:
    virtual void languageChange();

};

#endif // DSPCUSTOMERSBYCHARACTERISTIC_H
