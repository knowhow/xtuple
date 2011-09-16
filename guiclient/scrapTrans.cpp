/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "scrapTrans.h"

#include <QMessageBox>
#include <QSqlError>
#include <QValidator>
#include <QVariant>

#include "distributeInventory.h"
#include "inputManager.h"
#include "storedProcErrorLookup.h"

scrapTrans::scrapTrans(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

//  (void)statusBar();

  connect(_post,                 SIGNAL(clicked()), this, SLOT(sPost()));
  connect(_qty,SIGNAL(textChanged(const QString&)), this, SLOT(sPopulateQty()));
  connect(_warehouse,           SIGNAL(newID(int)), this, SLOT(sPopulateQOH(int)));

  _captive = FALSE;

  _item->setType(ItemLineEdit::cGeneralInventory | ItemLineEdit::cActive);
  _warehouse->setType(WComboBox::AllActiveInventory);
  _qty->setValidator(omfgThis->transQtyVal());
  _beforeQty->setPrecision(omfgThis->qtyVal());
  _afterQty->setPrecision(omfgThis->qtyVal());

  omfgThis->inputManager()->notify(cBCItem, this, _item, SLOT(setItemid(int)));
  omfgThis->inputManager()->notify(cBCItemSite, this, _item, SLOT(setItemsiteid(int)));

  if (!_metrics->boolean("MultiWhs"))
  {
    _warehouseLit->hide();
    _warehouse->hide();
  }

  _item->setFocus();
}

scrapTrans::~scrapTrans()
{
  // no need to delete child widgets, Qt does it all for us
}

void scrapTrans::languageChange()
{
  retranslateUi(this);
}


enum SetResponse scrapTrans::set(const ParameterList &pParams)
{
  XWidget::set(pParams);
  QVariant param;
  bool     valid;
  int      invhistid = -1;

  param = pParams.value("invhist_id", &valid);
  if (valid)
    invhistid = param.toInt();

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      _mode = cNew;

      setWindowTitle(tr("Enter Scrap Transaction"));
      _usernameLit->clear();
      _transDate->setEnabled(_privileges->check("AlterTransactionDates"));
      _transDate->setDate(omfgThis->dbDate());

      _item->setFocus();
    }
    else if (param.toString() == "view")
    {
      _mode = cView;

      setWindowTitle(tr("Scrap Transaction"));
      _transDate->setEnabled(FALSE);
      _item->setReadOnly(TRUE);
      _warehouse->setEnabled(FALSE);
      _qty->setEnabled(FALSE);
      _documentNum->setEnabled(FALSE);
      _notes->setEnabled(FALSE);
      _post->hide();
      _close->setText(tr("&Close"));
      _close->setFocus();

      q.prepare( "SELECT * "
                 "FROM invhist "
                 "WHERE (invhist_id=:invhist_id);" );
      q.bindValue(":invhist_id", invhistid);
      q.exec();
      if (q.first())
      {
        _transDate->setDate(q.value("invhist_transdate").toDate());
        _username->setText(q.value("invhist_user").toString());
        _qty->setDouble(q.value("invhist_invqty").toDouble());
        _beforeQty->setDouble(q.value("invhist_qoh_before").toDouble());
        _afterQty->setDouble(q.value("invhist_qoh_after").toDouble());
        _documentNum->setText(q.value("invhist_ordnumber"));
        _notes->setText(q.value("invhist_comments").toString());
        _item->setItemsiteid(q.value("invhist_itemsite_id").toInt());
      }
      else if (q.lastError().type() != QSqlError::NoError)
      {
	systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
	return UndefinedError;
      }

      _close->setFocus();
    }
  }

  return NoError;
}

void scrapTrans::sPost()
{
  struct {
    bool        condition;
    QString     msg;
    QWidget     *widget;
  } error[] = {
    { ! _item->isValid(),
      tr("You must select an Item before posting this transaction."), _item },
    { _qty->text().length() == 0,
      tr("<p>You must enter a Quantity before posting this Transaction."),
      _qty },
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
  q.prepare( "SELECT invScrap(itemsite_id, :qty, :docNumber,"
             "                :comments, :date) AS result "
             "FROM itemsite "
             "WHERE ( (itemsite_item_id=:item_id)"
             " AND (itemsite_warehous_id=:warehous_id) );" );
  q.bindValue(":qty", _qty->toDouble());
  q.bindValue(":docNumber", _documentNum->text());
  q.bindValue(":comments", _notes->toPlainText());
  q.bindValue(":item_id", _item->id());
  q.bindValue(":warehous_id", _warehouse->id());
  q.bindValue(":date",        _transDate->date());
  q.exec();
  if (q.first())
  {
    int result = q.value("result").toInt();
    if (result < 0)
    {
      rollback.exec();
      systemError(this, storedProcErrorLookup("invScrap", result),
                  __FILE__, __LINE__);
      return;
    }
    else if (q.lastError().type() != QSqlError::NoError)
    {
      rollback.exec();
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }

    if (distributeInventory::SeriesAdjust(q.value("result").toInt(), this) == XDialog::Rejected)
    {
      rollback.exec();
      QMessageBox::information(this, tr("Scrap Transaction"),
                               tr("Transaction Canceled") );
      return;
    }

    q.exec("COMMIT;");

    if (_captive)
      close();
    else
    {
      _close->setText(tr("&Close"));
      _item->setId(-1);
      _qty->clear();
      _documentNum->clear();
      _notes->clear();
      _beforeQty->clear();
      _afterQty->clear();
      _transDate->setDate(omfgThis->dbDate());

      _item->setFocus();
    }
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
  }
}

void scrapTrans::sPopulateQOH(int pWarehousid)
{
  if (_mode != cView)
  {
    q.prepare( "SELECT itemsite_qtyonhand "
               "FROM itemsite "
               "WHERE ( (itemsite_item_id=:item_id)"
               " AND (itemsite_warehous_id=:warehous_id) );" );
    q.bindValue(":item_id", _item->id());
    q.bindValue(":warehous_id", pWarehousid);
    q.exec();
    if (q.first())
    {
      _cachedQOH = q.value("itemsite_qtyonhand").toDouble();
      _beforeQty->setDouble(q.value("itemsite_qtyonhand").toDouble());
      sPopulateQty();
    }
    else if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }
}

void scrapTrans::sPopulateQty()
{
  _afterQty->setDouble(_cachedQOH - _qty->toDouble());
}

