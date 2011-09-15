/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef CUSTOMCOMMANDS_H
#define CUSTOMCOMMANDS_H

#include "xwidget.h"

#include "ui_customCommands.h"

class customCommands : public XWidget, public Ui::customCommands
{
    Q_OBJECT

public:
    customCommands(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = Qt::Window);
    ~customCommands();

public slots:
    virtual void sNew();
    virtual void sEdit();
    virtual void sDelete();
    virtual void sFillList();

protected slots:
    virtual void languageChange();

private:

};

#endif // CUSTOMCOMMANDS_H
