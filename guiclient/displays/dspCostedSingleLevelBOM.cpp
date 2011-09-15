/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspCostedSingleLevelBOM.h"

#include <QSqlError>
#include <QVariant>

dspCostedSingleLevelBOM::dspCostedSingleLevelBOM(QWidget* parent, const char*, Qt::WFlags fl)
    : dspCostedBOMBase(parent, "dspCostedSingleLevelBOM", fl)
{
  setWindowTitle(tr("Costed Single Level Bill of Materials"));
  setReportName("CostedSingleLevelBOM");
}

bool dspCostedSingleLevelBOM::setParams(ParameterList &params)
{
  params.append("singleLevelBOM");

  return dspCostedBOMBase::setParams(params);
}

void dspCostedSingleLevelBOM::sFillList()
{
  dspCostedBOMBase::sFillList();

  XSqlQuery qq;
  qq.prepare( "SELECT formatCost(actcost(:item_id)) AS actual,"
              "       formatCost(stdcost(:item_id)) AS standard;" );
  qq.bindValue(":item_id", _item->id());
  qq.exec();
  if (qq.first())
  {
    XTreeWidgetItem *last = new XTreeWidgetItem(list(), -1, -1, tr("Actual Cost"), "");
    last->setText(11, qq.value("actual").toString());

    last = new XTreeWidgetItem(list(), last, -1, -1, tr("Standard Cost"), "" );
    last->setText(11, qq.value("standard").toString());
  }
  else if (qq.lastError().type() != QSqlError::NoError)
  {
    systemError(this, qq.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}
