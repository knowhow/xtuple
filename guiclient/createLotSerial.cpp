/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "createLotSerial.h"

#include <QMessageBox>
#include <QSqlError>
#include <QValidator>
#include <QVariant>

#include <parameter.h>
#include <openreports.h>

createLotSerial::createLotSerial(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_assign, SIGNAL(clicked()), this, SLOT(sAssign()));
  connect(_lotSerial, SIGNAL(textChanged(QString)), this, SLOT(sHandleLotSerial()));

  _item->setReadOnly(TRUE);

  _serial = false;
  _itemsiteid = -1;
  _preassigned = false;
  adjustSize();

  _qtyToAssign->setValidator(omfgThis->qtyVal());
  _lotSerial->setFocus();
}

createLotSerial::~createLotSerial()
{
  // no need to delete child widgets, Qt does it all for us
}

void createLotSerial::languageChange()
{
  retranslateUi(this);
}

enum SetResponse createLotSerial::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("itemloc_series", &valid);
  if (valid)
    _itemlocSeries = param.toInt();

  param = pParams.value("itemlocdist_id", &valid);
  if (valid)
  {
    _itemlocdistid = param.toInt();

    q.prepare( "SELECT item_fractional, itemsite_controlmethod, itemsite_item_id,"
               "       itemsite_id, itemsite_perishable, itemsite_warrpurc, "
               "       COALESCE(itemsite_lsseq_id,-1) AS itemsite_lsseq_id, "
               "       invhist_ordtype, invhist_transtype, invhist_ordnumber "
               "FROM itemlocdist, itemsite, item, invhist "
               "WHERE ( (itemlocdist_itemsite_id=itemsite_id)"
               " AND (itemsite_item_id=item_id)"
               " AND (itemlocdist_invhist_id=invhist_id) "
               " AND (itemlocdist_id=:itemlocdist_id) );" );
    q.bindValue(":itemlocdist_id", _itemlocdistid);
    q.exec();
    if (q.first())
    {
      if (q.value("itemsite_controlmethod").toString() == "S")
      {
        _serial = true;
        _qtyToAssign->setText("1");
        _qtyToAssign->setEnabled(FALSE);
      }
      else
        _serial = false;

      _item->setItemsiteid(q.value("itemsite_id").toInt());
      _itemsiteid = q.value("itemsite_id").toInt();
      _expiration->setEnabled(q.value("itemsite_perishable").toBool());
      _warranty->setEnabled(q.value("itemsite_warrpurc").toBool() && q.value("invhist_ordtype").toString() == "PO");
      _fractional = q.value("item_fractional").toBool();
      
      //If there is preassigned trace info for an associated order, force user to select from list
      XSqlQuery preassign;
      preassign.prepare("SELECT lsdetail_id,ls_number,ls_number "
                        "FROM lsdetail,ls "
                        "WHERE ( (lsdetail_source_number=:docnumber) "
                        "AND (lsdetail_source_type=:transtype) "
                        "AND (lsdetail_ls_id=ls_id) "
                        "AND (lsdetail_qtytoassign > 0) )");
      preassign.bindValue(":transtype", q.value("invhist_transtype").toString());
      preassign.bindValue(":docnumber", q.value("invhist_ordnumber").toString());
      preassign.exec();
      if (preassign.first())
      {
        _lotSerial->setAllowNull(true);
        _lotSerial->populate(preassign);
        _preassigned = true;
      }
      else if (preassign.lastError().type() != QSqlError::NoError)
      {
        systemError(this, preassign.lastError().databaseText(), __FILE__, __LINE__);
        return UndefinedError;
      }
      else if (q.value("itemsite_lsseq_id").toInt() != -1)
      {
        // Auto sequence
        XSqlQuery fetchlsnum;
        fetchlsnum.prepare("SELECT fetchlsnumber(:lsseq_id) AS lotserial;");
        fetchlsnum.bindValue(":lsseq_id", q.value("itemsite_lsseq_id").toInt());
        fetchlsnum.exec();
        if (fetchlsnum.first())
          _lotSerial->setText(fetchlsnum.value("lotserial").toString());
        else if (fetchlsnum.lastError().type() != QSqlError::NoError)
        {
          systemError(this, fetchlsnum.lastError().databaseText(), __FILE__, __LINE__);
          return UndefinedError;
        }
        if (!_serial)
          _qtyToAssign->setFocus();
        else if (_expiration->isEnabled())
          _expiration->setFocus();
        else
          _warranty->setFocus();
      }
      else if (!_serial) {
        XSqlQuery lots;
        lots.prepare("SELECT itemloc_id, ls_number, ls_number "
                     "FROM ls "
                     " JOIN itemloc ON (itemloc_ls_id=ls_id) "
                     "WHERE (itemloc_itemsite_id=:itemsite_id);");
        lots.bindValue(":itemsite_id", q.value("itemsite_id").toInt());
        lots.exec();
        if (lots.lastError().type() != QSqlError::NoError)
        {
          systemError(this, lots.lastError().databaseText(), __FILE__, __LINE__);
          return UndefinedError;
        }
        else {
          _lotSerial->setAllowNull(true);
          _lotSerial->populate(lots);
          connect(_lotSerial, SIGNAL(newID(int)), this, SLOT(sLotSerialSelected()));
        }
      }
    }
    else if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return UndefinedError;
    }
  }
  
  param = pParams.value("qtyRemaining", &valid);
  if (valid)
    _qtyRemaining->setText(param.toString());

  return NoError;
}

