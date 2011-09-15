/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef TAXREGISTRATION_H
#define TAXREGISTRATION_H

#include "guiclient.h"
#include "xdialog.h"
#include <parameter.h>
#include <ui_taxRegistration.h>

class taxRegistration : public XDialog, public Ui::taxRegistration
{
  Q_OBJECT

  public:
    taxRegistration(QWidget* = 0, const char* = 0, bool = false, Qt::WFlags = 0);
    ~taxRegistration();

    QPushButton* _save;

  public slots:
    virtual SetResponse set(const ParameterList pParams);

  protected slots:
    virtual void languageChange();

    virtual void sHandleButtons();
    virtual void sPopulate();
    virtual void sSave();

  protected:
    int		_mode;
    int		_relid;
    QString	_reltype;
    int		_taxregid;

  private:
    int		handleReltype();
};

#endif // TAXREGISTRATION_H
