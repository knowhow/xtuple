/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspQuotesByItem.h"

#include <QAction>
#include <QMenu>
#include <QMessageBox>

#include "salesOrder.h"

dspQuotesByItem::dspQuotesByItem(QWidget* parent, const char*, Qt::WFlags fl)
  : display(parent, "dspQuotesByItem", fl)
{
  setupUi(optionsWidget());
  setWindowTitle(tr("Quote Lookup by Item"));
  setListLabel(tr("Quotes"));
  setMetaSQLOptions("quoteItems", "detail");
  setUseAltId(true);

  _dates->setStartNull(tr("Earliest"), omfgThis->startOfTime(), true);
  _dates->setStartCaption(tr("Starting Order Date"));
  _dates->setEndNull(tr("Latest"), omfgThis->endOfTime(), true);
  _dates->setEndCaption(tr("Ending Order Date"));

  _item->setType(ItemLineEdit::cSold);

  list()->addColumn(tr("Quote #"),    _orderColumn,   Qt::AlignLeft,   true,  "quhead_number"   );
  list()->addColumn(tr("Quote Date"), _dateColumn,    Qt::AlignCenter, true,  "quhead_quotedate" );
  list()->addColumn(tr("Customer"),   -1,             Qt::AlignLeft,   true,  "cust_name"   );
  list()->addColumn(tr("Status"),     _statusColumn,  Qt::AlignCenter, true,  "quhead_status" );
  list()->addColumn(tr("Quoted"),     _qtyColumn,     Qt::AlignRight,  true,  "quitem_qtyord"  );

  connect(omfgThis, SIGNAL(salesOrdersUpdated(int, bool)), this, SLOT(sFillList())); 
}

void dspQuotesByItem::languageChange()
{
  display::languageChange();
  retranslateUi(this);
}

enum SetResponse dspQuotesByItem::set(const ParameterList &pParams)
{
  XWidget::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("item_id", &valid);
  if (valid)
    _item->setId(param.toInt());

  param = pParams.value("startDate", &valid);
  if (valid)
    _dates->setStartDate(param.toDate());

  param = pParams.value("endDate", &valid);
  if (valid)
    _dates->setEndDate(param.toDate());

  return NoError;
}

void dspQuotesByItem::sPopulateMenu(QMenu *menuThis, QTreeWidgetItem*, int)
{
  menuThis->addAction(tr("Edit..."), this, SLOT(sEditOrder()));
  menuThis->addAction(tr("View..."), this, SLOT(sViewOrder()));
}

void dspQuotesByItem::sEditOrder()
{
  if (!checkSitePrivs(list()->altId()))
    return;
    
  ParameterList params;
  params.append("mode", "editQuote");
  params.append("quhead_id", list()->altId());
      
  salesOrder *newdlg = new salesOrder();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspQuotesByItem::sViewOrder()
{
  if (!checkSitePrivs(list()->altId()))
    return;
    
  ParameterList params;
  params.append("mode", "viewQuote");
  params.append("quhead_id", list()->altId());
      
  salesOrder *newdlg = new salesOrder();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

bool dspQuotesByItem::setParams(ParameterList & params)
{
  if (!_item->isValid())
  {
    QMessageBox::warning(this, tr("Item Required"),
      tr("You must specify an Item Number"));
    return false;
  }

  if(!_dates->allValid())
  {
    QMessageBox::warning(this, tr("Dates Required"),
      tr("You must specify a Start Date and End Date."));
    return false;
  }

  _dates->appendValue(params);
  params.append("item_id", _item->id());
  if (_showConverted->isChecked())
    params.append("showConverted", true);

  return true;
}

bool dspQuotesByItem::checkSitePrivs(int orderid)
{
  if (_preferences->boolean("selectedSites"))
  {
    XSqlQuery check;
    check.prepare("SELECT checkQuoteSitePrivs(:quheadid) AS result;");
    check.bindValue(":quheadid", orderid);
    check.exec();
    if (check.first())
    {
    if (!check.value("result").toBool())
      {
        QMessageBox::critical(this, tr("Access Denied"),
                                       tr("You may not view, edit, or convert this Quote as it references "
                                       "a Site for which you have not been granted privileges.")) ;
        return false;
      }
    }
  }
  return true;
}

