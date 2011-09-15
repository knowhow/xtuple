/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "materialReceiptTrans.h"

#include <QMessageBox>
#include <QSqlError>
#include <QValidator>
#include <QVariant>

#include "inputManager.h"
#include "distributeInventory.h"
#include "issueWoMaterialItem.h"
#include "storedProcErrorLookup.h"

materialReceiptTrans::materialReceiptTrans(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

//  (void)statusBar();

  connect(_item,      SIGNAL(newId(int)), this, SLOT(sPopulateQty()));
  connect(_post,       SIGNAL(clicked()), this, SLOT(sPost()));
  connect(_warehouse, SIGNAL(newID(int)), this, SLOT(sPopulateQty()));
  connect(_cost, SIGNAL(textChanged(const QString&)), this, SLOT(sCostUpdated()));

  _captive = FALSE;

  _item->setType(ItemLineEdit::cGeneralInventory | ItemLineEdit::cActive);
  _warehouse->setType(WComboBox::AllActiveInventory);
  _qty->setValidator(omfgThis->qtyVal());
  _beforeQty->setPrecision(omfgThis->qtyVal());
  _afterQty->setPrecision(omfgThis->qtyVal());
  _unitCost->setPrecision(omfgThis->costVal());
  _cost->setValidator(omfgThis->costVal());
  _wo->setType(cWoOpen | cWoExploded | cWoReleased | cWoIssued);

  omfgThis->inputManager()->notify(cBCWorkOrder, this, _wo, SLOT(setId(int)));
  omfgThis->inputManager()->notify(cBCItem, this, _item, SLOT(setItemid(int)));
  omfgThis->inputManager()->notify(cBCItemSite, this, _item, SLOT(setItemsiteid(int)));

  if (!_metrics->boolean("MultiWhs"))
  {
    _warehouseLit->hide();
    _warehouse->hide();
  }

  if (!_metrics->boolean("AllowAvgCostMethod"))
    _tab->removeTab(0);

  _item->setFocus();
}

materialReceiptTrans::~materialReceiptTrans()
{
  // no need to delete child widgets, Qt does it all for us
}

void materialReceiptTrans::languageChange()
{
  retranslateUi(this);
}

enum SetResponse materialReceiptTrans::set(const ParameterList &pParams)
{
  XWidget::set(pParams);
  QVariant param;
  bool     valid;
  int      _invhistid = -1;

  param = pParams.value("invhist_id", &valid);
  if (valid)
    _invhistid = param.toInt();

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      _mode = cNew;

      setWindowTitle(tr("Enter Material Receipt"));
      _usernameLit->clear();
      _transDate->setEnabled(_privileges->check("AlterTransactionDates"));
      _transDate->setDate(omfgThis->dbDate());

      connect(_qty, SIGNAL(textChanged(const QString &)), this, SLOT(sUpdateQty(const QString &)));
      connect(_qty, SIGNAL(textChanged(const QString &)), this, SLOT(sCostUpdated()));
      connect(_warehouse, SIGNAL(newID(int)), this, SLOT(sPopulateQty()));
      connect(_issueToWo, SIGNAL(toggled(bool)), this, SLOT(sPopulateQty()));
    }
    else if (param.toString() == "view")
    {
      _mode = cView;

      setWindowTitle(tr("Material Receipt"));
      _transDate->setEnabled(FALSE);
      _item->setReadOnly(TRUE);
      _warehouse->setEnabled(FALSE);
      _qty->setEnabled(FALSE);
      _issueToWo->setEnabled(FALSE);
      _wo->setReadOnly(TRUE);
      _documentNum->setEnabled(FALSE);
      _notes->setEnabled(FALSE);
      _post->hide();
      _close->setText(tr("&Close"));
      _close->setFocus();

      XSqlQuery popq;
      popq.prepare( "SELECT * "
                 "FROM invhist "
                 "WHERE (invhist_id=:invhist_id);" );
      popq.bindValue(":invhist_id", _invhistid);
      popq.exec();
      if (popq.first())
      {
        // _item first so it doesn't trigger sPopulateQty
        _item->setItemsiteid(popq.value("invhist_itemsite_id").toInt());
        _transDate->setDate(popq.value("invhist_transdate").toDate());
        _username->setText(popq.value("invhist_user").toString());
        _qty->setDouble(popq.value("invhist_invqty").toDouble());
        _beforeQty->setDouble(popq.value("invhist_qoh_before").toDouble());
        _afterQty->setDouble(popq.value("invhist_qoh_after").toDouble());
        _documentNum->setText(popq.value("invhist_ordnumber"));
        _notes->setText(popq.value("invhist_comments").toString());
      }
      else if (popq.lastError().type() != QSqlError::NoError)
      {
	systemError(this, popq.lastError().databaseText(), __FILE__, __LINE__);
	return UndefinedError;
      }
    }
  }

  return NoError;
}

