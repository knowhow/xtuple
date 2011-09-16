/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "miscVoucher.h"

#include <QCloseEvent>
#include <QKeyEvent>
#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include "guiErrorCheck.h"
#include "errorReporter.h"
#include "voucherMiscDistrib.h"

miscVoucher::miscVoucher(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  connect(_amountDistributed, SIGNAL(valueChanged()), this, SLOT(sPopulateBalanceDue()));
  connect(_amountToDistribute, SIGNAL(valueChanged()), this, SLOT(sPopulateBalanceDue()));
  connect(_amountToDistribute, SIGNAL(effectiveChanged(const QDate&)), this, SLOT(sFillMiscList()));
  connect(_amountToDistribute, SIGNAL(idChanged(int)), this, SLOT(sFillMiscList()));
  connect(_amountToDistribute, SIGNAL(valueChanged()), this, SLOT(sPopulateBalanceDue()));
  connect(_delete, SIGNAL(clicked()), this, SLOT(sDeleteMiscDistribution()));
  connect(_edit, SIGNAL(clicked()), this, SLOT(sEditMiscDistribution()));
  connect(_invoiceDate, SIGNAL(newDate(const QDate&)), this, SLOT(sPopulateDistDate()));
  connect(_invoiceDate, SIGNAL(newDate(const QDate&)), this, SLOT(sPopulateDueDate()));
  connect(_terms, SIGNAL(newID(int)), this, SLOT(sPopulateDueDate()));
  connect(_new, SIGNAL(clicked()), this, SLOT(sNewMiscDistribution()));
  connect(_save, SIGNAL(clicked()), this, SLOT(sSave()));
  connect(_voucherNumber, SIGNAL(lostFocus()), this, SLOT(sHandleVoucherNumber()));

  _terms->setType(XComboBox::APTerms);

  _vendor->setShowInactive(false);

  _recurring->setParent(-1, "V");

  _miscDistrib->addColumn(tr("Account"), -1,           Qt::AlignLeft,   true,  "account"  );
  _miscDistrib->addColumn(tr("Amount"),  _moneyColumn, Qt::AlignRight,  true,  "vodist_amount" );
}

miscVoucher::~miscVoucher()
{
  // no need to delete child widgets, Qt does it all for us
}

void miscVoucher::languageChange()
{
  retranslateUi(this);
}

enum SetResponse miscVoucher::set(const ParameterList &pParams)
{
  XWidget::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("vend_id", &valid);
  if (valid)
    _vendor->setId(param.toInt());

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      _mode = cNew;

      if (_metrics->value("VoucherNumberGeneration") == "A")
      {
        populateNumber();
        _vendor->setFocus();
      }
      else
        _voucherNumber->setFocus();

      connect(_vendor, SIGNAL(newId(int)), this, SLOT(sPopulateVendorInfo(int)));

      XSqlQuery insq;
      insq.prepare("INSERT INTO vohead ("
                "  vohead_number, vohead_misc,"
                "  vohead_posted, vohead_pohead_id, vohead_docdate"
                ") VALUES ("
                "  :vohead_number, true,"
                "  false, -1, CURRENT_DATE)"
                " RETURNING vohead_id;" );
      insq.bindValue(":vohead_id",     _voheadid);
      insq.bindValue(":vohead_number", _voucherNumber->text());
      insq.exec();
      if (insq.first())
      {
        _voheadid = insq.value("vohead_id").toInt();
        _recurring->setParent(_voheadid, "V");
      }
      else if (ErrorReporter::error(QtCriticalMsg, this, tr("Creating Voucher"),
                               insq, __FILE__, __LINE__))
        return UndefinedError;
   }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;

      _voucherNumber->setEnabled(false);
      _vendor->setEnabled(false);

      _save->setFocus();
    }
    else if (param.toString() == "view")
    {
      _mode = cView;

      _voucherNumber->setEnabled(false);
      _vendor->setReadOnly(true);
      _taxzone->setEnabled(false);
      _amountToDistribute->setEnabled(false);
      _distributionDate->setEnabled(false);
      _invoiceDate->setEnabled(false);
      _dueDate->setEnabled(false);
      _terms->setEnabled(false);
      _terms->setType(XComboBox::Terms);
      _vendor->setShowInactive(true);
      _invoiceNum->setEnabled(false);
      _reference->setEnabled(false);
      _miscDistrib->setEnabled(false);
      _new->setEnabled(false);
      _flagFor1099->setEnabled(false);
      _notes->setEnabled(false);
      _close->setText(tr("&Close"));
      _save->hide();

      _close->setFocus();
    }
  }

  param = pParams.value("vohead_id", &valid);
  if (valid)
  {
    _voheadid = param.toInt();
    populate();
  }

  return NoError;
}

