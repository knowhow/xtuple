/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include <QDebug>
#include <QDomDocument>
#include <QDomElement>
#include <QSqlError>
#include <QXmlQuery>

#include <currcluster.h>

#include "guiclient.h"
#include "cybersourceprocessor.h"
#include "version.h"
#include "xmessageboxmessagehandler.h"

#define DEBUG false

/** \ingroup creditcards
    \class   CyberSourceProcessor
    \brief   The implementation of CyberSource-specific credit card handling
 */

#define CPDATA_NS   "ns3"
#define CPTP_NS     "ns1"
#define SOAP_ENC_NS "SOAP-ENC"
#define SOAP_ENV_NS "SOAP-ENV"
#define WSSE_NS     "wsse"

#define CPDATA_NSVAL   "urn:schemas-cybersource-com:transaction-data-1.53"
#define CPTP_NSVAL     "urn:schemas-cybersource-com:transaction-data:TransactionProcessor"
#define SOAP_ENC_NSVAL "http://schemas.xmlsoap.org/soap/encoding/"
#define SOAP_ENV_NSVAL "http://schemas.xmlsoap.org/soap/envelope/"
#define WSSE_NSVAL     "http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-secext-1.0.xsd"

// convenience function to add <ChildName>Content</ChildName> to the Parent node
/* TODO: put this in a generic XMLCCProcessor, subclass of creditcardprocessor?
         make a generic XMLHelper class and make CyberSourceProcessor and
         YourPayProcessor inherit both CreditCardProcessor and XMLHelper?
 */

static QDomElement createChildTextNode(QDomElement parent, QString childName, QString content)
{
  QDomElement child;
  child = parent.ownerDocument().createElement(childName);
  child.appendChild(parent.ownerDocument().createTextNode(content));
  parent.appendChild(child);
  return child;
}

static QString get_r_ref(int pccpayid)
{
  XSqlQuery refq;
  refq.prepare("SELECT ccpay_r_ref FROM ccpay WHERE ccpay_id=:ccpay_id;");
  refq.bindValue(":ccpay_id", pccpayid);
  refq.exec();
  if (refq.first())
    return refq.value("ccpay_r_ref").toString();
  
  return QString();
  /* deal with errors if we learn that requestToken is required.
  else if (refq.lastError().type() != QSqlError::NoError)
  {
    _errorMsg = refq.lastError().databaseText();
    return -1;
  }
  else
  {
    _errorMsg = errorMsg(-17).arg(pccardid);
    return -17;
  }
  */
}

CyberSourceProcessor::CyberSourceProcessor() : CreditCardProcessor()
{
  _extraHeaders.append(qMakePair(QString("Content-Type"), QString("text/xml; charset=uft-8; action=\"runTransaction\"")));
  _extraHeaders.append(qMakePair(QString("SOAPAction"),   QString("runTransaction")));

  _defaultLivePort   = 0;
  _defaultLiveServer = "https://ics2ws.ic3.com/commerce/1.x/transactionProcessor";
  _defaultTestPort   = 0;
  _defaultTestServer = "https://ics2wstest.ic3.com/commerce/1.x/transactionProcessor";

  _msgHash.insert(-300, tr("The Merchant ID is required"));
  _msgHash.insert(-301, tr("The message sent to CyberSource was incomplete: %1"));
  _msgHash.insert(-302, tr("The message sent to CyberSource had invalid data in the following field: %1"));
  _msgHash.insert(-303, tr("CyberSource rejected this request (code %1)"));
  _msgHash.insert(-304, tr("CyberSource reports a general system failure"));
  _msgHash.insert(-305, tr("The Merchant ID %1 is too long"));
  _msgHash.insert(-306, tr("SOAP error (probably an xTuple ERP bug): %1"));
  _msgHash.insert(-307, tr("Error reading response XML: %1"));
  _msgHash.insert(-308, tr("CyberSource returned an error for this request (code %1)"));
  _msgHash.insert(-310, tr("The amount authorized was 0."));
  _msgHash.insert( 310, tr("Only a portion of the total amount requested was authorized"));

  if (FraudCheckResult *b = avsCodeLookup('B'))
  {
    b->sev  = NoMatch | PostalCode;
    b->text = tr("Street Address matches but Postal Code is not verified");
  }
  _avsCodes.append(new FraudCheckResult('C', NoMatch | Address | PostalCode, tr("Street Address and Postal Code do not match")));
  _avsCodes.append(new FraudCheckResult('D', Match,   tr("Street Address and Postal Code match")));

  if (FraudCheckResult *e = avsCodeLookup('E'))
    e->text = tr("AVS data are invalid or AVS not allowed for this card type");

  _avsCodes.append(new FraudCheckResult('F', NoMatch | Name, tr("Card holder's name does not match but Postal Code matches")));
  _avsCodes.append(new FraudCheckResult('H', NoMatch | Name, tr("Card holder's name does not match but Street Address and Postal Code match")));
  _avsCodes.append(new FraudCheckResult('I', NotProcessed | Address, tr("Address not verified")));
  _avsCodes.append(new FraudCheckResult('K', NoMatch | Address | PostalCode, tr("Card holder's name matches but Billing Street Address and Postal Code do not match")));
  _avsCodes.append(new FraudCheckResult('L', NoMatch | Address, tr("Card holder's name and Billing Postal Code match but Street Address does not match")));
  _avsCodes.append(new FraudCheckResult('M', Match,   tr("Street Address and Postal Code match")));
  _avsCodes.append(new FraudCheckResult('O', NoMatch | PostalCode, tr("Card holder's name and Billing Street Address match but Postal Code does not match")));

  if (FraudCheckResult *p = avsCodeLookup('P'))
  {
    p->sev  = NotProcessed | Address;
    p->text = tr("Postal Code matches but Street Address was not verified");
  }

  _avsCodes.append(new FraudCheckResult('T', NoMatch | Name,     tr("Card holder's name does not match but Street Address matches")));
  _avsCodes.append(new FraudCheckResult('U', NotProcessed | Address, tr("Address information unavailable; either the US bank does not support non-US AVS or the AVS at a US bank is not functioning properly")));
  _avsCodes.append(new FraudCheckResult('V', Match,       tr("Card holder's name, Street Address, and Postal Code match")));
  _avsCodes.append(new FraudCheckResult('1', Unsupported, tr("AVS is not supported for this processor or card type")));
  _avsCodes.append(new FraudCheckResult('2', Invalid,     tr("The processor returned an unrecognized AVS response")));

  _cvvCodes.append(new FraudCheckResult('D', Suspicious,  tr("The bank thinks this transaction is suspicious")));
  _cvvCodes.append(new FraudCheckResult('I', NoMatch,     tr("The CVV failed the processor's data validation check")));
  _cvvCodes.append(new FraudCheckResult('1', Unsupported, tr("CVV is not supported by the card association")));
  _cvvCodes.append(new FraudCheckResult('2', Invalid,     tr("The processor returned an unrecognized CVV response")));
  _cvvCodes.append(new FraudCheckResult('3', NotAvail,    tr("The processor did not return a CVV result")));

  _doc = 0;
}

