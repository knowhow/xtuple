/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef __LOGINOPTIONS_H__
#define __LOGINOPTIONS_H__

class QWidget;

#include <QDialog>
#include <QString>

#include "parameter.h"

#include "tmp/ui_login2Options.h"

class login2Options : public QDialog, public Ui::login2Options
{
    Q_OBJECT

  public:
    login2Options(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WFlags fl = 0);
    ~login2Options();

    QString _databaseURL;

  public slots:
    virtual void set(const ParameterList & pParams );

  protected slots:
    virtual void languageChange();

    virtual void sSave();

  private:
    bool _saveSettings;
};

#endif
