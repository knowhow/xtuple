/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */


#include <QFileInfo>
#include <QSqlError>

#include <currcluster.h>

#include "guiclient.h"
#include "yourpayprocessor.h"

#define DEBUG false

/** \ingroup creditcards
    \class   YourPayProcessor
 */

// convenience macro to add <ChildName>Content</ChildName> to the Parent node
#define CREATECHILDTEXTNODE(Parent, ChildName, Content) \
	{ \
	  QDomElement Child = Parent.ownerDocument().createElement(ChildName); \
	  Child.appendChild(Parent.ownerDocument().createTextNode(Content)); \
	  Parent.appendChild(Child); \
	}

YourPayProcessor::YourPayProcessor() : CreditCardProcessor()
{
  XSqlQuery currq;
  currq.exec("SELECT * FROM curr_symbol WHERE (curr_abbr='USD');");
  if (currq.first())
    _ypcurrid = currq.value("curr_id").toInt();
  else if (currq.lastError().type() != QSqlError::NoError)
    _errorMsg = currq.lastError().databaseText();
  else if (currq.exec("SELECT * "
		      "FROM curr_symbol "
		      "WHERE ((curr_abbr ~* 'US')"
		      "  AND  (curr_symbol ~ '\\$')"
		      "  AND  (curr_name ~* 'dollar'));") &&
	   currq.first())
    _ypcurrid = currq.value("curr_id").toInt();
  else if (currq.lastError().type() != QSqlError::NoError)
    _errorMsg = currq.lastError().databaseText();

  if (_ypcurrid <= 0)
  {
    _errorMsg = tr("Could not find US $ in the curr_symbol table; "
		    "defaulting to base currency.") + "\n\n" + _errorMsg;
    _ypcurrid = CurrDisplay::baseId();
  }

  _defaultLivePort   = 1129;
  _defaultLiveServer = "https://secure.linkpt.net";
  _defaultTestPort   = 1129;
  _defaultTestServer = "https://staging.linkpt.net";

  _msgHash.insert(-100, tr("No Approval Code\n%1\n%2\n%3"));
  _msgHash.insert(-102, tr("If LinkShield is enabled then the cutoff score "
			   "should be set between 1 and 100 but it is currently"
			   " set to %1. Higher Numbers indicate higher risk."));
  _msgHash.insert(-103, tr("The YourPay Store Number is not set."));
  _msgHash.insert(-106, tr("Transaction failed LinkShield check. Either the "
			   "score was greater than the maximum configured "
			   "allowed score or the score was empty (this may "
			   "happen if the application is configured to "
			   "check the LinkShield score but you have not "
			   "subscribed to this service)."));
  _msgHash.insert( 100, tr("Received unexpected approval code %1. This "
                           "transaction has been treated as approved by "
                           "YourPay."));
}

