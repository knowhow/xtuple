/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef DSPCOSTEDSINGLELEVELBOM_H
#define DSPCOSTEDSINGLELEVELBOM_H

#include "dspCostedBOMBase.h"

class dspCostedSingleLevelBOM : public dspCostedBOMBase
{
    Q_OBJECT

public:
    dspCostedSingleLevelBOM(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = Qt::Window);

    virtual bool setParams(ParameterList &params);

public slots:
    virtual void sFillList();

};

#endif // DSPCOSTEDSINGLELEVELBOM_H
