/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "correctProductionPosting.h"

#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include "closeWo.h"
#include "distributeInventory.h"
#include "inputManager.h"
#include "storedProcErrorLookup.h"

correctProductionPosting::correctProductionPosting(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_correct, SIGNAL(clicked()), this, SLOT(sCorrect()));
  connect(_wo,     SIGNAL(newId(int)), this, SLOT(populate()));

  _captive = FALSE;
  _transDate->setEnabled(_privileges->check("AlterTransactionDates"));
  _transDate->setDate(omfgThis->dbDate(), true);
  _qtyReceivedCache = 0.0;

  omfgThis->inputManager()->notify(cBCWorkOrder, this, _wo, SLOT(setId(int)));

  _wo->setType(cWoIssued);
  _qty->setValidator(omfgThis->transQtyVal());
  _qtyOrdered->setPrecision(omfgThis->transQtyVal());
  _qtyReceived->setPrecision(omfgThis->transQtyVal());
  _qtyBalance->setPrecision(omfgThis->transQtyVal());

  if (_preferences->boolean("XCheckBox/forgetful"))
    _backFlush->setChecked(true);
  _nonPickItems->setEnabled(_backFlush->isChecked() &&
			    _privileges->check("ChangeNonPickItems"));

  // TODO: unhide as part of implementation of 5847
  _nonPickItems->hide();

  adjustSize();
}

correctProductionPosting::~correctProductionPosting()
{
  // no need to delete child widgets, Qt does it all for us
}

void correctProductionPosting::languageChange()
{
  retranslateUi(this);
}

enum SetResponse correctProductionPosting::set(const ParameterList &pParams)
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

    _qty->setFocus();
  }

  return NoError;
}

bool correctProductionPosting::okToPost()
{
  if (!_transDate->isValid())
  {
    QMessageBox::critical(this, tr("Invalid date"),
                          tr("You must enter a valid transaction date.") );
    _transDate->setFocus();
    return false;
  }
  else if (_qty->toDouble() > _qtyReceivedCache)
  {
    QMessageBox::warning( this, tr("Cannot Post Correction"),
                          tr( "The Quantity to correct must be less than or equal to the Quantity already Posted." ) );
    _qty->setFocus();
    return false;
  }

  XSqlQuery itemtypeq;
  itemtypeq.prepare( "SELECT itemsite_costmethod "
             "FROM wo, itemsite "
             "WHERE ( (wo_itemsite_id=itemsite_id)"
             " AND (wo_id=:wo_id) );" );
  itemtypeq.bindValue(":wo_id", _wo->id());
  itemtypeq.exec();
  if (itemtypeq.first() && (itemtypeq.value("itemsite_costmethod").toString() == "J"))
  {
    QMessageBox::warning(this, tr("Cannot Post Correction"),
                         tr("You may not post a correction to a Work Order for a "
                            "Item Site with the Job cost method. You must, "
                            "instead, adjust shipped quantities."));
    return false;
  }
  else if (itemtypeq.lastError().type() != QSqlError::NoError)
  {
    systemError(this, itemtypeq.lastError().databaseText(), __FILE__, __LINE__);
    return false;
  }

  return true;
}

void correctProductionPosting::clear()
{
  _wo->setId(-1);
  _qty->clear();
  _close->setText(tr("&Close"));
  _wo->setFocus();
}

void correctProductionPosting::sCorrect()
{
  if (! okToPost())
    return;

  XSqlQuery rollback;
  rollback.prepare("ROLLBACK;");

  q.exec("BEGIN;");	// handle cancel of lot, serial, or loc distribution
  q.prepare("SELECT correctProduction(:wo_id, :qty, :backflushMaterials, 0, :date)"
            "        AS result;");
  q.bindValue(":wo_id", _wo->id());
  if (_wo->method() == "A")
    q.bindValue(":qty", _qty->toDouble());
  else
    q.bindValue(":qty", _qty->toDouble() * -1);
  q.bindValue(":backflushMaterials",  QVariant(_backFlush->isChecked()));
  q.bindValue(":date",  _transDate->date());
  q.exec();
  if (q.first())
  {
    int result = q.value("result").toInt();
    if (result < 0)
    {
      rollback.exec();
      systemError(this, storedProcErrorLookup("correctProduction", result),
                  __FILE__, __LINE__);
      return;
    }

    if (distributeInventory::SeriesAdjust(result, this) == XDialog::Rejected)
    {
      rollback.exec();
      QMessageBox::information( this, tr("Correct Production Posting"), tr("Transaction Canceled") );
      return;
    }

    q.exec("COMMIT;");
    omfgThis->sWorkOrdersUpdated(_wo->id(), TRUE);
  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    rollback.exec();
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  if (_captive)
    accept();
  else
    clear();
}

void correctProductionPosting::populate()
{
  if (_wo->id() != -1)
  {
    if (_wo->method() == "D")
    {
      _backFlush->setEnabled(false);
      _backFlush->setChecked(false);
      _qtyOrderedLit->setText(tr("Qty. to Disassemble:"));
      _qtyReceivedLit->setText(tr("Qty. Disassembled:"));
    }
    else
    {
      _backFlush->setEnabled(true);
      _qtyOrderedLit->setText(tr("Qty. Ordered:"));
      _qtyReceivedLit->setText(tr("Qty. Received:"));
    }
    
    _qtyOrdered->setText(_wo->qtyOrdered());
    _qtyReceived->setText(_wo->qtyReceived());
    _qtyBalance->setText(_wo->qtyBalance());

    _qtyReceivedCache = _wo->qtyReceived();
  }
  else
  {
    _qtyOrdered->clear();
    _qtyReceived->clear();
    _qtyBalance->clear();

    _qtyReceivedCache = 0;
  }
}