void miscVoucher::sSave()
{
  QList<GuiErrorCheck> errors;
  errors << GuiErrorCheck(! _invoiceDate->isValid(), _invoiceDate,
                           tr("<p>You must enter an Invoice Date before you "
                              "may save this Voucher."))
         << GuiErrorCheck(! _dueDate->isValid(), _dueDate,
                           tr("<p>You must enter a Due Date before you may "
                              "save this Voucher."))
         << GuiErrorCheck(! _distributionDate->isValid(), _distributionDate,
                           tr("<p>You must enter a Distribution Date before "
                              "you may save this Voucher."))
         << GuiErrorCheck(_amountToDistribute->isZero(), _amountToDistribute,
                           tr("<p>You must enter an Amount to Distribute "
                              "before you may save this Voucher."))
         << GuiErrorCheck(! _balance->isZero(), _amountToDistribute,
                           tr("<p>You must fully distribute the Amount before "
                              "you may save this Voucher."))
         << GuiErrorCheck(_metrics->boolean("ReqInvMiscVoucher") &&
                          _invoiceNum->text().trimmed().isEmpty(),
                          _invoiceNum,
                           tr("<p>You must enter a Vendor Invoice Number "
                              "before you may save this Voucher."))
         ;
  if (GuiErrorCheck::reportErrors(this, tr("Cannot Save Voucher"), errors))
    return;

  if (_invoiceNum->text().trimmed().length() > 0)
  {
    XSqlQuery dupq;
    dupq.prepare( "SELECT vohead_id "
               "FROM vohead "
               "WHERE ( (vohead_invcnumber=:vohead_invcnumber)"
               " AND (vohead_vend_id=:vend_id)"
               " AND (vohead_id<>:vohead_id) );" );
    dupq.bindValue(":vohead_invcnumber", _invoiceNum->text().trimmed());
    dupq.bindValue(":vend_id",   _vendor->id());
    dupq.bindValue(":vohead_id", _voheadid);
    dupq.exec();
    if (dupq.first())
    {
      if (QMessageBox::question(this, windowTitle(),
                                tr("<p>A Voucher for this Vendor has already "
                                   "been entered with the same Vendor Invoice "
                                   "Number. Are you sure you want to use this "
                                   "number again?" ),
                              QMessageBox::Yes | QMessageBox::No,
                              QMessageBox::No) == QMessageBox::No)
      {
        _invoiceNum->setFocus();
        return;
      }
    }
    else if (ErrorReporter::error(QtCriticalMsg, this, tr("Reused Invoice Check"),
                                  dupq, __FILE__, __LINE__))
      return;
  }

  RecurrenceWidget::RecurrenceChangePolicy cp = _recurring->getChangePolicy();
  if (cp == RecurrenceWidget::NoPolicy)
    return;

  XSqlQuery rollbackq;
  rollbackq.prepare("ROLLBACK;");

  XSqlQuery begin("BEGIN;");

  XSqlQuery updq;
  updq.prepare( "UPDATE vohead "
             "SET vohead_number=:vohead_number,"
             "    vohead_vend_id=:vohead_vend_id,"
             "    vohead_distdate=:vohead_distdate,"
             "    vohead_docdate=:vohead_docdate,"
             "    vohead_duedate=:vohead_duedate,"
             "    vohead_terms_id=:vohead_terms_id,"
             "    vohead_taxzone_id=:vohead_taxzone_id,"
             "    vohead_invcnumber=:vohead_invcnumber,"
             "    vohead_reference=:vohead_reference,"
             "    vohead_amount=:vohead_amount,"
             "    vohead_1099=:vohead_1099,"
             "    vohead_curr_id=:vohead_curr_id,"
             "    vohead_recurring_vohead_id=:vohead_recurring_vohead_id,"
             "    vohead_notes=:vohead_notes"
             " WHERE (vohead_id=:vohead_id);" );

  updq.bindValue(":vohead_id", _voheadid);
  updq.bindValue(":vohead_number", _voucherNumber->text().toInt());
  updq.bindValue(":vohead_vend_id", _vendor->id());
  updq.bindValue(":vohead_terms_id", _terms->id());
  if (_taxzone->isValid())
    updq.bindValue(":vohead_taxzone_id", _taxzone->id());
  updq.bindValue(":vohead_distdate", _distributionDate->date());
  updq.bindValue(":vohead_docdate", _invoiceDate->date());
  updq.bindValue(":vohead_duedate", _dueDate->date());
  updq.bindValue(":vohead_invcnumber", _invoiceNum->text().trimmed());
  updq.bindValue(":vohead_reference", _reference->text().trimmed());
  updq.bindValue(":vohead_amount", _amountToDistribute->localValue());
  updq.bindValue(":vohead_1099", QVariant(_flagFor1099->isChecked()));
  updq.bindValue(":vohead_curr_id", _amountToDistribute->id());
  if(_recurring->isRecurring())
  {
    if(_recurring->parentId() != 0)
      updq.bindValue(":vohead_recurring_vohead_id", _recurring->parentId());
    else
      updq.bindValue(":vohead_recurring_vohead_id", _voheadid);
  }
  updq.bindValue(":vohead_notes",   _notes->toPlainText());
  updq.exec();
  if (updq.lastError().type() != QSqlError::NoError)
  {
    rollbackq.exec();
    ErrorReporter::error(QtCriticalMsg, this, tr("Saving Voucher"),
                         updq, __FILE__, __LINE__);
    return;
  }

  QString errmsg;
  if (! _recurring->save(true, cp, &errmsg))
  {
    rollbackq.exec();
    ErrorReporter::error(QtCriticalMsg, this, tr("Saving Voucher Recurrence"),
                         errmsg, __FILE__, __LINE__);
    return;
  }

  XSqlQuery commitq("COMMIT;");

  omfgThis->sVouchersUpdated();

  _voheadid = -1;

  if(cNew != _mode)
  {
    close();
    return;
  }

  _voucherNumber->clear();
  _vendor->setId(-1);
  _amountToDistribute->clear();
  _amountDistributed->clear();
  _balance->clear();
  _invoiceDate->setNull();
  _distributionDate->setNull();
  _dueDate->setNull();
  _invoiceNum->clear();
  _reference->clear();
  _flagFor1099->setChecked(false);
  _miscDistrib->clear();
  _notes->setText("");
  _cachedAmountDistributed = 0.0;

  ParameterList params;
  params.append("mode", "new");
  set(params);
}

