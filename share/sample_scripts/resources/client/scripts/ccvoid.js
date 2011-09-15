/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

/* a completely scripted implementation of voiding previously-created
   credit card transactions through authorize.net. the window
   contains a list of transactions available to void, a button to
   void the currently-selected transaction, and a close button.
 */

var _ccp    = toolbox.getCreditCardProcessor();
if (_ccp == null)
  QMessageBox.critical(mywindow, qsTr("No Credit Card Processor"),
                       qsTr("Could not get a credit card processor"));
var _currid = -1;
var _transactions = mywindow.findChild("_transactions");
var _void         = mywindow.findChild("_void");

// with authorize.net, the CCServer is really a full url except for the port
var _url = new QUrl(metrics.value("CCServer"));
if (metrics.value("CCPort") > 0)
   _url.setPort(metrics.value("CCPort"));
else
   _url.setPort(_ccp.defaultPort());

var _netmgr = new QNetworkAccessManager(mywindow);

// network requests are asynchronous; let sHandleResponse deal with the reply when it comes
_netmgr.authenticationRequired.connect(sHandleAuthenticationRequired);
_netmgr.proxyAuthenticationRequired.connect(sHandleProxyAuthenticationRequired);
_netmgr.sslErrors.connect(sHandleSslError);

_netmgr.finished.connect(sHandleResponse);
_void.clicked.connect(sVoid);

_transactions.addColumn("Cust. #",     -1, 1, true, "cust_number");
_transactions.addColumn("Name",        -1, 1, true, "cust_name");
_transactions.addColumn("Type",        -1, 1, true, "type");
_transactions.addColumn("Transaction", -1, 1, true, "status");
_transactions.addColumn("Order-Seq.",  -1, 2, true, "docnumber");
_transactions.addColumn("Amount",      -1, 2, true, "ccpay_amount");
_transactions.addColumn("Currency",    -1, 1, true, "ccpay_currAbbr");

function sHandleAuthenticationRequired(reply, authenticator)
{
  print("sHandleAuthenticationRequired called");
  toolbox.listProperties(reply);
  toolbox.listProperties(authenticator);
}

function sHandleProxyAuthenticationRequired(proxy, authenticator)
{
  print("sHandleProxyAuthenticationRequired called");
  toolbox.listProperties(proxy);
  toolbox.listProperties(authenticator);
}

function sHandleSslError(reply, errors)
{
  print("sHandleSslError called");
  toolbox.listProperties(reply);
  toolbox.listProperties(errors);
}

function sPopulateTransactions()
{
  var params = new Object;

  params.preauth        = "Preauthorization";
  params.charge         = "Charge";
  params.refund         = "Refund/Credit";
  params.authorized     = "Authorized";
  params.approved       = "Approved";
  params.declined       = "Declined/Denied";
  params.voided         = "Voided";
  params.noapproval     = "Not Approved";
  if (metrics.value("CCValidDays") > 0)
    params.ccValidDays  = metrics.value("CCValidDays");
  else
    params.ccValidDays  = 7;

  _transactions.populate(toolbox.executeDbQuery("ccpayments", "list", params));
}

// convenience function to help build the message to send
function APPENDFIELD(request, name, content)
{
  if (request != "")
    request = request + "&";
  return request + name + "=" + content;
}

/* this was translated from the C++ sources
   authorizedotnetprocessor.cpp and creditcardprocessor.cpp
 */
