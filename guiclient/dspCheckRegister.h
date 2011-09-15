/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef DSPCHECKREGISTER_H
#define DSPCHECKREGISTER_H

#include "xwidget.h"

#include "ui_dspCheckRegister.h"

class dspCheckRegister : public XWidget, public Ui::dspCheckRegister
{
    Q_OBJECT

public:
    dspCheckRegister(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = Qt::Window);
    ~dspCheckRegister();
    virtual bool setParams(ParameterList &);

public slots:
    virtual void sPrint();
    virtual void sFillList();
    virtual void sVoidPosted();
	  virtual void sHandleButtons();
   
protected slots:
    virtual void languageChange();

    virtual void sPopulateMenu(QMenu *);

};

#endif // DSPCHECKREGISTER_H