void createLotSerial::sHandleLotSerial()
{
  _lotSerial->setText(_lotSerial->currentText().toUpper());
}

void createLotSerial::sAssign()
{
  int  lsid=-1;
  
  if (_lotSerial->currentText().isEmpty())
  {
    QMessageBox::critical( this, tr("Enter Lot/Serial Number"),
                           tr("<p>You must enter a Lot/Serial number."));
    _lotSerial->setFocus();
    return;
  }
  else if (_lotSerial->currentText().contains(QRegExp("\\s")) &&
           QMessageBox::question(this, tr("Lot/Serial Number Contains Spaces"),
                                 tr("<p>The Lot/Serial Number contains spaces. Do "
                                    "you want to save it anyway?"),
                                 QMessageBox::Yes,
                                 QMessageBox::No | QMessageBox::Default) == QMessageBox::No)
  {
    _lotSerial->setFocus();
    return;
  }

  if (_qtyToAssign->toDouble() == 0.0)
  {
    QMessageBox::critical( this, tr("Enter Quantity"),
                           tr("<p>You must enter a positive value to assign to "
                              "this Lot/Serial number.") );
    _qtyToAssign->setFocus();
    return;
  }

  if ( (_expiration->isEnabled()) && (!_expiration->isValid()) )
  {
    QMessageBox::critical( this, tr("Enter Expiration Date"),
                           tr("<p>You must enter an expiration date to this "
                              "Perishable Lot/Serial number.") );
    _expiration->setFocus();
    return;
  }
  
  if ( (_warranty->isEnabled()) && (!_warranty->isValid()) )
  {
    QMessageBox::critical( this, tr("Enter Warranty Expire Date"),
                           tr("<p>You must enter a warranty expiration date for this "
                              "Lot/Serial number.") );
    _warranty->setFocus();
    return;
  }

  if (!_fractional)
  {
    if (_qtyToAssign->toDouble() != _qtyToAssign->text().toInt())
    {
      QMessageBox::critical( this, tr("Item is Non-Fractional"),
                             tr( "<p>The Item in question is not stored in "
                                 "fractional quantities. You must enter a "
                                 "whole value to assign to this Lot/Serial "
                                 "number." ) );
      _qtyToAssign->setFocus();
      return;
    }
  }
  
  if (_preassigned)
  {
    q.prepare("SELECT lsdetail_qtytoassign "
              "FROM lsdetail "
              "WHERE (lsdetail_id=:lsdetail_id);");
    q.bindValue(":lsdetail_id", _lotSerial->id());
    q.exec();
    if (q.first())
    {
      if ( _qtyToAssign->toDouble() > q.value("lsdetail_qtytoassign").toDouble() )
      {
        QMessageBox::critical( this, tr("Invalid Qty"),
                               tr( "<p>The quantity being assigned is greater than the "
                                   " quantity preassigned to the order being received." ) );
        return;
      }
    }
    else
    {
      QMessageBox::critical( this, tr("Invalid Number"),
                             tr( "<p>The number entered is not valid.  Please select from the list "
                                 "of valid numbers." ) );
      _lotSerial->removeItem(_lotSerial->currentIndex());
      _lotSerial->setCurrentIndex(0);
      return;
    }
  }

  if (_serial)
  {
    q.prepare("SELECT COUNT(*) AS count FROM "
              "(SELECT itemloc_id AS count "
              "FROM itemloc,itemsite,ls "
              "WHERE ((itemloc_itemsite_id=:itemsite_id)"
              "  AND (itemloc_itemsite_id=itemsite_id)"
              "  AND (itemsite_item_id=ls_item_id)"
              "  AND (itemloc_ls_id=ls_id)"
              "  AND (ls_number=:lotserial))"
              "UNION "
              "SELECT itemlocdist_id "
              "FROM itemlocdist,itemsite,ls "
              "WHERE ((itemlocdist_itemsite_id=:itemsite_id) "
              "  AND (itemlocdist_itemsite_id=itemsite_id)"
              "  AND (itemlocdist_ls_id=ls_id)"
              "  AND (ls_number=:lotserial) "
              "  AND (itemlocdist_source_type='D'))) as data;");
    q.bindValue(":itemsite_id", _itemsiteid);
    q.bindValue(":lotserial", _lotSerial->currentText());
    q.exec();
    if (q.first())
    {
      if (q.value("count").toInt() > 0)
      {
        QMessageBox::critical(this, tr("Duplicate Serial Number"),
                              tr("This Serial Number has already been used "
                                 "and cannot be reused."));

        return;
      }
    }
    else if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }
  else
  {
    q.prepare("SELECT ls_id, ls_number "
              "FROM ls,itemsite "
              "WHERE ( (UPPER(ls_number)=UPPER(:lotserial)) "
              "AND (ls_item_id=itemsite_item_id) "
              "AND (itemsite_id=:itemsiteid) );");
    q.bindValue(":itemsiteid", _itemsiteid);
    q.bindValue(":lotserial", _lotSerial->currentText());
    q.exec();
    if (q.first())
    {
      if (!_serial)
        if (QMessageBox::question(this, tr("Use Existing?"),
                                  tr("<p>A record with for lot number %1 for this item already exists.  "
                                     "Reference existing lot?").arg(q.value("ls_number").toString().toUpper()),
                                  QMessageBox::Yes | QMessageBox::Default,
                                  QMessageBox::No  | QMessageBox::Escape) == QMessageBox::No)
          return;
      lsid=q.value("ls_id").toInt();
    }
    else if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }

  QString sql;
  if (_preassigned)
    sql = "SELECT createlotserial(:itemsite_id,:lotserial,:itemlocseries,lsdetail_source_type,lsdetail_source_id,:itemlocdist_id,:qty,:expiration,:warranty) "
          "FROM lsdetail,itemlocdist "
          "WHERE ((lsdetail_id=:lsdetail_id)"
          "AND (itemlocdist_id=:itemlocdist_id));";

  else
    sql = "SELECT createlotserial(:itemsite_id,:lotserial,:itemlocseries,'I',NULL,itemlocdist_id,:qty,:expiration,:warranty) "
          "FROM itemlocdist "
          "WHERE (itemlocdist_id=:itemlocdist_id);";

  q.prepare(sql);
  q.bindValue(":itemsite_id", _itemsiteid);
  q.bindValue(":lotserial", _lotSerial->currentText().toUpper());
  q.bindValue(":itemlocseries", _itemlocSeries);
  q.bindValue(":lsdetail_id", _lotSerial->id());
  q.bindValue(":qty", _qtyToAssign->toDouble());
  if (_expiration->isEnabled())
    q.bindValue(":expiration", _expiration->date());
  else
    q.bindValue(":expiration", omfgThis->endOfTime());
  if (_warranty->isEnabled())
    q.bindValue(":warranty", _warranty->date());
  q.bindValue(":itemlocdist_id", _itemlocdistid);
  q.exec();
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  accept();
}

void createLotSerial::sLotSerialSelected()
{
    if (!_expiration->isEnabled() &&
        !_warranty->isEnabled())
      return;

    XSqlQuery itemloc;
    itemloc.prepare("SELECT itemloc_expiration, itemloc_warrpurc "
            "FROM itemloc "
            "WHERE itemloc_id=:itemloc_id;");
    itemloc.bindValue(":itemloc_id", _lotSerial->id());
    itemloc.exec();
    if (itemloc.first()) {
      if (_expiration->isEnabled() &&
          itemloc.value("itemloc_expiration").toDate() != omfgThis->endOfTime())
        _expiration->setDate(itemloc.value("itemloc_expiration").toDate());
      if (_warranty->isEnabled())
        _warranty->setDate(itemloc.value("itemloc_warrpurc").toDate());
    }
}