function sVoid()
{
  try {
    // local error checks
    if (_transactions.currentItem().text("status") == "Voided")
    {
      QMessageBox.warning(mywindow, qsTr("Cannot Void"),
                          qsTr("This transaction cannot be voided a second time."));
      return;
    }

    // build the message
    var params = new Object;
    params.ccpayid = _transactions.id;
    params.key     = mainwindow.key;
    var anq = toolbox.executeQuery(
          "SELECT ccard_active,"
        + "  formatbytea(decrypt(setbytea(ccard_number),   setbytea(<? value(\"key\" ?>),'bf'))   AS ccard_number,"
        + "  formatccnumber(decrypt(setbytea(ccard_number),setbytea(<? value(\"key\" ?>),'bf')) AS ccard_number_x,"
        + "  formatbytea(decrypt(setbytea(ccard_name),     setbytea(<? value(\"key\" ?>),'bf')) AS ccard_name,"
        + "  formatbytea(decrypt(setbytea(ccard_address1), setbytea(<? value(\"key\" ?>),'bf')) AS ccard_address1,"
        + "  formatbytea(decrypt(setbytea(ccard_address2), setbytea(<? value(\"key\" ?>),'bf')) AS ccard_address2,"
        + "  formatbytea(decrypt(setbytea(ccard_city),     setbytea(<? value(\"key\" ?>),'bf')) AS ccard_city,"
        + "  formatbytea(decrypt(setbytea(ccard_state),    setbytea(<? value(\"key\" ?>),'bf')) AS ccard_state,"
        + "  formatbytea(decrypt(setbytea(ccard_zip),      setbytea(<? value(\"key\" ?>),'bf')) AS ccard_zip,"
        + "  formatbytea(decrypt(setbytea(ccard_country),  setbytea(<? value(\"key\" ?>),'bf')) AS ccard_country,"
        + "  formatbytea(decrypt(setbytea(ccard_month_expired),setbytea(<? value(\"key\" ?>),'bf')) AS ccard_month_expired,"
        + "  formatbytea(decrypt(setbytea(ccard_year_expired),setbytea(<? value(\"key\" ?>), 'bf')) AS ccard_year_expired,"
        + "  cust.*,"
        + "  ccpay.*,"
        + "  curr_symbol.*"
        + "  FROM ccard, cust, ccpay, curr_symbol "
        + "WHERE ((ccard_cust_id=cust_id)"
        + "  AND  (ccard_id=ccpay_ccard_id)"
        + "  AND  (curr_id=ccpay_curr_id)"
        + "  AND  (ccpay_id=<? value(\"ccpayid\") ?>));",
          params);

    if (anq.first())
    {
      if (! anq.value("ccard_active"))
      {
        QMessageBox.critical(mywindow, qsTr("Cannot Void"),
                             _ccp.errorMsg(-10).replace("%1", anq.value("ccard_number_x")));
        return -10;
      }
    }
    else if (anq.lastError().type != 0)
    {
      QMessageBox.critical(mywindow, qsTr("Cannot Void"),
                           qsTr("A database error occurred: %1").arg(anq.lastError().text));
      return -1;
    }
    else
    {
      QMessageBox.critical(mywindow, qsTr("Cannot Void"), _ccp.errorMsg(-17));
      return -17;
    }

    var prequest = "";
    _currid = anq.value("ccpay_curr_id");
    if (metrics.value("CCANDelim").length > 0)
      prequest = APPENDFIELD(prequest, "x_delim_char", metrics.value("CCANDelim"));

    prequest = APPENDFIELD(prequest, "x_version",      metrics.value("CCANVer"));
    prequest = APPENDFIELD(prequest, "x_delim_data", "TRUE");

    if (metrics.value("CCANEncap").length > 0)
      prequest = APPENDFIELD(prequest, "x_encap_char", metrics.value("CCANEncap"));

    prequest = APPENDFIELD(prequest, "x_login",    metricsenc.value("CCLogin"));
    prequest = APPENDFIELD(prequest, "x_tran_key", metricsenc.value("CCPassword"));
    prequest = APPENDFIELD(prequest, "x_amount",   _transactions.column("ccpay_amount").text);
    prequest = APPENDFIELD(prequest, "x_card_num", anq.value("ccard_number"));
    prequest = APPENDFIELD(prequest, "x_test_request", _ccp.isLive() ? "FALSE" : "TRUE");

    var work_month = anq.value("ccard_month_expired");
    if (work_month < 10 && work_month > 0)
      work_month = "0" + work_month;
    prequest = APPENDFIELD(prequest, "x_exp_date",
                           work_month + anq.value("ccard_year_expired") % 100);

    prequest = APPENDFIELD(prequest, "x_relay_response", "FALSE");
    prequest = APPENDFIELD(prequest, "x_duplicate_window", metrics.value("CCANDuplicateWindow"));

    var name = anq.value("ccard_name").split(/\s+/);
    prequest = APPENDFIELD(prequest, "x_first_name", name[0]);
    prequest = APPENDFIELD(prequest, "x_last_name",  name[name.length - 1]);
    prequest = APPENDFIELD(prequest, "x_company",    anq.value("cust_name"));
    prequest = APPENDFIELD(prequest, "x_address",    anq.value("ccard_address1"));
    prequest = APPENDFIELD(prequest, "x_city",       anq.value("ccard_city"));
    prequest = APPENDFIELD(prequest, "x_state",      anq.value("ccard_state"));
    prequest = APPENDFIELD(prequest, "x_zip",        anq.value("ccard_zip"));
    prequest = APPENDFIELD(prequest, "x_country",    anq.value("ccard_country"));

    if (metrics.CCANWellsFargoSecureSource)
    {
      prequest = APPENDFIELD(prequest, "x_phone",    anq.value("cust_phone"));
      prequest = APPENDFIELD(prequest, "x_email",    anq.value("cust_email"));
    }

    prequest = APPENDFIELD(prequest, "x_currency_code", anq.value("curr_abbr"));
    prequest = APPENDFIELD(prequest, "x_method",        "CC");
    prequest = APPENDFIELD(prequest, "x_type",          "VOID");
    
    prequest = APPENDFIELD(prequest, "x_trans_id",
                           _ccp.isLive() ? anq.value("ccpay_r_ordernum") : 12345);

    // post is asynchronous. while we're waiting, lock out the user so s/he doesn't try again
    _transactions.enabled = false;
    _void.enabled         = false;

    var netreq = new QNetworkRequest();
    netreq.setUrl(_url);

    _netmgr.post(netreq, prequest);
  }
  catch (e)
  {
    print("sVoid exception at " + e.lineNumber + ": " + e);
    _transactions.enabled = true;
    _void.enabled         = true;
  }
}