CyberSourceProcessor::~CyberSourceProcessor()
{
  if (_doc)
  {
    delete _doc;
    _doc = 0;
  }
}

int CyberSourceProcessor::buildCommon(const int pccardid, const int pcvv, const double pamount, const int pcurrid, QString &pneworder, QString & /*preforder*/, const CCTransaction ptranstype)
{
  XSqlQuery csq;
  csq.prepare(
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
    "  formatbytea(decrypt(setbytea(ccard_year_expired),setbytea(:key), 'bf')) AS ccard_year_expired,"
    "  ccard_type, cust_name, cntct_email"
    "  FROM ccard"
    "  JOIN custinfo ON (ccard_cust_id=cust_id)"
    "  LEFT OUTER JOIN cntct ON (cust_cntct_id=cntct_id)"
    "WHERE (ccard_id=:ccardid);");

  csq.bindValue(":ccardid", pccardid);
  csq.bindValue(":key",     omfgThis->_key);
  csq.exec();

  if (csq.first())
  {
    if (!csq.value("ccard_active").toBool())
    {
      _errorMsg = errorMsg(-10);
      return -10;
    }
  }
  else if (csq.lastError().type() != QSqlError::NoError)
  {
    _errorMsg = csq.lastError().databaseText();
    return -1;
  }
  else
  {
    _errorMsg = errorMsg(-17).arg(pccardid);
    return -17;
  }

  if (_doc)
    delete _doc;
  _doc = new QDomDocument();

  QDomNode xmlNode = _doc->createProcessingInstruction("xml",
                                                       "version=\"1.0\" "
                                                       "encoding=\"UTF-8\"");
  _doc->appendChild(xmlNode);

  QDomElement envelope = _doc->createElement(SOAP_ENV_NS ":Envelope");
  envelope.setAttribute("xmlns:" CPDATA_NS,   CPDATA_NSVAL);
  envelope.setAttribute("xmlns:" CPTP_NS,     CPTP_NSVAL);
  envelope.setAttribute("xmlns:" SOAP_ENC_NS, SOAP_ENC_NSVAL);
  envelope.setAttribute("xmlns:" SOAP_ENV_NS, SOAP_ENV_NSVAL);
  envelope.setAttribute("xmlns:" WSSE_NS,     WSSE_NSVAL);

  _doc->appendChild(envelope);

  QDomElement header = _doc->createElement(SOAP_ENV_NS ":Header");
  envelope.appendChild(header);

  QDomElement security = _doc->createElement(WSSE_NS ":Security");
  security.setAttribute(SOAP_ENV_NS ":mustUnderstand", "1");
  header.appendChild(security);

  QDomElement usernametoken = _doc->createElement(WSSE_NS ":UsernameToken");
  security.appendChild(usernametoken);

  createChildTextNode(usernametoken, WSSE_NS ":Username",
                      _metricsenc->value("CCLogin").toLower());

  QDomElement password = createChildTextNode(usernametoken, WSSE_NS ":Password",
                                             _metricsenc->value("CCPassword"));

  password.setAttribute("Type",
                        "http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-username-token-profile-1.0#PasswordText");

  QDomElement body = _doc->createElement(SOAP_ENV_NS ":Body");
  envelope.appendChild(body);

  _requestMessage = _doc->createElement(CPDATA_NS ":requestMessage");
  body.appendChild(_requestMessage);

  createChildTextNode(_requestMessage, CPDATA_NS ":merchantID", 
                      _metricsenc->value("CCLogin"));
  createChildTextNode(_requestMessage, CPDATA_NS ":merchantReferenceCode",
                      pneworder);
  createChildTextNode(_requestMessage, CPDATA_NS ":clientLibrary", "xTuple ERP");
  createChildTextNode(_requestMessage, CPDATA_NS ":clientLibraryVersion", _Version);
  createChildTextNode(_requestMessage, CPDATA_NS ":clientEnvironment",
#ifdef Q_WS_MAC
                      "Mac OS X"
#elif defined Q_WS_WIN
                      "Windows"
#elif defined Q_WS_X11
                      "X Windows"
#else
                      "undefined Qt client"
#endif
                     );

  QString country;
  if (ptranstype == Authorize || ptranstype == Charge || ptranstype == Credit)
  {
    country = csq.value("ccard_country").toString();
    if (country.length() > 2)
    {
      XSqlQuery countryq;
      countryq.prepare("SELECT country_abbr FROM country WHERE country_name=:name;");
      countryq.bindValue(":name", country);
      countryq.exec();
      if (countryq.first())
        country = countryq.value("country_abbr").toString();
      else if (csq.lastError().type() != QSqlError::NoError)
      {
        _errorMsg = csq.lastError().databaseText();
        return -1;
      }
    }

    QDomElement billto = _doc->createElement(CPDATA_NS ":billTo");
    _requestMessage.appendChild(billto);
    QStringList name = csq.value("ccard_name").toString().split(QRegExp("\\s+"));
    createChildTextNode(billto, CPDATA_NS ":firstName",  name.at(0).left(60));
    createChildTextNode(billto, CPDATA_NS ":lastName",   name.at(name.size() - 1).left(60));
    createChildTextNode(billto, CPDATA_NS ":street1",    csq.value("ccard_address1").toString().left(60));
    createChildTextNode(billto, CPDATA_NS ":city",       csq.value("ccard_city").toString().left(50));
    createChildTextNode(billto, CPDATA_NS ":state",      csq.value("ccard_state").toString().left(2));
    createChildTextNode(billto, CPDATA_NS ":postalCode", csq.value("ccard_zip").toString().left(10));
    createChildTextNode(billto, CPDATA_NS ":country",    country.left(2));
    createChildTextNode(billto, CPDATA_NS ":company",    csq.value("cust_name").toString().left(40));
    createChildTextNode(billto, CPDATA_NS ":email",      csq.value("cntct_email").toString().left(255));
  }

  if (ptranstype == Authorize || ptranstype == Reverse ||
      ptranstype == Capture   || ptranstype == Charge  || ptranstype == Credit)
  {
    QDomElement purchasetotals = _doc->createElement(CPDATA_NS ":purchaseTotals");
    _requestMessage.appendChild(purchasetotals);

    XSqlQuery currq;
    currq.prepare("SELECT curr_abbr FROM curr_symbol WHERE (curr_id=:currid);");
    currq.bindValue(":currid", pcurrid);
    currq.exec();
    if (currq.first())
      createChildTextNode(purchasetotals, CPDATA_NS ":currency",
                          currq.value("curr_abbr").toString());
    else if (currq.lastError().type() != QSqlError::NoError)
    {
      _errorMsg = currq.lastError().databaseText();
      return -1;
    }
    else
    {
      _errorMsg = errorMsg(-17).arg(pccardid);
      return -17;
    }

    createChildTextNode(purchasetotals, CPDATA_NS ":grandTotalAmount",
                        QString::number(pamount));
  }

  if (ptranstype == Authorize || ptranstype == Charge || ptranstype == Credit)
  {
    QString rawcardtype = csq.value("ccard_type").toString();
    QString cardtype;
    if (rawcardtype == "A")
      cardtype = "003";
    else if (rawcardtype == "D")
      cardtype = "004";
    else if (rawcardtype == "M")
      cardtype = "002";
    else if (rawcardtype == "V")
      cardtype = "001";
    else if (rawcardtype == "Diners Club")
      cardtype = "005";
    else if (rawcardtype == "Carte Blanche")
      cardtype = "006";
    else if (rawcardtype == "JCB")
      cardtype = "007";
    else
      cardtype = rawcardtype;

    QDomElement card = _doc->createElement(CPDATA_NS ":card");
    _requestMessage.appendChild(card);
    QString accountNumber = csq.value("ccard_number").toString();
    accountNumber.remove(QRegExp("[^0-9]"));

    QString month = csq.value("ccard_month_expired").toString();
    if (month.length() == 1)
      month = "0" + month;

    createChildTextNode(card, CPDATA_NS ":accountNumber",   accountNumber.left(20));
    createChildTextNode(card, CPDATA_NS ":expirationMonth", month.left(2));
    createChildTextNode(card, CPDATA_NS ":expirationYear",
                        csq.value("ccard_year_expired").toString().left(4));
    if (pcvv > 0)
      createChildTextNode(card, CPDATA_NS ":cvNumber",
                          QString::number(pcvv).left(4));
    createChildTextNode(card, CPDATA_NS ":cardType",        cardtype.left(3));
  }

  if (DEBUG)
    qDebug("CS:buildCommon built %s\n", qPrintable(_doc->toString()));
  return 0;
}

