/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef TRANSLATIONS_H
#define TRANSLATIONS_H

#include "guiclient.h"
#include "xwidget.h"

#include "ui_translations.h"

class QNetworkAccessManager;
class QNetworkReply;

class translations : public XWidget, public Ui::translations
{
    Q_OBJECT

public:
    translations(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = Qt::Window);
    ~translations();

public slots:
    virtual void sLocaleChanged();
    virtual void sLanguageSelected(QTreeWidgetItem * item, int column);
    virtual void sPopulateMenu(QMenu * pMenu, QTreeWidgetItem *, int pColumn);
    virtual void sDownload();

protected slots:
    virtual void languageChange();
    virtual void finished(QNetworkReply*);
    virtual void downloadProgress(qint64, qint64);

private:
    QNetworkAccessManager * nwam;
    QString langext;
};

#endif // TRANSLATIONS_H
