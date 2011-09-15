/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "vendorWorkBench.h"

#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include <metasql.h>
#include <openreports.h>
#include <parameter.h>

#include "crmaccount.h"
#include "dspAPApplications.h"
#include "dspCheckRegister.h"
#include "dspPOsByVendor.h"
#include "dspPoItemReceivingsByVendor.h"
#include "dspVendorAPHistory.h"
#include "selectPayments.h"
#include "unappliedAPCreditMemos.h"
#include "vendor.h"

vendorWorkBench::vendorWorkBench(QWidget* parent, const char *name, Qt::WFlags fl)
    : XWidget (parent, name, fl)
{
  setupUi(this);

  QWidget *hideme = 0;

  if (_privileges->check("ViewPurchaseOrders"))
  {
    _po = new dspPOsByVendor(this, "dspPOsByVendor", Qt::Widget);
    _poTab->layout()->addWidget(_po);
    _po->setCloseVisible(false);
    hideme = _po->findChild<QWidget*>("_vendGroup");
    hideme->hide();
    VendorGroup *povend = _po->findChild<VendorGroup*>("_vend");
    if (povend)
    {
      povend->setState(VendorGroup::Selected);
      connect(povend, SIGNAL(newVendId(int)), _po,    SLOT(sFillList()));
      connect(_vend,      SIGNAL(newId(int)), povend, SLOT(setVendId(int)));
    }
    _po->show();
  }
  else
    _tabWidget->setTabEnabled(_tabWidget->indexOf(_poTab), false);

  if (_privileges->check("ViewPurchaseOrders"))
  {
    _receipts = new dspPoItemReceivingsByVendor(this, "dspPoItemReceivingsByVendor", Qt::Widget);
    _receiptsTab->layout()->addWidget(_receipts);
    _receipts->setCloseVisible(false);
    hideme = _receipts->findChild<QWidget*>("_vendorGroup");
    hideme->hide();
    QWidget *rcptvend = _receipts->findChild<QWidget*>("_vendor");
    rcptvend->hide();
    connect(rcptvend,    SIGNAL(newId(int)), _receipts,     SLOT(sFillList()));
    connect(_vend,       SIGNAL(newId(int)), rcptvend,      SLOT(setId(int)));
  }
  else
    _tabWidget->setTabEnabled(_tabWidget->indexOf(_receiptsTab), false);

  if (_privileges->check("MaintainPayments"))
  {
    _payables = new selectPayments(this, "selectPayments", Qt::Widget);
    _payablesTab->layout()->addWidget(_payables);
    hideme = _payables->findChild<QWidget*>("_close");
    hideme->hide();
    VendorGroup *payvend = _payables->findChild<VendorGroup*>("_vendorgroup");
    payvend->setState(VendorGroup::Selected);
    payvend->hide();
    connect(payvend, SIGNAL(newVendId(int)), _payables,     SLOT(sFillList()));
    connect(_vend,       SIGNAL(newId(int)), payvend,       SLOT(setVendId(int)));
  }
  else
    _tabWidget->setTabEnabled(_tabWidget->indexOf(_payablesTab), false);

  if (_privileges->check("MaintainAPMemos") ||
      _privileges->check("ViewAPMemos"))
  {
    _credits = new unappliedAPCreditMemos(this, "unappliedAPCreditMemos", Qt::Widget);
    _cmTab->layout()->addWidget(_credits);
    hideme = _credits->findChild<QWidget*>("_close");
    hideme->hide();
    VendorGroup *cmvend = _credits->findChild<VendorGroup*>("_vendorgroup");
    cmvend->setState(VendorGroup::Selected);
    cmvend->hide();
    connect(cmvend,  SIGNAL(newVendId(int)), _credits,      SLOT(sFillList()));
    connect(_vend,       SIGNAL(newId(int)), cmvend,        SLOT(setVendId(int)));
  }
  else
    _tabWidget->setTabEnabled(_tabWidget->indexOf(_cmTab), false);

  if (_privileges->check("MaintainPayments"))
  {
    _checks = new dspCheckRegister(this, "dspCheckRegister", Qt::Widget);
    _checksTab->layout()->addWidget(_checks);
    _checks->findChild<QWidget*>("_close")->hide();
    _checks->findChild<QGroupBox*>("_recipGroup")->setChecked(true);
    _checks->findChild<QGroupBox*>("_recipGroup")->hide();
    _checks->findChild<DateCluster*>("_dates")->setStartNull(tr("Earliest"), omfgThis->startOfTime(), TRUE);
    _checks->findChild<DateCluster*>("_dates")->setEndNull(tr("Latest"),	  omfgThis->endOfTime(),   TRUE);
    VendorCluster *checkvend = _checks->findChild<VendorCluster*>("_vend");
    connect(checkvend,   SIGNAL(newId(int)), _checks,       SLOT(sFillList()));
    connect(_vend,       SIGNAL(newId(int)), checkvend,     SLOT(setId(int)));
  }
  else
    _tabWidget->setTabEnabled(_tabWidget->indexOf(_checksTab), false);
  
  if (_privileges->check("ViewAPOpenItems"))
  {
    _history = new dspVendorAPHistory(this, "dspVendorAPHistory", Qt::Widget);
    _historyTab->layout()->addWidget(_history);
    _history->setCloseVisible(false);
    _history->findChild<QWidget*>("_vendGroup")->hide();
    _history->findChild<DateCluster*>("_dates")->setStartNull(tr("Earliest"), omfgThis->startOfTime(), TRUE);
    _history->findChild<DateCluster*>("_dates")->setEndNull(tr("Latest"),	  omfgThis->endOfTime(),   TRUE);
    VendorCluster *histvend = _history->findChild<VendorCluster*>("_vend");
    connect(histvend,    SIGNAL(newId(int)), _history,      SLOT(sFillList()));
    connect(_vend,       SIGNAL(newId(int)), histvend,      SLOT(setId(int)));
  }
  else
    _tabWidget->setTabEnabled(_tabWidget->indexOf(_historyTab), false);

  connect(_crmacct,     SIGNAL(clicked()), this,          SLOT(sCRMAccount()));
  connect(_edit,        SIGNAL(clicked()), this,          SLOT(sVendor()));
  connect(_print,       SIGNAL(clicked()), this,          SLOT(sPrint()));
  connect(_vend,       SIGNAL(newId(int)), this,          SLOT(sPopulate()));
  connect(_contact1Button, SIGNAL(clicked()), this, SLOT(sHandleButtons()));
  connect(_contact2Button, SIGNAL(clicked()), this, SLOT(sHandleButtons()));

  _edit->setText(_privileges->check("MaintainVendors") ? tr("Edit") : tr("View"));

  _backlog->setPrecision(omfgThis->moneyVal());
  _lastYearsPurchases->setPrecision(omfgThis->moneyVal());
  _ytdPurchases->setPrecision(omfgThis->moneyVal());
  _openBalance->setPrecision(omfgThis->moneyVal());

  clear();
}

