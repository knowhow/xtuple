/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef UPDATEREORDERLEVELS_H
#define UPDATEREORDERLEVELS_H

#include "guiclient.h"
#include "xdialog.h"
#include "ui_updateReorderLevels.h"

class updateReorderLevels : public XDialog, public Ui::updateReorderLevels
{
    Q_OBJECT

public:
    updateReorderLevels(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WFlags fl = 0);
    ~updateReorderLevels();
  
    virtual bool setParams(ParameterList &);

public slots:
    virtual SetResponse set( const ParameterList & pParams );
    virtual void sUpdate();
    virtual void sSubmit();
    virtual void sPost();
    virtual void sOpenEdit(XTreeWidgetItem *item, const int col);
    virtual void sCloseEdit(XTreeWidgetItem *current, XTreeWidgetItem *previous);
    virtual void sItemChanged(XTreeWidgetItem *item, const int col);
    virtual void sHandleButtons();

protected slots:
    virtual void languageChange();

private:
    QButtonGroup*   _daysGroupInt;
};

#endif // UPDATEREORDERLEVELS_H
