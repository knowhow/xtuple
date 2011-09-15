/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspCountSlipsByWarehouse.h"

#include <QMessageBox>
#include <QVariant>

#include "xtreewidget.h"

#define DEBUG true

dspCountSlipsByWarehouse::dspCountSlipsByWarehouse(QWidget* parent, const char* name, Qt::WFlags fl)
  : display(parent, "dspCountSlipsByWarehouse", fl)
{
  setupUi(optionsWidget());
  setWindowTitle(tr("Count Slips by Site"));
  setListLabel(tr("Count Slips"));
  setReportName("CountSlipsByWarehouse");
  setMetaSQLOptions("countSlip", "detail");

  // before connect so we don't repeatedly trigger the query
  if (_preferences->boolean("XCheckBox/forgetful"))
    _showUnposted->setChecked(true);
  _dates->setStartNull(tr("Earliest"), omfgThis->startOfTime(), true);
  _dates->setEndNull(tr("Latest"), omfgThis->endOfTime(), true);

  list()->addColumn(tr("Slip #"),     _itemColumn, Qt::AlignLeft,  true, "cntslip_number");
  list()->addColumn(tr("Tag #"),     _orderColumn, Qt::AlignLeft,  true, "invcnt_tagnumber");
  list()->addColumn(tr("Site"),        _whsColumn, Qt::AlignCenter,true, "warehous_code");
  list()->addColumn(tr("Item"),       _itemColumn, Qt::AlignLeft,  true, "item_number");
  list()->addColumn(tr("Description"),         -1, Qt::AlignLeft,  true, "descrip");
  list()->addColumn(tr("Entered"),    _dateColumn, Qt::AlignCenter,true, "cntslip_entered");
  list()->addColumn(tr("By"),         _userColumn, Qt::AlignCenter,true, "user");
  list()->addColumn(tr("Qty. Counted"),_qtyColumn, Qt::AlignRight, true, "cntslip_qty");
  list()->addColumn(tr("Posted"),     _dateColumn, Qt::AlignCenter,true, "cntslip_posted");

  sFillList();
}

void dspCountSlipsByWarehouse::languageChange()
{
  display::languageChange();
  retranslateUi(this);
}

void dspCountSlipsByWarehouse::sFillList()
{
  if (DEBUG)
    qDebug("dspCountSlipsByWarehouse::sFillList() about to populate _cntslip");

  display::sFillList();

  if (_numericSlips->isChecked() && list()->topLevelItemCount() > 1)
  {
    if (DEBUG)
      qDebug("dspCountSlipsByWarehouse::sFillList() looking for slip # gaps "
             "in %d items", list()->topLevelItemCount());
    XTreeWidgetItem *last =list()->topLevelItem(0);
    int slipNumber = last->rawValue("slipnumber").toInt();
    for (int i = 1; i < list()->topLevelItemCount(); i++)
    {
      XTreeWidgetItem *curr = list()->topLevelItem(i);
      if (DEBUG)
        qDebug("row %d has slipNumber %d and current %d",
               i, slipNumber, curr->rawValue("slipnumber").toInt());
      if (slipNumber == (curr->rawValue("slipnumber").toInt() - 1) || slipNumber == -1)
        slipNumber = curr->rawValue("slipnumber").toInt();
      else if (slipNumber >= 0)
      {
        if (slipNumber == curr->rawValue("slipnumber").toInt() - 2)
          curr = new XTreeWidgetItem( list(), last, -1,
                                    QVariant("----"), "----", "----", "----",
                                    tr("Missing Slip #%1").arg(slipNumber + 1),
                                    "----", "----", "----" );
        else
          curr = new XTreeWidgetItem( list(), last, -1,
                                    QVariant("----"), "----", "----", "----",
                                    tr("Missing Slips #%1 to #%2").arg(slipNumber + 1).arg(curr->rawValue("slipnumber").toInt() - 1),
                                    "----", "----", "----" );

        curr->setTextColor(namedColor("error"));
        slipNumber = -1;
        //i++; // 'cause we just added an item!
      }
      last = curr;
    }
  }
}

bool dspCountSlipsByWarehouse::setParams(ParameterList &params)
{
  if (isVisible())
  {
    if (!_dates->startDate().isValid())
    {
      QMessageBox::warning( this, tr("Enter Start Date"),
                            tr("Please enter a valid Start Date.") );
      _dates->setFocus();
      return false;
    }
    else if (!_dates->endDate().isValid())
    {
      QMessageBox::warning( this, tr("Enter End Date"),
                            tr("Please enter a valid End Date.") );
      _dates->setFocus();
      return false;
    }
	else
      _dates->appendValue(params);
  }
  
  _warehouse->appendValue(params);

  if (_showUnposted->isChecked())
    params.append("showUnposted");

  params.append("byWarehouse");

  return true;
}