int  CyberSourceProcessor::doAuthorize(const int pccardid, const int pcvv, double &pamount, const double ptax, const bool ptaxexempt, const double pfreight, const double pduty, const int pcurrid, QString& pneworder, QString& preforder, int &pccpayid, ParameterList &pparams)
{
  if (DEBUG)
    qDebug("CS:doAuthorize(%d, %d, %f, %f, %d, %f, %f, %d, %s, %s, %d)",
           pccardid, pcvv, pamount, ptax, ptaxexempt,  pfreight,  pduty,
           pcurrid, qPrintable(pneworder), qPrintable(preforder), pccpayid);

  int returnValue = buildCommon(pccardid, pcvv, pamount, pcurrid, pneworder,
                                preforder, Authorize);
  if (returnValue != 0)
    return returnValue;

  QDomElement auth = _doc->createElement(CPDATA_NS ":ccAuthService");
  auth.setAttribute("run", "true");
  _requestMessage.appendChild(auth);

  if (DEBUG) qDebug("CS::doAuthorize sending %s", qPrintable(_doc->toString()));

  QString response;
  returnValue = sendViaHTTP(_doc->toString(), response);
  if (returnValue < 0)
    return returnValue;

  returnValue = handleResponse(response, pccardid, Authorize, pamount, pcurrid,
                               pneworder, preforder, pccpayid, pparams);

  return returnValue;
}

