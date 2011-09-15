/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "apOpenItem.h"

#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include "taxDetail.h"

apOpenItem::apOpenItem(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_docDate,        SIGNAL(newDate(const QDate&)),          this, SLOT(sPopulateDueDate()));
  connect(_buttonBox,      SIGNAL(accepted()),                     this, SLOT(sSave()));
  connect(_buttonBox,      SIGNAL(rejected()),                     this, SLOT(sClose()));
  connect(_terms,          SIGNAL(newID(int)),                     this, SLOT(sPopulateDueDate()));
  connect(_vend,           SIGNAL(newId(int)),                     this, SLOT(sPopulateVendInfo(int)));
  connect(_taxLit,         SIGNAL(leftClickedURL(const QString&)), this, SLOT(sTaxDetail()));

  _cAmount = 0.0;
  _apopenid = -1;

  _apapply->addColumn( tr("Type"),        _dateColumn, Qt::AlignCenter,true, "doctype");
  _apapply->addColumn( tr("Doc. #"),               -1, Qt::AlignLeft,  true, "docnumber");
  _apapply->addColumn( tr("Apply Date"),  _dateColumn, Qt::AlignCenter,true, "apapply_postdate");
  _apapply->addColumn( tr("Amount"),     _moneyColumn, Qt::AlignRight, true, "apapply_amount");
  _apapply->addColumn( tr("Currency"),_currencyColumn, Qt::AlignLeft,  true, "currabbr");

  if (omfgThis->singleCurrency())
      _apapply->hideColumn("currabbr");

  _terms->setType(XComboBox::APTerms);
  _journalNumber->setEnabled(FALSE);

  _altAccntid->setType(GLCluster::cRevenue | GLCluster::cExpense);
}

apOpenItem::~apOpenItem()
{
  // no need to delete child widgets, Qt does it all for us
}

void apOpenItem::languageChange()
{
  retranslateUi(this);
}

enum SetResponse apOpenItem::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  QVariant param;
  bool     valid;
  
  param = pParams.value("docType", &valid);
  if (valid)
  {
    if (param.toString() == "creditMemo")
    {
      setWindowTitle(windowTitle() + tr(" - Enter Misc. Credit Memo"));
      _docType->setCurrentIndex(0);
      _status->setEnabled(false);
    }
    else if (param.toString() == "debitMemo")
    {
      setWindowTitle(windowTitle() + tr(" - Enter Misc. Debit Memo"));
      _docType->setCurrentIndex(1);
    }
    else if (param.toString() == "voucher")
      _docType->setCurrentIndex(2);
    else
      return UndefinedError;
//  ToDo - better error return types

    _docType->setEnabled(FALSE);
  }

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      _mode = cNew;

      q.exec("SELECT fetchAPMemoNumber() AS number;");
      if (q.first())
        _docNumber->setText(q.value("number").toString());
//  ToDo

      _paid->clear();
      _buttonBox->button(QDialogButtonBox::Save)->setText(tr("Post"));
      populateStatus();
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;

      _vend->setReadOnly(TRUE);
      _docDate->setEnabled(FALSE);
      _dueDate->setEnabled(FALSE);
      _docType->setEnabled(FALSE);
      _docNumber->setEnabled(FALSE);
      _poNumber->setEnabled(FALSE);
      _journalNumber->setEnabled(FALSE);
      _terms->setEnabled(FALSE);
      _notes->setReadOnly(FALSE);
      _useAltPrepaid->setEnabled(FALSE);
      _altAccntid->setEnabled(FALSE);
    }
    else if (param.toString() == "view")
    {
      _mode = cView;

      _vend->setReadOnly(TRUE);
      _docDate->setEnabled(FALSE);
      _dueDate->setEnabled(FALSE);
      _docType->setEnabled(FALSE);
      _docNumber->setEnabled(FALSE);
      _poNumber->setEnabled(FALSE);
      _journalNumber->setEnabled(FALSE);
      _amount->setEnabled(FALSE);
      _terms->setEnabled(FALSE);
      _terms->setType(XComboBox::Terms);
      _notes->setReadOnly(TRUE);
      _useAltPrepaid->setEnabled(FALSE);
      _altAccntid->setEnabled(FALSE);
      _status->setEnabled(FALSE);
      _buttonBox->setStandardButtons(QDialogButtonBox::Close);
    }
  }

  param = pParams.value("vend_id", &valid);
  if (valid)
    _vend->setId(param.toInt());

  param = pParams.value("apopen_id", &valid);
  if (valid)
  {
    _apopenid = param.toInt();
    populate();
  }

  return NoError;
}

