/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef EMPLOYEE_H
#define EMPLOYEE_H

#include "guiclient.h"
#include "xdialog.h"
#include "ui_employee.h"

class employee : public XDialog, public Ui::employee
{
  Q_OBJECT
  
  public:
    employee(QWidget* = 0, const char * = 0, Qt::WindowFlags = 0);
    ~employee();

    static bool userHasPriv(const int = cView);
    virtual void setVisible(bool);

  public slots:
    virtual enum SetResponse set(const ParameterList &);
    virtual bool sPopulate();
    virtual bool sSave(const bool = true);
    virtual void reject();

  protected slots:
    virtual void languageChange();
    virtual void sAttachGroup();
    virtual void sCrmAccount();
    virtual void sDeleteCharass();
    virtual void sDetachGroup();
    virtual void sEditCharass();
    virtual void sEditGroup();
    virtual void sFillCharassList();
    virtual void sFillGroupsList();
    virtual void sHandleButtons();
    virtual void sNewCharass();
    virtual void sViewGroup();

  private:
    int _crmacctid;
    QString _crmowner;
    int _empid;
    int _mode;
    int _NumberGen;
    int _origmode;

signals:
    void saved();
    void populated();
};

#endif