int  CyberSourceProcessor::doCharge(const int pccardid, const int pcvv, const double pamount, const double ptax, const bool ptaxexempt, const double pfreight, const double pduty, const int pcurrid, QString& pneworder, QString& preforder, int &pccpayid, ParameterList &pparams)
{
  if (DEBUG)
    qDebug("CS:doCharge(%d, %d, %f, %f, %d, %f, %f, %d, %s, %s, %d)",
           pccardid, pcvv, pamount,  ptax, ptaxexempt,  pfreight,  pduty, pcurrid,
           qPrintable(pneworder), qPrintable(preforder), pccpayid);

  int returnValue = buildCommon(pccardid, pcvv, pamount, pcurrid, pneworder,
                                preforder, Charge);
  if (returnValue != 0)
    return returnValue;

  QDomElement auth = _doc->createElement(CPDATA_NS ":ccAuthService");
  auth.setAttribute("run", "true");
  _requestMessage.appendChild(auth);

  QDomElement capture = _doc->createElement(CPDATA_NS ":ccCaptureService");
  capture.setAttribute("run", "true");
  _requestMessage.appendChild(capture);

  if (DEBUG) qDebug("CS::doCharge sending %s", qPrintable(_doc->toString()));
  QString response;
  returnValue = sendViaHTTP(_doc->toString(), response);
  if (returnValue < 0)
    return returnValue;

  double amount = pamount;
  returnValue = handleResponse(response, pccardid, Charge, amount, pcurrid,
                               pneworder, preforder, pccpayid, pparams);

  return returnValue;
}

int CyberSourceProcessor::doChargePreauthorized(const int pccardid, const int pcvv, const double pamount, const int pcurrid, QString &pneworder, QString &preforder, int &pccpayid, ParameterList &pparams)
{
  if (DEBUG)
    qDebug("CS:doChargePreauthorized(%d, %d, %f, %d, %s, %s, %d)",
           pccardid, pcvv, pamount,  pcurrid,
           qPrintable(pneworder), qPrintable(preforder), pccpayid);

  int returnValue = buildCommon(pccardid, pcvv, pamount, pcurrid, pneworder, preforder, Capture);
  if (returnValue != 0)
    return returnValue;

  QDomElement capture = _doc->createElement(CPDATA_NS ":ccCaptureService");
  capture.setAttribute("run", "true");
  _requestMessage.appendChild(capture);

  /* TODO: test server returns an XML parse error with it, passes without it
  QString requestToken = get_r_ref(pccpayid);
  if (! requestToken.isEmpty())
    createChildTextNode(_requestMessage, CPDATA_NS ":orderRequestToken", requestToken);
   */

  createChildTextNode(capture, CPDATA_NS ":authRequestID",     preforder);

  if (DEBUG) qDebug("CS::doChargePreauthorized sending %s", qPrintable(_doc->toString()));
  QString response;
  returnValue = sendViaHTTP(_doc->toString(), response);
  if (returnValue < 0)
    return returnValue;

  double amount = pamount;
  returnValue = handleResponse(response, pccardid, Capture, amount, pcurrid,
                               pneworder, preforder, pccpayid, pparams);

  return returnValue;
}

int CyberSourceProcessor::doCredit(const int pccardid, const int pcvv, const double pamount, const double ptax, const bool ptaxexempt, const double pfreight, const double pduty, const int pcurrid, QString &pneworder, QString &preforder, int &pccpayid, ParameterList &pparams)
{
  if (DEBUG)
    qDebug("CS:doCredit(%d, %d, %f, %f, %d, %f, %f, %d, %s, %s, %d)",
           pccardid, pcvv, pamount, ptax, ptaxexempt,  pfreight,  pduty, pcurrid,
           qPrintable(pneworder), qPrintable(preforder), pccpayid);

  int returnValue = buildCommon(pccardid, pcvv, pamount, pcurrid, pneworder, preforder, Credit);
  if (returnValue != 0)
    return returnValue;

  QDomElement capture = _doc->createElement(CPDATA_NS ":ccCreditService");
  capture.setAttribute("run", "true");
  _requestMessage.appendChild(capture);

  createChildTextNode(capture, CPDATA_NS ":captureRequestID", preforder);

  QString requestToken = get_r_ref(pccpayid);
  if (! requestToken.isEmpty())
    createChildTextNode(_requestMessage, CPDATA_NS ":orderRequestToken", requestToken);

  if (DEBUG) qDebug("CS::doCredit sending %s", qPrintable(_doc->toString()));
  QString response;
  returnValue = sendViaHTTP(_doc->toString(), response);
  if (returnValue < 0)
    return returnValue;

  double amount = pamount;
  returnValue = handleResponse(response, pccardid, Credit, amount, pcurrid,
                               pneworder, preforder, pccpayid, pparams);
  return returnValue;
}

