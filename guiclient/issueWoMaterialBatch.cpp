/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "issueWoMaterialBatch.h"

#include <QVariant>
#include <QMessageBox>
#include <metasql.h>
#include "inputManager.h"
#include "distributeInventory.h"

issueWoMaterialBatch::issueWoMaterialBatch(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  _captive = false;

  // signals and slots connections
  connect(_issue, SIGNAL(clicked()), this, SLOT(sIssue()));
  connect(_wo, SIGNAL(newId(int)), this, SLOT(sFillList()));
  connect(_close, SIGNAL(clicked()), this, SLOT(close()));

  _hasPush = FALSE;
  _transDate->setEnabled(_privileges->check("AlterTransactionDates"));
  _transDate->setDate(omfgThis->dbDate(), true);
  _wo->setType(cWoExploded | cWoIssued | cWoReleased);

  omfgThis->inputManager()->notify(cBCWorkOrder, this, _wo, SLOT(setId(int)));

  _womatl->addColumn(tr("Item Number"),  _itemColumn, Qt::AlignLeft,   true,  "item_number"   );
  _womatl->addColumn(tr("Description"),  -1,          Qt::AlignLeft,   true,  "itemdescrip"   );
  _womatl->addColumn(tr("UOM"),          _uomColumn,  Qt::AlignCenter, true,  "uom_name" );
  _womatl->addColumn(tr("Issue Method"), _itemColumn, Qt::AlignCenter, true,  "issuemethod" );
  _womatl->addColumn(tr("Picklist"),     _itemColumn, Qt::AlignCenter, true,  "picklist" );
  _womatl->addColumn(tr("Required"),     _qtyColumn,  Qt::AlignRight,  true,  "required"  );
  _womatl->addColumn(tr("QOH"),          _qtyColumn,  Qt::AlignRight,  true,  "itemsite_qtyonhand"  );
  _womatl->addColumn(tr("Short"),        _qtyColumn,  Qt::AlignRight,  true,  "short"  );
}

issueWoMaterialBatch::~issueWoMaterialBatch()
{
  // no need to delete child widgets, Qt does it all for us
}

void issueWoMaterialBatch::languageChange()
{
  retranslateUi(this);
}

enum SetResponse issueWoMaterialBatch::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("wo_id", &valid);
  if (valid)
  {
    _captive = TRUE;

    _wo->setId(param.toInt());
    _wo->setReadOnly(TRUE);
    _issue->setFocus();
  }

  return NoError;
}

void issueWoMaterialBatch::sIssue()
{
  if (!_transDate->isValid())
  {
    QMessageBox::critical(this, tr("Invalid date"),
                          tr("You must enter a valid transaction date.") );
    _transDate->setFocus();
    return;
  }
  
  QString sqlissue =
        ( "SELECT itemsite_id, "
          "       item_number, "
          "       warehous_code, "
          "       (COALESCE((SELECT SUM(itemloc_qty) "
          "                    FROM itemloc "
          "                   WHERE (itemloc_itemsite_id=itemsite_id)), 0.0) "
          "        >= roundQty(item_fractional, noNeg(itemuomtouom(itemsite_item_id, womatl_uom_id, NULL, womatl_qtyreq - womatl_qtyiss)))) AS isqtyavail "
          "  FROM womatl, itemsite, item, warehous "
          " WHERE ( (womatl_itemsite_id=itemsite_id) "
          "   AND (itemsite_item_id=item_id) "
          "   AND (itemsite_warehous_id=warehous_id) "
          "   AND (NOT ((item_type = 'R') OR (itemsite_controlmethod = 'N'))) "
          "   AND ((itemsite_controlmethod IN ('L', 'S')) OR (itemsite_loccntrl)) "
          "   AND (womatl_issuemethod IN ('S', 'M')) "
          "  <? if exists(\"pickItemsOnly\") ?>"
          "   AND (womatl_picklist) "
          "  <? endif ?>"
          "   AND (womatl_wo_id=<? value(\"wo_id\") ?>)); ");
  MetaSQLQuery mqlissue(sqlissue);
  ParameterList params;
  params.append("wo_id", _wo->id());
  if (!_nonPickItems->isChecked())
    params.append("pickItemsOnly", true);
  XSqlQuery issue = mqlissue.toQuery(params);
  while(issue.next())
  {
    if(!(issue.value("isqtyavail").toBool()))
    {
      QMessageBox::critical(this, tr("Insufficient Inventory"),
        tr("Item Number %1 in Site %2 is a Multiple Location or\n"
           "Lot/Serial controlled Item which is short on Inventory.\n"
           "This transaction cannot be completed as is. Please make\n"
           "sure there is sufficient Quantity on Hand before proceeding.")
          .arg(issue.value("item_number").toString())
          .arg(issue.value("warehous_code").toString()));
      return;
    }
  }

  XSqlQuery rollback;
  rollback.prepare("ROLLBACK;");

  QString sqlitems =
               ("SELECT womatl_id,"
                "       CASE WHEN (womatl_qtyreq >= 0) THEN"
                "         roundQty(itemuomfractionalbyuom(item_id, womatl_uom_id), noNeg(womatl_qtyreq - womatl_qtyiss))"
                "       ELSE"
                "         roundQty(itemuomfractionalbyuom(item_id, womatl_uom_id), noNeg(womatl_qtyiss * -1))"
                "       END AS qty"
                "  FROM womatl, itemsite, item"
                " WHERE((womatl_itemsite_id=itemsite_id)"
                "   AND (itemsite_item_id=item_id)"
                "   AND (womatl_issuemethod IN ('S', 'M'))"
                "  <? if exists(\"pickItemsOnly\") ?>"
                "   AND (womatl_picklist) "
                "  <? endif ?>"
                "   AND (womatl_wo_id=<? value(\"wo_id\") ?>)); ");
  MetaSQLQuery mqlitems(sqlitems);
  XSqlQuery items = mqlitems.toQuery(params);
  while(items.next())
  {
    issue.exec("BEGIN;");	// because of possible lot, serial, or location distribution cancelations
    issue.prepare("SELECT issueWoMaterial(:womatl_id, :qty, 0, TRUE, :date) AS result;");
    issue.bindValue(":womatl_id", items.value("womatl_id").toInt());
    issue.bindValue(":qty", items.value("qty").toDouble());
    issue.bindValue(":date",  _transDate->date());
    issue.exec();
  
    if (issue.first())
    {
      if (issue.value("result").toInt() < 0)
      {
        rollback.exec();
        systemError( this, tr("A System Error occurred at issueWoMaterialBatch::%1, Work Order ID #%2, Error #%3.")
                           .arg(__LINE__)
                           .arg(_wo->id())
                           .arg(issue.value("result").toInt()) );
        return;
      }
      if (distributeInventory::SeriesAdjust(issue.value("result").toInt(), this) == XDialog::Rejected)
      {
        rollback.exec();
        QMessageBox::information( this, tr("Material Issue"), tr("Transaction Canceled") );
        return;
      }
    }
    else
    {
      rollback.exec();
      systemError( this, tr("A System Error occurred at issueWoMaterialBatch::%1, Work Order ID #%2.")
                         .arg(__LINE__)
                         .arg(_wo->id()) );
      return;
    }
    issue.exec("COMMIT;");
  }

  omfgThis->sWorkOrdersUpdated(_wo->id(), TRUE);

  if (_captive)
    accept();
  else
  {
    _wo->setId(-1);
    _womatl->clear();
    _wo->setFocus();
  }
}