void materialReceiptTrans::sPost()
{
  double cost = _cost->toDouble();

  struct {
    bool        condition;
    QString     msg;
    QWidget     *widget;
  } error[] = {
    { ! _item->isValid(),
      tr("You must select an Item before posting this transaction."), _item },
    { _qty->text().length() == 0 || _qty->toDouble() <= 0,
      tr("<p>You must enter a positive Quantity before posting this Transaction."),
      _qty },
    { _costAdjust->isEnabled() && _costAdjust->isChecked() && _costManual->isChecked() && (_cost->text().length() == 0 || cost ==0),
      tr("<p>You must enter a total cost value for the inventory to be transacted."),
      _cost },
    { true, "", NULL }
  };

  int errIndex;
  for (errIndex = 0; ! error[errIndex].condition; errIndex++)
    ;
  if (! error[errIndex].msg.isEmpty())
  {
    QMessageBox::critical(this, tr("Cannot Post Transaction"),
                          error[errIndex].msg);
    error[errIndex].widget->setFocus();
    return;
  }
  
  XSqlQuery rollback;
  rollback.prepare("ROLLBACK;");

  q.exec("BEGIN;");	// because of possible distribution cancelations
  q.prepare( "SELECT invReceipt(itemsite_id, :qty, '', :docNumber,"
             "                  :comments, :date, :cost) AS result "
             "FROM itemsite "
             "WHERE ( (itemsite_item_id=:item_id)"
             " AND (itemsite_warehous_id=:warehous_id) );" );
  q.bindValue(":qty", _qty->toDouble());
  q.bindValue(":docNumber", _documentNum->text());
  q.bindValue(":comments", _notes->toPlainText());
  q.bindValue(":item_id", _item->id());
  q.bindValue(":warehous_id", _warehouse->id());
  q.bindValue(":date",        _transDate->date());
  if(!_costAdjust->isChecked())
    q.bindValue(":cost", 0.0);
  else if(_costManual->isChecked())
    q.bindValue(":cost", cost);
  q.exec();
  if (q.first())
  {
    int result = q.value("result").toInt();
    if (result < 0)
    {
      rollback.exec();
      systemError(this, storedProcErrorLookup("invReceipt", result),
                  __FILE__, __LINE__);
      return;
    }
    else if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }

    if (distributeInventory::SeriesAdjust(q.value("result").toInt(), this) == XDialog::Rejected)
    {
      rollback.exec();
      QMessageBox::information(this, tr("Enter Receipt"),
                               tr("Transaction Canceled") );
      return;
    }

    q.exec("COMMIT;");
  } 
  else if (q.lastError().type() != QSqlError::NoError)
  {
    rollback.exec();
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
  else
  {
    rollback.exec();
    systemError( this,
                tr("<p>No transaction was done because Item %1 "
                   "was not found at Site %2.")
                .arg(_item->itemNumber()).arg(_warehouse->currentText()));
    return;
  }

  if (_issueToWo->isChecked())
  {
    q.prepare( "SELECT womatl_id, womatl_issuemethod "
               "FROM womatl, wo, itemsite "
               "WHERE ( ( womatl_itemsite_id=itemsite_id)"
               " AND (womatl_wo_id=wo_id)"
               " AND (itemsite_item_id=:item_id)"
               " AND (itemsite_warehous_id=:warehous_id)"
               " AND (wo_id=:wo_id) );" );
    q.bindValue(":item_id", _item->id());
    q.bindValue(":warehous_id", _warehouse->id());
    q.bindValue(":wo_id", _wo->id());
    q.exec();
    if (q.first())
    {
      if ( (q.value("womatl_issuemethod").toString() == "S") ||
           (q.value("womatl_issuemethod").toString() == "M") )
      {
        issueWoMaterialItem newdlg(this);
        ParameterList params;
        params.append("wo_id", _wo->id());
        params.append("womatl_id", q.value("womatl_id").toInt());
        params.append("qty", _qty->toDouble());
        if (newdlg.set(params) == NoError)
          newdlg.exec();
      }
    }
  }

  if (_captive)
    close();
  else
  {
    _close->setText(tr("&Close"));
    _item->setId(-1);
    _qty->clear();
    _beforeQty->clear();
    _afterQty->clear();
    _documentNum->clear();
    _issueToWo->setChecked(FALSE);
    _wo->setId(-1);
    _notes->clear();

    _item->setFocus();
  }
}

void materialReceiptTrans::sPopulateQty()
{
  q.prepare( "SELECT itemsite_qtyonhand, itemsite_costmethod "
             "FROM itemsite "
             "WHERE ( (itemsite_item_id=:item_id)"
             " AND (itemsite_warehous_id=:warehous_id) );" );
  q.bindValue(":item_id", _item->id());
  q.bindValue(":warehous_id", _warehouse->id());
  q.exec();
  if (q.first())
  {
    _cachedQOH = q.value("itemsite_qtyonhand").toDouble();
    if(_cachedQOH == 0.0)
      _costManual->setChecked(true);
    _beforeQty->setDouble(q.value("itemsite_qtyonhand").toDouble());
    _costAdjust->setChecked(true);
    _costAdjust->setEnabled(q.value("itemsite_costmethod").toString() == "A");

    if (_issueToWo->isChecked())
      _afterQty->setDouble(q.value("itemsite_qtyonhand").toDouble());
    else if (_qty->toDouble() != 0)
      _afterQty->setDouble(_cachedQOH + _qty->toDouble());
  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  _wo->setWarehouse(_warehouse->id());
}

void materialReceiptTrans::sUpdateQty(const QString &pQty)
{
  if (_issueToWo->isChecked())
    _afterQty->setDouble(_beforeQty->toDouble());
  else
    _afterQty->setDouble(_cachedQOH + pQty.toDouble());
}

void materialReceiptTrans::sCostUpdated()
{
  if(_cost->toDouble() == 0.0 || _qty->toDouble() == 0.0)
    _unitCost->setText(tr("N/A"));
  else
    _unitCost->setDouble(_cost->toDouble() / _qty->toDouble());
}