function sHandleResponse(netresponse)
{
  try
  {
    print("sHandleResponse entered");
    if (netresponse.error())
    {
      QMessageBox.critical(mywindow, qsTr("Error during data transfer"),
                   qsTr("<p>There was a network error trying to post the "
                      + "transaction: %1."
                      + "Look up this error code at "
                      + " <a>http://doc.trolltech.com/4.4/qnetworkreply.html"
                      + "#NetworkError-enum</a>").arg(netresponse.error()));
      return;
    }

    // parse the reply, which is a QVariant(QByteArray) so first grab it as a string
    var response = netresponse.readAll().toString();
    if (response.match(/^<HTML>/i))
    {
      QMessageBox.critical(mywindow, qsTr("Error returned from A.N"),
                         response);
      return;
    }

    // field[N] here corresponds to fieldValue(..., N+1, ...) in the C++.
    // Authorize.Net uses 1-based arrays, JavaScript uses 0-based arrays, and
    // C++ uses 0-based arrays but the C++ function fieldValue translates
    var field = response.split(metrics.value("CCANDelim").length > 0 ?
                                        metrics.value("CCANDelim") : ",");
    var params = new Object;
  
    if (field[0] == 2)
    {
      QMessageBox.critical(mywindow, qsTr("Declined"),
                           qsTr("The void transaction was declined"));
      return;
    }
    else if (field[0] == 3)
    {
      QMessageBox.critical(mywindow, qsTr("Error"),
                         qsTr("The void transaction received an error: %1")
                         .arg(field[3]));
      return;
    }
    else if (field[0] == 1)
      params.approved = "APPROVED";
    else if (field[0] == 4)
      params.approved = "HELDFORREVIEW";
    else
    {
      QMessageBox.critical(mywindow, qsTr("Error"),
                           qsTr("The void transaction received an unknown "
                              + "approval value: %1").arg(field[0]));
      return;
    }

    params.amount     = _transactions.currentItem().rawValue("ccpay_amount");
    params.ccpay_id   = _transactions.id;
    params.currid     = _currid;
    params.status     = "V";
    params.auth_charge= "V";
    params.type       = "V";
    params.reforder   = _transactions.column("docnumber").text;
    params.avs        = field[5];
    params.ordernum   = "";
    params.xactionid  = field[6];
    params.code       = field[4];
    params.shipping   = field[34];
    params.tax        = field[32];
    params.ref        = "";
    params.message    = field[5];
    params.auth       = false;

    // report the result
    var ccq = toolbox.executeQuery( 
             'UPDATE ccpay SET'
           + '<? if exists("fromcurr") ?>'
           + '      ccpay_amount=ROUND(currToCurr(<? value("fromcurr") ?>,'
           + '                                    <? value("tocurr") ?>,'
           + '                                    <? value("amount") ?>,'
           + '                                    CURRENT_DATE), 2),'
           + '       ccpay_curr_id=<? value("currid") ?>,'
           + '<? else ?>'
           + '       ccpay_amount=ROUND(<? value("amount") ?>, 2),'
           + '       ccpay_curr_id=<? value("currid") ?>,'
           + '<? endif ?>'
           + '       ccpay_auth=<? value("auth") ?>,'
           + '       ccpay_r_approved=<? value("approved") ?>,'
           + '       ccpay_r_avs=<? value("avs") ?>,'
           + '       ccpay_r_code=<? value("code") ?>,'
           + '       ccpay_r_error=<? value("error") ?>,'
           + '       ccpay_r_message=<? value("message") ?>,'
           + '       ccpay_r_ordernum=<? value("ordernum") ?>,'
           + '       ccpay_r_ref=<? value("ref") ?>,'
           + '<? if exists("shipping") ?>'
           + '       ccpay_r_shipping=<? value("shipping") ?>,'
           + '<? endif ?>'
           + '<? if exists("score") ?>'
           + '       ccpay_yp_r_score=<? value("score") ?>,'
           + '<? endif ?>'
           + '<? if exists("tax") ?>'
           + '       ccpay_r_tax=<? value("tax") ?>,'
           + '<? endif ?>'
           + '<? if exists("tdate") ?>'
           + '       ccpay_yp_r_tdate=<? value("tdate") ?>,'
           + '<? endif ?>'
           + '<? if exists("time") ?>'
           + '       ccpay_yp_r_time=<? value("time")?>,'
           + '<? endif ?>'
           + '       ccpay_status=<? value("status") ?>'
           + ' WHERE (ccpay_id=<? value("ccpay_id") ?>);',
           params);
    if (ccq.lastError().type > 0)
    {
      QMessageBox.critical(mywindow, qsTr("Query Failed"),
                         qsTr("There was an error querying the database: %1")
                         .arg(ccq.lastError().text));
      return;
    }

    QMessageBox.information(mywindow, qsTr("Transaction Complete"),
                       qsTr("<p>The ccpay table has been updated after "
                          + "completing the Void transaction."));
    sPopulateTransactions();
  }
  catch (e)
  {
    print("sHandleResponse exception at " + e.lineNumber + ": " + e);
  }
  finally
  {  // allow the user to process another
    print("sHandleResponse entered finally block");
    _transactions.enabled = true;
    _void.enabled         = (_transactions.id != -1);
  }
}

sPopulateTransactions();
