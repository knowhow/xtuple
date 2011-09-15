/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "purchaseRequest.h"

#include <QMessageBox>
#include <QValidator>
#include <QVariant>

#include <QCloseEvent>

purchaseRequest::purchaseRequest(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_close, SIGNAL(clicked()), this, SLOT(sClose()));
  connect(_create, SIGNAL(clicked()), this, SLOT(sCreate()));
  connect(_warehouse, SIGNAL(newID(int)), this, SLOT(sCheckWarehouse(int)));

  _planordid = -1;
  _item->setType(ItemLineEdit::cGeneralPurchased | ItemLineEdit::cGeneralManufactured);
  _item->setDefaultType(ItemLineEdit::cGeneralPurchased);
  _lastWarehousid = _warehouse->id();

  _number->setValidator(omfgThis->orderVal());
  _qty->setValidator(omfgThis->qtyVal());
  
  _project->setType(ProjectLineEdit::PurchaseOrder);
  if(!_metrics->boolean("UseProjects"))
    _project->hide();

  //If not multi-warehouse hide whs control
  if (!_metrics->boolean("MultiWhs"))
  {
    _warehouseLit->hide();
    _warehouse->hide();
  }
}

purchaseRequest::~purchaseRequest()
{
    // no need to delete child widgets, Qt does it all for us
}

void purchaseRequest::languageChange()
{
    retranslateUi(this);
}

enum SetResponse purchaseRequest::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  QVariant param;
  bool     valid;
  int      prid = -1;

  param = pParams.value("itemsite_id", &valid);
  if (valid)
  {
    _item->setItemsiteid(param.toInt());
    _item->setEnabled(FALSE);
    _warehouse->setEnabled(FALSE);
  }

  param = pParams.value("qty", &valid);
  if (valid)
    _qty->setDouble(param.toDouble());

  param = pParams.value("dueDate", &valid);
  if (valid)
    _dueDate->setDate(param.toDate());

  param = pParams.value("planord_id", &valid);
  if (valid)
    _planordid = param.toInt();

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      _mode = cNew;

      populateNumber();
    }
    else if (param.toString() == "view")
    {
      _mode = cView;

      param = pParams.value("pr_id", &valid);
      if (valid)
        prid = param.toInt();

      _number->setEnabled(FALSE);
      _item->setReadOnly(TRUE);
      _warehouse->setEnabled(FALSE);
      _qty->setEnabled(FALSE);
      _dueDate->setEnabled(FALSE);
      _notes->setEnabled(FALSE);
      _project->setEnabled(FALSE);
      _create->hide();

      q.prepare( "SELECT pr_itemsite_id,"
                 "       pr_number,"
                 "       pr_qtyreq,"
                 "       pr_duedate,"
                 "       pr_prj_id "
                 "FROM pr "
                 "WHERE (pr_id=:pr_id);" );
      q.bindValue(":pr_id", prid);
      q.exec();
      if (q.first())
      {
        _number->setText(q.value("pr_number").toString());
        _item->setItemsiteid(q.value("pr_itemsite_id").toInt());
        _qty->setDouble(q.value("pr_qtyreq").toDouble());
        _dueDate->setDate(q.value("pr_duedate").toDate());
        _project->setId(q.value("pr_prj_id").toInt());
      }
    }
    else if (param.toString() == "release")
    {
      _mode = cRelease;
      _captive = TRUE;

      _number->setEnabled(FALSE);
      _item->setReadOnly(TRUE);
      _warehouse->setEnabled(FALSE);
      _qty->setEnabled(FALSE);
      _dueDate->setEnabled(FALSE);

      q.prepare( "SELECT planord_itemsite_id, planord_duedate,"
                 "       planord_qty, planord_comments "
                 "FROM planord "
                 "WHERE (planord_id=:planord_id);" );
      q.bindValue(":planord_id", _planordid);
      q.exec();
      if (q.first())
      {
        _item->setItemsiteid(q.value("planord_itemsite_id").toInt());
        _qty->setDouble(q.value("planord_qty").toDouble());
        _dueDate->setDate(q.value("planord_duedate").toDate());
        _notes->setText(q.value("planord_comments").toString());

        populateNumber();
      }

      _create->setFocus();
    }
  }

  return NoError;
}

void purchaseRequest::sClose()
{
  closeEvent(NULL);
  reject();
}