int YourPayProcessor::buildCommon(const int pccardid, const int pcvv, const double pamount, QDomDocument &prequest, QString pordertype)
{
  QDomElement order = prequest.createElement("order");
  prequest.appendChild(order);

  // TODO: if check and not credit card transaction, do a different select
  XSqlQuery ypq;
  ypq.prepare(
    "SELECT ccard_active,"
    "  formatbytea(decrypt(setbytea(ccard_number),   setbytea(:key),'bf')) AS ccard_number,"
    "  formatccnumber(decrypt(setbytea(ccard_number),setbytea(:key),'bf')) AS ccard_number_x,"
    "  formatbytea(decrypt(setbytea(ccard_name),     setbytea(:key),'bf')) AS ccard_name,"
    "  formatbytea(decrypt(setbytea(ccard_address1), setbytea(:key),'bf')) AS ccard_address1,"
    "  formatbytea(decrypt(setbytea(ccard_address2), setbytea(:key),'bf')) AS ccard_address2,"
    "  formatbytea(decrypt(setbytea(ccard_city),     setbytea(:key),'bf')) AS ccard_city,"
    "  formatbytea(decrypt(setbytea(ccard_state),    setbytea(:key),'bf')) AS ccard_state,"
    "  formatbytea(decrypt(setbytea(ccard_zip),      setbytea(:key),'bf')) AS ccard_zip,"
    "  formatbytea(decrypt(setbytea(ccard_country),  setbytea(:key),'bf')) AS ccard_country,"
    "  formatbytea(decrypt(setbytea(ccard_month_expired),setbytea(:key),'bf')) AS ccard_month_expired,"
    "  formatbytea(decrypt(setbytea(ccard_year_expired),setbytea(:key), 'bf')) AS ccard_year_expired"
    "  FROM ccard "
    "WHERE (ccard_id=:ccardid);");
  ypq.bindValue(":ccardid", pccardid);
  ypq.bindValue(":key",     omfgThis->_key);
  ypq.exec();

  if (ypq.first())
  {
    if (!ypq.value("ccard_active").toBool())
    {
      _errorMsg = errorMsg(-10);
      return -10;
    }
  }
  else if (ypq.lastError().type() != QSqlError::NoError)
  {
    _errorMsg = ypq.lastError().databaseText();
    return -1;
  }
  else
  {
    _errorMsg = errorMsg(-17).arg(pccardid);
    return -17;
  }

  QDomElement elem;

  elem = prequest.createElement("merchantinfo");
  order.appendChild(elem);
  CREATECHILDTEXTNODE(elem, "configfile",
		      _metricsenc->value("CCYPStoreNum").toLatin1());

  elem = prequest.createElement("orderoptions");
  order.appendChild(elem);
  CREATECHILDTEXTNODE(elem, "ordertype", pordertype);
  QString resultText;
  if (isLive())
    resultText = "LIVE";
  else if (isTest())
  {
    if (_metrics->value("CCTestResult") == "F")
      resultText = (qrand() % 2) ? "DECLINE" : "DUPLICATE";
    else if (_metrics->value("CCTestResult") == "S")
      switch (qrand() % 10)
      {
	case 0: resultText = "DECLINE";
		break;
	case 1: resultText = "DUPLICATE";
		break;
	default:
		resultText = "GOOD";
		break;
      }
    else // if (_metrics->value("CCTestResult") == "P")
      resultText = "GOOD";
  }
  else
  {
    _errorMsg = errorMsg(-13).arg(pccardid);
    return -13;
  }
  CREATECHILDTEXTNODE(elem, "result", resultText);

  elem = prequest.createElement("payment");
  order.appendChild(elem);
  CREATECHILDTEXTNODE(elem, "chargetotal", QString::number(pamount));

  // TODO: if check and not credit card transaction do something else
  elem = prequest.createElement("creditcard");
  order.appendChild(elem);
  CREATECHILDTEXTNODE(elem, "cardnumber", ypq.value("ccard_number").toString());

  QString work_month;
  work_month.setNum(ypq.value("ccard_month_expired").toDouble());
  if (work_month.length() == 1)
    work_month = "0" + work_month;
  CREATECHILDTEXTNODE(elem,  "cardexpmonth", work_month);
  CREATECHILDTEXTNODE(elem,  "cardexpyear",
		      ypq.value("ccard_year_expired").toString().right(2));
  if (pcvv > 0)
  {
    CREATECHILDTEXTNODE(elem, "cvmvalue", QString::number(pcvv));
    CREATECHILDTEXTNODE(elem, "cvmindicator", "provided");
  }

  elem = prequest.createElement("billing");
  order.appendChild(elem);

  CREATECHILDTEXTNODE(elem, "name", ypq.value("ccard_name").toString());
  // TODO: company
  CREATECHILDTEXTNODE(elem, "address1", ypq.value("ccard_address1").toString());
  CREATECHILDTEXTNODE(elem, "address2", ypq.value("ccard_address2").toString());
  CREATECHILDTEXTNODE(elem, "city",     ypq.value("ccard_city").toString());
  CREATECHILDTEXTNODE(elem, "state",    ypq.value("ccard_state").toString());
  CREATECHILDTEXTNODE(elem, "zip",      ypq.value("ccard_zip").toString());
  // TODO: country // should be 2-char country code abbr
  // TODO: phone, fax, email
  CREATECHILDTEXTNODE(elem, "addrnum",
		    ypq.value("ccard_address1").toString().section(" ", 0, 0));

  if (DEBUG)
    qDebug("YP:buildCommon built %s", prequest.toString().toAscii().data());
  return 0;
}

