/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "returnWoMaterialItem.h"

#include <QVariant>
#include <QMessageBox>
#include <QSqlError>

#include "distributeInventory.h"
#include "inputManager.h"
#include "storedProcErrorLookup.h"

returnWoMaterialItem::returnWoMaterialItem(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_return, SIGNAL(clicked()), this, SLOT(sReturn()));
  connect(_womatl, SIGNAL(newId(int)), this, SLOT(sSetQOH(int)));
  connect(_qty, SIGNAL(textChanged(const QString&)), this, SLOT(sUpdateQty()));

  _captive = FALSE;
  _transDate->setEnabled(_privileges->check("AlterTransactionDates"));
  _transDate->setDate(omfgThis->dbDate(), true);

  omfgThis->inputManager()->notify(cBCWorkOrder, this, _wo, SLOT(setId(int)));

  _qty->setValidator(omfgThis->qtyVal());
  _beforeQty->setPrecision(omfgThis->transQtyVal());
  _afterQty->setPrecision(omfgThis->transQtyVal());

  // Change to Issued only
  //_wo->setType(cWoExploded | cWoReleased | cWoIssued);
  _wo->setType(cWoIssued);
  _wo->setFocus();
}

returnWoMaterialItem::~returnWoMaterialItem()
{
  // no need to delete child widgets, Qt does it all for us
}

void returnWoMaterialItem::languageChange()
{
  retranslateUi(this);
}

enum SetResponse returnWoMaterialItem::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  _captive = TRUE;

  QVariant param;
  bool     valid;

  param = pParams.value("womatl_id", &valid);
  if (valid)
  {
    q.prepare("SELECT womatl_wo_id FROM womatl WHERE (womatl_id=:womatl_id); ");
    q.bindValue(":womatl_id", param.toInt());
    q.exec();
    if(q.first())
    {
      _wo->setId(q.value("womatl_wo_id").toInt());
      _wo->setEnabled(false);
    }
   if (valid)
   {
    _womatl->setId(param.toInt());
    _womatl->setEnabled(false);
   }
    _qty->setFocus();
  }

  return NoError;
}

void returnWoMaterialItem::sReturn()
{
  if (!_transDate->isValid())
  {
    QMessageBox::critical(this, tr("Invalid date"),
                          tr("You must enter a valid transaction date.") );
    _transDate->setFocus();
    return;
  }
  else if (!_wo->isValid())
  {
    QMessageBox::critical( this, tr("Select Work Order"),
                           tr("You must select the Work Order from which you with to return Materal") );
    _wo->setFocus();
    return;
  }

  XSqlQuery rollback;
  rollback.prepare("ROLLBACK;");

  XSqlQuery returnItem;
  returnItem.exec("BEGIN;");	// because of possible lot, serial, or location distribution cancelations
  returnItem.prepare("SELECT returnWoMaterial(:womatl_id, :qty, :date) AS result;");
  returnItem.bindValue(":womatl_id", _womatl->id());
  returnItem.bindValue(":qty", _qty->toDouble());
  returnItem.bindValue(":date",  _transDate->date());
  returnItem.exec();
  if (returnItem.first())
  {
    int result = returnItem.value("result").toInt();
    if (result < 0)
    {
      rollback.exec();
      systemError(this, storedProcErrorLookup("returnWoMaterial", result),
                  __FILE__, __LINE__);
      return;
    }
    else if (distributeInventory::SeriesAdjust(returnItem.value("result").toInt(), this) == XDialog::Rejected)
    {
      rollback.exec();
      QMessageBox::information( this, tr("Material Return"), tr("Transaction Canceled") );
      return;
    }

    returnItem.exec("COMMIT;");
  }
  else if (returnItem.lastError().type() != QSqlError::NoError)
  {
    rollback.exec();
    systemError(this, returnItem.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  if (_captive)
    accept();
  else
  {
    _qty->clear();
    _close->setText(tr("&Close"));
    _womatl->setWoid(_wo->id());
    _womatl->setFocus();
  }
}

void returnWoMaterialItem::sSetQOH(int pWomatlid)
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
      _qty->setDouble(_womatl->qtyIssued());
    else
      _qty->setDouble((_womatl->qtyIssued() - _womatl->qtyRequired()));
      
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
      _uom->setText(qoh.value("uom_name").toString());
      _cachedQOH = qoh.value("qtyonhand").toDouble();
      _beforeQty->setDouble(_cachedQOH);
    }
    else
      systemError(this, tr("A System Error occurred at %1::%2.")
                        .arg(__FILE__)
                        .arg(__LINE__) );
  }
  sUpdateQty();
}

void returnWoMaterialItem::sUpdateQty()
{
  if (_womatl->isValid())
    _afterQty->setDouble(_cachedQOH + _qty->toDouble());
}

