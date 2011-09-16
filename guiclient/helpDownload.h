/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef HELPDOWNLOAD_H
#define HELPDOWNLOAD_H

#include "guiclient.h"
#include "xwidget.h"

#include "ui_helpDownload.h"

class QNetworkAccessManager;
class QNetworkReply;

class helpDownload : public XWidget, public Ui::helpDownload
{
    Q_OBJECT

    enum State {
      Idle = 0,
      Busy
    };
public:
    helpDownload(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = Qt::Window);
    ~helpDownload();

public slots:
    virtual void sAction();

protected slots:
    virtual void languageChange();
    virtual void finished(QNetworkReply*);
    virtual void downloadProgress(qint64, qint64);

private:
    QString ver;
    State _state;
    QNetworkAccessManager * nwam;
    QNetworkReply * _nwrep;
};

#endif // HELPDOWNLOAD_H
