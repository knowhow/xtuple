/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef PROSPECT_H
#define PROSPECT_H

#include "guiclient.h"
#include "xwidget.h"
#include <parameter.h>

#include "ui_prospect.h"

class prospect : public XWidget, public Ui::prospect
{
    Q_OBJECT

public:
    prospect(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = Qt::Window);
    ~prospect();

    Q_INVOKABLE virtual int id() const { return _prospectid; }

public slots:
    virtual SetResponse set(const ParameterList & pParams );

    virtual void	sCheckNumber();
    virtual void	sDeleteQuote();
    virtual void	sEditQuote();
    virtual void	sNewQuote();
    virtual void	sPrintQuote();
    virtual void	sSave();
    virtual void	sViewQuote();
    virtual void	sFillQuotesList();
    virtual void	sPopulateQuotesMenu(QMenu * menuThis);
    virtual void	populate();

signals:
            void newId(int);
            void populated();
            void saved(int);

protected slots:
    virtual void languageChange();

protected:
    virtual void closeEvent(QCloseEvent*);

private:
    int _mode;
    int _prospectid;
    int _NumberGen;
    QString _cachedNumber;
};

#endif // PROSPECT_H
