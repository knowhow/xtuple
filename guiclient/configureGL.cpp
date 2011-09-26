/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "configureGL.h"

#include <QMessageBox>
#include <QSqlError>
#include <QValidator>

#include "configureEncryption.h"
#include "guiclient.h"

configureGL::configureGL(QWidget* parent, const char* name, bool /*modal*/, Qt::WFlags fl)
    : XAbstractConfigure(parent, fl)
{
  setupUi(this);

  if (name)
    setObjectName(name);

  _yearend->setType(GLCluster::cEquity);
  _gainLoss->setType(GLCluster::cExpense);
  _discrepancy->setType(GLCluster::cExpense);

  // AP
  _nextAPMemoNumber->setValidator(omfgThis->orderVal());
  q.exec("SELECT currentAPMemoNumber() AS result;");
  if (q.first())
    _nextAPMemoNumber->setText(q.value("result"));

  _achGroup->setVisible(_metrics->boolean("ACHSupported"));
  if (_metrics->boolean("ACHSupported"))
  {
    _achGroup->setChecked(_metrics->boolean("ACHEnabled"));
    _nextACHBatchNumber->setValidator(omfgThis->orderVal());
    if (! _metrics->value("ACHCompanyId").trimmed().isEmpty())
      _companyId->setText(_metrics->value("ACHCompanyId"));
    if (! _metrics->value("ACHCompanyIdType").trimmed().isEmpty())
    {
      if (_metrics->value("ACHCompanyIdType").trimmed() == "D")
        _companyIdIsDUNS->setChecked(true);
      else if (_metrics->value("ACHCompanyIdType").trimmed() == "E")
        _companyIdIsEIN->setChecked(true);
      else if (_metrics->value("ACHCompanyIdType").trimmed() == "O")
        _companyIdIsOther->setChecked(true);
    }
    if (! _metrics->value("ACHCompanyName").trimmed().isEmpty())
      _companyName->setText(_metrics->value("ACHCompanyName"));

    QString eftsuffix = _metrics->value("ACHDefaultSuffix").trimmed();
    QString eftRregex = _metrics->value("EFTRoutingRegex").trimmed();
    QString eftAregex = _metrics->value("EFTAccountRegex").trimmed();
    QString eftproc   = _metrics->value("EFTFunction").trimmed();
    if (eftsuffix.isEmpty())
      _eftAch->setChecked(true);
    else if (eftsuffix == ".ach" &&
             eftRregex == _eftAchRoutingRegex->text() &&
             eftAregex == _eftAchAccountRegex->text() &&
             eftproc   == _eftAchFunction->text())
      _eftAch->setChecked(true);
    else if (eftsuffix == ".aba" &&
             eftRregex == _eftAbaRoutingRegex->text() &&
             eftAregex == _eftAbaAccountRegex->text() &&
             eftproc   == _eftAbaFunction->text())
      _eftAba->setChecked(true);
    else
    {
      _eftCustom->setChecked(true);
      _eftCustomRoutingRegex->setText(eftRregex);
      _eftCustomAccountRegex->setText(eftAregex);
      _eftCustomFunction->setText(eftproc);

       int suffixidx = _eftCustomSuffix->findText(_metrics->value("ACHDefaultSuffix"));
      if (suffixidx < 0)
      {
        _eftCustomSuffix->insertItem(0, _metrics->value("ACHDefaultSuffix"));
        _eftCustomSuffix->setCurrentIndex(0);
      }
      else
        _eftCustomSuffix->setCurrentIndex(suffixidx);
    }

    q.exec("SELECT currentNumber('ACHBatch') AS result;");
    if (q.first())
      _nextACHBatchNumber->setText(q.value("result"));
  }
  _reqInvoiceReg->setChecked(_metrics->boolean("ReqInvRegVoucher"));
  _reqInvoiceMisc->setChecked(_metrics->boolean("ReqInvMiscVoucher"));
  _recurringVoucherBuffer->setValue(_metrics->value("RecurringVoucherBuffer").toInt());

  // AR
  _nextARMemoNumber->setValidator(omfgThis->orderVal());
  _nextCashRcptNumber->setValidator(omfgThis->orderVal());

  q.exec("SELECT currentARMemoNumber() AS result;");
  if (q.first())
    _nextARMemoNumber->setText(q.value("result"));
  else if (q.lastError().type() != QSqlError::NoError)
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);

  q.exec("SELECT currentCashRcptNumber() AS result;");
  if (q.first())
    _nextCashRcptNumber->setText(q.value("result"));
  else if (q.lastError().type() != QSqlError::NoError)
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);

  _hideApplyto->setChecked(_metrics->boolean("HideApplyToBalance"));
  _customerDeposits->setChecked(_metrics->boolean("EnableCustomerDeposits"));
  _discountTax->setChecked(_metrics->boolean("CreditTaxDiscount"));

  _name->setText(_metrics->value("remitto_name"));
  _address->setLine1(_metrics->value("remitto_address1"));
  _address->setLine2(_metrics->value("remitto_address2"));
  _address->setLine3(_metrics->value("remitto_address3"));
  _address->setCity(_metrics->value("remitto_city"));
  _address->setState(_metrics->value("remitto_state"));
  _address->setPostalCode(_metrics->value("remitto_zipcode"));
  _address->setCountry(_metrics->value("remitto_country"));
  _phone->setText(_metrics->value("remitto_phone"));

  _warnLate->setChecked(_metrics->boolean("AutoCreditWarnLateCustomers"));
  if(!_metrics->value("DefaultAutoCreditWarnGraceDays").isEmpty())
    _graceDays->setValue(_metrics->value("DefaultAutoCreditWarnGraceDays").toInt());
  _incdtCategory->setId(_metrics->value("DefaultARIncidentStatus").toInt());
  _closeARIncdt->setChecked(_metrics->boolean("AutoCloseARIncident"));
  _recurringBuffer->setValue(_metrics->value("RecurringInvoiceBuffer").toInt());

  // GL
  _mainSize->setValue(_metrics->value("GLMainSize").toInt());

  bool extConsolAllowed = _metrics->value("Application") != "PostBooks";
  _externalConsolidation->setVisible(extConsolAllowed);
  if (_metrics->value("GLCompanySize").toInt() == 0)
  {
    _useCompanySegment->setChecked(FALSE);
    _externalConsolidation->setChecked(FALSE);
    _yearend->setId(_metrics->value("YearEndEquityAccount").toInt());
    _gainLoss->setId(_metrics->value("CurrencyGainLossAccount").toInt());
    _discrepancy->setId(_metrics->value("GLSeriesDiscrepancyAccount").toInt());
  }
  else
  {
    _useCompanySegment->setChecked(TRUE);
    _companySegmentSize->setValue(_metrics->value("GLCompanySize").toInt());

    _externalConsolidation->setChecked(_metrics->boolean("MultiCompanyFinancialConsolidation") &&
                                       extConsolAllowed);
  }

  if (_metrics->value("GLProfitSize").toInt() == 0)
    _useProfitCenters->setChecked(FALSE);
  else
  {
    _useProfitCenters->setChecked(TRUE);
    _profitCenterSize->setValue(_metrics->value("GLProfitSize").toInt());
    _ffProfitCenters->setChecked(_metrics->boolean("GLFFProfitCenters"));
  }

  if (_metrics->value("GLSubaccountSize").toInt() == 0)
    _useSubaccounts->setChecked(FALSE);
  else
  {
    _useSubaccounts->setChecked(TRUE);
    _subaccountSize->setValue(_metrics->value("GLSubaccountSize").toInt());
    _ffSubaccounts->setChecked(_metrics->boolean("GLFFSubaccounts"));
  }

  switch(_metrics->value("CurrencyExchangeSense").toInt())
  {
    case 1:
      _localToBase->setChecked(TRUE);
      break;
    case 0:
    default:
      _baseToLocal->setChecked(TRUE);
  }

  _mandatoryNotes->setChecked(_metrics->boolean("MandatoryGLEntryNotes"));
  _manualFwdUpdate->setChecked(_metrics->boolean("ManualForwardUpdate"));
  _taxauth->setId(_metrics->value("DefaultTaxAuthority").toInt());
  // TODO hide default tax authority, not used?
  _taxGroup->setVisible(FALSE);

  _int2gl->setChecked(_metrics->boolean("InterfaceToGL"));
  _cacheint2gl = _int2gl->isChecked();

  if (_metrics->boolean("UseJournals"))
  {
    _journal->setChecked(true);
    XSqlQuery qry;
    qry.exec("SELECT count(sltrans_id) > 0 AS result FROM sltrans WHERE (NOT sltrans_posted);");
    qry.first();
    if (qry.value("result").toBool())
      _postGroup->setEnabled(false);
  }
  else
    _generalLedger->setChecked(true);
  
  adjustSize();
}

