/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspCostedIndentedBOM.h"

#include <QSqlError>
#include <QVariant>

dspCostedIndentedBOM::dspCostedIndentedBOM(QWidget* parent, const char*, Qt::WFlags fl)
    : dspCostedBOMBase(parent, "dspCostedIndentedBOM", fl)
{
  setWindowTitle(tr("Costed Indented Bill of Materials"));
  setReportName("CostedIndentedBOM");
  list()->setRootIsDecorated(true);
  list()->setIndentation(10);
  list()->setPopulateLinear(true);
}

bool dspCostedIndentedBOM::setParams(ParameterList &params)
{
  params.append("indentedBOM");

  return dspCostedBOMBase::setParams(params);
}

void dspCostedIndentedBOM::sFillList()
{
  dspCostedBOMBase::sFillList();

  XSqlQuery qq;
  qq.prepare( "SELECT formatCost(SUM(bomdata_actextendedcost)) AS actextendedcost,"
             "       formatCost(SUM(bomdata_stdextendedcost)) AS stdextendedcost,"
             "       formatCost(actcost(:item_id)) AS actual,"
             "       formatCost(stdcost(:item_id)) AS standard "
             "FROM indentedbom(:item_id,"
             "                 :revision_id,0,0)"
             "WHERE (bomdata_bomwork_level=1) "
             "GROUP BY actual, standard;" );
  qq.bindValue(":item_id", _item->id());
  qq.bindValue(":revision_id",_revision->id());
  qq.exec();
  if (qq.first())
  {
    XTreeWidgetItem *last = new XTreeWidgetItem(list(), -1, -1);
    last->setText(1, tr("Total Cost"));
    if(_useStandardCosts->isChecked())
      last->setText(11, qq.value("stdextendedcost").toString());
    else
      last->setText(11, qq.value("actextendedcost").toString());

    last = new XTreeWidgetItem( list(), -1, -1);
    last->setText(1, tr("Actual Cost"));
    last->setText(11, qq.value("actual").toString());

    last = new XTreeWidgetItem( list(), -1, -1);
    last->setText(1, tr("Standard Cost"));
    last->setText(11, qq.value("standard").toString());
  }
  else if (qq.lastError().type() != QSqlError::NoError)
  {
    systemError(this, qq.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
  list()->expandAll();
}