vendorWorkBench::~vendorWorkBench()
{
  // no need to delete child widgets, Qt does it all for us
}

void vendorWorkBench::languageChange()
{
  retranslateUi(this);
}

enum SetResponse vendorWorkBench::set(const ParameterList & pParams)
{
  XWidget::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("vend_id", &valid);
  if (valid)
  {
    _vend->setId(param.toInt());
    _vend->setReadOnly(TRUE);
  }

  return NoError;
}

bool vendorWorkBench::setParams(ParameterList &params)
{
  if(!_vend->isValid())
  {
    QMessageBox::warning(this, tr("No Vendor Selected"),
      tr("You must select a valid Vendor.") );
    _vend->setFocus();
    return false;
  }

  params.append("vend_id", _vend->id());

  return true;
}

void vendorWorkBench::sPrint()
{
  ParameterList params;
  if (! setParams(params))
    return;

  orReport report("VendorInformation", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void vendorWorkBench::clear()
{
  _name->setText("");
  _vendType->setId(-1);
  _terms->setId(-1);
  _shipvia->setId(-1);
  _active->setChecked(true);
  _primaryContact->setId(-1);
  _secondaryContact->setId(-1);
  _crmacctId = -1;
  _backlog->clear();
  _lastYearsPurchases->clear();
  _ytdPurchases->clear();
  _backlog->clear();
}

void vendorWorkBench::sCRMAccount()
{
  ParameterList params;
  if (!_privileges->check("MaintainCRMAccounts"))
    params.append("mode", "view");
  else
    params.append("mode", "edit");

  params.append("crmacct_id", _crmacctId);

  crmaccount *newdlg = new crmaccount(this, "crmaccount");
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void vendorWorkBench::sPopulate()
{
  ParameterList params;
  if (! setParams(params))
  {
    clear();
    return;
  }

  MetaSQLQuery mql("SELECT vend_name,      vend_vendtype_id, vend_terms_id,"
                   "       vend_shipvia,   vend_active,      vend_cntct1_id,"
                   "       vend_cntct2_id, crmacct_id,"
                   "       MIN(pohead_orderdate) AS minpodate, "
                   "       MAX(pohead_orderdate) AS maxpodate, "
                   "       SUM(currToBase(pohead_curr_id,"
                   "           (poitem_qty_ordered - poitem_qty_received) * poitem_unitprice,"
                   "           CURRENT_DATE)) AS backlog "
                   "FROM vendinfo JOIN crmacct ON (crmacct_vend_id=vend_id)"
                   "     LEFT OUTER JOIN pohead ON (pohead_vend_id=vend_id)"
                   "     LEFT OUTER JOIN poitem ON (poitem_pohead_id=pohead_id"
                   "                            AND poitem_status='O')"
                   "WHERE (vend_id=<? value(\"vend_id\") ?>) "
                   "GROUP BY vend_name,      vend_vendtype_id, vend_terms_id,"
                   "         vend_shipvia,   vend_active,      vend_cntct1_id,"
                   "         vend_cntct2_id, crmacct_id;");

  q = mql.toQuery(params);
  if (q.first())
  {
    _name->setText(q.value("vend_name").toString());
    _vendType->setId(q.value("vend_vendtype_id").toInt());
    _terms->setId(q.value("vend_terms_id").toInt());
    _shipvia->setText(q.value("vend_shipvia").toString());
    _active->setChecked(q.value("vend_active").toBool());
    _primaryContact->setId(q.value("vend_cntct1_id").toInt());
    _secondaryContact->setId(q.value("vend_cntct2_id").toInt());
    _crmacctId = q.value("crmacct_id").toInt();
    _firstPurchase->setDate(q.value("minpodate").toDate());
    _lastPurchase->setDate(q.value("maxpodate").toDate());
    _backlog->setDouble(q.value("backlog").toDouble());
  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  MetaSQLQuery purchbydate("SELECT SUM(currToBase(vohead_curr_id,"
                           "             vohead_amount,"
                           "             vohead_gldistdate)) AS purchases "
                           "FROM vohead "
                           "WHERE (vohead_posted"
                           "  AND (vohead_gldistdate "
                           "       BETWEEN (<? literal(\"older\") ?>)"
                           "           AND (<? literal(\"younger\") ?>))"
                           "  AND (vohead_vend_id=<? value(\"vend_id\") ?>));");
  params.append("older",   "DATE_TRUNC('year', CURRENT_DATE) - INTERVAL '1 year'");
  params.append("younger", "DATE_TRUNC('year', CURRENT_DATE) - INTERVAL '1 day'");
  q = purchbydate.toQuery(params);
  if (q.first())
    _lastYearsPurchases->setDouble(q.value("purchases").toDouble());
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  ParameterList ytdparams;
  ytdparams.append("vend_id", _vend->id());
  ytdparams.append("older",   "DATE_TRUNC('year', CURRENT_DATE)");
  ytdparams.append("younger", "CURRENT_DATE - INTERVAL '1 day'");
  q = purchbydate.toQuery(ytdparams);
  if (q.first())
    _ytdPurchases->setDouble(q.value("purchases").toDouble());
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  MetaSQLQuery balm("SELECT COALESCE(SUM((apopen_amount-apopen_paid) / apopen_curr_rate * "
                    "  CASE WHEN (apopen_doctype IN ('D','V')) THEN 1 ELSE -1 END), 0.0) AS balance "
                    "FROM apopen "
                    "WHERE ((apopen_open)"
                    "   AND (apopen_vend_id=<? value(\"vend_id\") ?>));");
  q = balm.toQuery(params);
  if (q.first())
    _openBalance->setDouble(q.value("balance").toDouble());
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void vendorWorkBench::sVendor()
{
  ParameterList params;
  if (! setParams(params))
    return;

  params.append("mode", _privileges->check("MaintainVendors") ?  "edit" : "view");

  vendor *newdlg = new vendor(this, "vendor");
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void vendorWorkBench::sHandleButtons()
{
  if (_contact1Button->isChecked())
    _stackedWidget->setCurrentIndex(_stackedWidget->indexOf(_contact1Page));
  else
    _stackedWidget->setCurrentIndex(_stackedWidget->indexOf(_contact2Page));
}