void apOpenItem::sSave()
{
  if (_mode == cNew)
  {
    if (!_docDate->isValid())
    {
      QMessageBox::critical( this, tr("Cannot Save A/P Memo"),
                             tr("<p>You must enter a date for this A/P Memo "
                                "before you may save it") );
      _docDate->setFocus();
      return;
    }

    if (!_dueDate->isValid())
    {
      QMessageBox::critical( this, tr("Cannot Save A/P Memo"),
                             tr("<p>You must enter a date for this A/P Memo "
                                "before you may save it") );
      _dueDate->setFocus();
      return;
    }

    if (_amount->isZero())
    {
      QMessageBox::critical( this, tr("Cannot Save A/P Memo"),
                             tr("<p>You must enter an amount for this A/P Memo "
                                "before you may save it") );
      _amount->setFocus();
      return;
    }

    if(_tax->localValue() > _amount->localValue())
    {
      QMessageBox::critical( this, tr("Cannot Save A/P Memo"),
                             tr("The tax amount may not be greater than the total A/P Memo amount.") );
      return;
    }

    if (_useAltPrepaid->isChecked() && (!_altAccntid->isValid()))
    {
      QMessageBox::critical( this, tr("Cannot Save A/P Memo"),
                            tr("<p>You must choose a valid Alternate Prepaid "
                               "Account Number for this A/P Memo before you "
                               "may save it.") );
      return;
    }

    QString tmpFunctionName;
    QString queryStr;

    if (_docType->currentIndex() == 0)
      tmpFunctionName = "createAPCreditMemo";
    else if (_docType->currentIndex() == 1)
      tmpFunctionName = "createAPDebitMemo";
    else
    {
      systemError(this,
		  tr("Internal Error: _docType has an invalid document type %1")
		  .arg(_docType->currentIndex()), __FILE__, __LINE__);
      return;
    }

    queryStr = "SELECT " + tmpFunctionName + "( :apopen_id, :vend_id, " +
		(_journalNumber->text().isEmpty() ?
		      QString("fetchJournalNumber('AP-MISC')") : _journalNumber->text()) +
	       ", :apopen_docnumber, :apopen_ponumber, :apopen_docdate,"
	       "  :apopen_amount, :apopen_notes, :apopen_accnt_id,"
	       "  :apopen_duedate, :apopen_terms_id, :curr_id ) AS result;";
    q.prepare(queryStr);
    q.bindValue(":vend_id", _vend->id());
    q.bindValue(":apopen_docdate", _docDate->date());
  }
  else if (_mode == cEdit)
  {
    if (_cAmount != _amount->localValue())
      if ( QMessageBox::warning( this, tr("A/P Open Amount Changed"),
                                 tr( "<p>You are changing the open amount of "
                                    "this A/P Open Item.  If you do not post a "
                                    "G/L Transaction to distribute this change "
                                    "then the A/P Open Item total will be out "
                                    "of balance with the A/P Trial Balance(s). "
                                    "Are you sure that you want to save this "
                                    "change?" ),
                                 tr("Yes"), tr("No"), QString::null ) == 1 )
        return;

    q.prepare( "UPDATE apopen "
	       "SET apopen_doctype=:apopen_doctype,"
               "    apopen_ponumber=:apopen_ponumber, apopen_docnumber=:apopen_docnumber,"
               "    apopen_amount=:apopen_amount,"
               "    apopen_terms_id=:apopen_terms_id, "
	       "    apopen_notes=:apopen_notes, "
	           "    apopen_curr_id=:curr_id, "
		       "    apopen_status = :apopen_status "
               "WHERE (apopen_id=:apopen_id);" );
  }

  if (_apopenid != -1)
    q.bindValue(":apopen_id", _apopenid);
  q.bindValue(":apopen_docnumber", _docNumber->text());
  q.bindValue(":apopen_duedate", _dueDate->date());
  q.bindValue(":apopen_ponumber", _poNumber->text());
  q.bindValue(":apopen_amount", _amount->localValue());
  q.bindValue(":apopen_notes",   _notes->toPlainText());
  q.bindValue(":curr_id", _amount->id());
  q.bindValue(":apopen_terms_id", _terms->id());
  QString temp;
  if (_status->id() == 1)
    temp = "O" ;
  else
	temp = "H" ;
  q.bindValue(":apopen_status", temp);
  if(_useAltPrepaid->isChecked())
    q.bindValue(":apopen_accnt_id", _altAccntid->id());
  else
    q.bindValue(":apopen_accnt_id", -1);

  switch (_docType->currentIndex())
  {
    case 0:
      q.bindValue(":apopen_doctype", "C");
      break;

    case 1:
      q.bindValue(":apopen_doctype", "D");
      break;

    case 2:
      q.bindValue(":apopen_doctype", "V");
      break;
  }

  q.exec();
  if (q.first())
  {
    if (_mode == cNew)
    {
      if (q.value("result").toInt() == -1)
      {
        QMessageBox::critical( this, tr("Cannot Create A/P Memo"),
                               tr( "<p>The A/P Memo cannot be created as there "
                                  "are missing A/P Account Assignments for the "
                                  "selected Vendor. You must create an A/P "
                                  "Account Assignment for the selected "
                                  "Vendor's Vendor Type before you may create "
                                  "this A/P Memo." ) );
        return;
      }
    }
  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  if (_mode == cEdit)
    done(_apopenid);
  else
    done(q.value("result").toInt());
}

void apOpenItem::sClose()
{
  if (_mode == cNew)
  {
    q.prepare("SELECT releaseAPMemoNumber(:docNumber);");
    q.bindValue(":docNumber", _docNumber->text().toInt());
    q.exec();
  }

  reject();
}

void apOpenItem::populate()
{
  populateStatus();
  q.prepare( "SELECT apopen_vend_id, apopen_docdate, apopen_duedate,"
             "       apopen_doctype, apopen_docnumber,"
             "       apopen_ponumber, apopen_journalnumber,"
             "       apopen_amount,   apopen_paid, "
             "       (apopen_amount - apopen_paid) AS f_balance,"
             "       apopen_terms_id, apopen_notes, apopen_accnt_id, "
             "       apopen_curr_id, "
			 "       CASE WHEN apopen_status ='O' THEN 1 "
             "         ELSE CASE WHEN apopen_status = 'H' THEN 2 "
             "         END "
             "       END AS status_id, apopen_status, "
             "       (SELECT COALESCE(SUM(taxhist_tax),0) "
             "        FROM apopentax "
             "        WHERE (taxhist_parent_id=apopen_id)) AS tax, "
             "       CASE WHEN (apopen_doctype IN ('D', 'C')) THEN TRUE "
             "            ELSE FALSE "
             "       END AS showTax "
             "FROM apopen "
             "WHERE ( (apopen_id=:apopen_id)"
             "  AND   (apopen_void = FALSE) );" );
  q.bindValue(":apopen_id", _apopenid);
  q.exec();
  if (q.first())
  {
    _vend->setId(q.value("apopen_vend_id").toInt());
    _docDate->setDate(q.value("apopen_docdate").toDate(), true);
    _dueDate->setDate(q.value("apopen_duedate").toDate());
    _docNumber->setText(q.value("apopen_docnumber").toString());
    _poNumber->setText(q.value("apopen_ponumber").toString());
    _journalNumber->setText(q.value("apopen_journalnumber").toString());
    _amount->setId(q.value("apopen_curr_id").toInt());
    _amount->setLocalValue(q.value("apopen_amount").toDouble());
    _paid->setLocalValue(q.value("apopen_paid").toDouble());
    _balance->setLocalValue(q.value("f_balance").toDouble());
    _terms->setId(q.value("apopen_terms_id").toInt());
	if (q.value("apopen_status").toString() == "C")
	{
      QString status;
      status = " SELECT DISTINCT "
            " CASE WHEN apopen_status ='C' THEN 0 "
            " END AS status_id, "
            " CASE WHEN apopen_status ='C' THEN TEXT('Closed') "
            " END AS status, "
            " CASE WHEN apopen_status ='C' THEN TEXT('Closed') "
            " END AS status "
            " FROM apopen "
            " WHERE apopen_status <> '' " ;
	  _status->populate(status, -1);
	  _status->setEnabled(FALSE);
	}
	_status->setId(q.value("status_id").toInt());
	
	XSqlQuery selectpayment;
    selectpayment.prepare("SELECT * FROM apselect WHERE apselect_apopen_id = :apopen_id;");
    selectpayment.bindValue(":apopen_id", _apopenid);
    selectpayment.exec();
    if (selectpayment.first())
      _status->setEnabled(FALSE);
    _notes->setText(q.value("apopen_notes").toString());
    if (q.value("showTax").toBool())
      _tax->setLocalValue(q.value("tax").toDouble());
    else
    {
      _taxLit->hide();
      _tax->hide();
    }

    if(!q.value("apopen_accnt_id").isNull() && q.value("apopen_accnt_id").toInt() != -1)
    {
      _useAltPrepaid->setChecked(true);
      _altAccntid->setId(q.value("apopen_accnt_id").toInt());
    }

    QString docType = q.value("apopen_doctype").toString();
    if (docType == "C")
      _docType->setCurrentIndex(0);
    else if (docType == "D")
      _docType->setCurrentIndex(1);
    else if (docType == "V")
      _docType->setCurrentIndex(2);

    _cAmount = _amount->localValue();

    if ( (docType == "V") || (docType == "D") )
    {
      q.prepare( "SELECT apapply_id, apapply_source_apopen_id,"
                 "       CASE WHEN (apapply_source_doctype='C') THEN :creditMemo"
                 "            WHEN (apapply_source_doctype='K') THEN :check"
                 "            ELSE :other"
                 "       END AS doctype,"
                 "       apapply_source_docnumber AS docnumber,"
                 "       apapply_postdate, apapply_amount,"
		 "       currConcat(apapply_curr_id) AS currabbr,"
                 "       'curr' AS apapply_amount_xtnumericrole "
                 "FROM apapply "
                 "WHERE (apapply_target_apopen_id=:apopen_id) "
                 "ORDER BY apapply_postdate;" );

      q.bindValue(":creditMemo", tr("Credit Memo"));
      q.bindValue(":check", tr("Check"));
    }
    else if (docType == "C")
    {
      q.prepare( "SELECT apapply_id, apapply_target_apopen_id,"
                 "       CASE WHEN (apapply_target_doctype='V') THEN :voucher"
                 "            WHEN (apapply_target_doctype='D') THEN :debitMemo"
                 "            ELSE :other"
                 "       END AS doctype,"
                 "       apapply_target_docnumber AS docnumber,"
                 "       apapply_postdate, apapply_amount,"
		 "       currConcat(apapply_curr_id) AS currabbr,"
                 "       'curr' AS apapply_amount_xtnumericrole "
                 "FROM apapply "
                 "WHERE (apapply_source_apopen_id=:apopen_id) "
                 "ORDER BY apapply_postdate;" );

      q.bindValue(":voucher", tr("Voucher"));
      q.bindValue(":debitMemo", tr("Debit Memo"));
    }

    q.bindValue(":apopen_id", _apopenid);
    q.bindValue(":other", tr("Other"));
    q.exec();
    if (q.lastError().type() != QSqlError::NoError)
	systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    _apapply->populate(q, TRUE);
    if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }
}

void apOpenItem::sPopulateVendInfo(int vend_id)
{
  XSqlQuery vendor;
  vendor.prepare("SELECT vend_curr_id,"
                 "       vend_terms_id "
                 "  FROM vend"
                 " WHERE(vend_id = :vend_id);");
  vendor.bindValue(":vend_id", vend_id);
  vendor.exec();
  if (vendor.first())
  {
    _amount->setId(vendor.value("vend_curr_id").toInt());
    _tax->setId(vendor.value("vend_curr_id").toInt());
    _terms->setId(vendor.value("vend_terms_id").toInt());
  }
}

void apOpenItem::sPopulateDueDate()
{
  if ( (_terms->isValid()) && (_docDate->isValid()) && (!_dueDate->isValid()) )
  {
    XSqlQuery dueq;
    dueq.prepare("SELECT determineDueDate(:terms_id, :docDate) AS duedate;");
    dueq.bindValue(":terms_id", _terms->id());
    dueq.bindValue(":docDate", _docDate->date());
    dueq.exec();
    if (dueq.first())
      _dueDate->setDate(dueq.value("duedate").toDate());
    else if (dueq.lastError().type() != QSqlError::NoError)
    {
      systemError(this, dueq.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }
}

void apOpenItem::sTaxDetail()
{ 
  if (_apopenid == -1)
  {
    if (!_docDate->isValid() || !_dueDate->isValid())
    {
      QMessageBox::critical( this, tr("Cannot set tax amounts"),
                             tr("You must enter document and due dates for this A/P Memo before you may set tax amounts.") );
      _docDate->setFocus();
      return;
    }
    
    XSqlQuery ap;
    ap.prepare("SELECT nextval('apopen_apopen_id_seq') AS result;");
    ap.exec();
    if (ap.first())
      _apopenid = ap.value("result").toInt();
    else if (ap.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
    else
      return;
    
    ap.prepare("INSERT INTO apopen "
      "( apopen_id, apopen_docdate, apopen_duedate, apopen_doctype, "
      "  apopen_docnumber, apopen_curr_id, apopen_posted, apopen_amount ) "
      "VALUES "
      "( :apopen_id, :docDate, :dueDate, :docType, :docNumber, :currId, false, 0 ); ");
    ap.bindValue(":apopen_id",_apopenid);
    ap.bindValue(":docDate", _docDate->date());
    ap.bindValue(":dueDate", _dueDate->date());
    if (_docType->currentIndex())
      ap.bindValue(":docType", "D" );
    else
      ap.bindValue(":docType", "C" );
    ap.bindValue(":docNumber", _docNumber->text());
    ap.bindValue(":currId", _amount->id());
    ap.exec();
    if (ap.lastError().type() != QSqlError::NoError)
    {
      q.exec("ROLLBACK;");
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }
  
  taxDetail newdlg(this, "", true);
  ParameterList params;

  params.append("curr_id", _tax->id());
  params.append("date",    _tax->effective());
  if (_mode != cNew)
    params.append("readOnly");

  q.exec("SELECT getadjustmenttaxtypeid() as taxtype;");
  if(q.first())
    params.append("taxtype_id", q.value("taxtype").toInt());  
   
  params.append("order_type", "AP");
  params.append("order_id", _apopenid);
  params.append("display_type", "A");
  params.append("subtotal", _amount->localValue());
  params.append("adjustment");
  if (_docType->currentIndex())
    params.append("sense",-1);
  if (newdlg.set(params) == NoError)  
  {
    newdlg.exec();
    XSqlQuery taxq;
    taxq.prepare( "SELECT SUM(taxhist_tax) AS tax "
      "FROM apopentax "
      "WHERE (taxhist_parent_id=:apopen_id);" );
    taxq.bindValue(":apopen_id", _apopenid);
    taxq.exec();
    if (taxq.first())
    {
      if (_docType->currentIndex())
        _tax->setLocalValue(taxq.value("tax").toDouble() * -1);
      else
        _tax->setLocalValue(taxq.value("tax").toDouble());
    }
    else if (taxq.lastError().type() != QSqlError::NoError)
    {
      systemError(this, taxq.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }
}
 
void apOpenItem::populateStatus()
{
  QString status;
  status = "SELECT 1 AS status_id, TEXT('Open') AS status, TEXT('Open') AS status "
           "UNION "
           "SELECT 2 AS status_id, TEXT('On Hold') AS status, TEXT('On Hold') AS status;";
  _status->populate(status, -1);
}
