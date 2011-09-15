/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "returnAuthorizationWorkbench.h"

#include <QAction>
#include <QMenu>
#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include <openreports.h>
#include <metasql.h>

#include "creditcardprocessor.h"
#include "mqlutil.h"
#include "printCreditMemo.h"
#include "returnAuthorization.h"
#include "returnAuthCheck.h"
#include "storedProcErrorLookup.h"

returnAuthorizationWorkbench::returnAuthorizationWorkbench(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  connect(_ra, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateReviewMenu(QMenu*,QTreeWidgetItem*)));
  connect(_radue, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateDueMenu(QMenu*,QTreeWidgetItem*)));
  connect(_query, SIGNAL(clicked()), this, SLOT(sFillLists()));
  connect(_new, SIGNAL(clicked()), this, SLOT(sNew()));
  connect(_edit, SIGNAL(clicked()), this, SLOT(sEdit()));
  connect(_view, SIGNAL(clicked()), this, SLOT(sView()));
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_editdue, SIGNAL(clicked()), this, SLOT(sEditDue()));
  connect(_viewdue, SIGNAL(clicked()), this, SLOT(sViewDue()));
  connect(_printdue, SIGNAL(clicked()), this, SLOT(sPrintDue()));
  connect(_process, SIGNAL(clicked()), this, SLOT(sProcess()));
  connect(_radue, SIGNAL(valid(bool)), this, SLOT(sHandleButton()));
  connect(omfgThis, SIGNAL(returnAuthorizationsUpdated()), this, SLOT(sFillListReview()));
  connect(omfgThis, SIGNAL(returnAuthorizationsUpdated()), this, SLOT(sFillListDue()));

  _ra->addColumn(tr("Auth. #"),      _orderColumn,    Qt::AlignLeft,   true, "rahead_number"   );
  _ra->addColumn(tr("Customer"),     _bigMoneyColumn, Qt::AlignLeft,   true, "cust_name"  );
  _ra->addColumn(tr("Authorized"),   _dateColumn,     Qt::AlignRight,  true, "rahead_authdate"  );
  _ra->addColumn(tr("Expires"),      _dateColumn,     Qt::AlignRight,  true, "rahead_expiredate"  );
  _ra->addColumn(tr("Disposition"),  _itemColumn,     Qt::AlignRight,  true, "disposition"  );
  _ra->addColumn(tr("Credit By"),    _itemColumn,     Qt::AlignRight,  true, "creditmethod"  );
  _ra->addColumn(tr("Awaiting"),     -1,              Qt::AlignCenter, true, "awaiting" );

  _radue->addColumn(tr("Auth. #"),      _orderColumn,    Qt::AlignLeft, true, "rahead_number"   );
  _radue->addColumn(tr("Customer"),     _bigMoneyColumn, Qt::AlignLeft, true, "cust_name"   );
  _radue->addColumn(tr("Authorized"),   _dateColumn,     Qt::AlignRight,true, "rahead_authdate");
  _radue->addColumn(tr("Amount"),       _moneyColumn,    Qt::AlignRight,true, "amount"  );
  _radue->addColumn(tr("Currency"),     _currencyColumn, Qt::AlignRight,true, "currency"  );
  _radue->addColumn(tr("Amount (%1)").arg(CurrDisplay::baseCurrAbbr()),
					_bigMoneyColumn,    Qt::AlignRight,true, "baseamount"  );
  _radue->addColumn(tr("Credit By"),    -1,              Qt::AlignRight,true, "creditmethod"  );

  if (!_privileges->check("MaintainReturns"))
  {
    _new->setEnabled(false);
    _edit->setEnabled(false);
    _editdue->setEnabled(false);
  }

  if (_metrics->boolean("CCAccept") && _privileges->check("ProcessCreditCards"))
  {
    /*
    if (! CreditCardProcessor::getProcessor())
      QMessageBox::warning(this, tr("Credit Card Processing error"),
			   CreditCardProcessor::errorMsg());
    */
  }
  else
  {
    _creditcard->setChecked(false);
    _creditcard->hide();
  }

  if (!_privileges->check("PostARDocuments"))
  {
    _postmemos->setChecked(false);
    _postmemos->setEnabled(false);
  }
}

