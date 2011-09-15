/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef PRINTCHECKSREVIEW_H
#define PRINTCHECKSREVIEW_H

#include "xdialog.h"
#include "ui_printChecksReview.h"

class printChecksReview : public XDialog, public Ui::printChecksReview
{
    Q_OBJECT

public:
    printChecksReview(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WFlags fl = 0);
    ~printChecksReview();

public slots:
    virtual void sUnmark();
    virtual void sMarkPrinted();
    virtual void sMarkVoided();
    virtual void sMarkReplaced();
    virtual void sSelectAll();

protected:
    virtual void markSelected( int );

protected slots:
    virtual void languageChange();

    virtual void sComplete();

private:

};

#endif // PRINTCHECKSREVIEW_H
