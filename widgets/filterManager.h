/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef FILTERMANAGER_H
#define FILTERMANAGER_H

#include "parameter.h"

#include "ui_filterManager.h"

class filterManager : public QDialog, public Ui::filterManager
{
    Q_OBJECT

public:
    filterManager(QWidget* parent = 0, const char* name = 0);
    void set( ParameterList & pParams );
    void populate();
	
public slots:
    void shareFilter();
    void unshareFilter();
    void deleteFilter();

private slots:
    void handleButtons(bool valid);

signals:
    void filterDeleted();

private:
   QString _screen;
};

#endif // FILTERMANAGER_H
