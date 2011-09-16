/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef __XTHELP_H__
#define __XTHELP_H__

#include <QHelpEngine>
#include <QNetworkRequest>

class QByteArray;
class QNetworkAccessManager;
class QNetworkReply;

class xtHelp : public QHelpEngine
{
  Q_OBJECT
  public:
    Q_INVOKABLE static xtHelp *getInstance(QWidget *parent = 0);
    ~xtHelp();

    Q_INVOKABLE QByteArray     fileData(const QUrl &url) const;
    Q_INVOKABLE QUrl           homePage() const;
    Q_INVOKABLE bool           isOnline() const;

    Q_INVOKABLE static void reload();

  public slots:
    void                        sError(QNetworkReply *);

  protected:
    QNetworkAccessManager      *_nam;
    bool                        _online;

  private:
    xtHelp(QWidget *parent = 0);
};

#endif //__XTHELP_H__