returnAuthorizationWorkbench::~returnAuthorizationWorkbench()
{
  // no need to delete child widgets, Qt does it all for us
}

void returnAuthorizationWorkbench::languageChange()
{
  retranslateUi(this);
}

void returnAuthorizationWorkbench::sPrint()
{
  ParameterList params;

  _customerSelector->appendValue(params);

  if(_expired->isChecked())
    params.append("showExpired");

  if(_unauthorized->isChecked())
    params.append("showUnauthorized");

  if (_closed->isChecked())
  {
    params.append("showClosed");
    _dates->appendValue(params);
  }

  if (_receipts->isChecked())
    params.append("showReceipts");
  if (_shipments->isChecked())
    params.append("showShipments");
  if (_payment->isChecked())
    params.append("showPayments");

  orReport report("ReturnAuthorizationWorkbenchReview", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);

}

void returnAuthorizationWorkbench::sNew()
{
  ParameterList params;
  params.append("mode", "new");
  if (_customerSelector->isSelectedCust())
    params.append("cust_id", _customerSelector->custId());

  returnAuthorization *newdlg = new returnAuthorization(this);
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void returnAuthorizationWorkbench::sEdit()
{
  if (!checkSitePrivs(_ra->id()))
    return;
  
  ParameterList params;
  params.append("mode", "edit");
  params.append("rahead_id", _ra->id());

  returnAuthorization *newdlg = new returnAuthorization(this);
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void returnAuthorizationWorkbench::sView()
{
  if (!checkSitePrivs(_ra->id()))
    return;
  
  ParameterList params;
  params.append("mode", "view");
  params.append("rahead_id", _ra->id());

  returnAuthorization *newdlg = new returnAuthorization(this);
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void returnAuthorizationWorkbench::sPrintDue()
{
  ParameterList params;

  setParams(params);

  orReport report("ReturnAuthorizationWorkbenchDueCredit", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void returnAuthorizationWorkbench::sEditDue()
{
  if (!checkSitePrivs(_radue->id()))
    return;
  
  ParameterList params;
  params.append("mode", "edit");
  params.append("rahead_id", _radue->id());

  returnAuthorization *newdlg = new returnAuthorization(this);
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void returnAuthorizationWorkbench::sViewDue()
{
  if (!checkSitePrivs(_radue->id()))
    return;
  
  ParameterList params;
  params.append("mode", "view");
  params.append("rahead_id", _radue->id());

  returnAuthorization *newdlg = new returnAuthorization(this);
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void returnAuthorizationWorkbench::sHandleButton()
{
  _process->setEnabled(((_radue->altId() == 1 && _privileges->check("MaintainCreditMemos")) ||
	   (_radue->altId() == 2 && _privileges->check("MaintainPayments") && 
                              _privileges->check("MaintainCreditMemos") && 
								              _privileges->check("PostARDocuments")) ||
	   (_radue->altId() == 3 && _privileges->check("ProcessCreditCards") &&
				                      _privileges->check("PostARDocuments") &&
	                            _privileges->check("MaintainCreditMemos"))));   

  if (_radue->altId() > 1)
    _postmemos->hide();
  else
    _postmemos->show();
}

void returnAuthorizationWorkbench::sProcess()
{
  if (!checkSitePrivs(_radue->id()))
    return;
  
  bool _post = ((_radue->altId() > 1) ||
		(_radue->altId() == 1 && _postmemos->isChecked())) ;

  q.prepare("SELECT createRaCreditMemo(:rahead_id,:post) AS result;");
  q.bindValue(":rahead_id",_radue->id());
  q.bindValue(":post",QVariant(_post));
  q.exec();
  if (q.first())
  {
    int cmheadid = q.value("result").toInt();
    if (cmheadid < 0)
    {
      systemError(this, storedProcErrorLookup("createRaCreditMemo", cmheadid), __FILE__, __LINE__);
      return;
    }
    q.prepare( "SELECT cmhead_number "
               "FROM cmhead "
               "WHERE (cmhead_id=:cmhead_id);" );
    q.bindValue(":cmhead_id", cmheadid);
    q.exec();
    if (q.first())
    {
      QMessageBox::information( this, tr("New Credit Memo Created"),
                                tr("<p>A new CreditMemo has been created and "
				                   "assigned #%1")
                                   .arg(q.value("cmhead_number").toString()));
	  if (_printmemo->isChecked())
	  {
		ParameterList params;
		params.append("cmhead_id", cmheadid);
		if (_post)
		  params.append("posted");

		printCreditMemo newdlg(this, "", TRUE);
		newdlg.set(params);
		newdlg.exec();
	  }
      if (_radue->altId() == 2)
      {
        ParameterList params;
        params.append("cmhead_id", cmheadid);

        returnAuthCheck newdlg(this, "", TRUE);
        newdlg.set(params);
        if (newdlg.exec() != XDialog::Rejected)
          sFillListDue();
      }
      else if (_radue->altId() == 3)
      {
	ParameterList ccp;
	ccp.append("cmhead_id", cmheadid);
  MetaSQLQuery ccm = mqlLoad("creditMemoCreditCards", "detail");
	XSqlQuery ccq = ccm.toQuery(ccp);
	if (ccq.first())
	{
	  int ccpayid = ccq.value("ccpay_id").toInt();
	  CreditCardProcessor *cardproc = CreditCardProcessor::getProcessor();
	  if (! cardproc)
	    QMessageBox::critical(this, tr("Credit Card Processing Error"),
				  CreditCardProcessor::errorMsg());
	  else if (! cardproc->errorMsg().isEmpty())
	    QMessageBox::critical(this, tr("Credit Card Processing Warning"),
				 cardproc->errorMsg());
	  else
	  {
	    QString docnum = q.value("cmhead_number").toString();
	    QString refnum = ccq.value("cohead_number").toString();
	    int refid = -1;
	    int returnValue = cardproc->credit(ccq.value("ccard_id").toInt(),
					 -2,
					 ccq.value("total").toDouble(),
					 ccq.value("tax_in_cmcurr").toDouble(),
					 ccq.value("cmhead_tax_id").isNull(),
					 ccq.value("cmhead_freight").toDouble(),
					 0,
					 ccq.value("cmhead_curr_id").toInt(),
					 docnum, refnum, ccpayid,
					 QString(), refid);
	    if (returnValue < 0)
	      QMessageBox::critical(this, tr("Credit Card Processing Error"),
				    cardproc->errorMsg());
	    else if (returnValue > 0)
	      QMessageBox::warning(this, tr("Credit Card Processing Warning"),
				   cardproc->errorMsg());
	    else if (! cardproc->errorMsg().isEmpty())
	      QMessageBox::information(this, tr("Credit Card Processing Note"),
				   cardproc->errorMsg());
	  }
	  // requery regardless 'cause the new credit memo means nothing's "due"
	  sFillListDue();
	}
	else if (ccq.lastError().type() != QSqlError::NoError)
	{
	  systemError(this, ccq.lastError().databaseText(), __FILE__, __LINE__);
	  return;
	}
	else
	{
	  QMessageBox::critical(this, tr("Credit Card Processing Error"),
				tr("Could not find a Credit Card to use for "
				   "this Credit transaction."));
	  sFillListDue();
	  return;
	}
      }
      else
	sFillListDue();
    }
    else if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void returnAuthorizationWorkbench::setParams(ParameterList &params)
{
  _customerSelector->appendValue(params);

  params.append("credit",     tr("Credit"));
  params.append("return",     tr("Return"));
  params.append("none",       tr("None"));
  params.append("creditmemo", tr("Memo"));
  params.append("check",      tr("Check"));
  params.append("creditcard", tr("Card"));

  if (_creditmemo->isChecked())
    params.append("doM");
  if (_check->isChecked())
    params.append("doK");
  if (_creditcard->isChecked())
    params.append("doC");

  _dates->appendValue(params);
}

void returnAuthorizationWorkbench::sFillLists()
{
  if (_customerSelector->isSelectedCust() && _customerSelector->custId() == -1)
  {
    QMessageBox::information( this, tr("Customer not selected"),
			      tr("<p>Please select a customer.") );
    _customerSelector->setFocus();
    return;
  }
  
  sFillListReview();
  sFillListDue();
}

void returnAuthorizationWorkbench::sFillListReview()
{ 
  _ra->clear();

  //Fill Review List
  if (_closed->isChecked() && !_dates->allValid())
  {
    QMessageBox::information( this, tr("Invalid Dates"),
			      tr("<p>Invalid dates specified. Please specify a "
				 "valid date range.") );
    _dates->setFocus();
    return;
  }
  else if ((_receipts->isChecked()) || (_shipments->isChecked()) || 
	        (_payment->isChecked()) || (_closed->isChecked()) ||
			(_unauthorized->isChecked()))
  {
	bool bw;
	bw = false;
	QString sql (" SELECT *, :never  AS rahead_expiredate_xtnullrole FROM ( "
			  "SELECT rahead_id, rahead_number, COALESCE(cust_name,:undefined) AS cust_name, "
			  "rahead_authdate, rahead_expiredate, "
	  		  "CASE "
			  "  WHEN raitem_disposition = 'C' THEN "
			  "    :credit "
			  "  WHEN raitem_disposition = 'R' THEN "
			  "    :return "
			  "  WHEN raitem_disposition = 'P' THEN "
			  "    :replace "
			  "  WHEN raitem_disposition = 'V' THEN "
			  "    :service "
			  "  WHEN raitem_disposition = 'S' THEN "
			  "    :ship "
			  "  END AS disposition, "
			  "CASE "
			  "  WHEN rahead_creditmethod = 'N' THEN "
			  "    :none "
			  "  WHEN rahead_creditmethod = 'M' THEN "
			  "    :creditmemo "
			  "  WHEN rahead_creditmethod = 'K' THEN "
			  "    :check "
			  "  WHEN rahead_creditmethod = 'C' THEN "
			  "    :creditcard "
			  "END AS creditmethod, "
			  "CASE "
			  "  WHEN raitem_status = 'C' THEN "
			  "    :closed "
			  "  WHEN raitem_disposition = 'C' THEN "
			  "    :payment "
			  "  WHEN (raitem_disposition = 'R' "
			  "    AND SUM(raitem_qtyauthorized-raitem_qtycredited) > 0 "
			  "    AND SUM(raitem_qtyauthorized-raitem_qtyreceived) > 0) THEN "
			  "    :receipt || ',' || :payment "
			  "  WHEN raitem_disposition = 'R' "
			  "    AND SUM(raitem_qtyreceived-raitem_qtycredited) > 0 THEN "
			  "    :payment "
			  "  WHEN raitem_disposition = 'R' "
			  "    AND SUM(raitem_qtyauthorized-raitem_qtyreceived) > 0 THEN "
			  "    :receipt "
			  "  WHEN raitem_disposition = 'P' "
			  "    AND SUM(raitem_qtyauthorized-COALESCE(coitem_qtyshipped,0)) > 0 "
			  "    AND SUM(raitem_qtyauthorized-raitem_qtyreceived) > 0 "
                          "    AND SUM(raitem_qtyauthorized * raitem_unitprice - raitem_amtcredited) > 0 "
			  "    AND SUM(raitem_qtyreceived-raitem_qtycredited) > 0 THEN "
			  "    :receipt || ','  || :payment || ',' || :ship "
			  "  WHEN raitem_disposition = 'P' "
			  "    AND SUM(raitem_qtyauthorized-raitem_qtyreceived) > 0 "
                          "    AND SUM(raitem_qtyauthorized * raitem_unitprice) > 0 "
			  "    AND SUM(raitem_qtyreceived-raitem_qtycredited) > 0 THEN "
			  "    :receipt || ','  || :payment "
			  "  WHEN raitem_disposition = 'P' "
			  "    AND SUM(raitem_qtyauthorized-COALESCE(coitem_qtyshipped,0)) > 0 "
                          "    AND SUM(raitem_qtyauthorized * raitem_unitprice - raitem_amtcredited) > 0 "
			  "    AND SUM(raitem_qtyreceived-raitem_qtycredited) > 0 THEN "
			  "    :payment || ',' || :ship "
			  "  WHEN raitem_disposition IN ('P','V') "
			  "    AND SUM(raitem_qtyauthorized-COALESCE(coitem_qtyshipped,0)) > 0 "
			  "    AND SUM(raitem_qtyauthorized-raitem_qtyreceived) > 0 THEN "
			  "    :receipt || ',' || :ship "
			  "  WHEN raitem_disposition IN ('P','V') "
			  "    AND SUM(raitem_qtyauthorized-COALESCE(coitem_qtyshipped,0)) > 0 THEN "
			  "    :ship "
			  "  WHEN raitem_disposition IN ('P','V') "
			  "    AND SUM(raitem_qtyauthorized-raitem_qtyreceived) > 0 THEN "
			  "    :receipt "
			  "  WHEN raitem_disposition = 'S' THEN "
			  "    :ship "
			  "  ELSE '' "
			  "END AS awaiting, "
        "CASE WHEN (rahead_expiredate < current_date) THEN "
        "  'error' "
        "END AS rahead_expiredate_qtforegroundrole "
			  "FROM rahead "
			  "  LEFT OUTER JOIN custinfo ON (rahead_cust_id=cust_id) "
			  "  LEFT OUTER JOIN custtype ON (cust_custtype_id=custtype_id) "
			  "  LEFT OUTER JOIN custgrpitem ON (custgrpitem_cust_id=cust_id), "
			  " raitem "
			  "  LEFT OUTER JOIN coitem ON (raitem_new_coitem_id=coitem_id) "
			  "WHERE ( (rahead_id=raitem_rahead_id)"
                          "  AND   ((SELECT COUNT(*)"
                          "          FROM raitem JOIN itemsite ON (itemsite_id=raitem_itemsite_id)"
                          "                      JOIN site() ON (warehous_id=itemsite_warehous_id)"
                          "          WHERE (raitem_rahead_id=rahead_id)) > 0)" );
    
    if ((_customerSelector->isSelectedCust()))
	  sql += " AND (cust_id=:cust_id) ";
    else if (_customerSelector->isSelectedType())
	  sql += " AND (custtype_id=:custtype_id) ";
    else if (_customerSelector->isTypePattern())
	  sql += " AND (custtype_code ~ :custtype_pattern) ";
    else if (_customerSelector->isSelectedGroup())
	  sql += " AND (custgrpitem_custgrp_id=:custgrp_id) ";

	if (!_expired->isChecked())
	  sql +=  " AND (COALESCE(rahead_expiredate,current_date) >= current_date)";
	if (!_unauthorized->isChecked())
	  sql +=  " AND (raitem_qtyauthorized > 0)";
	if (_closed->isChecked())
	  sql +=  " AND (raitem_status='O' OR rahead_authdate BETWEEN :startDate AND :endDate)";
	else
      sql +=  " AND (raitem_status = 'O') ";

  	sql +=    " ) GROUP BY rahead_id,rahead_number,cust_name,rahead_expiredate, "
			  " rahead_authdate,raitem_status,raitem_disposition,rahead_creditmethod, "
			  " rahead_curr_id "
			  " ORDER BY rahead_authdate,rahead_number "
			  ") as data ";

	if (_receipts->isChecked())
	{
	  sql +=  " WHERE ((disposition IN (:return,:replace,:service)) "
			  " AND (awaiting ~ :receipt)) "; 
	  bw = true;
	}
	if (_shipments->isChecked())
	{
	  if (bw)
		sql += "OR (";
	  else
		sql += "WHERE (";
	  sql +=  " (disposition IN (:replace,:service,:ship))"
			  " AND (awaiting ~ :ship)) ";  
	  bw = true;
	}
	if (_payment->isChecked())
	{
	  if (bw)
		sql += "OR (";
	  else
		sql += "WHERE (";
	  sql +=  " (disposition IN (:credit,:return))"
			  " AND (awaiting ~ :payment)) "; 
	  bw = true;
	}
	if (_closed->isChecked())
	{
	  if (bw)
		sql += "OR (";
	  else
		sql += "WHERE (";
	  sql +=  "(awaiting = :closed)) "; 
	}
    
	XSqlQuery ra;
	ra.prepare(sql);
	ra.bindValue(":cust_id", _customerSelector->custId());
	ra.bindValue(":custtype_id", _customerSelector->custTypeId());
	ra.bindValue(":custtype_pattern", _customerSelector->typePattern());
        ra.bindValue(":custgrp_id", _customerSelector->custGroupId());
	ra.bindValue(":undefined",tr("Undefined"));
	ra.bindValue(":credit",tr("Credit"));
	ra.bindValue(":return",tr("Return"));
	ra.bindValue(":replace",tr("Replace"));
	ra.bindValue(":service",tr("Service"));
	ra.bindValue(":none",tr("None"));
	ra.bindValue(":creditmemo",tr("Memo"));
	ra.bindValue(":check",tr("Check"));
	ra.bindValue(":creditcard",tr("Card"));
	ra.bindValue(":payment",tr("Payment"));
	ra.bindValue(":receipt",tr("Receipt"));
	ra.bindValue(":ship",tr("Shipment"));
	ra.bindValue(":never",tr("Never"));
	ra.bindValue(":closed",tr("Closed"));
        _dates->bindValue(ra);
	ra.exec();
        _ra->populate(ra);
        if (ra.lastError().type() != QSqlError::NoError)
        {
          systemError(this, ra.lastError().databaseText(), __FILE__, __LINE__);
          return;
        }
  }
}

void returnAuthorizationWorkbench::sFillListDue()
{ 
  _radue->clear();

  //Fill Due Credit List
  if ((_creditmemo->isChecked()) || (_check->isChecked()) || (_creditcard->isChecked()))
  {
    bool bc;
    bc = false;

    MetaSQLQuery mql = mqlLoad("returnauthorizationworkbench", "duecredit");
    ParameterList params;
    setParams(params);

    XSqlQuery radue = mql.toQuery(params);
    _radue->populate(radue,TRUE);
    if (radue.lastError().type() != QSqlError::NoError)
    {
      systemError(this, radue.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }
}

bool returnAuthorizationWorkbench::checkSitePrivs(int orderid)
{
  if (_preferences->boolean("selectedSites"))
  {
    XSqlQuery check;
    check.prepare("SELECT checkRASitePrivs(:raheadid) AS result;");
    check.bindValue(":raheadid", orderid);
    check.exec();
    if (check.first())
    {
      if (!check.value("result").toBool())
      {
        QMessageBox::critical(this, tr("Access Denied"),
                              tr("You may not view or edit this Return Authorization as it references "
                                 "a Site for which you have not been granted privileges.")) ;
        return false;
      }
    }
  }
  return true;
}

void returnAuthorizationWorkbench::sPopulateReviewMenu(QMenu *pMenu, QTreeWidgetItem * /*pSelected*/)
{
  QAction *menuItem;

  menuItem = pMenu->addAction(tr("Edit..."), this, SLOT(sEdit()));
  menuItem->setEnabled(_privileges->check("MaintainReturns"));

  menuItem = pMenu->addAction(tr("View..."), this, SLOT(sView()));
}

void returnAuthorizationWorkbench::sPopulateDueMenu(QMenu *pMenu, QTreeWidgetItem * /*pSelected*/)
{
  QAction *menuItem;

  menuItem = pMenu->addAction(tr("Edit..."), this, SLOT(sEditDue()));
  menuItem->setEnabled(_privileges->check("MaintainReturns"));

  menuItem = pMenu->addAction(tr("View..."), this, SLOT(sViewDue()));
  
  menuItem = pMenu->addAction(tr("Process..."), this, SLOT(sProcess()));
  menuItem->setEnabled((_radue->altId() == 1 && _privileges->check("MaintainCreditMemos")) ||
                       (_radue->altId() == 2 && _privileges->check("MaintainPayments") &&
                        _privileges->check("MaintainCreditMemos") && 
                        _privileges->check("PostARDocuments")) ||
                       (_radue->altId() == 3 &&
                        _privileges->check("ProcessCreditCards") &&
                        _privileges->check("PostARDocuments") &&
                        _privileges->check("MaintainCreditMemos")));
}

