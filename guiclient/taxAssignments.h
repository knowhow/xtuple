/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef TAXASSIGNMENTS_H
#define TAXASSIGNMENTS_H

#include "guiclient.h"
#include "xwidget.h"

#include "ui_taxAssignments.h"

class taxAssignments : public XWidget, public Ui::taxAssignments
{
  Q_OBJECT

  public:
    taxAssignments(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = Qt::Window);
    ~taxAssignments();
	
  public slots:
    virtual void	sDelete();
    virtual void	sEdit();
    virtual void	sFillList();
    virtual void	sNew();
    virtual void	sView();

  protected slots:
    virtual void	languageChange();
  
  private:
	int _taxZoneid;
    int _taxTypeid;
};

#endif // TAXASSIGNMENTS_H
