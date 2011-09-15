/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef OPPORTUNITYLIST_H
#define OPPORTUNITYLIST_H

#include "guiclient.h"
#include "display.h"
#include <parameter.h>

#include "ui_opportunityList.h"

class opportunityList : public display, public Ui::opportunityList
{
  Q_OBJECT

  public:
    opportunityList(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = Qt::Window);

    Q_INVOKABLE virtual bool setParams(ParameterList &);

  public slots:
    virtual SetResponse	set(const ParameterList&);
    virtual void	sDelete();
    virtual void	sEdit();
    virtual void	sNew();
    virtual void sPopulateMenu(QMenu *, QTreeWidgetItem *, int);
    virtual void	sView();
    virtual void	sDeactivate();
    virtual void	sActivate();

};

#endif // OPPORTUNITYLIST_H