int YourPayProcessor::doAuthorize(const int pccardid, const int pcvv, double &pamount, const double ptax, const bool ptaxexempt, const double pfreight, const double pduty, const int pcurrid, QString& pneworder, QString& preforder, int &pccpayid, ParameterList &pparams)
{
  if (DEBUG)
    qDebug("YP:doAuthorize(%d, %d, %f, %f, %d, %f, %f, %d, %s, %s, %d)",
	   pccardid, pcvv, pamount, ptax, ptaxexempt, pfreight, pduty, pcurrid,
	   pneworder.toAscii().data(), preforder.toAscii().data(), pccpayid);

  int returnValue = 0;
  double amount = currToCurr(pcurrid, _ypcurrid, pamount, &returnValue);
  if (returnValue < 0)
    return returnValue;

  QDomDocument request;

  returnValue = buildCommon(pccardid, pcvv, amount, request, "PREAUTH");
  if (returnValue !=  0)
    return returnValue;

  QDomElement elem;

  elem = request.createElement("transactiondetails");
  request.documentElement().appendChild(elem);

  // in case we're reusing an order number
  QString oidstr = pneworder;
  XSqlQuery ypq;
  ypq.prepare("SELECT MAX(COALESCE(ccpay_order_number_seq, -1)) + 1"
	      "       AS next_seq "
	      "  FROM ccpay "
	      " WHERE (ccpay_order_number=:ccpay_order_number);");
  ypq.bindValue(":ccpay_order_number", pneworder);
  ypq.exec();
  if (ypq.first() && ! ypq.value("next_seq").isNull())
    oidstr = oidstr + "-" + ypq.value("next_seq").toString();
  else if (ypq.lastError().type() != QSqlError::NoError)
  {
    _errorMsg = errorMsg(-24);
    return -24;
  }
  CREATECHILDTEXTNODE(elem, "oid",          oidstr);
  CREATECHILDTEXTNODE(elem, "terminaltype", "UNSPECIFIED");
  CREATECHILDTEXTNODE(elem, "taxexempt",    ptaxexempt ? "Y" : "N");

  if (! preforder.isEmpty())
    CREATECHILDTEXTNODE(elem, "ponumber", preforder);

  /* TODO: salesOrder.cpp set the shipping elem. YP only seems to use it for
	   CALCSHIPPING transactions; we don't do these yet so don't bother
  elem = request.createElement("shipping");
  request.documentElement().appendChild(elem);
  CREATECHILDTEXTNODE(elem, "address1", _shipto_address1);
  CREATECHILDTEXTNODE(elem, "address2", _shipto_address2);
  CREATECHILDTEXTNODE(elem, "city",     _shipto_city);
  CREATECHILDTEXTNODE(elem, "name",     _shipto_name);
  CREATECHILDTEXTNODE(elem, "state",    _shipto_state);
  CREATECHILDTEXTNODE(elem, "zip",      _shipto_zip);
  */

  CREATECHILDTEXTNODE(request.elementsByTagName("payment").at(0),
		      "tax", QString::number(ptax));

  QString response;
  returnValue = sendViaHTTP(request.toString(), response);
  if (returnValue < 0)
    return returnValue;

  returnValue = handleResponse(response, pccardid, "A", pamount, pcurrid,
			       pneworder, preforder, pccpayid, pparams);

  return returnValue;
}