void miscVoucher::sHandleVoucherNumber()
{
  if (_voucherNumber->text().length() == 0)
  {
    if ((_metrics->value("VoucherNumberGeneration") == "A") ||
        (_metrics->value("VoucherNumberGeneration") == "O"))
      populateNumber();
    else
    {
      QMessageBox::critical( this, tr("Enter Voucher Number"),
                             tr("<p>You must enter a valid Voucher Number "
                                "before continuing.") );

      _voucherNumber->setFocus();
      return;
    }
  }
  else
  {
    XSqlQuery editq;
    editq.prepare( "SELECT vohead_id "
               "FROM vohead "
               "WHERE (vohead_number=:vohead_number);" );
    editq.bindValue(":vohead_number", _voucherNumber->text().toInt());
    editq.exec();
    if (editq.first())
    {
      _voheadid = editq.value("vohead_id").toInt();

      _voucherNumber->setEnabled(false);
      _vendor->setEnabled(false);

      _mode = cEdit;
      populate();

      return;
    }
    else if (ErrorReporter::error(QtCriticalMsg, this,
                                  tr("Looking for Existing Voucher"),
                                  editq, __FILE__, __LINE__))
      return;
  }
}

void miscVoucher::sPopulateVendorInfo(int pVendid)
{
  XSqlQuery vendq;
  vendq.prepare("SELECT vend_terms_id, vend_1099, vend_curr_id, vend_taxzone_id"
                "  FROM vendinfo"
                " WHERE (vend_id=:vend_id);" );
  vendq.bindValue(":vend_id", pVendid);
  vendq.exec();
  if (vendq.first())
  {
    _terms->setId(vendq.value("vend_terms_id").toInt());
    _flagFor1099->setChecked(vendq.value("vend_1099").toBool());
    _amountToDistribute->setId(vendq.value("vend_curr_id").toInt());
    _taxzone->setId(vendq.value("vend_taxzone_id").toInt());
  }
  else if (ErrorReporter::error(QtCriticalMsg, this, tr("Getting Vendor Info"),
                                vendq, __FILE__, __LINE__))
    return;
}

