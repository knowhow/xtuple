/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "buyCard.h"

#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include <metasql.h>
#include <openreports.h>

#include "guiclient.h"

buyCard::buyCard(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

//  (void)statusBar();

  // signals and slots connections
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_close, SIGNAL(clicked()), this, SLOT(close()));
  connect(_vendor, SIGNAL(newId(int)), this, SLOT(sHandleVendor(int)));
  connect(_itemSource, SIGNAL(newID(int)), this, SLOT(sHandleItemSource(int)));

  _item->setReadOnly(TRUE);

  _poitem->addColumn(tr("P/O #"),     _orderColumn,  Qt::AlignRight, true, "pohead_number");
  _poitem->addColumn(tr("Line"),      _whsColumn,    Qt::AlignCenter,true, "poitem_linenumber");
  _poitem->addColumn(tr("Status"),    _statusColumn, Qt::AlignCenter,true, "poitemstatus");
  _poitem->addColumn(tr("Due Date"),  _dateColumn,   Qt::AlignRight, true, "poitem_duedate");
  _poitem->addColumn(tr("Ordered"),   _qtyColumn,    Qt::AlignRight, true, "poitem_qty_ordered");
  _poitem->addColumn(tr("Received"),  _qtyColumn,    Qt::AlignRight, true, "qty_received");
  _poitem->addColumn(tr("Unit Cost"), _priceColumn,  Qt::AlignRight, true, "poitem_unitprice");
}

buyCard::~buyCard()
{
  // no need to delete child widgets, Qt does it all for us
}

void buyCard::languageChange()
{
  retranslateUi(this);
}

enum SetResponse buyCard::set(const ParameterList &pParams)
{
  XWidget::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("itemsrc_id", &valid);
  if (valid)
  {
    XSqlQuery vendid;
    vendid.prepare( "SELECT itemsrc_vend_id "
                    "FROM itemsrc "
                    "WHERE (itemsrc_id=:itemsrc_id);" );
    vendid.bindValue(":itemsrc_id", param.toInt());
    vendid.exec();
    if (vendid.first())
    {      
      _vendor->setReadOnly(TRUE);
      _item->setReadOnly(TRUE);
      _itemSource->setEnabled(FALSE);
      
      _vendor->setId(vendid.value("itemsrc_vend_id").toInt());
      _itemSource->setId(param.toInt());
    }
  }

  return NoError;
}

bool buyCard::setParams(ParameterList &pparams)
{
  pparams.append("vend_id",    _vendor->id());
  pparams.append("itemsrc_id", _itemSource->id());
  pparams.append("itemNumber", _itemSource->currentText());

  pparams.append("closed",     tr("Closed"));
  pparams.append("unposted",   tr("Unposted"));
  pparams.append("partial",    tr("Partial"));
  pparams.append("received",   tr("Received"));
  pparams.append("open",       tr("Open"));

  return true;
}

void buyCard::sPrint()
{
  ParameterList params;
  if (! setParams(params))
    return;

  orReport report("BuyCard", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void buyCard::sHandleVendor(int pVendorid)
{
  XSqlQuery itemSource;
  itemSource.prepare( "SELECT itemsrc_id, itemsrc_vend_item_number "
                      "FROM itemsrc "
                      "WHERE (itemsrc_vend_id=:vend_id) "
                      "ORDER BY itemsrc_vend_item_number;" );
  itemSource.bindValue(":vend_id", pVendorid);
  itemSource.exec();
  _itemSource->populate(itemSource);
}

void buyCard::sHandleItemSource(int pItemsrcid)
{
  XSqlQuery item;
  item.prepare( "SELECT itemsrc_vend_item_descrip, itemsrc_item_id, itemsrc_comments "
                "FROM itemsrc "
                "WHERE (itemsrc_id=:itemsrc_id);" );
  item.bindValue(":itemsrc_id", pItemsrcid);
  item.exec();
  if (item.first())
  {
    _vendorDescription->setText(item.value("itemsrc_vend_item_descrip").toString());
    _item->setId(item.value("itemsrc_item_id").toInt());
    _notes->setText(item.value("itemsrc_comments").toString());

    QString sql("SELECT poitem_id, pohead_number, poitem_linenumber,"
                "       CASE WHEN(poitem_status='C') THEN <? value(\"closed\") ?>"
                "            WHEN(poitem_status='U') THEN <? value(\"unposted\") ?>"
                "            WHEN(poitem_status='O' AND ((poitem_qty_received-poitem_qty_returned) > 0) AND (poitem_qty_ordered>(poitem_qty_received-poitem_qty_returned))) THEN <? value(\"partial\") ?>"
                "            WHEN(poitem_status='O' AND ((poitem_qty_received-poitem_qty_returned) > 0) AND (poitem_qty_ordered=(poitem_qty_received-poitem_qty_returned))) THEN <? value(\"received\") ?>"
                "            WHEN(poitem_status='O') THEN <? value(\"open\") ?>"
                "            ELSE poitem_status"
                "       END AS poitemstatus,"
                "       poitem_duedate,"
                "       poitem_qty_ordered,"
                "       COALESCE(SUM(porecv_qty), 0) AS qty_received,"
                "       poitem_unitprice,"
                "       'qty' AS poitem_qty_ordered_xtnumericrole,"
                "       'qty' AS qty_received_xtnumericrole,"
                "       'purchprice' AS poitem_unitprice_xtnumericrole "
                "FROM pohead, poitem LEFT OUTER JOIN porecv ON (porecv_poitem_id=poitem_id) "
                "WHERE ( (poitem_pohead_id=pohead_id)"
                " AND (pohead_vend_id=<? value(\"vend_id\") ?>)"
                " AND (poitem_vend_item_number=<? value(\"itemNumber\") ?>) ) "
                "GROUP BY poitem_id, pohead_number, poitem_linenumber,"
                "         poitem_status, poitem_qty_received, poitem_qty_returned,"
                "         poitem_duedate, poitem_qty_ordered, poitem_unitprice "
                "ORDER BY pohead_number, poitem_linenumber;" );
    MetaSQLQuery mql(sql);
    ParameterList params;
    if (!setParams(params))
      return;
    XSqlQuery poitem = mql.toQuery(params);
    _poitem->populate(poitem);
    if (poitem.lastError().type() != QSqlError::NoError)
    {
      systemError(this, poitem.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }
}