int  YourPayProcessor::doCharge(const int pccardid, const int pcvv, const double pamount, const double ptax, const bool ptaxexempt, const double pfreight, const double pduty, const int pcurrid, QString& pneworder, QString& preforder, int &pccpayid, ParameterList &pparams)
{
  if (DEBUG)
    qDebug("YP:doCharge(%d, %d, %f, %f, %d, %f, %f, %d, %s, %s, %d)",
	   pccardid, pcvv, pamount, ptax, ptaxexempt, pfreight, pduty, pcurrid,
	   pneworder.toAscii().data(), preforder.toAscii().data(), pccpayid);

  int returnValue = 0;
  double amount = currToCurr(pcurrid, _ypcurrid, pamount, &returnValue);
  if (returnValue < 0)
    return returnValue;

  QDomDocument request;

  returnValue = buildCommon(pccardid, pcvv, amount, request, "SALE");
  if (returnValue !=  0)
    return returnValue;

  QDomElement elem;

  elem = request.createElement("transactiondetails");
  request.documentElement().appendChild(elem);

  // in case we're reusing an order number
  QString oidstr = pneworder;
  XSqlQuery ypq;
  ypq.prepare("SELECT MAX(COALESCE(ccpay_order_number_seq, -1)) + 1"
	      "       AS next_seq "
	      "  FROM ccpay "
	      " WHERE (ccpay_order_number=:ccpay_order_number);");
  ypq.bindValue(":ccpay_order_number", pneworder);
  ypq.exec();
  if (ypq.first() && ! ypq.value("next_seq").isNull())
    oidstr = oidstr + "-" + ypq.value("next_seq").toString();
  else if (ypq.lastError().type() != QSqlError::NoError)
  {
    _errorMsg = errorMsg(-44);
    return -44;
  }
  CREATECHILDTEXTNODE(elem, "oid",          oidstr);
  CREATECHILDTEXTNODE(elem, "terminaltype", "UNSPECIFIED");
  CREATECHILDTEXTNODE(elem, "taxexempt",    ptaxexempt ? "Y" : "N");

  if (! preforder.isEmpty())
    CREATECHILDTEXTNODE(elem, "ponumber", preforder);

  /* TODO: salesOrder.cpp had the following, but it doesn't appear to be useful
	   except for having YP calculate shipping charges. since we don't
	   use YP to calculate shipping and this is a SALE and not a
	   CALCSHIPPING transaction, we won't bother for now.
  elem = request.createElement("shipping");
  request.documentElement().appendChild(elem);
  CREATECHILDTEXTNODE(elem, "address1", _shipto_address1);
  CREATECHILDTEXTNODE(elem, "address2", _shipto_address2);
  CREATECHILDTEXTNODE(elem, "city",     _shipto_city);
  CREATECHILDTEXTNODE(elem, "name",     _shipto_name);
  CREATECHILDTEXTNODE(elem, "state",    _shipto_state);
  CREATECHILDTEXTNODE(elem, "zip",      _shipto_zip);
  */

  CREATECHILDTEXTNODE(request.elementsByTagName("payment").at(0),
		      "tax", QString::number(ptax));

  QString response;
  returnValue = sendViaHTTP(request.toString(), response);
  if (returnValue < 0)
    return returnValue;

  returnValue = handleResponse(response, pccardid, "C", pamount, pcurrid,
			       pneworder, preforder, pccpayid, pparams);

  return returnValue;
}

int YourPayProcessor::doChargePreauthorized(const int pccardid, const int pcvv, const double pamount, const int pcurrid, QString &pneworder, QString &preforder, int &pccpayid, ParameterList &pparams)
{
  if (DEBUG)
    qDebug("YP:doChargePreauthorized(%d, %d, %f, %d, %s, %s, %d)",
	   pccardid, pcvv, pamount, pcurrid,
	   pneworder.toAscii().data(), preforder.toAscii().data(), pccpayid);

  int returnValue = 0;
  double amount = currToCurr(pcurrid, _ypcurrid, pamount, &returnValue);
  if (returnValue < 0)
    return returnValue;

  QDomDocument request;

  returnValue = buildCommon(pccardid, pcvv, amount, request, "POSTAUTH");
  if (returnValue !=  0)
    return returnValue;

  QDomElement elem = request.createElement("transactiondetails");
  request.documentElement().appendChild(elem);
  CREATECHILDTEXTNODE(elem, "oid",          preforder);
  CREATECHILDTEXTNODE(elem, "terminaltype", "UNSPECIFIED");

  QString response;
  returnValue = sendViaHTTP(request.toString(), response);
  if (returnValue < 0)
    return returnValue;

  returnValue = handleResponse(response, pccardid, "CP", pamount, pcurrid,
			       pneworder, preforder, pccpayid, pparams);

  return returnValue;
}

