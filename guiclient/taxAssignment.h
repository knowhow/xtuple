/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef TAXASSIGNMENT_H
#define TAXASSIGNMENT_H

#include "guiclient.h"
#include "xdialog.h"
#include <parameter.h>
#include "ui_taxAssignment.h"

class taxAssignment : public XDialog, public Ui::taxAssignment
{
  Q_OBJECT

  public:
    taxAssignment(QWidget* = 0, const char* = 0, bool = false, Qt::WFlags = 0);
    ~taxAssignment();

  public slots:
    virtual SetResponse	set(const ParameterList &);
    virtual void	sPopulate();
	virtual void	sPopulateTaxCode();
	virtual void	sAdd();
	virtual void	sRevoke();

  protected slots:
    virtual void	languageChange();

  private:
    bool	_modal;
    int		_mode;
    int		_taxassId;
	int		_taxzoneId;
	int		_taxtypeId;
};

#endif // TAXASSIGNMENT_H