void issueWoMaterialBatch::sFillList()
{
  _womatl->clear();
  _issue->setEnabled(false);

  if (_wo->isValid())
  {
    QTreeWidgetItem * hitem = _womatl->headerItem();
    if (_wo->method() == "A")
      hitem->setText(5, tr("Required"));
    else
      hitem->setText(5, tr("Returned"));
     
    XSqlQuery womatl;
    womatl.prepare( "SELECT womatl_id, item_number,"
                    " (item_descrip1 || ' ' || item_descrip2) AS itemdescrip, uom_name,"
                    " CASE WHEN (womatl_issuemethod = 'S') THEN :push"
                    "      WHEN (womatl_issuemethod = 'L') THEN :pull"
                    "      WHEN (womatl_issuemethod = 'M') THEN :mixed"
                    "      ELSE :error"
                    " END AS issuemethod,"
                    " womatl_picklist AS picklist,"
                    " CASE "
                    "   WHEN (womatl_qtyreq >= 0) THEN "
                    "     noNeg(womatl_qtyreq - womatl_qtyiss) "
                    "   ELSE "
                    "     (womatl_qtyiss * -1) "
                    "   END AS required,"
                    " itemsite_qtyonhand,"
                    " abs(noneg((itemsite_qtyonhand - womatl_qtyreq) * -1)) AS short,"
                    " 'qty' AS required_xtnumericrole,"
                    " 'qty' AS itemsite_qtyonhand_xtnumericrole,"
                    " 'qty' AS short_xtnumericrole,"
                    " CASE WHEN (womatl_issuemethod = 'L') THEN 'blue'"
                    " END AS issuemethod_qtforegroundrole, "
                    " CASE WHEN (abs(noneg((itemsite_qtyonhand - womatl_qtyreq) * -1)) > 0.0) THEN 'red'"
                    " END AS short_qtforegroundrole "
                    "FROM womatl, itemsite, item, uom "
                    "WHERE ((womatl_itemsite_id=itemsite_id)"
                    " AND (itemsite_item_id=item_id)"
                    " AND (womatl_uom_id=uom_id)"
                    " AND (womatl_wo_id=:wo_id)) "
                    "ORDER BY item_number;" );
    womatl.bindValue(":push", tr("Push"));
    womatl.bindValue(":pull", tr("Pull"));
    womatl.bindValue(":mixed", tr("Mixed"));
    womatl.bindValue(":error", tr("Error"));
    womatl.bindValue(":wo_id", _wo->id());
    womatl.exec();
    _womatl->populate(womatl);
    _issue->setEnabled(true);
  }
}