int YourPayProcessor::doCredit(const int pccardid, const int pcvv, const double pamount, const double ptax, const bool ptaxexempt, const double pfreight, const double pduty, const int pcurrid, QString &pneworder, QString &preforder, int &pccpayid, ParameterList &pparams)
{
  if (DEBUG)
    qDebug("YP:doCredit(%d, %d, %f, %f, %d, %f, %f, %d, %s, %s, %d)",
	   pccardid, pcvv, pamount, ptax, ptaxexempt, pfreight, pduty, pcurrid,
	   pneworder.toAscii().data(), preforder.toAscii().data(), pccpayid);

  int returnValue = 0;
  double amount = currToCurr(pcurrid, _ypcurrid, pamount, &returnValue);
  if (returnValue < 0)
    return returnValue;

  QDomDocument request;

  returnValue = buildCommon(pccardid, pcvv, amount, request, "CREDIT");
  if (returnValue !=  0)
    return returnValue;

  QDomElement elem = request.createElement("transactiondetails");
  request.documentElement().appendChild(elem);
  CREATECHILDTEXTNODE(elem, "oid",              preforder);
  CREATECHILDTEXTNODE(elem, "reference_number", pneworder);
  CREATECHILDTEXTNODE(elem, "terminaltype",     "UNSPECIFIED");
  CREATECHILDTEXTNODE(elem, "taxexempt",        ptaxexempt ? "Y" : "N");

  CREATECHILDTEXTNODE(request.elementsByTagName("payment").at(0),
		      "tax", QString::number(ptax));

  QString response;
  returnValue = sendViaHTTP(request.toString(), response);
  if (returnValue < 0)
    return returnValue;

  returnValue = handleResponse(response, pccardid, "R", pamount, pcurrid,
			       pneworder, preforder, pccpayid, pparams);

  return returnValue;
}

int YourPayProcessor::doVoidPrevious(const int pccardid, const int pcvv, const double pamount, const int pcurrid, QString &pneworder, QString &preforder, QString &papproval, int &pccpayid, ParameterList &pparams)
{
  if (DEBUG)
    qDebug("YP:doVoidPrevious(%d, %d, %f, %d, %s, %s, %s, %d)",
	   pccardid, pcvv, pamount, pcurrid,
	   pneworder.toAscii().data(), preforder.toAscii().data(),
	   papproval.toAscii().data(), pccpayid);

  int returnValue = 0;
  double amount = currToCurr(pcurrid, _ypcurrid, pamount, &returnValue);
  if (returnValue < 0)
    return returnValue;

  // distinguish void errors from errors from problems that cause us to void
  QString tmpErrorMsg = _errorMsg;

  QDomDocument request;

  returnValue = buildCommon(pccardid, pcvv, amount, request, "VOID");
  if (returnValue != 0)
    return returnValue;

  QDomElement elem = request.createElement("transactiondetails");
  request.documentElement().appendChild(elem);
  CREATECHILDTEXTNODE(elem, "oid",              preforder);
  CREATECHILDTEXTNODE(elem, "terminaltype",     "UNSPECIFIED");

  QString response;
  returnValue = sendViaHTTP(request.toString(), response);
  _errorMsg = tmpErrorMsg;
  if (returnValue < 0)
    return returnValue;

  returnValue = handleResponse(response, pccardid, "V", pamount, pcurrid,
			       pneworder, preforder, pccpayid, pparams);
  _errorMsg = tmpErrorMsg;
  return returnValue;
}

