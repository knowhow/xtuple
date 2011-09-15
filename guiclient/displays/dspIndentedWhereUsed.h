/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef DSPINDENTEDWHEREUSED_H
#define DSPINDENTEDWHEREUSED_H

#include "guiclient.h"
#include "display.h"
#include <parameter.h>

#include "ui_dspIndentedWhereUsed.h"

class dspIndentedWhereUsed : public display, public Ui::dspIndentedWhereUsed
{
    Q_OBJECT

public:
    dspIndentedWhereUsed(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = Qt::Window);

    virtual bool setParams(ParameterList &);

public slots:
    virtual enum SetResponse set( const ParameterList & pParams );
    virtual void sViewInventoryHistory();
    virtual void sPopulateMenu(QMenu * menu, QTreeWidgetItem *, int);
    virtual void sPreview();
    virtual void sPrint();
    virtual void sFillList();

protected slots:
    virtual void languageChange();

private:
    void worksetWrapper(int);
    int _worksetid;
};

#endif // DSPINDENTEDWHEREUSED_H
