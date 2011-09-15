/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef FILTERSAVE_H
#define FILTERSAVE_H

#include "parameter.h"

#include "ui_filterSave.h"

class filterSave : public QDialog, public Ui::filterSave
{
    Q_OBJECT

public:
    filterSave(QWidget* parent = 0, const char* name = 0);
    void set( ParameterList & pParams );
	
public slots:
    void save();

private:
   QString _filter;
   QString _username;
   QString _classname;
};

#endif // filterSave_H