configureGL::~configureGL()
{
  // no need to delete child widgets, Qt does it all for us
}

void configureGL::languageChange()
{
  retranslateUi(this);
}

bool configureGL::sSave()
{
  emit saving();

  if (!_cacheint2gl && _int2gl->isChecked())
  {
    q.exec("SELECT costcat_id "
           "FROM costcat "
           "WHERE (costcat_asset_accnt_id IS NULL) "
           "   OR (costcat_liability_accnt_id IS NULL) "
           "   OR (costcat_adjustment_accnt_id IS NULL) "
           "   OR (costcat_purchprice_accnt_id IS NULL) "
           "   OR (costcat_scrap_accnt_id IS NULL) "
           "   OR (costcat_invcost_accnt_id IS NULL) "
           "   OR (costcat_wip_accnt_id IS NULL) "
           "   OR (costcat_shipasset_accnt_id IS NULL) "
           "   OR (costcat_mfgscrap_accnt_id IS NULL) "
           "   OR (costcat_freight_accnt_id IS NULL) "
           "   OR (costcat_exp_accnt_id IS NULL) "
           "   OR (costcat_asset_accnt_id = -1) "
           "   OR (costcat_liability_accnt_id = -1) "
           "   OR (costcat_adjustment_accnt_id = -1) "
           "   OR (costcat_purchprice_accnt_id = -1) "
           "   OR (costcat_scrap_accnt_id = -1) "
           "   OR (costcat_invcost_accnt_id = -1) "
           "   OR (costcat_wip_accnt_id = -1) "
           "   OR (costcat_shipasset_accnt_id = -1) "
           "   OR (costcat_mfgscrap_accnt_id = -1) "
           "   OR (costcat_freight_accnt_id = -1) "
           "   OR (costcat_exp_accnt_id = -1) "
           "LIMIT 1;");
    if (q.first())
    {
      QMessageBox::critical(this, tr("Cannot Save Accounting Configuration"),
                            "You must assign G/L Accounts to all Cost Categories");
      return false;
    }
    if (_metrics->boolean("MultiWhs") && _metrics->boolean("Transforms"))
    {
      q.exec("SELECT costcat_id "
             "FROM costcat "
             "WHERE (costcat_transform_accnt_id IS NULL) "
             "   OR (costcat_transform_accnt_id = -1) "
             "LIMIT 1;");
      if (q.first())
      {
        QMessageBox::critical(this, tr("Cannot Save Accounting Configuration"),
                              "You must assign a Transform Clearing G/L Account to all Cost Categories");
        return false;
      }
    }
    if (_metrics->boolean("MultiWhs"))
    {
      q.exec("SELECT costcat_id "
             "FROM costcat "
             "WHERE (costcat_toliability_accnt_id IS NULL) "
             "   OR (costcat_toliability_accnt_id = -1) "
             "LIMIT 1;");
      if (q.first())
      {
        QMessageBox::critical(this, tr("Cannot Save Accounting Configuration"),
                              "You must assign a Transfer Order Liability Clearing G/L Account to all Cost Categories");
        return false;
      }
    }
    if (_metrics->boolean("Routings"))
    {
      q.exec("SELECT costcat_id "
             "FROM costcat "
             "WHERE (costcat_laboroverhead_accnt_id IS NULL) "
             "   OR (costcat_laboroverhead_accnt_id = -1) "
             "LIMIT 1;");
      if (q.first())
      {
        QMessageBox::critical(this, tr("Cannot Save Accounting Configuration"),
                              "You must assign a Labor and Overhead Costs G/L Account to all Cost Categories");
        return false;
      }
    }
    q.exec("SELECT expcat_id "
           "FROM expcat "
           "WHERE (expcat_exp_accnt_id IS NULL) "
           "   OR (expcat_liability_accnt_id IS NULL) "
           "   OR (expcat_freight_accnt_id IS NULL) "
           "   OR (expcat_purchprice_accnt_id IS NULL) "
           "   OR (expcat_exp_accnt_id = -1) "
           "   OR (expcat_liability_accnt_id = -1) "
           "   OR (expcat_freight_accnt_id = -1) "
           "   OR (expcat_purchprice_accnt_id = -1) "
           "LIMIT 1;");
    if (q.first())
    {
      QMessageBox::critical(this, tr("Cannot Save Accounting Configuration"),
                            "You must assign G/L Accounts to all Expense Categories");
      return false;
    }
    q.exec("SELECT apaccnt_id "
           "FROM apaccnt "
           "WHERE (apaccnt_ap_accnt_id IS NULL) "
           "   OR (apaccnt_prepaid_accnt_id IS NULL) "
           "   OR (apaccnt_discount_accnt_id IS NULL) "
           "   OR (apaccnt_ap_accnt_id = -1) "
           "   OR (apaccnt_prepaid_accnt_id = -1) "
           "   OR (apaccnt_discount_accnt_id = -1) "
           "LIMIT 1;");
    if (q.first())
    {
      QMessageBox::critical(this, tr("Cannot Save Accounting Configuration"),
                            "You must assign G/L Accounts to all Payables Assignments");
      return false;
    }
    q.exec("SELECT araccnt_id "
           "FROM araccnt "
           "WHERE (araccnt_ar_accnt_id IS NULL) "
           "   OR (araccnt_prepaid_accnt_id IS NULL) "
           "   OR (araccnt_discount_accnt_id IS NULL) "
           "   OR (araccnt_freight_accnt_id IS NULL) "
           "   OR (araccnt_ar_accnt_id = -1) "
           "   OR (araccnt_prepaid_accnt_id = -1) "
           "   OR (araccnt_discount_accnt_id = -1) "
           "   OR (araccnt_freight_accnt_id = -1) "
           "LIMIT 1;");
    if (q.first())
    {
      QMessageBox::critical(this, tr("Cannot Save Accounting Configuration"),
                            "You must assign G/L Accounts to all Receivables Assignments");
      return false;
    }
    if (_metrics->boolean("EnableCustomerDeposits"))
    {
      q.exec("SELECT araccnt_id "
             "FROM araccnt "
             "WHERE (araccnt_deferred_accnt_id IS NULL) "
             "   OR (araccnt_deferred_accnt_id = -1) "
             "LIMIT 1;");
      if (q.first())
      {
        QMessageBox::critical(this, tr("Cannot Save Accounting Configuration"),
                              "You must assign a Deferred Revenue G/L Account to all Receivables Assignments");
        return false;
      }
    }
    q.exec("SELECT salesaccnt_id "
           "FROM salesaccnt "
           "WHERE (salesaccnt_sales_accnt_id IS NULL) "
           "   OR (salesaccnt_credit_accnt_id IS NULL) "
           "   OR (salesaccnt_cos_accnt_id IS NULL) "
           "   OR (salesaccnt_cor_accnt_id IS NULL) "
           "   OR (salesaccnt_cow_accnt_id IS NULL) "
           "   OR (salesaccnt_sales_accnt_id = -1) "
           "   OR (salesaccnt_credit_accnt_id = -1) "
           "   OR (salesaccnt_cos_accnt_id = -1) "
           "   OR (salesaccnt_cor_accnt_id = -1) "
           "   OR (salesaccnt_cow_accnt_id = -1) "
           "LIMIT 1;");
    if (q.first())
    {
      QMessageBox::critical(this, tr("Cannot Save Accounting Configuration"),
                            "You must assign G/L Accounts to all Sales Assignments");
      return false;
    }
    if (_metrics->boolean("EnableReturnAuth"))
    {
      q.exec("SELECT salesaccnt_id "
             "FROM salesaccnt "
             "WHERE (salesaccnt_returns_accnt_id IS NULL) "
             "   OR (salesaccnt_returns_accnt_id = -1) "
             "LIMIT 1;");
      if (q.first())
      {
        QMessageBox::critical(this, tr("Cannot Save Accounting Configuration"),
                              "You must assign a Returns G/L Account to all Sales Assignments");
        return false;
      }
    }
    q.exec("SELECT salescat_id "
           "FROM salescat "
           "WHERE (salescat_sales_accnt_id IS NULL) "
           "   OR (salescat_prepaid_accnt_id IS NULL) "
           "   OR (salescat_ar_accnt_id IS NULL) "
           "   OR (salescat_sales_accnt_id = -1) "
           "   OR (salescat_prepaid_accnt_id = -1) "
           "   OR (salescat_ar_accnt_id = -1) "
           "LIMIT 1;");
    if (q.first())
    {
      QMessageBox::critical(this, tr("Cannot Save Accounting Configuration"),
                            "You must assign G/L Accounts to all Sales Categories");
      return false;
    }
  }

  if (_metrics->boolean("ACHSupported"))
  {
    QString tmpCompanyId = _companyId->text();
    struct {
      bool    condition;
      QString msg;
      QWidget *widget;
    } error[] = {
      { _achGroup->isChecked() && _companyId->text().isEmpty(),
        tr("Please enter a default Company Id if you are going to create "
           "ACH files."),
        _companyId },
      { _achGroup->isChecked() &&
        (_companyIdIsEIN->isChecked() || _companyIdIsDUNS->isChecked()) && 
        tmpCompanyId.remove("-").size() != 9,
        tr("EIN, TIN, and DUNS numbers are all 9 digit numbers. Other "
           "characters (except dashes for readability) are not allowed."),
        _companyId },
      { _achGroup->isChecked() &&
        _companyIdIsOther->isChecked() && _companyId->text().size() > 10,
        tr("Company Ids must be 10 characters or shorter (not counting dashes "
           "in EIN's, TIN's, and DUNS numbers)."),
        _companyId },
      { _achGroup->isChecked() &&
        ! (_companyIdIsEIN->isChecked() || _companyIdIsDUNS->isChecked() ||
           _companyIdIsOther->isChecked()),
        tr("Please mark whether the Company Id is an EIN, TIN, DUNS number, "
           "or Other."),
        _companyIdIsEIN }
    };
    for (unsigned int i = 0; i < sizeof(error) / sizeof(error[0]); i++)
      if (error[i].condition)
      {
        QMessageBox::critical(this, tr("Cannot Save Accounting Configuration"),
                              error[i].msg);
        error[i].widget->setFocus();
        return false;
      }
  }

  // AP
  q.prepare("SELECT setNextAPMemoNumber(:armemo_number) AS result;");
  q.bindValue(":armemo_number", _nextAPMemoNumber->text().toInt());
  q.exec();

  if (_metrics->boolean("ACHSupported"))
  {
    _metrics->set("ACHEnabled",           _achGroup->isChecked());
    if (_achGroup->isChecked())
    {
      _metrics->set("ACHCompanyId",     _companyId->text().trimmed());
      if (_companyId->text().trimmed().length() > 0)
      {
        if (_companyIdIsDUNS->isChecked())
          _metrics->set("ACHCompanyIdType", QString("D"));
        else if (_companyIdIsEIN->isChecked())
          _metrics->set("ACHCompanyIdType", QString("E"));
        else if (_companyIdIsOther->isChecked())
          _metrics->set("ACHCompanyIdType", QString("O"));
      }
      _metrics->set("ACHCompanyName",   _companyName->text().trimmed());

      if (_eftAch->isChecked())
      {
        _metrics->set("ACHDefaultSuffix", _eftAchSuffix->text().trimmed());
        _metrics->set("EFTRoutingRegex",  _eftAchRoutingRegex->text());
        _metrics->set("EFTAccountRegex",  _eftAchAccountRegex->text());
        _metrics->set("EFTFunction",      _eftAchFunction->text());
      }
      else if (_eftAba->isChecked())
      {
        _metrics->set("ACHDefaultSuffix", _eftAbaSuffix->text().trimmed());
        _metrics->set("EFTRoutingRegex",  _eftAbaRoutingRegex->text());
        _metrics->set("EFTAccountRegex",  _eftAbaAccountRegex->text());
        _metrics->set("EFTFunction",      _eftAbaFunction->text());
      }
      else
      {
        _metrics->set("ACHDefaultSuffix", _eftCustomSuffix->currentText().trimmed());
        _metrics->set("EFTRoutingRegex",  _eftCustomRoutingRegex->text().trimmed());
        _metrics->set("EFTAccountRegex",  _eftCustomAccountRegex->text().trimmed());
        _metrics->set("EFTFunction",      _eftCustomFunction->text().trimmed());
      }

      q.prepare("SELECT setNextNumber('ACHBatch', :number) AS result;");
      q.bindValue(":number", _nextACHBatchNumber->text().toInt());
      q.exec();
    }
  }
  _metrics->set("ReqInvRegVoucher", _reqInvoiceReg->isChecked());
  _metrics->set("ReqInvMiscVoucher", _reqInvoiceMisc->isChecked());
  _metrics->set("RecurringVoucherBuffer", _recurringVoucherBuffer->value());

  // AR
  q.prepare("SELECT setNextARMemoNumber(:armemo_number) AS result;");
  q.bindValue(":armemo_number", _nextARMemoNumber->text().toInt());
  q.exec();
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    _nextARMemoNumber->setFocus();
    return false;
  }

  q.prepare("SELECT setNextCashRcptNumber(:cashrcpt_number) AS result;");
  q.bindValue(":cashrcpt_number", _nextCashRcptNumber->text().toInt());
  q.exec();
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    _nextCashRcptNumber->setFocus();
    return false;
  }

  _metrics->set("HideApplyToBalance", _hideApplyto->isChecked());
  _metrics->set("EnableCustomerDeposits", _customerDeposits->isChecked());
  _metrics->set("CreditTaxDiscount", _discountTax->isChecked());

  _metrics->set("remitto_name", 	_name->text().trimmed());
  _metrics->set("remitto_address1",	_address->line1().trimmed());
  _metrics->set("remitto_address2",	_address->line2().trimmed());
  _metrics->set("remitto_address3",	_address->line3().trimmed());
  _metrics->set("remitto_city",		_address->city().trimmed());
  _metrics->set("remitto_state",	_address->state().trimmed());
  _metrics->set("remitto_zipcode",	_address->postalCode().trimmed());
  _metrics->set("remitto_country",	_address->country().trimmed());
  _metrics->set("remitto_phone",	_phone->text().trimmed());
  _address->save(AddressCluster::CHANGEONE);
  
  _metrics->set("AutoCreditWarnLateCustomers", _warnLate->isChecked());
  if(_warnLate->isChecked())
    _metrics->set("DefaultAutoCreditWarnGraceDays", _graceDays->value());

  _metrics->set("RecurringInvoiceBuffer", _recurringBuffer->value());
  _metrics->set("DefaultARIncidentStatus", _incdtCategory->id());
  _metrics->set("AutoCloseARIncident", _closeARIncdt->isChecked());
  
  // GL
  QAction *profitcenter = omfgThis->findChild<QAction*>("gl.profitCenterNumber");
  QAction *subaccounts  = omfgThis->findChild<QAction*>("gl.subaccountNumbers");
  QAction *companyseg   = omfgThis->findChild<QAction*>("gl.companies");

  _metrics->set("GLMainSize", _mainSize->value());

  if (_useCompanySegment->isChecked())
  {
    _metrics->set("GLCompanySize", _companySegmentSize->value());
    _metrics->set("MultiCompanyFinancialConsolidation", _externalConsolidation->isChecked());
  }
  else
  {
    _metrics->set("GLCompanySize", 0);
    _metrics->set("MultiCompanyFinancialConsolidation", 0);
    _metrics->set("YearEndEquityAccount", _yearend->id());
    _metrics->set("CurrencyGainLossAccount", _gainLoss->id());
    _metrics->set("GLSeriesDiscrepancyAccount", _discrepancy->id());
  }
  if(companyseg)
    companyseg->setEnabled(_useCompanySegment->isChecked());

  if (_useProfitCenters->isChecked())
  {
    _metrics->set("GLProfitSize", _profitCenterSize->value());
    _metrics->set("GLFFProfitCenters", _ffProfitCenters->isChecked());
    if(profitcenter)
      profitcenter->setEnabled(_privileges->check("MaintainChartOfAccounts"));
  }
  else
  {
    _metrics->set("GLProfitSize", 0);
    _metrics->set("GLFFProfitCenters", FALSE);
    if(profitcenter)
      profitcenter->setEnabled(FALSE);
  }

  if (_useSubaccounts->isChecked())
  {
    _metrics->set("GLSubaccountSize", _subaccountSize->value());
    _metrics->set("GLFFSubaccounts", _ffSubaccounts->isChecked());
    if(subaccounts)
      subaccounts->setEnabled(_privileges->check("MaintainChartOfAccounts"));

  }
  else
  {
    _metrics->set("GLSubaccountSize", 0);
    _metrics->set("GLFFSubaccounts", FALSE);
    if(subaccounts)
      subaccounts->setEnabled(FALSE);
  }

  _metrics->set("UseJournals", _journal->isChecked());

  if(_localToBase->isChecked())
    _metrics->set("CurrencyExchangeSense", 1);
  else // if(_baseToLocal->isChecked())
    _metrics->set("CurrencyExchangeSense", 0);

  _metrics->set("MandatoryGLEntryNotes", _mandatoryNotes->isChecked());
  _metrics->set("ManualForwardUpdate", _manualFwdUpdate->isChecked());
  _metrics->set("DefaultTaxAuthority", _taxauth->id());

  _metrics->set("InterfaceToGL", _int2gl->isChecked());

  omfgThis->sConfigureGLUpdated();

  if (_metrics->boolean("ACHSupported") && _metrics->boolean("ACHEnabled") &&
      omfgThis->_key.isEmpty())
  {
    if (_privileges->check("ConfigureEncryption"))
    {
      if (QMessageBox::question(this, tr("Set Encryption?"),
                                tr("Your encryption key is not set. You will "
                                   "not be able to configure electronic "
                                   "checking information for Vendors until you "
                                   "configure encryption. Would you like to do "
                                   "this now?"),
                                    QMessageBox::Yes | QMessageBox::Default,
                                    QMessageBox::No ) == QMessageBox::Yes)
        return false;
    }
    else
      QMessageBox::question(this, tr("Set Encryption?"),
                            tr("Your encryption key is not set. You will "
                               "not be able to configure electronic "
                               "checking information for Vendors until the "
                               "system is configured to perform encryption."));
  }

  return true;
}
