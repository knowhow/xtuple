/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef DSPGLSERIES_H
#define DSPGLSERIES_H

#include "guiclient.h"
#include "display.h"

#include "ui_dspGLSeries.h"

class dspGLSeries : public display, public Ui::dspGLSeries
{
    Q_OBJECT

public:
    dspGLSeries(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = Qt::Window);

    virtual bool setParams(ParameterList &);

public slots:
    virtual enum SetResponse set(const ParameterList &);
    virtual void sPopulateMenu(QMenu *, QTreeWidgetItem*, int);
    virtual void sReverse();
    virtual void sEdit();
    virtual void sDelete(bool edited = false);
    virtual void sPost();
    virtual void sViewJournal();

protected slots:
    virtual void languageChange();

private:
    bool _isJournal;

};

#endif // DSPGLSERIES_H
