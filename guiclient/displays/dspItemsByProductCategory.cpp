/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspItemsByProductCategory.h"

dspItemsByProductCategory::dspItemsByProductCategory(QWidget * parent, const char *, Qt::WindowFlags flags)
  : dspItemsByParameter(parent, "dspItemsByProductCategory", flags)
{
  setWindowTitle(tr("Items by Product Category"));
  setReportName("ItemsByProductCategory");
  _parameter->setType(ParameterGroup::ProductCategory);
}

bool dspItemsByProductCategory::setParams(ParameterList & pParams)
{
  pParams.append("sold");
  return dspItemsByParameter::setParams(pParams);
}
