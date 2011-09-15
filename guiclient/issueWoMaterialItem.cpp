/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "issueWoMaterialItem.h"

#include <QVariant>
#include <QMessageBox>
#include <QValidator>

#include "inputManager.h"
#include "distributeInventory.h"

/*
 *  Constructs a issueWoMaterialItem as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
issueWoMaterialItem::issueWoMaterialItem(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
    setupUi(this);

    // signals and slots connections
    connect(_womatl, SIGNAL(newId(int)), this, SLOT(sSetQOH(int)));
    connect(_qtyToIssue, SIGNAL(textChanged(const QString&)), this, SLOT(sPopulateQOH()));
    connect(_issue, SIGNAL(clicked()), this, SLOT(sIssue()));

    _captive = FALSE;
    _transDate->setEnabled(_privileges->check("AlterTransactionDates"));
    _transDate->setDate(omfgThis->dbDate(), true);

    omfgThis->inputManager()->notify(cBCWorkOrder, this, _wo, SLOT(setId(int)));
    omfgThis->inputManager()->notify(cBCItem, this, this, SLOT(sCatchItemid(int)));
    omfgThis->inputManager()->notify(cBCItemSite, this, this, SLOT(sCatchItemsiteid(int)));

    _wo->setType(cWoExploded | cWoIssued | cWoReleased);
    _qtyToIssue->setValidator(omfgThis->qtyVal());
    _beforeQty->setPrecision(omfgThis->transQtyVal());
    _afterQty->setPrecision(omfgThis->transQtyVal());
}

/*
 *  Destroys the object and frees any allocated resources
 */
issueWoMaterialItem::~issueWoMaterialItem()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void issueWoMaterialItem::languageChange()
{
    retranslateUi(this);
}

enum SetResponse issueWoMaterialItem::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("wo_id", &valid);
  if (valid)
  {
    _captive = TRUE;

    _wo->setId(param.toInt());
    _wo->setEnabled(false);
    _issue->setFocus();
  }

  param = pParams.value("womatl_id", &valid);
  if (valid)
  {
    _womatl->setId(param.toInt());
    _womatl->setEnabled(false);
  }

  param = pParams.value("qty", &valid);
  if (valid)
    _qtyToIssue->setDouble(param.toDouble());

  return NoError;
}

void issueWoMaterialItem::sCatchItemid(int pItemid)
{
  if (_wo->isValid())
  {
    q.prepare( "SELECT womatl_id "
               "FROM womatl, itemsite "
               "WHERE ( (womatl_itemsite_id=itemsite_id)"
               " AND (womatl_wo_id=:wo_id)"
               " AND (itemsite_item_id=:item_id) );" );
    q.bindValue(":wo_id", _wo->id());
    q.bindValue(":item_id", pItemid);
    q.exec();
    if (q.first())
      _womatl->setId(q.value("womatl_id").toInt());
    else
      audioReject();
  }
  else
    audioReject();
}

void issueWoMaterialItem::sCatchItemsiteid(int pItemsiteid)
{
  if (_wo->isValid())
  {
    q.prepare( "SELECT womatl_id "
               "FROM womatl "
               "WHERE ((womatl_itemsite_id=:itemsite_id)"
	       "  AND  (womatl_wo_id=:wo_id));" );
    q.bindValue(":itemsite_id", pItemsiteid);
    q.bindValue(":wo_id", _wo->id());
    q.exec();
    if (q.first())
      _womatl->setId(q.value("womatl_id").toInt());
    else
      audioReject();
  }
  else
    audioReject();
}

