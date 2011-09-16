/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef __DSPITEMSBYCLASSCODE_H__
#define __DSPITEMSBYCLASSCODE_H__

#include "dspItemsByParameter.h"

class dspItemsByClassCode : public dspItemsByParameter
{
  Q_OBJECT

  public:
    dspItemsByClassCode(QWidget * = 0, const char * = 0, Qt::WindowFlags = 0);
};

#endif //__DSPITEMSBYCLASSCODE_H__
