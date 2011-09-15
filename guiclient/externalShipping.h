/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef EXTERNALSHIPPING_H
#define EXTERNALSHIPPING_H

#include "guiclient.h"
#include "xdialog.h"
#include <ui_externalShipping.h>

class externalShipping : public XDialog, public Ui::externalShipping
{
    Q_OBJECT

  public:
    externalShipping(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WFlags fl = 0);
    ~externalShipping();

    static bool userHasPriv(const int = cView);
    virtual void setVisible(bool);
    void showEvent(QShowEvent *event);

  public slots:
    enum SetResponse set(const ParameterList &pParams);
    virtual SetResponse setModel(XSqlTableModel *model );

  protected slots:
    virtual void languageChange();
    virtual void sHandleOrder();
    virtual void sSave();
    virtual void sClose();
    virtual void sReject();

  private:
    XSqlTableModel   *_model;
    int               _idx;
    int               _mode;
};

#endif // EXTERNALSHIPPING_H
