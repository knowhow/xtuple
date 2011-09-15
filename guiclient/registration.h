/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef REGISTRATION_H
#define REGISTRATION_H

#include "xdialog.h"
#include <QHttp>

#include "ui_registration.h"

class registration : public XDialog, public Ui::registration
{
    Q_OBJECT

public:
  registration(QWidget* parent = 0, const char * = 0, Qt::WindowFlags fl = Qt::Dialog);
  ~registration();

public slots:
    virtual void sBack();
    virtual void sCancel();
    virtual void sDone(bool);
    virtual void sNext();
    virtual void sRead(int, int);
    virtual void sRegister(bool pretry = false);
    virtual void sSent(int, int);
    virtual void sState(int);
    virtual void reject();
    virtual void sChkDone(bool);
    virtual void sCheck();

protected slots:
    virtual void languageChange();

protected:
    QHttp *_postreq;
    QHttp *_userchk;
    QString encodedPair(const QString, const QString);
};

#endif // REGISTRATION_H