void purchaseRequest::sCreate()
{
  if (!_number->text().length())
  {
    QMessageBox::information( this, tr("Invalid Purchase Request Number"),
                              tr("You must enter a valid Purchase Request number before creating this Purchase Request")  );
    _number->setFocus();
    return;
  }

  if (!(_item->isValid()))
  {
    QMessageBox::information( this, tr("No Item Number Selected"),
                              tr("You must enter or select a valid Item number before creating this Purchase Request")  );
    return;
  }

  if (_qty->toDouble() == 0.0)
  {
    QMessageBox::information( this, tr("Invalid Quantity Ordered"),
                              tr( "You have entered an invalid Qty. Ordered.\n"
                                  "Please correct before creating this Purchase Request."  ) );
    _qty->setFocus();
    return;
  }

  if (!_dueDate->isValid())
  {
    QMessageBox::information( this, tr("Invalid Due Date Entered"),
                              tr( "You have entered an invalid Due Date.\n"
                                  "Please correct before creating this Purchase Request."  ) );
    _dueDate->setFocus();
    return;
  }

  q.prepare( "SELECT itemsite_id "
             "FROM itemsite "
             "WHERE ( (itemsite_item_id=:item_id)"
             " AND (itemsite_warehous_id=:warehous_id) );" );
  q.bindValue(":item_id", _item->id());
  q.bindValue(":warehous_id", _warehouse->id());
  q.exec();
  if (!q.first())
  {
    QMessageBox::warning(this, tr("Invalid Site"),
        tr("The selected Site for this Purchase Request is not\n"
           "a \"Supplied At\" Site. You must select a different\n"
           "Site before creating the Purchase Request.") );
    return;
  }

  int prid = -1;
  int itemsiteid = q.value("itemsite_id").toInt();

  if (_mode == cNew)
  {
    q.prepare( "SELECT createPr( :orderNumber, :itemsite_id, :qty,"
               "                 :dueDate, :notes ) AS prid;" );
    q.bindValue(":orderNumber", _number->text().toInt());
    q.bindValue(":itemsite_id", itemsiteid);
    q.bindValue(":qty", _qty->toDouble());
    q.bindValue(":dueDate", _dueDate->date());
    q.bindValue(":notes", _notes->toPlainText());
    q.exec();
    if (!q.first())
    {
      systemError(this, tr("A System Error occurred at %1::%2.")
                        .arg(__FILE__)
                        .arg(__LINE__) );
      return;
    }
    else
      prid = q.value("prid").toInt();
  }
  else if (_mode == cRelease)
  {
    q.prepare("SELECT createPr(:orderNumber, 'F', :planord_id, :notes) AS prid;");
    q.bindValue(":orderNumber", _number->text().toInt());
    q.bindValue(":planord_id", _planordid);
    q.bindValue(":notes", _notes->toPlainText());
    q.exec();
    if (q.first())
    {
      prid = q.value("prid").toInt();

      q.prepare("SELECT releasePlannedOrder(:planord_id, FALSE) AS result;");
      q.bindValue(":planord_id", _planordid);
      q.exec();
    }
  }

  if(-1 != prid)
  {
    q.prepare("UPDATE pr SET pr_prj_id=:prj_id WHERE (pr_id=:pr_id);");
    q.bindValue(":pr_id", prid);
    q.bindValue(":prj_id", _project->id());
    q.exec();
  }

  omfgThis->sPurchaseRequestsUpdated();

  if (_captive)
    done(prid);
  else
  {
    populateNumber();
    _item->setId(-1);
    _qty->clear();
    _dueDate->setNull();
    _notes->clear();

    _item->setFocus();
  }
}

void purchaseRequest::populateNumber()
{
  QString generationMethod = _metrics->value("PrNumberGeneration");

  if ((generationMethod == "A") || (generationMethod == "O"))
  {
    q.exec("SELECT fetchPrNumber() AS number;");
    if (!q.first())
    {
      systemError(this, tr("A System Error occurred at %1::%2.")
                        .arg(__FILE__)
                        .arg(__LINE__) );

      _number->setText("Error");
      return;
    }

    _number->setText(q.value("number").toString());
  }

  if (generationMethod == "M")
  {
    _number->setEnabled(TRUE);
    _number->setFocus();
  }
  else if (generationMethod == "O")
  {
    _number->setEnabled(TRUE);
    _item->setFocus();
  }
  else if (generationMethod == "A")
  {
    _number->setEnabled(FALSE);
    _item->setFocus();
  } 
}

void purchaseRequest::closeEvent(QCloseEvent *pEvent)
{
  if ( ((_mode == cNew) || (_mode == cRelease)) &&
       ((_metrics->value("PrNumberGeneration") == "A") || (_metrics->value("PrNumberGeneration") == "O")) )
  {
    q.prepare("SELECT releasePrNumber(:orderNumber);");
    q.bindValue(":orderNumber", _number->text().toInt());
    q.exec();
  }

  if (pEvent != NULL)
    XDialog::closeEvent(pEvent);
}

void purchaseRequest::sCheckWarehouse( int pWarehousid )
{
  if(_mode != cRelease)
    return;

  if(_lastWarehousid == pWarehousid)
    return;

  _lastWarehousid = pWarehousid;

  if(pWarehousid == -1)
  {
    QMessageBox::warning(this, tr("Invalid Site"),
        tr("The selected Site for this Purchase Request is not\n"
           "a \"Supplied At\" Site. You must select a different\n"
           "Site before creating the Purchase Request.") );
    _warehouse->setEnabled(TRUE);
  }
}