int YourPayProcessor::handleResponse(const QString &presponse, const int pccardid, const QString &ptype, const double pamount, const int pcurrid, QString &pneworder, QString &preforder, int &pccpayid, ParameterList &pparams)
{
  if (DEBUG)
    qDebug("YP::handleResponse(%s, %d, %s, %f, %d, %s, %d, pparams)",
	   presponse.toAscii().data(), pccardid,
	   ptype.toAscii().data(), pamount, pcurrid,
	   preforder.toAscii().data(), pccpayid);

  QDomDocument response;
  // YP doesn't even send back a valid XML doc!
  response.setContent("<yp_wrapper>" + presponse + "</yp_wrapper>");

  QDomNode node;
  QDomElement root = response.documentElement();

  QString r_approved;
  QString r_avs;
  QString r_code;
  QString r_error;
  QString r_message;
  QString r_ordernum;
  QString r_ref;
  QString r_score;
  QString r_shipping;
  QString r_tax;
  QString r_tdate;
  QString r_time;

  QString status;

  node = root.firstChild();
  while ( !node.isNull() )
  {
    if (node.isElement())
    {
      if (node.nodeName() == "r_approved" )
	r_approved = node.toElement().text();

      else if (node.nodeName() == "r_avs" )
	r_avs = node.toElement().text();

      else if (node.nodeName() == "r_code" )
	r_code = node.toElement().text();

      else if (node.nodeName() == "r_error" )
	r_error = node.toElement().text();

      else if (node.nodeName() == "r_message" )
	r_message = node.toElement().text();

      else if (node.nodeName() == "r_ordernum" )
	r_ordernum = node.toElement().text();

      else if (node.nodeName() == "r_ref" )
	r_ref = node.toElement().text();

      else if (node.nodeName() == "r_score" )
	r_score = node.toElement().text();

      else if (node.nodeName() == "r_shipping" )
	r_shipping = node.toElement().text();

      else if (node.nodeName() == "r_tax" )
	r_tax = node.toElement().text();

      else if (node.nodeName() == "r_tdate" )
	r_tdate = node.toElement().text();

      else if (node.nodeName() == "r_time" )
	r_time = node.toElement().text();
    }

    node = node.nextSibling();
  }

  if (isTest())
  {
    // in test mode YP doesn't send an approval code
    if (r_approved == "APPROVED" && r_code.isEmpty())
    r_code = "12345";

    // inject failures to test AVS and CVV checking but ONLY IN TEST MODE
    if (r_avs.isEmpty() && _metrics->value("CCTestResult") == "S")
    {
      switch (qrand() % 50)
      {
	case 0: r_avs = "NN";
		break;
	case 1: r_avs = "XN";
		break;
	case 2: r_avs = "YN";
		break;
	case 3: r_avs = "NX";
		break;
	case 4: r_avs = "XX";
		break;
	case 5: r_avs = "YX";
		break;
	default:
		r_avs = "YY";
		break;
      }
      switch (qrand() % 50)
      {
	case 0: r_avs += "N";
		break;
	case 1: r_avs += "P";
		break;
	case 2: r_avs += "S";
		break;
	case 3: r_avs += "U";
		break;
	default:
		r_avs += "M";
		break;
      }
    }
  }

  int returnValue = 0;
  if (r_approved == "APPROVED" || r_approved == "SUBMITTED")
  {
    _errorMsg = errorMsg(0).arg(r_ref);
    if (r_approved == "SUBMITTED")
    {
      _errorMsg = errorMsg(100).arg(r_approved);
      returnValue = 100;
    }

    if (ptype == "A")
      status = "A";	// Authorized
    else if (ptype == "V")
      status = "V";	// Voided
    else
      status = "C";	// Completed/Charged
  }

  else if (r_approved == "DENIED")
  {
    _errorMsg = errorMsg(-90).arg(r_message);
    returnValue = -90;
    status = "D";
  }

  else if (r_approved == "DUPLICATE")
  {
    _errorMsg = errorMsg(-91).arg(r_message);
    returnValue = -91;
    status = "D";
  }

  else if (r_approved == "DECLINED")
  {
    _errorMsg = errorMsg(-92).arg(r_error);
    returnValue = -92;
    status = "D";
  }

  else if (r_approved == "FRAUD")
  {
    _errorMsg = errorMsg(-93).arg(r_error);
    returnValue = -93;
    status = "D";
  }

  else if (r_approved.isEmpty())
  {
    _errorMsg = errorMsg(-100)
		     .arg(r_error).arg(r_message).arg(presponse);
    returnValue = -100;
    status = "X";
  }

  /* TODO: rewrite CreditCardProcessor::fraudChecks() as recommended in
           cybersourceprocessor.cpp, then rewrite this
   */
  // YP encodes AVS and CVV checking in the r_avs response field
  QRegExp avsRegExp("^[" + _metrics->value("CCAvsAddr") +
		    "][" + _metrics->value("CCAvsZIP") + "]");

  _passedAvs = _metrics->value("CCAvsCheck") == "X" ||
	       ! r_avs.contains(avsRegExp); // avsregexp matches failures

  _passedCvv = _metrics->value("CCCVVCheck") == "X" ||
	       ! r_avs.contains(QRegExp("[" + _metrics->value("CCCVVErrors") +
					"]$"));
  // end TODO

  _passedLinkShield = (! _metrics->boolean("CCYPLinkShield")) ||
	      (! r_score.isEmpty() &&
	       r_score.toInt() <= _metrics->value("CCYPLinkShieldMax").toInt());

  if (DEBUG)
    qDebug("YP:%s\t_passedAvs %d\t_passedCvv %d\t_passedLinkShield %d",
	    r_avs.toAscii().data(), _passedAvs, _passedCvv, _passedLinkShield);

  pparams.append("ccard_id",    pccardid);
  pparams.append("auth_charge", ptype);
  pparams.append("type",        ptype);
  pparams.append("reforder",    preforder.isEmpty() ? pneworder : preforder);
  pparams.append("status",      status);
  pparams.append("avs",         r_avs);
  pparams.append("ordernum",    pneworder);
  pparams.append("xactionid",   r_ordernum.isEmpty() ? pneworder : r_ordernum);
  pparams.append("error",       r_error);
  pparams.append("approved",    r_approved);
  pparams.append("code",        r_code);
  pparams.append("score",       r_score.toInt());
  pparams.append("shipping",    r_shipping);
  pparams.append("tax",         r_tax);
  pparams.append("tdate",       r_tdate);
  pparams.append("ref",         r_ref);
  pparams.append("message",     r_message);

  if (pcurrid != _ypcurrid)
  {
    pparams.append("fromcurr",  pcurrid);
    pparams.append("tocurr",    _ypcurrid);
  }
  else
    pparams.append("currid",    pcurrid);

  pparams.append("auth", QVariant(ptype == "A"));

  if (! r_time.isEmpty())
    pparams.append("time",     r_time);

  if (DEBUG)
    qDebug("YP:r_error.isEmpty() = %d", r_error.isEmpty());

  if (! r_error.isEmpty())
  {
    _errorMsg = errorMsg(-12).arg(r_error);
    returnValue = -12;
  }

  if (returnValue == 0)
    pparams.append("amount",   pamount);
  else
    pparams.append("amount",   0);	// no money changed hands this attempt

  return returnValue;
}