void issueWoMaterialItem::sIssue()
{
  if (!_transDate->isValid())
  {
    QMessageBox::critical(this, tr("Invalid date"),
                          tr("You must enter a valid transaction date.") );
    _transDate->setFocus();
    return;
  }
  
  q.prepare("SELECT itemsite_id, item_number, warehous_code, "
            "       (COALESCE((SELECT SUM(itemloc_qty) "
            "                    FROM itemloc "
            "                   WHERE (itemloc_itemsite_id=itemsite_id)), 0.0) >= roundQty(item_fractional, itemuomtouom(itemsite_item_id, womatl_uom_id, NULL, :qty))) AS isqtyavail "
            "  FROM womatl, itemsite, item, warehous "
            " WHERE ((womatl_itemsite_id=itemsite_id) "
            "   AND (itemsite_item_id=item_id) "
            "   AND (itemsite_warehous_id=warehous_id) "
            "   AND (NOT ((item_type = 'R') OR (itemsite_controlmethod = 'N'))) "
            "   AND ((itemsite_controlmethod IN ('L', 'S')) OR (itemsite_loccntrl)) "
            "   AND (womatl_id=:womatl_id)); ");
  q.bindValue(":womatl_id", _womatl->id());
  q.bindValue(":qty", _qtyToIssue->toDouble());
  q.exec();
  while(q.next())
  {
    if(!(q.value("isqtyavail").toBool()))
    {
      QMessageBox::critical(this, tr("Insufficient Inventory"),
        tr("Item Number %1 in Site %2 is a Multiple Location or\n"
           "Lot/Serial controlled Item which is short on Inventory.\n"
           "This transaction cannot be completed as is. Please make\n"
           "sure there is sufficient Quantity on Hand before proceeding.")
          .arg(q.value("item_number").toString())
          .arg(q.value("warehous_code").toString()));
      return;
    }
  }

  XSqlQuery rollback;
  rollback.prepare("ROLLBACK;");

  q.exec("BEGIN;");	// because of possible lot, serial, or location distribution cancelations
  q.prepare("SELECT issueWoMaterial(:womatl_id, :qty, TRUE, :date) AS result;");
  q.bindValue(":womatl_id", _womatl->id());
  q.bindValue(":qty", _qtyToIssue->toDouble());
  q.bindValue(":date",  _transDate->date());
  q.exec();
  if (q.first())
  {
    if (q.value("result").toInt() < 0)
    {
      rollback.exec();
      systemError( this, tr("A System Error occurred at issueWoMaterialItem::%1, Work Order ID #%2, Error #%3.")
                         .arg(__LINE__)
                         .arg(_wo->id())
                         .arg(q.value("result").toInt()) );
      return;
    }
    else if (distributeInventory::SeriesAdjust(q.value("result").toInt(), this) == XDialog::Rejected)
    {
      rollback.exec();
      QMessageBox::information( this, tr("Material Issue"), tr("Transaction Canceled") );
      return;
    }

    q.exec("COMMIT;");
  }
  else
  {
    rollback.exec();
    systemError( this, tr("A System Error occurred at issueWoMaterialItem::%1, Work Order ID #%2.")
                       .arg(__LINE__)
                       .arg(_wo->id()) );
    return;
  }

  if (_captive)
    close();
  else
  {
    _close->setText(tr("Close"));
    _qtyToIssue->clear();
    _womatl->setWoid(_wo->id());
    _womatl->setFocus();
  }
}

void issueWoMaterialItem::sSetQOH(int pWomatlid)
{
  if (pWomatlid == -1)
  {
    _cachedQOH = 0.0;
    _beforeQty->clear();
    _afterQty->clear();
  }
  else
  {
    if (_wo->method() == "A")
      _qtyToIssue->setDouble(_womatl->qtyRequired() - _womatl->qtyIssued());
    else
      _qtyToIssue->setDouble(_womatl->qtyIssued() * -1);
    
    XSqlQuery qoh;
    qoh.prepare( "SELECT itemuomtouom(itemsite_item_id, NULL, womatl_uom_id, itemsite_qtyonhand) AS qtyonhand,"
                 "       uom_name "
                 "  FROM womatl, itemsite, uom"
                 " WHERE((womatl_itemsite_id=itemsite_id)"
                 "   AND (womatl_uom_id=uom_id)"
                 "   AND (womatl_id=:womatl_id) );" );
    qoh.bindValue(":womatl_id", pWomatlid);
    qoh.exec();
    if (qoh.first())
    {
      _uomQty->setText(qoh.value("uom_name").toString());
      _cachedQOH = qoh.value("qtyonhand").toDouble();
      _beforeQty->setDouble(_cachedQOH);
    }
    else
      systemError(this, tr("A System Error occurred at %1::%2.")
                        .arg(__FILE__)
                        .arg(__LINE__) );
  }
  sPopulateQOH();
}

void issueWoMaterialItem::sPopulateQOH()
{
  _afterQty->setDouble(_cachedQOH - _qtyToIssue->toDouble());
}

