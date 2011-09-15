/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "changePoitemQty.h"

#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include "storedProcErrorLookup.h"

changePoitemQty::changePoitemQty(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_buttonBox, SIGNAL(accepted()), this, SLOT(sChangeQty()));
  connect(_buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
  connect(_newQty, SIGNAL(lostFocus()), this, SLOT(sQtyChanged()));
  connect(_po, SIGNAL(newId(int, QString)), this, SLOT(sPopulatePoitem(int)));
  connect(_poitem, SIGNAL(newID(int)), this, SLOT(sPopulate(int)));

  _captive = false;
  _cacheFreight = 0.0;

  _commentGroup->setEnabled(_postComment->isChecked());
  
  _newQty->setValidator(omfgThis->qtyVal());
  _newQtyReceived->setPrecision(omfgThis->qtyVal());
  _currentQtyOrdered->setPrecision(omfgThis->qtyVal());
  _currentQtyReceived->setPrecision(omfgThis->qtyVal());
  _currentQtyBalance->setPrecision(omfgThis->qtyVal());
  _newQtyBalance->setPrecision(omfgThis->qtyVal());

  _cmnttype->setType(XComboBox::AllCommentTypes);
  adjustSize();
}

changePoitemQty::~changePoitemQty()
{
  // no need to delete child widgets, Qt does it all for us
}

void changePoitemQty::languageChange()
{
  retranslateUi(this);
}

enum SetResponse changePoitemQty::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  _captive = TRUE;

  QVariant param;
  bool     valid;

  param = pParams.value("pohead_id", &valid);
  if (valid)
  {
    _po->setId(param.toInt());
    _po->setReadOnly(true);
    _newQty->setFocus();
  }

  param = pParams.value("poitem_id", &valid);
  if (valid)
  {
    q.prepare( "SELECT poitem_pohead_id "
               "FROM poitem "
               "WHERE (poitem_id=:poitem_id);" );
    q.bindValue(":poitem_id", param.toInt());
    q.exec();
    if (q.first())
    {
      _po->setId(q.value("poitem_pohead_id").toInt());
      _po->setReadOnly(true);
      _poitem->setId(param.toInt());
    }

    _newQty->setFocus();
  }

  param = pParams.value("newQty", &valid);
  if (valid)
    _newQty->setDouble(param.toDouble());

  return NoError;
}

void changePoitemQty::sPopulatePoitem(int pPoheadid)
{
  q.prepare( "SELECT poitem_id,"
             "       ( poitem_linenumber || '-' ||"
             "         COALESCE(item_number, poitem_vend_item_number) || ' (' ||"
             "         COALESCE(item_descrip1, firstLine(poitem_vend_item_descrip)) || ')' ) "
             "FROM poitem LEFT OUTER JOIN "
             "     ( itemsite JOIN item "
             "       ON (itemsite_item_id=item_id)"
             "     ) ON (poitem_itemsite_id=itemsite_id) "
             "WHERE ( (poitem_status <> 'C')"
             " AND (poitem_pohead_id=:pohead_id) ) "
             "ORDER BY poitem_linenumber;" );
  q.bindValue(":pohead_id", pPoheadid);
  q.exec();
  _poitem->populate(q);
}

void changePoitemQty::sPopulate(int pPoitemid)
{
  if (pPoitemid == -1)
  {
    _currentQtyReceived->clear();
    _newQtyReceived->clear();
    _currentQtyOrdered->clear();
    _newQty->clear();
    _currentQtyBalance->clear();
    _newQtyBalance->clear();
    _freight->clear();
  }
  else
  {
    q.prepare( "SELECT poitem_qty_ordered, poitem_qty_received,"
               "        poitem_qty_returned, poitem_freight, pohead_curr_id,"
               "        CURRENT_DATE AS date "
               "FROM poitem, pohead "
               "WHERE ((poitem_pohead_id=pohead_id)"
               "  AND  (poitem_id=:poitem_id));" );
    q.bindValue(":poitem_id", pPoitemid);
    q.exec();
    if (q.first())
    {
      _currentQtyOrdered->setDouble(q.value("poitem_qty_ordered").toDouble());
      _currentQtyReceived->setDouble(q.value("poitem_qty_received").toDouble());
      _currentQtyBalance->setDouble(q.value("poitem_qty_ordered").toDouble() - q.value("poitem_qty_received").toDouble());
      _newQtyReceived->setDouble(q.value("poitem_qty_received").toDouble());
      _cacheFreight = q.value("poitem_freight").toDouble();
      _freight->set(q.value("poitem_freight").toDouble(),
                    q.value("pohead_curr_id").toInt(),
                    q.value("date").toDate());
      sQtyChanged();
    }
  }
}

void changePoitemQty::sChangeQty()
{
  if (_newQty->toDouble() <= 0 &&
      QMessageBox::question(this, tr("Quantity 0?"),
			    tr("<p>You have changed the quantity to 0 or less. "
			       "Are you sure you want to do this?"),
			    QMessageBox::No | QMessageBox::Default,
			    QMessageBox::Yes) == QMessageBox::No)
    return;

  q.prepare("SELECT changePoitemQty(:poitem_id, :qty) AS result;");
  q.bindValue(":poitem_id", _poitem->id());
  q.bindValue(":qty", _newQty->toDouble());
  q.exec();
  if (q.first())
  {
    int result = q.value("result").toInt();
    if (result < 0)
    {
      systemError(this, storedProcErrorLookup("changePoitemQty", result), __FILE__, __LINE__);
      return;
    }
  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  if (_postComment->isChecked())
  {
    q.prepare("SELECT postComment(:cmnttype_id, 'PI', :poitem_id, :comment) AS result");
    q.bindValue(":cmnttype_id", _cmnttype->id());
    q.bindValue(":poitem_id", _poitem->id());
    q.bindValue(":comment", _comment->toPlainText());
    q.exec();
    if (q.first())
    {
      int result = q.value("result").toInt();
      if (result < 0)
      {
	systemError(this, storedProcErrorLookup("postcomment", result), __FILE__, __LINE__);
	return;
      }
    }
    else if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }

  if (_freight->localValue() != _cacheFreight)
  {
    q.prepare("UPDATE poitem SET poitem_freight=:poitem_freight"
              " WHERE (poitem_id=:poitem_id); ");
    q.bindValue(":poitem_id", _poitem->id());
    q.bindValue(":poitem_freight", _freight->localValue());
    q.exec();
    if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }

  omfgThis->sPurchaseOrdersUpdated(_po->id(), true);
  
  accept();
}

void changePoitemQty::sQtyChanged()
{
  double qtyBalance = (_newQty->toDouble() - _newQtyReceived->toDouble());
  if (qtyBalance < 0)
    qtyBalance = 0;

  _newQtyBalance->setDouble(qtyBalance);
}