void miscVoucher::sNewMiscDistribution()
{
  ParameterList params;
  params.append("mode", "new");
  params.append("vohead_id", _voheadid);
  params.append("vend_id", _vendor->id());
  params.append("voucher_type");
  params.append("curr_id", _amountToDistribute->id());
  params.append("curr_effective", _amountToDistribute->effective());
  params.append("amount", _balance->localValue());
  if (_taxzone->isValid())
   params.append("taxzone_id", _taxzone->id());

  voucherMiscDistrib newdlg(this, "", TRUE);
  newdlg.set(params);
  if (newdlg.exec() != XDialog::Rejected)
  {
    sFillMiscList();
    sPopulateDistributed();
  }
}

void miscVoucher::sEditMiscDistribution()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("vohead_id", _voheadid);
  params.append("voucher_type");
  params.append("vodist_id", _miscDistrib->id());
  params.append("curr_id", _amountToDistribute->id());
  params.append("curr_effective", _amountToDistribute->effective());
  if (_taxzone->isValid())
   params.append("taxzone_id", _taxzone->id());

  voucherMiscDistrib newdlg(this, "", TRUE);
  newdlg.set(params);
  if (newdlg.exec() != XDialog::Rejected)
  {
    sFillMiscList();
    sPopulateDistributed();
  }
}

void miscVoucher::sDeleteMiscDistribution()
{
  XSqlQuery delq;
  delq.prepare("DELETE FROM vodist WHERE (vodist_id=:vodist_id);");
  delq.bindValue(":vodist_id", _miscDistrib->id());
  delq.exec();
  if (ErrorReporter::error(QtCriticalMsg, this, tr("Deleting Misc Distribution"),
                           delq, __FILE__, __LINE__))
    return;

  sFillMiscList();
  sPopulateDistributed();
}

void miscVoucher::sFillMiscList()
{
  XSqlQuery getq;
  getq.prepare("SELECT vodist_id,"
             " (formatGLAccount(accnt_id) || ' - ' || accnt_descrip) AS account,"
             "       vodist_amount, 'curr' AS vodist_amount_xtnumericrole "
             "FROM vodist, accnt "
             "WHERE ( (vodist_poitem_id=-1)"
             " AND (vodist_accnt_id=accnt_id)"
             " AND (vodist_vohead_id=:vohead_id) ) "
             "UNION ALL "
             "SELECT vodist_id, (expcat_code || ' - ' || expcat_descrip) AS account,"
             "       vodist_amount, 'curr' AS vodist_amount_xtnumericrole "
             "  FROM vodist, expcat "
             " WHERE ( (vodist_poitem_id=-1)"
             "   AND   (vodist_expcat_id=expcat_id)"
             "   AND   (vodist_vohead_id=:vohead_id) ) "
             "UNION ALL "
             "SELECT vodist_id, (tax_code || ' - ' || tax_descrip) AS account,"
             "       vodist_amount, 'curr' AS vodist_amount_xtnumericrole "
             "  FROM vodist, tax "
             " WHERE ( (vodist_poitem_id=-1)"
             "   AND   (vodist_tax_id=tax_id)"
             "   AND   (vodist_vohead_id=:vohead_id) ) "
             "ORDER BY account;" );
  getq.bindValue(":vohead_id", _voheadid);
  getq.exec();
  _miscDistrib->populate(getq);
  if (ErrorReporter::error(QtCriticalMsg, this, tr("Getting Misc. Distributions"),
                           getq, __FILE__, __LINE__))
    return;
}

void miscVoucher::sPopulateDistributed()
{
  XSqlQuery sumq;
  sumq.prepare( "SELECT COALESCE(SUM(vodist_amount), 0) AS distrib "
             "FROM vodist "
             "WHERE (vodist_vohead_id=:vohead_id);" );
  sumq.bindValue(":vohead_id", _voheadid);
  sumq.exec();
  if (sumq.first())
  {
    _amountDistributed->setLocalValue(sumq.value("distrib").toDouble());
    sPopulateBalanceDue();
  }
  else if (ErrorReporter::error(QtCriticalMsg, this, tr("Getting Total"),
                                sumq, __FILE__, __LINE__))
    return;
}