int CyberSourceProcessor::doReverseAuthorize(const int pccardid, const double pamount, const int pcurrid, QString& pneworder, QString& preforder, int &pccpayid, ParameterList &pparams)
{
  if (DEBUG)
    qDebug("CS::doReverseAuthorize(%d, %f, %d, %s, %s, %d, pparams",
           pccardid, pamount, pcurrid, qPrintable(pneworder), qPrintable(preforder), pccpayid);

  QString tmpErrorMsg = _errorMsg;
  int returnValue = buildCommon(pccardid, -2, pamount, pcurrid, pneworder, preforder, Reverse);
  if (returnValue != 0)
    return returnValue;

  QDomElement reverse = _doc->createElement(CPDATA_NS ":CCAuthReversalService");
  reverse.setAttribute("run", "true");
  _requestMessage.appendChild(reverse);

  createChildTextNode(reverse, CPDATA_NS ":authRequestID", preforder);

  QString requestToken = get_r_ref(pccpayid);
  if (! requestToken.isEmpty())
    createChildTextNode(_requestMessage, CPDATA_NS ":orderRequestToken", requestToken);

  if (DEBUG) qDebug("CS::doReverseAuthorize sending %s", qPrintable(_doc->toString()));

  QString response;
  returnValue = sendViaHTTP(_doc->toString(), response);
  _errorMsg = tmpErrorMsg;
  if (returnValue < 0)
    return returnValue;

  double amount = pamount;
  returnValue = handleResponse(response, pccardid, Reverse, amount, pcurrid,
                               pneworder, preforder, pccpayid, pparams);
  if (! tmpErrorMsg.isEmpty())
    _errorMsg = tmpErrorMsg;

  return returnValue;
}

int CyberSourceProcessor::doVoidPrevious(const int pccardid, const int pcvv, const double pamount, const int pcurrid, QString &pneworder, QString &preforder, QString &papproval, int &pccpayid, ParameterList &pparams)
{
  if (DEBUG)
    qDebug("CS::doVoidPrevious(%d, %d, %f, %d, %s, %s, %s, %d)",
           pccardid, pcvv, pamount, pcurrid, qPrintable(pneworder), qPrintable(preforder),
           qPrintable(papproval), pccpayid);

  QString requestId;
  XSqlQuery refq;
  refq.prepare("SELECT ccpay_r_ref FROM ccpay WHERE ccpay_id=:ccpay_id;");
  refq.bindValue(":ccpay_id", pccpayid);
  refq.exec();
  if (refq.first())
    requestId = refq.value("ccpay_r_ref").toString();
  else if (refq.lastError().type() != QSqlError::NoError)
  {
    _errorMsg = refq.lastError().databaseText();
    return -1;
  }
  else
  {
    _errorMsg = errorMsg(-17).arg(pccardid);
    return -17;
  }

  QString tmpErrorMsg = _errorMsg;
  int returnValue = buildCommon(pccardid, pcvv, pamount, pcurrid, pneworder, preforder, Void);
  if (returnValue != 0)
    return returnValue;

  QDomElement voidsvc = _doc->createElement(CPDATA_NS ":voidService");
  voidsvc.setAttribute("run", "true");
  _requestMessage.appendChild(voidsvc);

  createChildTextNode(voidsvc, CPDATA_NS ":voidRequestID",     preforder);

  /* TODO: test server returns an XML parse error with it, passes without it
  QString requestToken = get_r_ref(pccpayid);
  if (! requestToken.isEmpty())
    createChildTextNode(_requestMessage, CPDATA_NS ":orderRequestToken", requestToken);
   */

  if (DEBUG) qDebug("CS::doVoidPrevious sending %s", qPrintable(_doc->toString()));

  QString response;
  returnValue = sendViaHTTP(_doc->toString(), response);
  _errorMsg = tmpErrorMsg;
  if (returnValue < 0)
    return returnValue;

  double amount = pamount;
  returnValue = handleResponse(response, pccardid, Void, amount, pcurrid,
                               pneworder, preforder, pccpayid, pparams);
  if (! tmpErrorMsg.isEmpty())
    _errorMsg = tmpErrorMsg;

  return returnValue;
}

/** \brief Return the result of extracting a given value from an XML
           response from CyberSource.

    The input query can specify a full or relative path within the XML stream.
    If it starts with a slash ('/') then it's treated as a full path.
    Otherwise it's treated as a relative path with respect to
    \c soap:Envelope/soap:Body/c:replyMessage

    Queries with relative paths always return the text() value.
    Queries with full paths must completely specify what they want returned.
    In either case, the query is wrapped with namespace handling.

    The xq parameter is expected to be an initialized QXmlQuery with focus
    already set on the XML to be queried.
 */
QVariant CyberSourceProcessor::xqresult(QXmlQuery *xq, QString query, QVariant::Type type)
{
  if (DEBUG)
    qDebug("xqresult(%p, %s, %d)", xq, qPrintable(query), type);

  QVariant result;
  QString  text;
  QString fullq = (query.startsWith("/") ?
              QString("<result xmlns:soap='%1' xmlns:c='%2'>{%3}</result>")
            : QString("<result xmlns:soap='%1' xmlns:c='%2'>"
                      "{/soap:Envelope/soap:Body/c:replyMessage/%3/text()}"
                      "</result>")
            ).arg(SOAP_ENV_NSVAL, CPDATA_NSVAL, query);
  xq->setQuery(fullq);

  if (xq->isValid() && type == QVariant::StringList)
  {
    QStringList listresult;
    if (xq->evaluateTo(&listresult))
    {
      if (DEBUG)
        qDebug() << "xqresult() evaluateTo evaluated to" << listresult;
      result = listresult;
    }
    else if (xq->evaluateTo(&text))
    {
      text = text.remove(QRegExp("</?result[^>]*>")).simplified();
      result = text;
    }
  }
  else if (xq->isValid() && xq->evaluateTo(&text))
  {
    text = text.remove(QRegExp("</?result[^>]*>")).simplified();
    if (DEBUG)
      qDebug("xqresult() evaluated to %s", qPrintable(text));
    bool ok = false;
    switch (type)
    {
      case QVariant::Bool: result = text.compare("true", Qt::CaseInsensitive);
                           break;

      case QVariant::Date:
      case QVariant::DateTime:
      case QVariant::Time:
      {
        QDateTime tmp = QDateTime::fromString(text.left(19), Qt::ISODate);
        ok = tmp.isValid();
        if (ok)
        {
          tmp.setTimeSpec(Qt::UTC);
          switch (type)
          {
            case QVariant::Date: result = tmp.date(); break;
            case QVariant::Time: result = tmp.time(); break;
            default:             result = tmp;        break;
          }
        }
      }
      break;

      case QVariant::Double:   result = text.toDouble(&ok);             break;
      case QVariant::Int:      result = text.toInt(&ok);                break;
      case QVariant::String:   result = text; ok = true;                break;
      default:                 result = text;                           break;
    }
    if (! ok)
      xq->messageHandler()->message(QtCriticalMsg,
                                    tr("Could not convert '%1' to %2")
                                    .arg(text, QVariant::typeToName(type)),
                                    tr("CyberSource XML response"));
  }

  if (DEBUG)
    qDebug("xqresult() returning %s", qPrintable(result.toString()));
  return result;
}