int YourPayProcessor::doTestConfiguration()
{
  if (DEBUG)
    qDebug("YP:doTestConfiguration()");

#ifdef Q_WS_WIN
   QString pemfile = _metrics->value("CCYPWinPathPEM");
#elif defined Q_WS_MACX
   QString pemfile = _metrics->value("CCYPMacPathPEM");
#elif defined Q_WS_X11
   QString pemfile = _metrics->value("CCYPLinPathPEM");
#endif

  if (pemfile.isEmpty())
  {
    _errorMsg = errorMsg(-15);
    return -15;
  }

  QFileInfo fileinfo(pemfile.toLatin1());
  if (!fileinfo.isFile())
  {
    _errorMsg = errorMsg(-16).arg(fileinfo.fileName());
   return -16;
  }

  if (_metricsenc->value("CCYPStoreNum").isEmpty())
  {
    _errorMsg = errorMsg(-103);
    return -103;
  }

  if (_metrics->boolean("CCYPLinkShield") &&
      _metrics->value("CCYPLinkShieldMax").toInt() <= 0)
  {
    _errorMsg = errorMsg(-102).arg(_metrics->value("CCYPLinkShieldMax"));
    return -102;
  }

  return 0;
}

void YourPayProcessor::reset()
{
  CreditCardProcessor::reset();
  _passedLinkShield = true;
}

int YourPayProcessor::fraudChecks()
{
  if (DEBUG)
    qDebug("YP:fraudChecks()");

  int returnValue = CreditCardProcessor::fraudChecks();
  if (returnValue < 0)
    return returnValue;

  else if (!_passedLinkShield)
  {
    _errorMsg = errorMsg(-106);
    returnValue = -106;
  }

  return returnValue;
}

bool YourPayProcessor::handlesCreditCards()
{
  return true;
}
