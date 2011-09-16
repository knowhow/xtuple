/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef DICTIONARIES_H
#define DICTIONARIES_H

#include "guiclient.h"
#include "xwidget.h"

#include "ui_dictionaries.h"

class QNetworkAccessManager;
class QNetworkReply;

class dictionaries : public XWidget, public Ui::dictionaries
{
    Q_OBJECT

    enum State {
      Idle = 0,
      Busy
    };
public:
    dictionaries(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = Qt::Window);
    ~dictionaries();

public slots:
    virtual void sAction();

protected slots:
    virtual void languageChange();
    virtual void finished(QNetworkReply*);
    virtual void downloadProgress(qint64, qint64);

private:
    QString langext;
    State _state;
    QNetworkAccessManager * nwam;
    QNetworkReply * _nwrep;
};

#endif // DICTIONARIES_H