int CyberSourceProcessor::handleResponse(const QString &presponse, const int pccardid, const CCTransaction ptype, double &pamount, const int pcurrid, QString &pneworder, QString &preforder, int &pccpayid, ParameterList &pparams)
{
  if (DEBUG)
    qDebug("CS::handleResponse(%s, %d, %d, %f, %d, %s, %d, pparams)",
           qPrintable(presponse), pccardid, ptype, pamount, pcurrid,
           qPrintable(preforder), pccpayid);

  QString r_approved;
  QString r_avs;
  QString r_code;
  QString r_cvv;
  QString r_error;
  QString r_message;
  QString r_ordernum;
  QString r_ref;
  QString r_shipping;
  QString r_tax;
  QString r_tdate;

  QString status;
  int     reasonCode = -1;
  QStringList missing;
  QStringList invalid;

  int returnValue = 0;

  QString requestedAmtStr;
  QString approvedAmtStr;
  XMessageBoxMessageHandler msghandler(this);
  msghandler.setTitle(tr("CyberSource XML Error"));
  msghandler.setPrefix(tr("<p>There was an error processing the response from "
                          "CyberSource, line %1 column %2: %3<br>(%4)"));

  QXmlQuery valueq;
  valueq.setMessageHandler(&msghandler);
  if (! valueq.setFocus(presponse))
  {
    _errorMsg = errorMsg(-307).arg(msghandler.lastMessage());
    returnValue = -307;
  }
  else
  {
    QVariant fieldVal;
    fieldVal = xqresult(&valueq, "/soap:Envelope/soap:Body/soap:Fault/faultstring/text()");
    if (! valueq.isValid())
    {
      _errorMsg   = errorMsg(-306).arg(msghandler.lastMessage());
      returnValue = -306;
    }
    else if (fieldVal.isValid() && ! fieldVal.toString().isEmpty())
    {
      _errorMsg   = errorMsg(-306).arg(fieldVal.toString());
      returnValue = -306;
    }
    else
    {
      fieldVal = xqresult(&valueq, "c:reasonCode", QVariant::Int);
      if (valueq.isValid() && fieldVal.isValid())
        reasonCode = fieldVal.toInt();
      else
      {
        _errorMsg = errorMsg(-307).arg(msghandler.lastMessage());
        returnValue = -307;
      }

      fieldVal = xqresult(&valueq, "c:decision", QVariant::String);
      if (valueq.isValid() && fieldVal.isValid())
        r_approved = fieldVal.toString();
      else
      {
        _errorMsg = errorMsg(-307).arg(msghandler.lastMessage());
        returnValue = -307;
      }

      fieldVal = xqresult(&valueq, "c:requestToken");
      if (valueq.isValid() && fieldVal.isValid())
      {
        preforder  = fieldVal.toString();
        r_ref = fieldVal.toString();
      }
      else
      {
        _errorMsg = errorMsg(-307).arg(msghandler.lastMessage());
        returnValue = -307;
      }

      fieldVal = xqresult(&valueq, "c:requestID");
      if (valueq.isValid() && fieldVal.isValid())
        r_ordernum = fieldVal.toString();
      else
      {
        _errorMsg = errorMsg(-307).arg(msghandler.lastMessage());
        returnValue = -307;
      }

      if (r_approved == "REJECT")
      {
        fieldVal = xqresult(&valueq, "/soap:Envelope/soap:Body/c:replyMessage/c:invalidField", QVariant::StringList);
        if (valueq.isValid() && fieldVal.isValid())
          invalid = fieldVal.toStringList();

        fieldVal = xqresult(&valueq, "/soap:Envelope/soap:Body/c:replyMessage/c:missingField", QVariant::StringList);
        if (valueq.isValid() && fieldVal.isValid())
          missing = fieldVal.toStringList();

        fieldVal = xqresult(&valueq, "c:ccAuthReply/c:avsCode");
        if (valueq.isValid() && fieldVal.isValid())
          r_avs = fieldVal.toString();

        fieldVal = xqresult(&valueq, "c:ccAuthReply/c:cvCode");
        if (valueq.isValid() && fieldVal.isValid())
          r_cvv = fieldVal.toString();
      }

      else if (r_approved == "ACCEPT")
      {
        switch (ptype)
        {
          case Authorize:
          {
            fieldVal = xqresult(&valueq, "c:ccAuthReply/c:amount");
            if (valueq.isValid() && fieldVal.isValid())
              approvedAmtStr = fieldVal.toString();
            else
            {
              _errorMsg = errorMsg(-307).arg(msghandler.lastMessage());
              returnValue = -307;
            }

            fieldVal = xqresult(&valueq, "c:ccAuthReply/c:authorizationCode");
            if (valueq.isValid() && fieldVal.isValid())
              r_code = fieldVal.toString();
            else
            {
              _errorMsg = errorMsg(-307).arg(msghandler.lastMessage());
              returnValue = -307;
            }

            fieldVal = xqresult(&valueq, "c:ccAuthReply/c:avsCode");
            if (valueq.isValid() && fieldVal.isValid())
              r_avs = fieldVal.toString();
            else
            {
              _errorMsg = errorMsg(-307).arg(msghandler.lastMessage());
              returnValue = -307;
            }

            fieldVal = xqresult(&valueq, "c:ccAuthReply/c:cvCode");
            if (valueq.isValid() && fieldVal.isValid())
              r_cvv = fieldVal.toString();
            else
            {
              _errorMsg = errorMsg(-307).arg(msghandler.lastMessage());
              returnValue = -307;
            }

            fieldVal = xqresult(&valueq, "c:ccAuthReply/c:authorizedDateTime", QVariant::DateTime);
            if (valueq.isValid() && fieldVal.isValid())
              r_tdate = fieldVal.toString();
            else
            {
              _errorMsg = errorMsg(-307).arg(msghandler.lastMessage());
              returnValue = -307;
            }

            // not always present
            fieldVal = xqresult(&valueq, "c:ccAuthReply/c:requestAmount");
            if (valueq.isValid() && fieldVal.isValid())
              requestedAmtStr = fieldVal.toString();
            else
            {
              _errorMsg = errorMsg(-307).arg(msghandler.lastMessage());
              returnValue = -307;
            }
          }
          break;

          case Reverse:
          {
            fieldVal = xqresult(&valueq, "c:ccAuthReversalReply/c:amount");
            if (valueq.isValid() && fieldVal.isValid())
              approvedAmtStr = fieldVal.toString();
            else
            {
              _errorMsg = errorMsg(-307).arg(msghandler.lastMessage());
              returnValue = -307;
            }

            fieldVal = xqresult(&valueq, "c:ccAuthReversalReply/c:authorizationCode");
            if (valueq.isValid() && fieldVal.isValid())
              r_code = fieldVal.toString();
            else
            {
              _errorMsg = errorMsg(-307).arg(msghandler.lastMessage());
              returnValue = -307;
            }
          }
          break;

          case Capture:
          {
            fieldVal = xqresult(&valueq, "c:ccCaptureReply/c:amount");
            if (valueq.isValid() && fieldVal.isValid())
              approvedAmtStr = fieldVal.toString();
            else
            {
              _errorMsg = errorMsg(-307).arg(msghandler.lastMessage());
              returnValue = -307;
            }
          }
          break;

          case Charge:
            break;

          case Credit:
          {
            fieldVal = xqresult(&valueq, "c:ccCreditReply/c:amount");
            if (valueq.isValid() && fieldVal.isValid())
              approvedAmtStr = fieldVal.toString();
            else
            {
              _errorMsg = errorMsg(-307).arg(msghandler.lastMessage());
              returnValue = -307;
            }
          }
          break;

          case Void:
          {
            fieldVal = xqresult(&valueq, "c:ccVoidReply/c:amount");
            if (valueq.isValid() && fieldVal.isValid())
              approvedAmtStr = fieldVal.toString();
            else
            {
              _errorMsg = errorMsg(-307).arg(msghandler.lastMessage());
              returnValue = -307;
            }
          }
          break;
        }
      }

      // let the switch below handle "else if (r_approved == "ERROR")"
    }
  }

  switch (reasonCode)
  {
    case 100: _errorMsg = r_message = errorMsg(0).arg(r_approved);
              break;
    case 101: _errorMsg = r_message = errorMsg(-301).arg(missing.join(tr(", ")));
              returnValue = -301;
              break;
    case 102:
            {
              returnValue = -302;
              if (r_avs == "2")
                _errorMsg = r_message = avsCodeLookup('2')->text;
              else if (r_cvv == "2")
                _errorMsg = r_message = cvvCodeLookup('2')->text;
              else if (invalid.isEmpty())
                _errorMsg = r_message = errorMsg(returnValue).arg(tr("[no invalid fields listed]"));
              else
                _errorMsg = r_message = errorMsg(returnValue).arg(invalid.join(tr(", ")));
            }
            break;
    case 110:   // partial approval
            {
              pamount = approvedAmtStr.toDouble();

              ParameterList reverseParams;
              int reverseResult = doReverseAuthorize(pccardid, pamount,
                                                     pcurrid, pneworder,
                                                     preforder, pccpayid,
                                                     reverseParams);
              if (reverseResult)
                returnValue = reverseResult;
              else if (pamount == 0)
              {
                _errorMsg = r_message = errorMsg(-310).arg(r_approved);
                returnValue = -310;
              }
              else
              {
                _errorMsg = r_message = errorMsg(310).arg(r_approved);
                returnValue = 310;
              }
            }
            break;
    case 150: _errorMsg = r_message = errorMsg(-304);
              break;
    case -1:    // assume _errorMsg and returnValue have already been set
              break;
    default:
             returnValue = (r_approved == "REJECT") ?
                             -303 : (r_approved == "ERROR") ? -308 : -95; 
             _errorMsg = r_message = errorMsg(returnValue).arg(reasonCode);
             break;
  }

  if (r_approved == "ACCEPT" || r_approved == "REVIEW")
  {
    _errorMsg = errorMsg(0).arg(r_code);
    if (ptype == Authorize)
      status = "A";
    else if (ptype == Void)
      status = "V";
    else
      status = "C";
  }
  else if (r_approved == "REJECT")
  {
    _errorMsg = errorMsg(-90).arg(r_message);
    if (returnValue >= 0)
      returnValue = -90;
    status = "D";
  }
  else if (r_approved == "ERROR")
  {
    r_error   = r_message;
    _errorMsg = errorMsg(-12).arg(r_message);
    returnValue = -12;
    status = "X";
  }
  else  // assume _errorMsg and returnValue have already been set
  {
    r_error = _errorMsg;
    status = "X";
  }

  // TODO: move this up to CreditCardProcessor::fraudChecks()
  // TODO: change avs and cvv failure check configuration
  CreditCardProcessor::FraudCheckResult *avsresult = avsCodeLookup(r_avs.at(0));
  if (avsresult)
  {
    bool addrMustMatch   = _metrics->value("CCAvsAddr").contains("N");
    bool addrMustBeAvail = _metrics->value("CCAvsAddr").contains("X");
    bool zipMustMatch    = _metrics->value("CCAvsZip").contains("N");
    bool zipMustBeAvail  = _metrics->value("CCAvsZip").contains("X");

    if (avsresult->sev & Name & NoMatch)
      _passedAvs = false;
    else if ((avsresult->sev & Address & NoMatch) && addrMustMatch)
      _passedAvs = false;
    else if ((avsresult->sev & Address & NotAvail) && addrMustBeAvail)
      _passedAvs = false;
    else if ((avsresult->sev & PostalCode & NoMatch) && zipMustMatch)
      _passedAvs = false;
    else if ((avsresult->sev & PostalCode & NotAvail) && zipMustBeAvail)
      _passedAvs = false;
    else if (avsresult->sev != Match)
      _passedAvs = false;
    else
      _passedAvs = true;
  }
  else
  {
    _passedAvs = false;
  }

  CreditCardProcessor::FraudCheckResult *cvvresult = cvvCodeLookup(r_cvv.at(0));
  if (cvvresult)
  {
    bool cvvMustMatch         = _metrics->value("CCCVVErrors").contains("N");
    bool cvvMustBeProcessed   = _metrics->value("CCCVVErrors").contains("P");
    bool cvvMustBeOnCard      = _metrics->value("CCCVVErrors").contains("S");
    bool cvvIssuerMustBeValid = _metrics->value("CCCVVErrors").contains("U");

    if ((cvvresult->sev & NoMatch) && cvvMustMatch)
      _passedCvv = false;
    else if ((cvvresult->sev & NotProcessed) && cvvMustBeProcessed)
      _passedCvv = false;
    else if ((cvvresult->sev & NotAvail) && cvvMustBeOnCard)
      _passedCvv = false;
    else if ((cvvresult->sev & IssuerNotCertified) && cvvIssuerMustBeValid)
      _passedCvv = false;
    else if (cvvresult->sev != Match)
      _passedCvv = false;
    else
      _passedCvv = true;
  }
  else
  {
    _passedCvv = false;
  }

  if (DEBUG)
    qDebug("CS::%s _passedAvs %d\t%s _passedCvv %d",
            qPrintable(r_avs), _passedAvs, qPrintable(r_cvv), _passedCvv);
  // end TODO

  if (r_approved.isEmpty() && ! r_message.isEmpty())
  {
    _errorMsg = errorMsg(-95).arg(r_message);
    returnValue = -95;
    status = "X";
  }

  else if (r_approved.isEmpty())
  {
    _errorMsg = errorMsg(-100).arg(r_error).arg(r_message).arg(presponse);
    returnValue = -100;
    status = "X";
  }

  pparams.append("ccard_id",    pccardid);
  pparams.append("currid",      pcurrid);
  pparams.append("auth_charge", typeToCode(ptype));
  pparams.append("type",        typeToCode(ptype));
  pparams.append("reforder",    (preforder.isEmpty()) ? pneworder : preforder);
  pparams.append("status",      status);
  pparams.append("avs",         r_avs);
  pparams.append("ordernum",    pneworder);
  pparams.append("xactionid",   r_ordernum);
  pparams.append("error",       r_error);
  pparams.append("approved",    r_approved);
  pparams.append("code",        r_code);
  pparams.append("shipping",    r_shipping);
  pparams.append("tax",         r_tax);
  pparams.append("ref",         r_ref);
  pparams.append("message",     r_message);
  pparams.append("tdate",       r_tdate);

  pparams.append("auth", QVariant(ptype == Authorize));

  if (DEBUG)
    qDebug("CS::r_error.isEmpty() = %d", r_error.isEmpty());

  if (returnValue >= 0)
    pparams.append("amount", pamount);
  else
    pparams.append("amount", 0);      // no money changed hands this attempt

  if (DEBUG)
    qDebug("CS::handleResponse returning %d %s", returnValue, qPrintable(errorMsg()));
  return returnValue;
}

int CyberSourceProcessor::doTestConfiguration()
{
  if (DEBUG)
    qDebug("CS::doTestConfiguration() entered");

  int returnValue = 0;

  if (_metricsenc->value("CCLogin").isEmpty())
  {
    _errorMsg = errorMsg(-300);
    returnValue = -300;
  }
  else if (_metricsenc->value("CCLogin").size() > 30)
  {
    _errorMsg = errorMsg(-305).arg(_metricsenc->value("CCLogin"));
    returnValue = -305;
  }

  if (DEBUG)
    qDebug("CS::doTestConfiguration() returning %d", returnValue);
  return returnValue;
}

bool CyberSourceProcessor::handlesCreditCards()
{
  return true;
}
