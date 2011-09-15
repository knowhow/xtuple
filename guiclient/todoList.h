/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef TODOLIST_H
#define TODOLIST_H

#include "guiclient.h"
#include "display.h"

#include "ui_todoList.h"

class todoList : public display, public Ui::todoList
{
  Q_OBJECT

  public:
    todoList(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = Qt::Window);

  public slots:
    virtual SetResponse	set(const ParameterList&);
    virtual void sDelete();
    virtual void sEdit();
    virtual void sEditIncident();
    virtual void sEditTask();
    virtual void sEditProject();
    virtual void sEditCustomer();
    virtual void sNew();
    virtual void sNewIncdt();
    virtual void sPopulateMenu(QMenu *, QTreeWidgetItem *, int);
    virtual void sView();
    virtual void sViewCustomer();
    virtual void sViewIncident();
    virtual void sViewTask();
    virtual void sViewProject();
    virtual bool setParams(ParameterList &);

  private:
    int		    _mode;
    int		    getIncidentId();
    int		    getProjectId();
};

#endif // TODOLIST_H