void miscVoucher::sPopulateBalanceDue()
{
  _balance->setLocalValue(_amountToDistribute->localValue() - _amountDistributed->localValue());
  if (_balance->isZero())
    _balance->setPaletteForegroundColor(QColor("black"));
  else
    _balance->setPaletteForegroundColor(QColor("red"));
}

void miscVoucher::populateNumber()
{
  XSqlQuery numq;
  numq.exec("SELECT fetchVoNumber() AS vouchernumber;");
  if (numq.first())
  {
    _voucherNumber->setText(numq.value("vouchernumber").toString());
    _voucherNumber->setEnabled(FALSE);
  }
  else if (ErrorReporter::error(QtCriticalMsg, this, tr("Getting Number"),
                                numq, __FILE__, __LINE__))
    return;
}

void miscVoucher::populate()
{
  XSqlQuery vohead;
  vohead.prepare( "SELECT vohead.* "
                  "FROM vohead "
                  "WHERE (vohead_id=:vohead_id);" );
  vohead.bindValue(":vohead_id", _voheadid);
  vohead.exec();
  if (vohead.first())
  {
    _voucherNumber->setText(vohead.value("vohead_number").toString());
    _vendor->setId(vohead.value("vohead_vend_id").toInt());
    _taxzone->setId(vohead.value("vohead_taxzone_id").toInt());
    _amountToDistribute->set(vohead.value("vohead_amount").toDouble(),
                             vohead.value("vohead_curr_id").toInt(),
                             vohead.value("vohead_docdate").toDate(), false);
    _terms->setId(vohead.value("vohead_terms_id").toInt());

    _distributionDate->setDate(vohead.value("vohead_distdate").toDate(), true);
    _invoiceDate->setDate(vohead.value("vohead_docdate").toDate());
    _dueDate->setDate(vohead.value("vohead_duedate").toDate());
    _invoiceNum->setText(vohead.value("vohead_invcnumber").toString());
    _reference->setText(vohead.value("vohead_reference").toString());
    _flagFor1099->setChecked(vohead.value("vohead_1099").toBool());
    _notes->setText(vohead.value("vohead_notes").toString());

    if(vohead.value("vohead_recurring_vohead_id").toInt() != 0)
      _recurring->setParent(vohead.value("vohead_recurring_vohead_id").toInt(), "V");
    else
      _recurring->setParent(_voheadid, "V");


    sFillMiscList();
    sPopulateDistributed();
  }
  else if (ErrorReporter::error(QtCriticalMsg, this, tr("Getting Voucher"),
                                vohead, __FILE__, __LINE__))
    return;
}

void miscVoucher::closeEvent(QCloseEvent *pEvent)
{
  if ( (_mode == cNew) && (_voheadid != -1) )
  {
    XSqlQuery delq;
    delq.prepare("DELETE FROM vohead WHERE (vohead_id=:vohead_id);");
    delq.bindValue(":vohead_id", _voheadid);
    delq.exec();
    ErrorReporter::error(QtCriticalMsg, this, tr("Deleting Voucher"),
                         delq, __FILE__, __LINE__);
  }

  pEvent->accept();
}

void miscVoucher::sPopulateDistDate()
{
  if ( _invoiceDate->isValid() && (!_distributionDate->isValid()) )
  {
    _distributionDate->setDate(_invoiceDate->date(), true);
    sPopulateDueDate();
  }
}

void miscVoucher::sPopulateDueDate()
{
  if ( _invoiceDate->isValid() && _terms->isValid() && (!_dueDate->isValid()) )
  {
    XSqlQuery dateq;
    dateq.prepare("SELECT determineDueDate(:terms_id, :invoiceDate) AS duedate;");
    dateq.bindValue(":terms_id", _terms->id());
    dateq.bindValue(":invoiceDate", _invoiceDate->date());
    dateq.exec();
    if (dateq.first())
      _dueDate->setDate(dateq.value("duedate").toDate());
    else if (ErrorReporter::error(QtCriticalMsg, this, tr("Getting Due Date"),
                                  dateq, __FILE__, __LINE__))
      return;
  }
}

void miscVoucher::keyPressEvent( QKeyEvent * e )
{
  if(e->key() == Qt::Key_Return || e->key() == Qt::Key_Enter)
  {
    _save->animateClick();
    e->accept();
  }
  else
    e->ignore();
}
