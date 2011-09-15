/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef STATE_H
#define STATE_H

#include "guiclient.h"
#include "xdialog.h"
#include <parameter.h>
#include "ui_state.h"

class state : public XDialog, public Ui::state
{
    Q_OBJECT

  public:
    state(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WFlags fl = 0);
    ~state();

    Q_INVOKABLE virtual void enableWindowModifiedSetting();
    Q_INVOKABLE         int id() const { return _stateid; }

  public slots:
    virtual void reject();
    virtual enum SetResponse set(const ParameterList &pParams);
    virtual void sDataChanged();
    virtual void sSave();
    virtual void populate();

  protected slots:
    virtual void languageChange();

  private:
    int _mode;
    int _stateid;
    QString _abbrCache;
    int     _countryCache;
    QString _nameCache;
};

#endif // STATE_H
